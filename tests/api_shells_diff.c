/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Omega API decisions against the pending C oracle. Same header, flag off here. */
#include "omega_api_shells.h"
#include "omega_fp8_slot_table.h"
#include "omega_gpu_fp8_api.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

uint32_t c_lane(uint32_t k);
void c_fp8def(uint32_t mt, uint32_t wp, uint32_t *m, uint32_t *w);
uint32_t c_fp8mt(uint32_t mt, uint32_t segs);
uint32_t c_ctabud(uint32_t segs, uint32_t ov);
int c_regfit(uint32_t w, uint32_t d);
uint32_t c_tgbm(uint32_t rt, uint32_t w, uint32_t n);
void c_tgspec(uint32_t rt, uint32_t w, uint32_t n, uint32_t *r, uint32_t *ww);
uint32_t c_tgsz(uint32_t m, uint32_t n, uint32_t bm);
void c_ternd(uint32_t k, uint32_t lay, uint32_t R, uint32_t gr, uint32_t gd, uint32_t *rows, uint32_t *depth);
void c_ternwc(uint32_t w, uint32_t ct, uint32_t *ww, uint32_t *cc);
uint32_t c_pad8(uint32_t n);
uint32_t c_segb(uint32_t i, uint32_t cnt, uint32_t n, uint32_t en, uint32_t mu);
int c_segend(uint32_t i, uint32_t cnt, uint32_t n, uint32_t en, uint32_t pv);
int c_finite(uint32_t bits);
uint64_t c_pkbytes(uint32_t e, uint32_t k);
int c_alignok(uint32_t lo, uint32_t mask);
int c_tile8(uint32_t c0);
uint32_t c_perm4(uint32_t q);
uint32_t c_pfbm(uint32_t rt);
uint32_t c_swsrc(uint32_t p, uint32_t n);
int c_maxmnk(uint32_t m, uint32_t k, uint32_t n);
int c_dcache(uint32_t layers, uint32_t mx, uint32_t rot);
int c_dreg(uint32_t decl);
int c_tpack(uint32_t n, uint32_t k, uint32_t lay, uint32_t R, uint32_t np);
uint32_t c_rrows(uint32_t s);
uint32_t c_rtdst(uint32_t g, uint32_t ex, uint32_t q, uint32_t e);
uint32_t c_rtsrc(uint32_t ex, uint32_t k, uint32_t g, uint32_t q);
void c_dsplit(uint32_t pos, uint32_t per, uint32_t mx, uint32_t *code, uint32_t *splits);
int c_vy2(uint32_t v);
int c_thok(uint32_t lo, uint32_t hi);
void c_tcsp(uint32_t rt, uint32_t ct, uint32_t n, uint32_t *r, uint32_t *c);
uint32_t c_tcw(uint32_t w);
uint32_t c_rup(uint32_t v, uint32_t q);
uint64_t c_woff(uint32_t kp, uint32_t col0);
int c_wok(uint32_t c0, uint32_t cols, uint32_t n);
uint32_t c_qgrp(uint32_t kb);
uint32_t c_refb(uint8_t b);
uint32_t c_kbits(uint8_t b);
int c_isnanb(uint8_t b);
int c_scalef(uint16_t bf16, int mutant, uint32_t *out);
uint32_t c_xfold(uint32_t bits);
uint32_t c_xfold16(uint16_t bf16);
int32_t c_slot(uint64_t op, uint64_t aux, uint64_t rec[16]);
uint64_t c_trit(const uint8_t v[16]);
uint64_t c_block(const uint8_t w[128], const uint8_t x[512], uint32_t sc);
uint32_t c_qrt(uint32_t rt);
uint32_t c_qwait(uint32_t set, uint32_t w);
float omega_bf16_to_fp32(uint16_t b) {
    uint32_t val = (uint32_t)b << 16;
    float f;
    memcpy(&f, &val, 4);
    return f;
}

