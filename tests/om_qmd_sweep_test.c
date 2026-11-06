/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host differential for om/qmd/bw_qmd.om. No device, no /dev/nvidia*.
 * Cited constants are WolframLanguageEvaluator results, not hand arithmetic:
 *   16^^00800002 -> 8388610
 *   16^^013f0000 -> 20905984
 *   16^^2f5003a4 -> 793772964
 *   BitOr[BitAnd[16^^04b44808, BitNot[16^^7ff]], 8] -> 78923784
 *   BitOr[BitAnd[16^^04b44808, BitNot[16^^7ff]], 256] -> 78924032
 *   16^^26420000 -> 641859584
 *   16^^00fffdc0 -> 16776640
 * With CUDAPP_MIGRATION_PENDING_QMD_DIFF the shell aborts on any C mismatch.
 * QMD_DIFF_FLIP (red) xors a written word; the first packing call must abort.
 */
#include "omega_blackwell_qmd.h"

#include <stdio.h>
#include <string.h>

static int g_checks, g_failed;

#define CHECK(cond) do { \
    g_checks++; \
    if (!(cond)) { \
        g_failed++; \
        printf("FAIL %s:%d\n", __FILE__, __LINE__); \
        return 1; \
    } \
} while (0)

static OmegaBlackwellQmdConfig base(void) {
    OmegaBlackwellQmdConfig c;
    memset(&c, 0, sizeof c);
    c.code_va = 0x1000;
    c.cbank_va = 0x2000;
    c.scratch_va = 0x3000;
    c.sem_va = 0x4000;
    c.qmd0_va = 0x5000;
    c.qmd1_va = 0x6000;
    c.threads_x = 128;
    c.threads_y = 1;
    c.grid_x = 1;
    c.grid_y = 1;
    c.num_elements = 128;
    c.gpr_count = 32;
    c.shared_bytes = 1024;
    return c;
}

static int pack1(const OmegaBlackwellQmdConfig *c, uint32_t q[OMEGA_BW_QMD_WORDS]) {
    memset(q, 0xa5, sizeof(uint32_t) * OMEGA_BW_QMD_WORDS);
    return omega_blackwell_build_qmd1(q, c);
}

static int cited(void) {
    uint32_t q0[OMEGA_BW_QMD_WORDS], q1[OMEGA_BW_QMD_WORDS], cb[OMEGA_BW_CBANK_DRIVER_WORDS];
    OmegaBlackwellQmdConfig c = base();
    memset(q0, 0, sizeof q0);
    CHECK(omega_blackwell_build_qmd0(q0, 0x5000, 0x6000) == 0);
    CHECK(q0[4] == 8388610u);
    CHECK(q0[12] == (uint32_t)(0x6000ull >> 8));
    CHECK(pack1(&c, q1) == 0);
    CHECK(q1[4] == 20905984u);
    CHECK(q1[14] == 793772964u);
    CHECK((q1[36] & 0x7ffu) == 8u);
    CHECK(q1[36] == 78923784u);
    CHECK(omega_blackwell_verify_qmd_invariants(q1) == 0);
    q1[37] ^= 1u;
    CHECK(omega_blackwell_verify_qmd_invariants(q1) == -1);
    CHECK(omega_blackwell_build_cbank_driver(cb, 0x2000) == 0);
    CHECK(cb[0] == 641859584u);
    CHECK(cb[223] == 16776640u);
    CHECK(cb[68] == 0u);
    CHECK(cb[69] == 0x2000u);
    return 0;
}

