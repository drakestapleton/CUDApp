/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_CONV2D_API_H
#define OMEGA_GPU_CONV2D_API_H
#include "omega_blackwell_codegen.h"
#include "omega_gpu_dnn_types.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Contiguous NCHW (layout=0) or NHWC (layout=1), for BOTH input and output.
 * Filter is always contiguous OIHW: [out_channels][in_channels/groups][kh][kw].
 * Cross-correlation, not reversed convolution. Zero padding is explicit on four
 * sides. OH=floor((H+top+bottom-dilation_h*(kh-1)-1)/stride_h)+1, likewise OW.
 * Input/filter storage uses dtype 0=f32,1=bf16,2=f16; accumulation/output f32.
 * No bias, scaling, activation or implicit SAME padding. No im2col workspace. */
typedef struct {
    uint32_t batch, in_channels, height, width, out_channels;
    uint32_t kernel_h, kernel_w, stride_h, stride_w;
    uint32_t pad_top, pad_bottom, pad_left, pad_right;
    uint32_t dilation_h, dilation_w, groups, layout, dtype;
} OmegaGpuConv2dShape;
enum { OMEGA_GPU_CONV2D_OK=0, OMEGA_GPU_CONV2D_BAD_ARGS=1,
       OMEGA_GPU_CONV2D_REFUSED=2, OMEGA_GPU_CONV2D_CODEGEN=3, OMEGA_GPU_CONV2D_CHIP=4 };
/* plan[0:2]=OH,OW; [2:5]=input/filter/output bytes; [5]=launch count;
 * [6:8]=grid_x,threads; [8:11]=byte offsets for a launch; [11]=output row.
 * All planning/refusal occurs before session access. Dimension bound 65535,
 * positive geometry, divisible groups and each tensor/launch count <2^30. */
int omega_gpu_conv2d_plan(const OmegaGpuConv2dShape *shape, uint64_t launch, uint64_t plan[32]);
int omega_gpu_conv2d_codegen(const OmegaGpuConv2dShape *shape, BlackwellIRProgram *program,
                           uint8_t *code, size_t capacity, OmegaBlackwellKernel *kernel);
int omega_gpu_conv2d(const OmegaGpuConv2dShape *shape, const void *image, const void *filter, float *output);
#ifdef __cplusplus
}
#endif
#endif
