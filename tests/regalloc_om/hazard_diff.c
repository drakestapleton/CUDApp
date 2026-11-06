/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* hazard_diff.c: om/codegen/bw_hazard.om (src/omega_bw_hazard.c default build) against the C oracle
 * (CUDAPP_MIGRATION_PENDING_C_HAZARD, symbols renamed legacy_*). The corpus partitions every branch of the C decoder
 * and walk: every 12-bit opcode with every size field and both address/descriptor bits, the RZ / register 254
 * boundaries, every barrier 0..7 (6 is the shift-by-64 case), every wait mask, guarded EXIT/BRA, branch targets in
 * range, at both ends and out of range (including rel = INT32_MAX), undecodable words, the 256-finding cap and the
 * committed GB10 fixture kernels with their waits dropped one at a time. */
#include "omega_bw_hazard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int legacy_omega_bw_decode(const uint8_t b[16], OmegaBwInsn *d);
int legacy_omega_bw_hazard_check(const uint8_t *code, size_t len, OmegaBwHazardReport *r);

static uint64_t g_s = 0x9e3779b97f4a7c15ull;
static uint64_t rnd(void) {
    uint64_t z = (g_s += 0x9e3779b97f4a7c15ull);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}

static const uint16_t k_ops[] = {0x918, 0xb1d, 0x992, 0x98f, 0x94d, 0x947, 0x945, 0x941, 0x202, 0x802, 0x919, 0xb82,
                                 0x7ac, 0x224, 0x824, 0xc24, 0x825, 0x227, 0x20c, 0x80c, 0xc0c, 0x210, 0x23c, 0x27a,
                                 0x819, 0x219, 0x812, 0x212, 0x221, 0x220, 0x223, 0x20b, 0x208, 0x209, 0x23e, 0x245,
                                 0x305, 0x308, 0xf89, 0x981, 0x986, 0x9a8, 0x984, 0x388, 0x988};
#define NOPS (sizeof k_ops / sizeof k_ops[0])
static const unsigned k_regs[] = {0, 1, 2, 3, 4, 5, 6, 7, 253, 254, 255};

static long g_decodes, g_dec_refused, g_checks, g_chk_refused, g_findings, g_capped, g_fail;

static void put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

static int same_insn(const OmegaBwInsn *a, const OmegaBwInsn *b) {
    if (a->op != b->op || a->guard != b->guard || a->stall != b->stall || a->yield != b->yield || a->wbar != b->wbar ||
        a->rbar != b->rbar || a->wait != b->wait || a->var != b->var || a->late != b->late || a->nr != b->nr ||
        a->nw != b->nw || a->flow != b->flow || a->target != b->target)
        return 0;
    for (unsigned j = 0; j < 5; ++j)
        if (a->rd[j].reg != b->rd[j].reg || a->rd[j].n != b->rd[j].n || a->rd[j].uniform != b->rd[j].uniform) return 0;
    if (a->wr[0].reg != b->wr[0].reg || a->wr[0].n != b->wr[0].n || a->wr[0].uniform != b->wr[0].uniform) return 0;
    if (!a->name || !b->name) return a->name == b->name;
    return strcmp(a->name, b->name) == 0;
}

static void decode_one(const uint8_t b[16]) {
    OmegaBwInsn x, y;
    memset(&x, 0xa5, sizeof x);
    memset(&y, 0x5a, sizeof y);
    int rx = legacy_omega_bw_decode(b, &x), ry = omega_bw_decode(b, &y);
    g_decodes++;
    if (rx) g_dec_refused++;
    if (rx != ry || !same_insn(&x, &y)) {
        if (g_fail++ < 10) {
            uint32_t w[4];
            memcpy(w, b, 16);
            printf("FAIL decode %08x %08x %08x %08x rc %d/%d op %x/%x nr %u/%u name %s/%s\n", w[0], w[1], w[2], w[3],
                   rx, ry, x.op, y.op, x.nr, y.nr, x.name ? x.name : "-", y.name ? y.name : "-");
        }
    }
}

