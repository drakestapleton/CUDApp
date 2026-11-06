/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_API_VIEWS_REST_H
#define OMEGA_API_VIEWS_REST_H
#include "cudapp_native.h"
#include "omega_gpu_session.h"
#include <stddef.h>
#include <stdlib.h>
#ifndef CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
#define CUDAPP_MIGRATION_PENDING_API_VIEWS_REST 0
#endif
/* A bridge trap is an internal shape/layout error, never a successful partial conversion. */
static inline void ar_check(uint32_t trap) { if (trap) abort(); }
static inline void ar_args(uint32_t *out, size_t words, const uint64_t *va, size_t count) {
    uint64_t r = 0;
    ar_check(omega_arargs(&r, va, count, out, words));
    if (r) abort();
}
static inline void ar_fill(uint32_t *out, size_t n, uint32_t value) {
    uint64_t r = 0;
    ar_check(omega_arfill(&r, value, out, n));
}
static inline size_t ar_count(const uint32_t *p, size_t n, uint32_t value) {
    uint64_t r = 0;
    ar_check(omega_arcount(&r, value, p, n));
    return (size_t)r;
}
static inline void ar_words(void *out, size_t words, const uint32_t *in, size_t count) {
    uint64_t r = 0;
    ar_check(omega_arpad(&r, in, count, out, words));
    if (r) abort();
}
static inline void ar_copy(void *out, const void *in, size_t bytes) {
    uint64_t r = 0;
    ar_check(omega_arcopy(&r, in, bytes, out, bytes));
    if (r) abort();
}
static inline uint64_t ar_plan(uint64_t q[32]) {
    uint64_t r = 0;
    ar_check(omega_arcalc(&r, q, 32));
    return r;
}
static inline void ar_args_extra(uint32_t *out, size_t words, const uint64_t *va, size_t count,
                                 const uint32_t *scalar, size_t nscalar) {
    uint64_t r = 0;
    ar_check(omega_arargsx(&r, va, count, scalar, nscalar, out, words));
    if (r) abort();
}
static inline void ar_key(void *out, size_t words, uint32_t kind, const void *spec, size_t count) {
    uint64_t r = 0;
    ar_check(omega_arkey(&r, kind, spec, count, out, words));
    if (r) abort();
}
static inline size_t ar_cache(const void *slots, size_t bytes, size_t count, size_t stride,
                              size_t valid, size_t offset, const void *key, size_t keybytes) {
    const uint64_t q[8] = {count, stride, valid, offset, keybytes};
    uint64_t r = UINT64_MAX;
    ar_check(omega_arcache(&r, q, 8, slots, bytes, key, keybytes));
    return (size_t)r;
}
_Static_assert(sizeof(void *) == sizeof(uint64_t), "launch pointer width");
_Static_assert(sizeof(OmegaGpuLaunch) == 80, "launch view extent");
_Static_assert(offsetof(OmegaGpuLaunch, code_va) == 0, "launch code offset");
_Static_assert(offsetof(OmegaGpuLaunch, gpr_count) == 8, "launch gpr offset");
_Static_assert(offsetof(OmegaGpuLaunch, threads_x) == 12, "launch threads offset");
_Static_assert(offsetof(OmegaGpuLaunch, threads_y) == 16, "launch threads y offset");
_Static_assert(offsetof(OmegaGpuLaunch, grid_x) == 20, "launch grid offset");
_Static_assert(offsetof(OmegaGpuLaunch, grid_y) == 24, "launch grid y offset");
_Static_assert(offsetof(OmegaGpuLaunch, num_elements) == 32, "launch elements offset");
_Static_assert(offsetof(OmegaGpuLaunch, args) == 40, "launch args offset");
_Static_assert(offsetof(OmegaGpuLaunch, n_args) == 48, "launch argc offset");
_Static_assert(offsetof(OmegaGpuLaunch, timeout_ms) == 56, "launch timeout offset");
_Static_assert(offsetof(OmegaGpuLaunch, shared_bytes) == 64, "launch shared offset");
_Static_assert(offsetof(OmegaGpuLaunch, spin_us) == 68, "launch spin offset");
_Static_assert(offsetof(OmegaGpuLaunch, target_sm) == 72, "launch sm offset");
static inline void ar_launch(OmegaGpuLaunch *launch, const uint64_t q[16]) {
    uint64_t r = 0;
    ar_check(omega_arlaunch(&r, q, 16, launch, 20));
    if (r) abort();
}
#endif
