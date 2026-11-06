/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_fp8_pack.c: host side of the FP8 e4m3 GEMV (OWN-30B Cut 6). See omega_gpu_fp8_api.h.
 *
 * The weight bytes keep the checkpoint's row-major [n][k] order on the device (a warp reads
 * 32 * lane_bytes contiguous bytes of a row per step), so "packing" is: refuse the two e4m3 NaN
 * codes, convert the bf16 scale grid to f32 times 2^OMEGA_FP8_SCALE_EXP, and stage x as f32 times
 * 2^OMEGA_FP8_X_EXP. All three are exact (powers of two; refused on overflow). The conversions
 * live in om/api/apish_pack.om; this file copies bytes.
 */
#include "omega_gpu_fp8_api.h"
#include "omega_blackwell_codegen_moe.h"
#include "omega_blackwell_matmul.h"
#include "omega_gpu_hostmath.h"
#include "omega_api_shells.h"
#include "omega_api_views_rest.h"
#include <math.h>
#include <string.h>

float omega_fp8_e4m3_ref(uint8_t b) {
    uint32_t bits = omega_api_refb(b);
    float f;
    memcpy(&f, &bits, 4);
    return f;
}

uint32_t omega_fp8_e4m3_kernel_bits(uint8_t b) { return omega_api_kbits(b); }

int omega_fp8_e4m3_is_nan(uint8_t b) { return omega_api_isnanb(b); }

size_t omega_fp8_pack_scan_nan(const uint8_t *w, size_t count, size_t *first) {
#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
    size_t n = 0;
    for (size_t i = 0; i < count; i++)
        if (omega_fp8_e4m3_is_nan(w[i])) { if (n == 0 && first) *first = i; n++; }
    return n;
#else
    uint64_t r = 0, scan[2] = {0, 0};
    ar_check(omega_arnan(&r, w, count, scan, 2));
    if (scan[1] && first) *first = (size_t)scan[0];
    return (size_t)scan[1];
#endif
}

int omega_fp8_pack_scales(const uint16_t *bf16, size_t count, float *out, int mutant_no_fold) {
#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
    for (size_t i = 0; i < count; i++) {
        uint32_t bits = 0;
        if (omega_api_scalef(bf16[i], mutant_no_fold, &bits) != 0) return -1;
        memcpy(&out[i], &bits, 4);
    }
    return 0;
#else
    uint64_t r = 0;
    ar_check(omega_arscale(&r, (uint32_t)mutant_no_fold, bf16, count, out, count));
    return r ? -1 : 0;
#endif
}

void omega_fp8_pack_x_f32(const float *x, uint32_t k, float *out) {
#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
    for (uint32_t i = 0; i < k; i++) {
        uint32_t bits;
        memcpy(&bits, &x[i], 4);
        bits = omega_api_xfold(bits);
        memcpy(&out[i], &bits, 4);
    }
#else
    uint64_t r = 0;
    ar_check(omega_arfold(&r, x, k, out, k));
#endif
}

void omega_fp8_pack_x_bf16(const uint16_t *x, uint32_t k, float *out) {
#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
    for (uint32_t i = 0; i < k; i++) {
        uint32_t bits = omega_api_xfold16(x[i]);
        memcpy(&out[i], &bits, 4);
    }
#else
    uint64_t r = 0;
    ar_check(omega_arfoldh(&r, x, k, out, k));
#endif
}

void omega_fp8_gemv_oracle(const uint8_t *w, const uint16_t *scale_bf16, uint32_t n, uint32_t k,
                           const float *x, double *y, double *yscale) {
#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
    const uint32_t kb = k / OMEGA_FP8_BLOCK;
    for (uint32_t r = 0; r < n; r++) {
        double acc = 0.0, mag = 0.0;
        const uint8_t *row = w + (size_t)r * k;
        const uint16_t *srow = scale_bf16 + (size_t)(r / OMEGA_FP8_BLOCK) * kb;
        for (uint32_t b = 0; b < kb; b++) {
            double s = (double)omega_bf16_to_fp32(srow[b]);
            for (uint32_t c = b * OMEGA_FP8_BLOCK; c < (b + 1) * OMEGA_FP8_BLOCK; c++) {
                double p = (double)omega_fp8_e4m3_ref(row[c]) * (double)x[c] * s;
                acc += p; mag += fabs(p);
            }
        }
        y[r] = acc;
        if (yscale) yscale[r] = mag;
    }
#else
    /* The reference has three runtime input views (six native slots). Returning one f64
     * per call keeps that ABI; magnitude is a second pass only when requested. */
    const size_t kb = k / OMEGA_FP8_BLOCK;
    for (uint32_t r = 0; r < n; r++) {
        uint64_t bits = 0;
        const uint8_t *row = w + (size_t)r * k;
        const uint16_t *sc = scale_bf16 + (size_t)(r / OMEGA_FP8_BLOCK) * kb;
        ar_check(omega_aracc(&bits, row, k, sc, kb, x, k));
        memcpy(&y[r], &bits, sizeof bits);
        if (yscale) {
            ar_check(omega_armag(&bits, row, k, sc, kb, x, k));
            memcpy(&yscale[r], &bits, sizeof bits);
        }
    }
#endif
}
