/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_attn_tables.h"
/*
 * omega_gpu_decode_attn256_api.c: resident KV cache, RoPE table and launch for the head_dim 256 decode attention
 * (omega-mv#544). A derivative of omega_gpu_decode_attn_api.c (see the header); the double-precision host reference
 * lives in tests/gpu_decode_attn256_test.c so the library links without libm. Session use mirrors the 128 kernel:
 * every buffer from omega_gpu_session_alloc (GPU-uncached, host-visible), sync_to_gpu after host writes, sync_to_cpu
 * before host reads, one omega_gpu_session_launch per call.
 */
#include "omega_gpu_decode_attn256_api.h"
#include "omega_gpu_session.h"
#include "omega_gpu_fmt.h"
#include "omega_gpu_hostmath.h"
#include "omega_api_shells.h"
#include <string.h>
#include "omega_gpu_cmdlist.h"

#define MARKER_TIMEOUT_MS 20000ull
#define POISON_F32 (-999.0f)
#define HD OMEGA_DATTN256_HEAD_DIM
#define RB OMEGA_DATTN256_ROW_BYTES
#define KVH OMEGA_DATTN256_KV_HEADS
#define OUTW OMEGA_DATTN256_OUT_WORDS

struct OmegaGpuDecodeAttn256Cache {
    uint32_t layers, max_pos, max_splits, rotary_dim;
    OmegaDecodeAttn256Spec shape;             /* q_heads / kv_heads / q_per_cta (0 = the 27B) */
    OmegaDattn256Shape sh;
    double theta;
    uint64_t plane_bytes;                     /* one K or V plane of a layer: 4 * max_pos * 512 + slack */
    OmegaGpuMem kv[OMEGA_GPU_DATTN256_MAX_LAYERS]; /* [K plane | V plane] per layer */
    OmegaGpuMem rope, norms, part, cnt, qkv, out;
};

typedef struct {
    bool valid;
    OmegaDecodeAttn256Spec spec;
    OmegaBlackwellKernel kernel;
    OmegaGpuMem code;
} Slot;

static struct {
    bool hooked;
    Slot slot[3];                             /* the decode kernel and the L369 multi-token prep and attend kernels */
    uint32_t depth, per_split;
    int mutant, multi_mut;
    OmegaGpuMem mpart, mcnt;                  /* L369 multi-token partials and counters, shared by every cache (launches run in order) */
} g;
static int g_rec; /* 1 while a record call runs (lock held): kernel_for must not evict */

static void slot_clear_locked(void) { for (unsigned i = 0; i < 3; i++) { if (g.slot[i].valid) omega_gpu_session_free(&g.slot[i].code); av_memset(&g.slot[i], 0, sizeof g.slot[i]); } }
static void on_session_close(void) { av_memset(g.slot, 0, sizeof g.slot); av_memset(&g.mpart, 0, sizeof g.mpart); av_memset(&g.mcnt, 0, sizeof g.mcnt); }
static OmegaGpuSession *dev_open_locked(void) {
    if (!g.hooked) { (void)omega_gpu_session_on_close(on_session_close); g.hooked = true; }
    return omega_gpu_session_open();
}
static uint32_t gpr_declared(uint32_t peak) { return omega_bw_gpr_declared(peak); }

void omega_gpu_decode_attn256_set_depth(uint32_t depth) { omega_gpu_session_lock(); g.depth = depth; slot_clear_locked(); omega_gpu_session_unlock(); }
void omega_gpu_decode_attn256_set_per_split(uint32_t per_split) { omega_gpu_session_lock(); g.per_split = per_split; omega_gpu_session_unlock(); }
void omega_gpu_decode_attn256_test_set_mutant(int mutant) { omega_gpu_session_lock(); g.mutant = mutant; slot_clear_locked(); omega_gpu_session_unlock(); }