static int boundaries(void) {
    static const uint32_t txs[] = {0, 1, 63, 64, 65, 255, 256, 1024, 65535, 65536};
    static const uint32_t tys[] = {0, 1, 65535};
    static const uint32_t tpbs[] = {0, 1, 32, 128};
    static const uint32_t gxs[] = {0, 1, 0xffffffffu};
    static const uint32_t gys[] = {0, 1, 7};
    static const uint32_t gws[] = {0, 1, 9};
    static const uint32_t nes[] = {0, 1, 63, 64, 65, 100000u, 0xffffffffu};
    static const uint32_t gprs[] = {0, 1, 15, 16, 17, 511, 512};
    static const uint32_t shs[] = {0, 1, 127, 128, 129, 1024, 32767, 32768, 32769, 0xffffffffu};
    static const uint64_t vas[] = {0, 1, 0xff, 0x100, 0xffffffffull, 0x100000000ull, ~0ull};
    uint32_t q[OMEGA_BW_QMD_WORDS];
    OmegaBlackwellQmdConfig c;
    size_t i;

    for (i = 0; i < sizeof txs / sizeof txs[0]; i++) {
        c = base();
        c.threads_x = txs[i];
        CHECK(pack1(&c, q) == 0);
        CHECK(omega_blackwell_verify_qmd_invariants(q) == 0);
    }
    for (i = 0; i < sizeof tys / sizeof tys[0]; i++) {
        c = base();
        c.threads_y = tys[i];
        CHECK(pack1(&c, q) == 0);
    }
    for (i = 0; i < sizeof tpbs / sizeof tpbs[0]; i++) {
        c = base();
        c.threads_x = 0;
        c.threads_per_block = tpbs[i];
        CHECK(pack1(&c, q) == 0);
    }
    for (i = 0; i < sizeof gxs / sizeof gxs[0]; i++) {
        c = base();
        c.grid_x = gxs[i];
        CHECK(pack1(&c, q) == 0);
    }
    for (i = 0; i < sizeof gys / sizeof gys[0]; i++) {
        c = base();
        c.grid_y = gys[i];
        CHECK(pack1(&c, q) == 0);
    }
    for (i = 0; i < sizeof gws / sizeof gws[0]; i++) {
        c = base();
        c.grid_x = 0;
        c.grid_width = gws[i];
        CHECK(pack1(&c, q) == 0);
    }
    for (i = 0; i < sizeof nes / sizeof nes[0]; i++) {
        c = base();
        c.grid_x = 0;
        c.grid_width = 0;
        c.num_elements = nes[i];
        CHECK(pack1(&c, q) == 0);
    }
    for (i = 0; i < sizeof gprs / sizeof gprs[0]; i++) {
        c = base();
        c.gpr_count = gprs[i];
        CHECK(pack1(&c, q) == 0);
    }
    for (i = 0; i < sizeof shs / sizeof shs[0]; i++) {
        c = base();
        c.shared_bytes = shs[i];
        int want = shs[i] > 32768u ? -1 : 0;
        CHECK(pack1(&c, q) == want);
        if (want == 0) {
            uint32_t field = shs[i] == 0 ? 8u : (shs[i] + 127u) / 128u;
            CHECK((q[36] & 0x7ffu) == field);
        }
    }
    for (i = 0; i < sizeof vas / sizeof vas[0]; i++) {
        c = base();
        c.code_va = vas[i];
        c.cbank_va = vas[i] + 0x100;
        c.scratch_va = vas[i] + 0x200;
        c.sem_va = vas[i] + 0x300;
        c.qmd0_va = vas[i] + 0x400;
        c.qmd1_va = vas[i] + 0x500;
        CHECK(pack1(&c, q) == 0);
        uint32_t q0[OMEGA_BW_QMD_WORDS];
        memset(q0, 0, sizeof q0);
        CHECK(omega_blackwell_build_qmd0(q0, c.qmd0_va, c.qmd1_va) == 0);
        CHECK(q0[12] == (uint32_t)(c.qmd1_va >> 8));
        uint32_t cb[OMEGA_BW_CBANK_DRIVER_WORDS];
        memset(cb, 0x5a, sizeof cb);
        CHECK(omega_blackwell_build_cbank_driver(cb, c.cbank_va) == 0);
        CHECK(cb[69] == (uint32_t)c.cbank_va);
        CHECK(cb[68] == (uint32_t)(c.cbank_va >> 32));
    }
    CHECK(omega_blackwell_build_qmd0(NULL, 1, 2) == -1);
    CHECK(omega_blackwell_build_qmd1(NULL, &c) == -1);
    CHECK(omega_blackwell_build_qmd1(q, NULL) == -1);
    CHECK(omega_blackwell_verify_qmd_invariants(NULL) == -1);
    CHECK(omega_blackwell_build_cbank_driver(NULL, 1) == -1);
    return 0;
}

