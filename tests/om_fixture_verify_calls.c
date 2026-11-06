/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen.h"
#include <stdio.h>
int main(void) {
    int (*f[])(void) = {omega_blackwell_verify_codegen_fixtures,omega_blackwell_verify_codegen_fixtures_fb1cut4,omega_blackwell_verify_codegen_fixtures_intops,omega_blackwell_verify_codegen_fixtures_fragmm,omega_blackwell_verify_codegen_fixtures_reconv,omega_blackwell_verify_codegen_fixtures_bfly_f2fp,omega_blackwell_verify_codegen_fixtures_atomf32};
    int bad=0;
    for(unsigned i=0;i<sizeof f/sizeof f[0];++i) { int rc=f[i](); printf("fixture %u rc %d\n",i,rc); if(rc) bad=1; }
    puts(bad ? "FAIL codegen fixture invocation" : "PASS seven codegen fixture entrypoints");
    return bad;
}
