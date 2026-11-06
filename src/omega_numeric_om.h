/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Copyright 2026 Drake Stapleton (AIEN / aien-dev)
 *
 * Private glue between src/omega_numeric.c, src/omega_numeric_reduce.c and
 * the generated Omega units om/numeric/numeric.om (omega_numeric_ref) and
 * om/numeric/tile.om (omega_numeric_tile). Not public API.
 *
 * Build switches (CUDApp migration convention):
 *   default                                 Omega units are the implementation
 *   -DCUDAPP_MIGRATION_PENDING_C_NUMERIC      the old C bodies (oracle)
 *   -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF both on every call; abort on any bit difference
 */
#ifndef OMEGA_NUMERIC_OM_H
#define OMEGA_NUMERIC_OM_H

#include <stdint.h>

#if defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC) && defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF)
#error "CUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF compares C with Omega: not with CUDAPP_MIGRATION_PENDING_C_NUMERIC"
#endif
#if defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC) || defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF)
#define OMEGA_NUMERIC_C_BODIES 1
#endif
#if !defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC)
#define OMEGA_NUMERIC_OM_PATH 1
#endif

/* Operation codes of the numeric_ref dispatcher in om/numeric/numeric.om. */
/* The one NaN every float-producing reference op returns (both paths). */
#define OMEGA_NUMERIC_CANON_NAN_BITS 0x7fffffffU

enum {
    OMEGA_NUMERIC_OM_FADD = 0,
    OMEGA_NUMERIC_OM_FSUB = 1,
    OMEGA_NUMERIC_OM_FMUL = 2,
    OMEGA_NUMERIC_OM_FFMA = 3,
    OMEGA_NUMERIC_OM_FFMA_INT = 4,
    OMEGA_NUMERIC_OM_FMIN = 5,
    OMEGA_NUMERIC_OM_FMAX = 6,
    OMEGA_NUMERIC_OM_FSETP_GE = 7,
    OMEGA_NUMERIC_OM_FSETP_PRED = 8,
    OMEGA_NUMERIC_OM_I2F = 9,
    OMEGA_NUMERIC_OM_U2F = 10,
    OMEGA_NUMERIC_OM_F2I = 11,
    OMEGA_NUMERIC_OM_F2I_FLOOR = 12,
    OMEGA_NUMERIC_OM_F2I_CEIL = 13,
    OMEGA_NUMERIC_OM_F2I_RNI = 14,
    OMEGA_NUMERIC_OM_F2U = 15,
    OMEGA_NUMERIC_OM_F32_TO_F16 = 16,
    OMEGA_NUMERIC_OM_F32_TO_BF16 = 17,
    OMEGA_NUMERIC_OM_F16_TO_F32 = 18,
    OMEGA_NUMERIC_OM_BF16_TO_F32 = 19,
    OMEGA_NUMERIC_OM_MATH_DIV = 20,
    OMEGA_NUMERIC_OM_MATH_SQRT = 21,
    OMEGA_NUMERIC_OM_MATH_EXP = 22,
    OMEGA_NUMERIC_OM_MATH_LOG = 23,
    OMEGA_NUMERIC_OM_IEEE_DIV = 24,
    OMEGA_NUMERIC_OM_IEEE_SQRT = 25,
    OMEGA_NUMERIC_OM_RCP_SEED = 26,
    OMEGA_NUMERIC_OM_BITS_EQUAL = 27,
    OMEGA_NUMERIC_OM_COMPARE_EQUAL = 28,
    OMEGA_NUMERIC_OM_REDUCE_LEVELS = 29,
    OMEGA_NUMERIC_OM_LAUNCH_SHAPE = 30,
};

#if defined(OMEGA_NUMERIC_OM_PATH)
/* omega_numeric_ref through the generated unit; a nonzero unit status traps. */
uint64_t omega_numeric_om(unsigned op, uint64_t a, uint64_t b, uint64_t c);
/* One 32-lane reduce tile (op = OmegaReduceOp) of FP32 bit patterns. */
uint32_t omega_numeric_om_tile(unsigned op, const uint32_t tile[32]);
#endif
#if defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF)
/* Abort with a report when the C oracle and the Omega result differ. */
uint64_t omega_numeric_om_diff(const char *what, uint64_t a, uint64_t b, uint64_t c, uint64_t c_bits,
                               uint64_t om_bits);
#endif

#endif
