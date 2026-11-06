/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen_pfhost.h"
#include "omega_cubin.h"
#include <stdio.h>
#include <string.h>
extern int omega_blackwell_codegen_rl_prog_c(const OmegaRlSpec *,BlackwellIRProgram *);
extern int omega_blackwell_codegen_mc_prog_c(const OmegaMcSpec *,BlackwellIRProgram *);
extern uint32_t omega_rl_shared_bytes_c(const OmegaRlSpec *);
static BlackwellIRProgram a,b;
static uint8_t x[BW_MAX_IR_INSNS*16],y[BW_MAX_IR_INSNS*16];
static int equal(uint32_t sa,uint32_t sb){
 size_t nx=0,ny=0,cx=0,cy=0;uint8_t *u=NULL,*v=NULL;char err[128];int ok=0;
 if(omega_bw_encode_program(&a,x,sizeof x,&nx)||omega_bw_encode_program(&b,y,sizeof y,&ny))return 0;
 if(omega_cubin_pack("pf_admit",x,nx,NULL,0,OMEGA_CUBIN_PARAM_BASE,omega_bw_gpr_declared(a.regalloc.peak_gpr_usage),sa,&u,&cx,err,sizeof err)){fprintf(stderr,"cubin: %s\n",err);return 0;}
 if(!omega_cubin_pack("pf_admit",y,ny,NULL,0,OMEGA_CUBIN_PARAM_BASE,omega_bw_gpr_declared(b.regalloc.peak_gpr_usage),sb,&v,&cy,err,sizeof err))ok=cx==cy&&!memcmp(u,v,cx);
 omega_cubin_free(u);omega_cubin_free(v);return ok;
}
int main(void){unsigned cases=0;
 for(int m=0;m<=2;m++)for(uint32_t k=128;k<=256;k+=128){
  OmegaRlSpec s={k,32,1,m};
  int ca=omega_blackwell_codegen_rl_prog_c(&s,&a),cb=omega_blackwell_codegen_rl_prog(&s,&b);
  if(ca!=cb)return 1;
  if(!ca){if(!equal(omega_rl_shared_bytes_c(&s),omega_rl_shared_bytes(&s)))return 1;cases++;}
  OmegaMcSpec t={k,4,m};
  ca=omega_blackwell_codegen_mc_prog_c(&t,&a);cb=omega_blackwell_codegen_mc_prog(&t,&b);
  if(ca!=cb)return 1;
  if(!ca){if(!equal(0,0))return 1;cases++;}
 }
 if(cases!=11){fprintf(stderr,"unexpected accepted cubin count %u\n",cases);return 1;}
 printf("PFHOST-ADMIT-CUBIN-IDENTITY PASS cases=%u matching_refusals=1\n",cases);return 0;
}
