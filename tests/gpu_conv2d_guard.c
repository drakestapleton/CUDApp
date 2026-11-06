/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Link-only safety fence for retained C3 host gates. No fake backend mode. */
#include "omega_gpu_session.h"
#include <stdlib.h>
OmegaGpuSession *__wrap_omega_gpu_session_open(void) {abort();}
void __wrap_omega_gpu_session_lock(void) {abort();}
void __wrap_omega_gpu_session_unlock(void) {abort();}
void __wrap_omega_gpu_session_free(OmegaGpuMem *m) {(void)m;abort();}
int __wrap_omega_gpu_session_alloc(size_t n,OmegaGpuMem *m) {(void)n;(void)m;abort();}