int c_le128(uint32_t decl);
int c_roomok(uint32_t n, uint32_t pos, uint32_t mx);
int c_pa256b(uint32_t m, uint32_t pos, uint32_t mx);
int c_qydiv(uint32_t qh, uint32_t pc, uint32_t *q);
uint64_t c_qfbyt(uint32_t rows, uint32_t qh);
uint32_t c_gxatt(uint32_t m);
uint32_t c_kvsb(uint32_t mx);
uint64_t c_hostb(uint32_t n, uint32_t w);
int c_vy16(uint32_t v);
uint32_t c_splt(uint32_t mx);
uint64_t c_plane(uint32_t kvh, uint32_t mx);
int c_persok(uint32_t p);
int c_stride(uint32_t need, uint32_t got, uint32_t mask);
uint64_t c_mtgrp(uint32_t left, uint32_t p0, uint32_t t0, uint32_t vy);
int c_mtbig(uint32_t n, uint32_t vy, uint32_t sm);
uint32_t c_smaxp(uint32_t pos, uint32_t n);
uint64_t c_mtps(uint32_t vy, uint32_t sm);
int c_p2bad(uint32_t v);
uint32_t c_nkbad(uint32_t n, uint32_t k);
int c_layok(uint32_t lay, uint32_t R);
int c_tritok(uint32_t v);
uint32_t c_qset(uint32_t k);
int c_pfargs(uint32_t n, uint32_t k, uint32_t lay);

static int fails;
static uint64_t checks;

static void bad(const char *what, uint64_t a, uint64_t b) {
    if (fails < 12) fprintf(stderr, "MISMATCH %s omega=%llu c=%llu\n", what,
                            (unsigned long long)a, (unsigned long long)b);
    fails++;
}
static void eq_u32(const char *what, uint32_t o, uint32_t c) {
    checks++;
    if (o != c) bad(what, o, c);
}
static void eq_i(const char *what, int o, int c) {
    checks++;
    if (o != c) bad(what, (uint64_t)(int64_t)o, (uint64_t)(int64_t)c);
}
static void eq_u64(const char *what, uint64_t o, uint64_t c) {
    checks++;
    if (o != c) bad(what, o, c);
}

static const uint32_t B[] = {
    0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 45, 63, 64, 127, 128, 129, 255, 256,
    511, 512, 1023, 1024, 2048, 4095, 4096, 8191, 8192, 16384, 32768, 65535, 65536,
    131072, 262144, 1048576, 16777215, 0x7fffffffu, 0xffffffffu
};
enum { NB = sizeof B / sizeof B[0] };

static uint32_t xs = 0xA11E5001u;
static uint32_t rnd(void) {
    xs ^= xs << 13;
    xs ^= xs >> 17;
    xs ^= xs << 5;
    return xs;
}
static uint32_t pick(void) {
    uint32_t r = rnd();
    if ((r & 7u) == 0) return B[r % NB];
    return r;
}

