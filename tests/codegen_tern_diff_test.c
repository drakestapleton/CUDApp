/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CUDAPP_MIGRATION_PENDING_C_CODEGEN_TERN 1
#define omega_tern_gemv_chunks oracle_chunks
#define omega_tern_gemv_spec_ok oracle_spec_ok
#define omega_tern_gemv_steps oracle_steps
#define omega_tern_gemv_coalesced_out oracle_coalesced
#define omega_tern_gemv_out_slot_bytes oracle_slot
#define omega_tern_gemv_shared_bytes oracle_shared
#define omega_tern_word_offset oracle_offset
#define omega_tern_i2fp_violations oracle_violations
#define omega_blackwell_codegen_tern_gemv_prog oracle_prog
#define omega_tern_gemv_prog_stats oracle_stats
#include "../src/omega_blackwell_codegen_tern.c"
#undef omega_tern_gemv_chunks
#undef omega_tern_gemv_spec_ok
#undef omega_tern_gemv_steps
#undef omega_tern_gemv_coalesced_out
#undef omega_tern_gemv_out_slot_bytes
#undef omega_tern_gemv_shared_bytes
#undef omega_tern_word_offset
#undef omega_tern_i2fp_violations
#undef omega_blackwell_codegen_tern_gemv_prog
#undef omega_tern_gemv_prog_stats
#include "omega_gpu_om_ir.h"
#include "cudapp_native.h"
extern uint32_t omega_tern_gemv_chunks(const OmegaTernGemvSpec *);
extern int omega_tern_gemv_spec_ok(const OmegaTernGemvSpec *);
extern uint32_t omega_tern_gemv_steps(const OmegaTernGemvSpec *);
extern int omega_tern_gemv_coalesced_out(const OmegaTernGemvSpec *);
extern uint32_t omega_tern_gemv_out_slot_bytes(const OmegaTernGemvSpec *);
extern uint32_t omega_tern_gemv_shared_bytes(const OmegaTernGemvSpec *);
extern uint64_t omega_tern_word_offset(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
extern long omega_tern_i2fp_violations(const BlackwellIRProgram *);
extern int omega_blackwell_codegen_tern_gemv_prog(const OmegaTernGemvSpec *,BlackwellIRProgram *);
static BlackwellIRProgram a,b;
static unsigned cases,refused;
static int equal(const OmegaTernGemvSpec *s) {
 int ra=oracle_prog(s,&a),rb=omega_blackwell_codegen_tern_gemv_prog(s,&b);
 if((ra==0)!=(rb==0)) goto fail;
 /* A refused program has no specified IR output; compare the refusal, not stale scratch state. */
 if(ra){refused++;return 1;}
 if(a.count!=b.count||memcmp(a.insns,b.insns,a.count*sizeof a.insns[0])||memcmp(&a.regalloc,&b.regalloc,sizeof a.regalloc)) goto fail;
 static uint8_t x[BW_MAX_IR_INSNS*16],y[BW_MAX_IR_INSNS*16];size_t nx=0,ny=0;
 if(omega_bw_encode_program(&a,x,sizeof x,&nx)||omega_bw_encode_program(&b,y,sizeof y,&ny)||nx!=ny||memcmp(x,y,nx))goto fail;
 if(omega_tern_i2fp_violations(&b)!=oracle_violations(&a))goto fail;
 cases++;return 1;
fail:
 fprintf(stderr,"TERN DIFF k=%u layout=%u rows=%u warps=%u depth=%u epi=%u mutant=%d C=%d Omega=%d count=%zu/%zu\n",s->k,s->layout,s->rows,s->warps,s->depth,s->epi,s->mutant,ra,rb,a.count,b.count);
 for(size_t i=0;i<a.count&&i<b.count;i++)if(memcmp(&a.insns[i],&b.insns[i],sizeof a.insns[i])){
  fprintf(stderr,"first IR difference %zu op=%d/%d ctrl=%x/%x dst=%d/%d src=%d,%d,%d/%d,%d,%d imm=%x/%x u=%d/%d\n",i,a.insns[i].op,b.insns[i].op,a.insns[i].control,b.insns[i].control,a.insns[i].dst_vreg,b.insns[i].dst_vreg,a.insns[i].src1_vreg,a.insns[i].src2_vreg,a.insns[i].src3_vreg,b.insns[i].src1_vreg,b.insns[i].src2_vreg,b.insns[i].src3_vreg,a.insns[i].imm,b.insns[i].imm,a.insns[i].ureg,b.insns[i].ureg);
  break;
 }
 return 0;
}
static uint32_t corrupt(uint64_t *r,uint64_t k,uint64_t x,uint64_t y,uint64_t z,void *b0,uint64_t nb,void *m,uint64_t nm){
 uint32_t rc=omega_cg_tern(r,k,x,y,z,b0,nb,m,nm);if(!rc&&!*r)((uint64_t*)b0)[6]^=1;return rc;
}
int main(int argc,char **argv){
 if(argc==8){OmegaTernGemvSpec s={(uint32_t)strtoul(argv[1],0,0),(uint32_t)strtoul(argv[2],0,0),(uint32_t)strtoul(argv[3],0,0),(uint32_t)strtoul(argv[4],0,0),(uint32_t)strtoul(argv[5],0,0),(uint32_t)strtoul(argv[6],0,0),atoi(argv[7])};return equal(&s)?0:1;}
 /* Exhaustive finite accepted domain, including all shipped mutation selectors. The spec guard is independently C. */
 for(unsigned k=256;k<=OMEGA_TERN_MAX_K;k+=256)for(unsigned l=0;l<2;l++)for(unsigned r=1;r<=4;r*=2)for(unsigned w=1;w<=32;w++)for(unsigned d=1;d<=16;d++)for(unsigned e=0;e<2;e++)for(int m=0;m<=6;m++){
 OmegaTernGemvSpec s={k,l,r,w,d,e,m};
 if(oracle_spec_ok(&s)!=omega_tern_gemv_spec_ok(&s))return 1;
 if(!oracle_spec_ok(&s))continue;
 if(m==0 && e==0 && w==1 && d==1){
  const uint32_t rr[]={0,1,2*r-1,2*r,UINT32_MAX};
  const uint32_t cc[]={0,1,k/16-1,k/16,UINT32_MAX};
  for(unsigned i=0;i<sizeof rr/sizeof rr[0];i++)for(unsigned j=0;j<sizeof cc/sizeof cc[0];j++)
   if(oracle_offset(l,r,k,rr[i],cc[j])!=omega_tern_word_offset(l,r,k,rr[i],cc[j]))return 1;
 }
 if(oracle_chunks(&s)!=(uint32_t)omega_tern_gemv_chunks(&s)||oracle_steps(&s)!=omega_tern_gemv_steps(&s)||oracle_coalesced(&s)!=omega_tern_gemv_coalesced_out(&s)||oracle_slot(&s)!=omega_tern_gemv_out_slot_bytes(&s)||oracle_shared(&s)!=omega_tern_gemv_shared_bytes(&s)||!equal(&s))return 1;
 }
 const uint32_t edge[]={0,1,255,257,8193,16385,UINT32_MAX};
 for(unsigned i=0;i<sizeof edge/sizeof edge[0];i++){OmegaTernGemvSpec s={edge[i],0,1,1,1,0,0};if(!equal(&s))return 1;}
 const OmegaTernGemvSpec invalid[]={
  {256,2,1,1,1,0,0},{256,0,0,1,1,0,0},{256,0,3,1,1,0,0},{256,0,8,1,1,0,0},
  {256,0,1,0,1,0,0},{256,0,1,33,1,0,0},{256,0,1,1,0,0,0},{256,0,1,1,17,0,0},
  {256,0,1,1,1,2,0},{256,0,1,1,1,0,-1},{256,0,1,1,1,0,7}
 };
 for(unsigned i=0;i<sizeof invalid/sizeof invalid[0];i++)
  if(oracle_spec_ok(&invalid[i])||omega_tern_gemv_spec_ok(&invalid[i])||!equal(&invalid[i]))return 1;
 OmegaTernGemvSpec s={256,0,1,1,1,0,0};
 if(omega_blackwell_codegen_tern_gemv_prog(NULL,&b)!=-1||omega_blackwell_codegen_tern_gemv_prog(&s,NULL)!=-1)return 1;
 if(oracle_prog(&s,&a)||omega_gpu_om_ir_replay(corrupt,256,UINT64_C(1)<<32,(UINT64_C(1)<<32)|1,0,&b,NULL))return 1;
 if(a.count==b.count&&!memcmp(a.insns,b.insns,a.count*sizeof a.insns[0])){fprintf(stderr,"record negative control escaped\n");return 1;}
 puts("CODEGEN TERN RECORD NEGATIVE CONTROL CAUGHT");
 printf("CODEGEN TERN DIFF PASS (%u programs, %u refusals)\n",cases,refused);return 0;
}
