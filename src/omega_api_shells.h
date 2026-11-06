/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Kernel-family API shell decisions. Default path is the om/api units.
 * Define CUDAPP_MIGRATION_PENDING_C_API_SHELLS to keep the previous C
 * expressions as the differential oracle. Pointer checks, locks, allocation
 * and launches stay in the C ABI shell. */
#ifndef OMEGA_API_SHELLS_H
#define OMEGA_API_SHELLS_H
#include "cudapp_native.h"
#include <stdint.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

static inline uint64_t apish_p(uint64_t op, uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
    uint64_t r = 0;
    if (omega_apishp(&r, op, a, b, c, d, e) != 0) return 0;
    return r;
}
static inline uint64_t apish_k(uint64_t op, uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
    uint64_t r = 0;
    if (omega_apishk(&r, op, a, b, c, d, e) != 0) return 0;
    return r;
}
static inline int32_t apish_l(uint64_t op, uint64_t aux, uint64_t *rec) {
    uint64_t r = 0;
    if (omega_apishl(&r, op, aux, rec, 16) != 0) return -1;
    return (int32_t)r;
}
static inline uint64_t apish_r16(const uint8_t *v) {
    uint64_t r = 0;
    if (omega_apishr(&r, v, 16) != 0) return 1ull << 32;
    return r;
}
static inline uint64_t apish_blk(const uint8_t *w, const uint8_t *x, uint32_t sc) {
    uint64_t r = 0;
    if (omega_apishb(&r, w, 128, x, 512, sc) != 0) return 0;
    return r;
}
static inline void apish_pair(uint64_t packed, uint32_t *lo, uint32_t *hi) {
    if (lo) *lo = (uint32_t)packed;
    if (hi) *hi = (uint32_t)(packed >> 32);
}

static inline uint32_t omega_api_lane(uint32_t k) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return k % 512u == 0u ? 16u : 8u;
#else
    return (uint32_t)apish_p(1, k, 0, 0, 0, 0);
#endif
}
static inline void omega_api_fp8_defaults(uint32_t mt, uint32_t wp, uint32_t *m, uint32_t *w) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (m) *m = mt ? mt : 4u;
    if (w) *w = wp ? wp : 16u;
#else
    apish_pair(apish_p(2, mt, wp, 0, 0, 0), m, w);
#endif
}
static inline uint32_t omega_api_fp8mt(uint32_t mt, uint32_t segs) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (mt) return mt;
    return segs < 8u ? 8u : 4u;
#else
    return (uint32_t)apish_p(30, mt, segs, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_ctabud(uint32_t segs, uint32_t ov) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (ov) return ov;
    return segs >= 64u ? 2u : segs >= 8u ? 12u : 96u;
#else
    return (uint32_t)apish_p(16, segs, ov, 0, 0, 0);
#endif
}
static inline int omega_api_regfit(uint32_t warps, uint32_t decl) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (uint64_t)32u * warps * decl <= 65536u;
#else
    return (int)apish_p(3, warps, decl, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_tgbm(uint32_t rt, uint32_t w, uint32_t n) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (rt) return 16u * rt;
    w = w ? w : (n % 256u ? 8u : 16u);
    return w == 16u ? 128u : 64u;
#else
    return (uint32_t)apish_p(4, rt, w, n, 0, 0);
#endif
}
static inline void omega_api_tgspec(uint32_t rt, uint32_t w, uint32_t n, uint32_t *r, uint32_t *ww) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint32_t resolved = w ? w : (n % 256u ? 8u : 16u);
    if (ww) *ww = resolved;
    if (r) *r = rt ? rt : (resolved == 16u ? 8u : 4u);
#else
    apish_pair(apish_p(5, rt, w, n, 0, 0), r, ww);
#endif
}
static inline uint32_t omega_api_tgsz(uint32_t m, uint32_t n, uint32_t bm) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (m == 0u || bm == 0u || m % bm) return 1u;
    if (m / bm > 65535u || (uint64_t)m * n >= (1ull << 32)) return 2u;
    return 0u;
