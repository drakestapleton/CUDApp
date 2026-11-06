/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen_pfhost.h"
#include "omega_gpu_om_ir.h"
#include "cudapp_native.h"
#include <stdio.h>
#include <string.h>
int omega_blackwell_codegen_rl_prog_c(const OmegaRlSpec *, BlackwellIRProgram *);
int omega_blackwell_codegen_mc_prog_c(const OmegaMcSpec *, BlackwellIRProgram *);
static BlackwellIRProgram c, o;
static uint8_t cb[BW_MAX_IR_INSNS * 16], ob[BW_MAX_IR_INSNS * 16];
static int same(void) {
    if (c.count != o.count || memcmp(c.insns, o.insns, c.count * sizeof c.insns[0]) ||
        memcmp(&c.regalloc, &o.regalloc, sizeof c.regalloc)) return 0;
    OmegaBlackwellKernel ck, ok;
    if (omega_blackwell_kernel_encode_into(&c, cb, sizeof cb, &ck) ||
        omega_blackwell_kernel_encode_into(&o, ob, sizeof ob, &ok)) return 0;
    return memcmp(cb, ob, c.count * 16) == 0;
}
int main(int argc, char **argv) {
    unsigned rl = 0, mc = 0, refused = 0;
    int quick = argc > 1 && strcmp(argv[1], "--quick") == 0;
    for (uint32_t k = 128; k <= 16384; k += 128)
    for (uint32_t e = 32; e <= 1024; e += 32)
    for (uint32_t t = 1; t <= 16; t++)
    for (int m = 0; m <= 2; m++) {
        OmegaRlSpec s = {k,e,t,m};
        if (!omega_rl_spec_ok(&s)) continue;
        if (quick && (k != 256 || e != 32 || t != 1)) continue;
        int cr = omega_blackwell_codegen_rl_prog_c(&s,&c);
        int rc = omega_blackwell_codegen_rl_prog(&s,&o);
        if (cr != rc || (!cr && !same())) {
            printf("FAIL rl k=%u e=%u t=%u mutant=%d c=%d om=%d count=%zu/%zu\n",k,e,t,m,cr,rc,c.count,o.count); return 1;
        }
        if (cr) refused++; else rl++;
    }
    for (uint32_t h = 128; h <= 4096; h += 128)
    for (uint32_t n = 4; n <= 16; n += 4)
    for (int m = 0; m <= 2; m++) {
        if (quick && (h != 128 || n != 4)) continue;
        OmegaMcSpec s = {h,n,m};
        int cr = omega_blackwell_codegen_mc_prog_c(&s,&c);
        int rc = omega_blackwell_codegen_mc_prog(&s,&o);
        if (cr != rc || (!cr && !same())) {
            printf("FAIL mc h=%u top_k=%u mutant=%d c=%d om=%d count=%zu/%zu\n",h,n,m,cr,rc,c.count,o.count); return 1;
        }
        if (cr) refused++; else mc++;
    }
    OmegaMcSpec s = {128,4,0};
    if (omega_blackwell_codegen_mc_prog_c(&s,&c) || omega_blackwell_codegen_mc_prog(&s,&o)) return 1;
    o.insns[0].imm ^= 1u;
    if (same()) { puts("FAIL flipped-record negative control survived"); return 1; }
    OmegaRlSpec badrl = {127,32,1,0}; OmegaMcSpec badmc = {128,3,0};
    if (omega_blackwell_codegen_rl_prog(&badrl,&o) != -1 || omega_blackwell_codegen_mc_prog(&badmc,&o) != -1 ||
        omega_blackwell_codegen_rl_prog(NULL,&o) != -1 || omega_blackwell_codegen_mc_prog(NULL,&o) != -1 ||
        omega_blackwell_codegen_mc_prog(&s,NULL) != -1) return 1;
    puts("PASS flipped-record negative control and invalid/null refusal");
    printf("PASS pfhost exhaustive IR, allocation and encoded-byte differential rl=%u mc=%u matching_refusals=%u\n",rl,mc,refused);

    uint64_t ins[OMEGA_OM_IR_WORDS] = {0}, meta[OMEGA_OM_IR_META] = {0}, result = 0;
    if (omega_cg_pfhost(&result, 2, 0, 0, 0, ins, OMEGA_OM_IR_WORDS, meta, OMEGA_OM_IR_META) != OMEGA_OK || result != 1) return 1;
    return 0;
}