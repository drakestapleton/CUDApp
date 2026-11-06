/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define CUDAPP_MIGRATION_PENDING_C_CHIP_ENUM_POLICY 1
#include "omega_chip_enum.h"
uint64_t oracle_enum(uint64_t m, uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    return omega_chip_enum_decide(m, a, b, c, d);
}
