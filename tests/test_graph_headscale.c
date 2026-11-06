/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static uint32_t bits(float x) { uint32_t u; memcpy(&u,&x,4); return u; }
static float value(uint32_t u) { float x; memcpy(&x,&u,4); return x; }
int main(void) {
    float x[4096], ref[4096];
    uint64_t cases=0;
    /* Every exponent, sign, and the rounding boundaries of each mantissa. */
    const uint32_t mant[]={0,1,2,0x3fffff,0x400000,0x400001,0x7ffffe,0x7fffff};
    for (unsigned sign=0;sign<2;sign++) for(unsigned e=0;e<256;e++) {
        for(unsigned i=0;i<8;i++) { x[i]=value((sign<<31)|(e<<23)|mant[i]); ref[i]=x[i]*0x1p-64f; }
        uint64_t r=99;
        if(omega_ghscale(&r,x,8)||r) return 1;
        for(unsigned i=0;i<8;i++) {
            uint32_t a=bits(x[i]),b=bits(ref[i]);
            if ((b&0x7fffffff)>0x7f800000) { if((a&0x7fffffff)<=0x7f800000) return 2; }
            else if(a!=b) return 3;
            cases++;
        }
    }
    for(unsigned i=0;i<4096;i++) { x[i]=value(0x3f800000+i); ref[i]=x[i]*0x1p-64f; }
    uint64_t r=99;
    if(omega_ghscale(&r,x,4096)||r||memcmp(x,ref,sizeof x)) return 4;
    if(omega_ghscale(&r,x,0)||r) return 5;
    printf("PASS graph head scaling float classes cases=%llu vector=4096 empty=0\n",(unsigned long long)cases);
    return 0;
}
