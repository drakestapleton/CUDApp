/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_attn_tables.h"
/*
 * omega_gpu_decode_attn_api.c: resident KV cache, RoPE table and launch for the one-launch decode attention (the
 * double-precision host reference lives in tests/gpu_decode_attn_test.c so the library links without libm). See
 * attention (see the header). Session use mirrors omega_gpu_fp8_api.c: every buffer from omega_gpu_session_alloc
 * (GPU-uncached, host-visible), sync_to_gpu after host writes, sync_to_cpu before host reads, one
 * omega_gpu_session_launch per call.
 */
#include "omega_gpu_decode_attn_api.h"
#include "omega_host_policy.h"
#include "omega_api_plan.h"
#include "omega_gpu_session.h"
#include "omega_gpu_fmt.h"
#include "omega_gpu_hostmath.h"
#include <string.h>
#include "omega_gpu_cmdlist.h"

#define MARKER_TIMEOUT_MS 20000ull
#define POISON_F32 (-999.0f)

struct OmegaGpuDecodeAttnCache {
    uint32_t layers, max_pos, max_splits;
    double theta;
    uint64_t plane_bytes;                     /* one K or V plane of a layer: 4 * max_pos * 256 + slack */
    OmegaGpuMem kv[OMEGA_GPU_DATTN_MAX_LAYERS]; /* [K plane | V plane] per layer */
    OmegaGpuMem rope, norms, part, cnt, qkv, out;
};

typedef struct {
    bool valid;
    OmegaDecodeAttnSpec spec;
    OmegaBlackwellKernel kernel;
    OmegaGpuMem code;
} Slot;

static struct {
    bool hooked;
    Slot slot;
    uint32_t depth, per_split;
    int mutant;
} g;
static int g_rec; /* 1 while a record call runs (lock held): kernel_for must not evict */

static void slot_clear_locked(void) { if (g.slot.valid) omega_gpu_session_free(&g.slot.code); av_memset(&g.slot, 0, sizeof g.slot); }
static void on_session_close(void) { av_memset(&g.slot, 0, sizeof g.slot); }
static OmegaGpuSession *dev_open_locked(void) {
    if (!g.hooked) { (void)omega_gpu_session_on_close(on_session_close); g.hooked = true; }
    return omega_gpu_session_open();
}
static uint32_t gpr_declared(uint32_t peak) { return omega_bw_gpr_declared(peak); } /* peak + 2 rounded up to 8: the chip hung at peak + 1 (omega_blackwell_codegen.h) */

void omega_gpu_decode_attn_set_depth(uint32_t depth) { omega_gpu_session_lock(); g.depth = depth; slot_clear_locked(); omega_gpu_session_unlock(); }
void omega_gpu_decode_attn_set_per_split(uint32_t per_split) { omega_gpu_session_lock(); g.per_split = per_split; omega_gpu_session_unlock(); }
void omega_gpu_decode_attn_test_set_mutant(int mutant) { omega_gpu_session_lock(); g.mutant = mutant; slot_clear_locked(); omega_gpu_session_unlock(); }
const char *omega_gpu_decode_attn_last_error(void) { return omega_gpu_session_last_error(); }
const char *omega_gpu_decode_attn_rc_name(int rc) {
    switch (rc) {
    case OMEGA_GPU_DATTN_OK: return "OK";
    case OMEGA_GPU_DATTN_BAD_ARGS: return "BAD_ARGS";
    case OMEGA_GPU_DATTN_TOO_LARGE: return "TOO_LARGE";
    case OMEGA_GPU_DATTN_CODEGEN_FAIL: return "CODEGEN_FAIL";
    case OMEGA_GPU_DATTN_CHIP_FAIL: return "CHIP_FAIL";
    default: return "?";
    }
}

