/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_CHIP_ENUM_H
#define OMEGA_CHIP_ENUM_H
#include "cudapp_native.h"
#include "omega_gpu_chip.h"
#include <stdlib.h>
#ifndef CUDAPP_MIGRATION_PENDING_C_CHIP_ENUM_POLICY
#define CUDAPP_MIGRATION_PENDING_C_CHIP_ENUM_POLICY 0
#endif
/* Modes: row identity, raw vendor, display class, field mask, output slot, dot entry. */
static inline uint64_t omega_chip_enum_decide(uint64_t mode, uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
#if CUDAPP_MIGRATION_PENDING_C_CHIP_ENUM_POLICY
    if (mode == 0) return a == c && b == d;
    if (mode == 1) return a == OMEGA_GPU_PCI_VENDOR_NVIDIA;
    if (mode == 2) return (a >> 16) == 0x03u;
    if (mode == 4) return (int64_t)a < (int64_t)b && c;
    if (mode == 5) return a == '.';
    return a & b;
#else
    uint64_t result;
    if (omega_chip_enum(&result, mode, a, b, c, d) != OMEGA_OK) abort();
    return result;
#endif
}
#endif