/* ---- host math ---- */
void omega_dattn256_rope_row(uint32_t pos, double theta, uint32_t rotary_dim, float *row256) {
    av_ropegen(row256, pos, theta, rotary_dim, 256, 1);
}
/* ---- cache ---- */
int omega_gpu_decode_attn256_cache_alloc(uint32_t layers, uint32_t max_pos, double rope_theta, uint32_t rotary_dim, OmegaGpuDecodeAttn256Cache **out) {
    return omega_gpu_decode_attn256_cache_alloc_shape(layers, max_pos, rope_theta, rotary_dim, 0, 0, 0, out);
}

int omega_gpu_decode_attn256_cache_alloc_shape(uint32_t layers, uint32_t max_pos, double rope_theta, uint32_t rotary_dim, uint32_t q_heads,
                                               uint32_t kv_heads, uint32_t q_per_cta, OmegaGpuDecodeAttn256Cache **out) {
    if (!out) return OMEGA_GPU_DATTN_BAD_ARGS;
    OmegaDecodeAttn256Spec shape; av_memset(&shape, 0, sizeof shape);
    shape.q_heads = q_heads; shape.kv_heads = kv_heads; shape.q_per_cta = q_per_cta;
    OmegaDattn256Shape sh;
    if (omega_dattn256_shape(&shape, &sh) || omega_api_vy16(sh.vy)) return OMEGA_GPU_DATTN_BAD_ARGS;
    *out = NULL;
    uint64_t th = 0;
    av_memcpy(&th, &rope_theta, sizeof th);
    if (omega_api_dcache(layers, max_pos, rotary_dim) || !omega_api_thok((uint32_t)th, (uint32_t)(th >> 32))) return OMEGA_GPU_DATTN_BAD_ARGS;
    OmegaGpuDecodeAttn256Cache *c = omega_gpu_host_calloc(1, sizeof *c);
    if (!c) return OMEGA_GPU_DATTN_CHIP_FAIL;
    c->layers = layers; c->max_pos = max_pos; c->theta = rope_theta; c->rotary_dim = rotary_dim;
    c->shape = shape; c->sh = sh;
    c->max_splits = omega_api_splt(max_pos);
    c->plane_bytes = omega_api_plane(c->sh.kvh, max_pos);
    int rc = OMEGA_GPU_DATTN_OK;
    omega_gpu_session_lock();
    if (!dev_open_locked()) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    for (uint32_t L = 0; L < layers && rc == OMEGA_GPU_DATTN_OK; L++) if (omega_gpu_session_alloc((size_t)(2 * c->plane_bytes), &c->kv[L]) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    if (rc == OMEGA_GPU_DATTN_OK && (omega_gpu_session_alloc((size_t)max_pos * OMEGA_DATTN256_ROPE_ROW_BYTES, &c->rope) != 0 ||
                                     omega_gpu_session_alloc((size_t)layers * 2 * HD * 4, &c->norms) != 0 ||
                                     omega_gpu_session_alloc((size_t)omega_api_hostb(c->sh.vy * c->max_splits, OMEGA_DATTN256_PARTIAL_WORDS), &c->part) != 0 ||
                                     omega_gpu_session_alloc(64, &c->cnt) != 0 ||
                                     omega_gpu_session_alloc((size_t)omega_api_hostb(1, c->sh.qkv_words), &c->qkv) != 0 ||
                                     omega_gpu_session_alloc((size_t)omega_api_hostb(1, c->sh.out_words), &c->out) != 0)) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    if (rc == OMEGA_GPU_DATTN_OK) {
        float *r = c->rope.cpu;
        av_ropegen(r, 0, rope_theta, rotary_dim, 256, max_pos);
        av_memset(c->cnt.cpu, 0, 64);
        av_memset(c->norms.cpu, 0, (size_t)layers * 2 * HD * 4);
        for (uint32_t L = 0; L < layers; L++) av_memset(c->kv[L].cpu, 0, (size_t)(2 * c->plane_bytes));
        if (omega_gpu_session_sync_to_gpu(&c->rope) != 0 || omega_gpu_session_sync_to_gpu(&c->cnt) != 0 || omega_gpu_session_sync_to_gpu(&c->norms) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
        for (uint32_t L = 0; L < layers && rc == OMEGA_GPU_DATTN_OK; L++) if (omega_gpu_session_sync_to_gpu(&c->kv[L]) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    }
    omega_gpu_session_unlock();
    if (rc != OMEGA_GPU_DATTN_OK) { omega_gpu_decode_attn256_cache_free(c); return rc; }
    *out = c;
    return OMEGA_GPU_DATTN_OK;
}
void omega_gpu_decode_attn256_cache_free(OmegaGpuDecodeAttn256Cache *c) {
    if (!c) return;
    omega_gpu_session_lock();
    for (uint32_t L = 0; L < c->layers; L++) if (c->kv[L].cpu) omega_gpu_session_free(&c->kv[L]);
    OmegaGpuMem *ms[] = { &c->rope, &c->norms, &c->part, &c->cnt, &c->qkv, &c->out };
    for (unsigned i = 0; i < 6; i++) if (ms[i]->cpu) omega_gpu_session_free(ms[i]);
    omega_gpu_session_unlock();
    omega_gpu_host_free(c);
}
int omega_gpu_decode_attn256_set_norms(OmegaGpuDecodeAttn256Cache *c, uint32_t layer, const float *q_norm, const float *k_norm) {
    if (!c || layer >= c->layers || !q_norm || !k_norm) return OMEGA_GPU_DATTN_BAD_ARGS;
    omega_gpu_session_lock();
    float *n = (float *)c->norms.cpu + (size_t)layer * 2 * HD;
    av_memcpy(n, q_norm, HD * 4); av_memcpy(n + HD, k_norm, HD * 4);
    int rc = omega_gpu_session_sync_to_gpu(&c->norms) == 0 ? OMEGA_GPU_DATTN_OK : OMEGA_GPU_DATTN_CHIP_FAIL;
    omega_gpu_session_unlock();
    return rc;
}
static uint16_t *row_ptr(OmegaGpuDecodeAttn256Cache *c, uint32_t layer, uint32_t kv_head, uint32_t pos, int plane) {
    return (uint16_t *)((uint8_t *)c->kv[layer].cpu + (plane ? c->plane_bytes : 0) + ((size_t)kv_head * c->max_pos + pos) * RB);
}
int omega_gpu_decode_attn256_cache_write(OmegaGpuDecodeAttn256Cache *c, uint32_t layer, uint32_t kv_head, uint32_t pos, uint32_t count, const uint16_t *k, const uint16_t *v) {
    if (!c || layer >= c->layers || kv_head >= c->sh.kvh || omega_api_roomok(count, pos, c->max_pos) || !k || !v) return OMEGA_GPU_DATTN_BAD_ARGS;
    omega_gpu_session_lock();
    av_memcpy(row_ptr(c, layer, kv_head, pos, 0), k, (size_t)count * RB); av_memcpy(row_ptr(c, layer, kv_head, pos, 1), v, (size_t)count * RB);
    int rc = omega_gpu_session_sync_to_gpu(&c->kv[layer]) == 0 ? OMEGA_GPU_DATTN_OK : OMEGA_GPU_DATTN_CHIP_FAIL;
    omega_gpu_session_unlock();
    return rc;
}
int omega_gpu_decode_attn256_cache_read(OmegaGpuDecodeAttn256Cache *c, uint32_t layer, uint32_t kv_head, uint32_t pos, uint16_t *k, uint16_t *v) {
    if (!c || layer >= c->layers || kv_head >= c->sh.kvh || pos >= c->max_pos || !k || !v) return OMEGA_GPU_DATTN_BAD_ARGS;
    omega_gpu_session_lock();
    int rc = omega_gpu_session_sync_to_cpu(&c->kv[layer]) == 0 ? OMEGA_GPU_DATTN_OK : OMEGA_GPU_DATTN_CHIP_FAIL;
    if (rc == OMEGA_GPU_DATTN_OK) { av_memcpy(k, row_ptr(c, layer, kv_head, pos, 0), RB); av_memcpy(v, row_ptr(c, layer, kv_head, pos, 1), RB); }
    omega_gpu_session_unlock();
    return rc;
}

/* ---- kernel ---- */
static int kernel_for(const OmegaDecodeAttn256Spec *spec, Slot **out, bool *hit) {
    Slot *s = NULL;
    for (unsigned i = 0; i < 3; i++) if (g.slot[i].valid && memcmp(&g.slot[i].spec, spec, sizeof *spec) == 0) { *out = &g.slot[i]; *hit = true; return OMEGA_GPU_DATTN_OK; }
    for (unsigned i = 0; i < 3 && !s; i++) if (!g.slot[i].valid) s = &g.slot[i];
    if (!s && g_rec) { omega_gpu_session_set_error("decode attn256 record: kernel cache miss would evict a resident kernel (warm up before recording)"); return OMEGA_GPU_DATTN_TOO_LARGE; }
    if (!s) { slot_clear_locked(); s = &g.slot[0]; }
    BlackwellIRProgram *prog = omega_gpu_host_calloc(1, sizeof *prog);
    if (!prog) { omega_gpu_session_set_error("decode attn256 codegen: host memory"); return OMEGA_GPU_DATTN_CODEGEN_FAIL; }
    int cg = omega_blackwell_codegen_decode_attn256_prog(spec, prog);
    size_t need = cg == 0 ? omega_bw_program_code_bytes(prog) : 0;
    if (cg != 0 || need == 0) { omega_gpu_host_free(prog); omega_gpu_session_set_error("decode attn256 codegen refused the spec"); return OMEGA_GPU_DATTN_CODEGEN_FAIL; }
    if (!omega_api_regfit(OMEGA_DATTN256_CHUNK / 32u, gpr_declared(prog->regalloc.peak_gpr_usage))) { omega_gpu_host_free(prog); omega_gpu_session_set_error("decode attn256: register file"); return OMEGA_GPU_DATTN_TOO_LARGE; }
    OmegaGpuMem code;
    if (omega_gpu_session_alloc_code(need, &code) != 0) { omega_gpu_host_free(prog); return OMEGA_GPU_DATTN_CHIP_FAIL; }
    OmegaBlackwellKernel kernel;
    cg = omega_blackwell_kernel_encode_into(prog, code.cpu, need, &kernel);
    omega_gpu_host_free(prog);
    if (cg != 0 || !kernel.code || kernel.code_size != need) { omega_gpu_session_free(&code); omega_gpu_session_set_error("decode attn256 encode"); return OMEGA_GPU_DATTN_CODEGEN_FAIL; }
    if (omega_gpu_session_sync_to_gpu(&code) != 0) { omega_gpu_session_free(&code); return OMEGA_GPU_DATTN_CHIP_FAIL; }
    av_memset(s, 0, sizeof *s);
    s->valid = true; s->spec = *spec; s->kernel = kernel; s->code = code;
    *out = s; *hit = false;
    return OMEGA_GPU_DATTN_OK;
}

/* rec non-NULL (record path, caller holds the session lock): qkv / out are the device buffers dev_qkv / dev_out, the
 * launch is appended to rec instead of submitted, nothing is staged, poisoned or read back. */
static int dattn_core(OmegaGpuDecodeAttn256Cache *c, uint32_t layer, uint32_t pos, const float *qkv, float eps, float out_scale,
                      float *out, OmegaGpuDecodeAttn256Info *info, OmegaGpuCmdList *rec, uint64_t dev_qkv, uint64_t dev_out) {
    uint64_t t0 = omega_gpu_session_now_ns();
    if (info) av_memset(info, 0, sizeof *info);
    if (!c || layer >= c->layers || pos >= c->max_pos || (rec ? (!dev_qkv || !dev_out) : (!qkv || !out))) return OMEGA_GPU_DATTN_BAD_ARGS;
    uint32_t code = 0, splits = 0;
    omega_api_dsplit(pos, g.per_split, c->max_splits, &code, &splits);
    if (code == 1u) return OMEGA_GPU_DATTN_BAD_ARGS;
    if (code == 2u) return OMEGA_GPU_DATTN_TOO_LARGE;
    const uint32_t per = g.per_split ? g.per_split : OMEGA_GPU_DATTN256_PER_SPLIT;
    OmegaDecodeAttn256Spec spec; av_memset(&spec, 0, sizeof spec);
    spec.depth = g.depth ? g.depth : OMEGA_GPU_DATTN256_DEPTH; spec.mutant = g.mutant;
    spec.q_heads = c->shape.q_heads; spec.kv_heads = c->shape.kv_heads; spec.q_per_cta = c->shape.q_per_cta;

    if (!rec) omega_gpu_session_lock();
#define UNLOCK() do { if (!rec) omega_gpu_session_unlock(); } while (0)
    int rc = OMEGA_GPU_DATTN_OK;
    if (!dev_open_locked()) { UNLOCK(); return OMEGA_GPU_DATTN_CHIP_FAIL; }
    Slot *ks = NULL; bool hit = false;
    g_rec = rec != NULL;
    rc = kernel_for(&spec, &ks, &hit);
    g_rec = 0;
    if (rc != OMEGA_GPU_DATTN_OK) { UNLOCK(); return rc; }
    float *od = c->out.cpu;
    if (!rec) {
        av_memcpy(c->qkv.cpu, qkv, (size_t)c->sh.qkv_words * 4);
        av_fillf32(od, c->sh.out_words, POISON_F32);
        av_memset(c->cnt.cpu, 0, 64);
        if (omega_gpu_session_sync_to_gpu(&c->qkv) != 0 || omega_gpu_session_sync_to_gpu(&c->out) != 0 || omega_gpu_session_sync_to_gpu(&c->cnt) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    }

    uint32_t args[32];
    const uint64_t kc = c->kv[layer].va, vc = kc + c->plane_bytes, nm = c->norms.va + (uint64_t)layer * 2 * HD * 4;
    const uint64_t qkv_va = rec ? dev_qkv : c->qkv.va, out_va = rec ? dev_out : c->out.va;
    float fe = eps, fo = out_scale, fq = (float)(0.0625 * 1.4426950408889634); /* log2(e) / sqrt(256) */
    uint32_t ue, uo, uq; av_memcpy(&ue, &fe, 4); av_memcpy(&uo, &fo, 4); av_memcpy(&uq, &fq, 4);
    const uint64_t pack[32] = { qkv_va, kc, vc, c->rope.va, nm, nm + HD * 4, out_va, c->part.va, c->cnt.va,
        pos, per, splits, (uint32_t)(c->max_pos * RB), ue, uo, uq };
    if (av_args(args, pack, 9, OMEGA_DATTN256_ARGS_WORDS)) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    if (info) {
        info->layer = layer; info->pos = pos; info->splits = splits; info->per_split = per; info->depth = spec.depth;
        info->grid_x = splits; info->grid_y = c->sh.vy; info->threads = OMEGA_DATTN256_CHUNK;
        info->kv_bytes = 2ull * c->sh.kvh * (pos + 1ull) * RB;
        info->kernel_cache_hit = hit;
    }
    if (rc == OMEGA_GPU_DATTN_OK) {
        OmegaGpuLaunch L = {
            .code_va = ks->code.va, .gpr_count = gpr_declared(ks->kernel.gpr_count),
            .threads_x = OMEGA_DATTN256_CHUNK, .threads_y = 1, .grid_x = splits, .grid_y = c->sh.vy,
            .num_elements = (size_t)c->sh.out_words,
            .args = args, .n_args = OMEGA_DATTN256_ARGS_WORDS, .timeout_ms = MARKER_TIMEOUT_MS,
            .shared_bytes = OMEGA_DATTN256_SHARED_BYTES,
        };
        if (!omega_bw_gpr_declared_ok(L.gpr_count, ks->kernel.gpr_count)) { omega_gpu_session_set_error("declared registers below peak + 2: refusing to launch"); rc = OMEGA_GPU_DATTN_CHIP_FAIL; }
        else if (rec) { if (omega_gpu_cmdlist_add(rec, &L) < 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL; }
        else if (omega_gpu_session_launch(&L, info ? &info->elapsed_ns : NULL, info ? &info->completion_marker : NULL) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    }
    if (rc == OMEGA_GPU_DATTN_OK && !rec && (omega_gpu_session_sync_to_cpu(&c->out) != 0 || omega_gpu_session_sync_to_cpu(&c->cnt) != 0)) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    if (rc == OMEGA_GPU_DATTN_OK && !rec) {
        size_t unchanged = 0;
        unchanged = av_countf32(od, c->sh.out_words, POISON_F32);
        const uint32_t *cn = c->cnt.cpu;
        uint32_t any = 0;
        any = av_any32(cn, c->sh.vy);
        if (unchanged) { omega_gpu_session_set_error("decode attn256: output poison survived"); rc = OMEGA_GPU_DATTN_CHIP_FAIL; }
        else if (any) { omega_gpu_session_set_error("decode attn256: a split counter did not return to zero"); rc = OMEGA_GPU_DATTN_CHIP_FAIL; }
        else av_memcpy(out, od, (size_t)c->sh.out_words * 4);
    }
    UNLOCK();
#undef UNLOCK
    if (info) info->call_ns = omega_gpu_session_now_ns() - t0;
    return rc;
}

int omega_gpu_decode_attn256_cache_view(const OmegaGpuDecodeAttn256Cache *c, uint32_t layer, OmegaGpuDecodeAttn256CacheView *v) {
    if (!c || !v || layer >= c->layers) return OMEGA_GPU_DATTN_BAD_ARGS;
    v->layers = c->layers; v->max_pos = c->max_pos; v->rotary_dim = c->rotary_dim; v->theta = c->theta;
    v->k_va = c->kv[layer].va; v->v_va = c->kv[layer].va + c->plane_bytes;
    v->rope_va = c->rope.va;
    v->q_norm_va = c->norms.va + (uint64_t)layer * 2 * HD * 4; v->k_norm_va = v->q_norm_va + HD * 4;
    v->part_va = c->part.va; v->cnt_va = c->cnt.va;
    v->q_heads = c->sh.qh; v->kv_heads = c->sh.kvh; v->q_per_cta = c->sh.h; v->qkv_words = c->sh.qkv_words; v->out_words = c->sh.out_words;
    return OMEGA_GPU_DATTN_OK;
}

int omega_gpu_decode_attn256(OmegaGpuDecodeAttn256Cache *c, uint32_t layer, uint32_t pos, const float *qkv, float eps, float out_scale, float *out, OmegaGpuDecodeAttn256Info *info) {
    return dattn_core(c, layer, pos, qkv, eps, out_scale, out, info, NULL, 0, 0);
}

int omega_gpu_decode_attn256_record(OmegaGpuCmdList *cl, OmegaGpuDecodeAttn256Cache *c, uint32_t layer, uint32_t pos, uint64_t qkv_va,
                                    float eps, float out_scale, uint64_t out_va) {
    if (!cl) return OMEGA_GPU_DATTN_BAD_ARGS;
    return dattn_core(c, layer, pos, NULL, eps, out_scale, NULL, NULL, cl, qkv_va, out_va);
}

int omega_gpu_decode_attn256_counters_check(OmegaGpuDecodeAttn256Cache *c) {
    if (!c || !c->cnt.cpu) return OMEGA_GPU_DATTN_BAD_ARGS;
    if (omega_gpu_session_sync_to_cpu(&c->cnt) != 0) return OMEGA_GPU_DATTN_CHIP_FAIL;
    const uint32_t *cn = c->cnt.cpu;
    uint32_t any = av_any32(cn, c->sh.vy);
    if (any) { omega_gpu_session_set_error("decode attn256: a split counter did not return to zero"); return OMEGA_GPU_DATTN_CHIP_FAIL; }
    return OMEGA_GPU_DATTN_OK;
}

/* ---- L369 multi-token prompt attention ---- */
#define MULTI_PART_BYTES (64ull << 20)
#define MULTI_MAX_TOKENS 128u
#define PB ((uint64_t)OMEGA_DATTN256_PARTIAL_WORDS * 4u)

void omega_gpu_decode_attn256_test_set_multi_mutant(int mutant) { omega_gpu_session_lock(); g.multi_mut = mutant; omega_gpu_session_unlock(); }

static void multi_spec(const OmegaGpuDecodeAttn256Cache *c, uint32_t prep, OmegaDecodeAttn256Spec *spec) {
    av_memset(spec, 0, sizeof *spec);
    spec->depth = g.depth ? g.depth : OMEGA_GPU_DATTN256_DEPTH; spec->mutant = g.mutant; spec->prep = prep; spec->multi = 1;
    spec->q_heads = c->shape.q_heads; spec->kv_heads = c->shape.kv_heads; spec->q_per_cta = c->shape.q_per_cta;
}

int omega_gpu_decode_attn256_multi_prepare(OmegaGpuDecodeAttn256Cache *c) {
    if (!c || omega_api_p2bad(c->sh.vy)) return OMEGA_GPU_DATTN_BAD_ARGS;
    omega_gpu_session_lock();
    int rc = dev_open_locked() ? OMEGA_GPU_DATTN_OK : OMEGA_GPU_DATTN_CHIP_FAIL;
    if (rc == OMEGA_GPU_DATTN_OK && !g.mpart.cpu) {
        if (omega_gpu_session_alloc((size_t)MULTI_PART_BYTES, &g.mpart) != 0 || omega_gpu_session_alloc((size_t)MULTI_MAX_TOKENS * 16u * 4u, &g.mcnt) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
        else { av_memset(g.mcnt.cpu, 0, (size_t)MULTI_MAX_TOKENS * 16u * 4u); if (omega_gpu_session_sync_to_gpu(&g.mcnt) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL; }
    }
    for (uint32_t prep = 0; prep < 2 && rc == OMEGA_GPU_DATTN_OK; prep++) {
        OmegaDecodeAttn256Spec spec; multi_spec(c, prep, &spec);
        Slot *s = NULL; bool hit = false;
        rc = kernel_for(&spec, &s, &hit);
    }
    omega_gpu_session_unlock();
    return rc;
}

int omega_gpu_decode_attn256_multi_record(OmegaGpuCmdList *cl, OmegaGpuDecodeAttn256Cache *c, uint32_t layer, uint32_t pos0, uint32_t tokens,
                                          uint64_t qkv_va, uint32_t xstride, float eps, float out_scale, uint64_t out_va, uint32_t ostride) {
    if (!cl || !c || layer >= c->layers || omega_api_roomok(tokens, pos0, c->max_pos) || !qkv_va || !out_va ||
        omega_api_stride((uint32_t)omega_api_hostb(1, c->sh.qkv_words), xstride, 15u) || !omega_api_alignok((uint32_t)qkv_va, 15u) ||
        omega_api_stride((uint32_t)omega_api_hostb(1, c->sh.out_words), ostride, 3u) || !g.mpart.cpu) return OMEGA_GPU_DATTN_BAD_ARGS;
    if (!omega_api_persok(g.per_split) || omega_api_p2bad(c->sh.vy)) return OMEGA_GPU_DATTN_BAD_ARGS;
    Slot *sp = NULL, *sa = NULL; bool hit = false;
    OmegaDecodeAttn256Spec spec;
    g_rec = 1;
    multi_spec(c, 1, &spec); int rc = kernel_for(&spec, &sp, &hit);
    if (rc == OMEGA_GPU_DATTN_OK) { multi_spec(c, 0, &spec); rc = kernel_for(&spec, &sa, &hit); }
    g_rec = 0;
    if (rc != OMEGA_GPU_DATTN_OK) return rc;
    uint32_t args[32];
    const uint64_t kc = c->kv[layer].va, vc = kc + c->plane_bytes, nm = c->norms.va + (uint64_t)layer * 2 * HD * 4;
    float fe = eps, fo = out_scale, fq = (float)(0.0625 * 1.4426950408889634);
    uint32_t ue, uo, uq; av_memcpy(&ue, &fe, 4); av_memcpy(&uo, &fo, 4); av_memcpy(&uq, &fq, 4);
    uint64_t pack[32] = { [2]=kc, [4]=vc, [6]=c->rope.va, [8]=nm, [10]=nm + HD * 4,
        [14]=g.mpart.va, [16]=g.mcnt.va, [19]=OMEGA_DATTN256_CHUNK, [21]=omega_api_kvsb(c->max_pos),
        [22]=ue, [23]=uo, [24]=uq, [25]=xstride, [26]=ostride };
    const uint64_t address_mask = (1ull<<0)|(1ull<<2)|(1ull<<4)|(1ull<<6)|(1ull<<8)|
        (1ull<<10)|(1ull<<12)|(1ull<<14)|(1ull<<16);
    /* prep: every token's K/V rows into the cache (TEST mutant 1: skipped, the chunk's own rows stay stale) */
    if (g.multi_mut != 1) {
        pack[0]=qkv_va; pack[12]=out_va; pack[18]=pos0; pack[20]=1; pack[27]=0;
        if (av_layout(args, pack, address_mask)) return OMEGA_GPU_DATTN_CHIP_FAIL;
        OmegaGpuLaunch L = { .code_va = sp->code.va, .gpr_count = gpr_declared(sp->kernel.gpr_count), .threads_x = OMEGA_DATTN256_CHUNK, .threads_y = 1,
                             .grid_x = tokens, .grid_y = c->sh.vy, .num_elements = (size_t)tokens * c->sh.qkv_words, .args = args,
                             .n_args = OMEGA_DATTN256_MULTI_ARGS_WORDS, .timeout_ms = MARKER_TIMEOUT_MS, .shared_bytes = OMEGA_DATTN256_SHARED_BYTES };
        if (!omega_bw_gpr_declared_ok(L.gpr_count, sp->kernel.gpr_count) || omega_gpu_cmdlist_add(cl, &L) < 0) return OMEGA_GPU_DATTN_CHIP_FAIL;
    }
    /* attend: groups of tokens whose partials fit the shared buffer */
    for (uint32_t t0 = 0; t0 < tokens;) {
        uint64_t packed = omega_api_mtgrp(tokens - t0, pos0, t0, c->sh.vy);
        uint32_t n = (uint32_t)packed, smax = (uint32_t)(packed >> 32);
        const uint64_t ps = omega_api_mtps(c->sh.vy, smax);
        if (omega_api_mtbig(n, c->sh.vy, smax)) return OMEGA_GPU_DATTN_TOO_LARGE;
        pack[0]=qkv_va + (uint64_t)t0 * xstride; pack[12]=out_va + (uint64_t)t0 * ostride;
        pack[18]=pos0 + t0; pack[20]=smax; pack[27]=ps;
        if (av_layout(args, pack, address_mask)) return OMEGA_GPU_DATTN_CHIP_FAIL;
        OmegaGpuLaunch L = { .code_va = sa->code.va, .gpr_count = gpr_declared(sa->kernel.gpr_count), .threads_x = OMEGA_DATTN256_CHUNK, .threads_y = 1,
                             .grid_x = smax, .grid_y = n * c->sh.vy, .num_elements = (size_t)n * c->sh.out_words, .args = args,
                             .n_args = OMEGA_DATTN256_MULTI_ARGS_WORDS, .timeout_ms = MARKER_TIMEOUT_MS, .shared_bytes = OMEGA_DATTN256_SHARED_BYTES };
        if (!omega_bw_gpr_declared_ok(L.gpr_count, sa->kernel.gpr_count) || omega_gpu_cmdlist_add(cl, &L) < 0) return OMEGA_GPU_DATTN_CHIP_FAIL;
        t0 += n;
    }
    return OMEGA_GPU_DATTN_OK;
}
