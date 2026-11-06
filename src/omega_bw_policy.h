/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_BW_POLICY_H
#define OMEGA_BW_POLICY_H
#include "cudapp_native.h"
#include <stdint.h>
/* The only branch here checks the generated ABI, not GPU policy. The scalar
 * unit is total and import-free for these canonical unsigned arguments. */
static inline uint64_t omega_bw_value(uint64_t op, uint64_t x, uint64_t y,
                                      uint64_t z, uint64_t w, uint64_t v) {
    uint64_t result;
    if (omega_bw_scalar(&result, op, x, y, z, w, v) != OMEGA_OK) __builtin_trap();
    return result;
}
#ifndef OMEGA_GPU_NO_POSIX
static inline int omega_bw_id(const uint8_t data[256], uint64_t n, uint8_t out[32]) {
    uint64_t result = 0;
    uint32_t rc = omega_bw_identity(&result, data, 256, n, out, 32);
    return (int)(int64_t)omega_bw_value(8, rc, result, 0, 0, 0);
}
#endif
#endif
