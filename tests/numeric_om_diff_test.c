/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Copyright 2026 Drake Stapleton (AIEN / aien-dev)
 *
 * C-vs-Omega differential for om/numeric/numeric.om and om/numeric/tile.om.
 * Built with -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF, so every public
 * function below runs the C oracle (numeric_c_*) and the generated Omega unit
 * and aborts on the first bit difference (src/omega_numeric.c). This driver
 * only supplies inputs:
 *   - exhaustive: all 2^16 F16 and BF16 patterns (decode), every F16/BF16
 *     boundary of FP32 (each representable value, its neighbours and the
 *     rounding midpoints) for the narrowing conversions;
 *   - edges: every ordered pair/triple of a fixed edge list (signed zeros,
 *     subnormals, infinities, quiet and signalling NaNs with payloads,
 *     integer limits, ties, exp/log limits) for every operation;
 *   - random: NUMERIC_DIFF_RANDOM uniform bit patterns per operation
 *     (2^20: a defect hit by 1e-5 of inputs escapes with probability
 *     (1-10^-5)^(2^20), about 2.8e-5, Wolfram).
 * Prints NUMERIC OM DIFF PASS <count> on success.
 */
#include "omega_numeric.h"
#include "omega_numeric_reduce.h"
#include "omega_numeric_om.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF)
#error "build with -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF"
#endif

#ifndef NUMERIC_DIFF_RANDOM
#define NUMERIC_DIFF_RANDOM 1048576u
#endif

static const uint32_t EDGES[] = {
    0x00000000u, 0x80000000u, 0x00000001u, 0x80000001u, 0x007fffffu, 0x807fffffu, 0x00800000u, 0x80800000u,
    0x00400000u, 0x3f800000u, 0xbf800000u, 0x3f000000u, 0xbf000000u, 0x3fc00000u, 0x40200000u, 0xc0200000u,
    0x3f800001u, 0x3f7fffffu, 0x4b000000u, 0x4b7fffffu, 0x4b800000u, 0x4effffffu, 0x4f000000u, 0xcf000000u,
    0xcf000001u, 0x4f800000u, 0x4f7fffffu, 0x42b17218u, 0x42b17217u, 0x42b20000u, 0xc2d00000u, 0xc2cfffffu,
    0xc2d00001u, 0xc2fe0000u, 0x7f7fffffu, 0xff7fffffu, 0x33800000u, 0x33000000u, 0x34000000u, 0x387fe000u,
    0x477fe000u, 0x477ff000u, 0x7f800000u, 0xff800000u, 0x7fc00000u, 0xffc00000u, 0x7f800001u, 0xff800001u,
    0x7fc00001u, 0x7fbfffffu, 0x7fffffffu, 0xffffffffu, 0x40490fdbu, 0x3eaaaaabu,
};
#define NEDGE (sizeof(EDGES) / sizeof(EDGES[0]))

static uint64_t rng = 0x9e3779b97f4a7c15ull;
static uint32_t next32(void) {
    rng ^= rng << 13;
    rng ^= rng >> 7;
    rng ^= rng << 17;
    return (uint32_t)(rng >> 16);
}

static unsigned long long checks;

/* Documented GPU NaN: every NaN a float-producing reference op returns is the
 * canonical 0x7fffffff, on any host (src/omega_numeric_om.h). */
static float cn(const char *what, float r) {
    uint32_t b = omega_float_to_bits(r);
    if ((b & 0x7fffffffu) > 0x7f800000u && b != OMEGA_NUMERIC_CANON_NAN_BITS) {
        fprintf(stderr, "FAIL numeric canon %s: NaN 0x%08x, documented 0x%08x\n", what, b, OMEGA_NUMERIC_CANON_NAN_BITS);
        exit(1);
    }
    return r;
}
#define CN(call) (void)cn(#call, call)

