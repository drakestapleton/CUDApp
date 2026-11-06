/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen_tc.h"
#include "omega_blackwell_codegen_tc2.h"
#include "omega_cubin.h"
#include <stdio.h>
#include <string.h>
extern int oracle_omega_blackwell_codegen_tcgemm_prog(const OmegaTcGemmSpec *,BlackwellIRProgram *);
extern int oracle_omega_blackwell_codegen_tc2_prog(const OmegaTc2Spec *,BlackwellIRProgram *);
extern uint32_t oracle_omega_tcgemm_shared_bytes(const OmegaTcGemmSpec *);
extern uint32_t oracle_omega_tc2_shared_bytes(const OmegaTc2Spec *);
static BlackwellIRProgram a,b;
static uint8_t x[BW_MAX_IR_INSNS*16],y[BW_MAX_IR_INSNS*16];
static int equal(uint32_t sa,uint32_t sb) {
 size_t nx=0,ny=0,cx=0,cy=0;uint8_t *u=NULL,*v=NULL;char err[128];int ok=0;
 if(omega_bw_encode_program(&a,x,sizeof x,&nx)||omega_bw_encode_program(&b,y,sizeof y,&ny))return 0;
 if(omega_cubin_pack("tc_host",x,nx,NULL,0,OMEGA_CUBIN_PARAM_BASE,omega_bw_gpr_declared(a.regalloc.peak_gpr_usage),sa,&u,&cx,err,sizeof err)) { fprintf(stderr,"oracle cubin: %s\n",err);return 0; }
 if(!omega_cubin_pack("tc_host",y,ny,NULL,0,OMEGA_CUBIN_PARAM_BASE,omega_bw_gpr_declared(b.regalloc.peak_gpr_usage),sb,&v,&cy,err,sizeof err))ok=cx==cy&&!memcmp(u,v,cx);
 omega_cubin_free(u);omega_cubin_free(v);return ok;
}
int main(void) {
 unsigned cases=0;
 for(int m=0;m<=4;m++) {
  OmegaTcGemmSpec s={2048,4,2,8,m};
  if(oracle_omega_blackwell_codegen_tcgemm_prog(&s,&a)||omega_blackwell_codegen_tcgemm_prog(&s,&b)||!equal(oracle_omega_tcgemm_shared_bytes(&s),omega_tcgemm_shared_bytes(&s)))return 1;
  cases++;
  for(unsigned ep=0;ep<=1;ep++)for(unsigned dr=0;dr<=1;dr++) {
   OmegaTc2Spec t={2048,8,2,16,1,ep,dr,1536,m};
   if(oracle_omega_blackwell_codegen_tc2_prog(&t,&a)||omega_blackwell_codegen_tc2_prog(&t,&b)||!equal(oracle_omega_tc2_shared_bytes(&t),omega_tc2_shared_bytes(&t)))return 1;
   cases++;
  }
 }
 printf("TC-HOST-CUBIN-IDENTITY PASS cases=%u\n",cases);return 0;
}
