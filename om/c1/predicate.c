/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "bw_warp_sim.h"

/* WolframLanguageEvaluator:
 * {IntegerString[BitShiftLeft[127,23],16,8],
 * Table[Xor[p,i],{p,{False,True}},{i,{False,True}}]}
 * -> {"3f800000", {{False, True}, {True, False}}}.
 * 0xfeedface is the existing simulator's canary (bw_warp_sim.h). */
int main(void) {
    static BlackwellIRProgram p;
    static const struct { BlackwellIROpcode op; uint32_t input, want; } cases[] = {
        {BW_IR_I2FP_U32, 1, 0x3f800000u},
        {BW_IR_F2I_U32, 0x3f800000u, 1},
        {BW_IR_MOV_IMM, 1, 1},
        {BW_IR_SHFL_DOWN, 1, 1}
    };
    size_t checks = 0;
    p.count = 2;
    p.regalloc.vreg_to_phys[0] = 0;
    p.regalloc.vreg_to_phys[1] = 1;
    p.insns[1] = (BlackwellIRInsn){.op = BW_IR_EXIT};
    for (size_t c = 0; c < sizeof cases / sizeof cases[0]; ++c) {
        for (unsigned pattern = 0; pattern < 3; ++pattern) for (unsigned inv = 0; inv < 2; ++inv) {
            p.insns[0] = (BlackwellIRInsn){
                .op = cases[c].op, .dst_vreg = 0, .src1_vreg = 1,
                .src2_vreg = -1, .src3_vreg = -1, .imm = 1,
                .predicate_p0 = 1, .predicate_not = inv
            };
            WsWarp w = {0};
            WsSim s = {.p = &p};
            w.present = UINT32_MAX;
            w.nfrag = 1;
            w.frag[0].mask = UINT32_MAX;
            for (unsigned lane = 0; lane < WS_WARP; ++lane) {
                w.lane[lane].p0 = (uint8_t)(pattern == 2 ? lane & 1u : pattern);
                w.lane[lane].r[0] = 0xfeedfaceu;
                w.lane[lane].r[1] = cases[c].input;
            }
            /* An inactive shuffle must walk past an exited lane, while the
             * corresponding active shuffle must retain the existing refusal. */
            if (cases[c].op == BW_IR_SHFL_DOWN && pattern == 0 && inv == 0) {
                w.exited = 1u;
                w.frag[0].mask &= ~1u;
            }
            if (ws_run_warp(&s, &w, 0, 0, 0, 0) != 0) {
                fprintf(stderr, "C1 PREDICATE FAIL op=%u: %s\n", (unsigned)cases[c].op, s.msg);
                return 1;
            }
            for (unsigned lane = 0; lane < WS_WARP; ++lane) {
                uint32_t want = w.lane[lane].p0 != inv ? cases[c].want : 0xfeedfaceu;
                ++checks;
                if (w.lane[lane].r[0] != want) {
                    fprintf(stderr, "C1 PREDICATE FAIL op=%u pattern=%u inv=%u lane=%u got=%08x want=%08x\n",
                            (unsigned)cases[c].op, pattern, inv, lane, w.lane[lane].r[0], want);
                    return 1;
                }
            }
        }
    }
    {
        WsWarp w = {0};
        WsSim s = {.p = &p};
        p.insns[0].op = BW_IR_SHFL_DOWN;
        p.insns[0].predicate_not = 0;
        w.present = UINT32_MAX;
        w.exited = 1u;
        w.nfrag = 1;
        w.frag[0].mask = UINT32_MAX & ~1u;
        for (unsigned lane = 0; lane < WS_WARP; ++lane) w.lane[lane].p0 = 1;
        ++checks;
        if (ws_run_warp(&s, &w, 0, 0, 0, 0) == 0 || s.err != WS_E_SHFL_EXITED) {
            fprintf(stderr, "C1 PREDICATE FAIL active SHFL structural refusal changed\n");
            return 1;
        }
    }
    printf("C1 PREDICATE PASS checks=%zu (guarded unsigned conversions, MOV and SHFL)\n", checks);
    return 0;
}