static OmegaBwHazardReport g_ra, g_rb;
static double g_t_c, g_t_om;
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (double)t.tv_sec + 1e-9 * (double)t.tv_nsec; }

static void check_one(const uint8_t *code, size_t len, int null_report) {
    double t0 = now();
    int ra = legacy_omega_bw_hazard_check(code, len, null_report ? NULL : &g_ra);
    double t1 = now();
    int rb = omega_bw_hazard_check(code, len, null_report ? NULL : &g_rb);
    g_t_c += t1 - t0;
    g_t_om += now() - t1;
    g_checks++;
    if (ra) g_chk_refused++;
    int ok = ra == rb;
    if (ok && !null_report) {
        ok = g_ra.insns == g_rb.insns && g_ra.reachable == g_rb.reachable && g_ra.count == g_rb.count &&
             g_ra.bad_pc == g_rb.bad_pc;
        for (size_t i = 0; ok && i < g_ra.count && i < OMEGA_BW_HAZARD_MAX; ++i)
            ok = memcmp(&g_ra.f[i], &g_rb.f[i], sizeof g_ra.f[i]) == 0;
        g_findings += (long)g_ra.count;
        if (g_ra.count > OMEGA_BW_HAZARD_MAX) g_capped++;
    }
    if (!ok && g_fail++ < 10)
        printf("FAIL check len %zu rc %d/%d insns %zu/%zu reach %zu/%zu count %zu/%zu bad %lld/%lld\n", len, ra, rb,
               g_ra.insns, g_rb.insns, g_ra.reachable, g_rb.reachable, g_ra.count, g_rb.count,
               (long long)g_ra.bad_pc, (long long)g_rb.bad_pc);
}

/* one random instruction; rel_span bounds the branch targets of a BRA */
static void gen_insn(uint8_t *b, size_t pc, size_t n, int allow_bad) {
    uint32_t op = k_ops[rnd() % NOPS];
    if (allow_bad && rnd() % 97 == 0) op = (uint32_t)(rnd() & 0xfffu);
    uint32_t guard = rnd() % 4 ? 7u : (uint32_t)(rnd() & 15u);
    uint32_t D = k_regs[rnd() % 11], A = k_regs[rnd() % 11], B = k_regs[rnd() % 11], C = k_regs[rnd() % 11];
    uint32_t size = rnd() % 8 ? (uint32_t)(rnd() % 7) : (uint32_t)(rnd() & 7u);
    if (!allow_bad && size == 7) size = 6;
    /* Mandatory size-class witness: hz_size opcode, size=4, live write.
     * same_insn compares nr/nw and every rd.n/wr.n, not just the return code. */
    if (pc == 0) { op = 0x981u; size = 4; D = 1; }
    uint32_t w0 = op | guard << 12 | D << 16 | A << 24;
    uint32_t w1 = B | (uint32_t)(rnd() & 0xffffff00u);
    uint32_t w2 = C | size << 9 | (rnd() & 1 ? 0x100u : 0) | (rnd() & 1 ? 0x08000000u : 0) | (uint32_t)(rnd() & 0xf0000000u);
    uint32_t wbar = (uint32_t)(rnd() % 3 ? 7 : rnd() & 7), rbar = (uint32_t)(rnd() % 3 ? 7 : rnd() & 7);
    uint32_t wait = (uint32_t)(rnd() % 3 ? 0 : rnd() & 63);
    uint32_t w3 = (uint32_t)(rnd() & 15) << 9 | (uint32_t)(rnd() & 1) << 13 | wbar << 14 | rbar << 17 | wait << 20 |
                  (uint32_t)(rnd() & 0xfc000000u);
    if (op == 0x947) {
        int64_t lo = -(int64_t)pc - 2, hi = (int64_t)(n - pc) + 1;
        int64_t t = lo + (int64_t)(rnd() % (uint64_t)(hi - lo + 1));
        int32_t rel = (int32_t)(t - 1);
        if (rnd() % 61 == 0) rel = rnd() & 1 ? 2147483647 : (-2147483647 - 1);
        w0 = (w0 & ~(0x3fu << 18)) | ((uint32_t)rel & 63u) << 18;
        w1 = (uint32_t)((rel >> 6) * 4);
    }
    put32(b, w0); put32(b + 4, w1); put32(b + 8, w2); put32(b + 12, w3);
}

