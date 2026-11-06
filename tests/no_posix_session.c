/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Verification-only transport: identical bytes from hosted and freestanding ABI. */
#include <stdint.h>
#include <stdio.h>
#include "omega_native.h"
static uint64_t idx[512], gen[512];
static int record(uint32_t status, uint64_t r, const uint64_t *o, unsigned n) {
    if (fwrite(&status, sizeof status, 1, stdout) != 1 || fwrite(&r, sizeof r, 1, stdout) != 1) return 1;
    return n && fwrite(o, sizeof *o, n, stdout) != n;
}
int main(void) {
    /* admit: uint32 truncation; alignment 8/page4096; gpr64; hook8;
     * sign-bit marker subtraction; uint64 epoch exhaustion and address carry. */
    const uint64_t v[] = {0,1,2,3,4,7,8,9,31,32,33,63,64,65,255,256,257,
        351,352,353,895,896,897,4095,4096,4097,8191,8192,8193,
        2147483647,2147483648,2147483649,4294967294,4294967295,4294967296,
        UINT64_MAX-1,UINT64_MAX};
    for (uint64_t op = 0; op < 16; ++op) {
        for (unsigned slot = 0; slot < 5; ++slot) for (unsigned k = 0; k < sizeof v / sizeof *v; ++k) {
            uint64_t a[16] = {1,1,1,1,1}, o[32] = {0}, r = 0;
            a[slot] = v[k];
            uint32_t s = omega2_decide(&r,op,a,16,o,4);
            if (record(s,r,o,4)) return 1;
        }
        for (unsigned mask = 0; mask < 32; ++mask) {
            uint64_t a[16] = {0}, o[4] = {0}, r = 0;
            for (unsigned j = 0; j < 5; ++j) a[j] = (mask >> j) & 1;
            uint32_t s = omega2_decide(&r,op,a,16,o,4);
            if (record(s,r,o,4)) return 2;
        }
    }
    /* Packet op dispatch, uint32 low/high carries, inline field truncation. */
    for (uint64_t op = 0; op < 9; ++op) for (unsigned slot = 0; slot < 3; ++slot)
        for (unsigned k = 0; k < sizeof v / sizeof *v; ++k) {
            uint64_t a[16] = {0}, o[32] = {0}, r = 0;
            a[slot] = v[k];
            uint32_t s = omega2_emit(&r,op,a,16,o,32);
            if (record(s,r,o,32)) return 3;
        }
    /* book: matching/stale generations, epoch mismatch, successor wrap. */
    for (uint64_t op = 0; op < 4; ++op) for (unsigned k = 0; k < sizeof v / sizeof *v; ++k)
        for (unsigned j = 0; j < sizeof v / sizeof *v; ++j) {
            uint64_t a[16] = {v[k],v[j],v[k],v[j]}, o[4] = {0}, r = 0;
            uint32_t s = omega2_book(&r,op,a,16,o,4);
            if (record(s,r,o,4)) return 4;
        }
    /* scan: empty, first/second/last slot, duplicate index wrong generation. */
    for (unsigned p = 0; p < 512; ++p) {
        uint64_t r = 0;
        idx[p] = UINT32_MAX; gen[p] = 1;
        uint32_t s = omega2_scan(&r,idx,512,gen,512,UINT32_MAX,1);
        if (record(s,r,0,0) || s || r != 1) return 5;
        s = omega2_scan(&r,idx,512,gen,512,UINT32_MAX,2);
        if (record(s,r,0,0) || s || r != 0) return 6;
        idx[p] = gen[p] = 0;
    }
    /* session xq_admit uses streams op6 with all 3 queue-open combinations. */
    for (unsigned mask = 0; mask < 8; ++mask) for (unsigned k = 0; k < sizeof v / sizeof *v; ++k) {
        uint64_t a[16] = {v[k],mask&1,(mask>>1)&1,(mask>>2)&1}, o[32] = {0}, r = 0;
        uint32_t s = omega2_streams_words(&r,6,a,16,o,32);
        if (record(s,r,o,32)) return 7;
    }
    /* Negative ABI controls must refuse without touching the caller's result. */
    uint64_t a[16] = {0}, o[32] = {0}, r = 99;
    if (omega2_decide(&r,0,a,15,o,4) != OMEGA_BAD_ARG || r != 99) return 8;
    if (omega2_emit(&r,0,a,16,o,31) != OMEGA_BAD_ARG || r != 99) return 9;
    if (omega2_book(&r,0,a,16,o,3) != OMEGA_BAD_ARG || r != 99) return 10;
    if (omega2_scan(&r,idx,511,gen,512,0,1) != OMEGA_BAD_ARG || r != 99) return 11;
#ifdef NO_POSIX_SESSION_RED
    return 90;
#else
    return 0;
#endif
}
