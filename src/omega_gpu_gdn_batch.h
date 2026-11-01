/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_GDN_BATCH_H
#define OMEGA_GPU_GDN_BATCH_H
#include "omega_gpu_gdn_api.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Experimental per-call GDN prefill submission. Mode 0 keeps the ordinary
 * path; mode 1 asks Omega to admit and order the existing PREP/SCAN/POST kernels
 * in one command list. Other modes refuse. Arithmetic, input staging, output
 * poison checks, tails and recurrent state keep the existing layouts.
 * One list is reused per GDN object and freed by omega_gpu_gdn_free. List
 * creation/free acquire the session lock themselves; submission is serialized
 * by the session lock. A recovered session refuses an earlier list.
 * No output is copied until every launch completed and host synchronization
 * and poison checks succeeded. With info, elapsed_ns contains the three GPU
 * timestamp intervals including their ordering barriers, and call_ns includes
 * allocation and submission. With NULL info, timestamp reports are omitted.
 * Existing host, recorded and decode entries keep their behavior. No global
 * selector or default change. See om/gdn-batch/README.md for qualification.
 */
int omega_gpu_gdn_prefill_scoped(OmegaGpuGdn *g, uint32_t layer, uint32_t tokens,
    const float *qkv, const float *z, const float *ab, float eps_l2,
    float eps_norm, float in_scale, float out_scale, float *out,
    OmegaGpuGdnPfInfo *info, uint32_t mode);

#ifdef __cplusplus
}
#endif
#endif
