/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_ATTN_VIEWS_H
#define OMEGA_ATTN_VIEWS_H
#include "cudapp_native.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#ifndef CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
#define CUDAPP_MIGRATION_PENDING_ATTN_VIEWS 0
#endif
/* Only native pointers/counts cross this boundary. All data walks are Omega.
 * The C oracle is default-OFF and exists solely for differential/coverage gates. */
static inline int av_args(uint32_t a[32],const uint64_t p[32],uint64_t na,uint64_t nw) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    if(na>16 || nw>32 || 2*na>nw)return 1;
    memset(a,0,32*sizeof *a);
    for(uint64_t i=0;i<na;i++){a[2*i]=(uint32_t)p[i];a[2*i+1]=(uint32_t)(p[i]>>32);}
    for(uint64_t i=0;i<nw-2*na;i++)a[2*na+i]=(uint32_t)p[na+i];return 0;
#else
    uint64_t r=1;return omega_av_pack(&r,p,32,a,32,na,nw)!=OMEGA_OK || r!=0;
#endif
}
static inline int av_layout(uint32_t a[32],const uint64_t p[32],uint64_t mask) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(uint64_t i=0;i<32;i++){a[i]=(uint32_t)p[i];if((mask>>i)&1){if(i==31)return 1;a[i+1]=(uint32_t)(p[i]>>32);i++;}}return 0;
#else
    uint64_t r=1;return omega_av_lay(&r,p,32,a,32,mask)!=OMEGA_OK || r!=0;
#endif
}
static inline void *av_memcpy(void *d,const void *s,size_t n) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    return memcpy(d,s,n);
#else
    if(d==s)return d;
    uint64_t r=0;
    if(omega_av_copy(&r,s,n,d,n,n)!=OMEGA_OK || r)__builtin_trap();
    return d;
#endif
}
static inline void *av_memset(void *d,int v,size_t n) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    return memset(d,v,n);
#else
    uint64_t r=0;if(omega_av_set(&r,d,n,n,(uint8_t)v)!=OMEGA_OK || r)__builtin_trap();return d;
#endif
}
static inline void av_fill32(uint32_t *d,size_t n,uint32_t v) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)d[i]=v;
#else
    uint64_t r=0;if(omega_av_fill(&r,d,n,n,v)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline void av_fillf32(float *d,size_t n,float v) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)d[i]=v;
#else
    uint64_t r=0;if(omega_av_fillf(&r,d,n,n,v)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline size_t av_count32(const uint32_t *s,size_t n,uint32_t v) {
    uint64_t r=0;
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)r+=s[i]==v;
#else
    if(omega_av_count(&r,s,n,n,v)!=OMEGA_OK)__builtin_trap();
#endif
    return (size_t)r;
}
static inline size_t av_countf32(const float *s,size_t n,float v) {
    uint64_t r=0;
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)r+=s[i]==v;
#else
    if(omega_av_cntf(&r,s,n,n,v)!=OMEGA_OK)__builtin_trap();
#endif
    return (size_t)r;
}
static inline void av_bfexpand(float *d,const uint16_t *s,size_t n) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++){uint32_t b=(uint32_t)s[i]<<16;memcpy(d+i,&b,4);}
#else
    uint64_t r=0;if(omega_av_bfexp(&r,s,n,d,n,n)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline size_t av_bffinite(const uint16_t *s,size_t n) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)if((s[i]&0x7f80u)==0x7f80u)return i+1;return 0;
#else
    uint64_t r=0;if(omega_av_bffin(&r,s,n,n)!=OMEGA_OK)__builtin_trap();return (size_t)r;
#endif
}
static inline uint32_t av_any32(const uint32_t *s,size_t n) {
    uint64_t r=0;
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)r|=s[i];
#else
    if(omega_av_any(&r,s,n,n)!=OMEGA_OK)__builtin_trap();
#endif
    return (uint32_t)r;
}
static inline size_t av_compact32(uint32_t *d,const int32_t *s,size_t n) {
    uint64_t r=0;
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)if(s[i]>=0)d[r++]=(uint32_t)s[i];
#else
    if(omega_av_comp(&r,s,n,d,n,n)!=OMEGA_OK)__builtin_trap();
#endif
    return (size_t)r;
}
static inline size_t av_routerpoison(const float *w,const uint32_t *ids,size_t n,float pv,uint32_t pi) {
    uint64_t r=0;
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)r+=w[i]==pv || ids[i]==pi;
#else
    uint32_t bits;memcpy(&bits,&pv,4);uint64_t poison=bits|((uint64_t)pi<<32);
    if(omega_av_rpois(&r,w,n,ids,n,n,poison)!=OMEGA_OK)__builtin_trap();
#endif
    return (size_t)r;
}
static inline size_t av_zero64(const uint64_t *s,size_t n) {
    uint64_t r=0;
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)r+=s[i]==0;
#else
    if(omega_av_z64(&r,s,n,n)!=OMEGA_OK)__builtin_trap();
#endif
    return (size_t)r;
}
static inline size_t av_badids(const uint32_t *s,size_t n,uint32_t limit) {
    uint64_t r=0;
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)r+=s[i]>=limit;
#else
    if(omega_av_bids(&r,s,n,n,limit)!=OMEGA_OK)__builtin_trap();
#endif
    return (size_t)r;
}
static inline void av_rowtab(uint32_t *d,size_t n,uint64_t base) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++){d[i]=0;d[n+i]=(uint32_t)(base+i+1);}
#else
    uint64_t r=0;if(omega_av_rows(&r,d,2*n,n,base)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline void av_context(uint32_t *d,const int32_t *s,size_t n) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)d[i]=s[i]>0?(uint32_t)s[i]:0;
#else
    uint64_t r=0;if(omega_av_ctx(&r,s,n,d,n,n)!=OMEGA_OK || r)__builtin_trap();
#endif
}
#endif
