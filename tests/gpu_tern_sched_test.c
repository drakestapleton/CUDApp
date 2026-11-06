/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* gpu_tern_sched_test: the I2FP consumer rule of the ternary GEMV generator (Sass#4, 2026-10-27). Host only, no device.
 *
 * The Sass#4 chip sweep (lat-20261027T025808Z) measured i2fp->fmul 18 and i2f->fmul 19 cycles, while the generator gave
 * the f32 epilogue's FMUL 6. Sass#4 then put I2FP on scoreboard 0 with the FMUL waiting on it at stall 4; on the GB10
 * that broke the epilogue (omega-mv#924 window 2026-11-01T00:55Z: gpu_tern_test "f32 q|k|v segments" 2040 of 3840 rows
 * off, stall-6 encoding 0). The generator now emits I2FP with no scoreboard, first in the epilogue, and refuses any
 * program in which an I2FP reader sits < OMEGA_TERN_I2FP_LAT cycles behind, waiting on a scoreboard or not
 * (omega_tern_i2fp_violations).
 *
 *  1. every production f32 spec builds, has an I2FP, the I2FP writes no scoreboard and the checker passes it;
 *  2. negative control: mutant 4 (the pre-Sass#4 I2FP, directly before its FMUL at stall 6) is refused for every spec;
 *  3. the checker on hand-built programs: I2FP -> FMUL at stall 6 fails, at 18 fails, at 19 passes, behind a
 *     scoreboard wait at stall 4 fails; an FMUL producer at stall 6 passes (other producers keep their rule).
 *
 * The public generator and checker now use seeded Omega by default. Selecting
 * CUDAPP_MIGRATION_PENDING_C_CODEGEN_TERN=1 in the builder selects its retained
 * C oracle; this same API-level test remains valid for either implementation. */
#include "omega_blackwell_codegen_tern.h"
#include "bw_warp_sim_k.h"
#include <stdio.h>
#ifndef CUDAPP_MIGRATION_PENDING_TERN_SCHED
#define CUDAPP_MIGRATION_PENDING_TERN_SCHED 1 /* legacy test-admission switch, not the builder selector */
#endif
#include <string.h>

#define CW(stall, wbar, rbar, wait) \
    (((uint32_t)(stall) << 9) | ((uint32_t)(wbar) << 14) | ((uint32_t)(rbar) << 17) | ((uint32_t)(wait) << 20))

static int fails;
static void check(int ok, const char *what) { printf("  %-72s %s\n", what, ok ? "ok" : "FAIL"); if (!ok) fails++; }

static BlackwellIRInsn insn(BlackwellIROpcode op, int d, int a, int b, uint32_t control) {
    BlackwellIRInsn i; memset(&i, 0, sizeof i);
    i.op = op; i.dst_vreg = d; i.src1_vreg = a; i.src2_vreg = b; i.src3_vreg = -1; i.ureg = -1; i.control = control;
    return i;
}

/* MOV r0; producer r1 <- r0 (control pc); `pad` independent MOVs (stall 1 each) to r3; FMUL r2 <- r1 * r1 (wait mask
 * cw); STG-free tail EXIT. Returns the checker's count, or -2 when the program cannot be built. */
static long synth(BlackwellIROpcode prod, uint32_t pc, int pad, uint32_t cw) {
    static BlackwellIRProgram p;
    omega_bw_ir_init(&p);
    int r0 = omega_bw_ir_alloc_vreg(&p), r1 = omega_bw_ir_alloc_vreg(&p), r2 = omega_bw_ir_alloc_vreg(&p), r3 = omega_bw_ir_alloc_vreg(&p);
    if (r0 < 0 || r1 < 0 || r2 < 0 || r3 < 0) return -2;
    BlackwellIRInsn m = insn(BW_IR_MOV_IMM, r0, -1, -1, CW(6, 7, 7, 0)); m.imm = 3;
    if (omega_bw_ir_append(&p, &m) < 0) return -2;
    BlackwellIRInsn pr = insn(prod, r1, r0, prod == BW_IR_FMUL ? r0 : -1, pc);
    if (omega_bw_ir_append(&p, &pr) < 0) return -2;
    for (int k = 0; k < pad; k++) { BlackwellIRInsn q = insn(BW_IR_MOV_IMM, r3, -1, -1, CW(1, 7, 7, 0)); q.imm = (uint32_t)k; if (omega_bw_ir_append(&p, &q) < 0) return -2; }
    BlackwellIRInsn f = insn(BW_IR_FMUL, r2, r1, r1, cw);
    if (omega_bw_ir_append(&p, &f) < 0) return -2;
    BlackwellIRInsn g = insn(BW_IR_FMUL, r3, r2, r3, CW(6, 7, 7, 0));
    if (omega_bw_ir_append(&p, &g) < 0) return -2;
    BlackwellIRInsn x = insn(BW_IR_EXIT, -1, -1, -1, CW(5, 7, 7, 0));
    if (omega_bw_ir_append(&p, &x) < 0) return -2;
    if (omega_bw_regalloc_solve(&p) != 0) return -2;
    return omega_tern_i2fp_violations(&p);
}

/* Independent row-major regression: k=512, n=13, rows=2, warps=2,
 * depth=2, grid=2. Row r has r+1 nonzero packed words, alternating sign;
 * x is all ones. No API packer, word-offset helper or algebra oracle is used.
 * Wolfram: Table[16(r+1)(-1)^r,{r,0,12}] gives the expected vector below.
 * Nonzero scalar LDG immediates distinguish adjacent rows and depth steps. */
static void rowmajor_sim(void) {
    static BlackwellIRProgram p;
    const OmegaTernGemvSpec spec = { 512, OMEGA_TERN_LAYOUT_ROWMAJOR, 2, 2, 2, OMEGA_TERN_EPI_I32, 0 };
    static const int32_t expected[13] = { 16, -32, 48, -64, 80, -96, 112, -128, 144, -160, 176, -192, 208 };
    _Alignas(16) uint32_t packed[16][32] = { { 0 } };
    _Alignas(16) int8_t x[512];
    _Alignas(16) uint32_t y[16];
    _Alignas(16) uint8_t smem[4096];
    memset(x, 1, sizeof x);
    for (unsigned r = 0; r < 13; r++)
        for (unsigned c = 0; c <= r; c++)
            packed[r][c] = r & 1u ? 2863311530u : 1431655765u;
    const int built = omega_blackwell_codegen_tern_gemv_prog(&spec, &p) == 0;
    check(built, "5. independent row-major regression builds");
    if (!built) return;
    const uint32_t shared = omega_tern_gemv_shared_bytes(&spec);
    check(shared <= sizeof smem, "5. regression shared-memory buffer fits");
    if (shared > sizeof smem) return;
    for (int order = 0; order < 2; order++) {
        memset(y, 0xa5, sizeof y);
        WsSim s; memset(&s, 0, sizeof s);
        s.p = &p; s.threads = 64; s.order = order;
        s.smem = smem; s.smem_bytes = shared;
        const uint64_t args[3] = { (uintptr_t)packed, (uintptr_t)x, (uintptr_t)y };
        for (unsigned i = 0; i < 3; i++) {
            s.cbank[(OMEGA_TERN_ARG_W + 8u * i) / 4u] = (uint32_t)args[i];
            s.cbank[(OMEGA_TERN_ARG_W + 8u * i) / 4u + 1u] = (uint32_t)(args[i] >> 32);
        }
        s.cbank[OMEGA_TERN_ARG_U / 4] = 4;
        s.cbank[OMEGA_TERN_ARG_STRIDE / 4] = 4;
        s.cbank[OMEGA_TERN_ARG_NEGN / 4] = 0u - 13u;
        s.nranges = 3;
        s.ranges[0].lo = (uintptr_t)packed; s.ranges[0].hi = (uintptr_t)packed + sizeof packed;
        s.ranges[1].lo = (uintptr_t)x; s.ranges[1].hi = (uintptr_t)x + sizeof x;
        s.ranges[2].lo = (uintptr_t)y; s.ranges[2].hi = (uintptr_t)y + sizeof expected;
        const int rc = ws_run(&s, 2, 1);
        unsigned bad = 0;
        for (unsigned r = 0; !rc && r < 13; r++)
            if ((int32_t)y[r] != expected[r]) bad++;
        printf("  row-major independent oracle order %d: rc %d, %u differing rows\n", order, rc, bad);
        if (rc) printf("  simulator: %s (pc %d)\n", s.msg, s.err_pc);
        check(!rc && !bad, "5. scalar LDG row/depth offsets match independent row values");
    }
}

int main(void) {
    if (!CUDAPP_MIGRATION_PENDING_TERN_SCHED) { printf("gpu_tern_sched_test: NOT_RUN (retired: CUDAPP_MIGRATION_PENDING_TERN_SCHED=0)\n"); return 77; }
    printf("gpu_tern_sched_test: I2FP consumer rule (OMEGA_TERN_I2FP_LAT %u)\n", OMEGA_TERN_I2FP_LAT);
    static BlackwellIRProgram prog;
    static const uint32_t ks[] = { 256, 2560, 8192 };
    static const uint32_t shapes[][3] = { { OMEGA_TERN_LAYOUT_ROWMAJOR, 1, 1 }, { OMEGA_TERN_LAYOUT_ROWMAJOR, 2, 1 },
                                          { OMEGA_TERN_LAYOUT_ROWMAJOR, 4, 1 }, { OMEGA_TERN_LAYOUT_BLOCK, 2, 1 },
                                          { OMEGA_TERN_LAYOUT_BLOCK, 4, 1 } };
    int built = 0, refused = 0, specs = 0, wired = 0;
    for (unsigned a = 0; a < sizeof ks / sizeof ks[0]; a++)
        for (unsigned b = 0; b < sizeof shapes / sizeof shapes[0]; b++)
            for (uint32_t w = 1; w <= 32; w *= 4) {
                OmegaTernGemvSpec s = { ks[a], shapes[b][0], shapes[b][1], w, shapes[b][2], OMEGA_TERN_EPI_F32, 0 };
                if (!omega_tern_gemv_spec_ok(&s)) continue;
                specs++;
                if (omega_blackwell_codegen_tern_gemv_prog(&s, &prog) == 0) {
                    built++;
                    int seen = 0, ok = 1;
                    for (size_t i = 0; i < prog.count; i++) {
                        if (prog.insns[i].op != BW_IR_I2FP) continue;
                        seen++;
                        const uint32_t wb = (prog.insns[i].control >> 14) & 7u;
                        if (wb != 7u) ok = 0;
                    }
                    if (seen && ok && omega_tern_i2fp_violations(&prog) == 0) wired++;
                }
                s.mutant = 4;
                if (omega_tern_gemv_spec_ok(&s) && omega_blackwell_codegen_tern_gemv_prog(&s, &prog) != 0) refused++;
            }
    printf("  f32 specs %d: built %d, I2FP without a scoreboard and >= %u cycles ahead of its reader %d, mutant 4 refused %d\n",
           specs, built, OMEGA_TERN_I2FP_LAT, wired, refused);
    check(specs > 0 && built == specs, "1. every production f32 spec builds");
    check(specs > 0 && wired == specs, "1. its I2FP writes no scoreboard and its reader sits >= OMEGA_TERN_I2FP_LAT behind");
    check(specs > 0 && refused == specs, "2. negative control: mutant 4 (I2FP fixed stall 6) refused for every spec");
    OmegaTernGemvSpec i32 = { 2560, OMEGA_TERN_LAYOUT_ROWMAJOR, 2, 4, 1, OMEGA_TERN_EPI_I32, 4 };
    check(omega_blackwell_codegen_tern_gemv_prog(&i32, &prog) == 0, "2. mutant 4 without an I2FP (i32 epilogue) still builds");

    check(synth(BW_IR_I2FP, CW(6, 7, 7, 0), 0, CW(6, 7, 7, 0)) == 1, "3. I2FP -> FMUL at stall 6, no scoreboard: violation");
    check(synth(BW_IR_I2FP, CW(15, 7, 7, 0), 3, CW(6, 7, 7, 0)) == 1, "3. I2FP -> FMUL at 18 cycles, no scoreboard: violation");
    check(synth(BW_IR_I2FP, CW(15, 7, 7, 0), 4, CW(6, 7, 7, 0)) == 0, "3. I2FP -> FMUL at 19 cycles, no scoreboard: pass");
    check(synth(BW_IR_I2FP, CW(4, 0, 1, 0), 0, CW(6, 7, 7, 1u)) == 1, "3. I2FP on scoreboard 0, FMUL waits on it at stall 4: violation");
    check(synth(BW_IR_I2FP, CW(4, 0, 1, 0), 0, CW(6, 7, 7, 2u)) == 1, "3. I2FP on scoreboard 0, FMUL waits on another: violation");
    check(synth(BW_IR_FMUL, CW(6, 7, 7, 0), 0, CW(6, 7, 7, 0)) == 0, "3. FMUL producer -> FMUL at stall 6: pass (not an I2FP)");

    /* Coalesced output from omega-mv #995. The algebra-oracle simulator stays in tests/gpu_tern_test.c (Omega).
     * These checks cover the predicate, the shared-memory size, and the two extra mutants. Mutant 4 remains the
     * I2FP refusal above; omega calls the slot-race mutant 4 and the clamp mutant 5. */
    OmegaTernGemvSpec coal = { 1024, OMEGA_TERN_LAYOUT_ROWMAJOR, 1, 5, 2, OMEGA_TERN_EPI_I32, 0 };
    check(omega_tern_gemv_coalesced_out(&coal) == 1, "4. row-major R=1 W=5 single chunk coalesces");
    check(omega_tern_gemv_shared_bytes(&coal) == coal.k * 2u + 2u * omega_tern_gemv_out_slot_bytes(&coal),
          "4. coalesced shared bytes are the x stage plus two slot sets");
    OmegaTernGemvSpec wide = { 1024, OMEGA_TERN_LAYOUT_BLOCK, 4, 8, 1, OMEGA_TERN_EPI_I32, 0 };
    check(omega_tern_gemv_spec_ok(&wide) && omega_tern_gemv_coalesced_out(&wide) == 0,
          "4. block R=4 W=8 keeps the per-row store");
    OmegaTernGemvSpec chunked = { 16384, OMEGA_TERN_LAYOUT_ROWMAJOR, 1, 1, 1, OMEGA_TERN_EPI_I32, 0 };
    check(omega_tern_gemv_spec_ok(&chunked) && omega_tern_gemv_coalesced_out(&chunked) == 0,
          "4. two chunks keep the per-row store");
    coal.mutant = 5;
    check(omega_tern_gemv_spec_ok(&coal) && omega_blackwell_codegen_tern_gemv_prog(&coal, &prog) == 0,
          "4. mutant 5 (one slot set) builds");
    coal.mutant = 6;
    check(omega_tern_gemv_spec_ok(&coal) && omega_blackwell_codegen_tern_gemv_prog(&coal, &prog) == 0,
          "4. mutant 6 (no unit clamp) builds");
    coal.mutant = 7;
    check(!omega_tern_gemv_spec_ok(&coal), "4. negative control: mutant 7 is refused");
    rowmajor_sim();
    if (fails) { printf("gpu_tern_sched_test: FAIL (%d)\n", fails); return 1; }
    printf("gpu_tern_sched_test: PASS\n");
    return 0;
}
