/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_ATTN_TABLES_H
#define OMEGA_ATTN_TABLES_H
#include "omega_attn_views.h"
#include "omega_api_plan.h"
#include "omega_gpu_hostmath.h"
static inline void av_ropegen(float *d,uint64_t pos,double theta,uint64_t dim,uint64_t width,size_t rows) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t r=0;r<rows;r++)for(uint64_t i=0;i<width/2;i++) {
        double s=0,c=1;
        if(i<dim/2){double inv=omega_hostmath_pow(theta,-2.0*(double)i/(double)dim);omega_hostmath_sincos((double)(pos+r)*inv,&s,&c);}
        d[r*width+i]=(float)c;d[r*width+width/2+i]=(float)s;
    }
#else
    uint64_t bits;memcpy(&bits,&theta,8);
    uint64_t q[16]={pos,bits,dim,width,rows},r=0;
    if(omega_av_rope(&r,d,rows*width,q,16)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline void av_gentab(const uint32_t *lo,const uint32_t *hi,uint32_t *tab,size_t n,uint64_t off,uint64_t stride) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++){tab[4*i]=lo[i];tab[4*i+1]=hi[i];tab[4*i+2]=(uint32_t)(off+i*stride);tab[4*i+3]=0;}
#else
    uint64_t q[16]={n,off,stride},r=0;
    if(omega_av_gtab(&r,lo,n,tab,4*n,q,16,0)!=OMEGA_OK || r ||
       omega_av_gtab(&r,hi,n,tab,4*n,q,16,1)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline void av_ranges(uint32_t *d,size_t n,const uint64_t q[16]) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++){uint64_t p[16]={q[0]+i,q[1],q[2],q[3],q[4]};(void)ap_plan(AP_AT,10,p);d[i]=(uint32_t)p[8];d[n+i]=(uint32_t)p[9];}
#else
    uint64_t r=0;if(omega_av_range(&r,q,16,d,2*n,n)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline uint32_t av_total(const uint32_t *e,size_t n,uint64_t bs) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    uint32_t r=0;for(size_t i=0;i<n;i++)r+=e[i]?(e[i]+bs-1)/bs:0;return r;
#else
    uint64_t r=0;if(omega_av_total(&r,e,n,n,bs)!=OMEGA_OK)__builtin_trap();return (uint32_t)r;
#endif
}
static inline uint32_t av_table(const uint32_t *ids,uint32_t *tab,size_t tw,int32_t *slots,uint32_t *touched,size_t nb,const uint64_t q[16],uint32_t g) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    (void)tw;(void)nb;
    for(size_t b=0;b<q[0];b++){uint32_t id=ids[b],v=g;
        if(q[1]==2)v=id;
        else if(q[1]){if(slots[id]<0){slots[id]=(int32_t)g;touched[g++]=id;}v=(uint32_t)slots[id];}
        else g++;
        tab[q[2]+b]=v;
    }
    tab[q[3]]=(uint32_t)q[4];return g;
#else
    (void)tw;uint64_t r=0;
    uint32_t *row=tab+q[2];
    av_memcpy(row,ids,q[0]*sizeof *ids);
    if(omega_av_table(&r,row,q[0],slots,nb,q,16,g)!=OMEGA_OK)__builtin_trap();
    uint32_t result=(uint32_t)r;
    if(q[1] && q[1]!=2)for(uint64_t start=0;;) {
        uint64_t count=q[0]-start;if(count>1000000)count=1000000;
        if(omega_av_touch(&r,start?ids+start:ids,count,slots,nb,touched,nb,g)!=OMEGA_OK)__builtin_trap();
        if(count==q[0]-start)break;
        start+=count;
    }
    av_fill32(tab+q[3],1,(uint32_t)q[4]);
    return result;
#endif
}
static inline int av_blocks(const uint32_t *ids,size_t n,const uint64_t q[16]) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++){uint64_t p[16]={ids[i],q[1],q[2],q[3],q[4],q[5]};if(ap_plan(AP_AT,16,p))return 1;}return 0;
#else
    uint64_t r=0;if(omega_av_blks(&r,ids,n,q,16,n)!=OMEGA_OK)__builtin_trap();return (int)r;
#endif
}
static inline int av_resfit(const uint32_t *ids,size_t n,uint64_t stride,uint64_t layer) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++)if(stride>UINT32_MAX || (uint64_t)ids[i]*stride+layer>UINT32_MAX)return 0;return 1;
#else
    uint64_t q[16]={stride,layer},r=0;if(omega_av_rfit(&r,ids,n,q,16,n)!=OMEGA_OK)__builtin_trap();return (int)r;
#endif
}
static inline void av_gather(const void *s,size_t sn,void *d,size_t dn,const uint64_t q[16]) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    (void)sn;(void)dn;
    for(size_t t=0;t<q[0];t++)for(size_t h=0;h<q[1];h++)memcpy((uint8_t*)d+(t*q[1]+h)*q[2],(const uint8_t*)s+(t*q[3]+h*q[4])*q[5],q[2]);
