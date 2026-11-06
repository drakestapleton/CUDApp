/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_HOST_POLICY_H
#define OMEGA_HOST_POLICY_H
#include "cudapp_native.h"
#ifndef CUDAPP_MIGRATION_PENDING_C_API_POLICY
#define CUDAPP_MIGRATION_PENDING_C_API_POLICY 0
#endif
/* Internal, not a public runtime API. All dimensions arrive as uint32_t. */
static inline int omega_host_shape(unsigned mode, uint32_t a, uint32_t b, uint32_t c) {
#if CUDAPP_MIGRATION_PENDING_C_API_POLICY
    switch (mode) {
    case 0: return (!a || !b || !c) ? -1 : (a > 8192u || b > 16384u || c > 262144u) ? -2 : 0;
    case 1: return (!a || !b || b % 128u) ? -1 : b > 16384u ? -2 : 0;
    case 2: return (!a || b < 2u || (b & 1u)) ? -1 : b > 2048u ? -2 : 0;
    case 3: return (!a || !b || !c) ? -1 : (c > 256u || a > 1024u) ? -2 : a % b ? -1 : 0;
    case 4: return a > c || b > c - a ? -1 : 0;
    case 5: return (!a || !b || b % 256u) ? -1 : b > c ? -2 : 0;
    case 6: return ((a != 128u && a != 256u) || b > 1u) ? -1 : 0;
    default: return -1;
    }
#else
    uint64_t result = 1;
    if (omega_api_policy(&result, mode, a, b, c) != OMEGA_OK || result > 2) return -1;
    return -(int)result;
#endif
}
#endif
