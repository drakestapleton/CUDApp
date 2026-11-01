/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* gpu_latmeas_test: dependent-chain latency kernels for GB10 (sm_121).
 *
 * The chip path is tools/latmeas/run_spark.sh. It encodes every kernel before
 * taking the shared lock, then launches them. This file does not invent a
 * measured latency: --self-test and --emit never open a device, and a launch
 * writes only numbers the chip returned.
 *
 * Clock. NAK reads the shader clock with CS2R on SM >= 50, not S2R
 * (nak/from_nir.rs, SM >= 50 branch on NAK_SV_CLOCK). nak_private.h names
 * NAK_SV_CLOCK_LO = 0x50 and NAK_SV_CLOCK_HI = 0x51. sm70_encode.rs encodes
 * OpCS2R as opcode 0x805, destination at bits 16..24, index at bits 72..80,
 * and bit 80 set for a 64-bit read. A 32-bit CS2R is Dualalu (fixed) in
 * sm120_instr_latencies.rs. The encoder has S2R and not CS2R, so each clock
 * site is emitted as S2R immediate 0x50 and the low 12 bits are rewritten to
 * 0x805. The index byte is left as 0x50. Two such sites bracket the chain.
 * Mesa commit 01bab9302a6bf0a082c7ba8c49d74427cf10b0cf.
 *
 * What is compared to NAK. Fixed-latency producers do not scoreboard a GPR
 * result (src/omega_numeric.c): the stall field is the wait. Each chain is
 * launched once per stall. The result has to match the host oracle (or, for
 * HMMA, the stall-36 result of the same kernel). The reported stall is the
 * smallest S such that S and every larger S matched on all runs. Cycles are
 * the 32-bit clock delta around that same chain, median and max-min over the
 * runs. An empty loop with the same trip count is the overhead control.
 *
 * The two CS2R reads bracket the loop, not one instruction. Between them sit
 * the producer, its consumer, the live-value advance, the sentinel move, and
 * the loop-back (increment, compare, branch). A programmed stall of 1 is only
 * the producer's field. The predicate chain puts that stall on the ISETP that
 * feeds @P0 BRA: a shorter stall leaves the previous P0, which was set true,
 * so the poison move runs and the clocks do not cover the whole trip count.
 *
 * The loop increment stalls 15 before ISETP. Stall 1 there is shorter than a
 * fixed-latency add, so the compare samples the old counter and the body runs
 * one extra time. sm121dis of the previous overhead kernel showed that IADD
 * (control 0x000fc200, stall 1) immediately before ISETP, with @!P0 BRA back
 * to the IADD. The overhead word is that counter. The host simulator below
 * applies the stall counts and rejects that shape.
 *
 * One warp, 32 threads, uniform control flow inside the timed region. Every
 * lane stores eight words. Mesa commit and the NAK line citations live in
 * tools/latmeas/diff_nak.py.
 *
 * Opening the device on a GB10 needs the driver's SM version 0xa04 admitted
 * (draft PR #251, or the same change). select_classes is not modified here.
 *
 * Each fixed-latency case writes a destination that was armed to SENTINEL and
 * does not read that destination. The next instruction reads it. A stale read
 * therefore adds SENTINEL, which the host oracle rejects. Back-to-back
 * `r = r + 1` can stay correct at stall 1 when the new value is forwarded, so
 * that shape cannot show a too-short stall. HMMA keeps the shape that resolved
 * on the chip: d starts at 0, HMMA writes it, FADD reads d0.
 */
#include "omega_blackwell_codegen.h"
#include "omega_gpu_session.h"
#include "omega_sm121_disasm.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#define NOREG (-1)
#define SB_NONE 7u
#define SB_GPR 0u
#define SB_RD 1u
#define CLOCK_LO 0x50u /* NAK_SV_CLOCK_LO, nak_private.h */
#define CS2R_OPC 0x805u
#define S2R_OPC 0x919u
#define POISON 0xA5A5A5A5u
#define PRED_POISON 0xDEADBEEFu
#define SWEEP_MAX 36
#define CODE_CAP 8192u

enum {
    RUNS = 11,
    N_FIXED = 32,
    HMMA_ITERS = 1,
    SWEEP_FIXED = 15,
    SWEEP_HMMA = 36
};
#define SENTINEL 0x7f800000u /* +inf bits: not equal to 1..N or to f32 1..N */
#define LANE_N 32
#define LANE_WORDS 8
#define LANE_STRIDE 32u /* LANE_WORDS * 4: t0, t1, r0, r1, r2, r3, tid, unused */
_Static_assert(RUNS % 2 == 1, "median uses the middle of an odd run count");
_Static_assert(SWEEP_HMMA <= SWEEP_MAX, "sweep fits the sample array");

static const char *MESA_COMMIT = "01bab9302a6bf0a082c7ba8c49d74427cf10b0cf";

typedef struct {
    const char *id;
    unsigned iters;
    unsigned unroll;
    unsigned sweep;
    unsigned ops;
    int href; /* 1: oracle is the result at sweep, not a host formula */
} CaseDef;

static const CaseDef CASES[] = {
    {"alu_alu", N_FIXED, 1, SWEEP_FIXED, N_FIXED, 0},
    {"fma_fma", N_FIXED, 1, SWEEP_FIXED, N_FIXED, 0},
    {"mixed", N_FIXED, 1, SWEEP_FIXED, N_FIXED * 2u, 0},
    {"pred", N_FIXED, 1, SWEEP_FIXED, N_FIXED, 0},
    {"fmnmx_alu", N_FIXED, 1, SWEEP_FIXED, N_FIXED, 0},
    {"imad_wide_lo", N_FIXED, 1, SWEEP_FIXED, N_FIXED, 0},
    {"hmma_reader", HMMA_ITERS, 1, SWEEP_HMMA, HMMA_ITERS, 0},
};
#define NCASES ((int)(sizeof CASES / sizeof CASES[0]))

static BlackwellIRProgram g_prog;

typedef struct {
    BlackwellIRProgram *p;
    int desc;
    uint32_t pending;
    int err;
} B;

static uint32_t cw(unsigned stall, unsigned wbar, unsigned rbar, unsigned wait, int yield) {
    uint32_t v = (stall << 9) | (wbar << 14) | (rbar << 17) | (wait << 20);
    if (yield) v |= 0x2000u;
    return v;
}

static int V(B *b) {
    int v = omega_bw_ir_alloc_vreg(b->p);
    if (v < 0) b->err = -1;
    return v;
}
static int V64(B *b) {
    int v = omega_bw_ir_alloc_vreg64(b->p);
    if (v < 0) b->err = -1;
    return v;
}
static int V128(B *b) {
    int v = omega_bw_ir_alloc_vreg128(b->p);
    if (v < 0) b->err = -1;
    return v;
}

static int add(B *b, BlackwellIROpcode op, int dst, int s1, int s2, int s3, uint32_t imm,
               uint32_t control, int p0, int pnot, int dst_sub, int s2_sub) {
    BlackwellIRInsn in;
    memset(&in, 0, sizeof in);
    in.op = op;
    in.dst_vreg = dst;
    in.src1_vreg = s1;
    in.src2_vreg = s2;
    in.src3_vreg = s3;
    in.ureg = NOREG;
    in.imm = imm;
    in.control = control;
    in.predicate_p0 = p0;
    in.predicate_not = pnot;
    in.dst_subreg = (uint8_t)dst_sub;
    in.src2_subreg = (uint8_t)s2_sub;
    if (op == BW_IR_LDCU64) in.is_uniform = 1;
    if (op == BW_IR_STG_E) in.ureg = b->desc;
    if (omega_bw_ir_append(b->p, &in) < 0) b->err = -1;
    return (int)b->p->count - 1;
}

static void movi(B *b, int dst, uint32_t imm, int sub) {
    uint32_t wait = b->pending;
    add(b, BW_IR_MOV_IMM, dst, NOREG, NOREG, NOREG, imm, cw(6, SB_NONE, SB_NONE, wait, 0), 0, 0, sub, 0);
    b->pending = 0;
}
static void movz(B *b, int dst, int sub) {
    uint32_t wait = b->pending;
    add(b, BW_IR_MOV_RZ, dst, NOREG, NOREG, NOREG, 0, cw(6, SB_NONE, SB_NONE, wait, 0), 0, 0, sub, 0);
    b->pending = 0;
}
static void iadd(B *b, int dst, int a, int c, unsigned stall, int dst_sub, int src1_sub, int src2_sub) {
    uint32_t wait = b->pending;
    int at = add(b, BW_IR_IADD3, dst, a, c, NOREG, 0, cw(stall, SB_NONE, SB_NONE, wait, 0), 0, 0, dst_sub, 0);
    if (at >= 0 && src1_sub) b->p->insns[at].src1_subreg = (uint8_t)src1_sub;
    if (at >= 0 && src2_sub) b->p->insns[at].src2_subreg = (uint8_t)src2_sub;
    b->pending = 0;
}
static void ffma(B *b, int dst, int a, int c, int d, unsigned stall) {
    uint32_t wait = b->pending;
    add(b, BW_IR_FFMA, dst, a, c, d, 0, cw(stall, SB_NONE, SB_NONE, wait, 0), 0, 0, 0, 0);
    b->pending = 0;
}
static void fadd(B *b, int dst, int a, int asub, int c, unsigned stall) {
    uint32_t wait = b->pending;
    int at = add(b, BW_IR_FADD, dst, a, c, NOREG, 0, cw(stall, SB_NONE, SB_NONE, wait, 0), 0, 0, 0, 0);
    if (at >= 0 && asub) b->p->insns[at].src1_subreg = (uint8_t)asub;
    b->pending = 0;
}
static void fmnmx(B *b, int dst, int a, int c, unsigned stall) {
    uint32_t wait = b->pending;
    add(b, BW_IR_FMNMX_MIN, dst, a, c, NOREG, 0, cw(stall, SB_NONE, SB_NONE, wait, 0), 0, 0, 0, 0);
    b->pending = 0;
}
static void isetp_ge(B *b, int a, int c, unsigned stall) {
    uint32_t wait = b->pending;
    add(b, BW_IR_ISETP_GE_U32, NOREG, a, c, NOREG, 0, cw(stall, SB_NONE, SB_NONE, wait, 0), 0, 0, 0, 0);
    b->pending = 0;
}
static int bra(B *b, int p0, int pnot, unsigned stall) {
    uint32_t wait = b->pending;
    int at = add(b, BW_IR_BRA, NOREG, NOREG, NOREG, NOREG, 0, cw(stall, SB_NONE, SB_NONE, wait, 0), p0, pnot, 0, 0);
    b->pending = 0;
    return at;
}
static void patch_bra(B *b, int at, int target) {
    if (at < 0 || target < 0 || (size_t)at >= b->p->count) {
        b->err = -1;
        return;
    }
    b->p->insns[at].imm = (uint32_t)(int32_t)(target - at);
}
static void nop_stall(B *b, unsigned stall) {
    add(b, BW_IR_NOP, NOREG, NOREG, NOREG, NOREG, 0, cw(stall, SB_NONE, SB_NONE, 0, 0), 0, 0, 0, 0);
}
/* Producer stall is at most 15. Extra cycles are NOP stalls so the consumer
 * issues `gap` cycles after the producer. */
static void gap_nops(B *b, unsigned gap) {
    while (gap) {
        unsigned s = gap > 15u ? 15u : gap;
        nop_stall(b, s);
        gap -= s;
    }
}
static unsigned stall_head(unsigned gap) { return gap > 15u ? 15u : gap; }

static void clock_read(B *b, int dst) {
    uint32_t wait = b->pending;
    /* Stall 15, no write barrier: the 32-bit CS2R is fixed-latency, and the
     * timed body uses other registers. Rewritten to CS2R after encoding. */
    add(b, BW_IR_S2R, dst, NOREG, NOREG, NOREG, CLOCK_LO, cw(15, SB_NONE, SB_NONE, wait, 0), 0, 0, 0, 0);
    b->pending = 0;
}

static void pin_live(BlackwellIRProgram *p) {
    int end = (int)p->count - 1;
    if (end < 0) return;
    for (int v = 0; v < p->regalloc.num_vregs; v++) {
        OmegaLiveInterval *iv = &p->regalloc.intervals[v];
        if (iv->first_def >= 0 && iv->last_use < end) iv->last_use = end;
    }
}

static uint32_t fbits(float f) {
    uint32_t u;
    memcpy(&u, &f, 4);
    return u;
}

/* HMMA.16816.F32.BF16: each of the four f32 accumulators is a dot of k=16.
 * This kernel writes bf16 1.0 into every A and B half and 0 into C, so each
 * accumulator is 16 products of 1*1. The same value on every lane, because
 * every lane's fragments are initialized alike. */
static uint32_t hmma_16816_ones(void) {
    float acc = 0.f;
    for (int k = 0; k < 16; k++) acc += 1.f * 1.f;
    return fbits(acc);
}

/* stale=1: the consumer sees SENTINEL instead of the value just written.
 * The two results differ for every fixed-latency case. HMMA has no closed form. */
static int eval_case(const CaseDef *c, int stale, uint32_t *result, uint32_t *aux) {
    if (!c || c->href) return -1;
    unsigned n = c->iters;
    *aux = 1;
    *result = 0;
    if (!strcmp(c->id, "alu_alu") || !strcmp(c->id, "imad_wide_lo")) {
        uint32_t acc = 0, live = 0;
        for (unsigned i = 0; i < n; i++) {
            uint32_t produced = live + 1u;
            acc += stale ? SENTINEL : produced;
            live += 1u;
        }
        *result = acc;
        return 0;
    }
    if (!strcmp(c->id, "fma_fma")) {
        float acc = 0.f, live = 0.f, inf = 0.f;
        memcpy(&inf, &(uint32_t){SENTINEL}, 4);
        for (unsigned i = 0; i < n; i++) {
            float produced = live + 1.f;
            float seen = stale ? inf : produced;
            acc = acc + seen;
            live = produced;
        }
        *result = fbits(acc);
        return 0;
    }
    if (!strcmp(c->id, "fmnmx_alu")) {
        uint32_t acc = 0;
        float cur = 1.f;
        for (unsigned i = 0; i < n; i++) {
            acc += stale ? SENTINEL : fbits(cur);
            cur += 1.f;
        }
        *result = acc;
        return 0;
    }
    if (!strcmp(c->id, "mixed")) {
        uint32_t acc = 0;
        float live = 0.f;
        for (unsigned i = 0; i < n; i++) {
            float produced = live + 1.f;
            acc += stale ? SENTINEL : fbits(produced);
            live = produced;
        }
        uint32_t acc2 = 0;
        float inf = 0.f;
        memcpy(&inf, &(uint32_t){SENTINEL}, 4);
        live = 1.f;
        for (unsigned i = 0; i < n; i++) {
            /* FFMA(0, 0, x) preserves a normal f32 and yields +inf for SENTINEL. */
            float out = stale ? inf : live;
            acc2 += fbits(out);
            live += 1.f;
        }
        *result = acc;
        *aux = acc2;
        return 0;
    }
    if (!strcmp(c->id, "pred")) {
        *result = stale ? PRED_POISON : n;
        return 0;
    }
    if (!strcmp(c->id, "hmma_reader")) {
        uint32_t e = hmma_16816_ones();
        *result = stale ? 0u : e;
        *aux = stale ? 0u : e;
        return 0;
    }
    return -1;
}

static int expect_abs(const CaseDef *c, uint32_t *result, uint32_t *aux) {
    return eval_case(c, 0, result, aux);
}

static void prologue(B *b, int *ptr) {
    b->desc = omega_bw_ir_alloc_uvreg64(b->p);
    if (b->desc < 0) b->err = -1;
    *ptr = V64(b);
    uint32_t wait = b->pending;
    add(b, BW_IR_LDCU64, b->desc, NOREG, NOREG, NOREG, 0x358, cw(4, 2, SB_NONE, wait, 1), 0, 0, 0, 0);
    b->pending = 1u << 2;
    wait = b->pending;
    add(b, BW_IR_LDC64, *ptr, NOREG, NOREG, NOREG, 0x380, cw(4, SB_GPR, SB_NONE, wait, 1), 0, 0, 0, 0);
    b->pending = 1u << SB_GPR;
}

/* Every lane stores eight words at ptr + tid * LANE_STRIDE. There is no
 * lane-0 filter: a lane that did not run would leave POISON in its slot. */
static void store_lanes(B *b, int ptr, int t0, int t1, int r0, int r0s, int r1, int r1s, int r2, int r2s, int r3,
                        int r3s) {
    int tid = V(b);
    int addr = V64(b);
    /* Stall 15, and a scoreboard the IMAD waits on. The address uses tid.
     * A stall of 4 here is shorter than a fixed-latency S2R in the host
     * simulator, which would then scale every lane by 0. */
    uint32_t wait = b->pending;
    add(b, BW_IR_S2R, tid, NOREG, NOREG, NOREG, BW_SR_TID_X, cw(15, SB_GPR, SB_NONE, wait, 1), 0, 0, 0, 0);
    b->pending = 1u << SB_GPR;
    wait = b->pending;
    add(b, BW_IR_IMAD_WIDE, addr, tid, NOREG, ptr, LANE_STRIDE, cw(6, SB_NONE, SB_NONE, wait, 0), 0, 0, 0, 0);
    b->pending = 0;
    uint32_t st = cw(4, SB_NONE, SB_RD, 0, 1);
    add(b, BW_IR_STG_E, NOREG, addr, t0, NOREG, 0, st, 0, 0, 0, 0);
    add(b, BW_IR_STG_E, NOREG, addr, t1, NOREG, 4, st, 0, 0, 0, 0);
    add(b, BW_IR_STG_E, NOREG, addr, r0, NOREG, 8, st, 0, 0, 0, r0s);
    add(b, BW_IR_STG_E, NOREG, addr, r1, NOREG, 12, st, 0, 0, 0, r1s);
    add(b, BW_IR_STG_E, NOREG, addr, r2, NOREG, 16, st, 0, 0, 0, r2s);
    add(b, BW_IR_STG_E, NOREG, addr, r3, NOREG, 20, st, 0, 0, 0, r3s);
    add(b, BW_IR_STG_E, NOREG, addr, tid, NOREG, 24, st, 0, 0, 0, 0);
    add(b, BW_IR_EXIT, NOREG, NOREG, NOREG, NOREG, 0, cw(5, SB_NONE, SB_NONE, 0, 0), 0, 0, 0, 0);
    add(b, BW_IR_BRA, NOREG, NOREG, NOREG, NOREG, 0, 0x000fc000u, 0, 0, 0, 0);
}

static void loop_ctl(B *b, int i, int nreg, int one, int *loop_back) {
    /* 15, not 1. The compare reads i. Stall 1 is shorter than a fixed-latency
     * IADD, so ISETP samples the previous i and the body runs one extra time:
     * the overhead word is then not the trip count, and every exact oracle
     * fails, including a stall at the top of the sweep. 15 is past every fixed
     * GPR latency this kit measures. The stall under test stays on the producer. */
    iadd(b, i, i, one, 15, 0, 0, 0);
    isetp_ge(b, i, nreg, 14); /* P0 = (i >= n) */
    *loop_back = bra(b, 1, 1, 5); /* @!P0 BRA loop */
}

/* Producer writes dst, which was armed to SENTINEL and which the producer does
 * not read. The very next instruction reads dst. A stale read adds SENTINEL. */
static void body(B *b, const CaseDef *c, unsigned stall, int live, int livef, int one, int zero, int fone, int acc,
                 int acc2, int dst, int dstf, int seen, int rpair, int tpair, int *poison_br) {
    unsigned head = stall_head(stall);
    unsigned rest = stall - head;
    if (!strcmp(c->id, "alu_alu")) {
        iadd(b, dst, live, one, head, 0, 0, 0);
        gap_nops(b, rest);
        iadd(b, acc, acc, dst, 15, 0, 0, 0);
        iadd(b, live, live, one, 15, 0, 0, 0);
        movi(b, dst, SENTINEL, 0);
    } else if (!strcmp(c->id, "fma_fma")) {
        ffma(b, dst, fone, fone, livef, head); /* dst = livef + 1 */
        gap_nops(b, rest);
        fadd(b, acc, acc, 0, dst, 15); /* FMA consumer, immediate */
        fadd(b, livef, livef, 0, fone, 15);
        movi(b, dst, SENTINEL, 0);
    } else if (!strcmp(c->id, "mixed")) {
        ffma(b, dstf, fone, fone, livef, head);
        gap_nops(b, rest);
        iadd(b, acc, acc, dstf, 15, 0, 0, 0); /* FMA -> ALU */
        fadd(b, livef, livef, 0, fone, 15);
        movi(b, dstf, SENTINEL, 0);
        iadd(b, dst, live, zero, head, 0, 0, 0); /* ALU copy of a normal f32 bit pattern */
        gap_nops(b, rest);
        ffma(b, seen, zero, zero, dst, 15); /* ALU -> FMA, immediate */
        iadd(b, acc2, acc2, seen, 15, 0, 0, 0);
        fadd(b, live, live, 0, fone, 15);
        movi(b, dst, SENTINEL, 0);
    } else if (!strcmp(c->id, "pred")) {
        /* P0 is true from the previous setup. This ISETP clears it. The next
         * instruction is @P0 BRA poison, so a stale predicate takes the poison. */
        isetp_ge(b, one, zero, 15);
        iadd(b, live, live, one, 15, 0, 0, 0);
        isetp_ge(b, zero, live, head);
        gap_nops(b, rest);
        *poison_br = bra(b, 1, 0, 1);
    } else if (!strcmp(c->id, "fmnmx_alu")) {
        fmnmx(b, dst, livef, livef, head);
        gap_nops(b, rest);
        iadd(b, acc, acc, dst, 15, 0, 0, 0);
        fadd(b, livef, livef, 0, fone, 15);
        movi(b, dst, SENTINEL, 0);
    } else if (!strcmp(c->id, "imad_wide_lo")) {
        uint32_t wait = b->pending;
        add(b, BW_IR_IMAD_WIDE, tpair, one, NOREG, rpair, 1, cw(head, SB_NONE, SB_NONE, wait, 0), 0, 0, 0, 0);
        b->pending = 0;
        gap_nops(b, rest);
        iadd(b, acc, acc, tpair, 15, 0, 0, 0);
        iadd(b, rpair, rpair, one, 15, 0, 0, 0);
        movi(b, tpair, SENTINEL, 0);
        movi(b, tpair, SENTINEL, 1);
    } else {
        b->err = -1;
    }
}

static int build(const CaseDef *c, unsigned stall, unsigned ovh_iters, int overhead, uint8_t *code, size_t cap,
                 size_t *out_len, uint32_t *gpr) {
    if (!overhead && !c) return -1;
    if (!overhead && (stall < 1u || stall > c->sweep)) return -1;
    memset(&g_prog, 0, sizeof g_prog);
    omega_bw_ir_init(&g_prog);
    B b;
    memset(&b, 0, sizeof b);
    b.p = &g_prog;
    b.desc = NOREG;
    int ptr = NOREG;
    prologue(&b, &ptr);
    int one = V(&b), zero = V(&b), fone = V(&b), i = V(&b), nreg = V(&b);
    int r = V(&b), acc = V(&b), acc2 = V(&b), cur = V(&b), tmp = V(&b), dsti = V(&b), seen = V(&b);
    int t0 = V(&b), t1 = V(&b), aux = V(&b);
    int rpair = NOREG, tpair = NOREG, dquad = NOREG, aquad = NOREG, bpair = NOREG, cquad = NOREG;
    int h0 = NOREG, h1 = NOREG, h2 = NOREG, h3 = NOREG;
    unsigned iters = overhead ? ovh_iters : c->iters;
    movi(&b, one, 1, 0);
    movz(&b, zero, 0);
    movi(&b, fone, 0x3f800000u, 0);
    movi(&b, nreg, iters, 0);
    movz(&b, i, 0);
    movz(&b, r, 0);
    movz(&b, acc, 0);
    movz(&b, acc2, 0);
    movz(&b, cur, 0);
    movi(&b, aux, 1, 0);
    if (!overhead && c && !c->href && strcmp(c->id, "pred") != 0 && strcmp(c->id, "imad_wide_lo") != 0)
        movi(&b, dsti, SENTINEL, 0);
    if (!overhead && !strcmp(c->id, "fmnmx_alu")) movi(&b, cur, 0x3f800000u, 0);
    if (!overhead && !strcmp(c->id, "mixed")) {
        movi(&b, r, 0x3f800000u, 0); /* ALU source: f32 1.0 bits, a normal number */
        movi(&b, tmp, SENTINEL, 0);  /* FMA destination, separate from dsti */
    }
    if (!overhead && !strcmp(c->id, "imad_wide_lo")) {
        rpair = V64(&b);
        tpair = V64(&b);
        movz(&b, rpair, 0);
        movz(&b, rpair, 1);
        movi(&b, tpair, SENTINEL, 0);
        movi(&b, tpair, SENTINEL, 1);
    }
    if (!overhead && !strcmp(c->id, "hmma_reader")) {
        aquad = V128(&b);
        bpair = V64(&b);
        cquad = V128(&b);
        dquad = V128(&b);
        for (int s = 0; s < 4; s++) movi(&b, aquad, 0x3f803f80u, s);
        for (int s = 0; s < 2; s++) movi(&b, bpair, 0x3f803f80u, s);
        for (int s = 0; s < 4; s++) movz(&b, cquad, s);
        for (int s = 0; s < 4; s++) movz(&b, dquad, s);
        movi(&b, acc, 0xffffffffu, 0);
    }
    clock_read(&b, t0);
    int loop = (int)g_prog.count;
    int poison_br = -1;
    int back = -1;
    if (!overhead && !strcmp(c->id, "hmma_reader")) {
        unsigned head = stall_head(stall);
        unsigned rest = stall - head;
        uint32_t wait = b.pending;
        add(&b, BW_IR_HMMA_BF16, dquad, aquad, bpair, cquad, 0, cw(head, SB_NONE, SB_NONE, wait, 0), 0, 0, 0, 0);
        b.pending = 0;
        gap_nops(&b, rest);
        /* Four consumers, one per destination register. Stall 1 between them
         * is the minimum issue gap: d1 is sampled one cycle after d0, d2 two,
         * d3 three. The programmed stall is the gap before d0. */
        h0 = V(&b);
        h1 = V(&b);
        h2 = V(&b);
        h3 = V(&b);
        fadd(&b, h0, dquad, 0, zero, 1);
        fadd(&b, h1, dquad, 1, zero, 1);
        fadd(&b, h2, dquad, 2, zero, 1);
        fadd(&b, h3, dquad, 3, zero, 1);
        if (c->iters > 1) {
            /* HMMA_ITERS is 1: a repeated identical HMMA would make a stale
             * read equal the previous correct write. */
            b.err = -1;
        }
    } else if (!overhead) {
        body(&b, c, stall, r, cur, one, zero, fone, acc, acc2, dsti, tmp, seen, rpair, tpair, &poison_br);
    }
    if (!(c && !overhead && c->iters == 1 && !strcmp(c->id, "hmma_reader"))) {
        loop_ctl(&b, i, nreg, one, &back);
        patch_bra(&b, back, loop);
    }
    int join_br = -1;
    if (poison_br >= 0) join_br = bra(&b, 0, 0, 5);
    if (poison_br >= 0) {
        int poison = (int)g_prog.count;
        movi(&b, r, PRED_POISON, 0);
        patch_bra(&b, poison_br, poison);
        patch_bra(&b, join_br, (int)g_prog.count);
    }
    clock_read(&b, t1);
    int result = r;
    int rsub = 0;
    int auxv = aux;
    int asub = 0;
    if (!overhead && c && !c->href && strcmp(c->id, "pred") != 0) result = acc;
    if (!overhead && !strcmp(c->id, "mixed")) auxv = acc2;
    int r2 = zero, r3 = zero, r2s = 0, r3s = 0;
    if (!overhead && !strcmp(c->id, "hmma_reader")) {
        result = h0;
        rsub = 0;
        auxv = h1;
        asub = 0;
        r2 = h2;
        r3 = h3;
    }
    if (overhead) result = i;
    store_lanes(&b, ptr, t0, t1, result, rsub, auxv, asub, r2, r2s, r3, r3s);
    if (b.err) return -1;
    pin_live(&g_prog);
    if (omega_bw_regalloc_solve(&g_prog) != 0) return -1;
    if (cap < omega_bw_program_code_bytes(&g_prog)) return -1;
    if (omega_bw_encode_program(&g_prog, code, cap, out_len) != 0) return -1;
    int clocks = 0;
    for (size_t at = 0; at + 16 <= *out_len && at / 16 < g_prog.count; at += 16) {
        uint32_t w0, w2;
        memcpy(&w0, code + at, 4);
        memcpy(&w2, code + at + 8, 4);
        if ((w0 & 0xfffu) == S2R_OPC && ((w2 >> 8) & 0xffu) == CLOCK_LO) {
            w0 = (w0 & ~0xfffu) | CS2R_OPC;
            memcpy(code + at, &w0, 4);
            clocks++;
        }
    }
    if (clocks != 2) return -1;
    if (gpr) *gpr = omega_bw_gpr_declared(g_prog.regalloc.peak_gpr_usage);
    return 0;
}

static uint32_t rd32(const uint8_t *p) {
    uint32_t w;
    memcpy(&w, p, 4);
    return w;
}
static unsigned stall_of(const uint8_t *insn) { return (rd32(insn + 12) >> 9) & 15u; }
static unsigned opc_of(const uint8_t *insn) { return rd32(insn) & 0xfffu; }

static int count_opc_stall(const uint8_t *code, size_t ninsn, unsigned opc, unsigned stall) {
    int n = 0;
    for (size_t i = 0; i < ninsn; i++) {
        const uint8_t *ins = code + i * 16;
        if (opc_of(ins) == opc && stall_of(ins) == stall) n++;
    }
    return n;
}
static unsigned reg_dst(const uint8_t *ins) { return (rd32(ins) >> 16) & 0xffu; }
static unsigned reg_a(const uint8_t *ins) { return (rd32(ins) >> 24) & 0xffu; }
static unsigned reg_b(const uint8_t *ins) { return rd32(ins + 4) & 0xffu; }
static unsigned reg_c(const uint8_t *ins) { return rd32(ins + 8) & 0xffu; }
static int bare_issue(const uint8_t *ins) {
    unsigned w3 = rd32(ins + 12);
    return ((w3 >> 13) & 1u) == 0u && ((w3 >> 14) & 7u) == 7u && ((w3 >> 17) & 7u) == 7u && ((w3 >> 20) & 63u) == 0u;
}
static int reads_reg(const uint8_t *ins, unsigned r) {
    unsigned opc = opc_of(ins);
    if (opc == 0x223u) return reg_c(ins) == r;
    if (opc == 0x221u || opc == 0x210u) return reg_a(ins) == r || reg_b(ins) == r;
    return 0;
}
static int bra_index(const uint8_t *ins, int index) {
    uint32_t w0 = rd32(ins), w1 = rd32(ins + 4);
    int32_t low = (int32_t)((w0 >> 18) & 0x3fu);
    int32_t high = (int32_t)w1 >> 2;
    int32_t rel = (high << 6) | low;
    return index + rel + 1;
}

/* Stall-1 shape: the consumer is the next instruction, it reads the producer's
 * destination, and the producer carries no scoreboard and no yield. A stale
 * read of the armed SENTINEL is then visible to the oracle. */
static int check_raw_window(const char *id, const uint8_t *code, size_t ninsn, unsigned stall) {
    int failed = 0, producers = 0;
#define W(cond, ...)                                                                                                   \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            failed++;                                                                                                  \
            printf("FAIL: %s: ", id);                                                                                  \
            printf(__VA_ARGS__);                                                                                       \
            printf("\n");                                                                                              \
        }                                                                                                              \
    } while (0)
    for (size_t i = 0; i + 1 < ninsn; i++) {
        const uint8_t *ins = code + i * 16;
        const uint8_t *nxt = code + (i + 1) * 16;
        unsigned opc = opc_of(ins);
        if (stall_of(ins) != stall) continue;
        int take = 0;
        if (!strcmp(id, "alu_alu") && opc == 0x210u && reg_dst(ins) != reg_a(ins)) take = 1;
        if (!strcmp(id, "fma_fma") && opc == 0x223u && reg_dst(ins) != reg_c(ins)) take = 1;
        if (!strcmp(id, "mixed") && opc == 0x223u && reg_dst(ins) != reg_c(ins)) take = 1;
        if (!strcmp(id, "mixed") && opc == 0x210u && reg_dst(ins) != reg_a(ins)) take = 1;
        if (!strcmp(id, "fmnmx_alu") && opc == 0x209u && reg_dst(ins) != reg_a(ins)) take = 1;
        if (!strcmp(id, "imad_wide_lo") && opc == 0x825u) take = 1;
        if (!strcmp(id, "hmma_reader") && opc == 0x23cu) take = 1;
        if (!strcmp(id, "pred") && opc == 0x20cu) take = 1;
        if (!take) continue;
        producers++;
        W(bare_issue(ins), "producer %zu is scoreboarded or yields (w3 0x%08x)", i, rd32(ins + 12));
        if (!strcmp(id, "pred")) {
            W(opc_of(nxt) == 0x947u, "predicate consumer is not BRA");
            W(((rd32(nxt) >> 12) & 0xfu) == 0u, "poison branch is not @P0");
            int tgt = bra_index(nxt, (int)i + 1);
            W(tgt >= 0 && (size_t)tgt < ninsn, "poison target %d", tgt);
            if (tgt >= 0 && (size_t)tgt < ninsn) {
                const uint8_t *mv = code + (size_t)tgt * 16;
                W(opc_of(mv) == 0x802u && rd32(mv + 4) == PRED_POISON, "poison mov missing");
            }
        } else {
            W(reads_reg(nxt, reg_dst(ins)), "consumer does not read producer dest R%u", reg_dst(ins));
        }
    }
    W(producers == (!strcmp(id, "mixed") ? 2 : 1), "producer count %d", producers);
