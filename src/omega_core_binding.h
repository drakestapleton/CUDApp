/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_CORE_BINDING_H
#define OMEGA_CORE_BINDING_H
#include <stdint.h>
#ifdef OMEGA_GPU_NO_POSIX
/* MIGRATION-PENDING: the pinned bridge-freest refuses om/core/sha256.om and fmtnum.om with error 24
 * (unsupported freest scalar capability), so freestanding objects keep the C core (sha256.c and
 * omega_gpu_fmt.c select it under OMEGA_GPU_NO_POSIX) and must not reach this binding. */
#error "omega_core_binding.h has no freestanding binding yet; the C core is used under OMEGA_GPU_NO_POSIX"
#else
#include "cudapp_native.h"
#include <stdlib.h>
#define CORE_CALL(name, ...) omega_##name(__VA_ARGS__)
#define CORE_STOP() abort()
#endif
#define CORE_VOID(name, ...) CORE_CALL(name, &(uint64_t){0}, __VA_ARGS__)
#endif
