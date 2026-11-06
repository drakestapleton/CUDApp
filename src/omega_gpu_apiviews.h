/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host data loops for the elementwise, matmul, gdn, fp8 and scan API residue.
 * Omega (om/apiviews) is the default. The old C compiles only when
 * CUDAPP_MIGRATION_PENDING_APIVIEWS is set (the differential oracle) or
 * CUDAPP_MIGRATION_PENDING_APIVIEWS_DIFF is set (both, abort on mismatch).
 * Pointer acquisition and the launch stay with the caller. */
#ifndef OMEGA_GPU_APIVIEWS_H
#define OMEGA_GPU_APIVIEWS_H

#include "cudapp_native.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(CUDAPP_MIGRATION_PENDING_APIVIEWS_DIFF)
#define AV_DIFF 1
#define AV_C 1
#define AV_OM 1
#elif defined(CUDAPP_MIGRATION_PENDING_APIVIEWS)
#define AV_DIFF 0
#define AV_C 1
#define AV_OM 0
#else
#define AV_DIFF 0
#define AV_C 0
#define AV_OM 1
#endif

static inline void av_die(const char *what) {
    fprintf(stderr, "FAIL apiviews C-vs-Omega differential: %s\n", what);
    abort();
}

#if defined(CUDAPP_MIGRATION_PENDING_APIVIEWS_DIFF_SELFTEST)
static int av_flipped;
static inline void av_flip(void *p, size_t n) {
    if (av_flipped || !p || n == 0) return;
    ((unsigned char *)p)[0] ^= 1u;
    av_flipped = 1;
}
#else
static inline void av_flip(void *p, size_t n) { (void)p; (void)n; }
#endif

static inline int av_u32_eq(uint32_t a, uint32_t b) { return a == b; }

static inline int av_f32_eq(float a, float b) {
    uint32_t ua, ub;
    memcpy(&ua, &a, 4);
    memcpy(&ub, &b, 4);
    if (ua == ub) return 1;
    /* NaN by class only. 0x7fffffff is the CUDApp quiet-NaN comparison convention. */
    if ((ua & 0x7fffffffu) > 0x7f800000u && (ub & 0x7fffffffu) > 0x7f800000u) return 1;
    return 0;
}

static inline void av_need(uint32_t st, uint64_t res) {
    if (st != OMEGA_OK || res != 0) av_die("omega unit refused");
}

#if AV_C
static uint8_t av_e4m3_c(float v) {
    uint32_t u;
    memcpy(&u, &v, 4);
    const uint8_t s = (uint8_t)((u >> 24) & 0x80u);
    u &= 0x7FFFFFFFu;
    float a;
    memcpy(&a, &u, 4);
    if (!(a < 448.0f)) return (uint8_t)(s | 0x7Eu);
    const int ex = (int)(u >> 23) - 127;
    if (ex < -6) {
        const float x = a * 512.0f;
        uint32_t xi = (uint32_t)x;
        const float fr = x - (float)xi;
        if (fr > 0.5f || (fr == 0.5f && (xi & 1u))) xi++;
        return (uint8_t)(s | xi);
    }
    const uint32_t m = u & 0x7FFFFFu, rb = (m >> 19) & 1u, st = m & 0x7FFFFu;
    uint32_t q = m >> 20, e = (uint32_t)(ex + 7);
    if (rb && (st || (q & 1u))) q++;
    if (q == 8u) { q = 0; e++; }
    uint32_t code = (e << 3) | q;
    if (code > 0x7Eu) code = 0x7Eu;
    return (uint8_t)(s | code);
}

static uint16_t av_bf16_c(uint32_t md, uint32_t u) {
    if (md == 0) {
        if ((u & 0x7f800000u) == 0x7f800000u && (u & 0x007fffffu)) return (uint16_t)((u >> 16) | 0x0040u);
        return (uint16_t)((u + 0x7fffu + ((u >> 16) & 1u)) >> 16);
    }
    if ((u & 0x7fffffffu) > 0x7f800000u) return 0x7fc0u;
    return (uint16_t)((u + 0x7fffu + ((u >> 16) & 1u)) >> 16);
}

