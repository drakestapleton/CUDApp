/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define _POSIX_C_SOURCE 200809L
#include "omega_attn_tables.h"
#include <stdlib.h>
#include <time.h>
#ifndef AV_BENCH
#error AV_BENCH must name the timing entry
#endif
uint64_t AV_BENCH(unsigned op,size_t n,unsigned rounds){
    unsigned char *s=malloc(n*4+2048),*d=malloc(n*4+4096);
    if(!s||!d)abort();
    for(size_t i=0;i<n*4+2048;i++)s[i]=(unsigned char)i;
    uint64_t p[32]={0},q[16]={4,2,8},g[16]={8,0,0};uint32_t args[32],lo[8],hi[8],tab[32];
    float partial[80],fold[32];
    for(unsigned i=0;i<32;i++)p[i]=UINT64_C(0x1234567812340000)+i;
    for(unsigned i=0;i<8;i++){lo[i]=i;hi[i]=i+16;}
    for(unsigned i=0;i<80;i++)partial[i]=(float)(i%7);
    for(unsigned row=0;row<4;row++)for(unsigned split=0;split<2;split++){
        partial[(row*2+split)*10+8]=-(float)split;
        partial[(row*2+split)*10+9]=1.0f;
    }
    struct timespec start,end;clock_gettime(CLOCK_MONOTONIC_RAW,&start);
    for(unsigned rep=0;rep<rounds;rep++){
        switch(op){
        case 0:av_args(args,p,9,25);break;
        case 1:av_memcpy(d,s,n);break;
        case 2:av_bfexpand((float*)d,(const uint16_t*)s,n);break;
        case 3:av_gentab(lo,hi,tab,8,g[1],g[2]);break;
        case 4:av_ropegen((float*)d,129,10000.0,128,128,4);break;
        case 5:av_fold(partial,fold,q[0],q[1],q[2]);break;
        default:abort();
        }
        __asm__ volatile(""::"r"(d),"r"(args),"r"(tab),"r"(fold):"memory");
    }
    clock_gettime(CLOCK_MONOTONIC_RAW,&end);
    free(s);free(d);
    return (uint64_t)(end.tv_sec-start.tv_sec)*UINT64_C(1000000000)+(uint64_t)end.tv_nsec-(uint64_t)start.tv_nsec;
}
