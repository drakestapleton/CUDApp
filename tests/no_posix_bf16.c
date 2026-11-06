/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Same transport caller for the canonical hosted view, native scalar, and C red.
 * Sweep every upper half-word, including both signs, every exponent and payload
 * prefix; lower-half representatives border zero, the RNE tie and UINT16_MAX.
 * Modes border zero and cover non-32-bit values (the canonical mode is ulong). */
#include "omega_gpu_apiviews.h"
#include <stdio.h>

int main(void) {
    static const uint32_t low[] = {0, 1, 32767, 32768, 32769, 65534, 65535};
    static const uint64_t modes[] = {0, 1, 2, UINT32_MAX, UINT64_C(4294967296), UINT64_MAX};
    float x[7];
    uint16_t y[9];
    for (unsigned m = 0; m < sizeof modes / sizeof modes[0]; ++m) {
        if (av_bf16_n(modes[m], 0, NULL, NULL)) return 2;
        for (uint32_t high = 0; high <= UINT16_MAX; ++high) {
            for (unsigned i = 0; i < 7; ++i) {
                uint32_t raw = (high << 16) | low[i];
                memcpy(&x[i], &raw, sizeof raw);
            }
            y[0] = 0x1234; y[8] = 0x5678;
            if (av_bf16_n(modes[m], 7, x, y + 1)) return 3;
            if (y[0] != 0x1234 || y[8] != 0x5678) return 4;
            if (fwrite(y + 1, sizeof *y, 7, stdout) != 7) return 5;
        }
    }
    return ferror(stdout) ? 6 : 0;
}