static uint32_t av_gdn_channel_c(uint32_t h, uint32_t s, uint32_t vh) {
    const uint32_t hk = h / (vh / 16u), D = 128u;
    if (s < D) return hk * D + s;
    if (s < 2u * D) return 16u * D + hk * D + (s - D);
    return 2u * 16u * D + h * D + (s - 2u * D);
}
#endif

static inline uint8_t av_e4m3(float v) {
#if AV_C
    uint8_t c = av_e4m3_c(v);
#endif
#if AV_OM
    uint32_t bits;
    memcpy(&bits, &v, 4);
    uint64_t code = 0;
    uint32_t st = omega_av_e4m3(&code, bits);
    if (st != OMEGA_OK) av_die("omega unit refused");
    uint8_t o = (uint8_t)code;
    av_flip(&o, 1);
#endif
#if AV_DIFF
    if (c != o) av_die("output bytes differ");
    return c;
#elif AV_C
    return c;
#else
    return o;
#endif
}

static inline int av_bf16_n(uint64_t md, uint64_t n, const float *x, uint16_t *y) {
#if AV_C
    for (uint64_t i = 0; i < n; i++) {
        uint32_t u;
        memcpy(&u, &x[i], 4);
        y[i] = av_bf16_c((uint32_t)md, u);
    }
#endif
#if AV_OM
    uint64_t res = 1;
    uint16_t *tmp = y;
#if AV_DIFF
    tmp = malloc((size_t)n * 2u);
    if (!tmp) av_die("omega unit refused");
#endif
    uint32_t st = omega_av_bf16(&res, md, x, n, tmp, n);
    av_need(st, res);
    av_flip(tmp, (size_t)n * 2u);
#if AV_DIFF
    if (memcmp(y, tmp, (size_t)n * 2u) != 0) av_die("output bytes differ");
    free(tmp);
#endif
#endif
    return 0;
}

static inline int av_widen_n(uint64_t n, const uint16_t *x, float *y) {
#if AV_C
    for (uint64_t i = 0; i < n; i++) {
        uint32_t b = (uint32_t)x[i] << 16;
        memcpy(&y[i], &b, 4);
    }
#endif
#if AV_OM
    uint64_t res = 1;
    float *tmp = y;
#if AV_DIFF
    tmp = malloc((size_t)n * 4u);
    if (!tmp) av_die("omega unit refused");
#endif
    uint32_t st = omega_av_widen(&res, x, n, tmp, n);
    av_need(st, res);
    av_flip(tmp, (size_t)n * 4u);
#if AV_DIFF
    for (uint64_t i = 0; i < n; i++) if (!av_f32_eq(y[i], tmp[i])) av_die("output bytes differ");
    free(tmp);
#endif
#endif
    return 0;
}

static inline uint32_t av_gdn_channel(uint32_t h, uint32_t s, uint32_t vh) {
#if AV_C
    uint32_t c = av_gdn_channel_c(h, s, vh);
#endif
#if AV_OM
    uint64_t ch = 0;
    uint32_t st = omega_av_gdnch(&ch, h, s, vh);
    if (st != OMEGA_OK) av_die("omega unit refused");
    uint32_t o = (uint32_t)ch;
    av_flip(&o, 4);
#endif
#if AV_DIFF
    if (c != o) av_die("output bytes differ");
    return c;
#elif AV_C
    return c;
#else
    return o;
#endif
}

