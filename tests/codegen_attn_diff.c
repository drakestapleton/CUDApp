/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen_attn.h"
#include "omega_blackwell_codegen_pattn.h"
#include "omega_gpu_om_ir.h"
#include "cudapp_native.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
int omega_codegen_attn_oracle(const OmegaDecodeAttnSpec *, BlackwellIRProgram *);
int omega_codegen_pattn_oracle(const OmegaPrefillAttnSpec *, BlackwellIRProgram *);
static BlackwellIRProgram c, o;
static unsigned char cb[BW_MAX_IR_INSNS * 16], ob[BW_MAX_IR_INSNS * 16];
static int same(int verbose) {
    if (c.count != o.count) return 0;
    for (size_t i = 0; i < c.count; ++i) if (memcmp(&c.insns[i], &o.insns[i], sizeof c.insns[i])) {
        const unsigned char *a = (const unsigned char *)&c.insns[i], *b = (const unsigned char *)&o.insns[i];
        for (size_t j = 0; j < sizeof c.insns[i]; ++j) if (a[j] != b[j]) {
            if (verbose) printf("IR mismatch record=%zu byte=%zu c=%u om=%u\n", i, j, a[j], b[j]);
            break;
        }
        return 0;
    }
    if (memcmp(&c.regalloc, &o.regalloc, sizeof c.regalloc)) { if (verbose) puts("allocation mismatch"); return 0; }
    OmegaBlackwellKernel ck, ok;
    if (omega_blackwell_kernel_encode_into(&c, cb, sizeof cb, &ck) ||
        omega_blackwell_kernel_encode_into(&o, ob, sizeof ob, &ok)) return 0;
    return ck.code_size == ok.code_size && memcmp(cb, ob, ck.code_size) == 0;
}
int main(void) {
    unsigned cases = 0, accepted = 0, changed = 0, mutants = 0;
    uint64_t error;
    for (unsigned d = 0; d <= OMEGA_DATTN_MAX_DEPTH + 1; ++d) for (int m = -1; m <= 5; ++m) {
        OmegaDecodeAttnSpec s = {d, m};
        int cr = omega_codegen_attn_oracle(&s, &c), rc = omega_blackwell_codegen_decode_attn_prog(&s, &o);
        ++cases;
        if ((cr == 0) != (rc == 0) || (!cr && !same(1))) { printf("FAIL decode depth=%u mutant=%d c=%d om=%d records=%zu/%zu\n", d, m, cr, rc, c.count, o.count); return 1; }
        if (!cr) {
            ++accepted; o.insns[0].imm ^= 1; if (same(0)) return 2; ++changed;
            if (omega_gpu_om_ir_replay((OmegaOmIrUnit)omega_cg_attn, 1, d, (uint64_t)m, 0, &o, &error) || same(0)) return 4;
            ++mutants;
        }
    }
    /* All accepted builder inputs: W in {4,8}, mutant 0..4, modes 0..2,
       mirror zero except APPEND, where both mirror values are accepted. */
    const unsigned warps[] = {0, 4, 8, 16};
    for (unsigned w = 0; w < sizeof warps / sizeof warps[0]; ++w)
        for (int m = -1; m <= 5; ++m) for (unsigned mode = 0; mode <= 3; ++mode) for (unsigned mirror = 0; mirror <= 2; ++mirror) {
            OmegaPrefillAttnSpec s = {warps[w], m, mode, mirror};
            int cr = omega_codegen_pattn_oracle(&s, &c), rc = omega_blackwell_codegen_prefill_attn_prog(&s, &o);
            ++cases;
            if ((cr == 0) != (rc == 0) || (!cr && !same(1))) { printf("FAIL prefill W=%u mutant=%d mode=%u mirror=%u c=%d om=%d records=%zu/%zu\n", s.warps, m, mode, mirror, cr, rc, c.count, o.count); return 1; }
            if (!cr) {
                ++accepted; o.insns[0].imm ^= 1; if (same(0)) return 2; ++changed;
                if (omega_gpu_om_ir_replay((OmegaOmIrUnit)omega_cg_pattn, 1, s.warps, (uint64_t)m,
                                          mode | ((uint64_t)mirror << 8), &o, &error) || same(0)) return 4;
                ++mutants;
            }
        }
    if (omega_blackwell_codegen_decode_attn_prog(NULL, &o) != -1 || omega_blackwell_codegen_prefill_attn_prog(NULL, &o) != -1) return 3;
    const OmegaDecodeAttnSpec bad_decode[] = {{UINT32_MAX, 0}, {1, INT_MIN}, {1, INT_MAX}};
    const OmegaPrefillAttnSpec bad_prefill[] = {
        {UINT32_MAX, 0, 0, 0}, {4, INT_MIN, 0, 0}, {4, INT_MAX, 0, 0},
        {4, 0, UINT32_MAX, 0}, {4, 0, 1, UINT32_MAX}
    };
    for (unsigned i = 0; i < sizeof bad_decode / sizeof bad_decode[0]; ++i)
        if (omega_codegen_attn_oracle(&bad_decode[i], &c) != -1 ||
            omega_blackwell_codegen_decode_attn_prog(&bad_decode[i], &o) != -1) return 5;
    for (unsigned i = 0; i < sizeof bad_prefill / sizeof bad_prefill[0]; ++i)
        if (omega_codegen_pattn_oracle(&bad_prefill[i], &c) != -1 ||
            omega_blackwell_codegen_prefill_attn_prog(&bad_prefill[i], &o) != -1) return 5;
    const OmegaDecodeAttnSpec ds = {1, 0};
    const OmegaPrefillAttnSpec ps = {4, 0, 0, 0};
    if (omega_blackwell_codegen_decode_attn_prog(&ds, NULL) != -1 ||
        omega_blackwell_codegen_prefill_attn_prog(&ps, NULL) != -1) return 5;
    /* Freeze the complete accepted set measured from the retained oracle. */
    if (accepted != 60 || changed != accepted || mutants != accepted) return 6;
    printf("PASS attention differential cases=%u accepted=%u record-negative-controls=%u Omega-mutants-killed=%u IR+allocation+encoded-bytes\n", cases, accepted, changed, mutants);
    return 0;
}