#undef W
    return failed;
}

/* Host latency simulator. The shared warp simulator (tests/bw_warp_sim.h) writes
 * a register and the next instruction already sees it, so it cannot catch a
 * stall that is too short. This one issues the IR in order and keeps each GPR
 * write hidden for SIM_GPR_LAT cycles and each P0 write hidden for
 * SIM_PRED_LAT cycles. Those are the NAK floors (ALU/FMA raw 4 plus one,
 * predicate raw 13 plus one), used here as the model's assumption. A read at
 * the issue cycle sees a write whose visible cycle is at or before that. The
 * stall field is how many cycles pass before the next issue. */
#define SIM_GPR_LAT 5u
#define SIM_PRED_LAT 14u
#define SIM_LOG 4096

float fmaf(float, float, float);
float fminf(float, float);

typedef struct {
    int is_pred;
    int reg;
    uint32_t issue, visible, value;
} SimWr;

typedef struct {
    SimWr wr[SIM_LOG];
    int nwr;
    uint32_t now;
    int pc;
    int stores;
    const char *err;
} LatSim;

static uint32_t lat_u2f_bits(float f) {
    uint32_t u;
    memcpy(&u, &f, 4);
    return u;
}
static float lat_f(uint32_t u) {
    float f;
    memcpy(&f, &u, 4);
    return f;
}

