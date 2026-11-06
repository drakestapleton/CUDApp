/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef GPU_RESIDENT_FIXTURE_H
#define GPU_RESIDENT_FIXTURE_H
#include <stdint.h>
/* Independent plain C sequential evaluator. f32 left-to-right; build with -ffp-contract=off. */
static void resident_reference(const uint32_t *d, unsigned n, float *x) {
    for(unsigned l=0;l<n;l++,d+=8) for(unsigned r=0;r<d[4];r++) {
        float v=0;
        if(d[0]==1) for(unsigned k=0;k<d[5];k++) v=v+x[d[1]+r*d[5]+k]*x[d[2]+k];
        if(d[0]==2) v=x[d[1]+r]+x[d[2]+r];
        if(d[0]==3) v=x[d[1]+r]*x[d[2]+r];
        if(d[0]==4) for(unsigned k=0;k<d[5];k++) v=v+x[d[1]+k];
        if(d[0]==5) v=x[d[1]+r];
        x[d[3]+r]=v;
    }
}
static const uint32_t resident_model[]={
    1,0,16,32,4,4,0,0,
    2,32,20,36,4,1,0,0,
    3,36,24,40,4,1,0,0,
    4,40,0,44,1,4,0,0,
    5,44,0,45,1,1,0,0
};
static void resident_input(float *x, unsigned witness) {
    for(unsigned i=0;i<64;i++) x[i]=0;
    for(unsigned i=0;i<28;i++) x[i]=(float)(i%4+1);
    if(witness) {
        x[0]=-1; x[1]=1.00000011920928955078125f; x[2]=0;x[3]=0;
        x[16]=1;x[17]=0.99999988079071044921875f;x[18]=0;x[19]=0;
    }
}
#endif
