/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* DEC warp simulation: the kernels the Omega unit cg_dec builds (through omega_blackwell_codegen_dec_prog, the
 * replay adapter) run in the SIMT warp simulator tests/bw_warp_sim_k.h, both fragment orders and three warp visit
 * orders, against host references from the header's contracts. Integer and data-movement kernels must match bit for
 * bit; kernels with MUFU or a reduction compare within a stated relative bound (the simulator's MUFU is the correctly
 * rounded value). Each builder mutant that changes results must fail its comparison (negative controls). */
#include "omega_blackwell_codegen_dec.h"
#include "bw_warp_sim_k.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static BlackwellIRProgram prog;
static uint8_t smem[OMEGA_DEC_SHARED_BYTES];
static unsigned runs;

static void put64(WsSim *s, uint32_t off, const void *p) {
    uint64_t a = (uint64_t)(uintptr_t)p;
    s->cbank[off / 4] = (uint32_t)a; s->cbank[off / 4 + 1] = (uint32_t)(a >> 32);
}
static void putf(WsSim *s, uint32_t off, float f) { memcpy(&s->cbank[off / 4], &f, 4); }
static uint32_t rng = 12345u;
static float rnd(void) { rng = rng * 1664525u + 1013904223u; return ((float)(rng >> 8) / 16777216.0f) * 4.0f - 2.0f; }

/* build the spec, run every schedule; setup refills inputs before each run, check returns the worst relative error */
typedef void (*Setup)(WsSim *);
typedef double (*Check)(void);
static int sim(const char *name, OmegaDecSpec sp, uint32_t gx, uint32_t gy, Setup setup, Check check, double tol, int expect_fail) {
    if (omega_blackwell_codegen_dec_prog(&sp, &prog) != 0) { printf("FAIL %s: build refused\n", name); return 1; }
    double worst = 0; int fails = 0;
    for (int order = 0; order < 2; order++) for (uint32_t ws = 0; ws < 3; ws++) {
        WsSim s; memset(&s, 0, sizeof s);
        s.p = &prog; s.threads = OMEGA_DEC_THREADS; s.smem = smem; s.smem_bytes = sizeof smem; s.order = order;
        s.warp_sched = ws == 2 ? 0x9e3779b9u : ws;
        setup(&s);
        if (ws_run(&s, gx, gy)) {
            if (expect_fail) { ++fails; continue; }
            printf("FAIL %s: simulator %s pc=%d %s\n", name, ws_err_name(s.err), s.err_pc, s.msg); return 1;
        }
        ++runs;
        double e = check();
        if (e > worst) worst = e;
        if (e > tol) ++fails;
    }
    if (expect_fail ? fails != 6 : fails != 0) {
        printf("FAIL %s: worst relative error %.3g against bound %.3g (%s)\n", name, worst, tol,
               expect_fail ? "the mutant was not caught" : "out of bound");
        return 1;
    }
    printf("ok %-28s %5zu insns worst %.3g bound %.3g%s\n", name, prog.count, worst, tol, expect_fail ? " (mutant caught)" : "");
    return 0;
}
static double rel(float got, double want) { double d = fabs((double)got - want), m = fabs(want); return m > 1e-30 ? d / m : d; }

/* AXPY: x += a over iters * stride, grid 2 (stride 512), 3 iterations */
static float ax[1536], ax0[1536], aa[1536];
static void ax_set(WsSim *s) {
    for (int i = 0; i < 1536; i++) { ax[i] = ax0[i]; }
    put64(s, OMEGA_DEC_AXPY_X, ax); put64(s, OMEGA_DEC_AXPY_A, aa); s->cbank[OMEGA_DEC_AXPY_ITERS / 4] = 3;
}
static double ax_chk(void) { double w = 0; for (int i = 0; i < 1536; i++) { float e = ax0[i] + aa[i]; if (memcmp(&e, &ax[i], 4)) w = 1; } return w; }

/* GATHER: out[r][n] = in[r * stride + tab[n]], width 300, stride 1000, 2 rows */
static float gin[2000], gout[600]; static uint32_t gtab[300];
static void ga_set(WsSim *s) {
    memset(gout, 0, sizeof gout);
    put64(s, OMEGA_DEC_GA_IN, gin); put64(s, OMEGA_DEC_GA_TAB, gtab); put64(s, OMEGA_DEC_GA_OUT, gout);
}
static double ga_chk(void) { for (int r = 0; r < 2; r++) for (int n = 0; n < 300; n++) if (memcmp(&gout[r * 300 + n], &gin[r * 1000 + gtab[n]], 4)) return 1; return 0; }

/* GATE: out[n] = a[n] * sigmoid(g[n]), count 300 */
static float ta[300], tg[300], tout[300];
static void gt_set(WsSim *s) {
    memset(tout, 0, sizeof tout);
    put64(s, OMEGA_DEC_GT_A, ta); put64(s, OMEGA_DEC_GT_G, tg); put64(s, OMEGA_DEC_GT_OUT, tout); s->cbank[OMEGA_DEC_GT_N / 4] = 300;
}
static double gt_chk(void) { double w = 0; for (int n = 0; n < 300; n++) { double e = rel(tout[n], ta[n] / (1.0 + exp(-(double)tg[n]))); if (e > w) w = e; } return w; }

/* SWIGLU: act[c] = silu(gu[c]) * gu[768 + c] * scale, one dense row, scale 0.5 */
static float sgu[1536], sact[768];
static void sw_set(WsSim *s) { memset(sact, 0, sizeof sact); put64(s, OMEGA_DEC_SW_GU, sgu); put64(s, OMEGA_DEC_SW_ACT, sact); putf(s, OMEGA_DEC_SW_SCALE, 0.5f); }
static double sw_chk(void) {
    double w = 0;
    for (int c = 0; c < 768; c++) { double v = sgu[c], e = rel(sact[c], v / (1.0 + exp(-v)) * sgu[768 + c] * 0.5); if (e > w) w = e; }
    return w;
}