static int lat_phys(int v, unsigned sub, int *reg) {
    if (v < 0) {
        *reg = -1;
        return 0;
    }
    if (v >= g_prog.regalloc.num_vregs) return -1;
    int ph = g_prog.regalloc.vreg_to_phys[v];
    if (ph < 0) return -1;
    *reg = (ph + (int)sub) & 0xff;
    return 0;
}

static uint32_t lat_gpr(const LatSim *s, int reg) {
    uint32_t val = 0, issue = 0;
    int have = 0;
    if (reg < 0) return 0;
    for (int i = 0; i < s->nwr; i++) {
        if (s->wr[i].is_pred || s->wr[i].reg != reg) continue;
        if (s->wr[i].visible > s->now) continue;
        if (!have || s->wr[i].issue >= issue) {
            have = 1;
            issue = s->wr[i].issue;
            val = s->wr[i].value;
        }
    }
    return val;
}

static int lat_p0(const LatSim *s) {
    uint32_t issue = 0, val = 0;
    int have = 0;
    for (int i = 0; i < s->nwr; i++) {
        if (!s->wr[i].is_pred) continue;
        if (s->wr[i].visible > s->now) continue;
        if (!have || s->wr[i].issue >= issue) {
            have = 1;
            issue = s->wr[i].issue;
            val = s->wr[i].value;
        }
    }
    return val != 0;
}

