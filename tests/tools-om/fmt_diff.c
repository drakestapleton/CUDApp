/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* fmt_diff N c|om: one line per (x, precision, conversion) for the specials and N splitmix64 doubles in five
 * families (raw bits, 2^-10 steps, 10^-3 steps in [-1000, 1000], 2^-20 steps, scaled 52-bit mantissas).
 * Mode c prints glibc snprintf("%.*f" / "%.*g"); mode om prints om/tools/fmt.om (fm_f / fm_g) through the
 * test export omega_fmtt (tests/tools-om/fmt.sh). The two outputs must be byte-identical. */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t omega_fmtt(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, void *p3, uint64_t p3_n);

static uint64_t s = 0x9e3779b97f4a7c15ull;
static uint64_t nx(void) {
    uint64_t z = (s += 0x9e3779b97f4a7c15ull);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}
static int om;

static void one(double x) {
    static const int fp[] = { 0, 1, 2, 6, 17 }, gp[] = { 0, 1, 6, 17 };
    uint64_t b;
    memcpy(&b, &x, 8);
    for (int g = 0; g < 2; g++)
        for (int k = 0; k < (g ? 4 : 5); k++) {
            int p = g ? gp[k] : fp[k];
            char t[1601];
            if (om) {
                unsigned char o[1600];
                uint64_t r = 0;
                uint32_t st = omega_fmtt(&r, b, (uint64_t)p, (uint64_t)g, o, sizeof o);
                if (st || r > sizeof o) { printf("%a %d %d status %u\n", x, p, g, st); continue; }
                memcpy(t, o, r);
                t[r] = 0;
            } else {
                snprintf(t, sizeof t, g ? "%.*g" : "%.*f", p, x);
            }
            printf("%a %d %d [%s]\n", x, p, g, t);
        }
}

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "usage: fmt_diff N c|om\n"); return 2; }
    long n = atol(argv[1]);
    om = strcmp(argv[2], "om") == 0;
    static const double sp[] = { 0.0, -0.0, 1.0, -1.0, 0.5, 1.5, 2.5, 0.25, 0.125, 0.05, 0.15, 0.35, 9.5, 99.5,
        999999.5, 9999995.0, 0.000099999995, 1e-5, 1e-4, 9.9999996, 123456.5, 1e15, 1e16, 1e300,
        1.7976931348623157e308, 4.9406564584124654e-324, 2.2250738585072014e-308, INFINITY, -INFINITY, NAN, -NAN,
        0.95, 1048576.0 * 3.05, 0.25 / 1.0, 0.75, 9.95, 99.95, 0.0005, 0.00049999999999999999 };
    for (size_t i = 0; i < sizeof sp / sizeof sp[0]; i++) one(sp[i]);
    for (long i = 0; i < n; i++) {
        uint64_t r = nx();
        double x;
        switch (i % 5) {
        case 0: memcpy(&x, &r, 8); break;
        case 1: x = (double)(int64_t)(r >> 20) / 1024.0; break;
        case 2: x = (double)(r % 2000001) / 1000.0 - 1000.0; break;
        case 3: x = (double)(r >> 11) / 1048576.0; break;
        default: x = ldexp((double)(r >> 12), (int)(nx() % 200) - 150); break;
        }
        one(x);
    }
    return 0;
}
