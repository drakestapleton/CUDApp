/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_FIXTURE_VERIFY_H
#define OMEGA_FIXTURE_VERIFY_H
#include <stdint.h>
#include <stdlib.h>
#ifndef CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST
#define CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST 0
#endif
uint32_t omega_fv_check(uint64_t *, const void *, uint64_t);
static inline uint64_t omega_fv_ask(uint64_t m[16]) {
#if CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST
    switch (m[0]) {
    case 0: case 7: {
        uint64_t mask = m[0] == 7 ? (m[8] ? 15 : 7) : m[9];
        for (unsigned i=0;i<4;++i) if ((mask & (1ull<<i)) && m[1+i] != m[5+i]) return 0;
        return 1;
    }
    case 1: return (uint32_t)-(uint32_t)(2*m[1]+m[2]);
    case 2: return m[2] ? m[1] == 0 : m[1] != 0;
    case 3: return m[9] == 0 ? (m[3]^m[7]) == 0x40000 : m[1] == m[5] && m[2] == m[6] && m[4] == m[8];
    case 4: return m[1] == m[5] && m[2] == (m[6]|0x04000000) && m[3] == m[7] && m[4] == m[8];
    case 5: return m[1] | (m[2]<<m[3]) | (m[4]<<m[5]);
    case 6: return m[1] == m[2] ? m[3] : m[4];
    default: abort();
    }
#else
    uint64_t v;
    if (omega_fv_check(&v, m, 16)) abort();
    return v;
#endif
}
static inline int omega_fv_words(const uint32_t w[4], uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t mask) {
    uint64_t m[16] = {0,w[0],w[1],w[2],w[3],a,b,c,d,mask};
    return (int)omega_fv_ask(m);
}
static inline int omega_fv_optional(const uint32_t w[4], uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint64_t m[16] = {7,w[0],w[1],w[2],w[3],a,b,c,d};
    return (int)omega_fv_ask(m);
}
static inline int omega_fv_error(uint64_t i, uint64_t kind) {
    uint64_t m[16] = {1,i,kind};
    return (int32_t)(uint32_t)omega_fv_ask(m);
}
static inline int omega_fv_status(int rc, int zero) {
    uint64_t m[16] = {2,(uint64_t)(int64_t)rc,(uint64_t)zero};
    return (int)omega_fv_ask(m);
}
static inline int omega_fv_pair(const uint32_t a[4], const uint32_t b[4], uint32_t kind) {
    uint64_t m[16] = {3,a[0],a[1],a[2],a[3],b[0],b[1],b[2],b[3],kind};
    return (int)omega_fv_ask(m);
}
static inline int omega_fv_bfly(const uint32_t a[4], const uint32_t b[4]) {
    uint64_t m[16] = {4,a[0],a[1],a[2],a[3],b[0],b[1],b[2],b[3]};
    return (int)omega_fv_ask(m);
}
static inline uint32_t omega_fv_bits(uint32_t a, uint32_t b, uint32_t bs, uint32_t c, uint32_t cs) {
    uint64_t m[16] = {5,a,b,bs,c,cs};
    return (uint32_t)omega_fv_ask(m);
}
static inline uint32_t omega_fv_choose(uint32_t a, uint32_t b, uint32_t yes, uint32_t no) {
    uint64_t m[16] = {6,a,b,yes,no};
    return (uint32_t)omega_fv_ask(m);
}
#endif
