/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_scan.c -- see omega_gpu_scan.h. The kernels and launch plans come from seeded Omega (om/scan/scan_ir.om,
 * om/scan/scan_plan.om through their generated units); this file replays them, runs a plan through a launch callback,
 * and stages, launches and checks the calls on the chip.
 *
 * MIGRATION-PENDING C: the earlier C generator (gen) and level drivers of kinds 0..4 and ops 0..3 stay below as the
 * differential oracle, compiled only with -DCUDAPP_MIGRATION_PENDING_C_SCAN (C primary for those kinds and ops) or
 * -DCUDAPP_MIGRATION_PENDING_SCAN_DIFF (both run, any difference aborts; -DCUDAPP_SCAN_DIFF_SELFTEST flips one Omega
 * word first and must abort). They are deleted in a later PR. */
#include "omega_gpu_scan.h"
#include "omega_gpu_om_ir.h"
#include "omega_gpu_session.h"
#include "cudapp_native.h"
#include "omega_attn_host_policy.h"

#include <stdlib.h>
#include <string.h>

#if defined(CUDAPP_MIGRATION_PENDING_C_SCAN) && defined(CUDAPP_MIGRATION_PENDING_SCAN_DIFF)
#error "CUDAPP_MIGRATION_PENDING_C_SCAN and CUDAPP_MIGRATION_PENDING_SCAN_DIFF are exclusive"
#endif
#if defined(CUDAPP_SCAN_DIFF_SELFTEST) && !defined(CUDAPP_MIGRATION_PENDING_SCAN_DIFF)
#error "CUDAPP_SCAN_DIFF_SELFTEST needs CUDAPP_MIGRATION_PENDING_SCAN_DIFF"
#endif
#if defined(CUDAPP_MIGRATION_PENDING_C_SCAN) || defined(CUDAPP_MIGRATION_PENDING_SCAN_DIFF)
#define SCAN_C_ORACLE 1
#include <stdio.h>
#include "omega_bw_em.h"
#endif

#define POISON 0x7fa00badu     /* a signalling NaN: no FADD or FMNMX writes it, so a word still holding it was never written */
#define TIMEOUT_MS 20000ull

/* ------------------------------------------------------------------ Omega: kernels and plans */

static int om_codegen(const OmegaScanSpec *sp, BlackwellIRProgram *prog) {
    if (sp->mutant < 0) return -1;
    uint64_t err = 0;
    return omega_gpu_om_ir_replay(omega_scan_ir, (uint64_t)sp->kind, (uint64_t)sp->mutant, 0, 0, prog, &err) == OMEGA_OM_IR_OK ? 0 : -1;
}

/* 0, or the unit's refusal mapped to BAD_ARGS (1) / TOO_LARGE (2) */
static int om_plan(OmegaScanOp op, size_t n, uint64_t in, uint64_t out, uint64_t scr, uint64_t *plan) {
    uint64_t r = 0;
    if (omega_scan_plan(&r, (uint64_t)op, (uint64_t)n, in, out, scr, plan, OMEGA_GPU_SCAN_PLAN_WORDS) != OMEGA_OK)
        return OMEGA_GPU_SCAN_BAD_ARGS;
    if (r == 0) return OMEGA_GPU_SCAN_OK;
    return r == 2 ? OMEGA_GPU_SCAN_TOO_LARGE : OMEGA_GPU_SCAN_BAD_ARGS;
}

#ifdef SCAN_C_ORACLE
/* ------------------------------------------------------------------ MIGRATION-PENDING C oracle: kinds 0..4 */

#define PAD_SUM 0x80000000u    /* -0.0: x + -0.0 == x for every x */
#define PAD_MINMAX 0x7fc00000u /* quiet NaN: max(NaN, x) == min(NaN, x) == x */

static void fmnmx(Em *e, BlackwellIROpcode op, int d, int a, int b) {
    BlackwellIRInsn i = I0(op); i.dst_vreg = d; i.src1_vreg = a; i.src2_vreg = b; emit(e, i, K_FIXED);
}
typedef struct { int bssy, bra; } Region;
/* the lanes with P0 set skip to the region's end; the warp joins again there */
static Region skip_if_p0(Em *e) { Region r; r.bssy = bssy_fwd(e); r.bra = bra_if_p0_fwd(e); return r; }
static void region_end(Em *e, Region r) { int at = here(e); bsync(e); patch_bra(e, r.bra, at); patch_bra(e, r.bssy, at + 1); }

