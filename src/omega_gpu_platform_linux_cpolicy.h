/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* C oracle for om/platform-linux/plat_policy.om. Compiled into the platform only when
 * CUDAPP_MIGRATION_PENDING_C_PLATFORM_LINUX is set. The differential test always includes it. */
#ifndef OMEGA_GPU_PLATFORM_LINUX_CPOLICY_H
#define OMEGA_GPU_PLATFORM_LINUX_CPOLICY_H
#include <stdint.h>
#include <string.h>

static uint64_t plat_c_byte(const uint64_t m[16], uint64_t idx) {
    return (m[2 + idx / 8] >> ((idx % 8) * 8)) & 255u;
}

static int plat_c_sp(uint64_t c) {
    return c == 32 || c == 9 || c == 10 || c == 11 || c == 12 || c == 13;
}

static uint64_t plat_c(const uint64_t m[16], uint64_t q[16]) {
    if (q) memset(q, 0, 16 * sizeof q[0]);
    if (m[0] == 1) {
        q[0] = (2ull << 28) | (4ull << 16) | (0x0028ull >> 2);
        q[4] = 0x11ull << 27;
        return 5;
    }
    if (m[0] == 2) {
        if (m[1] != 1 || m[2] < '2' || m[2] > '4' || m[3] != 0) return 1;
        return m[2] - '0';
    }
    if (m[0] == 3) {
        if (m[3] == 0 || m[2] > m[1] || m[3] > m[1] - m[2]) return 7;
        return 0;
    }
    if (m[0] == 4) {
        if (m[5] != 0) { q[0] = m[5]; return 1; }
        uint64_t local = m[1] && m[2] != 4 && (m[3] & 32u) == 0;
        uint64_t cached = m[4];
        if (local && cached) { q[0] = 10; return 1; }
        if (cached && (m[2] != 2 || (m[3] & 33u))) { q[0] = 10; return 1; }
        q[1] = local;
        q[2] = cached;
        q[3] = local ? 1 : cached ? 2 : 3;
        return 1;
    }
    if (m[0] == 5) {
        if (m[1] == 0 || m[1] > (UINT64_MAX - 0xfffull)) return 0;
        q[0] = (m[1] + 0xfffull) & ~0xfffull;
        return 1;
    }
    if (m[0] == 7) {
        uint64_t n = m[1];
        if (n == 0 || n > 32 || plat_c_byte(m, 0) == '-') return 0;
        uint64_t i = 0;
        while (i < n && plat_c_sp(plat_c_byte(m, i))) i++;
        int sign = 0;
        if (i < n) {
            uint64_t c = plat_c_byte(m, i);
            if (c == '+') i++;
            else if (c == '-') { sign = 1; i++; }
        }
        if (i == n) return 0;
        uint64_t acc = 0;
        for (; i < n; i++) {
            uint64_t d = plat_c_byte(m, i);
            if (d < '0' || d > '9') return 0;
            uint64_t dig = d - '0';
            if (acc > 1844674407370955161ull) return 0;
            acc *= 10;
            if (acc > UINT64_MAX - dig) return 0;
            acc += dig;
        }
        q[0] = sign ? 0ull - acc : acc;
        return 1;
    }
    if (m[0] == 8) return m[1] >= m[2] ? 12 : 0;
    if (m[0] == 9) {
        if (m[1] == 0 || m[1] > UINT64_MAX - m[2]) return 7;
        uint64_t words = m[1] + m[2];
        if (words > (UINT64_MAX / 4) || words * 4 > m[3]) return 7;
        return 0;
    }
    if (m[0] == 10) {
        if ((m[1] & 32u) == 0) return 8;
        if (m[2] & 3u) return 1;
        return 0;
    }
    if (m[0] == 12) {
        if (m[1] < 2) return 0;
        for (uint64_t i = 0; i < 3 && i + 1 < m[1]; i++) {
            uint64_t gen = m[5 + i * 2];
            if (gen && m[4 + i * 2] == m[2] && gen == m[3]) return i + 1;
        }
        return 0;
    }
    if (m[0] == 13) return m[1] && m[2] ? 1 : 0;
    if (m[0] == 14) return m[1] == 0 ? 0 : (m[2] ? m[2] : 121);
    return 0;
}
#endif
