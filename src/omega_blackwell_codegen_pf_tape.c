/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Replay the Omega command tape for the device-resident prefill copy kernel.
 * Scoreboards and live ranges stay in the existing emitter; Omega owns the instruction list. */
#include "omega_blackwell_codegen_pf.h"
#include "omega_bw_em.h"
#include "cudapp_native.h"
#include <string.h>

static int replay(const uint64_t *tape, uint64_t n, BlackwellIRProgram *prog) {
    if (!prog || n > 60) return -1;
    omega_bw_ir_init(prog);
    Em em; memset(&em, 0, sizeof em); em.p = prog; Em *e = &em;
    for (uint64_t r = 0; r < n; r++) {
        const uint64_t *w = tape + 2 + r * 4;
        switch (w[0]) {
        case 1: {
            int got;
            if (w[1] == 1) got = UV64(e);
            else if (w[1] == 2) got = V64(e);
            else if (w[1] == 3) got = V(e);
            else if (w[1] == 4) got = V128(e);
            else return -1;
            if (got != (int)w[2]) return -1;
            break;
        }
        case 10: ldcu64(e, (int)w[1], (uint32_t)w[2]); break;
        case 11: ldc64(e, (int)w[1], (uint32_t)w[2]); break;
        case 12: ldc32(e, (int)w[1], (uint32_t)w[2]); break;
        case 13: s2r(e, (int)w[1], (uint32_t)w[2]); break;
        case 14: imadi(e, (int)w[1], (int)(w[2] & 0xffffu), 0, (uint32_t)w[3], (int)(w[2] >> 16) - 1); break;
        case 15: isetp_ge_u32(e, (int)w[1], (int)w[2]); break;
        case 16: {
            BlackwellIRInsn x = I0(BW_IR_EXIT); x.predicate_p0 = true; emit(e, x, K_EXIT);
            break;
        }
        case 17: imadw(e, (int)w[1], (int)w[2], (uint32_t)w[3], (int)w[1]); break;
        case 18: ldg_wide(e, BW_IR_LDG_E_128, (int)w[1], (int)w[2], (uint32_t)w[3], 0, 1); break;
        case 19: {
            BlackwellIRInsn st = I0(BW_IR_STG_E);
            st.src1_vreg = (int)w[1]; st.src2_vreg = (int)w[2]; st.src2_subreg = (uint8_t)w[3];
            st.imm = (uint32_t)w[3] * 4u; st.ureg = 0; emit(e, st, K_STORE);
            break;
        }
        case 20: tail(e); break;
        default: return -1;
        }
        if (e->err) return -1;
    }
    if (e->err) return -1;
    return omega_bw_regalloc_solve(prog);
}

int omega_cg_pf_copy_from_omega(BlackwellIRProgram *prog, int mutant) {
    uint64_t tape[256];
    uint64_t result = 1;
    memset(tape, 0, sizeof tape);
    uint32_t st = omega_cg_pf_copy(&result, 0, tape, 256, mutant ? 1u : 0u, 0);
    if (st != 0 || result != 0) return -1;
    return replay(tape, tape[1], prog);
}
