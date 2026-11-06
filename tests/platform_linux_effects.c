/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host differential: omega2_plat_ring_store matches memcpy; over 1048576 refuses.
 * 1048576 = 2^20 (WolframLanguageEvaluator: 2^20 -> 1048576). No device is opened. */
#include "cudapp_plat_effects.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    unsigned char src[64], dst[64], oracle[64];
    uint64_t wrote = 0;
    for (int i = 0; i < 64; i++) src[i] = (unsigned char)(i * 3 + 1);
    memset(dst, 0x5a, sizeof dst);
    memcpy(oracle, src, 64);
    if (omega2_plat_ring_store(&wrote, dst, 64, src, 64, 64) != 0 || wrote != 64 || memcmp(dst, oracle, 64) != 0) {
        fprintf(stderr, "FAIL ring-store differential\n");
        return 1;
    }
    uint64_t fenced = 0;
    if (omega2_plat_fence(&fenced) != 0 || fenced != 1) {
        fprintf(stderr, "FAIL fence\n");
        return 1;
    }
    wrote = 7;
    if (omega2_plat_ring_store(&wrote, dst, 64, src, 64, 1048577ull) == 0 && wrote == 1048577ull) {
        fprintf(stderr, "FAIL over-bound store accepted\n");
        return 1;
    }
    printf("PASS platform linux effects differential\n");
    return 0;
}
