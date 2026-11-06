/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_dnn_activations_api.h"
#include "cudapp_native.h"
#include "bw_warp_sim_k.h"
#include "gpu_dnn_activations_ref.h"
#include <float.h>
#include <stdio.h>
static int failures,opens;
int __wrap_omega_gpu_session_open(void) { ++opens;return -1; }
#define CHECK(v) do { if (!(v)) { fprintf(stderr,"dnn-activations line %d: %s\n",__LINE__,#v);++failures; } } while(0)
static void put64(uint32_t *p,uintptr_t v) { p[0]=(uint32_t)v;p[1]=(uint32_t)((uint64_t)v>>32); }
static void *inputs(size_t n,unsigned dtype) {
    static const float values[]={0.0f,-0.0f,0.000001f,-0.000001f,0.125f,-0.125f,0.5f,-0.5f,1,-1,2,-2,3,-3,5,-5,8,-8,10,-10,20,-20,80,-80,100,-100,65504,-65504};
    void *p=malloc(n*(dtype?2:4));if (!p) abort();
    for(size_t i=0;i<n;++i) {
        float x=values[i%(sizeof values/sizeof *values)];
        if (i%113==111 && dtype!=2) x=FLT_MAX;
        if (i%113==112 && dtype!=2) x=-FLT_MAX;
        if (dtype==0) ((float*)p)[i]=x;
        else if (dtype==1) { uint32_t b;memcpy(&b,&x,4);((uint16_t*)p)[i]=(uint16_t)(b>>16); }
        else { _Float16 h=(_Float16)x;memcpy((uint16_t*)p+i,&h,2); }
    }
    return p;
}
static int run_case(uint64_t n,unsigned op,unsigned dtype,int wrong) {
    OmegaGpuDnnActivationsShape shape={n,op,dtype};
    void *in=inputs(n,dtype);float *out=malloc((n+1)*4);BlackwellIRProgram *p=malloc(sizeof *p);
    uint8_t *code=malloc(OMEGA_BW_CODE_CAP_MAX);OmegaBlackwellKernel k;
    if (!out||!p||!code) abort();
    for(size_t i=0;i<=n;++i) out[i]=12345.0f;
    int rc=omega_gpu_dnn_activations_codegen(&shape,p,code,OMEGA_BW_CODE_CAP_MAX,&k);
    CHECK(rc==0);
    if (!rc) {
        WsSim sim={0};sim.p=p;sim.threads=32;sim.order=op&1;sim.warp_sched=dtype&1;
        uint32_t *cb=&sim.cbank[0x380/4];put64(cb,(uintptr_t)in);put64(cb+2,(uintptr_t)out);cb[4]=(uint32_t)n;
        sim.ranges[0]=(WsRange){(uintptr_t)in,(uintptr_t)in+n*(dtype?2:4)};
        sim.ranges[1]=(WsRange){(uintptr_t)out,(uintptr_t)out+n*4};sim.nranges=2;
        rc=ws_run(&sim,(uint32_t)((n+31)/32),1);
        if(rc) fprintf(stderr,"activation simulator: %s\n",sim.msg);
        CHECK(rc==0);
    }
    int equal=!rc;
    for(size_t i=0;i<n&&!rc;++i) {
        double x=da_input(in,i,dtype),want=da_reference(x,op);
        if(wrong) want+=0.25;
        if(!da_equal(out[i],want)) { equal=0;if(!wrong) {fprintf(stderr,"activation n=%llu op=%u type=%u i=%zu x=%.9g got=%.9g want=%.17g\n",(unsigned long long)n,op,dtype,i,x,out[i],want);break;} }
        if(x==0.0 && signbit(x) && !wrong) CHECK(signbit(out[i]));
    }
    CHECK(out[n]==12345.0f);
    if(!wrong) CHECK(equal);
    free(in);free(out);free(p);free(code);return equal;
}
static void refusals(void) {
    OmegaGpuDnnActivationsShape s={0,0,0};float f=0;
    CHECK(omega_gpu_dnn_activations(&s,&f,&f)==OMEGA_GPU_DNN_ACTIVATIONS_REFUSED);
    s.elements=4294967296ULL;
    CHECK(omega_gpu_dnn_activations(&s,&f,&f)==OMEGA_GPU_DNN_ACTIVATIONS_REFUSED);
    s.elements=1;s.operation=4;
    CHECK(omega_gpu_dnn_activations(&s,&f,&f)==OMEGA_GPU_DNN_ACTIVATIONS_REFUSED);
    s.operation=0;s.dtype=3;
    CHECK(omega_gpu_dnn_activations(&s,&f,&f)==OMEGA_GPU_DNN_ACTIVATIONS_REFUSED);
    CHECK(opens==0);
    CHECK(omega_gpu_dnn_activations(NULL,&f,&f)==OMEGA_GPU_DNN_ACTIVATIONS_BAD_ARGS);
    uint64_t result=99,plan[8]={0};
    CHECK(omega_da_plan(&result,65,3,2,plan,8)==0 && result==0);
    CHECK(plan[0]==130 && plan[1]==260 && plan[2]==3 && plan[3]==32);
    CHECK(omega_da_plan(&result,4294967295ULL,3,0,plan,8)==0 && result==0);
    CHECK(plan[0]==17179869180ULL && plan[2]==134217728ULL);
}
int main(int argc,char **argv) {
    if(argc!=2 || (strcmp(argv[1],"--sim")&&strcmp(argv[1],"--quick"))) return 2;
    int quick=!strcmp(argv[1],"--quick");refusals();
    CHECK(!run_case(65,1,0,1));
    static const uint64_t counts[]={1,2,7,31,32,33,64,65,513,65535};
    for(unsigned op=0;op<4;++op) for(unsigned dtype=0;dtype<3;++dtype)
        for(unsigned j=0;j<(quick?8u:10u);++j) CHECK(run_case(counts[j],op,dtype,0));
    if(failures) {printf("dnn-activations host+warp-sim: FAIL (%d checks)\n",failures);return 1;}
    puts("dnn-activations host+warp-sim: PASS (4 operators, 3 types, tails, independent double references, refusal-before-open, wrong-result control)");return 0;
}