static int lat_post(LatSim *s, int is_pred, int reg, uint32_t lat, uint32_t value) {
    if (reg < 0 && !is_pred) return 0;
    if (s->nwr >= SIM_LOG) {
        s->err = "write log full";
        return -1;
    }
    s->wr[s->nwr].is_pred = is_pred;
    s->wr[s->nwr].reg = reg;
    s->wr[s->nwr].issue = s->now;
    s->wr[s->nwr].visible = s->now + lat;
    s->wr[s->nwr].value = value;
    s->nwr++;
    return 0;
}

static int lat_rd(const LatSim *s, int v, unsigned sub, uint32_t *out) {
    int reg = 0;
    if (lat_phys(v, sub, &reg) != 0) return -1;
    *out = lat_gpr(s, reg);
    return 0;
}

static int lat_wr(LatSim *s, int v, unsigned sub, uint32_t value, uint32_t lat) {
    int reg = 0;
    if (lat_phys(v, sub, &reg) != 0) return -1;
    return lat_post(s, 0, reg, lat, value);
}

/* One lane. mem[] is the whole warp's output, poisoned by the caller.
 * Returns 0 when this lane stored LANE_WORDS-1 words (the last slot is unused). */
static int lat_sim_lane(uint32_t tid, uint32_t *mem) {
    LatSim s;
    memset(&s, 0, sizeof s);
    for (int steps = 0; steps < 100000; steps++) {
        if (s.pc < 0 || (size_t)s.pc >= g_prog.count) {
            s.err = "pc outside the program";
            break;
        }
        const BlackwellIRInsn *in = &g_prog.insns[s.pc];
        unsigned stall = (in->control >> 9) & 15u;
        int next = s.pc + 1;
        uint32_t a = 0, b = 0, c = 0;
        if (lat_rd(&s, in->src1_vreg, in->src1_subreg, &a) || lat_rd(&s, in->src2_vreg, in->src2_subreg, &b) ||
            lat_rd(&s, in->src3_vreg, in->src3_subreg, &c)) {
            s.err = "vreg has no physical register";
            break;
        }
        switch (in->op) {
        case BW_IR_EXIT:
            if (s.stores != LANE_WORDS - 1) {
                s.err = "lane did not store its seven words";
                break;
            }
            return 0;
        case BW_IR_BRA: {
            int take = 1;
            if (in->predicate_p0) take = in->predicate_not ? !lat_p0(&s) : lat_p0(&s);
            if (take && (int32_t)in->imm == 0) {
                s.err = "branch to itself";
                break;
            }
            if (take) next = s.pc + (int32_t)in->imm;
            break;
        }
        case BW_IR_NOP:
        case BW_IR_LDCU64:
        case BW_IR_LDCU:
            break;
        case BW_IR_LDC64:
            if (lat_wr(&s, in->dst_vreg, 0, 0, SIM_GPR_LAT) || lat_wr(&s, in->dst_vreg, 1, 0, SIM_GPR_LAT))
                s.err = "write log full";
            break;
        case BW_IR_MOV_IMM:
            if (lat_wr(&s, in->dst_vreg, in->dst_subreg, in->imm, SIM_GPR_LAT)) s.err = "write log full";
            break;
        case BW_IR_MOV_RZ:
            if (lat_wr(&s, in->dst_vreg, in->dst_subreg, 0, SIM_GPR_LAT)) s.err = "write log full";
            break;
        case BW_IR_S2R: {
            uint32_t v = 0;
            if (in->imm == BW_SR_TID_X) v = tid;
            else if (in->imm == CLOCK_LO) v = s.now;
            if (lat_wr(&s, in->dst_vreg, in->dst_subreg, v, SIM_GPR_LAT)) s.err = "write log full";
            break;
        }
        case BW_IR_HMMA_BF16: {
            int ones = 1;
            for (unsigned sub = 0; sub < 4 && ones; sub++) {
                uint32_t u = 0;
                if (lat_rd(&s, in->src1_vreg, sub, &u) || u != 0x3f803f80u) ones = 0;
            }
            for (unsigned sub = 0; sub < 2 && ones; sub++) {
                uint32_t u = 0;
                if (lat_rd(&s, in->src2_vreg, sub, &u) || u != 0x3f803f80u) ones = 0;
            }
            for (unsigned sub = 0; sub < 4 && ones; sub++) {
                uint32_t u = 0;
                if (lat_rd(&s, in->src3_vreg, sub, &u) || u != 0) ones = 0;
            }
            if (!ones) {
                s.err = "hmma inputs are not the ones matrix";
                break;
            }
            uint32_t y = hmma_16816_ones();
            for (unsigned sub = 0; sub < 4; sub++)
                if (lat_wr(&s, in->dst_vreg, sub, y, SIM_GPR_LAT)) s.err = "write log full";
            break;
        }
        case BW_IR_IADD3:
            if (lat_wr(&s, in->dst_vreg, in->dst_subreg, a + b, SIM_GPR_LAT)) s.err = "write log full";
            break;
        case BW_IR_ISETP_GE_U32:
            if (lat_post(&s, 1, 0, SIM_PRED_LAT, a >= b ? 1u : 0u)) s.err = "write log full";
            break;
        case BW_IR_FFMA:
            if (lat_wr(&s, in->dst_vreg, in->dst_subreg, lat_u2f_bits(fmaf(lat_f(a), lat_f(b), lat_f(c))), SIM_GPR_LAT))
                s.err = "write log full";
            break;
        case BW_IR_FADD:
            if (lat_wr(&s, in->dst_vreg, in->dst_subreg, lat_u2f_bits(lat_f(a) + lat_f(b)), SIM_GPR_LAT))
                s.err = "write log full";
            break;
        case BW_IR_FMNMX_MIN:
            if (lat_wr(&s, in->dst_vreg, in->dst_subreg, lat_u2f_bits(fminf(lat_f(a), lat_f(b))), SIM_GPR_LAT))
                s.err = "write log full";
            break;
        case BW_IR_IMAD_WIDE: {
            /* Immediate form: Rd:Rd+1 = Ra * imm + Rc+1:Rc. src2 is NOREG. */
            uint32_t hi = 0;
            if (lat_rd(&s, in->src3_vreg, 1, &hi)) {
                s.err = "vreg has no physical register";
                break;
            }
            uint64_t prod = (uint64_t)a * in->imm + (((uint64_t)hi << 32) | c);
            if (lat_wr(&s, in->dst_vreg, 0, (uint32_t)prod, SIM_GPR_LAT) ||
                lat_wr(&s, in->dst_vreg, 1, (uint32_t)(prod >> 32), SIM_GPR_LAT))
                s.err = "write log full";
            break;
        }
        case BW_IR_STG_E: {
            uint32_t lo = 0, hi = 0;
            if (lat_rd(&s, in->src1_vreg, 0, &lo) || lat_rd(&s, in->src1_vreg, 1, &hi)) {
                s.err = "vreg has no physical register";
                break;
            }
            uint64_t ad = (uint64_t)lo | ((uint64_t)hi << 32);
            uint64_t at = ad + (uint32_t)in->imm;
            if ((at & 3u) || at / 4u >= (uint64_t)LANE_N * LANE_WORDS) {
                s.err = "store address is outside the lane buffer";
                break;
            }
            mem[at / 4u] = b;
            s.stores++;
            break;
        }
        default:
            s.err = "opcode not modelled";
            break;
        }
        if (s.err) break;
        s.now += stall ? stall : 1u;
        s.pc = next;
    }
    if (s.err) fprintf(stderr, "lat sim: %s at pc %d\n", s.err, s.pc);
    else fprintf(stderr, "lat sim: step budget at pc %d\n", s.pc);
    return -1;
}

/* The loop-back sm121dis must show: IADD (dest == src A) stall 15, ISETP stall
 * 14, then @!P0 BRA back to the body. The body is at or before that IADD
 * (overhead has no body, so the branch lands on the IADD). HMMA has no loop. */
static int check_loop_sass(const char *id, const uint8_t *code, size_t ninsn, int expect_loop) {
    int found = 0, failed = 0;
    for (size_t i = 0; i + 2 < ninsn; i++) {
        const uint8_t *a = code + i * 16;
        const uint8_t *bp = code + (i + 1) * 16;
        const uint8_t *c = code + (i + 2) * 16;
        if (opc_of(a) != 0x210u || opc_of(bp) != 0x20cu || opc_of(c) != 0x947u) continue;
        if (reg_dst(a) != reg_a(a)) continue;
        if (((rd32(c) >> 12) & 0xfu) != 8u) continue;
        found++;
        if (stall_of(a) != 15u) {
            failed++;
            printf("FAIL: %s loop increment stall %u, want 15\n", id, stall_of(a));
        }
        if (stall_of(bp) != 14u) {
            failed++;
            printf("FAIL: %s loop compare stall %u, want 14\n", id, stall_of(bp));
        }
        int tgt = bra_index(c, (int)i + 2);
        if (tgt < 0 || tgt > (int)i) {
            failed++;
            printf("FAIL: %s loop branch targets %d, increment is at %zu\n", id, tgt, i);
        }
    }
    if (found != expect_loop) {
        failed++;
        printf("FAIL: %s loop-back count %d, want %d\n", id, found, expect_loop);
    }
    return failed;
}

static uint32_t g_sim_mem[LANE_N * LANE_WORDS];

