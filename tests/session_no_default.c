/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Verification-only platform provider. Exercise hosted Omega decisions against
 * the same explicit fake platform as the freestanding suite, never a driver. */
#include "omega_gpu_platform_linux.h"
const OmegaGpuPlatform *omega_gpu_platform_linux(OmegaGpuPlatformCaps *caps) {
    (void)caps;
    return 0;
}
uint32_t omega_gpu_platform_linux_target_sm(void) {
    return 0;
}
