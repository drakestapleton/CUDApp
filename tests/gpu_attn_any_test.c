/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_attn_any_api.h"
#include "cudapp_native.h"
#include "bw_warp_sim_k.h"
#include "gpu_attn_any_ref.h"
#include <stdio.h>
static int failures;
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"attn-any failure line %d: %s\n",__LINE__,#x); ++failures; } } while (0)
static void put64(uint32_t *p, uintptr_t x) { p[0]=(uint32_t)x; p[1]=(uint32_t)((uint64_t)x>>32); }
static void *input(size_t n, unsigned dtype, unsigned salt) {
    void *p=malloc(n*(dtype ? 2u:4u)); if (!p) abort();
    for (size_t i=0;i<n;++i) {
        /* Exact dyadic inputs, differing heads and rows, bounded by 1/8. */
        float f=(float)((int)((i*17+salt*13)%9)-4)*0.03125f;
        if (dtype) { uint32_t u; memcpy(&u,&f,4); ((uint16_t*)p)[i]=(uint16_t)(u>>16); }
        else ((float*)p)[i]=f;
    }
    return p;
}
static int run_shape(OmegaGpuAttnAnyShape s, int chip, int wrong_mask) {
    size_t nq=(size_t)s.seq_q*s.q_heads*s.head_dim, nk=(size_t)s.seq_kv*s.kv_heads*s.head_dim;
    void *q=input(nq,s.dtype,1), *k=input(nk,s.dtype,3), *v=input(nk,s.dtype,7);
    float *o=malloc(nq*sizeof *o); double *ref=malloc(nq*sizeof *ref);
    BlackwellIRProgram *p=malloc(sizeof *p); uint8_t *code=malloc(OMEGA_BW_CODE_CAP_MAX);
    if (!o || !ref || !p || !code) abort();
    memset(o,0xa5,nq*sizeof *o);
    OmegaBlackwellKernel kernel;
    int rc=omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&kernel);
    if (rc) fprintf(stderr,"attn-any codegen rc=%d dim=%u type=%u\n",rc,s.head_dim,s.dtype);
    CHECK(rc==OMEGA_GPU_ATTN_ANY_OK);
    if (!rc && chip) rc=omega_gpu_attn_any(&s,q,k,v,o);
    if (!rc && !chip) {
        for (uint32_t h=0;h<s.q_heads;++h) {
            uint64_t shape[8]={s.seq_q,s.seq_kv,s.head_dim,s.q_heads,s.kv_heads,s.causal,s.dtype,0}, plan[16]={0}, result=0;
            uint32_t bits; memcpy(&bits,&s.scale,4); shape[7]=bits;
            CHECK(omega_aa_plan(&result,shape,8,h,plan,16)==0 && result==0);
            CHECK(plan[4]==(uint64_t)h*s.seq_q*s.head_dim*(s.dtype?2u:4u));
            CHECK(plan[5]==(uint64_t)(h/(s.q_heads/s.kv_heads))*s.seq_kv*s.head_dim*(s.dtype?2u:4u));
            WsSim sim={0}; sim.p=p; sim.threads=32; sim.order=h&1; sim.warp_sched=h&1;
            uint32_t *cb=&sim.cbank[0x380/4];
            put64(cb,(uintptr_t)q+plan[4]); put64(cb+2,(uintptr_t)k+plan[5]);
            put64(cb+4,(uintptr_t)v+plan[5]); put64(cb+6,(uintptr_t)o+plan[6]);
            cb[8]=s.seq_kv; cb[9]=bits;
            sim.ranges[0]=(WsRange){(uintptr_t)q,(uintptr_t)q+nq*(s.dtype?2u:4u)};
            sim.ranges[1]=(WsRange){(uintptr_t)k,(uintptr_t)k+nk*(s.dtype?2u:4u)};
            sim.ranges[2]=(WsRange){(uintptr_t)v,(uintptr_t)v+nk*(s.dtype?2u:4u)};
            sim.ranges[3]=(WsRange){(uintptr_t)o,(uintptr_t)o+nq*4}; sim.nranges=4;
            rc=ws_run(&sim,s.seq_q,1);
            if (rc) fprintf(stderr,"attn-any simulator: %s\n",sim.msg);
            CHECK(rc==0); if (rc) break;
        }
    }
    aa_reference(&s,q,k,v,ref,wrong_mask);
    int equal=!rc && aa_equal(o,ref,nq);
    if (!wrong_mask) CHECK(equal);
    free(q);free(k);free(v);free(o);free(ref);free(p);free(code);
    return equal;
}
static void refusals(void) {
    OmegaGpuAttnAnyShape s={1,1,257,4,1,0,0,0.125f};
    BlackwellIRProgram *p=malloc(sizeof *p); uint8_t *code=malloc(OMEGA_BW_CODE_CAP_MAX); OmegaBlackwellKernel k;
    CHECK(omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&k)==OMEGA_GPU_ATTN_ANY_REFUSED);
    float dummy=0; /* Admission must refuse before any session/device access. */
    CHECK(omega_gpu_attn_any(&s,&dummy,&dummy,&dummy,&dummy)==OMEGA_GPU_ATTN_ANY_REFUSED);
    s.head_dim=80;s.q_heads=3;s.kv_heads=2;
    CHECK(omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&k)==OMEGA_GPU_ATTN_ANY_REFUSED);
    s.q_heads=4;s.kv_heads=1;s.seq_kv=0;
    CHECK(omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&k)==OMEGA_GPU_ATTN_ANY_REFUSED);
    s.seq_kv=1;s.dtype=2;
    CHECK(omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&k)==OMEGA_GPU_ATTN_ANY_REFUSED);
    s.dtype=0;s.causal=2;
    CHECK(omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&k)==OMEGA_GPU_ATTN_ANY_REFUSED);
    s.causal=0;s.scale=INFINITY;
    CHECK(omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&k)==OMEGA_GPU_ATTN_ANY_REFUSED);
    s.scale=NAN;
    CHECK(omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&k)==OMEGA_GPU_ATTN_ANY_REFUSED);
    s.scale=0.125f;s.seq_q=65536;
    CHECK(omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&k)==OMEGA_GPU_ATTN_ANY_REFUSED);
    s.seq_q=65535;s.head_dim=256;s.q_heads=65535;
    CHECK(omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&k)==OMEGA_GPU_ATTN_ANY_REFUSED);
    s.seq_q=1;s.q_heads=0;
    CHECK(omega_gpu_attn_any_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&k)==OMEGA_GPU_ATTN_ANY_REFUSED);
    CHECK(omega_gpu_attn_any(NULL,NULL,NULL,NULL,NULL)==OMEGA_GPU_ATTN_ANY_BAD_ARGS);
    free(p);free(code);
}
int main(int argc,char **argv) {
    int chip=argc==2 && !strcmp(argv[1],"--chip");
    int quick=argc==2 && !strcmp(argv[1],"--quick");
    if (!chip && !quick && !(argc==2 && !strcmp(argv[1],"--sim"))) return 2;
    refusals();
    OmegaGpuAttnAnyShape control={7,65,80,4,1,1,0,0.125f};
    CHECK(run_shape(control,chip,0));
    CHECK(!run_shape(control,chip,1)); /* The independent wrong-mask reference must disagree. */
    OmegaGpuAttnAnyShape grouped={2,7,96,8,2,0,1,0.125f};
    CHECK(run_shape(grouped,chip,0));
    grouped.scale=0; CHECK(run_shape(grouped,chip,0)); /* Uniform weights, including bf16 input. */
    if (!quick) {
        static const uint32_t seq[]={1,2,7,64,65,513}, dim[]={64,80,96,128,256}, groups[]={1,4,8};
        for (unsigned i=0;i<30;++i) {
            OmegaGpuAttnAnyShape s={seq[i%6],seq[(i/6+i+1)%6],dim[i%5],2*groups[i%3],2,(i/3)%2,i%2,0.125f};
            CHECK(run_shape(s,chip,0));
        }
        static const uint32_t tails[]={1,3,31,33,255};
        for (unsigned i=0;i<5;++i) {
            OmegaGpuAttnAnyShape s={2,7,tails[i],1,1,i%2,i%2,-0.25f};
            CHECK(run_shape(s,chip,0));
        }
        OmegaGpuAttnAnyShape square={513,513,64,1,1,1,0,0.125f}; CHECK(run_shape(square,chip,0));
    }
    if (failures) { printf("attn-any %s: FAIL (%d checks)\n",chip?"chip":"host+warp-sim",failures);return 1; }
    printf("attn-any %s: PASS (reference, shape sweep, refusals, wrong-mask control)\n",chip?"chip":"host+warp-sim");return 0;
}