static int lat_sim(uint32_t lane0[LANE_WORDS]) {
    for (int i = 0; i < LANE_N * LANE_WORDS; i++) g_sim_mem[i] = POISON;
    for (uint32_t tid = 0; tid < LANE_N; tid++)
        if (lat_sim_lane(tid, g_sim_mem) != 0) return -1;
    if (lane0) memcpy(lane0, g_sim_mem, sizeof(uint32_t) * LANE_WORDS);
    return 0;
}

/* four=1: words 2..5 are the same value (HMMA d0..d3). Otherwise words 4 and 5 stay 0. */
static int sim_lanes_match(uint32_t result, uint32_t aux, int four) {
    for (int lane = 0; lane < LANE_N; lane++) {
        const uint32_t *w = g_sim_mem + (size_t)lane * LANE_WORDS;
        if (w[6] != (uint32_t)lane || w[2] != result || w[3] != aux) return 0;
        if (four) {
            if (w[4] != result || w[5] != result) return 0;
        } else if (w[4] != 0 || w[5] != 0) {
            return 0;
        }
    }
    return 1;
}

/* Seven stores at +0,+4,...,+24, addressed by IMAD.WIDE Rd, tid, 32, ptr. */
static int check_lane_stores(const char *id, const uint8_t *code, size_t ninsn) {
    int stg = 0, wide = 0, failed = 0;
    int saw_off[7] = {0, 0, 0, 0, 0, 0, 0};
    for (size_t i = 0; i < ninsn; i++) {
        const uint8_t *ins = code + i * 16;
        if (opc_of(ins) == 0x825u && rd32(ins + 4) == LANE_STRIDE) wide++;
        if (opc_of(ins) == 0x986u) {
            stg++;
            uint32_t off = (rd32(ins + 4) >> 8) & 0xffffffu;
            if (off % 4u == 0 && off / 4u < 7u) saw_off[off / 4u] = 1;
        }
    }
    if (wide != 1) {
        failed++;
        printf("FAIL: %s tid-scaled address count %d\n", id, wide);
    }
    if (stg != 7) {
        failed++;
        printf("FAIL: %s store count %d, want 7\n", id, stg);
    }
    for (int k = 0; k < 7; k++)
        if (!saw_off[k]) {
            failed++;
            printf("FAIL: %s missing store +%d\n", id, k * 4);
        }
    return failed;
}

static int loop_iadd_index(void) {
    int found = -1;
    for (size_t i = 0; i + 2 < g_prog.count; i++) {
        const BlackwellIRInsn *a = &g_prog.insns[i];
        const BlackwellIRInsn *b = &g_prog.insns[i + 1];
        const BlackwellIRInsn *c = &g_prog.insns[i + 2];
        if (a->op != BW_IR_IADD3 || b->op != BW_IR_ISETP_GE_U32 || c->op != BW_IR_BRA) continue;
        if (a->dst_vreg != a->src1_vreg) continue;
        if (!c->predicate_p0 || !c->predicate_not) continue;
        found = (int)i;
    }
    return found;
}

static int sim_self_test(void) {
    int failed = 0;
    uint8_t code[CODE_CAP];
    size_t len = 0;
    uint32_t gpr = 0;
    uint32_t out[LANE_WORDS];
#define SCHECK(cond, ...)                                                                                              \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            failed++;                                                                                                  \
            printf("FAIL: ");                                                                                          \
            printf(__VA_ARGS__);                                                                                       \
            printf("\n");                                                                                              \
        }                                                                                                              \
    } while (0)

    SCHECK(build(NULL, 0, N_FIXED, 1, code, sizeof code, &len, &gpr) == 0, "overhead encode");
    SCHECK(check_loop_sass("ovh32", code, g_prog.count, 1) == 0, "overhead sass");
    SCHECK(lat_sim(out) == 0, "overhead sim ran");
    SCHECK(out[2] == N_FIXED && out[3] == 1u, "overhead loop count is %u (aux 0x%x), want %u", out[2], out[3], N_FIXED);
    SCHECK(sim_lanes_match(N_FIXED, 1u, 0), "overhead count on every lane");
    {
        int at = loop_iadd_index();
        SCHECK(at >= 0, "overhead loop increment is in the IR");
        if (at >= 0) {
            uint32_t saved = g_prog.insns[at].control;
            g_prog.insns[at].control = (saved & ~(15u << 9)) | (1u << 9);
            uint32_t bad[LANE_WORDS] = {0};
            SCHECK(lat_sim(bad) == 0, "short-stall overhead sim ran");
            SCHECK(bad[2] != N_FIXED, "stall 1 on the increment must not store the trip count (got %u)", bad[2]);
            g_prog.insns[at].control = saved;
        }
    }
    SCHECK(build(NULL, 0, 1, 1, code, sizeof code, &len, &gpr) == 0, "one-trip overhead encode");
    SCHECK(lat_sim(out) == 0 && out[2] == 1u && sim_lanes_match(1u, 1u, 0), "one-trip overhead stored %u", out[2]);

    for (int ci = 0; ci < NCASES; ci++) {
        int hmma = !strcmp(CASES[ci].id, "hmma_reader");
        uint32_t fresh = 0, fr_aux = 0, stale = 0, st_aux = 0;
        SCHECK(eval_case(&CASES[ci], 0, &fresh, &fr_aux) == 0, "%s fresh", CASES[ci].id);
        SCHECK(eval_case(&CASES[ci], 1, &stale, &st_aux) == 0, "%s stale", CASES[ci].id);
        SCHECK(build(&CASES[ci], CASES[ci].sweep, 0, 0, code, sizeof code, &len, &gpr) == 0, "%s stall %u encode",
               CASES[ci].id, CASES[ci].sweep);
        SCHECK(check_loop_sass(CASES[ci].id, code, g_prog.count, hmma ? 0 : 1) == 0, "%s loop sass", CASES[ci].id);
        SCHECK(lat_sim(out) == 0, "%s stall %u sim", CASES[ci].id, CASES[ci].sweep);
        SCHECK(out[2] == fresh && out[3] == fr_aux, "%s stall %u sim result 0x%x aux 0x%x, fresh 0x%x aux 0x%x",
               CASES[ci].id, CASES[ci].sweep, out[2], out[3], fresh, fr_aux);
        SCHECK(sim_lanes_match(fresh, fr_aux, hmma), "%s stall %u every lane", CASES[ci].id, CASES[ci].sweep);
        SCHECK(build(&CASES[ci], 1, 0, 0, code, sizeof code, &len, &gpr) == 0, "%s stall 1 encode", CASES[ci].id);
        SCHECK(lat_sim(out) == 0, "%s stall 1 sim", CASES[ci].id);
        SCHECK(out[2] == stale && out[3] == st_aux, "%s stall 1 sim result 0x%x aux 0x%x, stale 0x%x aux 0x%x",
               CASES[ci].id, out[2], out[3], stale, st_aux);
        SCHECK(sim_lanes_match(stale, st_aux, hmma), "%s stall 1 every lane", CASES[ci].id);
    }
#undef SCHECK
    return failed;
}

static int lane_field_self_test(void);

static int self_test(void) {
    int failed = 0;
    uint8_t code[CODE_CAP];
    size_t len = 0;
    uint32_t gpr = 0;
#define CHECK(cond, ...)                                                                                               \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            failed++;                                                                                                  \
            printf("FAIL: ");                                                                                          \
            printf(__VA_ARGS__);                                                                                       \
            printf("\n");                                                                                              \
        }                                                                                                              \
    } while (0)
    CHECK(build(&CASES[0], 0, 0, 0, code, sizeof code, &len, &gpr) != 0, "stall 0 must be refused");
    CHECK(build(&CASES[0], 99, 0, 0, code, sizeof code, &len, &gpr) != 0, "stall past the sweep must be refused");
    for (int ci = 0; ci < NCASES; ci++) {
        if (CASES[ci].href) continue;
        uint32_t good = 0, bad = 0, ga = 0, ba = 0;
        CHECK(eval_case(&CASES[ci], 0, &good, &ga) == 0, "%s fresh oracle", CASES[ci].id);
        CHECK(eval_case(&CASES[ci], 1, &bad, &ba) == 0, "%s stale oracle", CASES[ci].id);
        CHECK(good != bad || ga != ba, "%s stale read is distinguishable (fresh 0x%x stale 0x%x)", CASES[ci].id, good, bad);
    }
    {
        uint32_t good = 0, aux = 0;
        CHECK(eval_case(&CASES[0], 0, &good, &aux) == 0 && good == (N_FIXED * (N_FIXED + 1u)) / 2u, "alu sum");
        CHECK(eval_case(&CASES[1], 0, &good, &aux) == 0 && good == fbits((float)(N_FIXED * (N_FIXED + 1u)) / 2u), "fma sum");
    }
    {
        uint32_t er = 0, ea = 0;
        float fifteen = 0.f;
        for (int k = 0; k < 15; k++) fifteen += 1.f;
        CHECK(hmma_16816_ones() == fbits(16.f), "hmma ones dot is 16.0f (got 0x%x)", hmma_16816_ones());
        CHECK(fbits(fifteen) != hmma_16816_ones(), "a 15-term dot must not match k=16");
        CHECK(expect_abs(&CASES[6], &er, &ea) == 0 && er == hmma_16816_ones() && ea == er, "hmma oracle");
    }

    for (int ci = 0; ci < NCASES; ci++) {
        unsigned stall = 1u;
        len = 0;
        int rc = build(&CASES[ci], stall, 0, 0, code, sizeof code, &len, &gpr);
        CHECK(rc == 0, "%s encode rc %d", CASES[ci].id, rc);
        if (rc != 0) continue;
        int clocks = 0, s2r_tid = 0;
        size_t ninsn = g_prog.count;
        for (size_t i = 0; i < ninsn; i++) {
            unsigned opc = opc_of(code + i * 16);
            unsigned idx = (rd32(code + i * 16 + 8) >> 8) & 0xffu;
            if (opc == CS2R_OPC && idx == CLOCK_LO) clocks++;
            if (opc == S2R_OPC && idx == BW_SR_TID_X) s2r_tid++;
        }
        CHECK(clocks == 2, "%s clock sites %d", CASES[ci].id, clocks);
        CHECK(s2r_tid == 1, "%s tid read %d", CASES[ci].id, s2r_tid);
        failed += check_lane_stores(CASES[ci].id, code, ninsn);
        char *text = NULL;
        size_t tlen = 0;
        FILE *mf = open_memstream(&text, &tlen);
        int unk = mf ? omega_sm121_listing(code, len, mf) : -1;
        if (mf) fclose(mf);
        CHECK(unk == 2, "%s disasm unknowns %d (the two CS2R sites)", CASES[ci].id, unk);
        failed += check_raw_window(CASES[ci].id, code, ninsn, stall);
        if (!strcmp(CASES[ci].id, "hmma_reader")) {
            int quads = 0;
            for (size_t i = 0; i + 4 < ninsn; i++) {
                if (opc_of(code + i * 16) != 0x23cu || stall_of(code + i * 16) != 1u) continue;
                unsigned d = reg_dst(code + i * 16);
                int ok = 1;
                for (int k = 0; k < 4; k++) {
                    const uint8_t *f = code + (i + 1 + (size_t)k) * 16;
                    if (opc_of(f) != 0x221u || reg_a(f) != d + (unsigned)k) ok = 0;
                }
                if (ok) quads++;
            }
            CHECK(quads == 1, "hmma stall 1 is followed by FADD of d0,d1,d2,d3");
            len = 0;
            rc = build(&CASES[ci], 29u, 0, 0, code, sizeof code, &len, &gpr);
            CHECK(rc == 0, "hmma stall 29 encode");
            ninsn = g_prog.count;
            CHECK(count_opc_stall(code, ninsn, 0x23c, 15) == 1, "hmma stall 15");
            CHECK(count_opc_stall(code, ninsn, 0x918, 14) == 1, "hmma padding nop stall 14 (15+14=29)");
            int saw = 0;
            for (size_t i = 0; i + 2 < ninsn; i++) {
                if (opc_of(code + i * 16) == 0x23c && stall_of(code + i * 16) == 15 &&
                    opc_of(code + (i + 1) * 16) == 0x918 && stall_of(code + (i + 1) * 16) == 14 &&
                    opc_of(code + (i + 2) * 16) == 0x221) {
                    unsigned dst = (rd32(code + i * 16) >> 16) & 0xffu;
                    unsigned src = (rd32(code + (i + 2) * 16) >> 24) & 0xffu;
                    CHECK(src == dst, "FADD reads HMMA d0 after the 29-cycle gap");
                    saw = 1;
                }
            }
            CHECK(saw, "hmma, nop, fadd sequence");
            /* stall 19 is HMMA stall 15 plus one NOP of stall 4: the gap the chip
             * can fail without faulting. The self-test only checks the shape. */
            len = 0;
            CHECK(build(&CASES[ci], 19u, 0, 0, code, sizeof code, &len, &gpr) == 0, "hmma stall 19 encodes");
            CHECK(count_opc_stall(code, g_prog.count, 0x23c, 15) == 1, "hmma 19 head");
            CHECK(count_opc_stall(code, g_prog.count, 0x918, 4) == 1, "hmma 19 tail nop");
        }
        for (size_t i = 0; i < ninsn; i++) {
            if (opc_of(code + i * 16) == CS2R_OPC) {
                unsigned guard = (rd32(code + i * 16) >> 12) & 0xfu;
                CHECK(guard == 7, "%s CS2R guard %u", CASES[ci].id, guard);
            }
        }
        free(text);
    }
    len = 0;
    CHECK(build(NULL, 0, 64, 1, code, sizeof code, &len, &gpr) == 0, "overhead kernel");
    failed += sim_self_test();
    failed += lane_field_self_test();

    int pass[RUNS][SWEEP_MAX];
    memset(pass, 0, sizeof pass);
    for (int r = 0; r < RUNS; r++)
        for (int s = 5; s <= 15; s++) pass[r][s - 1] = 1;
    /* declared below */
