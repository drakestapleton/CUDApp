/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen_dec.h"
#include "cudapp_native.h"
#include "bw_warp_sim_k.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void){
 static BlackwellIRProgram prog;uint32_t in[1280] __attribute__((aligned(16))),out[1024] __attribute__((aligned(16))),tab[256] __attribute__((aligned(16)));
 WsSim *sim=calloc(1,sizeof *sim);if(!sim)return 1;
 for(unsigned i=0;i<1280;i++){in[i]=i;}
 for(unsigned i=0;i<256;i++){tab[i]=i;}
 uint64_t v[16]={(uintptr_t)in,(uintptr_t)tab,(uintptr_t)out},w[32],r=0;
 if(omega_mg_wire(&r,v,16,w,32,4)||r!=1){free(sim);return 1;}
 unsigned killed=0;
 for(unsigned mutant=0;mutant<2;mutant++){
  OmegaDecSpec spec={.kind=OMEGA_DEC_GATHER,.stride=256,.width=256,.mutant=(int)mutant};
  if(omega_blackwell_codegen_dec_prog(&spec,&prog)){free(sim);return 1;}
  for(unsigned order=0;order<2;order++){
   memset(sim,0,sizeof *sim);memset(out,0xff,sizeof out);sim->p=&prog;sim->threads=256;sim->order=(int)order;
   for(unsigned i=0;i<6;i++)sim->cbank[OMEGA_DEC_GA_IN/4+i]=(uint32_t)w[i];
   sim->nranges=3;sim->ranges[0].lo=(uintptr_t)in;sim->ranges[0].hi=(uintptr_t)(in+1280);
   sim->ranges[1].lo=(uintptr_t)tab;sim->ranges[1].hi=(uintptr_t)(tab+256);
   sim->ranges[2].lo=(uintptr_t)out;sim->ranges[2].hi=(uintptr_t)(out+1024);
   int rc=ws_run(sim,1,4),same=!memcmp(in,out,sizeof out);
   if((!mutant&&(rc||!same))||(mutant&&!rc&&same)){fprintf(stderr,"FAIL graph gather warp mutant=%u order=%u rc=%d %s\n",mutant,order,rc,sim->msg);free(sim);return 1;}
   if(mutant)killed++;
  }
 }
 free(sim);printf("MODEL-GRAPHS gather warp PASS orders=2 rows=4 source-index-mutant-killed=%u\n",killed);return 0;
}
