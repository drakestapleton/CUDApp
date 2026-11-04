/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_ATTENTION_MIRROR_H
#define OMEGA_GPU_ATTENTION_MIRROR_H
#include "omega_gpu_prefill_attn256_api.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Experimental per-call 27B attention history placement. No global setting.
 * mode 0: ordinary path; 1: mirror every supported call; 2: mirror when Omega
 * admits the proposed 1024-row threshold. Other values/shapes are refused.
 * One GPU-private buffer is reused. Complete original K/V, including tail slack,
 * is copied after PREP completes and before ATTEND. The CPU never reads the
 * mirror; original cache stays authoritative for decode and saved state.
 * copy_ns, when non-NULL, receives copy chip time (zero for bypass). It is
 * separate from info.elapsed_ns; info.call_ns includes allocation and copy.
 * Release uses omega_gpu_prefill_attn256_release or session close.
 * Existing host, device and recorded entry points retain their behavior.
 */
int omega_gpu_prefill_attn256_scoped(OmegaGpuDecodeAttn256Cache *c, uint32_t layer,
    uint32_t pos0, uint32_t m, const float *qkv, float eps, float out_scale,
    float *out, OmegaGpuPrefillAttn256Info *info, uint32_t mode, uint64_t *copy_ns);

#ifdef __cplusplus
}
#endif
#endif