#else
    return (uint32_t)apish_p(6, m, n, bm, 0, 0);
#endif
}
static inline void omega_api_ternd(uint32_t k, uint32_t lay, uint32_t R, uint32_t gr, uint32_t gd, uint32_t *rows, uint32_t *depth) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint32_t nch = (k + 8191u) / 8192u;
    uint32_t ksc = nch ? k / nch / 256u : 0u;
    uint32_t r = lay == 1u ? R : (gr ? gr : 1u);
    uint32_t d = gd ? gd : (r ? 16u / r : 0u);
    while (d > 1u && (ksc % d || r * d > 16u)) d--;
    if (rows) *rows = r;
    if (depth) *depth = d;
#else
    apish_pair(apish_p(7, k, lay, R, gr, gd), rows, depth);
#endif
}
static inline void omega_api_ternwc(uint32_t w, uint32_t ct, uint32_t *ww, uint32_t *cc) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (ww) *ww = w ? w : 8u;
    if (cc) *cc = ct ? ct : 160u;
#else
    apish_pair(apish_p(8, w, ct, 0, 0, 0), ww, cc);
#endif
}
static inline uint32_t omega_api_pad8(uint32_t n) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (n + 7u) / 8u * 8u;
#else
    return (uint32_t)apish_p(9, n, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_segb(uint32_t i, uint32_t cnt, uint32_t n, uint32_t en, uint32_t mu) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (i + 1u >= cnt) return n;
    if (mu == 2u) return cnt ? (i + 1u) * (n / cnt) : 0u;
    return en;
#else
    return (uint32_t)apish_p(10, i, cnt, n, en, mu);
#endif
}
static inline int omega_api_segend(uint32_t i, uint32_t cnt, uint32_t n, uint32_t en, uint32_t pv) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (cnt == 0u || cnt > 4u || en == 0u) return 1;
    if (i && en <= pv) return 1;
    if (i + 1u == cnt && en != n) return 1;
    return 0;
#else
    return (int)apish_p(11, i, cnt, n, en, pv);
#endif
}
static inline int omega_api_finite(uint32_t bits) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return ((bits >> 23) & 255u) != 255u;
#else
    return (int)apish_p(12, bits, 0, 0, 0, 0);
#endif
}
static inline uint64_t omega_api_pkbytes(uint32_t e, uint32_t k) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (uint64_t)(k / 4u + 1u) * e * 16u;
#else
    return apish_p(13, e, k, 0, 0, 0);
#endif
}
static inline int omega_api_alignok(uint32_t lo, uint32_t mask) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (lo & mask) == 0u;
#else
    return (int)apish_p(14, lo, mask, 0, 0, 0);
#endif
}
static inline int omega_api_tile8(uint32_t c0) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return c0 % 8u == 0u;
#else
    return (int)apish_p(15, c0, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_perm4(uint32_t q) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    static const uint32_t o[4] = { 0u, 2u, 1u, 3u };
    return (q & ~3u) | o[q & 3u];
#else
    return (uint32_t)apish_p(17, q, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_pfbm(uint32_t rt) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return 16u * (rt ? rt : 8u);
#else
    return (uint32_t)apish_p(18, rt, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_swsrc(uint32_t p, uint32_t n) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint32_t G = p / 16u, h = p % 16u, i = omega_api_perm4(8u * G + h % 8u);
    return h < 8u ? i : n / 2u + i;
#else
    return (uint32_t)apish_p(19, p, n, 0, 0, 0);
#endif
}
static inline int omega_api_maxmnk(uint32_t m, uint32_t k, uint32_t n) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (m == 0u || k == 0u || n == 0u) return 0;
    if (m > 8192u || k > 16384u || n > 262144u) return 0;
    return 1;
#else
    return (int)apish_p(20, m, k, n, 0, 0);
#endif
}
static inline int omega_api_dcache(uint32_t layers, uint32_t mx, uint32_t rot) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (layers == 0u || layers > 64u) return 1;
    if (mx == 0u || mx > 131072u || mx % 256u) return 1;
    if (rot < 2u || rot > 256u || rot % 2u) return 1;
    return 0;
#else
    return (int)apish_p(21, layers, mx, rot, 0, 0);
#endif
}
static inline int omega_api_dreg(uint32_t decl) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (uint64_t)256u * decl <= 65536u;
#else
    return (int)apish_p(22, decl, 0, 0, 0, 0);
