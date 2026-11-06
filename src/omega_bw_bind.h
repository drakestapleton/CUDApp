/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_BW_BIND_H
#define OMEGA_BW_BIND_H
/* Spec bytes, the realization binding message, the machine descriptor, matmul dispatch, vector launch geometry,
 * program padding and the code-digest admission are first-party Omega (om/codegen/cg_bind.om, generated
 * src/cudapp_om_cg_bind.c, export omega_cg_bind). The C shells own memory, the structs and the SHA-256 calls. The C
 * kept under CUDAPP_MIGRATION_PENDING_C_BIND (default OFF; always on freestanding, whose link has no hosted trap
 * handling) is the differential oracle of tests/regalloc_om/run.sh. */
#if defined(OMEGA_GPU_NO_POSIX) || defined(CUDAPP_MIGRATION_PENDING_C_BIND)
#define BW_BIND_C_PRIMARY 1
#else
#include <stdint.h>
#include "cudapp_native.h"
/* cg_bind kind a b over the 256-byte wire w; a trap or nonzero status refuses (-1). */
static inline int64_t omega_bw_bind(uint64_t kind, uint64_t a, uint64_t b, uint8_t w[256]) {
    uint64_t r = UINT64_MAX;
    if (omega_cg_bind(&r, kind, a, b, w, 256) != 0) return -1;
    return (int64_t)r;
}
#endif
#endif
