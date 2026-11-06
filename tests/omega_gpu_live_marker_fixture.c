/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host-test stub. Not the live path. Integrator replaces this with the checked
 * lookup before any link that polls a real NVRM marker. */
#include <stdint.h>
struct omega_native_context;
int omega_gpu_live_marker(const void *cpu, uint64_t *mapping, uint64_t *offset, struct omega_native_context **ctx) {
    (void)cpu; (void)mapping; (void)offset; (void)ctx;
    return -1;
}