/* ---- host math ---- */
uint16_t omega_dattn_bf16_rne(float f) {
    uint32_t x; av_memcpy(&x, &f, 4);
    uint32_t r = x + 0x7fffu + ((x >> 16) & 1u);
    return (uint16_t)(r >> 16);
}
float omega_dattn_bf16_f32(uint16_t b) { uint32_t x = (uint32_t)b << 16; float f; av_memcpy(&f, &x, 4); return f; }
void omega_dattn_rope_row(uint32_t pos, double theta, float *row128) {
    av_ropegen(row128, pos, theta, 128, 128, 1);
}
/* ---- cache ---- */
int omega_gpu_decode_attn_cache_alloc(uint32_t layers, uint32_t max_pos, double rope_theta, OmegaGpuDecodeAttnCache **out) {
    if (!out) return OMEGA_GPU_DATTN_BAD_ARGS;
    *out = NULL;
    uint64_t theta;av_memcpy(&theta,&rope_theta,sizeof theta);
    uint64_t p[16]={layers,max_pos,theta};
    int plan_rc=ap_plan(AP_DA,0,p);if(plan_rc)return plan_rc;
    OmegaGpuDecodeAttnCache *c = omega_gpu_host_calloc(1, sizeof *c);
    if (!c) return OMEGA_GPU_DATTN_CHIP_FAIL;
    c->layers = layers; c->max_pos = max_pos; c->theta = rope_theta;
    c->max_splits=p[8];c->plane_bytes=p[9];
    int rc = OMEGA_GPU_DATTN_OK;
    omega_gpu_session_lock();
    if (!dev_open_locked()) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    for (uint32_t L = 0; L < layers && rc == OMEGA_GPU_DATTN_OK; L++) if (omega_gpu_session_alloc((size_t)(2 * c->plane_bytes), &c->kv[L]) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    if (rc == OMEGA_GPU_DATTN_OK && (omega_gpu_session_alloc(p[10], &c->rope) != 0 ||
                                     omega_gpu_session_alloc(p[11], &c->norms) != 0 ||
                                     omega_gpu_session_alloc(p[12], &c->part) != 0 ||
                                     omega_gpu_session_alloc(64, &c->cnt) != 0 ||
                                     omega_gpu_session_alloc((size_t)OMEGA_GPU_DATTN_QKV_WORDS * 4, &c->qkv) != 0 ||
                                     omega_gpu_session_alloc((size_t)OMEGA_GPU_DATTN_Q_HEADS * 128 * 4, &c->out) != 0)) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    if (rc == OMEGA_GPU_DATTN_OK) {
        float *r = c->rope.cpu;
        av_ropegen(r, 0, rope_theta, 128, 128, max_pos);
        av_memset(c->cnt.cpu, 0, 64);
        av_memset(c->norms.cpu, 0, (size_t)layers * 2 * 128 * 4);
        for (uint32_t L = 0; L < layers; L++) av_memset(c->kv[L].cpu, 0, (size_t)(2 * c->plane_bytes));
        if (omega_gpu_session_sync_to_gpu(&c->rope) != 0 || omega_gpu_session_sync_to_gpu(&c->cnt) != 0 || omega_gpu_session_sync_to_gpu(&c->norms) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
        for (uint32_t L = 0; L < layers && rc == OMEGA_GPU_DATTN_OK; L++) if (omega_gpu_session_sync_to_gpu(&c->kv[L]) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    }
    omega_gpu_session_unlock();
    if (rc != OMEGA_GPU_DATTN_OK) { omega_gpu_decode_attn_cache_free(c); return rc; }
    *out = c;
    return OMEGA_GPU_DATTN_OK;
}
void omega_gpu_decode_attn_cache_free(OmegaGpuDecodeAttnCache *c) {
    if (!c) return;
    omega_gpu_session_lock();
    for (uint32_t L = 0; L < c->layers; L++) if (c->kv[L].cpu) omega_gpu_session_free(&c->kv[L]);
    OmegaGpuMem *ms[] = { &c->rope, &c->norms, &c->part, &c->cnt, &c->qkv, &c->out };
    for (unsigned i = 0; i < 6; i++) if (ms[i]->cpu) omega_gpu_session_free(ms[i]);
    omega_gpu_session_unlock();
    omega_gpu_host_free(c);
}
int omega_gpu_decode_attn_set_norms(OmegaGpuDecodeAttnCache *c, uint32_t layer, const float *q_norm, const float *k_norm) {
    if (!c || !q_norm || !k_norm) return OMEGA_GPU_DATTN_BAD_ARGS;
    uint64_t p[16]={layer,c->layers,0,0,1,1};int pr=ap_plan(AP_DA,3,p);if(pr)return pr;
    omega_gpu_session_lock();
    float *n = (float *)c->norms.cpu + (size_t)layer * 256;
    av_memcpy(n, q_norm, 512); av_memcpy(n + 128, k_norm, 512);
    int rc = omega_gpu_session_sync_to_gpu(&c->norms) == 0 ? OMEGA_GPU_DATTN_OK : OMEGA_GPU_DATTN_CHIP_FAIL;
    omega_gpu_session_unlock();
    return rc;
}
static uint16_t *row_ptr(OmegaGpuDecodeAttnCache *c, uint32_t layer, uint32_t kv_head, uint32_t pos, int plane) {
    return (uint16_t *)((uint8_t *)c->kv[layer].cpu + (plane ? c->plane_bytes : 0) + ((size_t)kv_head * c->max_pos + pos) * OMEGA_DATTN_ROW_BYTES);
}
int omega_gpu_decode_attn_cache_write(OmegaGpuDecodeAttnCache *c, uint32_t layer, uint32_t kv_head, uint32_t pos, uint32_t count, const uint16_t *k, const uint16_t *v) {
    if (!c || !k || !v) return OMEGA_GPU_DATTN_BAD_ARGS;
    uint64_t p[16]={layer,c->layers,kv_head,pos,count,c->max_pos};int pr=ap_plan(AP_DA,3,p);if(pr)return pr;
    omega_gpu_session_lock();
    av_memcpy(row_ptr(c, layer, kv_head, pos, 0), k, (size_t)count * 256); av_memcpy(row_ptr(c, layer, kv_head, pos, 1), v, (size_t)count * 256);
    int rc = omega_gpu_session_sync_to_gpu(&c->kv[layer]) == 0 ? OMEGA_GPU_DATTN_OK : OMEGA_GPU_DATTN_CHIP_FAIL;
    omega_gpu_session_unlock();
    return rc;
}
int omega_gpu_decode_attn_cache_read(OmegaGpuDecodeAttnCache *c, uint32_t layer, uint32_t kv_head, uint32_t pos, uint16_t *k, uint16_t *v) {
    if (!c || !k || !v) return OMEGA_GPU_DATTN_BAD_ARGS;
    uint64_t p[16]={layer,c->layers,kv_head,pos,1,c->max_pos};int pr=ap_plan(AP_DA,3,p);if(pr)return pr;
    omega_gpu_session_lock();
    int rc = omega_gpu_session_sync_to_cpu(&c->kv[layer]) == 0 ? OMEGA_GPU_DATTN_OK : OMEGA_GPU_DATTN_CHIP_FAIL;
    if (rc == OMEGA_GPU_DATTN_OK) { av_memcpy(k, row_ptr(c, layer, kv_head, pos, 0), 256); av_memcpy(v, row_ptr(c, layer, kv_head, pos, 1), 256); }
    omega_gpu_session_unlock();
    return rc;
}

/* ---- kernel ---- */
static int kernel_for(const OmegaDecodeAttnSpec *spec, Slot **out, bool *hit) {
    Slot *s = &g.slot;
    if (s->valid && memcmp(&s->spec, spec, sizeof *spec) == 0) { *out = s; *hit = true; return OMEGA_GPU_DATTN_OK; }
    if (g_rec && s->valid) { omega_gpu_session_set_error("decode attn record: kernel cache miss would evict the resident kernel (warm up before recording)"); return OMEGA_GPU_DATTN_TOO_LARGE; }
    slot_clear_locked();
    BlackwellIRProgram *prog = omega_gpu_host_calloc(1, sizeof *prog);
    if (!prog) { omega_gpu_session_set_error("decode attn codegen: host memory"); return OMEGA_GPU_DATTN_CODEGEN_FAIL; }
    int cg = omega_blackwell_codegen_decode_attn_prog(spec, prog);
    size_t need = cg == 0 ? omega_bw_program_code_bytes(prog) : 0;
    if (cg != 0 || need == 0) { omega_gpu_host_free(prog); omega_gpu_session_set_error("decode attn codegen refused the spec"); return OMEGA_GPU_DATTN_CODEGEN_FAIL; }
    uint64_t budget[16]={gpr_declared(prog->regalloc.peak_gpr_usage)};
    if(ap_plan(AP_DA,4,budget)){omega_gpu_host_free(prog);omega_gpu_session_set_error("decode attn: register file");return OMEGA_GPU_DATTN_TOO_LARGE;}
    OmegaGpuMem code;
    if (omega_gpu_session_alloc_code(need, &code) != 0) { omega_gpu_host_free(prog); return OMEGA_GPU_DATTN_CHIP_FAIL; }
    OmegaBlackwellKernel kernel;
    cg = omega_blackwell_kernel_encode_into(prog, code.cpu, need, &kernel);
    omega_gpu_host_free(prog);
    if (cg != 0 || !kernel.code || kernel.code_size != need) { omega_gpu_session_free(&code); omega_gpu_session_set_error("decode attn encode"); return OMEGA_GPU_DATTN_CODEGEN_FAIL; }
    if (omega_gpu_session_sync_to_gpu(&code) != 0) { omega_gpu_session_free(&code); return OMEGA_GPU_DATTN_CHIP_FAIL; }
    av_memset(s, 0, sizeof *s);
    s->valid = true; s->spec = *spec; s->kernel = kernel; s->code = code;
    *out = s; *hit = false;
    return OMEGA_GPU_DATTN_OK;
}

/* rec non-NULL (record path, caller holds the session lock): qkv / out are the device buffers dev_qkv / dev_out, the
 * launch is appended to rec instead of submitted, nothing is staged, poisoned or read back. The split counters are not
 * zeroed by the host here: the kernel's last CTA of each KV head resets its own (header); omega_gpu_decode_attn_counters
 * checks them after the list ran. */
static int dattn_core(OmegaGpuDecodeAttnCache *c, uint32_t layer, uint32_t pos, const float *qkv, float eps, float out_scale,
                      float *out, OmegaGpuDecodeAttnInfo *info, OmegaGpuCmdList *rec, uint64_t dev_qkv, uint64_t dev_out) {
    uint64_t t0 = omega_gpu_session_now_ns();
    if (info) av_memset(info, 0, sizeof *info);
    if (!c || (rec ? (!dev_qkv || !dev_out) : (!qkv || !out))) return OMEGA_GPU_DATTN_BAD_ARGS;
    uint64_t span[16]={layer,c->layers,0,pos,1,c->max_pos};int span_rc=ap_plan(AP_DA,3,span);if(span_rc)return span_rc;
    uint64_t p[16]={pos,g.per_split,c->max_splits,g.depth};
    int plan_rc=ap_plan(AP_DA,2,p);if(plan_rc)return plan_rc;
    const uint32_t per=p[8],splits=p[9];
    OmegaDecodeAttnSpec spec; av_memset(&spec, 0, sizeof spec);
    spec.depth=p[10];spec.mutant=g.mutant;

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
        av_memcpy(c->qkv.cpu, qkv, (size_t)OMEGA_GPU_DATTN_QKV_WORDS * 4);
        av_fillf32(od, 32 * 128, POISON_F32);
        av_memset(c->cnt.cpu, 0, 64);
        if (omega_gpu_session_sync_to_gpu(&c->qkv) != 0 || omega_gpu_session_sync_to_gpu(&c->out) != 0 || omega_gpu_session_sync_to_gpu(&c->cnt) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    }

    uint32_t args[32];
    const uint64_t kc = c->kv[layer].va, vc = kc + c->plane_bytes, nm = c->norms.va + (uint64_t)layer * 1024;
    const uint64_t qkv_va = rec ? dev_qkv : c->qkv.va, out_va = rec ? dev_out : c->out.va;
    float fe = eps, fo = out_scale, fq = (float)(0.08838834764831845 * 1.4426950408889634); /* log2(e) / sqrt(128) */
    uint32_t ue, uo, uq; av_memcpy(&ue, &fe, 4); av_memcpy(&uo, &fo, 4); av_memcpy(&uq, &fq, 4);
    const uint64_t pack[32] = { qkv_va, kc, vc, c->rope.va, nm, nm + 512, out_va, c->part.va, c->cnt.va,
        pos, per, splits, (uint32_t)(c->max_pos * OMEGA_DATTN_ROW_BYTES), ue, uo, uq };
    if (av_args(args, pack, 9, OMEGA_DATTN_ARGS_WORDS)) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    if (info) {
        info->layer = layer; info->pos = pos; info->splits = splits; info->per_split = per; info->depth = spec.depth;
        info->grid_x = splits; info->grid_y = OMEGA_GPU_DATTN_KV_HEADS; info->threads = OMEGA_DATTN_CHUNK;
        info->kv_bytes=p[11];
        info->kernel_cache_hit = hit;
    }
    if (rc == OMEGA_GPU_DATTN_OK) {
        OmegaGpuLaunch L = {
            .code_va = ks->code.va, .gpr_count = gpr_declared(ks->kernel.gpr_count),
            .threads_x = OMEGA_DATTN_CHUNK, .threads_y = 1, .grid_x = splits, .grid_y = OMEGA_GPU_DATTN_KV_HEADS,
            .num_elements = (size_t)32 * 128,
            .args = args, .n_args = OMEGA_DATTN_ARGS_WORDS, .timeout_ms = MARKER_TIMEOUT_MS,
            .shared_bytes = OMEGA_DATTN_SHARED_BYTES,
        };
        if (!omega_bw_gpr_declared_ok(L.gpr_count, ks->kernel.gpr_count)) { omega_gpu_session_set_error("declared registers below peak + 2: refusing to launch"); rc = OMEGA_GPU_DATTN_CHIP_FAIL; }
        else if (rec) { if (omega_gpu_cmdlist_add(rec, &L) < 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL; }
        else if (omega_gpu_session_launch(&L, info ? &info->elapsed_ns : NULL, info ? &info->completion_marker : NULL) != 0) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    }
    if (rc == OMEGA_GPU_DATTN_OK && !rec && (omega_gpu_session_sync_to_cpu(&c->out) != 0 || omega_gpu_session_sync_to_cpu(&c->cnt) != 0)) rc = OMEGA_GPU_DATTN_CHIP_FAIL;
    if (rc == OMEGA_GPU_DATTN_OK && !rec) {
        size_t unchanged = 0;
        unchanged = av_countf32(od, 32 * 128, POISON_F32);
        const uint32_t *cn = c->cnt.cpu;
        if (unchanged) { omega_gpu_session_set_error("decode attn: output poison survived"); rc = OMEGA_GPU_DATTN_CHIP_FAIL; }
        else if (av_any32(cn, 4)) { omega_gpu_session_set_error("decode attn: a split counter did not return to zero"); rc = OMEGA_GPU_DATTN_CHIP_FAIL; }
        else av_memcpy(out, od, (size_t)32 * 128 * 4);
    }
    UNLOCK();
#undef UNLOCK
    if (info) info->call_ns = omega_gpu_session_now_ns() - t0;
    return rc;
}

/* ---- read-only view for the prefill attention (omega_gpu_prefill_attn_api.c) ---- */
int omega_gpu_decode_attn_cache_view(const OmegaGpuDecodeAttnCache *c, uint32_t layer, OmegaGpuDecodeAttnCacheView *v) {
    if (!c || !v) return OMEGA_GPU_DATTN_BAD_ARGS;
    uint64_t p[16]={layer,c->layers,0,0,1,1};int pr=ap_plan(AP_DA,3,p);if(pr)return pr;
    v->layers = c->layers; v->max_pos = c->max_pos;
    v->k_va = c->kv[layer].va; v->v_va = c->kv[layer].va + c->plane_bytes;
    v->rope_va = c->rope.va;
    v->q_norm_va = c->norms.va + (uint64_t)layer * 1024u; v->k_norm_va = v->q_norm_va + 512u;
    return OMEGA_GPU_DATTN_OK;
}

int omega_gpu_decode_attn(OmegaGpuDecodeAttnCache *c, uint32_t layer, uint32_t pos, const float *qkv, float eps, float out_scale, float *out, OmegaGpuDecodeAttnInfo *info) {
    return dattn_core(c, layer, pos, qkv, eps, out_scale, out, info, NULL, 0, 0);
}

/* Record path (OWN-30B W-L2a decode graph): see the header. Caller holds omega_gpu_session_lock(). */
int omega_gpu_decode_attn_record(OmegaGpuCmdList *cl, OmegaGpuDecodeAttnCache *c, uint32_t layer, uint32_t pos, uint64_t qkv_va,
                                 float eps, float out_scale, uint64_t out_va) {
    if (!cl) return OMEGA_GPU_DATTN_BAD_ARGS;
    return dattn_core(c, layer, pos, NULL, eps, out_scale, NULL, NULL, cl, qkv_va, out_va);
}

/* After a list ran (caller holds the lock): the four split counters must be back at zero (a non-zero word means a CTA
 * never arrived). OK or CHIP_FAIL. */
int omega_gpu_decode_attn_counters_check(OmegaGpuDecodeAttnCache *c) {
    if (!c || !c->cnt.cpu) return OMEGA_GPU_DATTN_BAD_ARGS;
    if (omega_gpu_session_sync_to_cpu(&c->cnt) != 0) return OMEGA_GPU_DATTN_CHIP_FAIL;
    const uint32_t *cn = c->cnt.cpu;
    if (cn[0] | cn[1] | cn[2] | cn[3]) { omega_gpu_session_set_error("decode attn: a split counter did not return to zero"); return OMEGA_GPU_DATTN_CHIP_FAIL; }
    return OMEGA_GPU_DATTN_OK;
}
