/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_MEM_CACHED_H
#define OMEGA_GPU_MEM_CACHED_H
/* GPU-cached device-visible DATA memory for weights or chip-only intermediates.
 * No CPU_READ or HOST_POLLED right is granted: nvos.h NVOS32_ATTR2_GPU_CACHEABLE
 * says system memory is not coherent with direct CPU mappings. Session command,
 * constant-bank, QMD and completion buffers therefore stay GPU-uncached.
 *
 * CPU initialization or rewrite requires omega_gpu_session_sync_to_gpu before
 * submission (the Linux platform tracks required L2 invalidation). To read a
 * chip-produced region on the host, copy it on the GPU into uncached staging,
 * observe completion, then sync/read staging; never dereference this cached
 * buffer as a CPU readback shortcut. The existing q38 graph uses that protocol.
 *
 * Caller holds the session lock with the device open, and retains the allocation
 * until GPU completion. Where an existing versioned slot owns it, pin that slot
 * before taking its address and unpin only after completion, without holding the
 * slot-table lock while polling. Uncertain completion must use the session latch
 * and parking protocol, not release memory still reachable by the device. This
 * allocator does not introduce another lifetime table or select caller roles. */
#include <stddef.h>
#include "omega_gpu_session.h"
#ifdef __cplusplus
extern "C" {
#endif
int omega_gpu_session_alloc_cached(size_t bytes, OmegaGpuMem *out);
#ifdef __cplusplus
}
#endif
#endif