/* ARGMAX identity: one CTA over 2048 values with the maximum repeated (the lowest index must win) */
static float mv[2048], mov; static uint32_t moi, mtie_lo;
static void am_set(WsSim *s) { mov = 0; moi = 0xffffffffu; put64(s, OMEGA_DEC_AM_V, mv); put64(s, OMEGA_DEC_AM_OV, &mov); put64(s, OMEGA_DEC_AM_OI, &moi); }
static double am_chk(void) { return (mov == 7.5f && moi == mtie_lo) ? 0 : 1; }

/* RES ADD|NORM: x += a, y = x * RSQ(mean(x^2) + eps) * nw * scale, width 2048 */
static float rx[2048], rx0[2048], ra[2048], rnw[2048], ry[2048];
static void rs_set(WsSim *s) {
    memcpy(rx, rx0, sizeof rx); memset(ry, 0, sizeof ry);
    put64(s, OMEGA_DEC_RES_X, rx); put64(s, OMEGA_DEC_RES_A, ra); put64(s, OMEGA_DEC_RES_NW, rnw); put64(s, OMEGA_DEC_RES_Y, ry);
    putf(s, OMEGA_DEC_RES_EPS, 1e-6f); putf(s, OMEGA_DEC_RES_SCALE, 1.0f);
}
static double rs_chk(void) {
    double ss = 0, w = 0;
    for (int i = 0; i < 2048; i++) { float x = rx0[i] + ra[i]; if (memcmp(&x, &rx[i], 4)) return 1; ss += (double)x * x; }
    double r = 1.0 / sqrt(ss / 2048.0 + 1e-6);
    for (int i = 0; i < 2048; i++) { double e = rel(ry[i], (double)rx[i] * r * rnw[i]); if (e > w) w = e; }
    return w;
}

int main(void) {
    for (int i = 0; i < 1536; i++) { ax0[i] = rnd(); aa[i] = rnd(); sgu[i] = rnd() * 3.0f; }
    for (int i = 0; i < 2000; i++) gin[i] = rnd();
    for (int n = 0; n < 300; n++) { gtab[n] = (uint32_t)((n * 7919u) % 1000u); ta[n] = rnd(); tg[n] = rnd() * 4.0f; }
    for (int i = 0; i < 2048; i++) { mv[i] = rnd(); rx0[i] = rnd(); ra[i] = rnd(); rnw[i] = 1.0f + rnd() * 0.25f; }
    mtie_lo = 1031; mv[1031] = 7.5f; mv[1038] = 7.5f; /* lowest tie in slot j = 7, which mutant 1 skips */ mv[2047] = 7.5f; mv[3] = 7.5f - 1e-6f;
    OmegaDecSpec sp;
    int bad = 0;
#define SPEC(k, md, st, mu, w) (memset(&sp, 0, sizeof sp), sp.kind = (k), sp.mode = (md), sp.stride = (st), sp.mutant = (mu), sp.width = (w), sp)
    /* AXPY: 3 iterations of stride 512 cover the 1536 elements once each, so x = x0 + a exactly */
    bad |= sim("AXPY", SPEC(OMEGA_DEC_AXPY, 0, 512, 0, 0), 2, 1, ax_set, ax_chk, 0, 0);
    bad |= sim("GATHER", SPEC(OMEGA_DEC_GATHER, 0, 1000, 0, 300), 2, 2, ga_set, ga_chk, 0, 0);
    bad |= sim("GATHER mutant 1", SPEC(OMEGA_DEC_GATHER, 0, 1000, 1, 300), 2, 2, ga_set, ga_chk, 0, 1);
    bad |= sim("GATE", SPEC(OMEGA_DEC_GATE, 0, 0, 0, 0), 2, 1, gt_set, gt_chk, 1e-6, 0);
    bad |= sim("GATE mutant 1", SPEC(OMEGA_DEC_GATE, 0, 0, 1, 0), 2, 1, gt_set, gt_chk, 1e-6, 1);
    bad |= sim("SWIGLU", SPEC(OMEGA_DEC_SWIGLU, 0, 0, 0, 0), 3, 1, sw_set, sw_chk, 1e-6, 0);
    bad |= sim("SWIGLU mutant 1", SPEC(OMEGA_DEC_SWIGLU, 0, 0, 1, 0), 3, 1, sw_set, sw_chk, 1e-6, 1);
    bad |= sim("ARGMAX identity", SPEC(OMEGA_DEC_ARGMAX, 0, 0, 0, 0), 1, 1, am_set, am_chk, 0, 0);
    bad |= sim("ARGMAX mutant 1", SPEC(OMEGA_DEC_ARGMAX, 0, 0, 1, 0), 1, 1, am_set, am_chk, 0, 1);
    bad |= sim("RES ADD|NORM", SPEC(OMEGA_DEC_RES, OMEGA_DEC_RES_ADD | OMEGA_DEC_RES_NORM, 0, 0, 0), 1, 1, rs_set, rs_chk, 1e-5, 0);
    bad |= sim("RES ADD|NORM mutant 1", SPEC(OMEGA_DEC_RES, OMEGA_DEC_RES_ADD | OMEGA_DEC_RES_NORM, 0, 1, 0), 1, 1, rs_set, rs_chk, 1e-5, 1);
    if (bad) return 1;
    printf("PASS dec warp simulation runs=%u (6 kernels x 6 schedules, 5 builder mutants caught)\n", runs);
    return 0;
}