static void unary(uint32_t u) {
    float a = omega_bits_to_float(u);
    (void)omega_ref_i2f((int32_t)u);
    (void)omega_ref_u2f(u);
    (void)omega_ref_f2i(a);
    (void)omega_ref_f2i_floor(a);
    (void)omega_ref_f2i_ceil(a);
    (void)omega_ref_f2i_rni(a);
    (void)omega_ref_f2u(a);
    (void)omega_ref_f32_to_f16(a);
    (void)omega_ref_f32_to_bf16(a);
    CN(omega_math_sqrt(a));
    CN(omega_math_exp(a));
    CN(omega_math_log(a));
    CN(omega_ieee_sqrt(a));
    (void)omega_numeric_host_rcp_seed(a);
    for (int m = 0; m <= OMEGA_CMP_BF16_BITS; m++) (void)omega_numeric_compare_equal((OmegaNumericCompare)m, u, u ^ 1u);
    checks += 20;
}

static void binary(uint32_t ua, uint32_t ub) {
    float a = omega_bits_to_float(ua), b = omega_bits_to_float(ub);
    (void)omega_numeric_bits_equal(a, b);
    CN(omega_ref_fadd(a, b));
    CN(omega_ref_fsub(a, b));
    CN(omega_ref_fmul(a, b));
    CN(omega_ref_fmin(a, b));
    CN(omega_ref_fmax(a, b));
    (void)omega_ref_fsetp_ge(a, b);
    for (int op = 0; op < OMEGA_NOP_COUNT; op++) (void)omega_ref_fsetp_pred(op, a, b);
    CN(omega_math_div(a, b));
    CN(omega_ieee_div(a, b));
    for (int m = 0; m <= OMEGA_CMP_BF16_BITS; m++) (void)omega_numeric_compare_equal((OmegaNumericCompare)m, ua, ub);
    checks += 10 + OMEGA_NOP_COUNT + 5;
}

static void ternary(uint32_t ua, uint32_t ub, uint32_t uc) {
    float a = omega_bits_to_float(ua), b = omega_bits_to_float(ub), c = omega_bits_to_float(uc);
    CN(omega_ref_ffma(a, b, c));
    CN(omega_ref_ffma_int(a, b, c));
    checks += 2;
}

static void tile(const uint32_t t[32]) {
    float x[32], out;
    memcpy(x, t, sizeof(x));
    for (int op = 0; op < OMEGA_RED_COUNT; op++) {
        if (omega_reduce_reference((OmegaReduceOp)op, x, 32, &out) != OMEGA_NUMERIC_OK) {
            fprintf(stderr, "FAIL numeric diff: reduce_reference op %d rc\n", op);
            exit(1);
        }
    }
    CN(omega_warp_reduce_sum(x));
    checks += 5;
}