#endif
}
static inline int omega_api_tpack(uint32_t n, uint32_t k, uint32_t lay, uint32_t R, uint32_t np) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint32_t step = lay == 1u ? R : 1u;
    if (n == 0u || k % 16u || np < n || step == 0u || np % (2u * step)) return 1;
    return 0;
#else
    return (int)apish_p(23, n, k, lay, R, np);
#endif
}
static inline uint32_t omega_api_rrows(uint32_t s) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return s ? s : 16u;
#else
    return (uint32_t)apish_p(24, s, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_rtdst(uint32_t g, uint32_t ex, uint32_t q, uint32_t e) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return ((g * e + ex) * 4u) + q;
#else
    return (uint32_t)apish_p(25, g, ex, q, e, 0);
#endif
}
static inline uint32_t omega_api_rtsrc(uint32_t ex, uint32_t k, uint32_t g, uint32_t q) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return ex * k + 4u * g + q;
#else
    return (uint32_t)apish_p(26, ex, k, g, q, 0);
#endif
}
static inline void omega_api_dsplit(uint32_t pos, uint32_t per, uint32_t mx, uint32_t *code, uint32_t *splits) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint32_t p = per ? per : 256u;
    if (p % 256u) { if (code) *code = 1u; return; }
    uint32_t sp = (pos + p) / p;
    if (sp > mx) { if (code) *code = 2u; return; }
    if (code) *code = 0u;
    if (splits) *splits = sp;
#else
    uint64_t v = apish_p(27, pos, per, mx, 0, 0);
    if (code) *code = (uint32_t)v;
    if (splits && (uint32_t)v == 0u) *splits = (uint32_t)(v >> 32);
#endif
}
static inline int omega_api_vy2(uint32_t v) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return v != 0u && (v & (v - 1u)) == 0u;
#else
    return (int)apish_p(28, v, 0, 0, 0, 0);
#endif
}
static inline int omega_api_thok(uint32_t lo, uint32_t hi) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint64_t bits = (uint64_t)lo | ((uint64_t)hi << 32);
    double t;
    memcpy(&t, &bits, sizeof t);
    return t > 0.0;
#else
    return (int)apish_p(29, lo, hi, 0, 0, 0);
#endif
}
static inline void omega_api_tcsp(uint32_t rt, uint32_t ct, uint32_t n, uint32_t *r, uint32_t *c) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (!rt) rt = 4u;
    if (!ct) ct = n % 128u == 0u ? 2u : 1u;
    if (r) *r = rt;
    if (c) *c = ct;
#else
    apish_pair(apish_p(31, rt, ct, n, 0, 0), r, c);
#endif
}
static inline uint32_t omega_api_tcw(uint32_t w) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return w ? w : 8u;
#else
    return (uint32_t)apish_p(32, w, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_rup(uint32_t v, uint32_t q) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return q ? ((v + q - 1u) / q) * q : 0u;
#else
    return (uint32_t)apish_p(33, v, q, 0, 0, 0);
#endif
}
static inline uint64_t omega_api_woff(uint32_t kp, uint32_t col0) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (uint64_t)(col0 / 8u) * kp * 16u;
#else
    return apish_p(34, kp, col0, 0, 0, 0);
#endif
}
static inline int omega_api_wok(uint32_t c0, uint32_t cols, uint32_t n) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (c0 % 8u || cols == 0u || c0 > n || cols > n - c0) return 0;
    return 1;
#else
    return (int)apish_p(35, c0, cols, n, 0, 0);
