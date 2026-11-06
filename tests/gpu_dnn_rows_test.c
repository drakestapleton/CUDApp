/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_dnn_rows_api.h"
#include "bw_warp_sim_k.h"
#include "gpu_dnn_rows_ref.h"
#include <stdio.h>
static int failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"rows failure line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
static void putptr(uint32_t *p,uintptr_t a) {p[0]=(uint32_t)a;p[1]=(uint32_t)((uint64_t)a>>32);}
static void *input(size_t n,unsigned dt,int constant) {
    void *p=malloc(n*(dt?2u:4u)); if(!p) abort();
    for(size_t i=0;i<n;++i) {
        float f=constant?0.25f:(float)((int)((i*17+i/7)%33)-16)*0.03125f;
        if(dt==0) ((float*)p)[i]=f;
        else if(dt==1) {uint32_t b;memcpy(&b,&f,4);((uint16_t*)p)[i]=(uint16_t)(b>>16);}
        else {_Float16 h=(_Float16)f;memcpy((uint16_t*)p+i,&h,2);}
    }
    return p;
}
static int run(OmegaGpuDnnRowsShape s,int constant,int chip,int wrong) {
    size_t n=(size_t)s.rows*s.length;void *in=input(n,s.dtype,constant);
    float *out=malloc(n*4);double *ref=malloc(n*sizeof *ref);
    BlackwellIRProgram *p=malloc(sizeof *p);uint8_t *code=malloc(65536);OmegaBlackwellKernel k;
    if(!out||!ref||!p||!code) abort();memset(out,0xa5,n*4);
    int rc=omega_gpu_dnn_rows_codegen(&s,p,code,65536,&k);
    if(rc) fprintf(stderr,"codegen rc=%d op=%u n=%u dt=%u\n",rc,s.op,s.length,s.dtype);
    CHECK(rc==0);
    if(!rc&&chip) rc=omega_gpu_dnn_rows(&s,in,out);
    if(!rc&&!chip) {
        WsSim sim={0};sim.p=p;sim.threads=32;sim.order=constant;sim.warp_sched=constant;
        uint32_t *cb=&sim.cbank[896/4];putptr(cb,(uintptr_t)in);putptr(cb+2,(uintptr_t)out);memcpy(cb+4,&s.epsilon,4);
        sim.ranges[0]=(WsRange){(uintptr_t)in,(uintptr_t)in+n*(s.dtype?2:4)};
        sim.ranges[1]=(WsRange){(uintptr_t)out,(uintptr_t)out+n*4};sim.nranges=2;
        rc=ws_run(&sim,s.rows,1);if(rc)fprintf(stderr,"rows sim %s\n",sim.msg);CHECK(rc==0);
    }
    dr_reference(&s,in,ref);if(wrong) ref[0]+=1.0;
    int equal=!rc&&dr_equal(out,ref,n);if(!wrong)CHECK(equal);
    free(in);free(out);free(ref);free(p);free(code);return equal;
}
static void refuse(OmegaGpuDnnRowsShape s) {
    float dummy=0;BlackwellIRProgram *p=malloc(sizeof *p);uint8_t code[4096];OmegaBlackwellKernel k;
    CHECK(omega_gpu_dnn_rows_codegen(&s,p,code,sizeof code,&k)==OMEGA_GPU_DNN_ROWS_REFUSED);
    CHECK(omega_gpu_dnn_rows(&s,&dummy,&dummy)==OMEGA_GPU_DNN_ROWS_REFUSED);free(p);
}
int main(int argc,char **argv) {
    int quick=argc==2&&!strcmp(argv[1],"--quick"),chip=argc==2&&!strcmp(argv[1],"--chip");
    if(!quick&&!chip&&!(argc==2&&!strcmp(argv[1],"--sim")))return 2;
    OmegaGpuDnnRowsShape s={2,33,0,0,0.03125f};
    CHECK(run(s,0,chip,0));CHECK(!run(s,0,chip,1));
    for(unsigned op=0;op<3;++op) {s.op=op;s.dtype=2;CHECK(run(s,0,chip,0));}
    s=(OmegaGpuDnnRowsShape){0,1,0,0,1};refuse(s);s.rows=1;s.length=0;refuse(s);
    s.length=65536;refuse(s);s.length=1;s.rows=65536;refuse(s);s.rows=1;s.dtype=3;refuse(s);
    s.dtype=0;s.op=3;refuse(s);s.op=1;s.epsilon=0;refuse(s);s.epsilon=-1;refuse(s);
    s.epsilon=NAN;refuse(s);s.epsilon=INFINITY;refuse(s);s.epsilon=0x1p-149f;refuse(s);
    s.epsilon=1;s.rows=65535;s.length=65535;refuse(s);
    CHECK(omega_gpu_dnn_rows(NULL,NULL,NULL)==OMEGA_GPU_DNN_ROWS_BAD_ARGS);
    if(!quick) {
        const unsigned lens[]={1,2,7,31,32,33,64,65,513,65535};
        const float eps[]={0.0009765625f,0.03125f,1.0f};
        for(unsigned op=0;op<3;++op) for(unsigned dt=0;dt<3;++dt)
        for(unsigned l=0;l<sizeof lens/sizeof *lens;++l) for(unsigned e=0;e<3;++e)
        for(int c=0;c<2;++c) {s=(OmegaGpuDnnRowsShape){2,lens[l],dt,op,eps[e]};CHECK(run(s,c,chip,0));}
    }
    if(failures){printf("dnn-rows: FAIL (%d checks)\n",failures);return 1;}
    printf("dnn-rows %s: PASS (all lengths/types/epsilon, constant+varied rows, tail ranges, refusals, wrong-result control)\n",chip?"chip":"host+warp-sim");return 0;
}
