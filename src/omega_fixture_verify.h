/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_FIXTURE_VERIFY_H
#define OMEGA_FIXTURE_VERIFY_H
#include <stdint.h>
#include <string.h>
#ifndef CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST
#define CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST 0
#endif
uint32_t omega_fv_run(uint64_t *, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
static inline uint64_t omega_fv_ask(uint64_t op, uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
#if CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST
    switch (op) {
    case 0: case 7: {
        uint64_t mask = op == 7 ? (d >> 32 ? 15 : 7) : e;
        uint32_t actual[4], expected[4];
        memcpy(actual, &a, 8); memcpy(actual+2, &b, 8);
        memcpy(expected, &c, 8); memcpy(expected+2, &d, 8);
        for (unsigned i=0;i<4;++i) if ((mask & (1ull<<i)) && actual[i] != expected[i]) return 0;
        return 1;
    }
    case 1: return (uint32_t)-(uint32_t)(2*a+b);
    case 2: return b ? a == 0 : a != 0;
    case 3: return e == 0 ? (uint32_t)(b^d) == 0x40000 : a == c && b >> 32 == d >> 32;
    case 4: return a == (c|0x0400000000000000ull) && b == d;
    case 5: return a | (b<<c) | (d<<e);
    case 6: return a == b ? c : d;
    default: __builtin_trap();
    }
#else
    uint64_t v;
    if (omega_fv_run(&v, op, a, b, c, d, e)) __builtin_trap();
    return v;
#endif
}
static inline int omega_fv_words(const uint32_t w[4], uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t mask) {
    uint64_t actual[2], expected[2]; uint32_t words[4] = {a,b,c,d};
    memcpy(actual, w, 16); memcpy(expected, words, 16);
    return (int)omega_fv_ask(0,actual[0],actual[1],expected[0],expected[1],mask);
}
static inline int omega_fv_optional(const uint32_t w[4], uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint64_t actual[2], expected[2]; uint32_t words[4] = {a,b,c,d};
    memcpy(actual, w, 16); memcpy(expected, words, 16);
    return (int)omega_fv_ask(7,actual[0],actual[1],expected[0],expected[1],0);
}
static inline int omega_fv_error(uint64_t i, uint64_t kind) {
    return (int32_t)(uint32_t)omega_fv_ask(1,i,kind,0,0,0);
}
static inline int omega_fv_status(int rc, int zero) {
    return (int)omega_fv_ask(2,(uint64_t)(int64_t)rc,(uint64_t)zero,0,0,0);
}
static inline int omega_fv_pair(const uint32_t a[4], const uint32_t b[4], uint32_t kind) {
    uint64_t x[2], y[2]; memcpy(x,a,16); memcpy(y,b,16);
    return (int)omega_fv_ask(3,x[0],x[1],y[0],y[1],kind);
}
static inline int omega_fv_bfly(const uint32_t a[4], const uint32_t b[4]) {
    uint64_t x[2], y[2]; memcpy(x,a,16); memcpy(y,b,16);
    return (int)omega_fv_ask(4,x[0],x[1],y[0],y[1],0);
}
static inline uint32_t omega_fv_bits(uint32_t a, uint32_t b, uint32_t bs, uint32_t c, uint32_t cs) {
    return (uint32_t)omega_fv_ask(5,a,b,bs,c,cs);
}
static inline uint32_t omega_fv_choose(uint32_t a, uint32_t b, uint32_t yes, uint32_t no) {
    return (uint32_t)omega_fv_ask(6,a,b,yes,no,0);
}
#endif
