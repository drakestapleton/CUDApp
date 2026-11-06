/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Differential: retained C DEC builder (oracle) against the Omega unit cg_dec through the checked replay.
 * Domain: every kind (plus out-of-enum values) crossed with boundary sets taken from each refusal predicate
 * (v-1/v/v+1) and 0/UINT32_MAX on every u32 field. Accepted programs compare IR records, the register
 * allocation and the encoded bytes; refusals must agree; each accepted case runs a flipped-record control. */
#include "omega_blackwell_codegen_dec.h"
#include "omega_gpu_om_ir.h"
#include "cudapp_native.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
int omega_codegen_dec_oracle(const OmegaDecSpec *, BlackwellIRProgram *);
static BlackwellIRProgram c, o;
static unsigned char cb[BW_MAX_IR_INSNS * 16], ob[BW_MAX_IR_INSNS * 16];
static int same(int verbose) {
    if (c.count != o.count) { if (verbose) printf("count mismatch c=%zu om=%zu\n", c.count, o.count); return 0; }
    for (size_t i = 0; i < c.count; ++i) if (memcmp(&c.insns[i], &o.insns[i], sizeof c.insns[i])) {
        const unsigned char *a = (const unsigned char *)&c.insns[i], *b = (const unsigned char *)&o.insns[i];
        for (size_t j = 0; j < sizeof c.insns[i]; ++j) if (a[j] != b[j]) {
            if (verbose) printf("IR mismatch record=%zu op=%d byte=%zu c=%u om=%u\n", i, (int)c.insns[i].op, j, a[j], b[j]);
            break;
        }
        return 0;
    }
    if (memcmp(&c.regalloc, &o.regalloc, sizeof c.regalloc)) { if (verbose) puts("allocation mismatch"); return 0; }
    OmegaBlackwellKernel ck, ok;
    if (omega_blackwell_kernel_encode_into(&c, cb, sizeof cb, &ck) ||
        omega_blackwell_kernel_encode_into(&o, ob, sizeof ob, &ok)) { if (verbose) puts("encode failed"); return 0; }
    return ck.code_size == ok.code_size && memcmp(cb, ob, ck.code_size) == 0;
}
int main(void) {
    static const int kinds[] = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, INT_MAX};
    static const uint32_t modes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, UINT32_MAX};
    static const uint32_t widths[] = {0, 1, 255, 256, 257, 767, 768, 769, 1023, 1024, 1025, 2048, 3072, 4096,
                                      5120, 6144, 7168, 8192, 8193, 9216, UINT32_MAX};
    static const uint32_t groups[] = {0, 1, 7, 8, 9, 10, 15, 16, 17, UINT32_MAX};
    static const uint32_t strides[] = {0, 1, 15, 16, 17, 128, 255, 256, 257, 512, UINT32_MAX};
    static const int mutants[] = {INT_MIN, -1, 0, 1, 2, 3, 4, INT_MAX};
#define N(a) (sizeof a / sizeof a[0])
    unsigned long cases = 0, accepted = 0, changed = 0;
    for (unsigned k = 0; k < N(kinds); ++k) for (unsigned md = 0; md < N(modes); ++md)
    for (unsigned w = 0; w < N(widths); ++w) for (unsigned g = 0; g < N(groups); ++g)
    for (unsigned st = 0; st < N(strides); ++st) for (unsigned mu = 0; mu < N(mutants); ++mu) {
        OmegaDecSpec s;
        memset(&s, 0, sizeof s);
        s.kind = (OmegaDecKind)kinds[k]; s.mode = modes[md]; s.width = widths[w]; s.groups = groups[g];
        s.stride = strides[st]; s.mutant = mutants[mu];
        int cr = omega_codegen_dec_oracle(&s, &c), rc = omega_blackwell_codegen_dec_prog(&s, &o);
        ++cases;
        if ((cr == 0) != (rc == 0) || (!cr && !same(1))) {
            printf("FAIL kind=%d mode=%u width=%u groups=%u stride=%u mutant=%d c=%d om=%d records=%zu/%zu\n",
                   kinds[k], s.mode, s.width, s.groups, s.stride, s.mutant, cr, rc, c.count, o.count);
            return 1;
        }
        if (!cr) { ++accepted; o.insns[0].imm ^= 1; if (same(0)) return 2; ++changed; }
    }
    if (omega_blackwell_codegen_dec_prog(NULL, &o) != -1) return 3;
    OmegaDecSpec ds; memset(&ds, 0, sizeof ds); ds.kind = OMEGA_DEC_GATE;
    if (omega_blackwell_codegen_dec_prog(&ds, NULL) != -1) return 3;
    printf("PASS dec differential cases=%lu accepted=%lu record-negative-controls=%lu IR+allocation+encoded-bytes\n",
           cases, accepted, changed);
    /* frozen: the accepted partition of the product above; a builder that widens or narrows admission changes it */
    return changed == accepted && accepted == 1220470ul ? 0 : 6;
}
