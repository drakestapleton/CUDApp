/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* The Omega Runtime API version this library was built at (src/omega_runtime_api.h). Compiled into libomega_gpu.a, so
 * a program can tell the version it was compiled against from the version it actually linked. */
#include "omega_runtime_api.h"

unsigned int omega_runtime_api_version(void) {
    return OMEGA_RUNTIME_API_VERSION;
}
