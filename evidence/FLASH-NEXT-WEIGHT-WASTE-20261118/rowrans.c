/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Measurement prototype: bit-exact row-addressable rANS coding of FP8 (E4M3) weight bytes.
 * One static 14-bit model per tensor, one independent stream per matrix row (so rows decode in parallel).
 * usage: rowrans <raw> <rows> <cols> [flip]   prints: raw_bytes coded_bytes roundtrip(0/1)
 * coded_bytes counts everything a decoder needs: 256 u16 frequencies + u32 offset per row + streams. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define SB 14
#define RL (1u << 23)
int main(int argc, char **argv) {
    if (argc < 4) return 2;
    size_t R = strtoul(argv[2], 0, 10), C = strtoul(argv[3], 0, 10), n = R * C;
    int flip = argc > 4;
    uint8_t *in = malloc(n), *out = malloc(n), *buf = malloc(n * 2 + 64);
    FILE *f = fopen(argv[1], "rb");
    if (!f || fread(in, 1, n, f) != n) return 2;
    fclose(f);
    uint64_t cnt[256] = {0};
    for (size_t i = 0; i < n; i++) cnt[in[i]]++;
    uint32_t fr[256], cum[257], tot = 0;
    for (int s = 0; s < 256; s++) { fr[s] = (uint32_t)((cnt[s] << SB) / n); if (cnt[s] && !fr[s]) fr[s] = 1; tot += fr[s]; }
    while (tot != (1u << SB)) {               /* fix the rounding on the largest symbols */
        int b = 0; for (int s = 1; s < 256; s++) if (fr[s] > fr[b]) b = s;
        if (tot > (1u << SB)) { fr[b]--; tot--; } else { fr[b]++; tot++; }
    }
    cum[0] = 0; for (int s = 0; s < 256; s++) cum[s + 1] = cum[s] + fr[s];
    static uint8_t slot[1 << SB];
    for (int s = 0; s < 256; s++) for (uint32_t k = cum[s]; k < cum[s + 1]; k++) slot[k] = (uint8_t)s;
    size_t total = 512 + 4 * R;
    uint32_t *off = malloc(4 * (R + 1)); uint8_t *all = malloc(n * 2 + 8 * R); size_t w = 0;
    for (size_t r = 0; r < R; r++) {
        uint8_t *p = buf + 2 * C + 32, *e = p; uint32_t x = RL;
        for (size_t i = C; i-- > 0;) {
            uint32_t s = in[r * C + i], xm = ((RL >> SB) << 8) * fr[s];
            while (x >= xm) { *--p = (uint8_t)x; x >>= 8; }
            x = ((x / fr[s]) << SB) + (x % fr[s]) + cum[s];
        }
        p -= 4; memcpy(p, &x, 4);
        off[r] = (uint32_t)w; memcpy(all + w, p, (size_t)(e - p)); w += (size_t)(e - p);
    }
    off[R] = (uint32_t)w; total += w;
    if (flip) all[w / 2] ^= 0x10;             /* negative control: one flipped bit must break the round trip */
    for (size_t r = 0; r < R; r++) {
        const uint8_t *p = all + off[r], *end = all + off[r + 1]; uint32_t x; memcpy(&x, p, 4); p += 4;
        for (size_t i = 0; i < C; i++) {
            uint32_t k = x & ((1u << SB) - 1), s = slot[k];
            out[r * C + i] = (uint8_t)s;
            x = fr[s] * (x >> SB) + k - cum[s];
            while (x < RL && p < end) x = (x << 8) | *p++;
        }
    }
    printf("%zu %zu %d\n", n, total, memcmp(in, out, n) == 0);
    return 0;
}
