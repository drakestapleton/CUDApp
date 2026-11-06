/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int decimal(const char *s, uint32_t *out) {
    uint32_t x = 0; int d = 0;
    while (*s >= '0' && *s <= '9') {
        if (++d > 9) return 0;
        x = x * 10u + (uint32_t)(*s++ - '0');
    }
    if (!d || *s) return 0;
    *out = x; return 1;
}
static int oracle(unsigned mode, const char *s, uint32_t *out, unsigned n) {
    if (mode == 0) {
        for (; *s; ++s) if ((unsigned char)*s < 32 || (unsigned char)*s == 127) return 0;
        return 1;
    }
    if (mode == 1) {
        if (strlen(s) != 64) return 0;
        for (; *s; ++s) if (!((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f'))) return 0;
        return 1;
    }
    char copy[128]; size_t len = strlen(s);
    if (mode == 2 && len >= 64) return 0;
    if (len >= sizeof copy) return 0;
    memcpy(copy, s, len + 1);
    char *p = copy;
    for (unsigned i = 0; i < n; ++i) {
        char *q = strchr(p, mode == 3 ? '.' : ',');
        if (i + 1 < n) { if (!q) return 0; *q = 0; }
        else if (q) return 0;
        if (!decimal(p, out + i)) return 0;
        if (q) p = q + 1;
    }
    return 1;
}
static int omega(unsigned mode, const char *s, uint32_t *out, unsigned n) {
    uint64_t state[8] = {0}, status; state[3] = n;
    for (const unsigned char *p = (const unsigned char *)s;; ++p) {
        if (omega_artifact_parse(&status, mode, *p, state, 8) || status == 2) return 0;
        if (mode >= 2 && (status == 1 || status == 3)) out[state[2] - (status == 3)] = (uint32_t)state[4];
        if (status == 1) return 1;
    }
}
static void check(unsigned mode, const char *s, unsigned n, int *bad) {
    uint32_t a[8] = {0}, b[8] = {0};
    int ca = oracle(mode, s, a, n), cb = omega(mode, s, b, n);
    if (ca != cb || (ca && memcmp(a, b, n * sizeof *a))) {
        fprintf(stderr, "ARTIFACT TEXT differential FAIL mode=%u n=%u len=%zu\n", mode, n, strlen(s)); ++*bad;
    }
}
int main(void) {
    int bad = 0;
    const char *edges[] = {"", "0", "1", "999999999", "1000000000", "000000000", "0000000000", "-1", "+1", " 1", "1 ", "1,2,3", "1.2.3", "0.0.0", "999999999.999999999.999999999", "1.2.3.", "1.2", "1..3", "1,2,", "1,,3", "1,2,3,4", "\n", "\r", "\177"};
    for (unsigned mode = 0; mode < 4; ++mode)
        for (unsigned n = 1; n <= 8; ++n)
            for (unsigned i = 0; i < sizeof edges / sizeof *edges; ++i) check(mode, edges[i], n, &bad);
    char s[96];
    for (unsigned c = 1; c <= 255; ++c) {
        memset(s, 'a', 64); s[64] = 0; s[31] = (char)c;
        check(0, s, 1, &bad); check(1, s, 1, &bad);
    }
    uint64_t rng = UINT64_C(0xb709e28515);
    for (unsigned i = 0; i < 100000; ++i) {
        rng = rng * UINT64_C(6364136223846793005) + 1;
        unsigned len = (unsigned)(rng >> 32) % 90;
        for (unsigned j = 0; j < len; ++j) {
            rng = rng * UINT64_C(6364136223846793005) + 1;
            s[j] = (char)(rng >> 32);
        }
        s[len] = 0;
        for (unsigned mode = 0; mode < 4; ++mode) check(mode, s, 1 + i % 8, &bad);
    }
    if (bad) return 1;
    puts("ARTIFACT TEXT differential PASS"); return 0;
}
