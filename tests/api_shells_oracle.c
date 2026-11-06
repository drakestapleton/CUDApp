/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Pending C formulas, compiled as the differential oracle. */
#define CUDAPP_MIGRATION_PENDING_C_API_SHELLS
#include "omega_api_shells.h"

uint32_t c_lane(uint32_t k) { return omega_api_lane(k); }
void c_fp8def(uint32_t mt, uint32_t wp, uint32_t *m, uint32_t *w) { omega_api_fp8_defaults(mt, wp, m, w); }
uint32_t c_fp8mt(uint32_t mt, uint32_t segs) { return omega_api_fp8mt(mt, segs); }
uint32_t c_ctabud(uint32_t segs, uint32_t ov) { return omega_api_ctabud(segs, ov); }
int c_regfit(uint32_t w, uint32_t d) { return omega_api_regfit(w, d); }
uint32_t c_tgbm(uint32_t rt, uint32_t w, uint32_t n) { return omega_api_tgbm(rt, w, n); }
void c_tgspec(uint32_t rt, uint32_t w, uint32_t n, uint32_t *r, uint32_t *ww) { omega_api_tgspec(rt, w, n, r, ww); }
uint32_t c_tgsz(uint32_t m, uint32_t n, uint32_t bm) { return omega_api_tgsz(m, n, bm); }
void c_ternd(uint32_t k, uint32_t lay, uint32_t R, uint32_t gr, uint32_t gd, uint32_t *rows, uint32_t *depth) {
    omega_api_ternd(k, lay, R, gr, gd, rows, depth);
}
void c_ternwc(uint32_t w, uint32_t ct, uint32_t *ww, uint32_t *cc) { omega_api_ternwc(w, ct, ww, cc); }
uint32_t c_pad8(uint32_t n) { return omega_api_pad8(n); }
uint32_t c_segb(uint32_t i, uint32_t cnt, uint32_t n, uint32_t en, uint32_t mu) { return omega_api_segb(i, cnt, n, en, mu); }
int c_segend(uint32_t i, uint32_t cnt, uint32_t n, uint32_t en, uint32_t pv) { return omega_api_segend(i, cnt, n, en, pv); }
int c_finite(uint32_t bits) { return omega_api_finite(bits); }
uint64_t c_pkbytes(uint32_t e, uint32_t k) { return omega_api_pkbytes(e, k); }
int c_alignok(uint32_t lo, uint32_t mask) { return omega_api_alignok(lo, mask); }
int c_tile8(uint32_t c0) { return omega_api_tile8(c0); }
uint32_t c_perm4(uint32_t q) { return omega_api_perm4(q); }
uint32_t c_pfbm(uint32_t rt) { return omega_api_pfbm(rt); }
uint32_t c_swsrc(uint32_t p, uint32_t n) { return omega_api_swsrc(p, n); }
int c_maxmnk(uint32_t m, uint32_t k, uint32_t n) { return omega_api_maxmnk(m, k, n); }
int c_dcache(uint32_t layers, uint32_t mx, uint32_t rot) { return omega_api_dcache(layers, mx, rot); }
int c_dreg(uint32_t decl) { return omega_api_dreg(decl); }
int c_tpack(uint32_t n, uint32_t k, uint32_t lay, uint32_t R, uint32_t np) { return omega_api_tpack(n, k, lay, R, np); }
uint32_t c_rrows(uint32_t s) { return omega_api_rrows(s); }
uint32_t c_rtdst(uint32_t g, uint32_t ex, uint32_t q, uint32_t e) { return omega_api_rtdst(g, ex, q, e); }
uint32_t c_rtsrc(uint32_t ex, uint32_t k, uint32_t g, uint32_t q) { return omega_api_rtsrc(ex, k, g, q); }
void c_dsplit(uint32_t pos, uint32_t per, uint32_t mx, uint32_t *code, uint32_t *splits) { omega_api_dsplit(pos, per, mx, code, splits); }
int c_vy2(uint32_t v) { return omega_api_vy2(v); }
int c_thok(uint32_t lo, uint32_t hi) { return omega_api_thok(lo, hi); }
void c_tcsp(uint32_t rt, uint32_t ct, uint32_t n, uint32_t *r, uint32_t *c) { omega_api_tcsp(rt, ct, n, r, c); }
uint32_t c_tcw(uint32_t w) { return omega_api_tcw(w); }
uint32_t c_rup(uint32_t v, uint32_t q) { return omega_api_rup(v, q); }
uint64_t c_woff(uint32_t kp, uint32_t col0) { return omega_api_woff(kp, col0); }
int c_wok(uint32_t c0, uint32_t cols, uint32_t n) { return omega_api_wok(c0, cols, n); }
uint32_t c_qgrp(uint32_t kb) { return omega_api_qgrp(kb); }
uint32_t c_refb(uint8_t b) { return omega_api_refb(b); }
uint32_t c_kbits(uint8_t b) { return omega_api_kbits(b); }
int c_isnanb(uint8_t b) { return omega_api_isnanb(b); }
int c_scalef(uint16_t bf16, int mutant, uint32_t *out) { return omega_api_scalef(bf16, mutant, out); }
uint32_t c_xfold(uint32_t bits) { return omega_api_xfold(bits); }
uint32_t c_xfold16(uint16_t bf16) { return omega_api_xfold16(bf16); }
int32_t c_slot(uint64_t op, uint64_t aux, uint64_t rec[16]) { return omega_api_slot(op, aux, rec); }
uint64_t c_trit(const uint8_t v[16]) { return omega_api_trit(v); }
uint64_t c_block(const uint8_t w[128], const uint8_t x[512], uint32_t sc) { return omega_api_block(w, x, sc); }
uint32_t c_qrt(uint32_t rt) { return omega_api_qrt(rt); }
uint32_t c_qwait(uint32_t set, uint32_t w) { return omega_api_qwait(set, w); }
int c_le128(uint32_t decl) { return omega_api_le128(decl); }
int c_roomok(uint32_t n, uint32_t pos, uint32_t mx) { return omega_api_roomok(n, pos, mx); }
int c_pa256b(uint32_t m, uint32_t pos, uint32_t mx) { return omega_api_pa256b(m, pos, mx); }
int c_qydiv(uint32_t qh, uint32_t pc, uint32_t *q) { return omega_api_qydiv(qh, pc, q); }
uint64_t c_qfbyt(uint32_t rows, uint32_t qh) { return omega_api_qfbyt(rows, qh); }
uint32_t c_gxatt(uint32_t m) { return omega_api_gxatt(m); }
uint32_t c_kvsb(uint32_t mx) { return omega_api_kvsb(mx); }
uint64_t c_hostb(uint32_t n, uint32_t w) { return omega_api_hostb(n, w); }
int c_vy16(uint32_t v) { return omega_api_vy16(v); }
uint32_t c_splt(uint32_t mx) { return omega_api_splt(mx); }
uint64_t c_plane(uint32_t kvh, uint32_t mx) { return omega_api_plane(kvh, mx); }
int c_persok(uint32_t p) { return omega_api_persok(p); }
int c_stride(uint32_t need, uint32_t got, uint32_t mask) { return omega_api_stride(need, got, mask); }
uint64_t c_mtgrp(uint32_t left, uint32_t p0, uint32_t t0, uint32_t vy) { return omega_api_mtgrp(left, p0, t0, vy); }
int c_mtbig(uint32_t n, uint32_t vy, uint32_t sm) { return omega_api_mtbig(n, vy, sm); }
uint32_t c_smaxp(uint32_t pos, uint32_t n) { return omega_api_smaxp(pos, n); }
uint64_t c_mtps(uint32_t vy, uint32_t sm) { return omega_api_mtps(vy, sm); }
int c_p2bad(uint32_t v) { return omega_api_p2bad(v); }
uint32_t c_nkbad(uint32_t n, uint32_t k) { return omega_api_nkbad(n, k); }
int c_layok(uint32_t lay, uint32_t R) { return omega_api_layok(lay, R); }
int c_tritok(uint32_t v) { return omega_api_tritok(v); }
uint32_t c_qset(uint32_t k) { return omega_api_qset(k); }
int c_pfargs(uint32_t n, uint32_t k, uint32_t lay) { return omega_api_pfargs(n, k, lay); }
