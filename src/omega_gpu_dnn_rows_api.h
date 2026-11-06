/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_DNN_ROWS_API_H
#define OMEGA_GPU_DNN_ROWS_API_H
#include <stddef.h>
#include <stdint.h>
#include "omega_gpu_dnn_types.h"
#include "omega_blackwell_codegen.h"
#ifdef __cplusplus
extern "C" {
#endif
#define OMEGA_GPU_DNN_SOFTMAX 0u
#define OMEGA_GPU_DNN_LAYERNORM 1u
#define OMEGA_GPU_DNN_RMSNORM 2u
#define OMEGA_GPU_DNN_ROWS_OK 0
#define OMEGA_GPU_DNN_ROWS_REFUSED 1
#define OMEGA_GPU_DNN_ROWS_BAD_ARGS 2
#define OMEGA_GPU_DNN_ROWS_CODEGEN 3
#define OMEGA_GPU_DNN_ROWS_CHIP 4
/* Contiguous [rows][length], input dtype from omega_gpu_dnn_types.h.
 * f32 accumulation/output. Pure normalization, no affine transformation.
 * rows,length 1..65535, at most 1073741824 elements.
 * Norm epsilon must be positive finite normal f32. Softmax ignores epsilon.
 * Finite inputs and finite f32 intermediate differences, sums, squares,
 * variance+epsilon and final output are caller preconditions. Experimental;
 * host/simulator evidence is not chip qualification. */
typedef struct { uint32_t rows, length, dtype, op; float epsilon; } OmegaGpuDnnRowsShape;
int omega_gpu_dnn_rows_codegen(const OmegaGpuDnnRowsShape *shape, BlackwellIRProgram *program, uint8_t *code, size_t capacity, OmegaBlackwellKernel *kernel);
/* Operator-exclusive GPU window required. Shape/type/size/epsilon refusal
 * occurs in seeded Omega before any session/device access. */
int omega_gpu_dnn_rows(const OmegaGpuDnnRowsShape *shape, const void *input, float *output);
#ifdef __cplusplus
}
#endif
#endif
