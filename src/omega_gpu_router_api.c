/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_attn_views.h"
/*
 * omega_gpu_router_api.c: resident router weights, directories, launch buffers and the one-launch router call
 * (see the header). Session use mirrors omega_gpu_decode_attn_api.c: every buffer from omega_gpu_session_alloc
 * (GPU-uncached, host-visible), sync_to_gpu after host writes, sync_to_cpu before host reads, one
 * omega_gpu_session_launch per call.
 */
#include "omega_gpu_router_api.h"
#include "omega_host_policy.h"
#include "omega_api_plan.h"
#include "omega_gpu_session.h"
#include <stdbool.h>
#include <string.h>
#include "omega_gpu_cmdlist.h"
#include "omega_gpu_q36_native.h"

#define MARKER_TIMEOUT_MS 20000ull
#define POISON_F32 (-999.0f)
#define POISON_ID 0xffffffffu

/* the launch buffer: one allocation, fixed offsets (bytes) */
#define LB_X 0u
#define LB_XOUT 8192u
#define LB_IDS 16384u
#define LB_WTS 16448u
#define LB_TAB_GU 16512u
#define LB_TAB_DN 16640u   /* 8 slots; with the shared slot (9) the down table is at LB_TAB_DN9 */
#define LB_BYTES 16768u
#define LB_TAB_DN9 16656u
#define LB_BYTES9 16800u

struct OmegaGpuRouter {
    uint32_t layers;
    OmegaGpuMem gate;  /* [layers][128][2048] bf16 */
    OmegaGpuMem norm;  /* [layers][2048] f32 */
    OmegaGpuMem dir;   /* [layers][2][128][2] u64 */
    OmegaGpuMem lb;    /* launch buffer */
    uint32_t experts, shared;  /* routed experts (128 or 256) and the shared slot (0 or 1) */
    uint32_t rows, slots;      /* gate rows = experts + shared; output slots = 8 + shared */
    uint32_t lb_tab_dn, lb_bytes;
    uint64_t owner_generation;
};
#define ROWB(r) ((size_t)(r)->rows * OMEGA_ROUTER_ROW_BYTES)  /* gate bytes per layer */
#define DIRB(r) ((size_t)(r)->rows * 16u)                     /* one directory per layer */

typedef struct {
    bool valid;
    OmegaRouterSpec spec;
    OmegaBlackwellKernel kernel;
    OmegaGpuMem code;
    uint64_t owner_generation;
} Slot;

static struct {
    bool hooked;
    Slot slot;  /* the plain kernel (grid 1 x 1) */
    Slot rows;  /* the ROWS form (lever 2: one launch per layer for every conversation); omega_gpu_router_prepare_rows */
    uint32_t warps, depth;
    int mutant;
} g;
static int g_rec; /* 1 while a record call runs (lock held): kernel_for must not evict */
/* Persistent across device close and object recreation; zero is refusal. */
static uint64_t router_owner_counter;
static uint64_t router_owner_next(void) {
    uint64_t next = 0;
    if (omega_native_revision(&next, router_owner_counter) != OMEGA_OK || !next) return 0;
    router_owner_counter = next;
    return next;
}

static void slot_free(Slot *s) { if (s->valid) omega_gpu_session_free(&s->code); av_memset(s, 0, sizeof *s); }
static void slot_clear_locked(void) { slot_free(&g.slot); slot_free(&g.rows); }
static void on_session_close(void) { av_memset(&g.slot, 0, sizeof g.slot); av_memset(&g.rows, 0, sizeof g.rows); }
static OmegaGpuSession *dev_open_locked(void) {
    if (!g.hooked) { (void)omega_gpu_session_on_close(on_session_close); g.hooked = true; }
    return omega_gpu_session_open();
}
static uint32_t gpr_declared(uint32_t peak) { return omega_bw_gpr_declared(peak); } /* peak + 2 rounded up to 8: the chip hung at peak + 1 (omega_blackwell_codegen.h) */

