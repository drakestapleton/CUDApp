/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/*
 * nvrm_policy.h -- decision policy for the NVRM client.
 *
 * Omega owns each decision (native/nvrm/policy.om, unit nvrm_policy). The raw
 * ioctl call sites stay in nvrm.c. The C oracle below is the original formula,
 * compiled only for the differential and when CUDAPP_MIGRATION_PENDING_NVRM_POLICY
 * is set; production defaults to the Omega wrapper.
 *
 * Opcode map: 1 nrstat, 2 hnext, 3 vunits, 4 vcover, 5 palign, 6 pspan,
 * 7 pfull, 8 chadm, 9 retry, 10 mthd, 11 gpentry.
 */
#ifndef NVRM_POLICY_H
#define NVRM_POLICY_H

#include <stdint.h>

#include "cudapp_native.h"

#define NVRM_POL_NRSTAT  1u
#define NVRM_POL_HNEXT   2u
#define NVRM_POL_VUNITS  3u
#define NVRM_POL_VCOVER  4u
#define NVRM_POL_PALIGN  5u
#define NVRM_POL_PSPAN   6u
#define NVRM_POL_PFULL   7u
#define NVRM_POL_CHADM   8u
#define NVRM_POL_RETRY   9u
#define NVRM_POL_MTHD    10u
#define NVRM_POL_GPENTRY 11u

#ifndef CUDAPP_MIGRATION_PENDING_NVRM_POLICY
#define CUDAPP_MIGRATION_PENDING_NVRM_POLICY 0
#endif

/* A result of 0 is a real answer. Bridge failure is the only error path. */
static inline int nvrm_om_call(uint64_t op, const uint64_t in[8], uint64_t *result, uint64_t out[4]) {
    uint64_t scratch[4] = {0, 0, 0, 0};
    uint64_t *dst = out ? out : scratch;
    return omega_nvrm_policy(result, op, in, 8, dst, 4) == 0;
}

/* --- C oracle: the original formulas, for the differential --- */

static inline uint64_t nvrm_c_nrstat(uint64_t st) {
    if (st == 0) return 0;
    if (st == 0x51u) return 1; /* NV_ERR_NO_MEMORY */
    return 2;
}

static inline uint64_t nvrm_c_hnext(uint64_t cls, uint64_t nxt, uint64_t accepted) {
    if (accepted == UINT64_MAX) return (1ull << 32); /* refused: accounting exhausted */
    if (cls == 0x41u) return 0;                      /* NV01_ROOT_CLIENT: handle 0, no advance */
    if (nxt == 0xffffffffu) return (1ull << 32);     /* refused: handle space exhausted */
    return (nxt & 0xffffffffu) | (1ull << 33);       /* handle in bits 0..31, advance in bit 33 */
}

static inline uint64_t nvrm_c_vunits(uint64_t b) {
    uint64_t u = b == 0 ? 1 : b; /* at least one 4 GiB unit */
    if (u > UINT64_MAX - 0xffffffffull) return 0; /* overflow: failure */
    return (u + 0xffffffffull) & ~0xffffffffull;
}

/* 0 = covered or no slot (no CAS), 1 = CAS needed (out[2]=owned_end, out[3]=want_end), 2 = refuse. */
static inline uint64_t nvrm_c_vcover(uint64_t base, uint64_t vanext, uint64_t endv, uint64_t out[4]) {
    if (base == 0) return 0;
    if (endv < base || vanext < base) return 2;
    uint64_t owned = nvrm_c_vunits(vanext - base);
    uint64_t want = nvrm_c_vunits(endv - base);
    if (owned == 0 || want == 0) return 2;
    if (want <= owned) return 0;
    if (base > UINT64_MAX - owned || base > UINT64_MAX - want) return 2;
    out[2] = base + owned;
    out[3] = base + want;
    return 1;
}

static inline uint64_t nvrm_c_palign(uint64_t base, uint64_t align) {
    if (align == 0 || (align & (align - 1)) != 0) return 0; /* not a power of two */
    uint64_t m = align - 1;
    if (base > UINT64_MAX - m) return 0; /* overflow */
    return (base + m) & ~m;
}

/* 0 = ok, 1 = bad span, 2 = VA overflow. */
static inline uint64_t nvrm_c_pspan(uint64_t off, uint64_t nwords, uint64_t size, uint64_t va) {
    if ((off & 3u) != 0) return 1;
    if (nwords == 0 || nwords > 0x1fffffu) return 1;
    uint64_t bytes = nwords * 4u;
    if (off > UINT64_MAX - bytes) return 1;
    if (off + bytes > size) return 1;
    if (va > UINT64_MAX - off) return 2;
    return 0;
}

static inline uint64_t nvrm_c_pfull(uint64_t putv, uint64_t retired, uint64_t entries) {
    uint32_t d = (uint32_t)putv - (uint32_t)retired; /* modulo 2^32, as in C */
    uint32_t e = (uint32_t)entries - 1u;
    return d >= e ? 1u : 0u;
}

/* 0 = ok, 1 = dirty state, 2 = extra exceeds max. */
static inline uint64_t nvrm_c_chadm(uint64_t faulted, uint64_t chgroup, uint64_t regd,
                                    uint64_t nextra, uint64_t extra, uint64_t maxv) {
    if (faulted != 0 || chgroup != 0 || regd != 0 || nextra != 0) return 1;
    if (extra > maxv) return 2;
    return 0;
}

