/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_matmul.h"
#include "sha256.h"
#ifndef OMEGA_GPU_NO_POSIX
#include <stdlib.h>
#endif
#include <string.h>
#include "cudapp_native.h"
#ifdef OMEGA_GPU_NO_POSIX
#include "omega_native.h"
#endif
#include "omega_bw_bind.h"
#ifndef BW_BIND_C_PRIMARY
int omega_matmul_spec_init(OmegaMatMulSpec *spec, uint32_t m, uint32_t k, uint32_t n, OmegaMatMulPrecision precision) {
    uint8_t w[256] = {0};
    if (!spec) return -1;
    if (omega_bw_bind(0, (uint64_t)m | (uint64_t)k << 32, (uint64_t)n | (uint64_t)(uint32_t)precision << 32, w) != 16)
        return -1;
    spec->m = m;
    spec->k = k;
    spec->n = n;
    spec->precision = precision;
    if (omega_bw_id(w, 16, spec->spec_id) != 0) return -1;
    return 0;
}
#else
/* Default-OFF C_BIND differential oracle. Both production ABIs bind the spec in Omega
 * (NO_POSIX: omega2_cg_bind through src/omega_bw_bind.h). */
int omega_matmul_spec_init(OmegaMatMulSpec *spec, uint32_t m, uint32_t k, uint32_t n, OmegaMatMulPrecision precision) {
    if (!spec || m == 0 || k == 0 || n == 0) return -1;
    if (m > OMEGA_BW_MATMUL_MAX_M || k > OMEGA_BW_MATMUL_MAX_K || n > OMEGA_BW_MATMUL_MAX_N) return -1;

    spec->m = m;
    spec->k = k;
    spec->n = n;
    spec->precision = precision;

    /* Canonical spec_id computed over (m, k, n, precision) */
    uint8_t spec_buf[16];
    spec_buf[0] = (uint8_t)(m & 0xff);
    spec_buf[1] = (uint8_t)((m >> 8) & 0xff);
    spec_buf[2] = (uint8_t)((m >> 16) & 0xff);
    spec_buf[3] = (uint8_t)((m >> 24) & 0xff);

    spec_buf[4] = (uint8_t)(k & 0xff);
    spec_buf[5] = (uint8_t)((k >> 8) & 0xff);
    spec_buf[6] = (uint8_t)((k >> 16) & 0xff);
    spec_buf[7] = (uint8_t)((k >> 24) & 0xff);

    spec_buf[8] = (uint8_t)(n & 0xff);
    spec_buf[9] = (uint8_t)((n >> 8) & 0xff);
    spec_buf[10] = (uint8_t)((n >> 16) & 0xff);
    spec_buf[11] = (uint8_t)((n >> 24) & 0xff);

    spec_buf[12] = (uint8_t)(precision & 0xff);
    spec_buf[13] = 0;
    spec_buf[14] = 0;
    spec_buf[15] = 0;

    sha256_hash(spec_buf, sizeof(spec_buf), spec->spec_id);
    return 0;
}
#endif /* BW_BIND_C_PRIMARY */

#if defined(OMEGA_GPU_NO_POSIX) || (defined(CUDAPP_MIGRATION_PENDING_BW_CPU) && CUDAPP_MIGRATION_PENDING_BW_CPU)
/* Freestanding exception pending omega-bridge#72; otherwise a default-OFF C oracle. */
int omega_matmul_cpu_oracle_i32(const uint32_t *a, const uint32_t *b, uint32_t *c, uint32_t m, uint32_t k, uint32_t n) {
    if (!a || !b || !c || m == 0 || k == 0 || n == 0) return -1;

    for (uint32_t i = 0; i < m; i++) {
        for (uint32_t j = 0; j < n; j++) {
            uint32_t acc = 0;
            for (uint32_t p = 0; p < k; p++) {
                acc += a[i * k + p] * b[p * n + j];
            }
            c[i * n + j] = acc;
        }
    }
    return 0;
}
#else
int omega_matmul_cpu_oracle_i32(const uint32_t *a, const uint32_t *b, uint32_t *c, uint32_t m, uint32_t k, uint32_t n) {
    if (!a || !b || !c) return -1;
    if (!omega_bw_value(16, m, k, n, 0, 0)) return -1;
    uint64_t result = 0;
    uint32_t rc = omega_bw_cpui(&result, a, omega_bw_value(17, m, k, 0, 0, 0),
                              b, omega_bw_value(17, k, n, 0, 0, 0),
                              c, omega_bw_value(17, m, n, 0, 0, 0));
    return (int)(int64_t)omega_bw_value(8, rc, result, 0, 0, 0);
}
#endif