#endif
}
static inline uint32_t omega_api_qgrp(uint32_t kb) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint32_t g = 32u;
    while (g > 0u && kb % g) g--;
    return g;
#else
    return (uint32_t)apish_p(36, kb, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_refb(uint8_t b) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint32_t e = (b >> 3) & 15u, m = b & 7u, s = b & 128u;
    float z;
    if (e == 15u && m == 7u) return 2143289344u;
    if (e == 0u) {
        z = (float)m * 1.953125e-3f;
        if (s) { uint32_t u; memcpy(&u, &z, 4); return u ^ 0x80000000u; }
        memcpy(&e, &z, 4); return e;
    }
    z = (1.0f + (float)m / 8.0f) * (float)(1u << (e > 7u ? e - 7u : 0u));
    if (e < 7u) z = (1.0f + (float)m / 8.0f) / (float)(1u << (7u - e));
    if (s) { uint32_t u; memcpy(&u, &z, 4); return u ^ 0x80000000u; }
    memcpy(&e, &z, 4); return e;
#else
    return (uint32_t)apish_k(1, b, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_kbits(uint8_t b) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return ((uint32_t)(b & 0x7fu) << 20) | ((uint32_t)(b & 0x80u) << 24);
#else
    return (uint32_t)apish_k(2, b, 0, 0, 0, 0);
#endif
}
static inline int omega_api_isnanb(uint8_t b) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (b & 0x7fu) == 0x7fu;
#else
    return (int)apish_k(3, b, 0, 0, 0, 0);
#endif
}
static inline int omega_api_scalef(uint16_t bf16, int mutant, uint32_t *out) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint32_t ub = (uint32_t)bf16 << 16, ug;
    float s, f, fold = mutant ? 1.0f : 72057594037927936.0f;
    memcpy(&s, &ub, 4);
    if (!(s <= 3.0e38f && s >= -3.0e38f)) return -1;
    f = s * fold;
    if (!(f <= 3.0e38f && f >= -3.0e38f)) return -1;
    memcpy(&ug, &f, 4);
    if (out) *out = ug;
    return 0;
#else
    uint64_t r = apish_k(4, bf16, (uint64_t)(mutant != 0), 0, 0, 0);
    if (r & (1ull << 32)) return -1;
    if (out) *out = (uint32_t)r;
    return 0;
#endif
}
static inline uint32_t omega_api_xfold(uint32_t bits) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    float x, f;
    uint32_t u;
    memcpy(&x, &bits, 4);
    f = x * 18446744073709551616.0f;
    memcpy(&u, &f, 4);
    return u;
#else
    return (uint32_t)apish_k(5, bits, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_xfold16(uint16_t bf16) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return omega_api_xfold((uint32_t)bf16 << 16);
#else
    return (uint32_t)apish_k(6, bf16, 0, 0, 0, 0);
#endif
}
static inline int32_t omega_api_slot(uint64_t op, uint64_t aux, uint64_t rec[16]) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint64_t st = rec[0], pins = rec[1];
    if (op == 1u) {
        if (st == 1u || (pins && rec[5] == 0u)) { rec[10]++; return -6; }
        rec[0] = 1u; return 0;
    }
    if (op == 2u) {
        if (st != 1u) return -1;
        if (rec[6]) {
            rec[0] = 2u; rec[2] = aux; rec[3] = rec[4]; rec[4]++; rec[8]++; return 0;
        }
        rec[0] = 0u; rec[2] = 0u; rec[9]++; return 0;
    }
    if (op == 3u) {
        if (st != 2u || rec[3] != aux || aux == 0u) { rec[11]++; return -7; }
        rec[1]++; rec[13]++; rec[14]++;
        if (rec[14] > rec[15]) rec[15] = rec[14];
        return 0;
    }
    if (op == 4u) {
        if (pins == 0u || (rec[5] == 0u && rec[3] != aux)) return -1;
        rec[1]--; rec[14]--; return 0;
    }
    if (op == 5u) {
        if (pins || st == 1u) { rec[10]++; return -6; }
        if (st == 2u) rec[12]++;
        rec[0] = 0u; rec[2] = 0u; return 0;
    }
    if (op == 6u) return (st == 1u || pins) ? 0 : 1;
    if (op == 7u) {
        if (st != 2u || rec[3] != aux || aux == 0u) return -7;
        if (pins == 0u) return -1;
        return 0;
    }
    return -1;