static int gen(Em *e, const OmegaScanSpec *sp) {
    const OmegaScanKind k = sp->kind;
    const int red = k <= OMEGA_SCAN_K_RED_MIN;
    int udesc = UV64(e), pin = V64(e), pout = V64(e), ptot = V64(e), len = V(e), tiles = V(e), warps = V(e);
    int tid = V(e), cta = V(e), lane = V(e), gw = V(e), one = V(e), tile = V(e), el = V(e), v = V(e), t = V(e);
    int a64 = V64(e), b64 = V64(e), rl = V(e), nz = V(e), c31 = V(e), m1 = V(e), off = V(e), tm1 = V(e);
    int mask[5];
    for (unsigned s = 0; s < 5; s++) mask[s] = k == OMEGA_SCAN_K_TILE ? V(e) : NOREG;
    if (e->err) return -1;
    ldcu64(e, udesc, 0x358);
    ldc64(e, pin, OMEGA_GPU_SCAN_A_IN); ldc64(e, pout, OMEGA_GPU_SCAN_A_OUT);
    if (k == OMEGA_SCAN_K_TILE) ldc64(e, ptot, OMEGA_GPU_SCAN_A_TOT);
    ldc32(e, len, OMEGA_GPU_SCAN_A_LEN); ldc32(e, tiles, OMEGA_GPU_SCAN_A_TILES); ldc32(e, warps, OMEGA_GPU_SCAN_A_WARPS);
    s2r(e, tid, BW_SR_TID_X); s2r(e, cta, BW_SR_CTAID_X);
    movi(e, one, 1); movi(e, m1, 0xffffffffu); movi(e, nz, PAD_SUM);
    andi(e, lane, tid, 0, 31u);
    shr(e, t, tid, 0, 5);
    imadi(e, gw, cta, 0, OMEGA_GPU_SCAN_THREADS / 32u, t);      /* global warp = cta * 8 + warp */
    if (k == OMEGA_SCAN_K_ADD) imadi(e, tile, gw, 0, 1u, one);  /* the carry starts at tile 1 */
    else imadi(e, tile, gw, 0, 1u, NOREG);
    if (k == OMEGA_SCAN_K_TILE) {
        /* reversed lanes: lane L holds tile position 31 - L, so SHFL.DOWN by d reads position - d */
        movi(e, c31, 31u);
        lop3(e, rl, lane, c31, NOREG, 0x3cu);                    /* 31 - L == L ^ 31 for L < 32 */
        /* mask[s] = all ones when position - d exists (L + d <= 31), else 0: ((L + d) >> 5) - 1 */
        for (unsigned s = 0; s < 5; s++) {
            movi(e, t, 1u << s);
            iadd(e, mask[s], lane, t);
            shr(e, mask[s], mask[s], 0, 5);
            imadi(e, mask[s], mask[s], 0, 1u, m1);
        }
    }

    const int top = here(e);
    loop_begin(e);
    isetp_ge_u32(e, tile, tiles);
    const int done = bra_if_p0_fwd(e);
    if (red) {
        const BlackwellIROpcode op = k == OMEGA_SCAN_K_RED_MAX ? BW_IR_FMNMX_MAX : BW_IR_FMNMX_MIN;
        imadi(e, el, tile, 0, 32u, lane);
        movi(e, v, k == OMEGA_SCAN_K_RED_SUM ? PAD_SUM : PAD_MINMAX);
        isetp_ge_u32(e, el, len);
        { Region r = skip_if_p0(e); imadw(e, a64, el, 4u, pin); ldg32(e, v, a64, udesc, 1); region_end(e, r); }
        for (uint32_t d = 16; d >= 1; d /= 2) {
            if (sp->mutant == 1 && d == 1) break;
            shfl_down(e, t, v, d);
            if (k == OMEGA_SCAN_K_RED_SUM) fadd(e, v, v, t); else fmnmx(e, op, v, v, t);
        }
        isetp_ge_u32(e, lane, one);
        { Region r = skip_if_p0(e); imadw(e, a64, tile, 4u, pout); stg(e, a64, 0, v, udesc); region_end(e, r); }
    } else if (k == OMEGA_SCAN_K_TILE) {
        imadi(e, el, tile, 0, 32u, rl);
        movi(e, v, PAD_SUM);
        isetp_ge_u32(e, el, len);
        { Region r = skip_if_p0(e); imadw(e, a64, el, 4u, pin); ldg32(e, v, a64, udesc, 1); region_end(e, r); }
        for (unsigned s = 0; s < 5; s++) {
            if (sp->mutant == 2 && s == 4) break;
            shfl_down(e, t, v, 1u << s);
            lop3(e, t, t, mask[s], nz, 0xe2u);                   /* (t & mask) | (-0.0 & ~mask) */
            fadd(e, v, t, v);                                    /* lower position on the left */
        }
        isetp_ge_u32(e, el, len);
        { Region r = skip_if_p0(e); imadw(e, a64, el, 4u, pout); stg(e, a64, 0, v, udesc); region_end(e, r); }
        isetp_ge_u32(e, lane, one);                               /* lane 0 holds position 31: the tile total */
        { Region r = skip_if_p0(e); imadw(e, a64, tile, 4u, ptot); stg(e, a64, 0, v, udesc); region_end(e, r); }
    } else {
        imadi(e, el, tile, 0, 32u, lane);
        isetp_ge_u32(e, el, len);
        {
            Region r = skip_if_p0(e);
            if (sp->mutant == 3) imadi(e, tm1, tile, 0, 1u, NOREG); else imadi(e, tm1, tile, 0, 1u, m1);
            imadw(e, b64, tm1, 4u, pout); imadw(e, a64, el, 4u, pin);
            ldg32(e, off, b64, udesc, 1); ldg32(e, v, a64, udesc, 0);
            fadd(e, v, off, v);
            stg(e, a64, 0, v, udesc);
            region_end(e, r);
        }
    }
    iadd(e, tile, tile, warps);
    bra(e, top);
    loop_end(e);
    patch_bra(e, done, here(e));
    tail(e);
    return e->err;
}