void omega_gpu_router_set_shape(uint32_t warps, uint32_t depth) { omega_gpu_session_lock(); g.warps = warps; g.depth = depth; slot_clear_locked(); omega_gpu_session_unlock(); }
void omega_gpu_router_test_set_mutant(int mutant) { omega_gpu_session_lock(); g.mutant = mutant; slot_clear_locked(); omega_gpu_session_unlock(); }
const char *omega_gpu_router_last_error(void) { return omega_gpu_session_last_error(); }
const char *omega_gpu_router_rc_name(int rc) {
    switch (rc) {
    case OMEGA_GPU_ROUTER_OK: return "OK";
    case OMEGA_GPU_ROUTER_BAD_ARGS: return "BAD_ARGS";
    case OMEGA_GPU_ROUTER_TOO_LARGE: return "TOO_LARGE";
    case OMEGA_GPU_ROUTER_CODEGEN_FAIL: return "CODEGEN_FAIL";
    case OMEGA_GPU_ROUTER_CHIP_FAIL: return "CHIP_FAIL";
    default: return "?";
    }
}

/* ---- buffers ---- */
int omega_gpu_router_alloc(uint32_t layers, OmegaGpuRouter **out) { return omega_gpu_router_alloc_shape(layers, 0, 0, out); }
int omega_gpu_router_alloc_shape(uint32_t layers, uint32_t experts, uint32_t shared, OmegaGpuRouter **out) {
    if (!out) return OMEGA_GPU_ROUTER_BAD_ARGS;
    uint64_t p[16]={layers,experts,shared};int plan_rc=ap_plan(AP_RT,0,p);
    if(p[7])*out=NULL;
    if(plan_rc)return plan_rc;
    experts=p[8];
    OmegaGpuRouter *r = omega_gpu_host_calloc(1, sizeof *r);
    if (!r) return OMEGA_GPU_ROUTER_BAD_ARGS;
    r->layers=layers;r->experts=experts;r->shared=shared;r->rows=p[9];r->slots=p[10];
    r->lb_tab_dn=p[11];r->lb_bytes=p[12];
    omega_gpu_session_lock();
    int rc = OMEGA_GPU_ROUTER_OK;
    if (!dev_open_locked()) rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (rc == OMEGA_GPU_ROUTER_OK && omega_gpu_session_alloc(p[13], &r->gate) != 0) rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (rc == OMEGA_GPU_ROUTER_OK && omega_gpu_session_alloc(p[14], &r->norm) != 0) rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (rc == OMEGA_GPU_ROUTER_OK && omega_gpu_session_alloc(p[15], &r->dir) != 0) rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (rc == OMEGA_GPU_ROUTER_OK && omega_gpu_session_alloc(r->lb_bytes, &r->lb) != 0) rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (rc == OMEGA_GPU_ROUTER_OK) {
        av_memset(r->dir.cpu, 0, (size_t)layers * 2u * DIRB(r));
        av_memset(r->lb.cpu, 0, r->lb_bytes);
        if (omega_gpu_session_sync_to_gpu(&r->dir) != 0 || omega_gpu_session_sync_to_gpu(&r->lb) != 0) rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    }
    if (rc == OMEGA_GPU_ROUTER_OK && !(r->owner_generation = router_owner_next()))
        rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    omega_gpu_session_unlock();
    if (rc != OMEGA_GPU_ROUTER_OK) { omega_gpu_router_free(r); return rc; }
    *out = r;
    return OMEGA_GPU_ROUTER_OK;
}

void omega_gpu_router_free(OmegaGpuRouter *r) {
    if (!r) return;
    omega_gpu_session_lock();
    if (r->gate.cpu) omega_gpu_session_free(&r->gate);
    if (r->norm.cpu) omega_gpu_session_free(&r->norm);
    if (r->dir.cpu) omega_gpu_session_free(&r->dir);
    if (r->lb.cpu) omega_gpu_session_free(&r->lb);
    omega_gpu_session_unlock();
    omega_gpu_host_free(r);
}

