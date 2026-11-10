/* ORACLE: default-OFF C ABI reference below; all policy/QMD packing is om/encoder/amp_runtime.om + amp_qmd.om. */
/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_ampere_qmd.h"
#include <stddef.h>
#include <string.h>
#ifdef CUDAPP_MIGRATION_PENDING_AMP_QMD_DIFF
#include <stdlib.h>
#endif
/* Original C caller retained for differential verification, never the default. */
#if defined(CUDAPP_MIGRATION_PENDING_AMP_QMD) || defined(CUDAPP_MIGRATION_PENDING_AMP_QMD_DIFF)
extern uint32_t omega_amp_runtime(uint64_t *, const void *, uint64_t, void *, uint64_t);
static int c_ampere_policy(const uint64_t m[16], uint32_t q[64]) {
    uint64_t out[64] = {0}, accepted = 0;
    if (!m || omega_amp_runtime(&accepted, m, 16, out, 64) != 0 || accepted != 1) return -1;
    if (q) for (size_t i = 0; i < 64; i++) q[i] = (uint32_t)out[i];
    return 0;
}
static int c_ampere_build_qmd(uint32_t q[64], uint64_t code, uint64_t cbank,
                           uint64_t semaphore, uint32_t tx, uint32_t ty,
                           uint32_t gx, uint32_t gy, uint32_t registers,
                           uint32_t shared_bytes) {
    const uint64_t m[16] = {3, code, cbank, semaphore, tx, ty, gx, gy, registers, shared_bytes};
    return q ? c_ampere_policy(m, q) : -1;
}
#endif
#ifndef CUDAPP_MIGRATION_PENDING_AMP_QMD
#ifdef OMEGA_GPU_NO_POSIX
#include "omega_native.h"
#define omega_amp_qmd omega2_amp_qmd
#else
#include "cudapp_native.h"
#endif
_Static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, "amp_qmd.om emits little-endian uint32 bytes");
#endif
int omega_ampere_policy(const uint64_t m[16], uint32_t q[64]) {
#ifdef CUDAPP_MIGRATION_PENDING_AMP_QMD
    return c_ampere_policy(m, q);
#else
    uint8_t scratch[256];
    uint64_t accepted = 0;
    if (!m) return -1;
    int rc = omega_amp_qmd(&accepted, 0, m, 16, q ? (void *)q : scratch, 256) == 0 && accepted == 1 ? 0 : -1;
#ifdef CUDAPP_MIGRATION_PENDING_AMP_QMD_DIFF
    uint32_t oracle[64] = {0};
    int crc = c_ampere_policy(m, oracle);
    if (rc != crc || (rc == 0 && q && memcmp(q, oracle, sizeof oracle))) abort();
#endif
    return rc;
#endif
}
int omega_ampere_build_qmd(uint32_t q[64], uint64_t code, uint64_t cbank,
                           uint64_t semaphore, uint32_t tx, uint32_t ty,
                           uint32_t gx, uint32_t gy, uint32_t registers,
                           uint32_t shared_bytes) {
#ifdef CUDAPP_MIGRATION_PENDING_AMP_QMD
    return c_ampere_build_qmd(q, code, cbank, semaphore, tx, ty, gx, gy, registers, shared_bytes);
#else
    const uint64_t a[16] = {code, cbank, semaphore, tx, ty, gx, gy, registers, shared_bytes};
    uint64_t accepted = 0;
    if (!q) return -1;
    int rc = omega_amp_qmd(&accepted, 1, a, 16, (void *)q, 256) == 0 && accepted == 1 ? 0 : -1;
#ifdef CUDAPP_MIGRATION_PENDING_AMP_QMD_DIFF
    uint32_t oracle[64] = {0};
    int crc = c_ampere_build_qmd(oracle, code, cbank, semaphore, tx, ty, gx, gy, registers, shared_bytes);
    if (rc != crc || (rc == 0 && memcmp(q, oracle, sizeof oracle))) abort();
#endif
    return rc;
#endif
}
