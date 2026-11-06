/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CUDAPP_MIGRATION_PENDING_C_API_POLICY 1
#define omega_host_shape oracle_shape
#include "omega_host_policy.h"
#undef omega_host_shape
#undef OMEGA_HOST_POLICY_H
#undef CUDAPP_MIGRATION_PENDING_C_API_POLICY
#define CUDAPP_MIGRATION_PENDING_C_API_POLICY 0
#include "omega_host_policy.h"
static uint64_t rng = 0x67687374657374ULL;
static uint32_t next(void) { rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17; return (uint32_t)rng; }
static void check(int ok) { if (!ok) { fputs("HOST POLICY differential FAIL\n", stderr); exit(1); } }
static int templ(const uint32_t *q0,const uint32_t *q1) {
    if(q0[12] || q1[12] || q1[15] || q1[16])return 0;
    if(q1[32] || (q1[33] & ~(0xau<<21)))return 0;
    if(q1[42] || (q1[43]&0x7ffffu))return 0;
    if(q1[44] || (q1[45]&0x7ffffu))return 0;
    if(q1[52] != (0x300u>>6) || (q1[53]&0x7ffffu))return 0;
    return !(q1[56] || (q1[57]&0x7ffffu) || q1[59]);
}
static int launch(const uint32_t *q,const uint64_t *s) {
    uint32_t gx=s[0],gy=s[1],bx=s[2],by=s[3],bz=s[4],shared=s[5],gpr=s[6];
    uint32_t wg=gpr<16?16:gpr, ws=shared?(shared+127u)/128u:8u;
    if(!gx||!gy||!bx||!by||!bz||bx>0xffffu||by>0xffffu||bz>0xffu||wg>0x1ffu||ws>0x7ffu)return 0;
    return q[34]==(bx|(by<<16)) && (q[35]&0xffu)==bz && ((q[35]>>8)&0x1ffu)==wg && (q[36]&0x7ffu)==ws && q[39]==gx && q[40]==gy && q[41]==1;
}
static void qcheck(uint32_t *a,uint32_t *b,uint64_t *s) {
    uint64_t r;
    check(omega_artifact_policy(&r,0,a,256,b,256,s,7)==OMEGA_OK);check(r==(uint64_t)templ(a,b));
    check(omega_artifact_policy(&r,1,b,256,b,256,s,7)==OMEGA_OK);check(r==(uint64_t)launch(b,s));
}
int main(void) {
    const uint32_t edge[]={0,1,2,3,63,64,65,127,128,129,255,256,257,511,512,1023,1024,1025,2047,2048,2049,8191,8192,8193,16383,16384,16385,262143,262144,262145,UINT32_MAX};
    for(unsigned mode=0;mode<8;mode++)for(size_t i=0;i<sizeof edge/sizeof *edge;i++)for(size_t j=0;j<sizeof edge/sizeof *edge;j++)for(size_t k=0;k<sizeof edge/sizeof *edge;k++)
        check(omega_host_shape(mode,edge[i],edge[j],edge[k])==oracle_shape(mode,edge[i],edge[j],edge[k]));
    for(unsigned i=0;i<100000;i++) { uint32_t a=next(),b=next(),c=next();for(unsigned mode=0;mode<8;mode++)check(omega_host_shape(mode,a,b,c)==oracle_shape(mode,a,b,c)); }
    uint32_t a[96]={0},b[96]={0};uint64_t s[7]={1,1,32,1,1,0,16};
    b[52]=0x300u>>6;b[34]=32|(1u<<16);b[35]=1|(16u<<8);b[36]=8;b[39]=1;b[40]=1;b[41]=1;
    qcheck(a,b,s);
    for(unsigned which=0;which<2;which++)for(unsigned word=0;word<96;word++)for(unsigned bit=0;bit<32;bit++){uint32_t *q=which?a:b;q[word]^=1u<<bit;qcheck(a,b,s);q[word]^=1u<<bit;}
    for(size_t i=0;i<sizeof edge/sizeof *edge;i++)for(unsigned j=0;j<7;j++){uint64_t old=s[j];s[j]=edge[i];qcheck(a,b,s);s[j]=old;}
    for(unsigned i=0;i<10000;i++){for(unsigned j=0;j<96;j++){a[j]=next();b[j]=next();}for(unsigned j=0;j<7;j++)s[j]=next();qcheck(a,b,s);}
    puts("HOST POLICY differential PASS");return 0;
}
