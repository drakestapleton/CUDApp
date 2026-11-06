/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Verification-only: original fixture data, independent unpacked dot product.
 * Expand ~ to HOME before compiling, as with the receipt logs.
 * The SPDX and path-expansion comments are archival, not behavioral changes. */
#define main omega_tern_original_main
#include "~/work/cudastack/omega-mv/tests/gpu_tern_test.c"
#undef main
int main(void) {
    OmegaTernGemvSpec sp = {512, 0, 2, 2, 2, 0, 0};
    int8_t *w = make_w(13u * 512u, 0.3, 101), *x = make_x(512, 108);
    static const int32_t expected[13] = {-3013,2779,-1089,-2155,-1793,-2083,-240,604,-1545,1900,-156,1931,-1037};
    uint32_t y[13]; char msg[240]; int bad = 0;
    int32_t *ref = oracle(w, 13, 512, x);
    int rc = sim_run(&sp, 13, w, x, NULL, NULL, 1.0f, 2, 0, y, msg, sizeof msg);
    printf("original smallest case: sim rc %d\n", rc);
    for (unsigned r = 0; r < 13; r++) {
        int32_t dot = 0;
        for (unsigned c = 0; c < 512; c++) dot += (int32_t)w[r*512+c] * (int32_t)x[c];
        printf("row %u: Wolfram %d independent-dot %d algebra %d sim %d\n",r,expected[r],dot,ref[r],(int32_t)y[r]);
        if (expected[r] != dot || ref[r] != dot || (int32_t)y[r] != dot) bad++;
    }
    free(w); free(x); free(ref);
    printf("independent original rows: %d mismatches\n",bad);
    return rc || bad ? 1 : 0;
}