static int c_codegen(const OmegaScanSpec *spec, BlackwellIRProgram *prog) {
    omega_bw_ir_init(prog);
    Em em; memset(&em, 0, sizeof em); em.p = prog;
    if (gen(&em, spec) != 0 || em.err || em.depth != 0) return -1;
    return omega_bw_regalloc_solve(prog);
}

/* ------------------------------------------------------------------ MIGRATION-PENDING C oracle: ops 0..3 */

static uint32_t c_grid(size_t len) {
    size_t tiles = (len + 31) / 32, ctas = (tiles + OMEGA_GPU_SCAN_THREADS / 32u - 1) / (OMEGA_GPU_SCAN_THREADS / 32u);
    return ctas > OMEGA_GPU_SCAN_MAX_CTAS ? OMEGA_GPU_SCAN_MAX_CTAS : (uint32_t)(ctas ? ctas : 1);
}

/* one launch record at plan[16 + 12 i] (the shift word stays 0 for these ops) */
static void c_launch(uint64_t *plan, OmegaScanKind k, uint64_t in, uint64_t out, uint64_t tot, size_t len) {
    uint64_t *a = plan + OMEGA_GPU_SCAN_PLAN_LAUNCH(plan[0]);
    a[0] = (uint64_t)k; a[1] = c_grid(len);
    a[2] = (uint32_t)in; a[3] = (uint32_t)(in >> 32); a[4] = (uint32_t)out; a[5] = (uint32_t)(out >> 32);
    a[6] = (uint32_t)tot; a[7] = (uint32_t)(tot >> 32);
    a[8] = (uint32_t)len; a[9] = (uint32_t)((len + 31) / 32); a[10] = c_grid(len) * (OMEGA_GPU_SCAN_THREADS / 32u); a[11] = 0;
    plan[0]++;
}

static void c_plan_reduce(OmegaGpuRedOp op, size_t n, uint64_t in_va, uint64_t scratch_va, uint64_t *plan) {
    const OmegaScanKind k = (OmegaScanKind)(OMEGA_SCAN_K_RED_SUM + (int)op);
    uint64_t in = in_va, out = scratch_va;
    size_t len = n, w = 0;
    do {
        c_launch(plan, k, in, out, 0, len);
        len = (len + 31) / 32; w += len;
        in = out; out += len * 4u;
    } while (len > 1);
    plan[1] = w; plan[2] = in;
}

