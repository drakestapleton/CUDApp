/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Verification-only corruption at the strong Omega ABI. */
#include <stdint.h>
#include <stdlib.h>
uint32_t __real_omega_so_oracles(uint64_t *,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t);
uint32_t __wrap_omega_so_oracles(uint64_t *out,uint64_t op,uint64_t a,uint64_t b,uint64_t c,uint64_t d) {
    uint32_t rc=__real_omega_so_oracles(out,op,a,b,c,d);
    const char *red=getenv("SO_RED_OP");
    if(!rc && red && op==strtoul(red,NULL,0))*out=!*out;
    return rc;
}
