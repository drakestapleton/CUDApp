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

int main(void) {
    if (cited()) return 1;
    if (boundaries()) return 1;
    if (cross()) return 1;
    if (random_cases()) return 1;
    printf("om qmd sweep: %d checks PASS\n", g_checks);
    return g_failed ? 1 : 0;
}