static int c_plan_prefix(size_t n, uint64_t in_va, uint64_t out_va, uint64_t scratch_va, uint64_t *plan) {
    enum { MAXL = 8 }; /* 32^6 >= 2^28 */
    uint64_t buf[MAXL], tot[MAXL];
    size_t len[MAXL];
    unsigned L = 0;
    uint64_t in = in_va, sp = scratch_va;
    buf[0] = out_va; len[0] = n;
    for (;;) { /* forward: scan every level, its totals become the next level (scanned in place) */
        size_t nt = (len[L] + 31) / 32;
        tot[L] = sp; sp += nt * 4u;
        c_launch(plan, OMEGA_SCAN_K_TILE, in, buf[L], tot[L], len[L]);
        if (nt == 1) break;
        if (L + 1 >= MAXL) return OMEGA_GPU_SCAN_TOO_LARGE;
        buf[L + 1] = tot[L]; len[L + 1] = nt; in = tot[L];
        L++;
    }
    while (L-- > 0) /* back down: carry the scanned totals of level L + 1 into level L */
        c_launch(plan, OMEGA_SCAN_K_ADD, buf[L], buf[L + 1], 0, len[L]);
    plan[1] = (sp - scratch_va) / 4u;
    return OMEGA_GPU_SCAN_OK;
}

static int c_plan(OmegaScanOp op, size_t n, uint64_t in, uint64_t out, uint64_t scr, uint64_t *plan) {
    memset(plan, 0, OMEGA_GPU_SCAN_PLAN_WORDS * sizeof *plan);
    if (op <= OMEGA_SCAN_OP_RED_MIN) { c_plan_reduce((OmegaGpuRedOp)op, n, in, scr, plan); return OMEGA_GPU_SCAN_OK; }
    return c_plan_prefix(n, in, out, scr, plan);
}
#endif /* SCAN_C_ORACLE */

#ifdef CUDAPP_MIGRATION_PENDING_SCAN_DIFF
/* ------------------------------------------------------------------ the C-vs-Omega differential */

static void diff_same(const char *what, unsigned a, size_t b) {
    const char *path = getenv("CUDAPP_SCAN_DIFF_LOG");
    if (!path || !*path) return;
    FILE *f = fopen(path, "a");
    if (!f) return;
    fprintf(f, "%s %u %zu SAME\n", what, a, b);
    fclose(f);
}

static void diff_abort(const char *what, unsigned a, size_t b) {
    fprintf(stderr, "omega_gpu_scan differential: %s %u %zu differs between C and Omega\n", what, a, b);
    abort();
}

#ifdef CUDAPP_SCAN_DIFF_SELFTEST
/* the self-test build corrupts one Omega result before the comparison: CUDAPP_SCAN_DIFF_TAMPER=codegen or plan */
static int selftest_is(const char *what) {
    const char *t = getenv("CUDAPP_SCAN_DIFF_TAMPER");
    return t && strcmp(t, what) == 0;
}
#endif

static void diff_prog(const OmegaScanSpec *sp, BlackwellIRProgram *om) {
    BlackwellIRProgram *c = calloc(1, sizeof *c);
    if (!c) diff_abort("codegen (out of memory) kind", (unsigned)sp->kind, (size_t)sp->mutant);
#ifdef CUDAPP_SCAN_DIFF_SELFTEST
    if (selftest_is("codegen")) om->insns[0].imm ^= 1u;
#endif
    int same = c_codegen(sp, c) == 0 && c->count == om->count &&
               memcmp(c->insns, om->insns, om->count * sizeof om->insns[0]) == 0 &&
               memcmp(&c->regalloc, &om->regalloc, sizeof om->regalloc) == 0;
    free(c);
    if (!same) diff_abort("codegen kind", (unsigned)sp->kind, (size_t)sp->mutant);
    diff_same("codegen kind", (unsigned)sp->kind, (size_t)sp->mutant);
}

static void diff_plan(OmegaScanOp op, size_t n, uint64_t in, uint64_t out, uint64_t scr, uint64_t *om) {
    uint64_t *c = malloc(OMEGA_GPU_SCAN_PLAN_WORDS * sizeof *c);
    if (!c) diff_abort("plan (out of memory) op", (unsigned)op, n);
#ifdef CUDAPP_SCAN_DIFF_SELFTEST
    if (selftest_is("plan")) om[1] ^= 1u;
#endif
    int same = c_plan(op, n, in, out, scr, c) == OMEGA_GPU_SCAN_OK &&
               memcmp(c, om, OMEGA_GPU_SCAN_PLAN_WORDS * sizeof *c) == 0;
    free(c);
    if (!same) diff_abort("plan op", (unsigned)op, n);
    diff_same("plan op", (unsigned)op, n);
}
#endif /* CUDAPP_MIGRATION_PENDING_SCAN_DIFF */