int omega_gpu_router_set_layer(OmegaGpuRouter *r, uint32_t layer, const uint16_t *gate_bf16, const float *norm_w) {
    if (!r || !gate_bf16 || !norm_w) return OMEGA_GPU_ROUTER_BAD_ARGS;
    uint64_t p[16]={layer,r->layers};int pr=ap_plan(AP_RT,3,p);if(pr)return pr;
    omega_gpu_session_lock();
    r->owner_generation = 0;
    av_memcpy((uint8_t *)r->gate.cpu + (size_t)layer * ROWB(r), gate_bf16, ROWB(r));
    av_memcpy((uint8_t *)r->norm.cpu + (size_t)layer * OMEGA_ROUTER_HIDDEN * 4u, norm_w, (size_t)OMEGA_ROUTER_HIDDEN * 4u);
    int rc = (omega_gpu_session_sync_to_gpu(&r->gate) == 0 && omega_gpu_session_sync_to_gpu(&r->norm) == 0) ? OMEGA_GPU_ROUTER_OK : OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (rc == OMEGA_GPU_ROUTER_OK && !(r->owner_generation = router_owner_next()))
        rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    omega_gpu_session_unlock();
    return rc;
}

int omega_gpu_router_set_directory(OmegaGpuRouter *r, uint32_t layer, const uint64_t *gu, const uint64_t *dn) {
    if (!r || !gu || !dn) return OMEGA_GPU_ROUTER_BAD_ARGS;
    uint64_t p[16]={layer,r->layers};int pr=ap_plan(AP_RT,3,p);if(pr)return pr;
    omega_gpu_session_lock();
    r->owner_generation = 0;
    uint8_t *d = (uint8_t *)r->dir.cpu + (size_t)layer * 2u * DIRB(r);
    av_memcpy(d, gu, DIRB(r));
    av_memcpy(d + DIRB(r), dn, DIRB(r));
    int rc = omega_gpu_session_sync_to_gpu(&r->dir) == 0 ? OMEGA_GPU_ROUTER_OK : OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (rc == OMEGA_GPU_ROUTER_OK && !(r->owner_generation = router_owner_next()))
        rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    omega_gpu_session_unlock();
    return rc;
}

int omega_gpu_router_set_directory_fp8(OmegaGpuRouter *r, uint32_t layer, const OmegaGpuFp8Tensor *gate_up, const OmegaGpuFp8Tensor *down) {
    if (!r || !gate_up || !down) return OMEGA_GPU_ROUTER_BAD_ARGS;
    uint64_t gu[(256u + 1u) * 2], dn[(256u + 1u) * 2];
    for (uint32_t x = 0; x < r->rows; x++) {
        if (omega_gpu_fp8_tensor_entry(gate_up, x, &gu[2 * x], &gu[2 * x + 1]) != 0) return OMEGA_GPU_ROUTER_BAD_ARGS;
        if (omega_gpu_fp8_tensor_entry(down, x, &dn[2 * x], &dn[2 * x + 1]) != 0) return OMEGA_GPU_ROUTER_BAD_ARGS;
    }
    return omega_gpu_router_set_directory(r, layer, gu, dn);
}

void omega_gpu_router_device_outputs(const OmegaGpuRouter *r, uint64_t *xout_va, uint64_t *tab_gu_va, uint64_t *tab_dn_va) {
    if (xout_va) *xout_va = r ? r->lb.va + LB_XOUT : 0;
    if (tab_gu_va) *tab_gu_va = r ? r->lb.va + LB_TAB_GU : 0;
    if (tab_dn_va) *tab_dn_va = r ? r->lb.va + r->lb_tab_dn : 0;
}

int omega_gpu_router_read_back(OmegaGpuRouter *r, float *xout, uint64_t *tab_gu, uint64_t *tab_dn) {
    if (!r) return OMEGA_GPU_ROUTER_BAD_ARGS;
    omega_gpu_session_lock();
    int rc = omega_gpu_session_sync_to_cpu(&r->lb) == 0 ? OMEGA_GPU_ROUTER_OK : OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (rc == OMEGA_GPU_ROUTER_OK) {
        const uint8_t *b = r->lb.cpu;
        if (xout) av_memcpy(xout, b + LB_XOUT, OMEGA_ROUTER_HIDDEN * 4u);
        if (tab_gu) av_memcpy(tab_gu, b + LB_TAB_GU, (size_t)r->slots * 16u);
        if (tab_dn) av_memcpy(tab_dn, b + r->lb_tab_dn, (size_t)r->slots * 16u);
    }
    omega_gpu_session_unlock();
    return rc;
}