#undef CHECK
    printf("gpu_latmeas_test --self-test %s (%d checks failed)\n", failed ? "FAIL" : "PASS", failed);
    return failed ? 1 : 0;
}

/* The suffix helper is tested from main after self_test's static copy. */
static int required_stall(const int pass[][SWEEP_MAX], int runs, int sweep) {
    for (int s = 1; s <= sweep; s++) {
        int ok = 1;
        for (int t = s; t <= sweep && ok; t++)
            for (int r = 0; r < runs && ok; r++)
                if (!pass[r][t - 1]) ok = 0;
        if (ok) return s;
    }
    return -1;
}
/* 1 means every stall matched, which is not a measured threshold. -1 means no suffix did. */
static int reported_threshold(const int pass[][SWEEP_MAX], int runs, int sweep) {
    int start = required_stall(pass, runs, sweep);
    return start > 1 ? start : -1;
}

static int self_test_suffix(void) {
    int failed = 0;
    int pass[RUNS][SWEEP_MAX];
    memset(pass, 0, sizeof pass);
    for (int r = 0; r < RUNS; r++)
        for (int s = 5; s <= 15; s++) pass[r][s - 1] = 1;
    if (required_stall(pass, RUNS, 15) != 5) {
        printf("FAIL: suffix threshold want 5\n");
        failed++;
    }
    pass[3][4] = 0; /* run 3 fails stall 5; 6..15 still pass */
    if (required_stall(pass, RUNS, 15) != 6) {
        printf("FAIL: one-run hole must move the threshold to 6\n");
        failed++;
    }
    memset(pass, 0, sizeof pass);
    if (required_stall(pass, RUNS, 15) != -1) {
        printf("FAIL: empty sweep must not invent a stall\n");
        failed++;
    }
    for (int r = 0; r < RUNS; r++)
        for (int s = 1; s <= 15; s++) pass[r][s - 1] = 1;
    if (reported_threshold(pass, RUNS, 15) != -1) {
        printf("FAIL: a sweep that passes at stall 1 is not a measured threshold\n");
        failed++;
    }
    return failed;
}

static int plan_launches(void) {
    int n = 0;
    int seen64 = 0, seen32 = 0, seen1 = 0;
    for (int i = 0; i < NCASES; i++) {
        n += RUNS * (int)CASES[i].sweep;
        if (CASES[i].iters == 64) seen64 = 1;
        if (CASES[i].iters == 32) seen32 = 1;
        if (CASES[i].iters == 1) seen1 = 1;
    }
    n += RUNS * (seen64 + seen32 + seen1);
    return n;
}

static void print_plan(void) {
    int n = plan_launches();
    unsigned upper_s = (unsigned)((n * 50 + 999) / 1000);
    printf("latmeas plan: %d cases, %d runs, sweep 1..15 (HMMA 1..36), %d launches\n", NCASES, RUNS, n);
    printf("estimated chip window: %u s upper bound (%d launches; 50 ms each covers a cold shader invalidate). Usually shorter.\n",
           upper_s, n);
    printf("The runner wraps only the launch with: flock /tmp/aien-gb10.lock timeout 600\n");
    printf("One warp, 32 threads. Clock: CS2R idx 0x50. Mesa %s\n", MESA_COMMIT);
}

static int mkdir_p(const char *dir);

static int emit_kernels(const char *dir) {
    if (mkdir_p(dir) != 0) return -1;
    char path[512], manp[512];
    snprintf(manp, sizeof manp, "%s/manifest.txt", dir);
    FILE *man = fopen(manp, "w");
    if (!man) return -1;
    uint8_t code[CODE_CAP];
    for (int ci = 0; ci < NCASES; ci++) {
        uint32_t expv = 0, auxv = 1;
        const char *mode = CASES[ci].href ? "ref" : "abs";
        if (!CASES[ci].href && expect_abs(&CASES[ci], &expv, &auxv) != 0) {
            fclose(man);
            return -1;
        }
        for (unsigned s = 1; s <= CASES[ci].sweep; s++) {
            size_t len = 0;
            uint32_t gpr = 0;
            if (build(&CASES[ci], s, 0, 0, code, sizeof code, &len, &gpr) != 0) {
                fprintf(stderr, "emit %s stall %u failed\n", CASES[ci].id, s);
                fclose(man);
                return -1;
            }
            snprintf(path, sizeof path, "%s/%s_%u.bin", dir, CASES[ci].id, s);
            FILE *f = fopen(path, "wb");
            if (!f || fwrite(code, 1, len, f) != len) {
                if (f) fclose(f);
                fclose(man);
                return -1;
            }
            fclose(f);
            fprintf(man, "%s %u %s %u %u %u %u %u %zu\n", CASES[ci].id, s, mode, expv, auxv, CASES[ci].iters,
                    CASES[ci].ops, gpr, len);
        }
    }
    unsigned ovh_iters[8];
    int novh = 0;
    for (int ci = 0; ci < NCASES; ci++) {
        int seen = 0;
        for (int j = 0; j < novh; j++)
            if (ovh_iters[j] == CASES[ci].iters) seen = 1;
        if (!seen && novh < 8) ovh_iters[novh++] = CASES[ci].iters;
    }
    for (int k = 0; k < novh; k++) {
        size_t len = 0;
        uint32_t gpr = 0;
        if (build(NULL, 0, ovh_iters[k], 1, code, sizeof code, &len, &gpr) != 0) {
            fclose(man);
            return -1;
        }
        snprintf(path, sizeof path, "%s/ovh%u_0.bin", dir, ovh_iters[k]);
        FILE *f = fopen(path, "wb");
        if (!f || fwrite(code, 1, len, f) != len) {
            if (f) fclose(f);
            fclose(man);
            return -1;
        }
        fclose(f);
        fprintf(man, "ovh%u 0 abs %u 1 %u 0 %u %zu\n", ovh_iters[k], ovh_iters[k], ovh_iters[k], gpr, len);
    }
    if (fclose(man) != 0) return -1;
    printf("emitted kernels in %s (%d launches when run)\n", dir, plan_launches());
    return 0;
}

static int mkdir_p(const char *dir) {
    if (!dir || !dir[0]) return -1;
    if (mkdir(dir, 0755) == 0 || errno == EEXIST) return 0;
    return -1;
}

static int cmp_u64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a, y = *(const uint64_t *)b;
    return (x > y) - (x < y);
}
static uint64_t median_spread(uint64_t *v, int n, uint64_t *spread) {
    if (n <= 0) {
        *spread = 0;
        return 0;
    }
    qsort(v, (size_t)n, sizeof *v, cmp_u64);
    *spread = v[n - 1] - v[0];
    return v[n / 2];
}

typedef struct {
    char id[64];
    unsigned stall, iters, ops, gpr;
    char mode[8];
    uint32_t expect, aux;
    size_t nbytes;
    uint8_t *code;
} Kern;

static Kern *load_manifest(const char *dir, int *nout) {
    char manp[512];
    snprintf(manp, sizeof manp, "%s/manifest.txt", dir);
    FILE *man = fopen(manp, "r");
    if (!man) return NULL;
    int cap = 256, n = 0;
    Kern *k = calloc((size_t)cap, sizeof *k);
    if (!k) {
        fclose(man);
        return NULL;
    }
    while (1) {
        Kern t;
        memset(&t, 0, sizeof t);
        unsigned stall, expv, auxv, iters, ops, gpr;
        size_t nbytes = 0;
        int got = fscanf(man, "%63s %u %7s %u %u %u %u %u %zu\n", t.id, &stall, t.mode, &expv, &auxv, &iters, &ops,
                         &gpr, &nbytes);
        if (got != 9) break;
        if (n == cap) {
            cap *= 2;
            Kern *g = realloc(k, (size_t)cap * sizeof *k);
            if (!g) {
                free(k);
                fclose(man);
                return NULL;
            }
            k = g;
        }
        t.stall = stall;
        t.expect = expv;
        t.aux = auxv;
        t.iters = iters;
        t.ops = ops;
        t.gpr = gpr;
        t.nbytes = nbytes;
        char path[512];
        snprintf(path, sizeof path, "%s/%s_%u.bin", dir, t.id, t.stall);
        FILE *f = fopen(path, "rb");
        t.code = f ? malloc(nbytes ? nbytes : 1) : NULL;
        if (!f || !t.code || fread(t.code, 1, nbytes, f) != nbytes) {
            if (f) fclose(f);
            free(t.code);
            free(k);
            fclose(man);
            return NULL;
        }
        fclose(f);
        k[n++] = t;
    }
    fclose(man);
    *nout = n;
    return k;
}

static Kern *find_kern(Kern *k, int n, const char *id, unsigned stall) {
    for (int i = 0; i < n; i++)
        if (!strcmp(k[i].id, id) && k[i].stall == stall) return &k[i];
    return NULL;
}

static void read_driver(char *dst, size_t n) {
    dst[0] = 0;
    FILE *f = fopen("/proc/driver/nvidia/version", "r");
    if (!f) return;
    if (!fgets(dst, (int)n, f)) dst[0] = 0;
    fclose(f);
    size_t L = strlen(dst);
    while (L && (dst[L - 1] == '\n' || dst[L - 1] == '\r')) dst[--L] = 0;
}

/* mem holds LANE_N * LANE_WORDS words. Cycles come from lane 0's two clocks.
 * Returns -3 when a lane's id slot is not that lane (the store address is wrong). */
