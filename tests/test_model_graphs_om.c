/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host-only independent C oracle; no platform or device linked. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cudapp_native.h"
static uint64_t rng=0x429071234ULL;
static uint64_t rand64(void) { rng^=rng<<13; rng^=rng>>7; rng^=rng<<17; return rng; }
static void ref_wire(uint32_t out[32],const uint64_t in[16],uint32_t kind) {
    memset(out,0,128);
    unsigned n=kind==1?5:kind==2?2:kind==3?4:kind==4?3:kind==5?3:kind==6?5:7;
    for(unsigned i=0;i<n;i++) {out[2*i]=(uint32_t)in[i];out[2*i+1]=(uint32_t)(in[i]>>32);}
    if(kind==1){out[10]=(uint32_t)in[5];out[11]=(uint32_t)in[6];}
    if(kind==2)out[4]=(uint32_t)in[2];
    if(kind==5)out[6]=(uint32_t)in[3];
    if(kind==6)out[10]=(uint32_t)in[5];
    if(kind==7){out[14]=(uint32_t)in[7];out[15]=(uint32_t)in[8];}
}
int main(void) {
    uint64_t in[16],result,raw[32];uint32_t old[32],got[32];unsigned cases=0;
    for(unsigned k=1;k<=7;k++)for(unsigned j=0;j<10032;j++) {
        for(unsigned i=0;i<16;i++)in[i]=j<32?(j==0?0:UINT64_MAX>>(j-1)):rand64();
        ref_wire(old,in,k);memset(got,0xa5,sizeof got);result=0;
        int rc=omega_mg_wire(&result,in,16,raw,32,k);
        if(rc||result!=1){fprintf(stderr,"FAIL graph wire adapter kind=%u case=%u rc=%d result=%llu\n",k,j,rc,(unsigned long long)result);return 1;}
        for(unsigned i=0;i<32;i++)got[i]=(uint32_t)raw[i];
        if(memcmp(old,got,sizeof got)) {
            fprintf(stderr,"FAIL graph wire kind=%u case=%u\n",k,j);return 1;
        }
        got[j%32]^=1;
        if(!memcmp(old,got,sizeof got)){fputs("FAIL record flip accepted\n",stderr);return 1;}
        cases++;
    }
    printf("MODEL-GRAPHS wire PASS cases=%u record-flip=CAUGHT\n",cases);
    return 0;
}