#else
    uint64_t r=0;if(omega_av_gath(&r,s,sn,d,dn,q,16)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline void av_stage(const void *s,size_t sn,void *d,size_t dn,const uint32_t *ids,const uint64_t q[16]) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    (void)sn;(void)dn;
    for(size_t i=0;i<q[0];i++)memcpy((uint8_t*)d+(q[4]+i)*q[3],(const uint8_t*)s+(uint64_t)ids[i]*q[1]+q[2],q[3]);
#else
    /* Three independent data views plus metadata exceed six unsized seed slots.
     * Dispatch pointer windows only; every byte/data access remains in Omega. */
    for(uint64_t start=0;;) {
        uint64_t count=q[0]-start;if(count>1000000)count=1000000;
        uint64_t wq[16]={count,q[1],q[2],q[3],q[4]+start},r=0;
        if(omega_av_stage(&r,s,sn,d,dn,start?ids+start:ids,count,wq,16)!=OMEGA_OK || r)__builtin_trap();
        if(count==q[0]-start)break;
        start+=count;
    }
#endif
}
static inline void av_reset(int32_t *slots,size_t nb,const uint32_t *ids,size_t n) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    (void)nb;for(size_t i=0;i<n;i++)slots[ids[i]]=-1;
#else
    uint64_t r=0;if(omega_av_reset(&r,slots,nb,ids,n,n)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline void av_headmap(uint32_t *d,uint64_t nqh,uint64_t nkv,uint64_t hb,uint64_t mutant) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(uint64_t i=0;i<nqh;i++){uint64_t p[16]={i,nqh,nkv,hb,mutant};(void)ap_plan(AP_AT,25,p);d[i]=(uint32_t)p[8];}
#else
    uint64_t q[16]={0,nqh,nkv,hb,mutant},r=0;if(omega_av_hmap(&r,d,nqh,q,16)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline void av_eff(uint32_t *d,const uint32_t *ctx,const uint32_t *nb,size_t n,uint64_t bs) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    for(size_t i=0;i<n;i++){uint64_t p[16]={ctx[i],nb[i],bs};(void)ap_plan(AP_AT,15,p);d[i]=(uint32_t)p[8];}
#else
    av_memcpy(d,ctx,n*sizeof *ctx);
    uint64_t r=0;if(omega_av_eff(&r,nb,n,d,n,bs)!=OMEGA_OK || r)__builtin_trap();
#endif
}
static inline uint32_t av_best(const uint32_t *e,size_t n,uint64_t hd) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    uint32_t r=0;for(size_t i=0;i<n;i++){uint64_t p[16]={e[i],hd};(void)ap_plan(AP_AT,3,p);if(p[8]>r)r=(uint32_t)p[8];}return r;
#else
    uint64_t r=0;if(omega_av_best(&r,e,n,n,hd)!=OMEGA_OK)__builtin_trap();return (uint32_t)r;
#endif
}
static inline uint32_t av_max(const uint32_t *s,size_t n) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    uint32_t r=1;for(size_t i=0;i<n;i++)if(s[i]>r)r=s[i];return r;
#else
    uint64_t r=0;if(omega_av_max(&r,s,n,n)!=OMEGA_OK)__builtin_trap();return (uint32_t)r;
#endif
}
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
static inline float av_exp2_oracle(float x) {
    if(x<=-127.0f)return 0.0f;if(x>127.0f)x=127.0f;
    int n=(int)(x+(x<0?-0.5f:0.5f));float f=x-(float)n,px=1.535336188319500e-4f;
    px=px*f+1.339887440266574e-3f;px=px*f+9.618437357674640e-3f;px=px*f+5.550332471162809e-2f;
    px=px*f+2.402264791363012e-1f;px=px*f+6.931472028550421e-1f;px=1.0f+f*px;
    if(n < -126){px*=0.5f;n=-126;}uint32_t b=(uint32_t)(n+127)<<23;float s;memcpy(&s,&b,4);return px*s;
}
#endif
static inline void av_fold(const float *p,float *out,size_t rows,uint32_t S,uint32_t HD) {
#if CUDAPP_MIGRATION_PENDING_ATTN_VIEWS
    size_t pw=HD+2;
    for(size_t r=0;r<rows;r++){float mx=-__builtin_inff();size_t base=r*S*pw;
        for(uint32_t s=0;s<S;s++)if(p[base+s*pw+HD]>mx)mx=p[base+s*pw+HD];
        if(!(mx>-__builtin_inff())){memset(out+r*HD,0,HD*4);continue;}
        float w[16],lsum=0;for(uint32_t s=0;s<S;s++){w[s]=av_exp2_oracle(p[base+s*pw+HD]-mx);lsum+=w[s]*p[base+s*pw+HD+1];}
        float inv=1.0f/lsum;for(uint32_t d=0;d<HD;d++){float a=0;for(uint32_t s=0;s<S;s++)a+=w[s]*p[base+s*pw+d];out[r*HD+d]=a*inv;}
    }
#else
    uint64_t q[16]={rows,S,HD},r=0;if(omega_av_fold(&r,p,rows*S*(HD+2),out,rows*HD,q,16)!=OMEGA_OK || r)__builtin_trap();
#endif
}
#endif