static int launch_kern(const Kern *k, OmegaGpuMem *code, OmegaGpuMem *buf, uint32_t *mem, uint64_t *cycles) {
    if (k->nbytes > code->size) return -1;
    memset(code->cpu, 0, code->size);
    memcpy(code->cpu, k->code, k->nbytes);
    uint32_t *w = buf->cpu;
    for (int i = 0; i < LANE_N * LANE_WORDS; i++) w[i] = POISON;
    if (omega_gpu_session_sync_to_gpu(code) || omega_gpu_session_sync_to_gpu(buf)) return -1;
    uint32_t args[2] = {(uint32_t)buf->va, (uint32_t)(buf->va >> 32)};
    OmegaGpuLaunch L = {.code_va = code->va,
                        .gpr_count = k->gpr,
                        .threads_x = 32,
                        .threads_y = 1,
                        .grid_x = 1,
                        .grid_y = 1,
                        .num_elements = 4,
                        .args = args,
                        .n_args = 2,
                        .timeout_ms = 3000,
                        .spin_us = 200000,
                        .target_sm = 121};
    if (omega_gpu_session_launch(&L, NULL, NULL) != 0) return -1;
    if (omega_gpu_session_sync_to_cpu(buf) != 0) return -1;
    memcpy(mem, buf->cpu, sizeof(uint32_t) * LANE_N * LANE_WORDS);
    for (int lane = 0; lane < LANE_N; lane++)
        if (mem[(size_t)lane * LANE_WORDS + 6] != (uint32_t)lane) return -3;
    uint32_t t0 = mem[0], t1 = mem[1];
    if (t0 == POISON && t1 == POISON) return -2;
    *cycles = t1 >= t0 ? (uint64_t)t1 - t0 : (1ull << 32) + t1 - t0;
    return 0;
}

/* Host oracle for the four stored result words (lane record words 2..5).
 * HMMA: d0..d3, each the 16-term ones dot. Other cases: result, aux, 0, 0. */
static void oracle_words(const CaseDef *c, uint32_t o[4]) {
    uint32_t r = 0, a = 0;
    o[0] = o[1] = o[2] = o[3] = 0;
    if (!c || expect_abs(c, &r, &a) != 0) return;
    int four = !strcmp(c->id, "hmma_reader");
    o[0] = r;
    o[1] = a;
    o[2] = four ? r : 0;
    o[3] = four ? r : 0;
}

typedef struct {
    int run; /* -1 when this stall never mismatched */
    int lane;
    int word; /* 2..5, the lane-record index */
    uint32_t got, want;
} LaneMiss;

/* How many lanes match o[0..3] in words 2..5. The first failure, if any, is written to *miss (lane stays -1 when every lane matches). */
static int lanes_report(const uint32_t *mem, const uint32_t o[4], LaneMiss *miss) {
    int n = 0;
    miss->run = -1;
    miss->lane = -1;
    miss->word = 0;
    miss->got = miss->want = 0;
    for (int lane = 0; lane < LANE_N; lane++) {
        const uint32_t *w = mem + (size_t)lane * LANE_WORDS;
        int bad = 0;
        for (int k = 0; k < 4; k++) {
            if (w[2 + k] == o[k]) continue;
            if (miss->lane < 0) {
                miss->lane = lane;
                miss->word = 2 + k;
                miss->got = w[2 + k];
                miss->want = o[k];
            }
            bad = 1;
        }
        if (!bad) n++;
    }
    return n;
}

static uint32_t lane_fail_bits(const uint32_t *mem, const uint32_t o[4]) {
    uint32_t mask = 0;
    for (int lane = 0; lane < LANE_N; lane++) {
        const uint32_t *w = mem + (size_t)lane * LANE_WORDS;
        for (int k = 0; k < 4; k++)
            if (w[2 + k] != o[k]) {
                mask |= 1u << lane;
                break;
            }
    }
    return mask;
}

static int mem_match(const uint32_t *mem, uint32_t result, uint32_t aux, int four) {
    uint32_t o[4] = {result, aux, four ? result : 0u, four ? result : 0u};
    LaneMiss m;
    return lanes_report(mem, o, &m) == LANE_N;
}

static void fprint_lane_fields(FILE *jf, const uint32_t o[4], const uint8_t *matched_min, const uint32_t *fail_mask,
                               const LaneMiss *miss, int sweep, int have) {
    fprintf(jf, "      \"oracle\": [%u, %u, %u, %u],\n", o[0], o[1], o[2], o[3]);
    fprintf(jf, "      \"reference_result\": %u,\n", o[0]);
    fprintf(jf, "      \"lanes_matched_min\": [");
    for (int s = 0; s < sweep; s++) {
        if (!have || matched_min[s] == 0xff)
            fprintf(jf, "%snull", s ? ", " : "");
        else
            fprintf(jf, "%s%u", s ? ", " : "", matched_min[s]);
    }
    fprintf(jf, "],\n      \"lane_fail_mask\": [");
    for (int s = 0; s < sweep; s++) {
        if (!have || matched_min[s] == 0xff)
            fprintf(jf, "%snull", s ? ", " : "");
        else
            fprintf(jf, "%s\"0x%08x\"", s ? ", " : "", fail_mask[s]);
    }
    fprintf(jf, "],\n      \"first_mismatch\": [");
    for (int s = 0; s < sweep; s++) {
        if (!have || matched_min[s] == 0xff || miss[s].lane < 0)
            fprintf(jf, "%snull", s ? ", " : "");
        else
            fprintf(jf,
                    "%s{\"run\": %d, \"lane\": %d, \"word\": %d, \"got\": %u, \"want\": %u}",
                    s ? ", " : "", miss[s].run, miss[s].lane, miss[s].word, miss[s].got, miss[s].want);
    }
    fprintf(jf, "],\n");
}

static int lane_field_self_test(void) {
    int failed = 0;
#define LCHECK(cond, ...)                                                                                              \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            failed++;                                                                                                  \
            printf("FAIL: ");                                                                                          \
            printf(__VA_ARGS__);                                                                                       \
            printf("\n");                                                                                              \
        }                                                                                                              \
    } while (0)
    uint32_t o[4];
    oracle_words(&CASES[6], o);
    LCHECK(o[0] == hmma_16816_ones() && o[1] == o[0] && o[2] == o[0] && o[3] == o[0],
           "hmma oracle is four destination words");
    oracle_words(&CASES[0], o);
    LCHECK(o[0] == (N_FIXED * (N_FIXED + 1u)) / 2u && o[1] == 1u && o[2] == 0u && o[3] == 0u, "alu oracle words");

    uint32_t mem[LANE_N * LANE_WORDS];
    memset(mem, 0, sizeof mem);
    for (int lane = 0; lane < LANE_N; lane++) {
        mem[(size_t)lane * LANE_WORDS + 2] = o[0];
        mem[(size_t)lane * LANE_WORDS + 3] = o[1];
        mem[(size_t)lane * LANE_WORDS + 4] = o[2];
        mem[(size_t)lane * LANE_WORDS + 5] = o[3];
        mem[(size_t)lane * LANE_WORDS + 6] = (uint32_t)lane;
    }
    LaneMiss m;
    LCHECK(lanes_report(mem, o, &m) == LANE_N && m.lane < 0, "all lanes match the oracle");
    mem[7u * LANE_WORDS + 4] = 0x11111111u;
    LCHECK(lanes_report(mem, o, &m) == LANE_N - 1 && m.lane == 7 && m.word == 4 && m.got == 0x11111111u && m.want == o[2],
           "a planted word-4 mismatch names lane 7");
    LCHECK(lane_fail_bits(mem, o) == (1u << 7), "fail mask is lane 7");

    uint8_t mins[2] = {31, 32};
    uint32_t masks[2] = {1u << 7, 0};
    LaneMiss ms[2];
    ms[0] = m;
    ms[0].run = 3;
    ms[1].lane = -1;
    char *text = NULL;
    size_t tlen = 0;
    FILE *mf = open_memstream(&text, &tlen);
    if (mf) {
        fprint_lane_fields(mf, o, mins, masks, ms, 2, 1);
        fclose(mf);
    }
    LCHECK(text && strstr(text, "\"oracle\": [") && strstr(text, "\"lanes_matched_min\": [31, 32]") &&
               strstr(text, "\"lane_fail_mask\"") && strstr(text, "\"lane\": 7") && strstr(text, "\"word\": 4") &&
               strstr(text, "\"run\": 3") && strstr(text, ", null]"),
           "json lane fields: %s", text ? text : "(none)");
    free(text);

    uint8_t code[CODE_CAP];
    size_t len = 0;
    uint32_t gpr = 0, lane0[LANE_WORDS];
    LCHECK(build(&CASES[0], CASES[0].sweep, 0, 0, code, sizeof code, &len, &gpr) == 0, "alu top encode");
    LCHECK(lat_sim(lane0) == 0, "alu top sim");
    oracle_words(&CASES[0], o);
    LCHECK(lanes_report(g_sim_mem, o, &m) == LANE_N && m.lane < 0, "sim stall 15 matches on every lane");
    LCHECK(build(&CASES[0], 1, 0, 0, code, sizeof code, &len, &gpr) == 0, "alu stall 1 encode");
    LCHECK(lat_sim(lane0) == 0, "alu stall 1 sim");
    LCHECK(lanes_report(g_sim_mem, o, &m) == 0 && m.lane == 0 && m.word == 2, "sim stall 1 fails word 2 on lane 0");
    oracle_words(&CASES[6], o);
    LCHECK(build(&CASES[6], 1, 0, 0, code, sizeof code, &len, &gpr) == 0, "hmma stall 1 encode");
    LCHECK(lat_sim(lane0) == 0, "hmma stall 1 sim");
    LCHECK(lanes_report(g_sim_mem, o, &m) == 0 && m.word >= 2 && m.word <= 5, "hmma stall 1 fails a destination word");
#undef LCHECK
    return failed;
}

static void json_escape(const char *s, char *dst, size_t n) {
    size_t j = 0;
    if (!s) s = "";
    for (size_t i = 0; s[i] && j + 2 < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '"' || c == '\\') {
            if (j + 3 >= n) break;
            dst[j++] = '\\';
        }
        if (c < 32) c = ' ';
        dst[j++] = (char)c;
    }
    dst[j] = 0;
}

