/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_ATTN_HOST_POLICY_H
#define OMEGA_ATTN_HOST_POLICY_H
#include "cudapp_native.h"
#include <stddef.h>
#include <stdint.h>
#ifndef CUDAPP_MIGRATION_PENDING_C_ATTENTION_HOST_POLICY
#define CUDAPP_MIGRATION_PENDING_C_ATTENTION_HOST_POLICY 0
#endif
#ifndef CUDAPP_MIGRATION_PENDING_C_SCAN_CHECK_REST
#define CUDAPP_MIGRATION_PENDING_C_SCAN_CHECK_REST 0
#endif
#ifndef CUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS
#define CUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS 0
#endif
/* Layout transport: rows, heads, dim, per, maxb, has_ids; output slots match attn_host.om. */
static inline uint64_t ah_policy(uint64_t q[32],uint32_t op) {
#if CUDAPP_MIGRATION_PENDING_C_ATTENTION_HOST_POLICY
    if(op==0){uint32_t t=((uint32_t)q[2]+31u)&~31u;q[8]=t;q[9]=(uint32_t)(16u*t);return 0;}
    if(op==1){q[8]=q[1]*q[2];size_t idw=q[5]?q[3]*q[4]:1u;q[9]=q[3]*4u+q[1]+idw;q[10]=0;return 0;}
    if(op==3){q[10]=q[22];return 0;}
    if(q[10]>=q[0])return 0;
    uint64_t nr=q[0]-q[10];if(nr>q[3])nr=q[3];
    q[11]=nr;q[12]=(uint32_t)(nr*4);q[13]=(uint32_t)(nr*4+q[1]);q[14]=q[13]+(q[5]?nr*q[4]:1u);
    q[15]=q[10]*q[8];q[16]=nr*q[8]*4;q[17]=q[14]*4;q[18]=q[10]*q[4];q[19]=nr*q[4]*4;
    q[20]=q[1]*4;q[21]=(uint32_t)(q[12]*4);q[22]=q[10]+q[3];return 1;
#else
    uint64_t r=0;if(omega_ah_plan(&r,q,32,op)!=OMEGA_OK)__builtin_trap();return r;
#endif
}
static inline size_t ah_check(uint32_t op,const uint32_t *w,const uint32_t *x,size_t n) {
#if CUDAPP_MIGRATION_PENDING_C_SCAN_CHECK_REST
    size_t bad=0;
    if(op==0){for(size_t i=0;i<n;i++)bad+=w[i]==0x7fa00badu;}
    else if(op==1){if(n){bad=w[0]!=x[0];for(size_t i=1;i<n;i++)bad+=(uint32_t)(w[i]-w[i-1])!=x[i];}}
    else{for(size_t i=1;i<n;i++)bad+=w[i]<w[i-1];}
    return bad;
#else
    uint64_t r=0;if(omega_sc_check(&r,w,n,x,n,op)!=OMEGA_OK)__builtin_trap();return (size_t)r;
#endif
}
static inline uint64_t ah_clock(uint64_t sec,uint64_t nsec) {
#if CUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS
    return sec*1000000000ULL+nsec;
#else
    uint64_t r=0;if(omega_clock_ns(&r,sec,nsec)!=OMEGA_OK)__builtin_trap();return r;
#endif
}
#endif
