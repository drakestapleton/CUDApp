/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Decision gates of the Blackwell submit and engine shells, the M16 requalify and
 * concurrent programs and the NVRM lifecycle and submission-visibility gates.
 * Each function asks om/engine/eng.om (ops LIFE, REQF, VIS, BWE, TOMS, SUBM, CONC)
 * and keeps its C oracle for the file's CUDAPP_MIGRATION_PENDING_C_<PART> switch
 * (the includer defines ENG_C_PART) and for the freestanding build.
 * Every function takes plain integers so tests/om_eng_diff_test.c can drive both
 * paths with the same inputs. A binding failure never passes a gate: a predicate
 * returns 0 and a shape or rounding returns -1. Include omega_eng_dec.h first. */
#ifndef OMEGA_ENG_GATES_H
#define OMEGA_ENG_GATES_H

#include <stdint.h>

#include "omega_eng_dec.h"

/* Lifecycle predicate kinds (nvrm/lifecycle_gates.c). Arguments a..f in order. */
enum {
    OMEGA_LIFE_EQ = 0,           /* a == b                                    */
    OMEGA_LIFE_ZEROED = 1,       /* a, b, c, d all 0                          */
    OMEGA_LIFE_NONZERO = 2,      /* a != 0                                    */
    OMEGA_LIFE_UNMAPPED = 3,     /* msync rc a != 0 and errno b == ENOMEM 12  */
    OMEGA_LIFE_QUARANTINE = 4,   /* a, b, c != 0, d == 1, e == f              */
    OMEGA_LIFE_ZERO = 5,         /* a == 0                                    */
    OMEGA_LIFE_RECOVERED = 6,    /* a == 0, b > c, d <= e                     */
    OMEGA_LIFE_OVERFLOW = 7,     /* a, b != 0, c == d, e == f                 */
    OMEGA_LIFE_CHURN = 8,        /* a == 0, b > 256 GiB, c < 2 GiB            */
    OMEGA_LIFE_RETRIED = 9,      /* a == 0, b != 0, c == 3, d == 0            */
    OMEGA_LIFE_BOUNDED = 10,     /* a != 0, b == 0, c == d, e == 3            */
    OMEGA_LIFE_LE = 11,          /* a <= b                                    */
    OMEGA_LIFE_CORRUPT = 13,     /* a, b != 0, c == 1, d != 0                 */
    OMEGA_LIFE_DISTINCT = 14,    /* a != 0, a != b                            */
    OMEGA_LIFE_TORNDOWN = 15,    /* a == 0, b == 0, c == d                    */
    OMEGA_LIFE_BOTHZERO = 16     /* a == 0, b == 0                            */
};

static inline int omega_gate_life(uint32_t k, uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e,
                                  uint64_t f)
{
    if (!eng_on()) {
        switch (k) {
        case OMEGA_LIFE_EQ: return a == b;
        case OMEGA_LIFE_ZEROED: return (a | b | c | d) == 0u;
        case OMEGA_LIFE_NONZERO: return a != 0u;
        case OMEGA_LIFE_UNMAPPED: return a != 0u && b == 12u;
        case OMEGA_LIFE_QUARANTINE: return a != 0u && b != 0u && c != 0u && d == 1u && e == f;
        case OMEGA_LIFE_ZERO: return a == 0u;
        case OMEGA_LIFE_RECOVERED: return a == 0u && b > c && d <= e;
        case OMEGA_LIFE_OVERFLOW: return a != 0u && b != 0u && c == d && e == f;
        case OMEGA_LIFE_CHURN: return a == 0u && b > 256ull * 1024 * 1024 * 1024 && c < 2ull * 1024 * 1024 * 1024;
        case OMEGA_LIFE_RETRIED: return a == 0u && b != 0u && c == 3u && d == 0u;
        case OMEGA_LIFE_BOUNDED: return a != 0u && b == 0u && c == d && e == 3u;
        case OMEGA_LIFE_LE: return a <= b;
        case OMEGA_LIFE_CORRUPT: return a != 0u && b != 0u && c == 1u && d != 0u;
        case OMEGA_LIFE_DISTINCT: return a != 0u && a != b;
        case OMEGA_LIFE_TORNDOWN: return a == 0u && b == 0u && c == d;
        case OMEGA_LIFE_BOTHZERO: return a == 0u && b == 0u;
        default: return 0;
        }
    }
    {
        uint64_t in[7] = {k, a, b, c, d, e, f}, w[16];
        if (eng_ask(ENG_OP_LIFE, in, 7, w) != 0) return 0;
        return w[0] == 1u;
    }
}

/* M16 requalify: REQF k0 marker a stayed 0, k1 word count a (an int) above 0,
 * k2 wait rc a is 0, k3 libcuda-mapped flag a is 0. */
enum { OMEGA_REQF_MARKER_ZERO = 0, OMEGA_REQF_WORDS = 1, OMEGA_REQF_WAIT_OK = 2, OMEGA_REQF_NO_LIBCUDA = 3 };
#define OMEGA_REQF_PAYLOAD 0x16c0ffeeu

