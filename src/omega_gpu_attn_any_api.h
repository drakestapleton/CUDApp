/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_ATTN_ANY_API_H
#define OMEGA_GPU_ATTN_ANY_API_H
#include <stddef.h>
#include <stdint.h>
#include "omega_blackwell_codegen.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Contiguous [head][sequence][component], no padding. Output is always f32.
 * Causal is top-left aligned: key j is visible iff j <= query i.
 * Finite inputs and finite intermediate dot products are required.
 * f16 is not supported by the scalar IR and is refused, never reinterpreted.
 * This is experimental and host/simulator exercised, not chip qualified. */
typedef struct {
    uint32_t seq_q, seq_kv, head_dim, q_heads, kv_heads, causal, dtype;
    float scale;
} OmegaGpuAttnAnyShape;
#define OMEGA_GPU_ATTN_ANY_MAX_HEAD_DIM 256u
#define OMEGA_GPU_ATTN_ANY_F32 0u
#define OMEGA_GPU_ATTN_ANY_BF16 1u
#define OMEGA_GPU_ATTN_ANY_OK 0
#define OMEGA_GPU_ATTN_ANY_REFUSED 1
#define OMEGA_GPU_ATTN_ANY_BAD_ARGS 2
#define OMEGA_GPU_ATTN_ANY_CODEGEN 3
#define OMEGA_GPU_ATTN_ANY_CHIP 4
/* CPU-only code generation; does not open the GPU. Caller supplies program and
 * code storage, conventionally sizeof(BlackwellIRProgram) and 64 KiB. */
int omega_gpu_attn_any_codegen(const OmegaGpuAttnAnyShape *shape, BlackwellIRProgram *program, uint8_t *code, size_t capacity, OmegaBlackwellKernel *kernel);
/* GPU execution: requires the operator's exclusive window and normal preflight.
 * Unsupported geometry/format is rejected by Omega before the device is opened. */
int omega_gpu_attn_any(const OmegaGpuAttnAnyShape *shape, const void *q, const void *k, const void *v, float *out);
#ifdef __cplusplus
}
#endif
#endif
