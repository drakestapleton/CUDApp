/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Public declarations for the platform Omega units. The generated headers stay
 * in om/platform-linux/gen so a wildcard src/cudapp_om_*.c cannot link two
 * copies of omega_native_context_create. */
#ifndef CUDAPP_PLAT_EFFECTS_PUBLIC_H
#define CUDAPP_PLAT_EFFECTS_PUBLIC_H
#include "../om/platform-linux/gen/plat_effects.h"
/* Marker poll. Same context type as the fence unit; the wait object defines
 * these symbols. WolframLanguageEvaluator: {2^31, 2^32-1, 1000000/50000,
 * 2000*(1000000/50000)} -> {2147483648, 4294967295, 20, 40000}. */
uint32_t omega2_plat_wait_budget(uint64_t *result, uint64_t o_ms);
uint32_t omega2_ctx_plat_wait_ge(omega_native_context *context, uint64_t *result,
                                 uint64_t o_handle, uint64_t o_off, uint64_t o_want, uint64_t o_polls);
#endif