static inline void av_gdn_slot(float *dst, const float src[3]) {
#if AV_C
    float c[4] = {src[0], src[1], src[2], 0.0f};
#endif
#if AV_OM
    uint64_t res = 1;
    float o[4];
    uint32_t st = omega_av_gdnsl(&res, src, 3, o, 4);
    av_need(st, res);
    av_flip(o, sizeof o);
#if AV_DIFF
    for (int i = 0; i < 4; i++) if (!av_f32_eq(c[i], o[i])) av_die("output bytes differ");
    memcpy(dst, c, sizeof c);
#else
    memcpy(dst, o, sizeof o);
#endif
#else
    memcpy(dst, c, sizeof c);
#endif
}

static inline int av_gdn_scales(uint64_t n, const float *ng, const float *dt, float *sc) {
#if AV_C
    for (uint64_t i = 0; i < n; i++) {
        sc[2 * i] = ng[i];
        sc[2 * i + 1] = dt[i];
    }
#endif
#if AV_OM
    uint64_t res = 1;
    float *tmp = sc;
#if AV_DIFF
    tmp = malloc((size_t)n * 2u * 4u);
    if (!tmp) av_die("omega unit refused");
#endif
    uint32_t st = omega_av_gdnsc(&res, ng, n, dt, n, tmp, n * 2u);
    av_need(st, res);
    av_flip(tmp, (size_t)n * 8u);
#if AV_DIFF
    for (uint64_t i = 0; i < n * 2u; i++) if (!av_f32_eq(sc[i], tmp[i])) av_die("output bytes differ");
    free(tmp);
#endif
#endif
    return 0;
}

static inline void av_fp8_group(uint64_t ent[2], uint64_t id, uint64_t wv, uint64_t sv, uint64_t ws, uint64_t ss) {
#if AV_C
    uint64_t c0 = wv + id * ws, c1 = sv + id * ss;
#endif
#if AV_OM
    uint64_t res = 1, o[2];
    uint32_t st = omega_av_fp8en(&res, id, wv, sv, ws, ss, o, 2);
    av_need(st, res);
    av_flip(o, sizeof o);
#endif
#if AV_DIFF
    if (c0 != o[0] || c1 != o[1]) av_die("output bytes differ");
    ent[0] = c0;
    ent[1] = c1;
#elif AV_C
    ent[0] = c0;
    ent[1] = c1;
#else
    ent[0] = o[0];
    ent[1] = o[1];
#endif
}

static inline int av_rope_head(uint64_t n, const float *cs, const float *sn, float *tab) {
#if AV_C
    for (uint64_t i = 0; i < n; i++) {
        tab[i] = cs[i];
        tab[i + n] = sn[i];
    }
#endif
#if AV_OM
    uint64_t res = 1;
    float *tmp = tab;
#if AV_DIFF
    tmp = malloc((size_t)n * 2u * 4u);
    if (!tmp) av_die("omega unit refused");
#endif
    uint32_t st = omega_av_rope(&res, cs, n, sn, n, tmp, n * 2u);
    av_need(st, res);
    av_flip(tmp, (size_t)n * 8u);
#if AV_DIFF
    for (uint64_t i = 0; i < n * 2u; i++) if (!av_f32_eq(tab[i], tmp[i])) av_die("output bytes differ");
    free(tmp);
#endif
#endif
    return 0;
}