/* ---- kernel ---- */
static int kernel_for(const OmegaRouterSpec *spec, Slot **out, bool *hit) {
    Slot *s = spec->xstride ? &g.rows : &g.slot;
    if (s->valid && memcmp(&s->spec, spec, sizeof *spec) == 0) { *out = s; *hit = true; return OMEGA_GPU_ROUTER_OK; }
    if (g_rec && s->valid) { omega_gpu_session_set_error("router record: kernel cache miss would evict the resident kernel (warm up before recording)"); return OMEGA_GPU_ROUTER_TOO_LARGE; }
    slot_free(s);
    BlackwellIRProgram *prog = omega_gpu_host_calloc(1, sizeof *prog);
    if (!prog) { omega_gpu_session_set_error("router codegen: host memory"); return OMEGA_GPU_ROUTER_CODEGEN_FAIL; }
    int cg = omega_blackwell_codegen_router_prog(spec, prog);
    size_t need = cg == 0 ? omega_bw_program_code_bytes(prog) : 0;
    if (cg != 0 || need == 0) { omega_gpu_host_free(prog); omega_gpu_session_set_error("router codegen refused the spec"); return OMEGA_GPU_ROUTER_CODEGEN_FAIL; }
    uint64_t budget[16]={spec->warps,gpr_declared(prog->regalloc.peak_gpr_usage)};
    if(ap_plan(AP_RT,4,budget)){omega_gpu_host_free(prog);omega_gpu_session_set_error("router: register file");return OMEGA_GPU_ROUTER_TOO_LARGE;}
    OmegaGpuMem code;
    if (omega_gpu_session_alloc_code(need, &code) != 0) { omega_gpu_host_free(prog); return OMEGA_GPU_ROUTER_CHIP_FAIL; }
    OmegaBlackwellKernel kernel;
    cg = omega_blackwell_kernel_encode_into(prog, code.cpu, need, &kernel);
    omega_gpu_host_free(prog);
    if (cg != 0 || !kernel.code || kernel.code_size != need) { omega_gpu_session_free(&code); omega_gpu_session_set_error("router encode"); return OMEGA_GPU_ROUTER_CODEGEN_FAIL; }
    if (omega_gpu_session_sync_to_gpu(&code) != 0) { omega_gpu_session_free(&code); return OMEGA_GPU_ROUTER_CHIP_FAIL; }
    uint64_t generation = router_owner_next();
    if (!generation) { omega_gpu_session_free(&code); return OMEGA_GPU_ROUTER_CHIP_FAIL; }
    av_memset(s, 0, sizeof *s);
    s->valid = true; s->spec = *spec; s->kernel = kernel; s->code = code;
    s->owner_generation = generation;
    *out = s; *hit = false;
    return OMEGA_GPU_ROUTER_OK;
}

static void spec_for(const OmegaGpuRouter *r, OmegaRouterSpec *spec, uint32_t xstride, uint32_t lbstride) {
    av_memset(spec, 0, sizeof *spec);
    uint64_t p[16]={g.warps,g.depth,r->experts};(void)ap_plan(AP_RT,1,p);
    spec->warps=p[8];spec->depth=p[9];spec->mutant=g.mutant;
    spec->experts=p[10];spec->shared=r->shared;
    spec->xstride = xstride; spec->lbstride = lbstride;
}

/* rec non-NULL (record path, caller holds the session lock): x is the device row dev_x, the launch is appended to rec
 * instead of submitted, nothing is staged, poisoned or read back (outputs stay at the launch-buffer addresses).
 * rows >= 2 (record path only, lever 2): the ROWS kernel, grid_y rows, row i at dev_x + i * xstride and rec_lb + i * lbstride. */
