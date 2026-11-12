/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Test-only oracle: no product links this file. The system header supplies all numbers/layouts.
 * Allowed boundary: AGENTS.md tests and PROVENANCE.md class A, following tests/share/abi-oracle.c.
 * Native execution prints requests.tsv. Cross compilation keeps the exact same values in .kfdlayout,
 * so objcopy can check x86-64 layouts without executing a cross binary or opening any device. */
#include <linux/kfd_ioctl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define R(name, type, n) {(uint64_t)AMDKFD_IOC_##name, sizeof(struct type), n},
#define F(type, field) {offsetof(struct type, field), sizeof(((struct type *)0)->field), 1},
#define A(name, type, n) {0, sizeof(struct type), n},
#define C(name, val) {(uint64_t)(val), 0, 0},
#define U(name) {0, sizeof(__u32), 1},
#define UF(name) {0, sizeof(__u32), 1},
struct row { uint64_t a, b, c; };
__attribute__((section(".kfdlayout"), used))
const struct row kfd_layout[] = {
#include "layout-fields.inc"
};
#undef R
#undef F
#undef A
#undef C
#undef U
#undef UF
#define R(name, type, n) #name,
#define F(type, field) #field,
#define A(name, type, n) #name,
#define C(name, val) #name,
#define U(name) #name,
#define UF(name) #name,
static const char *const labels[] = {
#include "layout-fields.inc"
};
int main(void) {
    for (size_t i = 0; i < sizeof(kfd_layout)/sizeof(kfd_layout[0]); ++i)
        printf("%llu\t%llu\t%llu\t# %s\n",
               (unsigned long long)kfd_layout[i].a,
               (unsigned long long)kfd_layout[i].b,
               (unsigned long long)kfd_layout[i].c, labels[i]);
    return ferror(stdout) ? 74 : 0;
}