static void check_pack_bytes(void) {
    uint8_t w[256], xraw[512], v[16];
    float out_o[8], out_c[8], xf[32], yo[32], yc[32];
    uint16_t sc[8];
    for (int i = 0; i < 256; i++) w[i] = (uint8_t)i;
    for (int i = 0; i < 512; i++) xraw[i] = (uint8_t)(i * 17);
    for (int i = 0; i < 16; i++) v[i] = (uint8_t)(i * 3 + 1);
    eq_u64("trit", omega_api_trit(v), c_trit(v));
    eq_u64("block", omega_api_block(w, xraw, 0x3f80), c_block(w, xraw, 0x3f80));
    eq_u64("block0", omega_api_block(w, xraw, 0), c_block(w, xraw, 0));
    for (int b = 0; b < 256; b++) {
        eq_u32("refb", omega_api_refb((uint8_t)b), c_refb((uint8_t)b));
        eq_u32("kbits", omega_api_kbits((uint8_t)b), c_kbits((uint8_t)b));
        eq_i("isnan", omega_api_isnanb((uint8_t)b), c_isnanb((uint8_t)b));
        eq_u32("e4ref", omega_fp8_e4m3_kernel_bits((uint8_t)b), c_kbits((uint8_t)b));
        eq_i("e4nan", omega_fp8_e4m3_is_nan((uint8_t)b), c_isnanb((uint8_t)b));
    }
    size_t first = 99;
    size_t n = omega_fp8_pack_scan_nan(w, 256, &first);
    checks++;
    if (n != 2 || first != 127) bad("scan", n, first);
    for (uint32_t bf = 0; bf < 65536u; bf++) {
        uint32_t ob = 1, cb = 2;
        int oc = omega_api_scalef((uint16_t)bf, 0, &ob);
        int cc = c_scalef((uint16_t)bf, 0, &cb);
        eq_i("scalef", oc, cc);
        if (oc == 0) eq_u32("scalebits", ob, cb);
        ob = 1; cb = 2;
        oc = omega_api_scalef((uint16_t)bf, 1, &ob);
        cc = c_scalef((uint16_t)bf, 1, &cb);
        eq_i("scalefm", oc, cc);
        if (oc == 0) eq_u32("scalebitsm", ob, cb);
        eq_u32("x16", omega_api_xfold16((uint16_t)bf), c_xfold16((uint16_t)bf));
    }
    sc[0] = 0x3f80; sc[1] = 0x4000; sc[2] = 0x7f80; sc[3] = 0;
    int pr = omega_fp8_pack_scales(sc, 4, out_o, 0);
    checks++;
    if (pr != -1) bad("packscale-refuse", (uint64_t)pr, 1);
    sc[2] = 0x3f00;
    pr = omega_fp8_pack_scales(sc, 4, out_o, 0);
    eq_i("packscale", pr, 0);
    for (int i = 0; i < 4; i++) {
        uint32_t bits = 0, cb = 0;
        c_scalef(sc[i], 0, &cb);
        memcpy(&bits, &out_o[i], 4);
        eq_u32("packscale-byte", bits, cb);
    }
    for (uint32_t i = 0; i < 32; i++) xf[i] = (float)(i + 1);
    omega_fp8_pack_x_f32(xf, 32, yo);
    for (uint32_t i = 0; i < 32; i++) {
        uint32_t bits, inb, cb;
        memcpy(&inb, &xf[i], 4);
        cb = c_xfold(inb);
        memcpy(&bits, &yo[i], 4);
        eq_u32("xf32", bits, cb);
    }
    uint16_t xb[32];
    for (uint32_t i = 0; i < 32; i++) xb[i] = (uint16_t)(0x3f80 + i);
    omega_fp8_pack_x_bf16(xb, 32, yc);
    for (uint32_t i = 0; i < 32; i++) {
        uint32_t bits;
        memcpy(&bits, &yc[i], 4);
        eq_u32("xbf16", bits, c_xfold16(xb[i]));
    }
    /* bit patterns around the subnormal fold and the NaN boundary */
    uint32_t edge[] = {0, 1, 0x7f, 0x80, 0xff, 0x100, 0x7fffff, 0x800000, 0xffffff,
                       0x7f7fffff, 0x7f800000, 0x80000000u, 0xff800000u, 0xffffffffu};
    for (unsigned i = 0; i < sizeof edge / sizeof edge[0]; i++)
        eq_u32("xfold-e", omega_api_xfold(edge[i]), c_xfold(edge[i]));
}