float omega_fp16_to_fp32(uint16_t h) {
    uint32_t val = (uint32_t)omega_bw_value(1, h, 0, 0, 0, 0);
    float f;
    memcpy(&f, &val, 4);
    return f;
}

float omega_bf16_to_fp32(uint16_t b) {
    uint32_t val = (uint32_t)omega_bw_value(2, b, 0, 0, 0, 0);
    float f;
    memcpy(&f, &val, 4);
    return f;
}

uint16_t omega_fp32_to_fp16(float f) {
    uint32_t bits;
    uint64_t out;
    memcpy(&bits, &f, sizeof(bits));
    if (omega_native_f16(&out, bits) != OMEGA_OK) __builtin_trap();
    return (uint16_t)out;
}

uint16_t omega_fp32_to_bf16(float f) {
#if defined(OMEGA_GPU_NO_POSIX) && !defined(CUDAPP_MIGRATION_PENDING_C_NO_POSIX)
    uint32_t bits;
    uint64_t out;
    memcpy(&bits, &f, sizeof(bits));
    if (omega2_no_bf16(&out, bits) != OMEGA_OK) __builtin_trap();
    return (uint16_t)out;
#else
    return omega_fp32_to_bf16_rne(f);
#endif
}

#if defined(OMEGA_GPU_NO_POSIX) || (defined(CUDAPP_MIGRATION_PENDING_BW_CPU) && CUDAPP_MIGRATION_PENDING_BW_CPU)
/* Freestanding exception pending omega-bridge#72; otherwise default-OFF C oracles. */
int omega_matmul_cpu_oracle_f16(const uint16_t *a, const uint16_t *b, float *c, uint32_t m, uint32_t k, uint32_t n) {
    if (!a || !b || !c || m == 0 || k == 0 || n == 0) return -1;
    for (uint32_t i = 0; i < m; i++) {
        for (uint32_t j = 0; j < n; j++) {
            float sum = 0.0f;
            for (uint32_t p = 0; p < k; p++) {
                sum += omega_fp16_to_fp32(a[i * k + p]) * omega_fp16_to_fp32(b[p * n + j]);
            }
            c[i * n + j] = sum;
        }
    }
    return 0;
}

int omega_matmul_cpu_oracle_bf16(const uint16_t *a, const uint16_t *b, float *c, uint32_t m, uint32_t k, uint32_t n) {
    if (!a || !b || !c || m == 0 || k == 0 || n == 0) return -1;
    for (uint32_t i = 0; i < m; i++) {
        for (uint32_t j = 0; j < n; j++) {
            float sum = 0.0f;
            for (uint32_t p = 0; p < k; p++) {
                sum += omega_bf16_to_fp32(a[i * k + p]) * omega_bf16_to_fp32(b[p * n + j]);
            }
            c[i * n + j] = sum;
        }
    }
    return 0;
}
#else
int omega_matmul_cpu_oracle_f16(const uint16_t *a, const uint16_t *b, float *c, uint32_t m, uint32_t k, uint32_t n) {
    if (!a || !b || !c) return -1;
    if (!omega_bw_value(16, m, k, n, 0, 0)) return -1;
    uint64_t result = 0;
    uint32_t rc = omega_bw_cpuf16(&result, a, omega_bw_value(17, m, k, 0, 0, 0),
                                b, omega_bw_value(17, k, n, 0, 0, 0),
                                c, omega_bw_value(17, m, n, 0, 0, 0));
    return (int)(int64_t)omega_bw_value(8, rc, result, 0, 0, 0);
}
int omega_matmul_cpu_oracle_bf16(const uint16_t *a, const uint16_t *b, float *c, uint32_t m, uint32_t k, uint32_t n) {
    if (!a || !b || !c) return -1;
    if (!omega_bw_value(16, m, k, n, 0, 0)) return -1;
    uint64_t result = 0;
    uint32_t rc = omega_bw_cpubf16(&result, a, omega_bw_value(17, m, k, 0, 0, 0),
                                 b, omega_bw_value(17, k, n, 0, 0, 0),
                                 c, omega_bw_value(17, m, n, 0, 0, 0));
    return (int)(int64_t)omega_bw_value(8, rc, result, 0, 0, 0);
}
#endif
#ifndef OMEGA_GPU_NO_POSIX /* frees what the hosted allocating generators malloc (omega_blackwell_codegen.h, cut A.2) */
void omega_blackwell_kernel_free(OmegaBlackwellKernel *kernel) {
    if (!kernel) return;
    if (kernel->code) {
        free(kernel->code);
        kernel->code = NULL;
    }
    kernel->code_size = 0;
    kernel->insn_count = 0;
}
#endif

