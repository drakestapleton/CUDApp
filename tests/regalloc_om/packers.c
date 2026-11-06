/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Source-derived partition: 16-row/column A tiles, 8-column B tiles, row0
 * partial/empty/extra padded blocks, every source bit class, and guard words.
 * The driver is identical for the migration oracle and the Omega successor. */
#include "omega_blackwell_codegen.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint32_t rng = 0x516a9be3u;
static uint32_t next(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
static void dump(const uint16_t *d, size_t n) { if (fwrite(d, sizeof *d, n, stdout) != n) abort(); }
static void row0_regression(void) {
    uint16_t *s = malloc(sizeof *s), d[16 * 48];
    if (!s) abort();
    *s = 0x1234; memset(d, 0x5a, sizeof d);
    omega_matmul_frag_pack_a_row0(s, 1, 48, d);
    if (d[0] != *s || d[256] != 0 || d[512] != 0 || d[2] != 0x5a5a) abort();
    free(s);
}
static void sentinels(const uint32_t *dims, size_t nd, const uint32_t *bits, size_t nb) {
    for (size_t mi = 0; mi < nd; ++mi) for (size_t ni = 0; ni < nd; ++ni) {
        uint32_t m = dims[mi], n = dims[ni], np = n + 3;
        size_t size = (size_t)m * np;
        float *d = malloc((size + 2) * sizeof *d);
        if (!d) abort();
        for (size_t pi = 0; pi < nb; ++pi) {
            float poison;
            memcpy(&poison, bits + pi, sizeof poison);
            for (size_t i = 0; i < size + 2; ++i) memcpy(d + i, bits + i % nb, sizeof *d);
            uint64_t counts[3];
            counts[0] = omega_matmul_result_sentinels(d + 1, m, n, np, poison, 0);
            counts[1] = omega_matmul_result_sentinels(d + 1, m, n, np, poison, pi & 1 ? -1 : 1);
            counts[2] = omega_matmul_result_sentinels(d + 1, m, n, np, poison, 0);
            if (fwrite(counts, sizeof *counts, 3, stdout) != 3 ||
                fwrite(d, sizeof *d, size + 2, stdout) != size + 2) abort();
        }
        free(d);
    }
}
int main(int argc, char **argv) {
    if (argc == 1 || strcmp(argv[1], "--mutant") != 0) row0_regression();
    if (argc > 1 && strcmp(argv[1], "--row0") == 0) return 0;
    const uint32_t dims[] = {0, 1, 2, 7, 8, 15, 16, 17, 31, 32, 47};
    const uint32_t bits[] = {0, 0x80000000u, 0x3f800000u, 0x3f808000u,
        0x3f818000u, 0x7f800000u, 0xff800000u, 0x7f800001u, 0xffc12345u,
        0x00000001u, 0x007fffffu, 0x7fffffffu, 0xffffffffu};
    sentinels(dims, sizeof dims / sizeof *dims, bits, sizeof bits / sizeof *bits);
    for (unsigned mi = 0; mi < sizeof dims / sizeof *dims; ++mi) {
        uint32_t m = dims[mi], mp = ((m + 15u) / 16u + 1u) * 16u;
        for (unsigned ki = 0; ki < sizeof dims / sizeof *dims; ++ki) {
            uint32_t k = dims[ki], kp = ((k + 15u) / 16u + 2u) * 16u;
            size_t sn = (size_t)m * k, dn = (size_t)mp * kp;
            uint16_t *s = malloc((sn + k + 1) * sizeof *s);
            float *f = malloc((sn + k + 1) * sizeof *f);
            uint16_t *d = malloc((dn + 2) * sizeof *d);
            if (!s || !f || !d) abort();
            for (size_t i = 0; i < sn + k + 1; ++i) {
                uint32_t x = i < sizeof bits / sizeof *bits ? bits[i] : next();
                s[i] = (uint16_t)next(); memcpy(f + i, &x, sizeof x);
            }
            memset(d, 0xa5, (dn + 2) * sizeof *d);
            omega_matmul_frag_pack_a(s, m, k, mp, kp, d + 1); dump(d, dn + 2);
            memset(d, 0xa5, (dn + 2) * sizeof *d);
            omega_matmul_frag_pack_a_f32(f, m, k, mp, kp, d + 1); dump(d, dn + 2);
            memset(d, 0xa5, (dn + 2) * sizeof *d);
            omega_matmul_frag_pack_a_row0(s, k, kp, d + 1); dump(d, dn + 2);
            memset(d, 0xa5, (dn + 2) * sizeof *d);
            omega_matmul_frag_pack_a_row0_f32(f, k, kp, d + 1); dump(d, dn + 2);
            free(s); free(f); free(d);
        }
    }
    for (unsigned ki = 0; ki < sizeof dims / sizeof *dims; ++ki) {
        uint32_t k = dims[ki], kp = ((k + 15u) / 16u + 1u) * 16u;
        for (unsigned ni = 0; ni < sizeof dims / sizeof *dims; ++ni) {
            uint32_t n = dims[ni], nt = (n + 7u) / 8u;
            uint16_t *s = malloc(((size_t)k * n + 1) * sizeof *s);
            if (!s) abort();
            for (size_t i = 0; i < (size_t)k * n + 1; ++i) s[i] = (uint16_t)next();
            for (uint32_t nt0 = 0; nt0 <= nt + 1; ++nt0) {
                uint32_t nt1 = nt + 1;
                size_t dn = (size_t)(nt1 - nt0) * kp * 8;
                uint16_t *d = malloc((dn + 2) * sizeof *d);
                if (!d) abort();
                memset(d, 0xa5, (dn + 2) * sizeof *d);
                omega_matmul_frag_pack_b(s, k, n, kp, nt0, nt1, d + 1);
                dump(d, dn + 2); free(d);
            }
            free(s);
        }
    }
    return 0;
}
