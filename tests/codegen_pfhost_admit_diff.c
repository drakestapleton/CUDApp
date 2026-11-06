/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen_pfhost.h"
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
extern int omega_rl_spec_ok_c(const OmegaRlSpec *);
extern uint32_t omega_rl_shared_bytes_c(const OmegaRlSpec *);
extern int omega_mc_spec_ok_c(const OmegaMcSpec *);
static unsigned cases;static int red;
static int same(uint64_t a,uint64_t b){cases++;return (a^(red&&cases==1))==b;}
static int rl(OmegaRlSpec s){
 uint64_t r;
 for(unsigned op=0;op<2;op++){
  uint64_t want=op?omega_rl_shared_bytes_c(&s):(uint32_t)omega_rl_spec_ok_c(&s);
  uint64_t got=op?omega_rl_shared_bytes(&s):(uint32_t)omega_rl_spec_ok(&s);
  if(omega_pf_admit(&r,op,s.k,s.e,s.t,(uint32_t)s.mutant)||!same(r,want)||got!=want)return 0;
 }
 return 1;
}
static int mc(OmegaMcSpec s){uint64_t r;return !omega_pf_admit(&r,2,s.h,s.top_k,0,(uint32_t)s.mutant)&&same(r,omega_mc_spec_ok_c(&s))&&r==(uint64_t)omega_mc_spec_ok(&s);}
int main(int argc,char **argv){(void)argv;red=argc>1;
 if(omega_rl_spec_ok(NULL)||omega_rl_shared_bytes(NULL)||omega_mc_spec_ok(NULL))return 1;
 for(uint32_t k=128;k<=16384;k+=128)for(uint32_t e=32;e<=1024;e+=32)for(uint32_t t=1;t<=16;t++)for(int m=-1;m<=3;m++){OmegaRlSpec s={k,e,t,m};if(!rl(s))goto fail;}
 for(uint32_t h=128;h<=4096;h+=128)for(uint32_t top=4;top<=16;top+=4)for(int m=-1;m<=3;m++){OmegaMcSpec s={h,top,m};if(!mc(s))goto fail;}
 const uint32_t edge[]={0,1,2,3,4,5,15,16,17,31,32,33,127,128,129,1023,1024,1025,4095,4096,4097,16383,16384,16385,UINT32_MAX};
 for(unsigned f=0;f<4;f++)for(unsigned j=0;j<sizeof edge/sizeof *edge;j++){uint32_t v[4]={128,32,1,0};v[f]=edge[j];OmegaRlSpec s={v[0],v[1],v[2],(int)v[3]};if(!rl(s))goto fail;}
 for(unsigned f=0;f<3;f++)for(unsigned j=0;j<sizeof edge/sizeof *edge;j++){uint32_t v[3]={128,4,0};v[f]=edge[j];OmegaMcSpec s={v[0],v[1],(int)v[2]};if(!mc(s))goto fail;}
 printf("PFHOST-ADMIT-DIFF PASS cases=%u\n",cases);return 0;
fail:fprintf(stderr,"PFHOST-ADMIT-DIFF FAIL case=%u\n",cases);return 1;}
