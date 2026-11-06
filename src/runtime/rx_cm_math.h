/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef RX_CM_MATH_H
#define RX_CM_MATH_H
#include "cudapp_native.h"
#include <math.h>
#include <stdint.h>
#include <string.h>
#ifndef CUDAPP_MIGRATION_PENDING_C_CM_MATH
#define CUDAPP_MIGRATION_PENDING_C_CM_MATH 0
#endif
#if !CUDAPP_MIGRATION_PENDING_C_CM_MATH
/* Binary64 transport only. Numerical computation and exceptional-value policy
 * are in the first-party Omega unit, not a host libm call. */
static inline double rx_host_math(unsigned mode, double x) {
    uint64_t input, output;
    memcpy(&input, &x, sizeof input);
    if (omega_cm_math(&output, mode, input) != OMEGA_OK) return NAN;
    memcpy(&x, &output, sizeof x);
    return x;
}
static inline double rx_host_log(double x) { return rx_host_math(0, x); }
static inline double rx_host_log2(double x) { return rx_host_math(1, x); }
static inline double rx_host_ceil(double x) { return rx_host_math(2, x); }
static inline double rx_host_exp(double x) { return rx_host_math(3, x); }
static inline double rx_host_exp2(double x) { return rx_host_math(4, x); }
static inline double rx_host_sqrt(double x) { return rx_host_math(5, x); }
static inline double rx_host_erfc(double x) { return rx_host_math(6, x); }
static inline double rx_host_fabs(double x) { return rx_host_math(7, x); }
#define log rx_host_log
#define log2 rx_host_log2
#define ceil rx_host_ceil
#define exp rx_host_exp
#define exp2 rx_host_exp2
#define sqrt rx_host_sqrt
#define erfc rx_host_erfc
#define fabs rx_host_fabs
#endif
#endif