static void check_shapes(void) {
    for (unsigned i = 0; i < NB; i++) {
        uint32_t a = B[i];
        eq_u32("lane", omega_api_lane(a), c_lane(a));
        eq_u32("pad8", omega_api_pad8(a), c_pad8(a));
        eq_u32("perm4", omega_api_perm4(a), c_perm4(a));
        eq_u32("pfbm", omega_api_pfbm(a), c_pfbm(a));
        eq_i("finite", omega_api_finite(a), c_finite(a));
        eq_i("tile8", omega_api_tile8(a), c_tile8(a));
        eq_i("dreg", omega_api_dreg(a), c_dreg(a));
        eq_u32("rrows", omega_api_rrows(a), c_rrows(a));
        eq_i("vy2", omega_api_vy2(a), c_vy2(a));
        eq_u32("tcw", omega_api_tcw(a), c_tcw(a));
        eq_u32("qrt", omega_api_qrt(a), c_qrt(a));
        eq_i("le128", omega_api_le128(a), c_le128(a));
        eq_u32("qgrp", omega_api_qgrp(a), c_qgrp(a));
        for (unsigned j = 0; j < NB; j++) {
            uint32_t b = B[j];
            uint32_t om = 0, ow = 0, cm = 0, cw = 0;
            omega_api_fp8_defaults(a, b, &om, &ow);
            c_fp8def(a, b, &cm, &cw);
            eq_u32("fp8m", om, cm);
            eq_u32("fp8w", ow, cw);
            eq_u32("fp8mt", omega_api_fp8mt(a, b), c_fp8mt(a, b));
            eq_u32("ctabud", omega_api_ctabud(a, b), c_ctabud(a, b));
            eq_i("regfit", omega_api_regfit(a, b), c_regfit(a, b));
            eq_i("align", omega_api_alignok(a, b), c_alignok(a, b));
            eq_u64("pk", omega_api_pkbytes(a, b), c_pkbytes(a, b));
            eq_u32("swsrc", omega_api_swsrc(a, b), c_swsrc(a, b));
            eq_i("thok", omega_api_thok(a, b), c_thok(a, b));
            eq_u32("rup", omega_api_rup(a, b), c_rup(a, b));
            eq_u64("woff", omega_api_woff(a, b), c_woff(a, b));
            eq_u32("qwait", omega_api_qwait(a, b), c_qwait(a, b));
            uint32_t orows = 9, oc = 9, crows = 8, cc = 8;
            omega_api_ternwc(a, b, &orows, &oc);
            c_ternwc(a, b, &crows, &cc);
            eq_u32("ternw", orows, crows);
            eq_u32("ternc", oc, cc);
        }
    }
    /* exhaustive small domains */
    for (uint32_t k = 0; k <= 256; k++) eq_u32("lane-x", omega_api_lane(k), c_lane(k));
    for (uint32_t q = 0; q < 16; q++) eq_u32("perm-x", omega_api_perm4(q), c_perm4(q));
    for (uint32_t n = 0; n <= 64; n++) eq_u32("pad-x", omega_api_pad8(n), c_pad8(n));
    for (uint32_t g = 1; g <= 32; g++) eq_u32("qgrp-x", omega_api_qgrp(g * 3), c_qgrp(g * 3));
    eq_u32("qgrp0", omega_api_qgrp(0), c_qgrp(0));
    eq_u32("qgrp1", omega_api_qgrp(1), c_qgrp(1));
    eq_u32("qgrp32", omega_api_qgrp(32), c_qgrp(32));
}

