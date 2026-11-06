/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef NVRM_STRUCT_H
#define NVRM_STRUCT_H
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "cudapp_native.h"
#ifndef CUDAPP_MIGRATION_PENDING_C_NVRM_STRUCT
#define CUDAPP_MIGRATION_PENDING_C_NVRM_STRUCT 0
#endif
static inline int ns_pack(void *p, size_t n, uint64_t kind, const uint64_t *a, size_t count) {
    uint64_t got = 0;
    return omega_nvrm_struct(&got, 0, kind, a, count, p, n) || got != n;
}
/* Internal schema/bridge errors are programming defects, never a decoded zero.
 * This cannot encounter driver-controlled sizes: every caller passes sizeof. */
static inline uint64_t ns_get(void *p, size_t n, uint64_t kind, uint64_t field, uint64_t index) {
    uint64_t a[2] = {field, index}, got = 0;
    if (omega_nvrm_struct(&got, 1, kind, a, 2, p, n)) abort();
    return got;
}
#define NS_PACK(p, kind, ...) ns_pack(&(p), sizeof(p), (kind), (const uint64_t[]){__VA_ARGS__}, sizeof((const uint64_t[]){__VA_ARGS__}) / sizeof(uint64_t))
#define NS_FILL(p, kind, ...) do { if (NS_PACK(p, kind, __VA_ARGS__)) return fail(rm, "Omega RM/UVM struct refused"); } while (0)
#if CUDAPP_MIGRATION_PENDING_C_NVRM_STRUCT
#define NS_VALUE(p, kind, field, legacy) (legacy)
#else
#define NS_VALUE(p, kind, field, legacy) NS_GET(p, kind, field)
#endif
#define NS_GET(p, kind, field) ns_get(&(p), sizeof(p), (kind), (field), 0)
#endif
