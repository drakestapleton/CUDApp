/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_DNN_ACTIVATIONS_API_H
#define OMEGA_GPU_DNN_ACTIVATIONS_API_H
#include "omega_gpu_dnn_types.h"
#include "omega_blackwell_codegen.h"
#ifdef __cplusplus
extern "C" {
#endif
#define OMEGA_GPU_DNN_RELU 0u
#define OMEGA_GPU_DNN_GELU_ERF 1u
#define OMEGA_GPU_DNN_GELU_TANH 2u
#define OMEGA_GPU_DNN_SILU 3u
typedef struct { uint64_t elements; uint32_t operation, dtype; } OmegaGpuDnnActivationsShape;
enum { OMEGA_GPU_DNN_ACTIVATIONS_OK=0, OMEGA_GPU_DNN_ACTIVATIONS_BAD_ARGS=1,
       OMEGA_GPU_DNN_ACTIVATIONS_REFUSED=2, OMEGA_GPU_DNN_ACTIVATIONS_CODEGEN=3,
       OMEGA_GPU_DNN_ACTIVATIONS_CHIP=4 };
/* Contiguous finite inputs; f32 output. Nonempty count <= UINT32_MAX.
 * The caller supplies enough readable input and writable output storage.
 * Codegen is host-only; the execution entry requires an authorized GPU window. */
int omega_gpu_dnn_activations_codegen(const OmegaGpuDnnActivationsShape *shape, BlackwellIRProgram *program, uint8_t *code, size_t capacity, OmegaBlackwellKernel *kernel);
int omega_gpu_dnn_activations(const OmegaGpuDnnActivationsShape *shape, const void *input, float *output);
#ifdef __cplusplus
}
#endif
#endif
