/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host-test stub for links that use tests/fake_m16_native.c instead of nvrm/nvrm.c (which defines the real
 * omega_gpu_live_marker). The fake maps memfd pages through libc mmap, so the platform binds those buffers itself and
 * never reaches this lookup; a lookup that does reach it is refused. */
#include "nvrm.h"

int omega_gpu_live_marker(const Nvrm *rm, const void *cpu, uint64_t *mapping, uint64_t *offset,
                          struct omega_native_context **ctx) {
    (void)rm; (void)cpu; (void)mapping; (void)offset;
    if (ctx) *ctx = 0;
    return -1;
}
