/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Byte-exact differential: om/platform-linux/plat_policy.om against the C oracle.
 * Does not open a device. Chip tier is NOT_RUN. */
#include "omega_gpu_platform_linux_cpolicy.h"
#include <stdio.h>
#include <string.h>

#include "cudapp_om_plat_policy.c"

static int fail(const char *what) {
    fprintf(stderr, "FAIL %s\n", what);
    return 1;
}

static int one(const uint64_t m[16], const char *what) {
    uint64_t q_om[16], q_c[16], result = 0;
    memset(q_om, 0, sizeof q_om);
    memset(q_c, 0, sizeof q_c);
    uint32_t st = omega_plat_policy(&result, m, 16, q_om, 16);
    if (st != 0) return fail(what);
    uint64_t c = plat_c(m, q_c);
    if (c != result || memcmp(q_om, q_c, sizeof q_om) != 0) return fail(what);
    return 0;
}

static void pack(uint64_t m[16], const char *s) {
    memset(m, 0, 16 * sizeof m[0]);
    m[0] = 7;
    m[1] = strlen(s);
    memcpy(&m[2], s, m[1]);
}

int main(void) {
    uint64_t m[16];
    memset(m, 0, sizeof m);
    m[0] = 1;
    if (one(m, "l2")) return 1;
    m[0] = 2; m[1] = 0;
    if (one(m, "chan-null")) return 1;
    m[1] = 1; m[2] = '3'; m[3] = 0;
    if (one(m, "chan-3")) return 1;
    m[2] = '5';
    if (one(m, "chan-5")) return 1;
    m[1] = 2; m[2] = '2'; m[3] = 0;
    if (one(m, "chan-long")) return 1;
    memset(m, 0, sizeof m);
    m[0] = 3; m[1] = 16; m[2] = 8; m[3] = 8;
    if (one(m, "range-ok")) return 1;
    m[3] = 9;
    if (one(m, "range-over")) return 1;
    m[3] = 0;
    if (one(m, "range-zero")) return 1;
    memset(m, 0, sizeof m);
    m[0] = 4; m[1] = 1; m[2] = 2; m[3] = 4; m[4] = 0; m[5] = 0;
    if (one(m, "alloc-vram")) return 1;
    m[4] = 1;
    if (one(m, "alloc-local-cached")) return 1;
    m[1] = 0; m[4] = 1; m[2] = 2; m[3] = 4;
    if (one(m, "alloc-cached")) return 1;
    m[3] = 1;
    if (one(m, "alloc-cached-read")) return 1;
    m[5] = 9;
    if (one(m, "alloc-check")) return 1;
    memset(m, 0, sizeof m);
    m[0] = 5; m[1] = 1;
    if (one(m, "page-1")) return 1;
    m[1] = 0;
    if (one(m, "page-0")) return 1;
    m[1] = UINT64_MAX;
    if (one(m, "page-max")) return 1;
    const char *mins[] = {"0", "42", "18446744073709551615", "18446744073709551616", "", "-", "+", "  -1", " +12", "12a", "  7"};
    for (unsigned i = 0; i < sizeof mins / sizeof mins[0]; i++) {
        pack(m, mins[i]);
        if (one(m, mins[i][0] ? mins[i] : "empty")) return 1;
    }
    memset(m, 0, sizeof m);
    m[0] = 8; m[1] = 1; m[2] = 1;
    if (one(m, "busy")) return 1;
    m[1] = 0;
    if (one(m, "idle")) return 1;
    memset(m, 0, sizeof m);
    m[0] = 9; m[1] = 4; m[2] = 5; m[3] = 36;
    if (one(m, "fit-ok")) return 1;
    m[3] = 35;
    if (one(m, "fit-over")) return 1;
    m[1] = 0;
    if (one(m, "fit-zero")) return 1;
    memset(m, 0, sizeof m);
    m[0] = 10; m[1] = 32; m[2] = 4;
    if (one(m, "poll-ok")) return 1;
    m[1] = 1;
    if (one(m, "poll-rights")) return 1;
    m[1] = 32; m[2] = 2;
    if (one(m, "poll-align")) return 1;
    memset(m, 0, sizeof m);
    m[0] = 12; m[1] = 3; m[2] = 7; m[3] = 9; m[4] = 1; m[5] = 2; m[6] = 7; m[7] = 9;
    if (one(m, "xq-hit")) return 1;
    m[7] = 8;
    if (one(m, "xq-miss")) return 1;
    memset(m, 0, sizeof m);
    m[0] = 13; m[1] = 1; m[2] = 1;
    if (one(m, "pending")) return 1;
    m[2] = 0;
    if (one(m, "pending-off")) return 1;
    memset(m, 0, sizeof m);
    m[0] = 14; m[1] = 1; m[2] = 0;
    if (one(m, "sm-default")) return 1;
    m[2] = 86;
    if (one(m, "sm-86")) return 1;
    m[1] = 0;
    if (one(m, "sm-closed")) return 1;
    m[0] = 99;
    if (one(m, "unknown")) return 1;
    /* Negative control: a caller that expects the L2 word to be zero must fail. */
    memset(m, 0, sizeof m);
    m[0] = 1;
    uint64_t q[16], result = 0;
    if (omega_plat_policy(&result, m, 16, q, 16) != 0 || q[0] == 0) return fail("l2-nonzero");
    printf("PASS platform linux policy differential\n");
    return 0;
}