#else
    return apish_l(op, aux, rec);
#endif
}
static inline uint64_t omega_api_trit(const uint8_t v[16]) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint64_t word = 0;
    for (unsigned i = 0; i < 16u; i++) {
        uint32_t code = v[i] == 1u ? 1u : v[i] == 255u ? 2u : v[i] == 0u ? 0u : 3u;
        if (code == 3u) return 1ull << 32;
        word |= (uint64_t)code << (2u * i);
    }
    return word;
#else
    return apish_r16(v);
#endif
}
static inline uint64_t omega_api_block(const uint8_t w[128], const uint8_t x[512], uint32_t sc) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    float sf;
    uint32_t ub = sc << 16;
    double acc = 0.0;
    memcpy(&sf, &ub, 4);
    for (unsigned i = 0; i < 128u; i++) {
        uint32_t rb = omega_api_refb(w[i]);
        uint32_t xb = (uint32_t)x[4u * i] | ((uint32_t)x[4u * i + 1] << 8) | ((uint32_t)x[4u * i + 2] << 16) | ((uint32_t)x[4u * i + 3] << 24);
        float rv, xv;
        memcpy(&rv, &rb, 4);
        memcpy(&xv, &xb, 4);
        acc += (double)rv * (double)xv * (double)sf;
    }
    uint64_t bits;
    memcpy(&bits, &acc, 8);
    return bits;
#else
    return apish_blk(w, x, sc);
#endif
}
static inline uint32_t omega_api_qrt(uint32_t rt) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return rt ? rt : 4u;
#else
    return (uint32_t)apish_p(37, rt, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_qwait(uint32_t set, uint32_t w) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return set ? w : 45u;
#else
    return (uint32_t)apish_p(38, set, w, 0, 0, 0);
#endif
}
static inline int omega_api_le128(uint32_t decl) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return decl <= 128u;
#else
    return apish_p(39, decl, 0, 0, 0, 0) != 0u;
#endif
}
static inline int omega_api_roomok(uint32_t n, uint32_t pos, uint32_t mx) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (n == 0u || pos > mx || n > mx - pos) return 1;
    return 0;
#else
    return (int)apish_p(40, n, pos, mx, 0, 0);
#endif
}
static inline int omega_api_pa256b(uint32_t m, uint32_t pos, uint32_t mx) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (m > 32768u) return 1;
    return omega_api_roomok(m, pos, mx);
#else
    return (int)apish_p(46, m, pos, mx, 0, 0);
#endif
}
static inline int omega_api_qydiv(uint32_t qh, uint32_t pc, uint32_t *q) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (pc == 0u) return 1;
    if (q) *q = qh / pc;
    return 0;
#else
    uint64_t r = apish_p(41, qh, pc, 0, 0, 0);
    if (r >> 32) return 1;
    if (q) *q = (uint32_t)r;
    return 0;
#endif
}
static inline uint64_t omega_api_qfbyt(uint32_t rows, uint32_t qh) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (uint64_t)((rows + 1u) / 2u) * qh * 1024u;
#else
    return apish_p(42, rows, qh, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_gxatt(uint32_t m) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (m + 7u) / 8u;
#else
    return (uint32_t)apish_p(43, m, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_kvsb(uint32_t mx) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return mx * 512u;
#else
    return (uint32_t)apish_p(44, mx, 0, 0, 0, 0);
#endif
}
static inline uint64_t omega_api_hostb(uint32_t n, uint32_t w) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (uint64_t)n * w * 4u;
#else
    return apish_p(45, n, w, 0, 0, 0);
#endif
}
static inline int omega_api_vy16(uint32_t v) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return v > 16u;
#else
    return (int)apish_p(47, v, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_splt(uint32_t mx) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return mx / 256u;
#else
    return (uint32_t)apish_p(48, mx, 0, 0, 0, 0);
#endif
}
static inline uint64_t omega_api_plane(uint32_t kvh, uint32_t mx) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (uint64_t)kvh * mx * 512u + 8256u;
#else
    return apish_p(49, kvh, mx, 0, 0, 0);
