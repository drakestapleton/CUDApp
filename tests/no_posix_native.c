/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Verification-only transport. No device or driver dependency. */
#include <stdint.h>
#include "omega_native.h"
static uint64_t ins[8192], phys[288], meta[104], image[16777216], result[16];
static uint8_t code[16384], name[64], text[16777216];
static int exercise(void) {
    uint64_t bytes = 0;
    if (omega2_bw_enc_sm121(&bytes, ins, 8192, phys, 288, 0, 1, code, 16384) || bytes != 512) return 1;
    if (code[0] != 0x18 || code[1] != 0x79 || code[12] != 0 || code[13] != 0xc0 || code[14] != 0x0f) return 2;
    result[0] = bytes;
    if (omega2_bw_enc_sm121(&bytes, ins, 8192, phys, 288, 0, 0, code, 16384) || bytes) return 3;
    ins[0] = UINT64_MAX;
    if (omega2_bw_enc_sm121(&bytes, ins, 8192, phys, 288, 1, 0, code, 16384) || bytes) return 4;
    bytes = 99;
    if (omega2_bw_enc_sm121(&bytes, ins, 8191, phys, 288, 0, 0, code, 16384) != OMEGA_BAD_ARG || bytes != 99) return 5;
    meta[6] = 999;
    image[0] = 17;
    if (omega2_cubin_elf(&bytes, meta, 104, name, 64, text, 16777216, image, 16777216) || bytes != 1 || image[0] != 17) return 6;
    result[1] = bytes;
    /* Exact RNE/NaN vectors derived in evidence NO-POSIX README with Wolfram. */
    const uint64_t inputs[] = {0,2147483648,1065353216,1065385984,1065451520,2139095041,4294967295,2139095040};
    const uint64_t wanted[] = {0,32768,16256,16256,16258,32704,65535,32640};
    for (unsigned i = 0; i < sizeof(inputs)/sizeof(inputs[0]); ++i) {
        if (omega2_no_bf16(&result[2+i], inputs[i]) || result[2+i] != wanted[i]) return 7;
    }
#ifdef NO_POSIX_RED
    return 90;
#else
    return 0;
#endif
}
#ifdef NO_POSIX_BARE
#include <asm/unistd.h>
#define S1(x) #x
#define S(x) S1(x)
int no_posix_main(void) {
    int rc = exercise();
    if (rc) return rc;
    register long x0 __asm__("x0") = 1;
    register long x1 __asm__("x1") = (long)result;
    register long x2 __asm__("x2") = sizeof(result);
    register long x8 __asm__("x8") = __NR_write;
    __asm__ volatile("svc #0" : "+r"(x0) : "r"(x1), "r"(x2), "r"(x8) : "memory", "cc");
    return x0 == sizeof(result) ? 0 : 91;
}
__asm__(".text\n.global _start\n.type _start,%function\n_start:\nbl no_posix_main\nmov x8,#" S(__NR_exit) "\nsvc #0\nbrk #0\n");
#else
#include <stdio.h>
int main(void) {
    int rc = exercise();
    if (rc) return rc;
    return fwrite(result, 1, sizeof(result), stdout) == sizeof(result) ? 0 : 91;
}
#endif
