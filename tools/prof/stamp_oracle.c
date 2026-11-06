/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Differential oracle for emit_stamp in src/omega_gpu_cmdlist.c.
 * Prints the six method words, one per line. It does not open a device. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t bw_mthd(uint32_t subc, uint32_t mthd, uint32_t count) {
    return (2u << 28) | (count << 16) | (subc << 13) | (mthd >> 2);
}

int main(int argc, char **argv) {
    unsigned long long va;
    unsigned long long slot;
    unsigned long long addr;
    char *end = NULL;
    if (argc != 3) {
        return 2;
    }
    va = strtoull(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0') {
        return 2;
    }
    slot = strtoull(argv[2], &end, 10);
    if (end == argv[2] || *end != '\0') {
        return 2;
    }
    addr = va + slot * 16ull;
    printf("%u\n%llu\n0\n%u\n%u\n0\n",
           bw_mthd(1u, 0x0158u, 5u),
           slot,
           (unsigned)addr,
           (unsigned)(addr >> 32));
    return 0;
}