/* 0 = success, 1 = retry, 2 = budget exhausted, 3 = non-memory error. */
static inline uint64_t nvrm_c_retry(uint64_t failed, uint64_t tries, uint64_t budget, uint64_t nomem) {
    if (failed == 0) return 0;
    if (nomem == 0) return 3;
    if (tries >= budget) return 2;
    return 1;
}

static inline uint64_t nvrm_c_mthd(uint64_t subc, uint64_t mthd, uint64_t count) {
    return ((2ull << 28) | ((count & 0xffffffffu) << 16) | ((subc & 0xffffffffu) << 13) |
            ((mthd & 0xffffffffu) >> 2)) &
           0xffffffffull;
}

static inline uint64_t nvrm_c_gpentry(uint64_t va, uint64_t nwords) {
    return (va & ~3ull) | ((nwords & 0x1fffffu) << 42) | (1ull << 41);
}

/* --- Omega wrappers --- */

static inline uint64_t nvrm_om_nrstat(uint64_t st) {
    uint64_t in[8] = {st, 0, 0, 0, 0, 0, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_NRSTAT, in, &result, 0)) return 2;
    return result;
}

static inline uint64_t nvrm_om_hnext(uint64_t cls, uint64_t nxt, uint64_t accepted) {
    uint64_t in[8] = {cls, nxt, accepted, 0, 0, 0, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_HNEXT, in, &result, 0)) return (1ull << 32);
    return result;
}

static inline uint64_t nvrm_om_vunits(uint64_t b) {
    uint64_t in[8] = {b, 0, 0, 0, 0, 0, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_VUNITS, in, &result, 0)) return 0;
    return result;
}

static inline uint64_t nvrm_om_vcover(uint64_t base, uint64_t vanext, uint64_t endv, uint64_t out[4]) {
    uint64_t in[8] = {base, vanext, endv, 0, 0, 0, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_VCOVER, in, &result, out)) return 2;
    return result;
}

static inline uint64_t nvrm_om_palign(uint64_t base, uint64_t align) {
    uint64_t in[8] = {base, align, 0, 0, 0, 0, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_PALIGN, in, &result, 0)) return 0;
    return result;
}

static inline uint64_t nvrm_om_pspan(uint64_t off, uint64_t nwords, uint64_t size, uint64_t va) {
    uint64_t in[8] = {off, nwords, size, va, 0, 0, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_PSPAN, in, &result, 0)) return 1;
    return result;
}

static inline uint64_t nvrm_om_pfull(uint64_t putv, uint64_t retired, uint64_t entries) {
    uint64_t in[8] = {putv, retired, entries, 0, 0, 0, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_PFULL, in, &result, 0)) return 1;
    return result;
}

static inline uint64_t nvrm_om_chadm(uint64_t faulted, uint64_t chgroup, uint64_t regd,
                                     uint64_t nextra, uint64_t extra, uint64_t maxv) {
    uint64_t in[8] = {faulted, chgroup, regd, nextra, extra, maxv, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_CHADM, in, &result, 0)) return 1;
    return result;
}

static inline uint64_t nvrm_om_retry(uint64_t failed, uint64_t tries, uint64_t budget, uint64_t nomem) {
    uint64_t in[8] = {failed, tries, budget, nomem, 0, 0, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_RETRY, in, &result, 0)) return 3;
    return result;
}

static inline uint64_t nvrm_om_mthd(uint64_t subc, uint64_t mthd, uint64_t count) {
    uint64_t in[8] = {subc, mthd, count, 0, 0, 0, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_MTHD, in, &result, 0)) return 0;
    return result;
}

static inline uint64_t nvrm_om_gpentry(uint64_t va, uint64_t nwords) {
    uint64_t in[8] = {va, nwords, 0, 0, 0, 0, 0, 0}, result = 0;
    if (!nvrm_om_call(NVRM_POL_GPENTRY, in, &result, 0)) return 0;
    return result;
}

/* --- production selection: Omega by default, C oracle behind the macro --- */

#if CUDAPP_MIGRATION_PENDING_NVRM_POLICY
#define nvrm_policy_nrstat  nvrm_c_nrstat
#define nvrm_policy_hnext   nvrm_c_hnext
#define nvrm_policy_vunits  nvrm_c_vunits
#define nvrm_policy_vcover  nvrm_c_vcover
#define nvrm_policy_palign  nvrm_c_palign
#define nvrm_policy_pspan   nvrm_c_pspan
#define nvrm_policy_pfull   nvrm_c_pfull
#define nvrm_policy_chadm   nvrm_c_chadm
#define nvrm_policy_retry   nvrm_c_retry
#define nvrm_policy_mthd    nvrm_c_mthd
#define nvrm_policy_gpentry nvrm_c_gpentry
#else
#define nvrm_policy_nrstat  nvrm_om_nrstat
#define nvrm_policy_hnext   nvrm_om_hnext
#define nvrm_policy_vunits  nvrm_om_vunits
#define nvrm_policy_vcover  nvrm_om_vcover
#define nvrm_policy_palign  nvrm_om_palign
#define nvrm_policy_pspan   nvrm_om_pspan
#define nvrm_policy_pfull   nvrm_om_pfull
#define nvrm_policy_chadm   nvrm_om_chadm
#define nvrm_policy_retry   nvrm_om_retry
#define nvrm_policy_mthd    nvrm_om_mthd
#define nvrm_policy_gpentry nvrm_om_gpentry
#endif

#endif /* NVRM_POLICY_H */