#ifndef BW_BIND_C_PRIMARY
int omega_blackwell_bind_matmul_realization(const OmegaMatMulSpec *spec, const OmegaBlackwellKernel *kernel, OmegaBlackwellRealizationIdentity *id) {
    uint8_t w[256] = {0};
    if (!spec || !kernel || !id) return -1;
    if (omega_bw_bind(2, 0, 0, w) != 27) return -1;
    int64_t arch = omega_bw_bind(9, 0, 0, w);
    if (arch < 0) return -1;
    memcpy(id->spec_id, spec->spec_id, 32);
    if (omega_bw_id(w, 27, id->machine_id) != 0) return -1;
    memcpy(id->code_digest, kernel->code_digest, 32);
    id->sm_arch = (uint32_t)arch;
    memcpy(w + 128, id->spec_id, 32);
    memcpy(w + 160, id->machine_id, 32);
    memcpy(w + 192, id->code_digest, 32);
    if (omega_bw_bind(1, id->sm_arch, 0, w) != 100) return -1;
    if (omega_bw_id(w, 100, id->realization_id) != 0) return -1;
    return 0;
}
#else
int omega_blackwell_bind_matmul_realization(const OmegaMatMulSpec *spec, const OmegaBlackwellKernel *kernel, OmegaBlackwellRealizationIdentity *id) {
    if (!spec || !kernel || !id) return -1;

    memcpy(id->spec_id, spec->spec_id, 32);
    /* NVIDIA_DGX_SPARK_GB10_SM121 canonical machine descriptor digest */
    const char *machine_str = "NVIDIA_DGX_SPARK_GB10_SM121";
    sha256_hash((const uint8_t *)machine_str, strlen(machine_str), id->machine_id);
    memcpy(id->code_digest, kernel->code_digest, 32);
    id->sm_arch = 121;

    /* Realization ID = SHA-256(spec_id || machine_id || code_digest || sm_arch) */
    uint8_t bound[100];
    memcpy(bound + 0, id->spec_id, 32);
    memcpy(bound + 32, id->machine_id, 32);
    memcpy(bound + 64, id->code_digest, 32);
    bound[96] = (uint8_t)(id->sm_arch & 0xff);
    bound[97] = (uint8_t)((id->sm_arch >> 8) & 0xff);
    bound[98] = (uint8_t)((id->sm_arch >> 16) & 0xff);
    bound[99] = (uint8_t)((id->sm_arch >> 24) & 0xff);

    sha256_hash(bound, sizeof(bound), id->realization_id);
    return 0;
}
#endif /* BW_BIND_C_PRIMARY */

#include "omega_blackwell_codegen.h"

#ifndef OMEGA_GPU_NO_POSIX
int omega_blackwell_codegen_matmul(const OmegaMatMulSpec *spec, OmegaBlackwellKernel *kernel) {
    if (!spec || !kernel) return -1;
#ifndef BW_BIND_C_PRIMARY
    uint8_t w[256] = {0};
    int64_t g = omega_bw_bind(3, (uint32_t)spec->precision, 0, w);
    if (g == 0) return omega_blackwell_codegen_matmul_i32(spec, kernel);
    if (g == 1) return omega_blackwell_codegen_matmul_tensor(spec, kernel);
    return -1;
#else
    if (spec->precision == OMEGA_MATMUL_PRECISION_INT32) {
        return omega_blackwell_codegen_matmul_i32(spec, kernel);
    } else if (spec->precision == OMEGA_MATMUL_PRECISION_FP16 || spec->precision == OMEGA_MATMUL_PRECISION_BF16) {
        return omega_blackwell_codegen_matmul_tensor(spec, kernel);
    }
    return -1;
#endif
}
#endif
