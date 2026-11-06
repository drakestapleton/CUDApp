/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef MODEL_GRAPHS_NESTED_OBJECTS_H
#define MODEL_GRAPHS_NESTED_OBJECTS_H
#include <stdint.h>
#include "omega_gpu_native_runtime.h"
/* Only resource observations are synthetic. Each implementation includes the
 * production TU, so opaque layout changes cannot silently desynchronise a copy.
 * Weights/state are not executed; no production recorder is wrapped or replaced. */
void *mg_nested_fp8(unsigned slot, uint32_t experts, uint32_t n, uint32_t k);
void *mg_nested_bf16(unsigned slot, uint32_t n, uint32_t k);
void *mg_nested_gdn(unsigned slot, uint32_t ratio, uint32_t layers);
void *mg_nested_attn(unsigned slot, uint32_t layers);
void *mg_nested_attn256(unsigned slot, uint32_t layers, uint32_t qh, uint32_t kvh, uint32_t qpc);
void *mg_nested_router(uint32_t layers, uint32_t experts, uint32_t shared);
void mg_nested_record_begin(void);
int mg_nested_window(uint32_t rows,const OmegaGpuNativeArtifact **a,OmegaGpuNativeLease *lease);
#endif