static int run_launch(const char *kdir, const char *out_path) {
    int nk = 0;
    Kern *kerns = load_manifest(kdir, &nk);
    if (!kerns || nk < 1) {
        fprintf(stderr, "no kernels in %s (run --emit first, outside the lock)\n", kdir);
        return 2;
    }
    print_plan();
    char driver[256];
    read_driver(driver, sizeof driver);
    struct utsname ut;
    if (uname(&ut) != 0) memset(&ut, 0, sizeof ut);

    int pass[NCASES][RUNS][SWEEP_MAX];
    uint64_t cyc[NCASES][RUNS][SWEEP_MAX];
    uint64_t ovh_cyc[8][RUNS];
    uint32_t run0_res[NCASES][SWEEP_MAX];
    uint8_t lane_min[NCASES][SWEEP_MAX];
    uint32_t fail_mask[NCASES][SWEEP_MAX];
    LaneMiss first_miss[NCASES][SWEEP_MAX];
    memset(pass, 0, sizeof pass);
    memset(cyc, 0, sizeof cyc);
    memset(ovh_cyc, 0, sizeof ovh_cyc);
    memset(run0_res, 0, sizeof run0_res);
    memset(lane_min, 0xff, sizeof lane_min);
    memset(fail_mask, 0, sizeof fail_mask);
    for (int ci = 0; ci < NCASES; ci++)
        for (int s = 0; s < SWEEP_MAX; s++) first_miss[ci][s].lane = -1;
    char notes[NCASES][384];
    memset(notes, 0, sizeof notes);
    unsigned ovh_list[8];
    int novh = 0;
    for (int ci = 0; ci < NCASES && novh < 8; ci++) {
        int seen = 0;
        for (int j = 0; j < novh; j++)
            if (ovh_list[j] == CASES[ci].iters) seen = 1;
        if (!seen) ovh_list[novh++] = CASES[ci].iters;
    }
    int device_error = 0;
    const char *err = "";

    omega_gpu_session_lock();
    if (!omega_gpu_session_open()) {
        err = omega_gpu_session_last_error();
        fprintf(stderr, "GPU not available (%s)\n", err);
        device_error = 1;
        goto write_json;
    }
    if (omega_gpu_session_target_sm() != 121) {
        err = "opened device is not sm_121";
        fprintf(stderr, "%s (%u)\n", err, omega_gpu_session_target_sm());
        device_error = 1;
        goto write_json;
    }
    OmegaGpuMem code = {0}, buf = {0};
    if (omega_gpu_session_alloc_code(CODE_CAP, &code) != 0 || omega_gpu_session_alloc(4096, &buf) != 0) {
        err = omega_gpu_session_last_error();
        device_error = 1;
        goto write_json;
    }

    for (int oi = 0; oi < novh && !device_error; oi++) {
        char id[16];
        snprintf(id, sizeof id, "ovh%u", ovh_list[oi]);
        Kern *k = find_kern(kerns, nk, id, 0);
        if (!k) {
            err = "missing overhead kernel";
            device_error = 1;
            break;
        }
        for (int r = 0; r < RUNS; r++) {
            uint32_t mem[LANE_N * LANE_WORDS];
            uint64_t cy = 0;
            int rc = launch_kern(k, &code, &buf, mem, &cy);
            printf("overhead %s run %d/%d rc %d\n", id, r + 1, RUNS, rc);
            fflush(stdout);
            if (rc != 0) {
                err = rc == -3 ? "lane id was not stored" : omega_gpu_session_last_error();
                device_error = 1;
                break;
            }
            ovh_cyc[oi][r] = cy;
            if (!mem_match(mem, ovh_list[oi], 1u, 0)) err = "overhead loop count did not match";
        }
    }

    for (int ci = 0; ci < NCASES && !device_error; ci++) {
        const CaseDef *c = &CASES[ci];
        for (int r = 0; r < RUNS && !device_error; r++) {
            printf("case %s run %d/%d\n", c->id, r + 1, RUNS);
            fflush(stdout);
            for (unsigned s = 1; s <= c->sweep && !device_error; s++) {
                Kern *k = find_kern(kerns, nk, c->id, s);
                uint32_t mem[LANE_N * LANE_WORDS];
                uint64_t cy = 0;
                int rc = !k ? -1 : launch_kern(k, &code, &buf, mem, &cy);
                if (rc != 0) {
                    err = rc == -3 ? "lane id was not stored" : (k ? omega_gpu_session_last_error() : "missing kernel");
                    device_error = 1;
                    break;
                }
                cyc[ci][r][s - 1] = cy;
                uint32_t oracle[4];
                oracle_words(c, oracle);
                LaneMiss one;
                int nmatch = lanes_report(mem, oracle, &one);
                if (lane_min[ci][s - 1] == 0xff || nmatch < lane_min[ci][s - 1]) lane_min[ci][s - 1] = (uint8_t)nmatch;
                fail_mask[ci][s - 1] |= lane_fail_bits(mem, oracle);
                if (nmatch < LANE_N && first_miss[ci][s - 1].lane < 0) {
                    one.run = r;
                    first_miss[ci][s - 1] = one;
                }
                pass[ci][r][s - 1] = nmatch == LANE_N;
                if (r == 0) run0_res[ci][s - 1] = mem[2];
            }
        }
    }

    omega_gpu_session_free(&code);
    omega_gpu_session_free(&buf);

write_json:;
    int locked = 1;
    char tmp[512];
    snprintf(tmp, sizeof tmp, "%s.tmp", out_path);
    FILE *jf = fopen(tmp, "w");
    if (!jf) {
        if (locked) {
            omega_gpu_session_unlock();
            omega_gpu_session_close();
        }
        free(kerns);
        return 1;
    }
    char err_esc[512], drv_esc[512], mach_esc[128];
    json_escape(err, err_esc, sizeof err_esc);
    json_escape(driver, drv_esc, sizeof drv_esc);
    json_escape(ut.machine, mach_esc, sizeof mach_esc);
    fprintf(jf,
            "{\n  \"kind\": \"cudapp-latmeas\",\n  \"synthetic\": false,\n  \"measured\": %s,\n  \"mesa_commit\": \"%s\",\n"
            "  \"driver\": \"%s\",\n  \"machine\": \"%s\",\n  \"runs\": %d,\n  \"threads\": 32,\n  \"warps\": 1,\n"
            "  \"clock\": \"CS2R idx 0x50 (NAK_SV_CLOCK_LO)\",\n  \"launches\": %d,\n  \"error\": \"%s\",\n  \"cases\": {\n",
            device_error ? "false" : "true", MESA_COMMIT, drv_esc, mach_esc, RUNS, plan_launches(), err_esc);
    for (int ci = 0; ci < NCASES; ci++) {
        const CaseDef *c = &CASES[ci];
        int req = device_error ? -1 : reported_threshold(pass[ci], RUNS, (int)c->sweep);
        int spread_s = 0;
        int smin = 1000, smax = -1, nthr = 0;
        if (!device_error) {
            for (int r = 0; r < RUNS; r++) {
                int one[1][SWEEP_MAX];
                memcpy(one[0], pass[ci][r], sizeof one[0]);
                int row = reported_threshold(one, 1, (int)c->sweep);
                if (row > 1) {
                    if (row < smin) smin = row;
                    if (row > smax) smax = row;
                    nthr++;
                }
            }
            if (nthr == RUNS) spread_s = smax - smin;
        }
        if (!device_error && req < 0 && notes[ci][0] == 0) {
            int any_fail = 0;
            for (unsigned s = 1; s <= c->sweep && !any_fail; s++)
                for (int r = 0; r < RUNS; r++)
                    if (!pass[ci][r][s - 1]) any_fail = 1;
            snprintf(notes[ci], sizeof notes[ci],
                     any_fail ? "no stall was correct through the top of the sweep"
                              : "every stall matched the oracle; threshold is at or below 1");
        }
        uint64_t samples[RUNS];
        uint64_t cmed = 0, cspread = 0;
        int have_cyc = 0;
        if (req > 0) {
            for (int r = 0; r < RUNS; r++) samples[r] = cyc[ci][r][req - 1];
            uint64_t copy[RUNS];
            memcpy(copy, samples, sizeof copy);
            cmed = median_spread(copy, RUNS, &cspread);
            have_cyc = 1;
            for (int r = 0; r < RUNS; r++)
                if (samples[r] == 0) have_cyc = 0;
        }
        int oi = 0;
        for (int k = 0; k < novh; k++)
            if (ovh_list[k] == c->iters) oi = k;
        uint64_t oc[RUNS];
        memcpy(oc, ovh_cyc[oi], sizeof oc);
        uint64_t ospread = 0;
        uint64_t omed = median_spread(oc, RUNS, &ospread);
        (void)ospread;
        uint64_t s1[RUNS], s1copy[RUNS], s1med = 0, s1spread = 0;
        int have_s1 = !device_error;
        for (int r = 0; r < RUNS; r++) {
            s1[r] = cyc[ci][r][0];
            if (s1[r] == 0) have_s1 = 0;
        }
        if (have_s1) {
            memcpy(s1copy, s1, sizeof s1);
            s1med = median_spread(s1copy, RUNS, &s1spread);
        }
        char note_esc[512];
        json_escape(notes[ci], note_esc, sizeof note_esc);
        fprintf(jf, "    \"%s\": {\n", c->id);
        if (req > 0 && !device_error)
            fprintf(jf, "      \"stall_required\": %d,\n", req);
        else
            fprintf(jf, "      \"stall_required\": null,\n");
        fprintf(jf, "      \"stall_spread\": %d,\n", req > 0 && !device_error ? spread_s : 0);
        if (have_cyc && !device_error) {
            fprintf(jf, "      \"cycle_median\": %llu,\n      \"cycle_spread\": %llu,\n", (unsigned long long)cmed,
                    (unsigned long long)cspread);
        } else {
            fprintf(jf, "      \"cycle_median\": null,\n      \"cycle_spread\": null,\n");
        }
        if (have_s1) {
            fprintf(jf, "      \"cycle_median_stall1\": %llu,\n      \"cycle_spread_stall1\": %llu,\n",
                    (unsigned long long)s1med, (unsigned long long)s1spread);
        } else {
            fprintf(jf, "      \"cycle_median_stall1\": null,\n      \"cycle_spread_stall1\": null,\n");
        }
        if (device_error)
            fprintf(jf, "      \"overhead_cycle_median\": null,\n");
        else
            fprintf(jf, "      \"overhead_cycle_median\": %llu,\n", (unsigned long long)omed);
        {
            uint32_t oracle[4];
            oracle_words(c, oracle);
            fprint_lane_fields(jf, oracle, lane_min[ci], fail_mask[ci], first_miss[ci], (int)c->sweep, !device_error);
        }
        fprintf(jf, "      \"run0_result\": [");
        for (unsigned s = 1; s <= c->sweep; s++)
            fprintf(jf, "%s%u", s == 1 ? "" : ", ", run0_res[ci][s - 1]);
        fprintf(jf, "],\n");
        fprintf(jf, "      \"iters\": %u,\n      \"ops\": %u,\n", c->iters, c->ops);
        fprintf(jf, "      \"note\": \"%s\"\n    }%s\n", note_esc, ci + 1 == NCASES ? "" : ",");
    }
    fprintf(jf, "  }\n}\n");
    if (fclose(jf) != 0) {
        omega_gpu_session_unlock();
        omega_gpu_session_close();
        free(kerns);
        return 1;
    }
    if (rename(tmp, out_path) != 0) {
        omega_gpu_session_unlock();
        omega_gpu_session_close();
        free(kerns);
        return 1;
    }
    omega_gpu_session_unlock();
    omega_gpu_session_close();
    for (int i = 0; i < nk; i++) free(kerns[i].code);
    free(kerns);
    printf("wrote %s\n", out_path);
    return device_error ? 1 : 0;
}

static void usage(void) {
    fprintf(stderr,
            "usage:\n"
            "  gpu_latmeas_test --self-test\n"
            "  gpu_latmeas_test --print-plan\n"
            "  gpu_latmeas_test --emit DIR\n"
            "  gpu_latmeas_test --launch DIR --out raw.json\n");
}

int main(int argc, char **argv) {
    const char *emit = NULL, *launch = NULL, *out = NULL;
    int self = 0, plan = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--self-test")) self = 1;
        else if (!strcmp(argv[i], "--print-plan")) plan = 1;
        else if (!strcmp(argv[i], "--emit") && i + 1 < argc) emit = argv[++i];
        else if (!strcmp(argv[i], "--launch") && i + 1 < argc) launch = argv[++i];
        else if (!strcmp(argv[i], "--out") && i + 1 < argc) out = argv[++i];
        else {
            usage();
            return 2;
        }
    }
    if (self) {
        int f = self_test();
        f += self_test_suffix();
        return f ? 1 : 0;
    }
    if (plan && !emit && !launch) {
        print_plan();
        return 0;
    }
    if (emit && !launch) return emit_kernels(emit) == 0 ? 0 : 1;
    if (launch && out) return run_launch(launch, out);
    usage();
    return 2;
}
