/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_MATHLIBS_HOST_H
#define OMEGA_GPU_MATHLIBS_HOST_H
#include <stdint.h>
#include <stdlib.h>
#include "cudapp_native.h"
/* Scalar helpers have no allocation and operate only on admitted integer data.
 * An unexpected bridge trap fails closed, exactly like a seeded native trap;
 * admission entries below instead translate traps to CODEGEN_FAIL. */
static inline uint64_t omega_mh_time(uint64_t a, uint64_t b) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 0, a, b, 0, 0, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_sub(uint64_t a, uint64_t b) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 1, a, b, 0, 0, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_mul(uint64_t a, uint64_t b) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 2, a, b, 0, 0, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_add(uint64_t a, uint64_t b) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 3, a, b, 0, 0, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_min(uint64_t a, uint64_t b) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 4, a, b, 0, 0, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_overlap(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 5, a, b, c, d, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_rng(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 6, a, b, c, d, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_rngval(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 7, a, b, c, d, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_seed(uint64_t a, uint64_t b, uint64_t c) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 8, a, b, c, 0, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_phil(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 9, a, b, c, d, e)) abort();
    return r;
}
static inline uint64_t omega_mh_lg(uint64_t a, uint64_t b, uint64_t c) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 10, a, b, c, 0, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_chunk(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 11, a, b, c, d, e)) abort();
    return r;
}
static inline uint64_t omega_mh_fill(uint64_t a, uint64_t b) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 12, a, b, 0, 0, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_poison(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 13, a, b, c, d, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_div(uint64_t a, uint64_t b) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 14, a, b, 0, 0, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_basic(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    uint64_t r=0;
    if (omega_native_mhval(&r, 15, a, b, c, d, 0)) abort();
    return r;
}
static inline uint64_t omega_mh_index(const uint64_t l[8], const uint64_t e[4], uint64_t rank, uint64_t idx) {
    uint64_t r=0;
    if (omega_native_mhidx(&r,l,8,e,4,rank,idx,0)) abort();
    return r;
}
static inline uint64_t omega_mh_eachcnt(const uint64_t e[4], uint64_t rank, uint64_t batch) {
    const uint64_t l[8]={0}; uint64_t r=0;
    if (omega_native_mhidx(&r,l,8,e,4,rank,batch,1)) abort();
    return r;
}
static inline uint64_t omega_mh_walk(const uint64_t p[8], uint64_t o[8]) {
    uint64_t r=3;
    return omega_native_mhwalk(&r,p,8,o,8)?3:r;
}
static inline uint64_t omega_mh_walkstep(uint64_t step, uint64_t batch, uint64_t cursor, uint64_t o[16]) {
    uint64_t r=3;
    return omega_native_mhstep(&r,step,batch,cursor,o,16)?3:r;
}
static inline uint64_t omega_mh_many(const uint64_t p[32], uint64_t o[8192]) {
    uint64_t r=3;
    return omega_native_mhmany(&r,p,32,o,8192)?3:r;
}
static inline uint64_t omega_mh_fft(uint64_t in, uint64_t out, uint64_t n, uint64_t batch, uint64_t inv, uint64_t o[8]) {
    uint64_t r=3;
    return omega_native_mhfft(&r,in,out,n,batch,inv,o,8)?3:r;
}
#endif