static int cross(void) {
    static const uint32_t txs[] = {0, 1, 64, 65535};
    static const uint32_t tpbs[] = {0, 32, 128};
    static const uint32_t gxs[] = {0, 1};
    static const uint32_t gws[] = {0, 3};
    static const uint32_t nes[] = {0, 63, 64, 65, 0xffffffffu};
    uint32_t q[OMEGA_BW_QMD_WORDS];
    for (size_t a = 0; a < 4; a++)
    for (size_t b = 0; b < 3; b++)
    for (size_t d = 0; d < 2; d++)
    for (size_t e = 0; e < 2; e++)
    for (size_t f = 0; f < 5; f++) {
        OmegaBlackwellQmdConfig c = base();
        c.threads_x = txs[a];
        c.threads_per_block = tpbs[b];
        c.grid_x = gxs[d];
        c.grid_width = gws[e];
        c.num_elements = nes[f];
        CHECK(pack1(&c, q) == 0);
    }
    return 0;
}

static uint32_t rnd(uint32_t *s) {
    *s = *s * 1664525u + 1013904223u;
    return *s;
}

static int random_cases(void) {
    uint32_t s = 0xC0FFEEU;
    uint32_t q[OMEGA_BW_QMD_WORDS];
    uint32_t args[16];
    uint32_t cb[OMEGA_BW_CBANK_DRIVER_WORDS];
    for (int n = 0; n < 10000; n++) {
        OmegaBlackwellQmdConfig c = base();
        c.threads_x = rnd(&s);
        c.threads_y = rnd(&s) & 0xffffu;
        c.threads_per_block = rnd(&s) & 0x1ffu;
        c.grid_x = rnd(&s);
        c.grid_y = rnd(&s) & 7u;
        c.grid_width = rnd(&s) & 0x3ffu;
        c.num_elements = rnd(&s);
        c.shared_bytes = rnd(&s) & 0x1ffffu;
        c.gpr_count = rnd(&s) & 0x3ffu;
        c.code_va = ((uint64_t)rnd(&s) << 32) | rnd(&s);
        c.cbank_va = ((uint64_t)rnd(&s) << 32) | rnd(&s);
        int want = c.shared_bytes > 32768u ? -1 : 0;
        CHECK(pack1(&c, q) == want);
        if (want == 0) CHECK(omega_blackwell_verify_qmd_invariants(q) == 0);
        memset(args, 0x11, sizeof args);
        CHECK(omega_blackwell_build_cbank_args(args, c.code_va, c.cbank_va, c.scratch_va, c.num_elements) == 0);
        CHECK(args[7] == 0x11111111u);
        memset(cb, 0x22, sizeof cb);
        CHECK(omega_blackwell_build_cbank_driver_2d(cb, c.cbank_va, c.threads_x, c.threads_y, c.grid_x, c.grid_y) == 0);
        if (c.threads_x > 0) CHECK(cb[216] == c.threads_x);
        CHECK(omega_blackwell_build_cbank_args_matmul(args, c.code_va, c.cbank_va, c.scratch_va, c.threads_x, c.threads_y, c.grid_x) == 0);
    }
    return 0;
}

/* Source-derived partition. Every threshold the C oracle (src/omega_blackwell_qmd.c c_bw_*) compares a field
 * against or cuts it at, each as v-1, v, v+1, plus 0 and UINT32_MAX, set one field at a time on base(), and every
 * value pushed through every packer. u32 thresholds: 1 and 16 (gpr floor), 64 (tx default), 128 (smem7 rounding),
 * 0x1ff/0x200 (gpr mask), 0x8000 (MAX_SHARED_BYTES), 0xffff/0x10000 (tx/ty masks), 0xffffffff-127 (ceil-div wrap of
 * num_elements + tx - 1). u64 thresholds: 2^s for the shifts 4, 6, 8, 32, 36, 38 and the 57-bit field tops
 * (36+21, 38+19), and 2^64-0x300 (the cbank_va + 0x300 wrap). Under CUDAPP_MIGRATION_PENDING_QMD_DIFF the shell
 * aborts on the first word where .om and C differ. */
