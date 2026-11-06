/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifndef CM_ROUTE_IMPL
#define CM_ROUTE_IMPL "cudapp_om_cm_route.c"
#endif
#include CM_ROUTE_IMPL
static int one(uint64_t n, int red) {
    uint64_t d[16] = {n}, r = 0;
    if (omega_cm_route(&r, 0, d, 16) || r != 1 || d[1] != n / 64 ||
        d[2] != n % 64 || d[3] != (n / 64) * 64 + (unsigned)red) return 1;
    return 0;
}
int main(int argc, char **argv) {
    (void)argv;
    const uint64_t cases[] = {0,1,62,63,64,65,126,127,128,129,UINT32_MAX-1,
        UINT32_MAX,(uint64_t)UINT32_MAX+1,UINT64_MAX-1,UINT64_MAX};
    for (unsigned i=0;i<sizeof cases/sizeof cases[0];++i)
        if (one(cases[i],argc>1)) { fputs("FAIL cm route block plan\n",stderr); return 1; }
    for (unsigned field=0;field<6;++field) for (unsigned i=0;i<sizeof cases/sizeof cases[0];++i) {
        uint64_t d[16]={1,2,3,4,5,6}, expected=0, result=0;
        d[field]=cases[i];
        for (unsigned j=0;j<6;++j) expected+=d[j];
        if (omega_cm_route(&result,1,d,16) || result!=expected) {
            fputs("FAIL cm route layout sum\n",stderr); return 1;
        }
    }
    for (unsigned i=0;i<sizeof cases/sizeof cases[0];++i)
        for (unsigned j=0;j<sizeof cases/sizeof cases[0];++j) {
            uint64_t d[16]={cases[i],cases[j]}, result;
            if (omega_cm_route(&result,2,d,16) || result!=(cases[i]<cases[j]) ||
                omega_cm_route(&result,3,d,16) || result!=cases[i]+cases[j] ||
                omega_cm_route(&result,4,d,16) || result!=cases[i]-cases[j]) {
                fputs("FAIL cm route scalar admission/offset\n",stderr); return 1;
            }
        }
    uint64_t d[16]={0}, result=99;
    if (omega_cm_route(&result,UINT64_MAX,d,16) || result!=0) return 1;
    if (omega_cm_route(&result,0,d,15)!=32) return 1;
    puts("PASS cm route source-partition differential (block, tail, layout, ABI refusal)");
    return 0;
}