int main(void) {
    if (!omega_numeric_fpenv_ok()) {
        fprintf(stderr, "FAIL numeric diff: FP environment is not the required one\n");
        return 1;
    }
    /* host-vs-documented NaN: inf*0 (x86 SSE gives 0xffc00000), payload and
     * signalling inputs (AArch64 keeps the payload) all give 0x7fffffff */
    {
        const float inf = omega_bits_to_float(0x7f800000u), z = 0.0f, one = 1.0f;
        const float pay = omega_bits_to_float(0xffc12345u), snan = omega_bits_to_float(0x7f800001u);
        const struct { const char *what; float r; } want[] = {
            { "fmul(inf,0)", omega_ref_fmul(inf, z) }, { "fsub(inf,inf)", omega_ref_fsub(inf, inf) },
            { "fadd(payload,1)", omega_ref_fadd(pay, one) }, { "fadd(snan,1)", omega_ref_fadd(snan, one) },
            { "ffma(inf,0,1)", omega_ref_ffma(inf, z, one) }, { "ffma(1,1,payload)", omega_ref_ffma(one, one, pay) },
            { "ffma_int(1,1,payload)", omega_ref_ffma_int(one, one, pay) }, { "ieee_div(0,0)", omega_ieee_div(z, z) },
            { "ieee_sqrt(-1)", omega_ieee_sqrt(-one) }, { "ieee_div(payload,1)", omega_ieee_div(pay, one) },
            { "math_div(0,0)", omega_math_div(z, z) }, { "math_sqrt(-1)", omega_math_sqrt(-one) },
            { "math_exp(payload)", omega_math_exp(pay) }, { "math_log(-1)", omega_math_log(-one) },
            { "fmin(payload,snan)", omega_ref_fmin(pay, snan) }, { "fmax(snan,payload)", omega_ref_fmax(snan, pay) },
        };
        for (size_t i = 0; i < sizeof(want) / sizeof(want[0]); i++) {
            uint32_t b = omega_float_to_bits(want[i].r);
            if (b != OMEGA_NUMERIC_CANON_NAN_BITS) {
                fprintf(stderr, "FAIL numeric canon %s: 0x%08x, documented 0x%08x\n", want[i].what, b, OMEGA_NUMERIC_CANON_NAN_BITS);
                return 1;
            }
            checks++;
        }
        printf("numeric documented NaN: %zu cases 0x%08x\n", sizeof(want) / sizeof(want[0]), OMEGA_NUMERIC_CANON_NAN_BITS);
    }
    /* exhaustive 16-bit decode and the F16/BF16 boundaries of FP32 */
    for (uint32_t h = 0; h < 65536u; h++) {
        float f = omega_ref_f16_to_f32((uint16_t)h);
        (void)omega_ref_bf16_to_f32((uint16_t)h);
        uint32_t fb = omega_float_to_bits(f), bb = h << 16;
        for (int32_t d = -1; d <= 1; d++) {
            (void)omega_ref_f32_to_f16(omega_bits_to_float(fb + (uint32_t)d));
            (void)omega_ref_f32_to_bf16(omega_bits_to_float(bb + (uint32_t)d));
        }
        /* midpoint to the next F16 / BF16 value and one ULP either side */
        uint32_t fn = omega_float_to_bits(omega_ref_f16_to_f32((uint16_t)(h + 1u)));
        if ((fb >> 31) == (fn >> 31) && fn > fb) {
            uint32_t mid = fb + (fn - fb) / 2u;
            for (int32_t d = -1; d <= 1; d++) (void)omega_ref_f32_to_f16(omega_bits_to_float(mid + (uint32_t)d));
        }
        for (int32_t d = -1; d <= 1; d++) (void)omega_ref_f32_to_bf16(omega_bits_to_float(bb + 0x8000u + (uint32_t)d));
        checks += 14;
    }
    for (size_t i = 0; i < NEDGE; i++) {
        unary(EDGES[i]);
        for (size_t j = 0; j < NEDGE; j++) {
            binary(EDGES[i], EDGES[j]);
            for (size_t k = 0; k < NEDGE; k++) ternary(EDGES[i], EDGES[j], EDGES[k]);
        }
    }
    for (uint32_t n = 0; n < NUMERIC_DIFF_RANDOM; n++) {
        uint32_t a = next32(), b = next32(), c = next32();
        unary(a);
        binary(a, b);
        /* same-exponent-ish pairs exercise cancellation and ties */
        binary(a, (a & 0xff800000u) | (b & 0x007fffffu));
        ternary(a, b, c);
        ternary(a, b, (a & 0xff800000u) + (b & 0x7f000000u) - 0x3f800000u + (c & 0x807fffffu));
        /* the finite exp/log/div/sqrt range: exponent field 0x60..0x9f */
        uint32_t m = (a & 0x807fffffu) | ((0x60u + (b & 0x3fu)) << 23);
        (void)omega_math_exp(omega_bits_to_float(m));
        (void)omega_math_log(omega_bits_to_float(m & 0x7fffffffu));
        (void)omega_math_sqrt(omega_bits_to_float(m & 0x7fffffffu));
        (void)omega_math_div(omega_bits_to_float(m), omega_bits_to_float((c & 0x807fffffu) | ((0x60u + (c >> 26)) << 23)));
        checks += 4;
        (void)omega_reduce_levels((size_t)a * b);
        uint32_t tc = 0, gw = 0;
        omega_numeric_launch_shape((size_t)a, &tc, &gw);
        checks += 2;
    }
    for (size_t n = 0; n < 70; n++) (void)omega_reduce_levels(n < 64 ? ((size_t)1 << n) - 1u : (size_t)-1 - n);
    for (uint32_t n = 0; n < 4096u; n++) {
        uint32_t t[32];
        for (int i = 0; i < 32; i++) t[i] = (next32() & 7u) ? next32() : EDGES[next32() % NEDGE];
        tile(t);
    }
    uint32_t tc = 0, gw = 0;
    omega_numeric_launch_shape(0, &tc, &gw);
    omega_numeric_launch_shape((size_t)-1, &tc, &gw);
    printf("NUMERIC OM DIFF PASS %llu\n", checks);
    return 0;
}
