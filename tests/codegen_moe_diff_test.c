/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "omega_blackwell_codegen_moe.h"
#include "omega_blackwell_codegen_moe2.h"
#include "omega_cubin.h"
#include "cudapp_native.h"
#include "omega_gpu_om_ir.h"
extern int omega_blackwell_codegen_fp8_gemv_prog_oracle(const OmegaFp8GemvSpec *, BlackwellIRProgram *);
extern int omega_blackwell_codegen_fp8_gemv2_prog_oracle(const OmegaFp8GemvSpec *, BlackwellIRProgram *);
extern uint32_t omega_fp8_gemv_steps_oracle(const OmegaFp8GemvSpec *);
extern uint32_t omega_fp8_gemv_shared_bytes_oracle(const OmegaFp8GemvSpec *);
extern uint32_t omega_fp8_gemv2_shared_bytes_oracle(const OmegaFp8GemvSpec *);
extern int omega_fp8_gemv2_spec_ok_oracle(const OmegaFp8GemvSpec *);
static BlackwellIRProgram a,b;
static unsigned cases, refused;
static int equal(unsigned version, const OmegaFp8GemvSpec *s) {
 int ra=version==1?omega_blackwell_codegen_fp8_gemv_prog_oracle(s,&a):omega_blackwell_codegen_fp8_gemv2_prog_oracle(s,&a);
 int rb=version==1?omega_blackwell_codegen_fp8_gemv_prog(s,&b):omega_blackwell_codegen_fp8_gemv2_prog(s,&b);
 if(ra!=rb)goto fail;
 if(ra){refused++;return 1;}
 if(a.count!=b.count||memcmp(a.insns,b.insns,a.count*sizeof a.insns[0])||memcmp(&a.regalloc,&b.regalloc,sizeof a.regalloc))goto fail;
 static uint8_t x[BW_MAX_IR_INSNS*16],y[BW_MAX_IR_INSNS*16];size_t nx=0,ny=0;
 if(omega_bw_encode_program(&a,x,sizeof x,&nx)||omega_bw_encode_program(&b,y,sizeof y,&ny)||nx!=ny||memcmp(x,y,nx))goto fail;
 uint8_t *cx=NULL,*cy=NULL;size_t lx=0,ly=0;char error[256];
 const OmegaCubinParam params[]={{OMEGA_FP8_ARG_TAB,8,1},{OMEGA_FP8_ARG_X,8,1},{OMEGA_FP8_ARG_Y,8,1},{OMEGA_FP8_ARG_RG,4,0},{OMEGA_FP8_ARG_XSTRIDE,4,0},{OMEGA_FP8_ARG_YSTRIDE,4,0},{OMEGA_FP8_ARG_STRIDE,4,0}};
 uint32_t shared=version==1?omega_fp8_gemv_shared_bytes(s):omega_fp8_gemv2_shared_bytes(s);
 int ex=omega_cubin_pack("moe",x,nx,params,7,OMEGA_FP8_ARG_TAB+40,omega_bw_gpr_declared(a.regalloc.peak_gpr_usage),shared,&cx,&lx,error,sizeof error);
 int ey=omega_cubin_pack("moe",y,ny,params,7,OMEGA_FP8_ARG_TAB+40,omega_bw_gpr_declared(b.regalloc.peak_gpr_usage),shared,&cy,&ly,error,sizeof error);
 int ceq=!ex&&!ey&&lx==ly&&!memcmp(cx,cy,lx);omega_cubin_free(cx);omega_cubin_free(cy);
 if(!ceq)goto fail;
 cases++;return 1;
fail:
 fprintf(stderr,"MOE%u DIFF k=%u lb=%u rows=%u warps=%u depth=%u mutant=%d sched=%u coal=%u wfirst=%u C=%d Omega=%d count=%zu/%zu\n",version,s->k,s->lane_bytes,s->rows,s->warps,s->depth,s->mutant,s->sched,s->coal,s->wfirst,ra,rb,a.count,b.count);
 for(size_t i=0;i<a.count&&i<b.count;i++)if(memcmp(&a.insns[i],&b.insns[i],sizeof a.insns[i])) {
  fprintf(stderr,"IR %zu op=%d/%d ctrl=%x/%x dst=%d/%d src=%d,%d,%d/%d,%d,%d imm=%x/%x u=%d/%d\n",i,a.insns[i].op,b.insns[i].op,a.insns[i].control,b.insns[i].control,a.insns[i].dst_vreg,b.insns[i].dst_vreg,a.insns[i].src1_vreg,a.insns[i].src2_vreg,a.insns[i].src3_vreg,b.insns[i].src1_vreg,b.insns[i].src2_vreg,b.insns[i].src3_vreg,a.insns[i].imm,b.insns[i].imm,a.insns[i].ureg,b.insns[i].ureg);break;
 }
 return 0;
}
static void set_axis(OmegaFp8GemvSpec *s,unsigned axis,uint32_t value){
 switch(axis){
 case 0:s->k=value;break;case 1:s->lane_bytes=value;break;
 case 2:s->rows=value;break;case 3:s->warps=value;break;
 case 4:s->depth=value;break;case 5:s->mutant=(int32_t)value;break;
 case 6:s->sched=value;break;case 7:s->coal=value;break;case 8:s->wfirst=value;break;
 }
}
static uint32_t corrupt(uint64_t *r,uint64_t k,uint64_t a0,uint64_t q,uint64_t c,void *records,uint64_t nr,void *meta,uint64_t nm){
 const uint64_t fields[16]={256,8,1,1};
 (void)k;(void)a0;(void)q;(void)c;
 uint32_t rc=omega_cg_moe(r,fields,16,records,nr,meta,nm);
 if(!rc&&!*r)((uint64_t*)records)[6]^=1;
 return rc;
}
static int negative(void){
 OmegaFp8GemvSpec s={256,8,1,1,0,0,0,0,0};
 if(omega_blackwell_codegen_fp8_gemv_prog_oracle(&s,&a)||omega_gpu_om_ir_replay(corrupt,0,0,0,0,&b,NULL))return 0;
 if(a.count==b.count&&!memcmp(a.insns,b.insns,a.count*sizeof a.insns[0]))return 0;
 puts("CODEGEN MOE RECORD NEGATIVE CONTROL CAUGHT");return 1;
}
int main(int argc,char **argv) {
 if(argc==11){OmegaFp8GemvSpec s={0};unsigned v=(unsigned)strtoul(argv[1],0,0);s.k=(uint32_t)strtoul(argv[2],0,0);s.lane_bytes=(uint32_t)strtoul(argv[3],0,0);s.rows=(uint32_t)strtoul(argv[4],0,0);s.warps=(uint32_t)strtoul(argv[5],0,0);s.depth=(uint32_t)strtoul(argv[6],0,0);s.mutant=atoi(argv[7]);s.sched=(uint32_t)strtoul(argv[8],0,0);s.coal=(uint32_t)strtoul(argv[9],0,0);s.wfirst=(uint32_t)strtoul(argv[10],0,0);return equal(v,&s)?0:1;}
 for(unsigned v=1;v<=2;v++)for(unsigned k=256;k<=32768;k+=256)for(unsigned lb=8;lb<=16;lb*=2)for(unsigned r=1;r<=8;r*=2)for(unsigned w=1;w<=32;w++)for(unsigned d=0;d<=(v==1?16:0);d++)for(int m=0;m<=(v==1?5:3);m++)for(unsigned sc=0;sc<=(v==1?1:0);sc++)for(unsigned co=0;co<=(v==1?1:0);co++)for(unsigned wf=0;wf<=(v==1?1:0);wf++){
  OmegaFp8GemvSpec s={k,lb,r,w,d,m,sc,co,wf};
  if(v==1){if(omega_fp8_gemv_steps(&s)!=omega_fp8_gemv_steps_oracle(&s)||omega_fp8_gemv_shared_bytes(&s)!=omega_fp8_gemv_shared_bytes_oracle(&s))return 1;if(!omega_fp8_gemv_steps_oracle(&s))continue;}
  else{if(omega_fp8_gemv2_spec_ok(&s)!=omega_fp8_gemv2_spec_ok_oracle(&s)||omega_fp8_gemv2_shared_bytes(&s)!=omega_fp8_gemv2_shared_bytes_oracle(&s))return 1;if(!omega_fp8_gemv2_spec_ok_oracle(&s))continue;}
  if(!equal(v,&s))return 1;
  /* Variant 2 never reads these four scalar axes in the C oracle. Exercise
   * every bit and all-ones independently for every accepted structural tuple. */
  if(v==2)for(unsigned axis=4;axis<=8;axis++){
   if(axis==5)continue;
   for(unsigned bit=0;bit<=32;bit++){
    OmegaFp8GemvSpec scalar=s;set_axis(&scalar,axis,bit==32?UINT32_MAX:UINT32_C(1)<<bit);
    if(!equal(v,&scalar))return 1;
   }
  }
 }
 const uint32_t edges[]={0,1,2,3,4,7,8,15,16,17,31,32,33,127,128,255,256,257,511,512,513,32767,32768,32769,UINT32_MAX};
 for(unsigned v=1;v<=2;v++)for(unsigned axis=0;axis<9;axis++){
  for(unsigned i=0;i<sizeof edges/sizeof edges[0];i++){
   OmegaFp8GemvSpec s={512,8,1,3,2,0,1,1,1};set_axis(&s,axis,edges[i]);if(!equal(v,&s))return 1;
  }
  for(unsigned bit=0;bit<32;bit++){
   OmegaFp8GemvSpec s={512,8,1,3,2,0,1,1,1};set_axis(&s,axis,UINT32_C(1)<<bit);if(!equal(v,&s))return 1;
  }
 }
 OmegaFp8GemvSpec valid={512,8,1,3,2,0,1,1,1};
 if(omega_blackwell_codegen_fp8_gemv_prog(NULL,&b)!=-1||omega_blackwell_codegen_fp8_gemv_prog(&valid,NULL)!=-1||
    omega_blackwell_codegen_fp8_gemv2_prog(NULL,&b)!=-1||omega_blackwell_codegen_fp8_gemv2_prog(&valid,NULL)!=-1||!negative())return 1;
 printf("CODEGEN MOE DIFF PASS (%u programs, %u refusals)\n",cases,refused);return 0;
}