/* ------------------------------------------------------------------ codegen, plan, driver */

int omega_gpu_scan_codegen(const OmegaScanSpec *spec, BlackwellIRProgram *prog) {
    if (!spec || !prog || (unsigned)spec->kind >= OMEGA_SCAN_K_COUNT) return -1;
#ifdef CUDAPP_MIGRATION_PENDING_C_SCAN
    if (spec->kind <= OMEGA_SCAN_K_ADD && spec->mutant >= 0 && spec->mutant <= 3) return c_codegen(spec, prog);
#endif
    if (om_codegen(spec, prog) != 0) return -1;
#ifdef CUDAPP_MIGRATION_PENDING_SCAN_DIFF
    if (spec->kind <= OMEGA_SCAN_K_ADD && spec->mutant <= 3) diff_prog(spec, prog);
#endif
    return 0;
}

int omega_gpu_scan_plan(OmegaScanOp op, size_t n, uint64_t in_va, uint64_t out_va, uint64_t scratch_va,
                        uint64_t plan[OMEGA_GPU_SCAN_PLAN_WORDS]) {
    if (!plan || (unsigned)op >= OMEGA_SCAN_OP_COUNT || n == 0) return OMEGA_GPU_SCAN_BAD_ARGS;
    if (n > OMEGA_GPU_SCAN_MAX_N) return OMEGA_GPU_SCAN_TOO_LARGE;
#ifdef CUDAPP_MIGRATION_PENDING_C_SCAN
    if (op <= OMEGA_SCAN_OP_PREFIX_F32) return c_plan(op, n, in_va, out_va, scratch_va, plan);
#endif
    int rc = om_plan(op, n, in_va, out_va, scratch_va, plan);
#ifdef CUDAPP_MIGRATION_PENDING_SCAN_DIFF
    if (rc == OMEGA_GPU_SCAN_OK && op <= OMEGA_SCAN_OP_PREFIX_F32) diff_plan(op, n, in_va, out_va, scratch_va, plan);
#endif
    return rc;
}

size_t omega_gpu_scan_scratch_words(OmegaScanOp op, size_t n) {
    uint64_t *plan = malloc(OMEGA_GPU_SCAN_PLAN_WORDS * sizeof *plan);
    size_t w = plan && omega_gpu_scan_plan(op, n, 0, 0, 0, plan) == OMEGA_GPU_SCAN_OK ? (size_t)plan[1] : 0;
    free(plan);
    return w;
}

int omega_gpu_scan_drive(OmegaScanOp op, size_t n, uint64_t in_va, uint64_t out_va, uint64_t scratch_va,
                         uint64_t *result_va, OmegaScanLaunchFn launch, void *ctx, uint32_t *launches) {
    if (!launch) return OMEGA_GPU_SCAN_BAD_ARGS;
    uint64_t *plan = malloc(OMEGA_GPU_SCAN_PLAN_WORDS * sizeof *plan);
    if (!plan) return OMEGA_GPU_SCAN_BAD_ARGS;
    int rc = omega_gpu_scan_plan(op, n, in_va, out_va, scratch_va, plan);
    for (uint64_t i = 0; rc == OMEGA_GPU_SCAN_OK && i < plan[0]; i++) {
        const uint64_t *L = plan + OMEGA_GPU_SCAN_PLAN_LAUNCH(i);
        uint32_t a[OMEGA_GPU_SCAN_ARGS_WORDS];
        for (unsigned j = 0; j < OMEGA_GPU_SCAN_ARGS_WORDS; j++) a[j] = (uint32_t)L[2 + j];
        if (L[0] >= OMEGA_SCAN_K_COUNT) rc = OMEGA_GPU_SCAN_CODEGEN_FAIL;
        else if (launch(ctx, (OmegaScanKind)L[0], a, (uint32_t)L[1]) != 0) rc = OMEGA_GPU_SCAN_CHIP_FAIL;
        else if (launches) (*launches)++;
    }
    if (rc == OMEGA_GPU_SCAN_OK && result_va) *result_va = plan[2];
    free(plan);
    return rc;
}

