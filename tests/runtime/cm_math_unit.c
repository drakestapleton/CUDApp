/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static uint64_t bits(double x) { uint64_t b; memcpy(&b, &x, 8); return b; }
static double value(uint64_t b) { double x; memcpy(&x, &b, 8); return x; }
static uint64_t ordered(uint64_t b) { return b >> 63 ? ~b : b | UINT64_C(0x8000000000000000); }
static uint64_t maxima[7], arguments[7], got_bits[7], ref_bits[7];
static int check(unsigned mode, uint64_t arg) {
    double x = value(arg), ref;
    switch (mode) {
    case 0: ref = log(x); break;
    case 1: ref = log2(x); break;
    case 2: ref = ceil(x); break;
    case 3: ref = exp(x); break;
    case 4: ref = exp2(x); break;
    case 5: ref = sqrt(x); break;
    default: ref = erfc(x); break;
    }
    uint64_t out;
    if (omega_cm_math(&out, mode, arg)) return 1;
    double got = value(out);
    if (isnan(ref)) return !isnan(got);
    if (isnan(got)) return 1;
    uint64_t a = ordered(out), b = ordered(bits(ref));
    uint64_t delta = a >= b ? a - b : b - a;
    if (delta > maxima[mode]) {
        maxima[mode] = delta; arguments[mode] = arg;
        got_bits[mode] = out; ref_bits[mode] = bits(ref);
    }
    uint64_t limit = mode == 3 ? 1 : mode == 6 ? 2 : 0;
    if (delta > limit) return 1;
    return 0;
}
int main(void) {
    int bad = 0;
    /* Wolfram 150-digit rounding references, not the glibc oracle. */
    uint64_t out;
    if (omega_cm_math(&out, 3, UINT64_C(0x3e50000000000000)) || out != UINT64_C(0x3ff0000004000001)) ++bad;
    if (omega_cm_math(&out, 6, UINT64_C(0x402570dc08794a6a)) || out != UINT64_C(0x354eb3d792b02a8a)) ++bad;
    uint64_t special[] = {0, UINT64_C(0x8000000000000000), 1, 2, UINT64_C(0x000fffffffffffff), UINT64_C(0x0010000000000000), UINT64_C(0x3fefffffffffffff), UINT64_C(0x3ff0000000000000), UINT64_C(0x3ff0000000000001), UINT64_C(0x7fefffffffffffff), UINT64_C(0x7ff0000000000000), UINT64_C(0xfff0000000000000), UINT64_C(0x7ff8000000000000), UINT64_C(0x7ff0000000000001)};
    for (unsigned m = 0; m < 7; ++m) {
        for (unsigned i = 0; i < sizeof special / sizeof *special; ++i) {
            bad += check(m, special[i]); bad += check(m, special[i] | UINT64_C(0x8000000000000000));
        }
        /* Every exponent boundary and its adjacent representable values. */
        for (uint64_t e = 1; e < 2047; ++e) {
            uint64_t b = e << 52;
            for (unsigned d = 0; d < 3; ++d) {
                bad += check(m, b + d - 1); bad += check(m, (b + d - 1) | UINT64_C(0x8000000000000000));
            }
        }
        uint64_t rng = UINT64_C(0xda079346a277);
        for (unsigned i = 0; i < 65536; ++i) {
            rng = rng * UINT64_C(6364136223846793005) + 1;
            bad += check(m, rng);
        }
        if (m == 3 || m == 4 || m == 6) {
            int lo = m == 3 ? -6000 : m == 4 ? -8608 : -224;
            int hi = m == 3 ? 5680 : m == 4 ? 8192 : 224;
            for (int i = lo; i <= hi; ++i) {
                double x = (double)i * 0.125;
                bad += check(m, bits(x));
                bad += check(m, bits(nextafter(x, INFINITY)));
                bad += check(m, bits(nextafter(x, -INFINITY)));
            }
        }
        printf("CM MATH mode=%u max_ulp=%" PRIu64 " input_bits=%016" PRIx64 " got=%016" PRIx64 " oracle=%016" PRIx64 "\n", m, maxima[m], arguments[m], got_bits[m], ref_bits[m]);
    }
    if (bad) { fprintf(stderr, "CM MATH differential FAIL errors=%d\n", bad); return 1; }
    puts("CM MATH differential sweep complete"); return 0;
}