static inline int omega_gate_reqf(uint32_t k, int64_t a)
{
    if (!eng_on()) {
        if (k == OMEGA_REQF_WORDS) return a > 0;
        return a == 0;
    }
    {
        uint64_t in[2] = {k, (uint64_t)a}, w[16];
        if (eng_ask(ENG_OP_REQF, in, 2, w) != 0) return 0;
        return w[0] == 1u;
    }
}

/* The requalify release payload, from REQF w1; 0 on binding failure. */
static inline uint32_t omega_gate_reqf_payload(void)
{
    if (!eng_on()) return OMEGA_REQF_PAYLOAD;
    {
        uint64_t in[2] = {0u, 0u}, w[16];
        if (eng_ask(ENG_OP_REQF, in, 2, w) != 0) return 0u;
        return (uint32_t)w[1];
    }
}

/* Submission visibility: VIS k0 negative control (wait rc a != 0, marker b == 0),
 * k1 positive (wait rc a == 0, marker b == payload c), k2 put == retired. */
enum { OMEGA_VIS_NEGATIVE = 0, OMEGA_VIS_POSITIVE = 1, OMEGA_VIS_RETIRED = 2 };

static inline int omega_gate_vis(uint32_t k, uint64_t a, uint64_t b, uint64_t c)
{
    if (!eng_on()) {
        if (k == OMEGA_VIS_NEGATIVE) return a != 0u && b == 0u;
        if (k == OMEGA_VIS_POSITIVE) return a == 0u && b == c;
        if (k == OMEGA_VIS_RETIRED) return a == b;
        return 0;
    }
    {
        uint64_t in[6] = {k, a, b, c, 0u, 0u}, w[16];
        if (k == OMEGA_VIS_RETIRED) {
            in[1] = 0u;
            in[2] = 0u;
            in[4] = a;
            in[5] = b;
        }
        if (eng_ask(ENG_OP_VIS, in, 6, w) != 0) return 0;
        return w[0] == 1u;
    }
}

/* BWE k0: page rounding with a 4096 minimum; -1 for 0 or overflow. */
static inline int omega_gate_round_pages(uint64_t bytes, uint64_t *out)
{
    if (!eng_on()) {
        uint64_t v;
        if (bytes == 0u || bytes > UINT64_MAX - 0xfffu) return -1;
        v = (bytes + 0xfffu) & ~(uint64_t)0xfffu;
        if (v < 0x1000u) v = 0x1000u;
        *out = v;
        return 0;
    }
    {
        uint64_t in[2] = {0u, bytes}, w[16];
        if (eng_ask(ENG_OP_BWE, in, 2, w) != 0 || w[0] != 0u) return -1;
        *out = w[1];
        return 0;
    }
}

/* BWE k1: VECTOR_1D shape. -1 for n == 0 or n > UINT32_MAX - 63. */
static inline int omega_gate_vec_shape(uint64_t n, uint32_t *grid, uint32_t *tpb, uint64_t *need)
{
    if (!eng_on()) {
        if (n == 0u || n > UINT32_MAX - 63u) return -1;
        *grid = (uint32_t)((n + 63u) / 64u);
        *tpb = 64u;
        *need = n * 4u;
        return 0;
    }
    {
        uint64_t in[2] = {1u, n}, w[16];
        if (eng_ask(ENG_OP_BWE, in, 2, w) != 0 || w[0] != 0u) return -1;
        *grid = (uint32_t)w[1];
        *tpb = (uint32_t)w[2];
        *need = w[3];
        return 0;
    }
}

/* BWE k2: QMD1 release words 9, 15, 16, 17, 19 against sem_va. 1 when they match. */
static inline int omega_gate_qmd1_release(uint32_t w9, uint32_t w15, uint32_t w16, uint32_t w17, uint32_t w19,
                                          uint64_t sem_va)
{
    if (!eng_on())
        return w9 == 3u && w15 == (uint32_t)sem_va && w16 == (uint32_t)(sem_va >> 32) && w17 == 6u &&
               w19 == 0x81810000u;
    {
        uint64_t in[7] = {2u, w9, w15, w16, w17, w19, sem_va}, w[16];
        if (eng_ask(ENG_OP_BWE, in, 7, w) != 0) return 0;
        return w[0] == 1u;
    }
}

/* BWE k3: persistence (env present, its first byte) and device reuse.
 * Returns persist; *reuse is dev_open && persist && !faulted. On binding failure
 * persistence is off and nothing is reused (the old one-open-per-job path). */
static inline int omega_gate_persist(int env_present, unsigned char e0, int dev_open, int faulted, int *reuse)
{
    if (!eng_on()) {
        int p = !(env_present && e0 == '1');
        *reuse = dev_open && p && !faulted;
        return p;
    }
    {
        uint64_t in[5] = {3u, env_present != 0, e0, dev_open != 0, faulted != 0}, w[16];
        if (eng_ask(ENG_OP_BWE, in, 5, w) != 0) {
            *reuse = 0;
            return 0;
        }
        *reuse = (int)w[1];
        return (int)w[0];
    }
}