static int router_core(OmegaGpuRouter *r, uint32_t layer, const float *x, float eps, int norm_topk, float out_scale,
                       uint32_t *ids, float *weights, OmegaGpuRouterInfo *info, OmegaGpuCmdList *rec, uint64_t dev_x, uint64_t rec_lb,
                       uint32_t xstride, uint32_t lbstride, uint32_t rows) {
    uint64_t t0 = omega_gpu_session_now_ns();
    if (info) av_memset(info, 0, sizeof *info);
    if (!r || (rec ? !dev_x : (!x || !ids || !weights))) return OMEGA_GPU_ROUTER_BAD_ARGS;
    uint64_t p[16]={layer,r->layers,rows,rec!=NULL,rec_lb!=0,xstride,lbstride};
    int plan_rc=ap_plan(AP_RT,3,p);if(plan_rc)return plan_rc;
    OmegaRouterSpec spec; spec_for(r,&spec,p[8],p[9]);
    OmegaRouterSmem sm;
    if (omega_router_smem(&spec, &sm) != 0) return OMEGA_GPU_ROUTER_BAD_ARGS;

    if (!rec) omega_gpu_session_lock();
#define UNLOCK() do { if (!rec) omega_gpu_session_unlock(); } while (0)
    int rc = OMEGA_GPU_ROUTER_OK;
    if (!dev_open_locked()) { UNLOCK(); return OMEGA_GPU_ROUTER_CHIP_FAIL; }
    Slot *ks = NULL; bool hit = false;
    g_rec = rec != NULL;
    rc = kernel_for(&spec, &ks, &hit);
    g_rec = 0;
    if (rc != OMEGA_GPU_ROUTER_OK) { UNLOCK(); return rc; }
    uint8_t *b = r->lb.cpu;
    float *xo = (float *)(b + LB_XOUT), *wd = (float *)(b + LB_WTS);
    uint32_t *idd = (uint32_t *)(b + LB_IDS);
    if (!rec) {
        av_memcpy(b + LB_X, x, OMEGA_ROUTER_HIDDEN * 4u);
        av_fillf32(xo, OMEGA_ROUTER_HIDDEN, POISON_F32);
        av_fillf32(wd, r->slots, POISON_F32); av_fill32(idd, r->slots, POISON_ID);
        av_memset(b + LB_TAB_GU, 0, (size_t)r->slots * 16u); av_memset(b + r->lb_tab_dn, 0, (size_t)r->slots * 16u);
        if (omega_gpu_session_sync_to_gpu(&r->lb) != 0) rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    }

    uint32_t args[32];
    const uint64_t gate = r->gate.va + (uint64_t)layer * ROWB(r);
    const uint64_t norm = r->norm.va + (uint64_t)layer * OMEGA_ROUTER_HIDDEN * 4u;
    const uint64_t dgu = r->dir.va + (uint64_t)layer * 2u * DIRB(r), ddn = dgu + DIRB(r);
    float fe = eps, fo = out_scale; uint32_t ue, uo; av_memcpy(&ue, &fe, 4); av_memcpy(&uo, &fo, 4);
    const uint64_t lb = rec_lb ? rec_lb : r->lb.va; /* caller-owned outputs for record_to */
    const uint64_t pack[32] = { rec ? dev_x : r->lb.va + LB_X, gate, norm, lb + LB_XOUT,
        lb + LB_IDS, lb + LB_WTS, dgu, ddn, lb + LB_TAB_GU, lb + r->lb_tab_dn,
        ue, uo, norm_topk ? 1u : 0u };
    if (av_args(args, pack, 10, OMEGA_ROUTER_ARGS_WORDS)) rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (info) {
        info->layer = layer; info->warps = spec.warps; info->depth = spec.depth; info->threads = 32u * spec.warps;
        info->weight_bytes = ROWB(r);
        info->kernel_cache_hit = hit;
    }
    if (rc == OMEGA_GPU_ROUTER_OK) {
        OmegaGpuLaunch L = {
            .code_va = ks->code.va, .gpr_count = gpr_declared(ks->kernel.gpr_count),
            .threads_x = 32u * spec.warps, .threads_y = 1, .grid_x = 1, .grid_y = p[10],
            .num_elements = (size_t)OMEGA_ROUTER_HIDDEN * p[10],
            .args = args, .n_args = OMEGA_ROUTER_ARGS_WORDS, .timeout_ms = MARKER_TIMEOUT_MS,
            .shared_bytes = sm.total,
        };
        if (!omega_bw_gpr_declared_ok(L.gpr_count, ks->kernel.gpr_count)) { omega_gpu_session_set_error("declared registers below peak + 2: refusing to launch"); rc = OMEGA_GPU_ROUTER_CHIP_FAIL; }
        else if (rec) { if (omega_gpu_cmdlist_add(rec, &L) < 0) rc = OMEGA_GPU_ROUTER_CHIP_FAIL; }
        else if (omega_gpu_session_launch(&L, info ? &info->elapsed_ns : NULL, info ? &info->completion_marker : NULL) != 0) rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    }
    if (rc == OMEGA_GPU_ROUTER_OK && !rec && omega_gpu_session_sync_to_cpu(&r->lb) != 0) rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (rc == OMEGA_GPU_ROUTER_OK && !rec) {
        size_t unchanged = 0;
        unchanged = av_countf32(xo, OMEGA_ROUTER_HIDDEN, POISON_F32);
        unchanged += av_routerpoison(wd, idd, r->slots, POISON_F32, POISON_ID);
        const uint64_t *tg = (const uint64_t *)(b + LB_TAB_GU), *td = (const uint64_t *)(b + r->lb_tab_dn);
        size_t tab_zero = 0;
        tab_zero = av_zero64(tg, 2u * r->slots) + av_zero64(td, 2u * r->slots);
        size_t bad_id = 0;
        bad_id = av_badids(idd, OMEGA_ROUTER_TOPK, r->experts);
        if (r->shared && idd[OMEGA_ROUTER_TOPK] != r->experts) bad_id++;
        if (unchanged) { omega_gpu_session_set_error("router: output poison survived"); rc = OMEGA_GPU_ROUTER_CHIP_FAIL; }
        else if (bad_id) { omega_gpu_session_set_error("router: an expert id is out of range"); rc = OMEGA_GPU_ROUTER_CHIP_FAIL; }
        else if (tab_zero) { omega_gpu_session_set_error("router: a group table word stayed zero (directory not set?)"); rc = OMEGA_GPU_ROUTER_CHIP_FAIL; }
        else { av_memcpy(ids, idd, (size_t)r->slots * 4u); av_memcpy(weights, wd, (size_t)r->slots * 4u); }
    }
    UNLOCK();
#undef UNLOCK
    if (info) info->call_ns = omega_gpu_session_now_ns() - t0;
    return rc;
}