static void check_multi(void) {
    for (unsigned i = 0; i < NB; i++) {
        for (unsigned j = 0; j < 16; j++) {
            uint32_t a = B[i], b = B[j % NB], c = B[(i + j) % NB];
            uint32_t d = B[(i * 3 + j) % NB], e = B[(j * 5) % NB];
            eq_u32("tgbm", omega_api_tgbm(a, b, c), c_tgbm(a, b, c));
            uint32_t or = 1, ow = 1, cr = 2, cw = 2;
            omega_api_tgspec(a, b, c, &or, &ow);
            c_tgspec(a, b, c, &cr, &cw);
            eq_u32("tgspecr", or, cr);
            eq_u32("tgspecw", ow, cw);
            eq_u32("tgsz", omega_api_tgsz(a, b, c), c_tgsz(a, b, c));
            uint32_t od = 3, orows = 3, cd = 4, crows = 4;
            omega_api_ternd(a, b & 1u, c & 7u, d & 15u, e & 31u, &orows, &od);
            c_ternd(a, b & 1u, c & 7u, d & 15u, e & 31u, &crows, &cd);
            eq_u32("terndr", orows, crows);
            eq_u32("terndd", od, cd);
            eq_u32("segb", omega_api_segb(a, b, c, d, e), c_segb(a, b, c, d, e));
            eq_i("segend", omega_api_segend(a, b, c, d, e), c_segend(a, b, c, d, e));
            eq_i("maxmnk", omega_api_maxmnk(a, b, c), c_maxmnk(a, b, c));
            eq_i("dcache", omega_api_dcache(a, b, c), c_dcache(a, b, c));
            eq_i("tpack", omega_api_tpack(a, b, c & 1u, d, e), c_tpack(a, b, c & 1u, d, e));
            eq_u32("rtdst", omega_api_rtdst(a, b, c, d), c_rtdst(a, b, c, d));
            eq_u32("rtsrc", omega_api_rtsrc(a, b, c, d), c_rtsrc(a, b, c, d));
            uint32_t ocode = 9, ospl = 9, ccode = 8, cspl = 9;
            omega_api_dsplit(a, b, c, &ocode, &ospl);
            c_dsplit(a, b, c, &ccode, &cspl);
            eq_u32("dsplitc", ocode, ccode);
            eq_u32("dsplits", ospl, cspl);
            uint32_t otr = 1, otc = 1, ctr = 2, ctc = 2;
            omega_api_tcsp(a, b, c, &otr, &otc);
            c_tcsp(a, b, c, &ctr, &ctc);
            eq_u32("tcspr", otr, ctr);
            eq_u32("tcspc", otc, ctc);
            eq_i("wok", omega_api_wok(a, b, c), c_wok(a, b, c));
        }
    }
}

static void check_random(void) {
    for (int n = 0; n < 10000; n++) {
        uint32_t a = pick(), b = pick(), c = pick(), d = pick(), e = pick();
        eq_u32("r-lane", omega_api_lane(a), c_lane(a));
        eq_u32("r-tgbm", omega_api_tgbm(a, b, c), c_tgbm(a, b, c));
        eq_u32("r-segb", omega_api_segb(a % 8u, (b % 5u) + 1u, c ? c : 1u, d, e),
               c_segb(a % 8u, (b % 5u) + 1u, c ? c : 1u, d, e));
        eq_i("r-segend", omega_api_segend(a % 8u, (b % 5u) + 1u, c ? c : 1u, d, e & 1u),
             c_segend(a % 8u, (b % 5u) + 1u, c ? c : 1u, d, e & 1u));
        eq_i("r-align", omega_api_alignok(a, b | 1u), c_alignok(a, b | 1u));
        eq_u64("r-pk", omega_api_pkbytes(a % 64u, b), c_pkbytes(a % 64u, b));
        eq_i("r-max", omega_api_maxmnk(a, b, c), c_maxmnk(a, b, c));
        eq_i("r-tpack", omega_api_tpack(a, b, c & 1u, d % 8u, e), c_tpack(a, b, c & 1u, d % 8u, e));
        eq_u32("r-rtdst", omega_api_rtdst(a % 16u, b, c % 8u, d % 8u), c_rtdst(a % 16u, b, c % 8u, d % 8u));
        eq_u32("r-rtsrc", omega_api_rtsrc(a, b, c % 16u, d % 8u), c_rtsrc(a, b, c % 16u, d % 8u));
        uint32_t oc = 1, os = 1, cc = 2, cs = 1;
        omega_api_dsplit(a, b % 64u, c % 128u, &oc, &os);
        c_dsplit(a, b % 64u, c % 128u, &cc, &cs);
        eq_u32("r-dsc", oc, cc);
        eq_u32("r-dss", os, cs);
        eq_u64("r-woff", omega_api_woff(a % 4096u, b), c_woff(a % 4096u, b));
        eq_i("r-wok", omega_api_wok(a, b, c ? c : 1u), c_wok(a, b, c ? c : 1u));
        eq_u32("r-rup", omega_api_rup(a, b % 64u), c_rup(a, b % 64u));
        eq_i("r-finite", omega_api_finite(a), c_finite(a));
        eq_u32("r-xfold", omega_api_xfold(a), c_xfold(a));
        uint32_t ob = 0, cb = 1;
        int so = omega_api_scalef((uint16_t)(a & 65535u), (int)(b & 1u), &ob);
        int sc = c_scalef((uint16_t)(a & 65535u), (int)(b & 1u), &cb);
        eq_i("r-sc", so, sc);
        if (so == 0) eq_u32("r-scb", ob, cb);
    }
}

