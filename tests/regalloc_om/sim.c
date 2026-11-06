/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "bw_warp_sim_tc.h"
#include "omega_blackwell_matmul.h"
#include "omega_blackwell_encoder.h"
#include <limits.h>

static int signed_ge(void) {
    static const int32_t values[] = {INT_MIN, INT_MIN + 1, -2, -1, 0, 1, INT_MAX};
    BlackwellIRProgram p = {0};
    p.regalloc.vreg_to_phys[0] = 2;
    p.regalloc.vreg_to_phys[1] = 3;
    WsSim s = {.p = &p};
    unsigned cases = 0;
    for (unsigned i=0;i<sizeof values/sizeof values[0];++i) {
        for (unsigned j=0;j<sizeof values/sizeof values[0];++j) {
            for (int reg=0;reg<=1;++reg) {
                BlackwellIRInsn in = {.op=BW_IR_ISETP_GE, .src1_vreg=0,
                    .src2_vreg=reg ? 1 : -1, .src3_vreg=-1, .imm=(uint32_t)values[j]};
                WsLane lane = {0};
                lane.r[2]=(uint32_t)values[i]; lane.r[3]=(uint32_t)values[j];
                ws_lane_exec(&s,&lane,&in,0,0,0,0);
                if (s.err || lane.p0 != (values[i] >= values[j])) {
                    fprintf(stderr,"FAIL signed GE lhs=%d rhs=%d register=%d\n",values[i],values[j],reg);
                    return 1;
                }
                ++cases;
            }
        }
    }
    printf("PASS signed GE host-reference cases=%u register and immediate\n",cases);
    return 0;
}

static void pointer(WsSim *s, unsigned off, const void *p) {
    uintptr_t v = (uintptr_t)p;
    s->cbank[off / 4] = (uint32_t)v;
    s->cbank[off / 4 + 1] = (uint32_t)(v >> 32);
}

int main(void) {
    if (signed_ge()) return 1;
    static uint16_t a[64 * 64], b[64 * 32], ap[64 * 64], bp[64 * 32];
    if (omega_blackwell_verify_encoder_fixtures() ||
        omega_blackwell_verify_codegen_fixtures() ||
        omega_blackwell_verify_codegen_fixtures_fb1cut4() ||
        omega_blackwell_verify_codegen_fixtures_intops() ||
        omega_blackwell_verify_codegen_fixtures_fragmm() ||
        omega_blackwell_verify_codegen_fixtures_reconv() ||
        omega_blackwell_verify_codegen_fixtures_bfly_f2fp() ||
        omega_blackwell_verify_codegen_fixtures_atomf32()) {
        fprintf(stderr, "FAIL existing encoder golden fixtures\n"); return 1;
    }
    puts("PASS existing encoder golden fixtures and vecadd SHA-256");
    static float c[64 * 32], expected[64 * 32];
    unsigned cases = 0;
    for (uint32_t k = 16; k <= 64; k *= 2) {
        for (uint32_t rt = 1; rt <= 4; rt *= 2) {
            const uint32_t m = rt * 16, n = 8;
            for (uint32_t i = 0; i < m * k; ++i) a[i] = omega_fp32_to_bf16((float)(i % 3));
            for (uint32_t i = 0; i < k * n; ++i) b[i] = omega_fp32_to_bf16((float)(i % 5));
            omega_matmul_frag_pack_a(a, m, k, m, k, ap);
            omega_matmul_frag_pack_b(b, k, n, k, 0, 1, bp);
            if (omega_matmul_cpu_oracle_bf16(a, b, expected, m, k, n)) return 1;
            for (int blocked = 0; blocked <= 1; ++blocked) {
                if (blocked && rt == 1) continue; /* the blocked builder refuses the single-tile shape */
                OmegaMatMulSpec spec;
                BlackwellIRProgram p;
                if (omega_matmul_spec_init(&spec, m, k, n, OMEGA_MATMUL_PRECISION_BF16)) return 1;
                int rc = blocked
                    ? omega_blackwell_codegen_matmul_frag_block_prog(&spec, 1, 1, rt, 1, 0, &p)
                    : omega_blackwell_codegen_matmul_frag_prog(&spec, 1, 1, 0, &p);
                if (rc) { fprintf(stderr, "FAIL sim builder k=%u rt=%u blocked=%d\n", k, rt, blocked); return 1; }
                memset(c, 0, sizeof c);
                WsSim s = { .p = &p, .threads = 32, .nranges = 3 };
                s.ranges[0] = (WsRange){(uintptr_t)ap, (uintptr_t)(ap + m * k)};
                s.ranges[1] = (WsRange){(uintptr_t)bp, (uintptr_t)(bp + k * n)};
                s.ranges[2] = (WsRange){(uintptr_t)c, (uintptr_t)(c + m * n)};
                pointer(&s, 0x380, ap); pointer(&s, 0x388, bp); pointer(&s, 0x390, c);
                if (ws_run(&s, 1, blocked ? 1 : rt)) {
                    fprintf(stderr, "FAIL sim %s k=%u rt=%u blocked=%d op=%d\n", s.msg, k, rt, blocked,
                            s.err_pc >= 0 ? (int)p.insns[s.err_pc].op : -1); return 1;
                }
                if (memcmp(c, expected, m * n * sizeof(float))) {
                    fprintf(stderr, "FAIL sim output k=%u rt=%u blocked=%d\n", k, rt, blocked); return 1;
                }
                ++cases;
            }
        }
    }
    printf("PASS matmul warp-sim cases=%u CPU oracle bit-identical\n", cases);
    return 0;
}
