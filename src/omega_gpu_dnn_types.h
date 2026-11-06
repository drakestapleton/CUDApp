/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_DNN_TYPES_H
#define OMEGA_GPU_DNN_TYPES_H
/* Contiguous scalar input formats. DNN kernels accumulate and return f32;
 * f16 and bf16 are storage formats, never aliases for one another. */
#define OMEGA_GPU_DNN_F32 0u
#define OMEGA_GPU_DNN_BF16 1u
#define OMEGA_GPU_DNN_F16 2u
#endif
