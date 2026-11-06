/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Marker poll through mapget. No device is opened. A memfd is the fixture.
 * WolframLanguageEvaluator: {2^31, 2^32-1, 1000000/50000, 2000*(1000000/50000)}
 * -> {2147483648, 4294967295, 20, 40000}. */
#include "../om/platform-linux/gen/plat_wait.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void) {
    int fd = memfd_create("plat-wait", 0);
    assert(fd >= 0);
    assert(ftruncate(fd, 4096) == 0);
    uint32_t *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    assert(p != MAP_FAILED);
    uint64_t h = 0, r = 0;
    /* WolframLanguageEvaluator: 256*1024*1024 -> 268435456. Alias, not the C mmap address. */
    assert(omega2_plat_wait_hold(&h, 268435456ull, 4096) == 0 && (int64_t)h > 0);
    assert(omega2_plat_wait_bind(&r, h, (uint64_t)fd) == 0 && (int64_t)r == 0);

    p[1] = 5;
    assert(omega2_plat_wait_ge(&r, h, 4, 5, 1) == 0 && (int64_t)r == 0);
    p[1] = 4;
    assert(omega2_plat_wait_ge(&r, h, 4, 5, 2) == 0 && (int64_t)r == -1);
    /* 32-bit wrap: 1 is two past 0xfffffffe. An unmasked compare would miss it. */
    p[1] = 1;
    assert(omega2_plat_wait_ge(&r, h, 4, 0xfffffffe, 1) == 0 && (int64_t)r == 0);
    p[1] = 0xfffffffd;
    assert(omega2_plat_wait_ge(&r, h, 4, 0xfffffffe, 1) == 0 && (int64_t)r == -1);
    /* zero polls still reads once */
    p[1] = 9;
    assert(omega2_plat_wait_ge(&r, h, 4, 9, 0) == 0 && (int64_t)r == 0);

    assert(omega2_plat_wait_budget(&r, 0) == 0 && r == 0);
    assert(omega2_plat_wait_budget(&r, 1) == 0 && r == 20);
    assert(omega2_plat_wait_budget(&r, 2000) == 0 && r == 40000);
    assert(omega2_plat_wait_budget(&r, 2001) == 0 && r == 40000);

    omega_native_context *foreign = NULL;
    assert(omega_native_context_create(4, &foreign) == 0);
    uint32_t st = omega2_ctx_plat_wait_ge(foreign, &r, h, 4, 1, 1);
    assert(st != 0);
    assert(omega_native_context_destroy(foreign) == 0);
    assert(omega2_plat_wait_drop(&r, h) == 0 && (int64_t)r == 0);
    assert(munmap(p, 4096) == 0 && close(fd) == 0);
    printf("PASS platform linux wait\n");
    return 0;
}
