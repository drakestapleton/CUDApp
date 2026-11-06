/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* HOST_ONLY link control: names alone must not select a C transport. No
 * definitions of these forbidden functions are supplied to the linker. */
#define ioctl cudapp_nvrm_forbidden_fixture_ioctl
#define mmap cudapp_nvrm_forbidden_fixture_mmap
#include "nvrm.c"
#undef ioctl
#undef mmap
#include <assert.h>
_Static_assert(NVRM_C_TRANSPORT == 0, "production macros selected the C transport");
int main(void) {
    int fd=nvrm_file_open("/dev/null");
    assert(fd>=0 && nvrm_file_close(fd)==0);
    puts("NVRM PRODUCTION BOUNDARY PASS (ioctl/mmap macro names cannot select C; Omega symbols link and run; HOST_ONLY)");
    return 0;
}