int omega_gpu_router(OmegaGpuRouter *r, uint32_t layer, const float *x, float eps, int norm_topk, float out_scale,
                     uint32_t *ids, float *weights, OmegaGpuRouterInfo *info) {
    return router_core(r, layer, x, eps, norm_topk, out_scale, ids, weights, info, NULL, 0, 0, 0, 0, 0);
}

/* Record path (OWN-30B W-L2a decode graph): see the header. Caller holds omega_gpu_session_lock(). */
int omega_gpu_router_record(OmegaGpuCmdList *cl, OmegaGpuRouter *r, uint32_t layer, uint64_t x_va, float eps, int norm_topk,
                            float out_scale, OmegaGpuRouterDevOut *out) {
    if (!cl) return OMEGA_GPU_ROUTER_BAD_ARGS;
    int rc = router_core(r, layer, NULL, eps, norm_topk, out_scale, NULL, NULL, NULL, cl, x_va, 0, 0, 0, 0);
    if (rc == OMEGA_GPU_ROUTER_OK && out) {
        out->xout_va = r->lb.va + LB_XOUT; out->ids_va = r->lb.va + LB_IDS; out->wts_va = r->lb.va + LB_WTS;
        out->tab_gu_va = r->lb.va + LB_TAB_GU; out->tab_dn_va = r->lb.va + r->lb_tab_dn;
    }
    return rc;
}
/* Record path with the outputs in the caller's buffer (L312 v2, the stroke: one buffer per conversation, the layout of
 * omega_gpu_router_layout_bytes). Caller holds omega_gpu_session_lock(). */
int omega_gpu_router_record_to(OmegaGpuCmdList *cl, OmegaGpuRouter *r, uint32_t layer, uint64_t x_va, float eps, int norm_topk,
                               float out_scale, uint64_t lb_va, OmegaGpuRouterDevOut *out) {
    if (!cl || !r || !lb_va || (lb_va & 255u)) return OMEGA_GPU_ROUTER_BAD_ARGS;
    int rc = router_core(r, layer, NULL, eps, norm_topk, out_scale, NULL, NULL, NULL, cl, x_va, lb_va, 0, 0, 0);
    if (rc == OMEGA_GPU_ROUTER_OK && out) {
        out->xout_va = lb_va + LB_XOUT; out->ids_va = lb_va + LB_IDS; out->wts_va = lb_va + LB_WTS;
        out->tab_gu_va = lb_va + LB_TAB_GU; out->tab_dn_va = lb_va + r->lb_tab_dn;
    }
    return rc;
}
/* Lever 2 (L312): one launch for `rows` conversations of one layer (the ROWS form of the kernel, grid_y rows). Caller holds
 * omega_gpu_session_lock(). rows 1 is record_to. The rows kernel must be resident (omega_gpu_router_prepare_rows). */
