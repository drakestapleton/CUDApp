/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/*
 * omega_gpu_gdn_admit.h: the admission and layout decisions of omega_gpu_gdn_api.c. Each returns 0 admitted, -1 refused
 * (the caller maps -1 to OMEGA_GPU_GDN_BAD_ARGS). The decisions are the Omega unit om/gdn/admit.om; the retained C is
 * behind CUDAPP_MIGRATION_PENDING_C_GDN_ADMIT (default off). Pointer checks stay with the callers.
 */
#ifndef OMEGA_GPU_GDN_ADMIT_H
#define OMEGA_GPU_GDN_ADMIT_H
#include <stddef.h>
#include <stdint.h>
#include "omega_blackwell_codegen_gdn.h"

#define OMEGA_GDN_ADMIT_KIND_COPY 4u /* omega_gpu_gdn_api.c KK_COPY */
#define OMEGA_GDN_ADMIT_KIND_ZERO 5u /* KK_ZERO */
#define OMEGA_GDN_PF_SECTIONS 6u

int omega_gdn_admit_shape(uint32_t ratio, uint32_t state_bf16, OmegaGdnShape *s);
int omega_gdn_admit_alloc(uint32_t layers, uint32_t ratio, uint32_t state_bf16, OmegaGdnShape *s);
int omega_gdn_admit_layer(uint32_t layer, uint32_t layers);
int omega_gdn_admit_finite(float eps_l2, float eps_norm, float in_scale, float out_scale);
int omega_gdn_admit_multi(uint32_t layer, uint32_t layers, uint32_t tokens, uint64_t qkv_va, uint64_t z_va, uint32_t xstride,
                          uint64_t ab_va, uint32_t abstride, uint64_t out_va, uint32_t ostride);
int omega_gdn_admit_tokens(uint32_t tokens);
/* the prefill scratch for T <= OMEGA_GPU_GDN_PF_MAX_T tokens: section offsets and total bytes */
int omega_gdn_admit_layout(uint32_t ratio, uint32_t tokens, size_t off[OMEGA_GDN_PF_SECTIONS], size_t *bytes);
int omega_gdn_admit_move(unsigned kind, uint64_t dst, size_t bytes, uint32_t *grid);
#endif
