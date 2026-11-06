/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_BLACKWELL_MATMUL_H
#define OMEGA_BLACKWELL_MATMUL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "omega_bw_policy.h"

#define OMEGA_BW_MATMUL_MAX_M   8192
#define OMEGA_BW_MATMUL_MAX_K   16384
#define OMEGA_BW_MATMUL_MAX_N   262144

/* Precision modes for Milestone 18 tensor matrix multiplication */
typedef enum {
    OMEGA_MATMUL_PRECISION_INT32 = 0, /* Exact integer arithmetic mod 2^32 */
    OMEGA_MATMUL_PRECISION_FP16  = 1, /* FP16 inputs with FP32 accumulator */
    OMEGA_MATMUL_PRECISION_BF16  = 2  /* BF16 inputs with FP32 accumulator */
} OmegaMatMulPrecision;

/* Semantic matrix multiplication operation descriptor in G_S */
typedef struct {
    uint32_t m;
    uint32_t k;
    uint32_t n;
    OmegaMatMulPrecision precision;
    uint8_t spec_id[32];
} OmegaMatMulSpec;

/* Dynamically synthesized sm_121 machine code artifact */
typedef struct {
    uint8_t *code;
    size_t code_size;
    size_t insn_count;
    uint8_t code_digest[32];
    uint32_t gpr_count;
    uint32_t uniform_gpr_count;
} OmegaBlackwellKernel;

/* Four-component realization identity binding */
typedef struct {
    uint8_t spec_id[32];
    uint8_t machine_id[32];
    uint8_t code_digest[32];
    uint32_t sm_arch;
    uint8_t realization_id[32];
} OmegaBlackwellRealizationIdentity;

/* Initializes semantic MatMul specification and computes canonical spec_id */
int omega_matmul_spec_init(OmegaMatMulSpec *spec, uint32_t m, uint32_t k, uint32_t n, OmegaMatMulPrecision precision);

/* Mathematical CPU reference oracles for verification (zero tolerance).
 * Row-major positive m/k/n; c must not overlap either input (inputs may overlap).
 * Hosted checked views return -1 for an aliased output before touching memory.
 * The freestanding C exception is pending omega-bridge#72.
 */
int omega_matmul_cpu_oracle_i32(const uint32_t *a, const uint32_t *b, uint32_t *c, uint32_t m, uint32_t k, uint32_t n);

/* Mathematical CPU reference oracle for FP16 and BF16 tensor matrix multiplication */
int omega_matmul_cpu_oracle_f16(const uint16_t *a, const uint16_t *b, float *c, uint32_t m, uint32_t k, uint32_t n);
int omega_matmul_cpu_oracle_bf16(const uint16_t *a, const uint16_t *b, float *c, uint32_t m, uint32_t k, uint32_t n);

/* Format conversion functions */
float omega_fp16_to_fp32(uint16_t h);
float omega_bf16_to_fp32(uint16_t b);
uint16_t omega_fp32_to_fp16(float f);
uint16_t omega_fp32_to_bf16(float f);
/* Checked import-free Omega round-to-nearest-even conversion. NaNs preserve
 * sign/top payload and are quieted; the caller transfers raw binary32 bits. */
static inline uint16_t omega_fp32_to_bf16_rne(float f) {
    union { float f; uint32_t u; } v = {f};
    return (uint16_t)omega_bw_value(3, v.u, 0, 0, 0, 0);
}

/* Dynamic sm_121 code generator: transforms G_S semantic spec into machine code */
int omega_blackwell_codegen_matmul(const OmegaMatMulSpec *spec, OmegaBlackwellKernel *kernel);

/* Frees resources allocated for dynamically synthesized kernel */
void omega_blackwell_kernel_free(OmegaBlackwellKernel *kernel);

/* Binds realization identity from spec, machine, and dynamically synthesized code */
int omega_blackwell_bind_matmul_realization(const OmegaMatMulSpec *spec, const OmegaBlackwellKernel *kernel, OmegaBlackwellRealizationIdentity *id);

#endif /* OMEGA_BLACKWELL_MATMUL_H */
