/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Fixed-view ABI marshalling only; decisions and arithmetic live in dag_fmt.om. */
#ifndef DAGSIM_FORMAT_H
#define DAGSIM_FORMAT_H
#include "cudapp_native.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static inline uint64_t ds_fmt(unsigned mode, uint64_t a[16], char s[256]) {
    uint64_t result = 0;
    if (omega_dag_fmt(&result, mode, a, 16, s, 256)) abort();
    return result;
}
static inline uint64_t ds_bits(double d) {
    uint64_t u; memcpy(&u, &d, sizeof u); return u;
}
static inline double ds_unbits(uint64_t u) {
    double d; memcpy(&d, &u, sizeof d); return d;
}
static inline uint64_t ds_decide(unsigned mode, uint64_t a[16]) {
    char s[256]; return ds_fmt(mode, a, s);
}
#endif
