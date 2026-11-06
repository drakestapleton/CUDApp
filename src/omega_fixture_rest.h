/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_FIXTURE_REST_H
#define OMEGA_FIXTURE_REST_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifndef CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST
#define CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST 0
#endif
#ifndef CUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS
#define CUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS 0
#endif
enum { FR_NS, FR_MS, FR_ADD, FR_SUB, FR_MAX, FR_FIRST, FR_LAST, FR_SIZE,
       FR_LOG, FR_COUNT, FR_OFFSET, FR_BYTES, FR_PREV, FR_SUM3, FR_FAILED,
       FR_EQ, FR_BOTH, FR_NS_MS, FR_PREV32 };
uint32_t omega_fr_calc(uint64_t *, uint32_t, uint64_t, uint64_t, uint64_t);
uint32_t omega_fr_flush(uint64_t *, uint64_t, uint64_t);
uint32_t omega_fr_scale(uint64_t *, uint64_t, uint32_t);
uint32_t omega_fr_text(uint64_t *, const void *, uint64_t, const void *, uint64_t, uint64_t);
/* Binding failure is fatal, never a plausible timestamp or successful gate. */
static inline uint64_t omega_fixture_calc(uint32_t op, uint64_t a, uint64_t b, uint64_t c) {
#if CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST || CUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS
    switch (op) {
    case FR_NS: return a * 1000000000ull + b;
    case FR_MS: return a * 1000ull + b / 1000000ull;
    case FR_ADD: return a + b;
    case FR_SUB: return a - b;
    case FR_MAX: return a > b ? a : b;
    case FR_FIRST: return a & 255;
    case FR_LAST: return (a ^ 255) & 255;
    case FR_SIZE: { static const uint64_t sizes[] = {4096,65536,1048576,4194304,16777216,33554432,67108864,67108864}; return sizes[a % 8]; }
    case FR_LOG: return a % 2000 == 0;
    case FR_COUNT: return a ? (uint32_t)(b * 2 + 3) : 2;
    case FR_OFFSET: return (uint32_t)((a - 1) * 128);
    case FR_BYTES: return a * 4;
    case FR_PREV: return a - 1;
    case FR_SUM3: return a + b + c;
    case FR_FAILED: return a != 0;
    case FR_EQ: return a == b;
    case FR_BOTH: return a != 0 && b != 0;
    case FR_NS_MS: return a / 1000000;
    case FR_PREV32: return (uint32_t)(a - 1);
    default: abort();
    }
#else
    uint64_t v;
    if (omega_fr_calc(&v, op, a, b, c)) abort();
    return v;
#endif
}
static inline uint32_t omega_fixture_flush(uint64_t ix, uint64_t va) {
#if CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST
    uint64_t m2 = va + 0x10;
    switch (ix) {
    case 0: return (2u << 28) | (4u << 16) | (0x28u >> 2);
    case 4: return 0x10u << 27;
    case 5: return (2u << 28) | (5u << 16) | (0x5cu >> 2);
    case 6: return (uint32_t)m2;
    case 7: return (uint32_t)(m2 >> 32);
    case 8: return 0x46464646;
    case 10: return 1u | (1u << 20);
    default: return 0;
    }
#else
    uint64_t v;
    if (omega_fr_flush(&v, ix, va)) abort();
    return (uint32_t)v;
#endif
}
static inline double omega_fixture_scale(uint64_t val, uint32_t kind) {
#if CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST
    return (double)val / (kind == 0 ? 1073741824.0 : 1048576.0);
#else
    uint64_t v;
    double d;
    if (omega_fr_scale(&v, val, kind)) abort();
    memcpy(&d, &v, sizeof d);
    return d;
#endif
}
static inline int omega_fixture_text(const unsigned char buf[512], const unsigned char pat[32], uint64_t n) {
#if CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST
    if (!n || n > 32) return 0;
    for (size_t i = 0; i < 512 && buf[i]; ++i) {
        if (n <= 512-i && memcmp(buf+i, pat, (size_t)n) == 0) return 1;
    }
    return 0;
#else
    uint64_t v;
    if (omega_fr_text(&v, buf, 512, pat, 32, n)) abort();
    return (int)v;
#endif
}
#endif