/* ------------------------------------------------------------------ the chip */

static struct {
    int mutant;
    OmegaGpuMem code[OMEGA_SCAN_K_COUNT];
    uint32_t gpr[OMEGA_SCAN_K_COUNT];
    int hooked;
} g;

/* lock held; the session calls it right before it closes the device (also in omega_gpu_session_recover): the kernels go
 * with the device, the next call builds them again on the new one */
static void on_session_close(void) {
    for (unsigned k = 0; k < OMEGA_SCAN_K_COUNT; k++) if (g.code[k].cpu) omega_gpu_session_free(&g.code[k]);
    memset(g.code, 0, sizeof g.code);
}

void omega_gpu_scan_test_set_mutant(int mutant) {
    omega_gpu_session_lock();
    for (unsigned k = 0; k < OMEGA_SCAN_K_COUNT; k++) if (g.code[k].cpu) omega_gpu_session_free(&g.code[k]);
    memset(g.code, 0, sizeof g.code);
    g.mutant = mutant;
    omega_gpu_session_unlock();
}

const char *omega_gpu_scan_rc_name(int rc) {
    switch (rc) {
    case OMEGA_GPU_SCAN_OK: return "OK";
    case OMEGA_GPU_SCAN_BAD_ARGS: return "BAD_ARGS";
    case OMEGA_GPU_SCAN_TOO_LARGE: return "TOO_LARGE";
    case OMEGA_GPU_SCAN_CODEGEN_FAIL: return "CODEGEN_FAIL";
    case OMEGA_GPU_SCAN_CHIP_FAIL: return "CHIP_FAIL";
    default: return "?";
    }
}

/* lock held */
static int kernel_locked(OmegaScanKind k) {
    if (g.code[k].cpu) return OMEGA_GPU_SCAN_OK;
    if (!g.hooked) { /* without the hook a kernel would outlive its device */
        if (omega_gpu_session_on_close(on_session_close) != 0) return OMEGA_GPU_SCAN_CHIP_FAIL;
        g.hooked = 1;
    }
    BlackwellIRProgram *prog = omega_gpu_host_calloc(1, sizeof *prog);
    if (!prog) return OMEGA_GPU_SCAN_CODEGEN_FAIL;
    OmegaScanSpec s = { k, g.mutant };
    int rc = OMEGA_GPU_SCAN_OK;
    size_t need = 0;
    if (omega_gpu_scan_codegen(&s, prog) != 0 || (need = omega_bw_program_code_bytes(prog)) == 0) rc = OMEGA_GPU_SCAN_CODEGEN_FAIL;
    else if (omega_gpu_session_alloc_code(need, &g.code[k]) != 0) rc = OMEGA_GPU_SCAN_CHIP_FAIL;
    else {
        OmegaBlackwellKernel kk;
        if (omega_blackwell_kernel_encode_into(prog, g.code[k].cpu, need, &kk) != 0 || kk.code_size != need) rc = OMEGA_GPU_SCAN_CODEGEN_FAIL;
        else if (omega_gpu_session_sync_to_gpu(&g.code[k]) != 0) rc = OMEGA_GPU_SCAN_CHIP_FAIL;
        else { g.gpr[k] = omega_bw_gpr_declared(kk.gpr_count); if (g.gpr[k] < 64u) g.gpr[k] = 64u; }
        if (rc != OMEGA_GPU_SCAN_OK) { omega_gpu_session_free(&g.code[k]); memset(&g.code[k], 0, sizeof g.code[k]); }
    }
    omega_gpu_host_free(prog);
    return rc;
}

typedef struct { uint64_t elapsed_ns; } ChipCtx;

/* lock held */
static int chip_launch(void *ctx, OmegaScanKind k, const uint32_t args[OMEGA_GPU_SCAN_ARGS_WORDS], uint32_t grid_x) {
    if (kernel_locked(k) != OMEGA_GPU_SCAN_OK) return -1;
    OmegaGpuLaunch L = {
        .code_va = g.code[k].va, .gpr_count = g.gpr[k], .threads_x = OMEGA_GPU_SCAN_THREADS, .threads_y = 1,
        .grid_x = grid_x, .grid_y = 1, .num_elements = (size_t)grid_x * OMEGA_GPU_SCAN_THREADS,
        .args = args, .n_args = OMEGA_GPU_SCAN_ARGS_WORDS, .timeout_ms = TIMEOUT_MS,
    };
    uint64_t ns = 0;
    if (omega_gpu_session_launch(&L, &ns, NULL) != 0) return -1;
    ((ChipCtx *)ctx)->elapsed_ns += ns;
    return 0;
}