static int pack_all(const OmegaBlackwellQmdConfig *c) {
    uint32_t q[OMEGA_BW_QMD_WORDS], q0[OMEGA_BW_QMD_WORDS], cb[OMEGA_BW_CBANK_DRIVER_WORDS], args[16];
    int want = c->shared_bytes > 32768u ? -1 : 0;
    CHECK(pack1(c, q) == want);
    if (want == 0) CHECK(omega_blackwell_verify_qmd_invariants(q) == 0);
    memset(q0, 0, sizeof q0);
    CHECK(omega_blackwell_build_qmd0(q0, c->qmd0_va, c->qmd1_va) == 0);
    memset(cb, 0x5a, sizeof cb);
    CHECK(omega_blackwell_build_cbank_driver(cb, c->cbank_va) == 0);
    memset(cb, 0x22, sizeof cb);
    CHECK(omega_blackwell_build_cbank_driver_2d(cb, c->cbank_va, c->threads_x, c->threads_y, c->grid_x, c->grid_y) == 0);
    memset(args, 0x11, sizeof args);
    CHECK(omega_blackwell_build_cbank_args(args, c->code_va, c->cbank_va, c->scratch_va, c->num_elements) == 0);
    CHECK(omega_blackwell_build_cbank_args_matmul(args, c->code_va, c->cbank_va, c->scratch_va, c->threads_x,
                                                  c->threads_y, c->grid_x) == 0);
    return 0;
}

static int g_partition_cfgs;
static int partition(void) {
    static const uint32_t t32[] = {1u, 16u, 64u, 128u, 0x1ffu, 0x200u, 0x8000u, 0xffffu, 0x10000u, 0xffffffffu - 127u};
    static const unsigned sh[] = {4, 6, 8, 32, 36, 38, 57};
    uint32_t v32[3 * sizeof t32 / sizeof t32[0] + 2];
    uint64_t v64[3 * (sizeof sh / sizeof sh[0] + 1) + 2];
    size_t n32 = 0, n64 = 0, i, f;
    for (i = 0; i < sizeof t32 / sizeof t32[0]; i++) {
        v32[n32++] = t32[i] - 1u;
        v32[n32++] = t32[i];
        v32[n32++] = t32[i] + 1u;
    }
    v32[n32++] = 0u;
    v32[n32++] = 0xffffffffu;
    for (i = 0; i <= sizeof sh / sizeof sh[0]; i++) {
        uint64_t v = i < sizeof sh / sizeof sh[0] ? 1ull << sh[i] : 0ull - 0x300ull;
        v64[n64++] = v - 1u;
        v64[n64++] = v;
        v64[n64++] = v + 1u;
    }
    v64[n64++] = 0u;
    v64[n64++] = ~0ull;
    for (f = 0; f < 9; f++)
        for (i = 0; i < n32; i++) {
            OmegaBlackwellQmdConfig c = base();
            uint32_t *fld[9] = {&c.num_elements, &c.threads_per_block, &c.grid_width, &c.threads_x, &c.threads_y,
                                &c.grid_x, &c.grid_y, &c.gpr_count, &c.shared_bytes};
            *fld[f] = v32[i];
            /* the derived-default chains (tx from tpb, gx from grid_width or num_elements) only read a field when
             * the ones before it are 0: cover both settings */
            if (pack_all(&c)) return 1;
            c.threads_x = 0;
            c.grid_x = 0;
            if (f != 2) c.grid_width = 0;
            if (pack_all(&c)) return 1;
            g_partition_cfgs += 2;
        }
    for (f = 0; f < 6; f++)
        for (i = 0; i < n64; i++) {
            OmegaBlackwellQmdConfig c = base();
            uint64_t *fld[6] = {&c.code_va, &c.cbank_va, &c.scratch_va, &c.sem_va, &c.qmd0_va, &c.qmd1_va};
            *fld[f] = v64[i];
            if (pack_all(&c)) return 1;
            g_partition_cfgs++;
        }
    return 0;
}

int main(void) {
    if (cited()) return 1;
    if (boundaries()) return 1;
    if (cross()) return 1;
    if (random_cases()) return 1;
    if (partition()) return 1;
    printf("om qmd sweep: %d checks PASS (partition %d configs)\n", g_checks, g_partition_cfgs);
    return g_failed ? 1 : 0;
}