/* TOMS: marker, release semaphore, marker2 and visibility limits in ms. */
static inline int omega_gate_timeouts(uint32_t t[4])
{
    if (!eng_on()) {
        t[0] = t[1] = t[2] = t[3] = 5000u;
        return 0;
    }
    {
        uint64_t w[16];
        if (eng_ask(ENG_OP_TOMS, NULL, 0, w) != 0) return -1;
        t[0] = (uint32_t)w[0];
        t[1] = (uint32_t)w[1];
        t[2] = (uint32_t)w[2];
        t[3] = (uint32_t)w[3];
        return 0;
    }
}

#define OMEGA_GATE_MM_MAX_M 8192u
#define OMEGA_GATE_MM_MAX_K 16384u
#define OMEGA_GATE_MM_MAX_N 262144u

/* SUBM k0: matmul arguments. 1 when accepted. */
static inline int omega_gate_matmul_args(int ptrs, int code, uint64_t m, uint64_t k, uint64_t n)
{
    if (!eng_on())
        return ptrs && code && m != 0u && k != 0u && n != 0u && m <= OMEGA_GATE_MM_MAX_M &&
               k <= OMEGA_GATE_MM_MAX_K && n <= OMEGA_GATE_MM_MAX_N;
    {
        uint64_t in[6] = {0u, ptrs != 0, code != 0, m, k, n}, w[16];
        if (eng_ask(ENG_OP_SUBM, in, 6, w) != 0) return 0;
        return w[0] == 1u;
    }
}

/* SUBM k1 (16x16 tiles) and k2 (MMA: 32x1 threads, grid n/8 by m/16, minimum 1).
 * s[0] grid_x, s[1] grid_y, s[2] threads_x, s[3] threads_y. */
static inline int omega_gate_matmul_shape(int mma, uint64_t m, uint64_t n, uint32_t s[4])
{
    if (!eng_on()) {
        if (mma) {
            s[0] = (uint32_t)(n / 8u);
            s[1] = (uint32_t)(m / 16u);
            if (s[0] == 0u) s[0] = 1u;
            if (s[1] == 0u) s[1] = 1u;
            s[2] = 32u;
            s[3] = 1u;
        } else {
            s[0] = (uint32_t)((n + 15u) / 16u);
            s[1] = (uint32_t)((m + 15u) / 16u);
            s[2] = 16u;
            s[3] = 16u;
        }
        return 0;
    }
    {
        uint64_t in[6] = {mma ? 2u : 1u, 0u, 0u, m, 0u, n}, w[16];
        if (eng_ask(ENG_OP_SUBM, in, 6, w) != 0) return -1;
        s[0] = (uint32_t)w[0];
        s[1] = (uint32_t)w[1];
        s[2] = (uint32_t)w[2];
        s[3] = (uint32_t)w[3];
        return 0;
    }
}

/* SUBM k3: tensor precision is one of the two accepted values. */
static inline int omega_gate_precision(uint64_t p, uint64_t fp16, uint64_t bf16)
{
    if (!eng_on()) return p == fp16 || p == bf16;
    {
        uint64_t in[4] = {3u, p, fp16, bf16}, w[16];
        if (eng_ask(ENG_OP_SUBM, in, 4, w) != 0) return 0;
        return w[0] == 1u;
    }
}

/* CONC: m16_concurrent.c checks. Returns 1 on pass; *exit_code is the failure exit.
 * k0 withheld (all four words 0, exit 2), k1 parity reread (a == want b, marker2 c,
 * exit 7), k2 cross-talk (markerB, marker2B 0, exit 4), k3 final pages (exit 7),
 * k4 distinct roots a != b (exit 6). On binding failure the gate fails with exit 1. */
enum { OMEGA_CONC_WITHHELD = 0, OMEGA_CONC_PARITY = 1, OMEGA_CONC_CROSSTALK = 2, OMEGA_CONC_FINAL = 3,
       OMEGA_CONC_ROOTS = 4 };

static inline int omega_gate_conc(uint32_t k, uint64_t a, uint64_t b, uint64_t c, uint64_t d, int *exit_code)
{
    if (!eng_on()) {
        switch (k) {
        case OMEGA_CONC_WITHHELD: *exit_code = 2; return (a | b | c | d) == 0u;
        case OMEGA_CONC_PARITY: *exit_code = 7; return a == b && c == 0x46464646u;
        case OMEGA_CONC_CROSSTALK: *exit_code = 4; return a == 0u && b == 0u;
        case OMEGA_CONC_FINAL:
            *exit_code = 7;
            return a == 0x16000001u && b == 0x46464646u && c == 0x16000002u && d == 0x46464646u;
        default: *exit_code = 6; return a != b;
        }
    }
    {
        uint64_t in[5] = {k, a, b, c, d}, w[16];
        if (eng_ask(ENG_OP_CONC, in, 5, w) != 0) {
            *exit_code = 1;
            return 0;
        }
        *exit_code = (int)w[1];
        return w[0] == 1u;
    }
}

#endif
