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
 * One warp, 32 threads, uniform control flow inside the timed region. Only
 * lane 0 stores. Mesa commit and the NAK line citations live in
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
    {"hmma_reader", HMMA_ITERS, 1, SWEEP_HMMA, HMMA_ITERS, 1},
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

static void s2r_sb(B *b, int dst, uint32_t sr) {
    uint32_t wait = b->pending;
    add(b, BW_IR_S2R, dst, NOREG, NOREG, NOREG, sr, cw(4, SB_GPR, SB_NONE, wait, 1), 0, 0, 0, 0);
    b->pending = 1u << SB_GPR;
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

static void store_tail(B *b, int ptr, int t0, int t1, int result, int rsub, int aux, int asub) {
    int tid = V(b);
    int one = V(b);
    movi(b, one, 1, 0);
    s2r_sb(b, tid, BW_SR_TID_X);
    isetp_ge(b, tid, one, 15); /* P0 = (tid >= 1); lane 0 falls through */
    int skip_br = bra(b, 1, 0, 5);
    uint32_t st = cw(4, SB_NONE, SB_RD, 0, 1);
    add(b, BW_IR_STG_E, NOREG, ptr, t0, NOREG, 0, st, 0, 0, 0, 0);
    add(b, BW_IR_STG_E, NOREG, ptr, t1, NOREG, 4, st, 0, 0, 0, 0);
    add(b, BW_IR_STG_E, NOREG, ptr, result, NOREG, 8, st, 0, 0, 0, rsub);
    add(b, BW_IR_STG_E, NOREG, ptr, aux, NOREG, 12, st, 0, 0, 0, asub);
    int skip = (int)b->p->count;
    patch_bra(b, skip_br, skip);
    add(b, BW_IR_EXIT, NOREG, NOREG, NOREG, NOREG, 0, cw(5, SB_NONE, SB_NONE, 0, 0), 0, 0, 0, 0);
    add(b, BW_IR_BRA, NOREG, NOREG, NOREG, NOREG, 0, 0x000fc000u, 0, 0, 0, 0);
}

static void loop_ctl(B *b, int i, int nreg, int one, int *loop_back) {
    iadd(b, i, i, one, 1, 0, 0, 0);
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
        fadd(&b, acc, dquad, 0, zero, 15); /* acc = d0 + 0 */
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
    if (!overhead && !strcmp(c->id, "hmma_reader")) {
        result = acc;
        auxv = dquad; /* late read of d1, after the timed region: diagnostic only */
        asub = 1;
    }
    if (overhead) result = i;
    store_tail(&b, ptr, t0, t1, result, rsub, auxv, asub);
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
        CHECK(expect_abs(&CASES[6], &er, &ea) != 0, "hmma has no closed host formula");
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
        CHECK(s2r_tid == 1, "%s lane filter %d", CASES[ci].id, s2r_tid);
        char *text = NULL;
        size_t tlen = 0;
        FILE *mf = open_memstream(&text, &tlen);
        int unk = mf ? omega_sm121_listing(code, len, mf) : -1;
        if (mf) fclose(mf);
        CHECK(unk == 2, "%s disasm unknowns %d (the two CS2R sites)", CASES[ci].id, unk);
        failed += check_raw_window(CASES[ci].id, code, ninsn, stall);
        if (!strcmp(CASES[ci].id, "hmma_reader")) {
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

static int launch_kern(const Kern *k, OmegaGpuMem *code, OmegaGpuMem *buf, uint32_t outw[4], uint64_t *cycles) {
    if (k->nbytes > code->size) return -1;
    memset(code->cpu, 0, code->size);
    memcpy(code->cpu, k->code, k->nbytes);
    uint32_t *w = buf->cpu;
    for (int i = 0; i < 4; i++) w[i] = POISON;
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
    memcpy(outw, buf->cpu, 16);
    uint32_t t0 = outw[0], t1 = outw[1];
    if (t0 == POISON && t1 == POISON && outw[2] == POISON && outw[3] == POISON) return -2;
    *cycles = t1 >= t0 ? (uint64_t)t1 - t0 : (1ull << 32) + t1 - t0;
    return 0;
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
    uint32_t href[NCASES][RUNS];
    int href_ok[NCASES][RUNS];
    uint64_t ovh_cyc[8][RUNS];
    uint32_t run0_res[NCASES][SWEEP_MAX];
    uint32_t run0_ref[NCASES];
    memset(pass, 0, sizeof pass);
    memset(cyc, 0, sizeof cyc);
    memset(href, 0, sizeof href);
    memset(href_ok, 0, sizeof href_ok);
    memset(ovh_cyc, 0, sizeof ovh_cyc);
    memset(run0_res, 0, sizeof run0_res);
    memset(run0_ref, 0, sizeof run0_ref);
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
            uint32_t w[4];
            uint64_t cy = 0;
            int rc = launch_kern(k, &code, &buf, w, &cy);
            printf("overhead %s run %d/%d rc %d\n", id, r + 1, RUNS, rc);
            fflush(stdout);
            if (rc != 0) {
                err = omega_gpu_session_last_error();
                device_error = 1;
                break;
            }
            ovh_cyc[oi][r] = cy;
            if (w[2] != ovh_list[oi] || w[3] != 1) err = "overhead loop count did not match";
        }
    }

    for (int ci = 0; ci < NCASES && !device_error; ci++) {
        const CaseDef *c = &CASES[ci];
        for (int r = 0; r < RUNS && !device_error; r++) {
            printf("case %s run %d/%d\n", c->id, r + 1, RUNS);
            fflush(stdout);
            uint32_t refv = 0;
            int have_ref = 0;
            if (c->href) {
                Kern *kr = find_kern(kerns, nk, c->id, c->sweep);
                uint32_t w[4];
                uint64_t cy = 0;
                if (!kr || launch_kern(kr, &code, &buf, w, &cy) != 0) {
                    err = omega_gpu_session_last_error();
                    device_error = 1;
                    break;
                }
                refv = w[2];
                have_ref = (w[3] != POISON && refv != 0 && refv != POISON);
                href[ci][r] = refv;
                href_ok[ci][r] = have_ref;
                cyc[ci][r][c->sweep - 1] = cy;
                pass[ci][r][c->sweep - 1] = have_ref;
                if (r == 0) {
                    run0_ref[ci] = refv;
                    run0_res[ci][c->sweep - 1] = refv;
                }
                if (!have_ref)
                    snprintf(notes[ci], sizeof notes[ci],
                             "HMMA reference at stall %u is 0x%08x (aux 0x%08x); d0 did not move", c->sweep, w[2], w[3]);
            }
            for (unsigned s = 1; s <= c->sweep && !device_error; s++) {
                if (c->href && s == c->sweep) continue;
                Kern *k = find_kern(kerns, nk, c->id, s);
                uint32_t w[4];
                uint64_t cy = 0;
                if (!k || launch_kern(k, &code, &buf, w, &cy) != 0) {
                    err = k ? omega_gpu_session_last_error() : "missing kernel";
                    device_error = 1;
                    break;
                }
                cyc[ci][r][s - 1] = cy;
                int ok = 0;
                if (c->href) {
                    ok = href_ok[ci][r] && w[2] == refv;
                } else {
                    uint32_t expv = 0, auxv = 0;
                    if (expect_abs(c, &expv, &auxv) == 0) ok = (w[2] == expv && w[3] == auxv && w[0] != POISON);
                }
                pass[ci][r][s - 1] = ok;
                if (r == 0) run0_res[ci][s - 1] = w[2];
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
        if (c->href && !device_error)
            fprintf(jf, "      \"reference_result\": %u,\n", run0_ref[ci]);
        else
            fprintf(jf, "      \"reference_result\": null,\n");
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
