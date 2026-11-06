/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Boundary regressions for the exact argument layouts used by the C shells. */
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
#define CHECK(c) do { if (!(c)) { printf("FAIL policy line %d: %s\n", __LINE__, #c); return 1; } } while (0)
int main(void) {
    uint64_t a[16] = {0}, o[4] = {0}, r = 0;
    a[0] = 0; a[1] = 0; a[2] = 1; a[3] = 1;
    CHECK(omega_book(&r, 0, a, 16, o, 4) == 0 && r == 0);
    a[2] = 2;
    CHECK(omega_book(&r, 0, a, 16, o, 4) == 0 && r == 1);
    a[0] = 1; a[1] = 0;
    CHECK(omega_book(&r, 1, a, 16, o, 4) == 0 && r == 1);
    a[0] = 0;
    CHECK(omega_book(&r, 1, a, 16, o, 4) == 0 && r == 0);
    a[1] = UINT32_MAX;
    CHECK(omega_book(&r, 1, a, 16, o, 4) == 0 && r == 1);
    a[0] = UINT64_C(4294967296);
    CHECK(omega_decide(&r, 1, a, 16, o, 4) == 0 && r == a[0]);
    a[0] = 0; a[1] = 0; a[2] = 1;
    CHECK(omega_decide(&r, 7, a, 16, o, 4) == 0 && r == 3);
    a[0] = 64; a[1] = 4096; a[2] = 0; a[3] = 1; a[4] = 8192;
    CHECK(omega_decide(&r, 6, a, 16, o, 4) == 0 && r == 0 && o[0] == 8192);
    puts("PASS session policy: stale argument order, latch generation, size_t page and scratch high-water boundaries");
    return 0;
}