static inline int av_ew_rec(uint64_t kind, uint64_t n, uint64_t p0, uint64_t p1, uint64_t rec[8]) {
#if AV_C
    uint64_t c[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int cref = 0;
    if (n == 0) cref = 1;
    else if (kind == 0) {
        uint64_t ni = n > 131072u ? 131072u : n;
        c[0] = ni;
        c[1] = (ni + 127u) / 128u;
        c[2] = ni;
        c[6] = 128u;
    } else if (kind == 1) {
        if (p0 < 2 || (p0 & 1u) || p0 > 2048u) cref = 1;
        else {
            uint64_t nh = n > 1024u ? 1024u : n;
            uint64_t hf = p0 / 2u;
            c[0] = nh;
            c[1] = hf;
            c[2] = p0;
            c[3] = hf;
            c[6] = hf;
            c[7] = nh;
        }
    } else if (kind == 2) {
        if (p1 == 0 || p1 > 16384u) cref = 1;
        else {
            uint64_t nr = n > 1024u ? 1024u : n;
            float inv = 1.0f / (float)p1;
            uint32_t ib;
            memcpy(&ib, &inv, 4);
            c[0] = nr;
            c[1] = p1 / 128u;
            c[2] = ib;
            c[3] = p0;
            c[4] = p1;
            c[6] = 128u;
            c[7] = nr;
        }
    } else cref = 1;
#endif
#if AV_OM
    uint64_t res = 1, o[8];
    memset(o, 0, sizeof o);
    uint32_t st = omega_av_ewarg(&res, kind, n, p0, p1, o, 8);
    if (st != OMEGA_OK) av_die("omega unit refused");
    av_flip(&res, 8);
#endif
#if AV_DIFF
    if ((uint64_t)cref != res) av_die("output bytes differ");
    if (!cref && memcmp(c, o, sizeof c) != 0) av_die("output bytes differ");
    if (cref) return 1;
    memcpy(rec, c, sizeof c);
    return 0;
#elif AV_C
    if (cref) return 1;
    memcpy(rec, c, sizeof c);
    return 0;
#else
    if (res != 0) return 1;
    memcpy(rec, o, sizeof o);
    return 0;
#endif
}

static inline int av_scan_tile(uint64_t op, uint64_t n, const float *in, float out[32]) {
    if (n == 0 || n > 32 || op > 3) return 1;
#if AV_C
    float c[32], pad;
    uint32_t padb = (op == 0 || op == 3) ? 0x80000000u : 0x7fc00000u;
    memcpy(&pad, &padb, 4);
    for (uint64_t i = 0; i < 32; i++) c[i] = i < n ? in[i] : pad;
    if (op == 3) {
        for (uint64_t t = 0; t < 5; t++) {
            uint64_t d = 1ull << t;
            for (uint64_t i = 0; i < 32; i++) {
                uint64_t k = 31u - i;
                if (k >= d) c[k] = c[k - d] + c[k];
            }
        }
    } else {
        for (uint64_t t = 0; t < 5; t++) {
            uint64_t d = 16ull >> t;
            for (uint64_t i = 0; i + d < 32; i++) {
                float a = c[i], b = c[i + d], r;
                if (op == 0) r = a + b;
                else if (op == 1) r = __builtin_fmaxf(a, b);
                else r = __builtin_fminf(a, b);
                c[i] = r;
            }
        }
    }
#endif
#if AV_OM
    float inn[32], o[32];
    memset(inn, 0, sizeof inn);
    for (uint64_t i = 0; i < n; i++) inn[i] = in[i];
    uint64_t res = 1;
    uint32_t st = omega_av_scan(&res, op, n, inn, 32, o, 32);
    av_need(st, res);
    av_flip(o, sizeof o);
#endif
#if AV_DIFF
    for (int i = 0; i < 32; i++) if (!av_f32_eq(c[i], o[i])) av_die("output bytes differ");
    memcpy(out, c, sizeof c);
#elif AV_C
    memcpy(out, c, sizeof c);
#else
    memcpy(out, o, sizeof o);
#endif
    return 0;
}

/* Block iteration stays here: it is pointer acquisition. The amax, the scale and
 * the 128-wide quantize are the units. */
static inline int av_fp8_quantize(uint32_t n, uint32_t k, const uint16_t *w, uint8_t *w8, uint16_t *scale) {
    if (!w || !w8 || !scale || n == 0 || k == 0 || (n % 128u) || (k % 128u)) return 1;
    const uint32_t kb = k / 128u;
    for (uint32_t bn = 0; bn < n / 128u; bn++) {
        for (uint32_t bk = 0; bk < kb; bk++) {
#if AV_C
            float amax = 0.0f;
            int bad = 0;
            uint16_t sb = 0x3F80u;
            uint8_t rowb[128][128];
            for (uint32_t r = 0; r < 128u && !bad; r++) {
                const uint16_t *row = w + ((size_t)bn * 128u + r) * k + (size_t)bk * 128u;
                for (uint32_t c = 0; c < 128u; c++) {
                    uint32_t b = (uint32_t)(row[c] & 0x7FFFu) << 16;
                    float f;
                    memcpy(&f, &b, 4);
                    if (!(f <= 3.0e38f)) { bad = 1; break; }
                    if (f > amax) amax = f;
                }
            }
            if (!bad && amax > 0.0f) {
                const float q = amax / 448.0f;
                uint32_t b;
                memcpy(&b, &q, 4);
                sb = (uint16_t)((b >> 16) + ((b & 0xFFFFu) ? 1u : 0u));
            }
            if (!bad) {
                uint32_t sbits = (uint32_t)sb << 16;
                float sc;
                memcpy(&sc, &sbits, 4);
                for (uint32_t r = 0; r < 128u; r++) {
                    const size_t at = ((size_t)bn * 128u + r) * k + (size_t)bk * 128u;
                    for (uint32_t c = 0; c < 128u; c++) {
                        uint32_t b = (uint32_t)w[at + c] << 16;
                        float f;
                        memcpy(&f, &b, 4);
                        rowb[r][c] = av_e4m3_c(f / sc);
                    }
                }
            }
#endif
#if AV_OM
            float acc = 0.0f;
            int obad = 0;
            uint16_t osb = 0;
            uint8_t orow[128][128];
            for (uint32_t r = 0; r < 128u && !obad; r++) {
                const uint16_t *row = w + ((size_t)bn * 128u + r) * k + (size_t)bk * 128u;
                uint64_t res = 1;
                uint32_t st = omega_av_fp8am(&res, 128, row, 128, &acc, 1);
                if (st != OMEGA_OK || res == 1) av_die("omega unit refused");
                if (res == 2) obad = 1;
            }
            if (!obad) {
                uint32_t abits;
                memcpy(&abits, &acc, 4);
                uint64_t sb64 = 0;
                uint32_t st = omega_av_fp8sc(&sb64, abits);
                if (st != OMEGA_OK) av_die("omega unit refused");
                osb = (uint16_t)sb64;
                for (uint32_t r = 0; r < 128u; r++) {
                    const size_t at = ((size_t)bn * 128u + r) * k + (size_t)bk * 128u;
                    const uint16_t *row = w + at;
                    uint64_t res = 1;
                    st = omega_av_fp8q(&res, 128, osb, row, 128, orow[r], 128);
                    av_need(st, res);
                }
                av_flip(&osb, 2);
                av_flip(orow, sizeof orow);
            }
#endif
#if AV_DIFF
            if (bad != obad) av_die("output bytes differ");
            if (bad) return 1;
            if (sb != osb) av_die("output bytes differ");
            for (uint32_t r = 0; r < 128u; r++) {
                const size_t at = ((size_t)bn * 128u + r) * k + (size_t)bk * 128u;
                if (memcmp(rowb[r], orow[r], 128) != 0) av_die("output bytes differ");
                memcpy(w8 + at, rowb[r], 128);
            }
            scale[(size_t)bn * kb + bk] = sb;
#elif AV_C
            if (bad) return 1;
            scale[(size_t)bn * kb + bk] = sb;
            for (uint32_t r = 0; r < 128u; r++) {
                const size_t at = ((size_t)bn * 128u + r) * k + (size_t)bk * 128u;
                memcpy(w8 + at, rowb[r], 128);
            }
#else
            if (obad) return 1;
            scale[(size_t)bn * kb + bk] = osb;
            for (uint32_t r = 0; r < 128u; r++) {
                const size_t at = ((size_t)bn * 128u + r) * k + (size_t)bk * 128u;
                memcpy(w8 + at, orow[r], 128);
            }
#endif
        }
    }
    return 0;
}

#endif
