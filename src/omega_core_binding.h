/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_CORE_BINDING_H
#define OMEGA_CORE_BINDING_H
#include <stdint.h>
#ifdef OMEGA_GPU_NO_POSIX
#include "cudapp_freest_native.h"
#define CORE_CALL(name, ...) omega2_##name(__VA_ARGS__)
#define CORE_STOP() __builtin_trap()
#else
#include "cudapp_native.h"
#include <stdlib.h>
#define CORE_CALL(name, ...) omega_##name(__VA_ARGS__)
#define CORE_STOP() abort()
#endif
#define CORE_VOID(name, ...) CORE_CALL(name, &(uint64_t){0}, __VA_ARGS__)
#endif
