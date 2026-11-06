/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_conv2d_api.h"
#include "omega_gpu_gemm.h"
#include "omega_gpu_session.h"
#include "bw_warp_sim_k.h"
#include "gpu_conv2d_ref.h"
#include <stdio.h>
static int failed,checks,opened,allocations,open_success,shell_contract;
#define CHECK(x) do {checks++;if(!(x)){failed++;fprintf(stderr,"conv2d FAIL line %d: %s\n",__LINE__,#x);}}while(0)
OmegaGpuSession *__wrap_omega_gpu_session_open(void) {static OmegaGpuSession dummy;if(!shell_contract)abort();opened++;return open_success?&dummy:NULL;}
int __wrap_omega_gpu_session_alloc(size_t n,OmegaGpuMem *m) {(void)n;(void)m;if(!shell_contract)abort();allocations++;return -1;}
void __wrap_omega_gpu_session_lock(void) {if(!shell_contract)abort();}
void __wrap_omega_gpu_session_unlock(void) {if(!shell_contract)abort();}
void __wrap_omega_gpu_session_free(OmegaGpuMem *m) {(void)m;if(!shell_contract)abort();}
static void ptr(uint32_t *p,uintptr_t x){p[0]=(uint32_t)x;p[1]=(uint32_t)((uint64_t)x>>32);}
static void *input(size_t n,unsigned t,unsigned salt) {
 void *p=calloc(n,t?2:4);if(!p)abort();
 for(size_t i=0;i<n;i++) {
  int z=(int)((i*17+salt*11)%29)-14;
  float f=z*0.0625f;
  if(!t)((float*)p)[i]=f;
  else if(t==1){uint32_t u;memcpy(&u,&f,4);((uint16_t*)p)[i]=(uint16_t)(u>>16);}
  else {unsigned a=(unsigned)(z<0?-z:z),e=0,m=0;if(a){unsigned k=0;while((a>>(k+1))!=0)k++;e=k+11;m=(a-(1u<<k))<<(10-k);}((uint16_t*)p)[i]=(uint16_t)((z<0?32768:0)|(e<<10)|m);}
 } return p;
}
static int run_conv(OmegaGpuConv2dShape s,int wrong) {
 uint64_t plan[32];int rc=omega_gpu_conv2d_plan(&s,0,plan);CHECK(!rc);if(rc)return 0;
 void *a=input(plan[2]/(s.dtype?2:4),s.dtype,1),*b=input(plan[3]/(s.dtype?2:4),s.dtype,7);
 size_t count=plan[4]/4;float *out=malloc(plan[4]);double *ref=malloc(count*8),*mag=malloc(count*8);
 BlackwellIRProgram *p=malloc(sizeof *p);uint8_t *code=malloc(OMEGA_BW_CODE_CAP_MAX);OmegaBlackwellKernel kernel;
 memset(out,0xa5,plan[4]);rc=omega_gpu_conv2d_codegen(&s,p,code,OMEGA_BW_CODE_CAP_MAX,&kernel);
 CHECK(!rc);if(rc)fprintf(stderr,"conv codegen rc %d\n",rc);
 for(uint64_t i=0;!rc&&i<plan[5];i++) {
  CHECK(!omega_gpu_conv2d_plan(&s,i,plan));
  WsSim sim={0};sim.p=p;sim.threads=(unsigned)plan[7];sim.order=i&1;sim.warp_sched=i&1;
  uint32_t *args=sim.cbank+0x380/4;ptr(args,(uintptr_t)a+plan[8]);ptr(args+2,(uintptr_t)b+plan[9]);ptr(args+4,(uintptr_t)out+plan[10]);args[6]=plan[11];
  sim.ranges[0]=(WsRange){(uintptr_t)a,(uintptr_t)a+plan[2]};sim.ranges[1]=(WsRange){(uintptr_t)b,(uintptr_t)b+plan[3]};sim.ranges[2]=(WsRange){(uintptr_t)out,(uintptr_t)out+plan[4]};sim.nranges=3;
  rc=ws_run(&sim,plan[6],1);if(rc)fprintf(stderr,"conv sim %s pc%d layout%u dtype%u\n",sim.msg,sim.err_pc,s.layout,s.dtype);
 } CHECK(!rc);
 cv_reference(&s,a,b,ref,mag,wrong);
 int equal=!rc;double k=(double)(s.in_channels/s.groups)*s.kernel_h*s.kernel_w;
 double gamma=(k/0x1p24)/(1-k/0x1p24);
 for(size_t i=0;i<count&&equal;i++)if(!isfinite(out[i])||fabs((double)out[i]-ref[i])>gamma*mag[i]+0x1p-126)equal=0;
 if(!wrong)CHECK(equal);
 free(a);free(b);free(out);free(ref);free(mag);free(p);free(code);return equal;
}
static int dense_launch(void *ctx,OmegaGemmKind kind,const uint32_t *args,uint32_t gx,uint32_t gy,uint32_t nt) {
 WsSim *sim=ctx;(void)kind;memcpy(sim->cbank+0x380/4,args,OMEGA_GPU_GEMM_ARGS_WORDS*4);sim->threads=nt;
 int rc=ws_run(sim,gx,gy);if(rc)fprintf(stderr,"dense sim: %s pc%d\n",sim->msg,sim->err_pc);return rc;
}
static void dense(void) {
 const unsigned shapes[][3]={{1,1,1},{2,3,9},{3,129,7},{5,131,17},{7,33,27}};uint64_t digest=1469598103934665603ull;
 for(unsigned v=0;v<5;v++) {
  unsigned M=shapes[v][0],N=shapes[v][1],K=shapes[v][2];
  float *a=input(M*K,0,v+1),*b=input(K*N,0,v+7),*out=calloc(M*N,4);
  OmegaGemmCfg cfg;omega_gpu_gemm_default_cfg(OMEGA_GPU_GEMM_F32_FFMA,&cfg);
  BlackwellIRProgram *p=malloc(sizeof *p);CHECK(!omega_gpu_gemm_codegen(&cfg,OMEGA_GEMM_K_MAIN,p));
  WsSim sim={0};sim.p=p;sim.ranges[0]=(WsRange){(uintptr_t)a,(uintptr_t)(a+M*K)};sim.ranges[1]=(WsRange){(uintptr_t)b,(uintptr_t)(b+K*N)};sim.ranges[2]=(WsRange){(uintptr_t)out,(uintptr_t)(out+M*N)};sim.nranges=3;
  int rc=omega_gpu_gemm_drive(&cfg,0,M,N,K,(uintptr_t)a,(uintptr_t)b,0,0,(uintptr_t)out,dense_launch,&sim);CHECK(!rc);
  for(unsigned m=0;m<M;m++)for(unsigned n=0;n<N;n++) {double ex=0,mag=0;for(unsigned k=0;k<K;k++){double z=(double)a[m*K+k]*b[k*N+n];ex+=z;mag+=fabs(z);}double gamma=(K/0x1p24)/(1-K/0x1p24);CHECK(fabs(out[m*N+n]-ex)<=gamma*mag+0x1p-126);uint32_t u;memcpy(&u,out+m*N+n,4);digest=(digest^u)*1099511628211ull;}
  free(a);free(b);free(out);free(p);
 }
 printf("dense F32 digest %016llx\n",(unsigned long long)digest);
}
static void refusals(void) {
 OmegaGpuConv2dShape base={1,3,5,7,5,3,3,1,1,1,1,1,1,1,1,1,0,0};
 float f=0;uint64_t p[32];
 for(unsigned i=0;i<10;i++) {OmegaGpuConv2dShape s=base;switch(i){case 0:s.batch=0;break;case 1:s.groups=2;break;case 2:s.stride_h=0;break;case 3:s.dilation_w=0;break;case 4:s.layout=2;break;case 5:s.dtype=3;break;case 6:s.height=65536;break;case 7:s.kernel_h=13;s.pad_top=0;s.pad_bottom=0;break;case 8:s.batch=65535;s.height=65535;break;case 9:s.out_channels=0;break;}CHECK(omega_gpu_conv2d_plan(&s,0,p)==OMEGA_GPU_CONV2D_REFUSED);CHECK(omega_gpu_conv2d(&s,&f,&f,&f)==OMEGA_GPU_CONV2D_REFUSED);}
 CHECK(opened==0);CHECK(omega_gpu_conv2d(NULL,NULL,NULL,NULL)==OMEGA_GPU_CONV2D_BAD_ARGS);
 if(!shell_contract)return;
 CHECK(omega_gpu_conv2d(&base,&f,&f,&f)==OMEGA_GPU_CONV2D_CHIP);
 CHECK(opened==1);
 CHECK(allocations==0);
 open_success=1;
 CHECK(omega_gpu_conv2d(&base,&f,&f,&f)==OMEGA_GPU_CONV2D_CHIP);
 CHECK(opened==2 && allocations==1);
 open_success=0;
}
int main(int argc,char **argv) {
 if(argc!=2)return 2;
#ifdef CONV2D_GUARD_MUTANT
 if(!strcmp(argv[1],"--sim"))(void)omega_gpu_session_open();
#endif
 if(!strcmp(argv[1],"--shell-contract")) {shell_contract=1;refusals();}
 else
 if(!strcmp(argv[1],"--gemm-f32"))dense();
 else if(!strcmp(argv[1],"--sim")||!strcmp(argv[1],"--quick")) {
  refusals();
  const OmegaGpuConv2dShape cases[]={
   {2,3,5,7,5,1,1,1,1,0,0,0,0,1,1,1,0,0},
   {2,3,5,7,5,3,3,1,1,1,2,2,1,1,1,1,0,0},
   {2,6,7,9,9,3,3,2,2,2,1,1,2,2,2,3,0,0},
   {2,3,5,7,3,3,3,1,2,1,1,1,1,1,1,3,0,0},
   {1,3,3,131,5,1,1,1,1,0,0,0,0,1,1,1,0,0}};
  for(unsigned l=0;l<2;l++)for(unsigned t=0;t<3;t++)for(unsigned i=0;i<5;i++){OmegaGpuConv2dShape s=cases[i];s.layout=l;s.dtype=t;run_conv(s,0);}
  OmegaGpuConv2dShape s=cases[2];CHECK(!run_conv(s,1));
 }else return 2;
 printf("conv2d host+warp-sim: %s (%d checks, %d failures)\n",failed?"FAIL":"PASS",checks,failed);return failed?1:0;
}