/* lock held: one data buffer [x xw | x2 x2w | out | scratch], the inputs copied in, the rest poisoned */
static int stage_locked(const void *x, size_t xw, const void *x2, size_t x2w, size_t out_words, size_t scratch_words,
                        OmegaGpuMem *m) {
    if (!omega_gpu_session_open()) return OMEGA_GPU_SCAN_CHIP_FAIL;
    const size_t total = xw + x2w + out_words + scratch_words;
    if (omega_gpu_session_alloc(total * 4u, m) != 0) return OMEGA_GPU_SCAN_CHIP_FAIL;
    uint32_t *w = m->cpu;
    memcpy(w, x, xw * 4u);
    if (x2w) memcpy(w + xw, x2, x2w * 4u);
    for (size_t i = xw + x2w; i < total; i++) w[i] = POISON;
    if (omega_gpu_session_sync_to_gpu(m) != 0) { omega_gpu_session_free(m); return OMEGA_GPU_SCAN_CHIP_FAIL; }
    return OMEGA_GPU_SCAN_OK;
}

/* the whole call on the chip: stage, plan and launch, read back; *m stays allocated on success for the caller's
 * readback and is freed by it (lock held throughout) */
typedef struct {
    OmegaScanOp op; size_t n;
    const void *x; size_t xw; const void *x2; size_t x2w; size_t out_words;
} ChipCall;

static int chip_run_locked(const ChipCall *c, OmegaGpuMem *m, uint64_t *result_va, OmegaGpuScanInfo *info) {
    const size_t sw = omega_gpu_scan_scratch_words(c->op, c->n);
    if (sw == 0) return OMEGA_GPU_SCAN_CODEGEN_FAIL;
    int rc = stage_locked(c->x, c->xw, c->x2, c->x2w, c->out_words, sw, m);
    if (rc != OMEGA_GPU_SCAN_OK) return rc;
    ChipCtx cc = { 0 }; uint32_t nl = 0;
    const uint64_t in = m->va, out = m->va + (c->xw + c->x2w) * 4u, scr = out + c->out_words * 4u;
    rc = omega_gpu_scan_drive(c->op, c->n, in, c->out_words ? out : 0, scr, result_va, chip_launch, &cc, &nl);
    if (rc == OMEGA_GPU_SCAN_OK && omega_gpu_session_sync_to_cpu(m) != 0) rc = OMEGA_GPU_SCAN_CHIP_FAIL;
    if (info) { info->launches = nl; info->elapsed_ns = cc.elapsed_ns; }
    if (rc != OMEGA_GPU_SCAN_OK) omega_gpu_session_free(m);
    return rc;
}

