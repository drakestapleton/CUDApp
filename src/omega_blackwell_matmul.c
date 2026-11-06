/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_matmul.h"
#include "sha256.h"
#ifndef OMEGA_GPU_NO_POSIX
#include <stdlib.h>
#endif
#include <string.h>
#include "omega_bw_policy.h"

int omega_matmul_spec_init(OmegaMatMulSpec *spec, uint32_t m, uint32_t k, uint32_t n, OmegaMatMulPrecision precision) {
    if (!spec || !omega_bw_value(0, m, k, n, 0, 0)) return -1;

    spec->m = m;
    spec->k = k;
    spec->n = n;
    spec->precision = precision;

#ifdef OMEGA_GPU_NO_POSIX
    /* Freestanding exception pending omega-bridge#72 adoption: the portable
     * import-free converter refuses SHA array units. Hosted uses Omega below. */
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
#else
    const uint64_t nums[4] = {m, k, n, (uint32_t)precision};
    const uint8_t ids[96] = {0};
    return omega_bw_id(0, nums, ids, spec->spec_id);
#endif
    return 0;
}

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


float omega_fp16_to_fp32(uint16_t h) {
    uint32_t val = (uint32_t)omega_bw_value(1, h, 0, 0, 0, 0);
    float f;
    memcpy(&f, &val, sizeof f);
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
    return omega_fp32_to_bf16_rne(f);
}

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

int omega_blackwell_bind_matmul_realization(const OmegaMatMulSpec *spec, const OmegaBlackwellKernel *kernel, OmegaBlackwellRealizationIdentity *id) {
    if (!spec || !kernel || !id) return -1;

#ifdef OMEGA_GPU_NO_POSIX
    /* Same freestanding SHA exception as omega_matmul_spec_init. */
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
#else
    const uint64_t nums[4] = {121, 0, 0, 0};
    uint8_t ids[96] = {0};
    memcpy(id->spec_id, spec->spec_id, 32);
    if (omega_bw_id(1, nums, ids, id->machine_id) != 0) return -1;
    memcpy(id->code_digest, kernel->code_digest, 32);
    id->sm_arch = 121;
    memcpy(ids, id->spec_id, 32);
    memcpy(ids + 32, id->machine_id, 32);
    memcpy(ids + 64, id->code_digest, 32);
    return omega_bw_id(2, nums, ids, id->realization_id);
#endif
    return 0;
}

#include "omega_blackwell_codegen.h"

#ifndef OMEGA_GPU_NO_POSIX
int omega_blackwell_codegen_matmul(const OmegaMatMulSpec *spec, OmegaBlackwellKernel *kernel) {
    if (!spec || !kernel) return -1;
    uint64_t route = omega_bw_value(5, (uint32_t)spec->precision, 0, 0, 0, 0);
    if (route == 0) {
        return omega_blackwell_codegen_matmul_i32(spec, kernel);
    } else if (route == 1) {
        return omega_blackwell_codegen_matmul_tensor(spec, kernel);
    }
    return -1;
}
#endif