static void shadow_step(uint64_t rec[16], uint64_t op, uint64_t aux, int32_t *rc) {
    *rc = c_slot(op, aux, rec);
}

static void check_slots(void) {
    uint64_t rec[16];
    memset(rec, 0, sizeof rec);
    int32_t cr, orc;
    /* direct opcode differential over the legal transitions */
    struct { uint64_t op, aux; } seq[] = {
        {1, 0}, {1, 0}, {2, 0}, {1, 0}, {3, 0x11}, {4, 0}, {4, 0}, {5, 0}, {5, 0},
        {7, 0}, {6, 0}, {3, 1}, {8, 0}
    };
    for (unsigned i = 0; i < sizeof seq / sizeof seq[0]; i++) {
        uint64_t copy[16];
        memcpy(copy, rec, sizeof rec);
        orc = omega_api_slot(seq[i].op, seq[i].aux, rec);
        cr = c_slot(seq[i].op, seq[i].aux, copy);
        eq_i("slot-rc", orc, cr);
        for (int k = 0; k < 16; k++) eq_u64("slot-rec", rec[k], copy[k]);
    }
    /* bad op and short record are refusals; a null record must not be passed */
    uint64_t junk[16];
    memset(junk, 0, sizeof junk);
    eq_i("slot-badop", omega_api_slot(9, 0, junk), c_slot(9, 0, junk));
    eq_i("slot-stale-gate", omega_api_slot(7, 0, junk), -7);

    OmegaFp8SlotTable table;
    if (omega_fp8_slot_table_init(&table, 2) != 0) {
        bad("slot-init", 1, 0);
        return;
    }
    uint64_t ver = 0;
    int tr = omega_fp8_slot_begin_load(&table, 0);
    eq_i("tbl-begin", tr, 0);
    tr = omega_fp8_slot_begin_load(&table, 0);
    eq_i("tbl-busy", tr, OMEGA_FP8_SLOT_BUSY);
    tr = omega_fp8_slot_finish_load(&table, 0, 42, 1, &ver);
    eq_i("tbl-fin", tr, 0);
    checks++;
    if (ver == 0) bad("tbl-ver", ver, 1);
    tr = omega_fp8_slot_pin(&table, 0, ver);
    eq_i("tbl-pin", tr, 0);
    tr = omega_fp8_slot_pin(&table, 0, ver + 1);
    eq_i("tbl-pin-stale", tr, OMEGA_FP8_SLOT_STALE);
    tr = omega_fp8_slot_evict(&table, 0);
    eq_i("tbl-evict-busy", tr, OMEGA_FP8_SLOT_BUSY);
    tr = omega_fp8_slot_unpin(&table, 0, ver);
    eq_i("tbl-unpin", tr, 0);
    tr = omega_fp8_slot_unpin(&table, 0, ver);
    eq_i("tbl-unpin-bad", tr, OMEGA_FP8_SLOT_BAD_ARGS);
    tr = omega_fp8_slot_evict(&table, 0);
    eq_i("tbl-evict", tr, 0);
    tr = omega_fp8_slot_evict(&table, 0);
    eq_i("tbl-evict-stale", tr, 0);
    eq_i("tbl-idle", omega_fp8_slot_table_idle(&table), 1);
    omega_fp8_slot_table_set_mutant(&table, 1);
    tr = omega_fp8_slot_begin_load(&table, 1);
    eq_i("tbl-mbegin", tr, 0);
    tr = omega_fp8_slot_pin(&table, 1, 0);
    eq_i("tbl-mpin", tr, OMEGA_FP8_SLOT_STALE);
    omega_fp8_slot_table_fini(&table);
}

