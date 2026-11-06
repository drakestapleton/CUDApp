/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define main mg_timeline_reference_main
#include "test_omega_gpu_decode_graph_timeline.c"
#undef main
#include "model_graphs_fixture.h"
#include "omega_gpu_q36_graph.h"
#include "omega_gpu_q38_graph.h"
#include "omega_gpu_fp8gemm_api.h"
#include "omega_gpu_cmdlist_native.h"
#include "omega_gpu_native_runtime.h"
static OmegaGpuCmdList *list;
static uint64_t metadata[8192][33];static unsigned nm;
static uint64_t fb(float f){uint32_t u;memcpy(&u,&f,4);return u;}
static void route(OmegaGpuRouterDevOut *out,uint64_t base){out->xout_va=base+256;out->ids_va=base+64;out->wts_va=base+128;out->tab_gu_va=base+16384;out->tab_dn_va=base+24576;}
void mg_fixture_setup(void *cl){list=cl;nm=0;}
void *mg_fixture_list(void){return list;}
int mg_record(uint64_t kind,const uint64_t *words,size_t n){
    if(n>32)return -1;
    if(kind>=200){if(nm==8192)return -1;memset(metadata[nm],0,sizeof metadata[nm]);metadata[nm][0]=kind;memcpy(metadata[nm]+1,words,n*8);nm++;return 0;}
    uint32_t args[64]={0};for(size_t i=0;i<n;i++){args[i*2]=(uint32_t)words[i];args[i*2+1]=words[i]>>32;}
    OmegaGpuLaunch launch={0};launch.code_va=UINT64_C(0x800000000)+kind*0x1000;launch.gpr_count=64;
    launch.threads_x=256;launch.threads_y=1;launch.grid_x=1;launch.grid_y=1;launch.num_elements=1;
    launch.args=args;launch.n_args=n*2;launch.timeout_ms=20000;
    return omega_gpu_cmdlist_add(list,&launch)<0?-1:0;
}
int __wrap_omega_gpu_bf16_gemv_record(OmegaGpuCmdList *cl,const OmegaGpuBf16Tensor *t,uint64_t x,uint64_t y){(void)cl;const uint64_t w[]={ (uintptr_t)t,x,y };return mg_record(1,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_bf16_gemv_rows_record(OmegaGpuCmdList *cl,const OmegaGpuBf16Tensor *t,uint32_t rows,uint64_t x,uint64_t y){(void)cl;const uint64_t w[]={ (uintptr_t)t,rows,x,y };return mg_record(2,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_bf16_gemv_xbatch_record(OmegaGpuCmdList *cl,const OmegaGpuBf16Tensor *t,uint32_t rows,uint64_t x,uint64_t y,uint32_t stride){(void)cl;const uint64_t w[]={ (uintptr_t)t,rows,x,y,stride };return mg_record(3,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_bf16_gemv_mask_record(OmegaGpuCmdList *cl,const OmegaGpuBf16Tensor *t,uint64_t x,uint64_t y,uint64_t hi,uint64_t tau){(void)cl;const uint64_t w[]={ (uintptr_t)t,x,y,hi,tau };return mg_record(4,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_fp8_gemv_grouped_record(OmegaGpuCmdList *cl,const OmegaGpuFp8Tensor *t,uint32_t groups,uint64_t tab,uint64_t x,uint32_t xr,uint64_t y){(void)cl;const uint64_t w[]={ (uintptr_t)t,groups,tab,x,xr,y };return mg_record(5,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_fp8gemm_record(OmegaGpuCmdList *cl,const OmegaGpuFp8Tensor *t,uint32_t rows,uint32_t segs,uint64_t seg,uint64_t x,uint64_t y,uint64_t xm,uint64_t ym,uint32_t mt,uint32_t budget,OmegaGpuFp8GemmInfo *info){(void)cl;(void)info;const uint64_t w[]={ (uintptr_t)t,rows,segs,seg,x,y,xm,ym,mt,budget };return mg_record(6,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_gdn_record(OmegaGpuCmdList *cl,OmegaGpuGdn *g,uint32_t l,uint64_t q,uint64_t z,uint64_t ab,float e1,float e2,float s1,float s2,uint64_t *out){(void)cl;*out=UINT64_C(0x600000000)+(uintptr_t)g*0x1000;const uint64_t w[]={ (uintptr_t)g,l,q,z,ab,fb(e1),fb(e2),fb(s1),fb(s2) };return mg_record(7,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_gdn_multi_record(OmegaGpuCmdList *cl,OmegaGpuGdn *g,uint32_t l,uint32_t n,uint64_t q,uint64_t z,uint32_t xs,uint64_t ab,uint32_t as,uint64_t out,uint32_t os,float e1,float e2,float s1,float s2){(void)cl;const uint64_t w[]={ (uintptr_t)g,l,n,q,z,xs,ab,as,out,os,fb(e1),fb(e2),fb(s1),fb(s2) };return mg_record(8,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_decode_attn_record(OmegaGpuCmdList *cl,OmegaGpuDecodeAttnCache *g,uint32_t l,uint32_t p,uint64_t q,float e,float s,uint64_t out){(void)cl;const uint64_t w[]={ (uintptr_t)g,l,p,q,fb(e),fb(s),out };return mg_record(9,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_decode_attn256_record(OmegaGpuCmdList *cl,OmegaGpuDecodeAttn256Cache *g,uint32_t l,uint32_t p,uint64_t q,float e,float s,uint64_t out){(void)cl;const uint64_t w[]={ (uintptr_t)g,l,p,q,fb(e),fb(s),out };return mg_record(10,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_decode_attn256_multi_record(OmegaGpuCmdList *cl,OmegaGpuDecodeAttn256Cache *g,uint32_t l,uint32_t p,uint32_t n,uint64_t q,uint32_t xs,float e,float s,uint64_t out,uint32_t os){(void)cl;const uint64_t w[]={ (uintptr_t)g,l,p,n,q,xs,fb(e),fb(s),out,os };return mg_record(11,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_router_record(OmegaGpuCmdList *cl,OmegaGpuRouter *g,uint32_t l,uint64_t x,float e,int norm,float s,OmegaGpuRouterDevOut *out){(void)cl;route(out,UINT64_C(0x700000000));const uint64_t w[]={ (uintptr_t)g,l,x,fb(e),norm,fb(s) };return mg_record(12,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_router_record_rows(OmegaGpuCmdList *cl,OmegaGpuRouter *g,uint32_t l,uint64_t x,uint32_t xs,float e,int norm,float s,uint64_t lb,uint32_t lbs,uint32_t rows,OmegaGpuRouterDevOut *out){(void)cl;route(out,lb);const uint64_t w[]={ (uintptr_t)g,l,x,xs,fb(e),norm,fb(s),lb,lbs,rows };return mg_record(13,w,sizeof w/sizeof w[0]);}
int __wrap_omega_gpu_fp8gemm_record_native(OmegaGpuCmdList *cl,const OmegaGpuFp8Tensor *t,const OmegaGpuNativeArtifact *a,const OmegaGpuNativeLease *lease,uint32_t rows,uint32_t segs,uint64_t tab,uint64_t x,uint64_t y,uint64_t xm,uint64_t ym){(void)cl;(void)lease;const uint64_t w[]={(uintptr_t)t,(uintptr_t)a,rows,segs,tab,x,y,xm,ym};return mg_record(14,w,sizeof w/sizeof w[0]);}
int old_mg_q36_run(const MgCase*);int new_mg_q36_run(const MgCase*);
int old_mg_q38_run(const MgCase*);int new_mg_q38_run(const MgCase*);
int old_mg_decode_run(const MgCase*);int new_mg_decode_run(const MgCase*);
int __wrap_omega_gpu_fp8_tensor_entry(const OmegaGpuFp8Tensor *t,uint32_t e,uint64_t *w,uint64_t *s){*w=UINT64_C(0xa00000000)+(uintptr_t)t*0x10000+e*0x1000;*s=*w+0x800;return 0;}
typedef struct {uint8_t bank[1024],qmd[1024];uint64_t code,wait;uint32_t args,independent;} Snap;
static Snap snaps[OMEGA_GPU_CMDLIST_MAX_LAUNCHES];
static uint64_t oldmeta[8192][33];
static uint64_t rng=UINT64_C(0x185293abcdf00123);
static uint64_t rand64(void){rng^=rng<<13;rng^=rng>>7;rng^=rng<<17;return rng;}
static int one(MgCase c,int (*oldfn)(const MgCase*),int (*newfn)(const MgCase*),unsigned index){
    omega_gpu_cmdlist_reset(list);nm=0;int a=oldfn(&c);unsigned count=omega_gpu_cmdlist_count(list),mn=nm;
    memcpy(oldmeta,metadata,mn*sizeof metadata[0]);
    for(unsigned i=0;i<count;i++){OmegaGpuCmdListEntry e;if(omega_gpu_cmdlist_entry(list,i,&e))return 1;memcpy(snaps[i].bank,e.bank,1024);memcpy(snaps[i].qmd,e.qmd_slot,1024);snaps[i].code=e.code_va;snaps[i].wait=e.pending_wait;snaps[i].args=e.n_args;snaps[i].independent=e.independent;}
    omega_gpu_cmdlist_reset(list);nm=0;int b=newfn(&c);
    if(a!=b||a||count!=omega_gpu_cmdlist_count(list)||mn!=nm||memcmp(oldmeta,metadata,mn*sizeof metadata[0])){fprintf(stderr,"graph case %u mode %u flags %x mutant %u attn %u layer %u/%u heads %u state rc %d/%d count %u/%u meta %u/%u\n",index,c.mode,c.flags,c.mutant,c.attn,c.layer,c.layers,c.heads,a,b,count,omega_gpu_cmdlist_count(list),mn,nm);return 1;}
    for(unsigned i=0;i<count;i++){
        OmegaGpuCmdListEntry e;omega_gpu_cmdlist_entry(list,i,&e);Snap *s=&snaps[i];
        if(memcmp(s->bank,e.bank,1024)||memcmp(s->qmd,e.qmd_slot,1024)||s->code!=e.code_va||s->wait!=e.pending_wait||s->args!=e.n_args||s->independent!=e.independent){
            fprintf(stderr,"graph case %u mode %u flags %x mutant %u attn %u layer %u/%u heads %u record %u code=%llx/%llx args=%u/%u independent=%u/%u\n",index,c.mode,c.flags,c.mutant,c.attn,c.layer,c.layers,c.heads,i,(unsigned long long)s->code,(unsigned long long)e.code_va,s->args,e.n_args,s->independent,e.independent);
            for(unsigned j=0;j<256;j++){
                uint32_t x,y;memcpy(&x,s->bank+j*4,4);memcpy(&y,e.bank+j*4,4);if(x!=y)fprintf(stderr," bank[%u] %08x/%08x\n",j,x,y);
                memcpy(&x,s->qmd+j*4,4);memcpy(&y,e.qmd_slot+j*4,4);if(x!=y)fprintf(stderr," qmd[%u] %08x/%08x\n",j,x,y);
            }
            return 1;
        }
        s->bank[0]^=1;if(!memcmp(s->bank,e.bank,1024))return 1;s->bank[0]^=1;
    }
    return 0;
}
static int submission(const char *name,MgCase c,int (*oldfn)(const MgCase*),int (*newfn)(const MgCase*)){
    if(one(c,oldfn,newfn,10032))return 1;
    unsigned counts[2]={0};
    int (*record[2])(const MgCase*)={oldfn,newfn};
    for(unsigned i=0;i<2;i++){
        omega_gpu_cmdlist_reset(list);nm=0;
        if(record[i](&c))return 1;
        unsigned before=fk.submits;OmegaGpuCmdListStats st;
        if(omega_gpu_cmdlist_submit(list,NULL,&st)){fprintf(stderr,"%s submission failed: %s\n",name,omega_gpu_session_last_error());return 1;}
        counts[i]=fk.submits-before;
    }
    printf("MODEL-GRAPHS submissions model=%s attention=%u generated-words=1 old=%u new=%u fake-platform=1\n",name,c.attn,counts[0],counts[1]);
    return counts[0]!=1||counts[1]!=1;
}
int main(void){
    if(install()||omega_gpu_cmdlist_create(4096,&list)){fprintf(stderr,"fake platform/list setup failed\n");return 1;}
    for(unsigned i=0;i<10032;i++){
        MgCase c={.seed=UINT64_C(0x900000000)+(rand64()&0xfffff00),.pos=(uint32_t)(rand64()%262144),.layers=1+(uint32_t)(rand64()%64),.heads=1+(uint32_t)(rand64()%8),.attn=(uint32_t)(rand64()&1),.flags=(uint32_t)rand64()&2047,.mutant=(uint32_t)(rand64()%7)};
        c.layer=(uint32_t)(rand64()%c.layers);
        if(i<32){c.pos=i?((1u<<i)-1u):0;c.layer=(i&1)?c.layers-1:0;c.heads=(i&2)?8:1;}
        unsigned flags=c.flags;
        for(c.mode=0;c.mode<10;c.mode++){if(c.mode==8)continue;c.flags=flags;if(c.mode==5)c.flags&=~12u;if(one(c,old_mg_q36_run,new_mg_q36_run,i))return 1;}
        c.flags=flags;
        for(c.mode=0;c.mode<2;c.mode++){if(one(c,old_mg_q38_run,new_mg_q38_run,i))return 1;if(one(c,old_mg_decode_run,new_mg_decode_run,i))return 1;}
        for(c.mode=3;c.mode<5;c.mode++){if(one(c,old_mg_decode_run,new_mg_decode_run,i))return 1;}
    }
    for(unsigned attn=0;attn<2;attn++){
        MgCase c={.seed=UINT64_C(0x900000000),.pos=65535,.layers=64,.heads=1,.attn=attn,.mode=8};
        if(submission("q36",c,old_mg_q36_run,new_mg_q36_run))return 1;
        c.mode=2;
        if(submission("q38",c,old_mg_q38_run,new_mg_q38_run)||submission("decode",c,old_mg_decode_run,new_mg_decode_run))return 1;
    }
    omega_gpu_cmdlist_free(list);omega_gpu_session_close();
    puts("MODEL-GRAPHS command-list PASS cases=150480 record-flip=CAUGHT exact-head=covered standalone-glue=covered nested-ABI=deterministic-host-adapters");return 0;
}