#endif
}
static inline int omega_api_persok(uint32_t p) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return p == 0u || p == 256u;
#else
    return (int)apish_p(50, p, 0, 0, 0, 0);
#endif
}
static inline int omega_api_stride(uint32_t need, uint32_t got, uint32_t mask) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (got < need || (got & mask)) return 1;
    return 0;
#else
    return (int)apish_p(51, need, got, mask, 0, 0);
#endif
}
static inline uint64_t omega_api_mtgrp(uint32_t left, uint32_t p0, uint32_t t0, uint32_t vy) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    uint32_t n = left > 128u ? 128u : left;
    uint32_t pos = p0 + t0;
    while (n > 1u && (uint64_t)n * vy * ((pos + n - 1u) / 256u + 1u) * 6192u > 67108864u) n--;
    uint32_t smax = (pos + n - 1u) / 256u + 1u;
    return (uint64_t)n | ((uint64_t)smax << 32);
#else
    return apish_p(52, left, p0, t0, vy, 0);
#endif
}
static inline int omega_api_mtbig(uint32_t n, uint32_t vy, uint32_t sm) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (uint64_t)n * vy * sm * 6192u > 67108864u;
#else
    return (int)apish_p(53, n, vy, sm, 0, 0);
#endif
}
static inline uint32_t omega_api_smaxp(uint32_t pos, uint32_t n) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (pos + n - 1u) / 256u + 1u;
#else
    return (uint32_t)apish_p(54, pos, n, 0, 0, 0);
#endif
}
static inline uint64_t omega_api_mtps(uint32_t vy, uint32_t sm) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return (uint64_t)vy * sm * 6192u;
#else
    return apish_p(55, vy, sm, 0, 0, 0);
#endif
}
static inline int omega_api_p2bad(uint32_t v) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return v == 0u || (v & (v - 1u)) != 0u;
#else
    return (int)apish_p(56, v, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_nkbad(uint32_t n, uint32_t k) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if (n == 0u || n > 0x40000000u || k == 0u || (k % 256u) != 0u) return 1u;
    if (k > 16384u || (k > 8192u && (k % 512u) != 0u)) return 2u;
    return 0u;
#else
    return (uint32_t)apish_p(57, n, k, 0, 0, 0);
#endif
}
static inline int omega_api_layok(uint32_t lay, uint32_t R) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return lay > 1u || (lay == 1u && R != 2u && R != 4u);
#else
    return (int)apish_p(58, lay, R, 0, 0, 0);
#endif
}
static inline int omega_api_tritok(uint32_t v) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return v == 0u || v == 1u || v == 255u;
#else
    return (int)apish_p(59, v, 0, 0, 0, 0);
#endif
}
static inline uint32_t omega_api_qset(uint32_t k) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    if ((k % 128u) != 0u) return 0u;
    uint32_t kb = k / 128u;
    if (kb <= 32u) return 0u;
    uint32_t g = 32u;
    while (g > 0u && (kb % g) != 0u) g--;
    return g;
#else
    return (uint32_t)apish_p(60, k, 0, 0, 0, 0);
#endif
}
static inline int omega_api_pfargs(uint32_t n, uint32_t k, uint32_t lay) {
#ifdef CUDAPP_MIGRATION_PENDING_C_API_SHELLS
    return n == 0u || k == 0u || (n % 128u) != 0u || (k % 128u) != 0u || lay > 1u || (lay == 1u && (n % 256u) != 0u);
#else
    return (int)apish_p(61, n, k, lay, 0, 0);
#endif
}

#ifdef __cplusplus
}
#endif
#endif