int omega_gpu_reduce(OmegaGpuRedOp op, const float *x, size_t n, float *out, OmegaGpuScanInfo *info) {
    uint64_t t0 = omega_gpu_session_now_ns();
    if (info) memset(info, 0, sizeof *info);
    if ((unsigned)op > OMEGA_GPU_RED_MIN || !x || !out || n == 0) return OMEGA_GPU_SCAN_BAD_ARGS;
    if (n > OMEGA_GPU_SCAN_MAX_N) return OMEGA_GPU_SCAN_TOO_LARGE;
    omega_gpu_session_lock();
    OmegaGpuMem m; uint64_t res = 0;
    const ChipCall c = { (OmegaScanOp)op, n, x, n, NULL, 0, 0 };
    int rc = chip_run_locked(&c, &m, &res, info);
    if (rc == OMEGA_GPU_SCAN_OK) {
        uint32_t r; memcpy(&r, (const uint8_t *)m.cpu + (res - m.va), 4);
        if (r == POISON) { omega_gpu_session_set_error("reduce: the result word was never written"); rc = OMEGA_GPU_SCAN_CHIP_FAIL; }
        else memcpy(out, &r, 4);
        omega_gpu_session_free(&m);
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = omega_gpu_session_now_ns() - t0;
    return rc;
}

int omega_gpu_prefix_sum(const float *x, size_t n, float *y, OmegaGpuScanInfo *info) {
    uint64_t t0 = omega_gpu_session_now_ns();
    if (info) memset(info, 0, sizeof *info);
    if (!x || !y || n == 0) return OMEGA_GPU_SCAN_BAD_ARGS;
    if (n > OMEGA_GPU_SCAN_MAX_N) return OMEGA_GPU_SCAN_TOO_LARGE;
    omega_gpu_session_lock();
    OmegaGpuMem m;
    const ChipCall c = { OMEGA_SCAN_OP_PREFIX_F32, n, x, n, NULL, 0, n };
    int rc = chip_run_locked(&c, &m, NULL, info);
    if (rc == OMEGA_GPU_SCAN_OK) {
        const uint32_t *w = (const uint32_t *)m.cpu + n;
        size_t unwritten = ah_check(0,w,w,n);
        if (unwritten) { omega_gpu_session_set_error("prefix sum: output words never written"); rc = OMEGA_GPU_SCAN_CHIP_FAIL; }
        else memcpy(y, w, n * 4u);
        omega_gpu_session_free(&m);
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = omega_gpu_session_now_ns() - t0;
    return rc;
}

int omega_gpu_prefix_sum_u32(const uint32_t *x, size_t n, uint32_t *y, OmegaGpuScanInfo *info) {
    uint64_t t0 = omega_gpu_session_now_ns();
    if (info) memset(info, 0, sizeof *info);
    if (!x || !y || n == 0) return OMEGA_GPU_SCAN_BAD_ARGS;
    if (n > OMEGA_GPU_SCAN_MAX_N) return OMEGA_GPU_SCAN_TOO_LARGE;
    omega_gpu_session_lock();
    OmegaGpuMem m;
    const ChipCall c = { OMEGA_SCAN_OP_PREFIX_U32, n, x, n, NULL, 0, n };
    int rc = chip_run_locked(&c, &m, NULL, info);
    if (rc == OMEGA_GPU_SCAN_OK) {
        const uint32_t *w = (const uint32_t *)m.cpu + n;
        size_t wrong = ah_check(1,w,x,n);
        if (wrong) { omega_gpu_session_set_error("u32 prefix sum: output words do not step by the input"); rc = OMEGA_GPU_SCAN_CHIP_FAIL; }
        else memcpy(y, w, n * 4u);
        omega_gpu_session_free(&m);
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = omega_gpu_session_now_ns() - t0;
    return rc;
}

static int sort_chip(uint32_t *keys, uint32_t *vals, size_t n, OmegaGpuScanInfo *info) {
    uint64_t t0 = omega_gpu_session_now_ns();
    if (info) memset(info, 0, sizeof *info);
    if (!keys || n == 0) return OMEGA_GPU_SCAN_BAD_ARGS;
    if (n > OMEGA_GPU_SCAN_MAX_N) return OMEGA_GPU_SCAN_TOO_LARGE;
    omega_gpu_session_lock();
    OmegaGpuMem m;
    const ChipCall c = { vals ? OMEGA_SCAN_OP_SORT_PAIRS : OMEGA_SCAN_OP_SORT_KEYS, n, keys, n, vals, vals ? n : 0, 0 };
    int rc = chip_run_locked(&c, &m, NULL, info);
    if (rc == OMEGA_GPU_SCAN_OK) {
        const uint32_t *w = m.cpu;
        size_t down = ah_check(2,w,w,n);
        if (down) { omega_gpu_session_set_error("radix sort: output keys out of order"); rc = OMEGA_GPU_SCAN_CHIP_FAIL; }
        else { memcpy(keys, w, n * 4u); if (vals) memcpy(vals, w + n, n * 4u); }
        omega_gpu_session_free(&m);
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = omega_gpu_session_now_ns() - t0;
    return rc;
}

int omega_gpu_radix_sort_u32(uint32_t *keys, size_t n, OmegaGpuScanInfo *info) { return sort_chip(keys, NULL, n, info); }

int omega_gpu_radix_sort_pairs_u32(uint32_t *keys, uint32_t *vals, size_t n, OmegaGpuScanInfo *info) {
    if (!vals) return OMEGA_GPU_SCAN_BAD_ARGS;
    return sort_chip(keys, vals, n, info);
}
