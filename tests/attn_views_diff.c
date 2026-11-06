/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static void oracle(uint32_t *a,const uint64_t *p,size_t na,size_t nw){
    memset(a,0,32*sizeof *a);
    for(size_t i=0;i<na;i++){a[2*i]=(uint32_t)p[i];a[2*i+1]=(uint32_t)(p[i]>>32);}
    for(size_t i=0;i<nw-2*na;i++)a[2*na+i]=(uint32_t)p[na+i];
}
int main(void){
    uint64_t p[32],r=99;uint32_t got[32],want[32];
    for(size_t na=0;na<=16;na++)for(size_t nw=2*na;nw<=32;nw++){
        for(size_t i=0;i<32;i++)p[i]=UINT64_C(0xdeadbeef12345678)^((uint64_t)i<<40);
        oracle(want,p,na,nw);
        if(omega_av_pack(&r,p,32,got,32,na,nw)||r||memcmp(got,want,sizeof got))return 1;
    }
    if(omega_av_pack(&r,p,32,got,32,17,32)||r!=1)return 2;
    if(omega_av_pack(&r,p,32,got,32,2,3)||r!=1)return 3;
    if(omega_av_pack(&r,p,32,got,32,0,33)||r!=1)return 4;
    if(omega_av_pack(&r,p,31,got,32,0,0)==0)return 5;
    if(omega_av_pack(&r,p,32,(uint32_t*)p,32,0,0)==0)return 6;
    oracle(want,p,4,16);want[1]^=1;
    if(omega_av_pack(&r,p,32,got,32,4,16)||!memcmp(got,want,sizeof got))return 7;
    puts("PASS attention launch differential; red check rejected; extent and alias refused");
    return 0;
}