int omega_gpu_router_record_rows(OmegaGpuCmdList *cl, OmegaGpuRouter *r, uint32_t layer, uint64_t x_va, uint32_t xstride, float eps,
                                 int norm_topk, float out_scale, uint64_t lb_va, uint32_t lbstride, uint32_t rows, OmegaGpuRouterDevOut *out) {
    if (!cl || !r || !lb_va || (lb_va & 255u)) return OMEGA_GPU_ROUTER_BAD_ARGS;
    uint64_t p[16]={rows,xstride,lbstride,r->lb_bytes};
    int plan_rc=ap_plan(AP_RT,2,p);if(plan_rc)return plan_rc;
    if (p[8]) return omega_gpu_router_record_to(cl, r, layer, x_va, eps, norm_topk, out_scale, lb_va, out);
    int rc = router_core(r, layer, NULL, eps, norm_topk, out_scale, NULL, NULL, NULL, cl, x_va, lb_va, xstride, lbstride, rows);
    if (rc == OMEGA_GPU_ROUTER_OK && out) {
        out->xout_va = lb_va + LB_XOUT; out->ids_va = lb_va + LB_IDS; out->wts_va = lb_va + LB_WTS;
        out->tab_gu_va = lb_va + LB_TAB_GU; out->tab_dn_va = lb_va + r->lb_tab_dn;
    }
    return rc;
}
/* Builds (or finds) the ROWS kernel for these strides and keeps it resident beside the plain one (its own cache slot). */
int omega_gpu_router_prepare_rows(OmegaGpuRouter *r, uint32_t xstride, uint32_t lbstride) {
    if (!r) return OMEGA_GPU_ROUTER_BAD_ARGS;
    uint64_t p[16]={2,xstride,lbstride,r->lb_bytes};int plan_rc=ap_plan(AP_RT,2,p);if(plan_rc)return plan_rc;
    OmegaRouterSpec spec; spec_for(r, &spec, xstride, lbstride);
    omega_gpu_session_lock();
    int rc = OMEGA_GPU_ROUTER_CHIP_FAIL;
    if (dev_open_locked()) { Slot *ks = NULL; bool hit = false; g_rec = 0; rc = kernel_for(&spec, &ks, &hit); }
    omega_gpu_session_unlock();
    return rc;
}
uint32_t omega_gpu_router_layout_bytes(const OmegaGpuRouter *r) { return r ? r->lb_bytes : 0u; }

int omega_gpu_q36_router_observe(const OmegaGpuRouter *r, uint32_t layer,
    uint64_t recorded_code_va, OmegaGpuQ36RouterObservation *out) {
    const Slot *s = NULL;
    if (!out) return -1;
    av_memset(out, 0, sizeof *out);
    if (!r || !r->dir.cpu) return -1;
    uint64_t p[16]={layer,r->layers};if(ap_plan(AP_RT,3,p))return -1;
    if (g.slot.valid && g.slot.code.va == recorded_code_va) s = &g.slot;
    if (g.rows.valid && g.rows.code.va == recorded_code_va) {
        if (s) return -1;
        s = &g.rows;
    }
    if (!s || !s->code.cpu || !s->kernel.code_size ||
        s->kernel.code_size > s->code.size) return -1;
    out->code = s->code; out->code_bytes = s->kernel.code_size;
    out->owner_generation = r->owner_generation;
    out->code_generation = s->owner_generation;
    out->gate = r->gate; out->norm = r->norm;
    out->directory = r->dir; out->launch_buffer = r->lb;
    out->entries = r->rows;
    out->gu_directory = (const uint64_t *)((const uint8_t *)r->dir.cpu +
        (size_t)layer * 2u * DIRB(r));
    out->dn_directory = out->gu_directory + (size_t)r->rows * 2u;
    return 0;
}