static void check_negative(void) {
    uint32_t o = omega_api_lane(256);
    uint32_t flipped = o ^ 8u;
    checks++;
    if (flipped == c_lane(256)) bad("neg-control", flipped, c_lane(256));
    uint64_t rec[16];
    memset(rec, 0, sizeof rec);
    rec[0] = 2; rec[1] = 1; rec[3] = 1;
    int32_t rc = omega_api_slot(7, 0, rec);
    checks++;
    if (rc != -7) bad("neg-slot", (uint64_t)(int64_t)rc, 7);
    rec[1] ^= 1u;
    int32_t rc2 = omega_api_slot(7, 0, rec);
    checks++;
    if (rc2 == rc && rec[1] == 1) bad("neg-flip", rc2, rc);
}

static void check_attn(void) {
    uint32_t bounds[] = {0, 1, 2, 7, 8, 15, 16, 17, 127, 128, 255, 256, 257, 32767, 32768, 32769, 65535, 65536, 131072};
    unsigned nb = sizeof bounds / sizeof bounds[0];
    for (unsigned i = 0; i < nb; i++) {
        uint32_t a = bounds[i];
        eq_u32("gxatt", omega_api_gxatt(a), c_gxatt(a));
        eq_u32("kvsb", omega_api_kvsb(a), c_kvsb(a));
        eq_i("vy16", omega_api_vy16(a), c_vy16(a));
        eq_u32("splt", omega_api_splt(a), c_splt(a));
        eq_i("persok", omega_api_persok(a), c_persok(a));
        eq_i("p2bad", omega_api_p2bad(a), c_p2bad(a));
        for (unsigned j = 0; j < nb; j++) {
            uint32_t b = bounds[j];
            uint32_t oq = 9, cq = 8;
            eq_i("qydiv", omega_api_qydiv(a, b, &oq), c_qydiv(a, b, &cq));
            if (b) eq_u32("qydivq", oq, cq);
            eq_u64("qfbyt", omega_api_qfbyt(a, b), c_qfbyt(a, b));
            eq_u64("hostb", omega_api_hostb(a, b), c_hostb(a, b));
            eq_u64("plane", omega_api_plane(a, b), c_plane(a, b));
            eq_u32("smaxp", omega_api_smaxp(a, b), c_smaxp(a, b));
            eq_u64("mtps", omega_api_mtps(a, b), c_mtps(a, b));
            for (unsigned k = 0; k < nb; k++) {
                uint32_t c = bounds[k];
                eq_i("roomok", omega_api_roomok(a, b, c), c_roomok(a, b, c));
                eq_i("pa256b", omega_api_pa256b(a, b, c), c_pa256b(a, b, c));
                eq_i("stride", omega_api_stride(a, b, c), c_stride(a, b, c));
                eq_i("mtbig", omega_api_mtbig(a, b, c), c_mtbig(a, b, c));
                eq_u64("mtgrp", omega_api_mtgrp(a, b, c, bounds[k % 8]), c_mtgrp(a, b, c, bounds[k % 8]));
            }
        }
    }
    for (uint32_t n = 0; n < 10000u; n++) {
        uint32_t a = pick(), b = pick(), c = pick(), d = pick() & 31u;
        uint32_t oq = 1, cq = 2;
        eq_i("room-r", omega_api_roomok(a, b, c), c_roomok(a, b, c));
        eq_i("pa-r", omega_api_pa256b(a & 65535u, b, c), c_pa256b(a & 65535u, b, c));
        eq_i("qy-r", omega_api_qydiv(a, b, &oq), c_qydiv(a, b, &cq));
        if (b) eq_u32("qyq-r", oq, cq);
        eq_u64("qf-r", omega_api_qfbyt(a, b), c_qfbyt(a, b));
        eq_u32("gx-r", omega_api_gxatt(a), c_gxatt(a));
        eq_u64("host-r", omega_api_hostb(a, b), c_hostb(a, b));
        eq_i("vy-r", omega_api_vy16(a), c_vy16(a));
        eq_u64("plane-r", omega_api_plane(a & 63u, b), c_plane(a & 63u, b));
        eq_i("p2-r", omega_api_p2bad(a), c_p2bad(a));
        eq_i("st-r", omega_api_stride(a, b, c), c_stride(a, b, c));
        eq_u64("grp-r", omega_api_mtgrp(a & 255u, b, c, d), c_mtgrp(a & 255u, b, c, d));
        eq_i("big-r", omega_api_mtbig(a & 255u, d, b), c_mtbig(a & 255u, d, b));
    }
}