static uint8_t *load(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *p = malloc((size_t)n);
    if (p && fread(p, 1, (size_t)n, f) != (size_t)n) { free(p); p = NULL; }
    fclose(f);
    *len = (size_t)n;
    return p;
}

int main(void) {
    uint8_t b[16];
    memset(b, 0, sizeof b);
    put32(b, 0x981u | 7u << 12 | 1u << 16 | 2u << 24);
    put32(b + 8, 4u << 9);
    decode_one(b);
    /* decode: every opcode x size field x bit 8 x bit 27, registers at the RZ boundary */
    for (uint32_t op = 0; op < 4096; ++op)
        for (uint32_t s = 0; s < 8; ++s)
            for (uint32_t bits = 0; bits < 4; ++bits) {
                unsigned r = k_regs[(op + s + bits) % 11];
                uint32_t w0 = op | (uint32_t)(rnd() & 15) << 12 | r << 16 | k_regs[(op * 3 + s) % 11] << 24;
                uint32_t w2 = k_regs[(op * 7 + bits) % 11] | s << 9 | (bits & 1 ? 0x100u : 0) | (bits & 2 ? 0x08000000u : 0);
                put32(b, w0); put32(b + 4, k_regs[(op * 5 + s) % 11] | (uint32_t)(rnd() & 0xffffff00u));
                put32(b + 8, w2); put32(b + 12, (uint32_t)rnd());
                decode_one(b);
            }
    /* decode: random words over the known opcodes, every register value, BRA rel at both int32 ends */
    for (long i = 0; i < 300000; ++i) {
        uint64_t x = rnd(), y = rnd();
        memcpy(b, &x, 8); memcpy(b + 8, &y, 8);
        uint32_t w0;
        memcpy(&w0, b, 4);
        w0 = (w0 & ~0xfffu) | k_ops[rnd() % NOPS];
        memcpy(b, &w0, 4);
        decode_one(b);
    }
    {
        static const uint32_t k_bra[][2] = {{0x947u | 0x3fu << 18, 0x7fffffffu & ~3u}, {0x947u, 0x80000000u},
                                            {0x947u | 0x3fu << 18, 0xfffffffcu}, {0x947u, 0}};
        for (unsigned i = 0; i < 4; ++i) {
            memset(b, 0, 16);
            put32(b, k_bra[i][0]); put32(b + 4, k_bra[i][1]);
            decode_one(b);
        }
    }
    {
        OmegaBwInsn d;
        if (legacy_omega_bw_decode(NULL, &d) != omega_bw_decode(NULL, &d) || omega_bw_decode(b, NULL) != -1) {
            g_fail++;
            printf("FAIL decode NULL\n");
        }
        g_decodes += 2;
    }

    /* check: refusals */
    static uint8_t prog[16 * 6000];
    memset(prog, 0, sizeof prog);
    check_one(prog, 16, 1);
    check_one(NULL, 16, 0);
    check_one(prog, 0, 0);
    check_one(prog, 1, 0);
    check_one(prog, 15, 0);
    check_one(prog, 17, 0);

    /* check: random programs over every branch of the walk */
    for (long it = 0; it < 40000; ++it) {
        size_t n = 1 + (size_t)(rnd() % (it % 10 == 0 ? 120 : 24));
        int allow_bad = rnd() % 4 == 0;
        for (size_t pc = 0; pc < n; ++pc) gen_insn(prog + 16 * pc, pc, n, allow_bad);
        if (rnd() % 2) { /* a final EXIT keeps most programs walkable */
            uint8_t *e = prog + 16 * (n - 1);
            memset(e, 0, 16);
            put32(e, 0x94du | 7u << 12);
            put32(e + 12, 7u << 14 | 7u << 17);
        }
        check_one(prog, 16 * n, 0);
    }
    /* the 256-finding cap: 300 LDG writes of R0..R?, then 300 unwaited readers */
    {
        size_t n = 0;
        for (unsigned i = 0; i < 300; ++i, ++n) {
            uint8_t *p = prog + 16 * n;
            memset(p, 0, 16);
            put32(p, 0x981u | 7u << 12 | (i % 250) << 16 | 254u << 24);
            put32(p + 8, 0x100u);
            put32(p + 12, (uint32_t)(i % 6) << 14 | 7u << 17);
        }
        for (unsigned i = 0; i < 300; ++i, ++n) {
            uint8_t *p = prog + 16 * n;
            memset(p, 0, 16);
            put32(p, 0x210u | 7u << 12 | 253u << 16 | (i % 250) << 24);
            put32(p + 4, (i * 7) % 250);
            put32(p + 8, (i * 11) % 250);
            put32(p + 12, 7u << 14 | 7u << 17);
        }
        memset(prog + 16 * n, 0, 16);
        put32(prog + 16 * n, 0x94du | 7u << 12);
        put32(prog + 16 * n + 12, 7u << 14 | 7u << 17);
        check_one(prog, 16 * (n + 1), 0);
    }
    /* the portable unit's fixed code view: 2048 instructions agree with the oracle; 2049 is the documented
     * divergence (the oracle has no cap; the successor refuses with insns set and bad_pc -1) */
    for (size_t n = 2047; n <= 2049; ++n) {
        for (size_t pc = 0; pc < n; ++pc) gen_insn(prog + 16 * pc, pc, n, 0);
        memset(prog + 16 * (n - 1), 0, 16);
        put32(prog + 16 * (n - 1), 0x94du | 7u << 12);
        put32(prog + 16 * (n - 1) + 12, 7u << 14 | 7u << 17);
        if (n <= 2048) {
            check_one(prog, 16 * n, 0);
            continue;
        }
        int ra = legacy_omega_bw_hazard_check(prog, 16 * n, &g_ra);
        int rb = omega_bw_hazard_check(prog, 16 * n, &g_rb);
        g_checks++;
        if (ra != 0 || rb != -1 || g_rb.insns != n || g_rb.bad_pc != -1 || g_rb.count != 0) {
            g_fail++;
            printf("FAIL length cap n %zu rc %d/%d\n", n, ra, rb);
        }
    }
    /* the committed GB10 fixtures, as is and with every wait dropped, one at a time (first 400) */
    static const char *k_fix[] = {"pattn256_attend_ad5d650b.bin", "pattn256_attend_e3a26848.bin",
                                  "pattn256_prep_ad5d650b.bin", "pattn256_prep_e3a26848.bin"};
    long fixtures = 0;
    for (unsigned f = 0; f < 4; ++f) {
        char path[256];
        snprintf(path, sizeof path, "tests/fixtures/bw_hazard/%s", k_fix[f]);
        size_t len = 0;
        uint8_t *code = load(path, &len);
        if (!code) { g_fail++; printf("FAIL fixture %s missing\n", path); continue; }
        check_one(code, len, 0);
        fixtures++;
        unsigned dropped = 0;
        for (size_t pc = 0; pc < len / 16 && dropped < 400; ++pc) {
            uint32_t w3 = (uint32_t)code[16 * pc + 12] | (uint32_t)code[16 * pc + 13] << 8 |
                          (uint32_t)code[16 * pc + 14] << 16 | (uint32_t)code[16 * pc + 15] << 24;
            if (!(w3 >> 20 & 63u)) continue;
            put32(code + 16 * pc + 12, w3 & ~(63u << 20));
            check_one(code, len, 0);
            put32(code + 16 * pc + 12, w3);
            dropped++;
            fixtures++;
        }
        free(code);
    }
    if (g_fail) {
        printf("FAIL hazard differential failures=%ld\n", g_fail);
        return 1;
    }
    printf("PASS hazard decode differential cases=%ld refused=%ld\n", g_decodes, g_dec_refused);
    printf("PASS hazard check differential cases=%ld refused=%ld findings=%ld capped=%ld fixtures=%ld c_s=%.3f omega_s=%.3f\n",
           g_checks, g_chk_refused, g_findings, g_capped, fixtures, g_t_c, g_t_om);
    return 0;
}