static void check_upload(void) {
    uint32_t ns[] = {0, 1, 255, 256, 512, 8192, 8193, 16384, 16385, 1073741824u, 1073741825u};
    uint32_t ks[] = {0, 1, 255, 256, 512, 768, 8192, 8448, 8704, 16384, 16640};
    uint32_t lays[] = {0, 1, 2, 3};
    uint32_t rs[] = {0, 1, 2, 3, 4, 5};
    unsigned nn = sizeof ns / sizeof ns[0], nk = sizeof ks / sizeof ks[0];
    for (unsigned i = 0; i < nn; i++) {
        for (unsigned j = 0; j < nk; j++) {
            eq_u32("nkbad", omega_api_nkbad(ns[i], ks[j]), c_nkbad(ns[i], ks[j]));
            eq_u32("qset", omega_api_qset(ks[j]), c_qset(ks[j]));
            for (unsigned L = 0; L < 4; L++)
                eq_i("pfargs", omega_api_pfargs(ns[i], ks[j], lays[L]), c_pfargs(ns[i], ks[j], lays[L]));
        }
    }
    for (unsigned L = 0; L < 4; L++)
        for (unsigned r = 0; r < 6; r++)
            eq_i("layok", omega_api_layok(lays[L], rs[r]), c_layok(lays[L], rs[r]));
    for (uint32_t v = 0; v < 256u; v++)
        eq_i("tritok", omega_api_tritok(v), c_tritok(v));
    for (uint32_t n = 0; n < 10000u; n++) {
        uint32_t a = pick(), b = pick(), c = pick() & 3u, d = pick() & 7u;
        eq_u32("nk-r", omega_api_nkbad(a, b), c_nkbad(a, b));
        eq_i("lay-r", omega_api_layok(c, d), c_layok(c, d));
        eq_i("trit-r", omega_api_tritok(a & 255u), c_tritok(a & 255u));
        eq_u32("qset-r", omega_api_qset(a), c_qset(a));
        eq_i("pf-r", omega_api_pfargs(a, b, c), c_pfargs(a, b, c));
    }
}

int main(void) {
    check_upload();
    check_pack_bytes();
    check_shapes();
    check_attn();
    check_multi();
    check_random();
    check_slots();
    check_negative();
    if (fails) {
        fprintf(stderr, "FAIL api-shells checks=%llu fails=%d\n",
                (unsigned long long)checks, fails);
        return 1;
    }
    printf("PASS api-shells checks=%llu\n", (unsigned long long)checks);
    return 0;
}
