/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdio.h>
#include <string.h>
#include "omega_blackwell_codegen_moe.h"
#include "omega_blackwell_codegen_moe2.h"
#include "bw_warp_sim_k.h"
/* Independent reference, WolframLanguageEvaluator:
 * {(1+Mod[56,8]/8)*2^(Quotient[56,8]-7),512*(1+Mod[56,8]/8)*2^(Quotient[56,8]-7)} -> {1,512}
 * {2^64,2^56,512*1*1*1,512/128} -> {18446744073709551616,72057594037927936,512,4}.
 * All weights encode one, unfolded inputs and scales are one, hence each row is 512.
 * Seven output rows and three warps exercise repeated and partial final row groups.
 * This simulator models functional IR execution, NOT scoreboard timing or chip qualification. */
static BlackwellIRProgram prog;
static WsSim sim;
static uint8_t weights[7*512], smem[8192];
static float input[512],scales[4],output[7];
static int run(unsigned variant, int mutant, int order, unsigned warp_order) {
 OmegaFp8GemvSpec spec={512,8,1,3,2,mutant,1,1,1};
 if((variant==1?omega_blackwell_codegen_fp8_gemv_prog(&spec,&prog):omega_blackwell_codegen_fp8_gemv2_prog(&spec,&prog)))return 0;
 memset(weights,56,sizeof weights);
 for(unsigned i=0;i<512;i++)input[i]=18446744073709551616.0f;
 for(unsigned i=0;i<4;i++)scales[i]=72057594037927936.0f;
 for(unsigned i=0;i<7;i++)output[i]=-999.0f;
 uint64_t table[2]={(uintptr_t)weights,(uintptr_t)scales};
 memset(&sim,0,sizeof sim);sim.p=&prog;sim.threads=96;sim.order=order;sim.warp_sched=warp_order;
 sim.smem=smem;sim.smem_bytes=variant==1?omega_fp8_gemv_shared_bytes(&spec):omega_fp8_gemv2_shared_bytes(&spec);
 const uint64_t ptrs[]={(uintptr_t)table,(uintptr_t)input,(uintptr_t)output};
 const unsigned offsets[]={OMEGA_FP8_ARG_TAB,OMEGA_FP8_ARG_X,OMEGA_FP8_ARG_Y};
 for(unsigned i=0;i<3;i++){sim.cbank[offsets[i]/4]=(uint32_t)ptrs[i];sim.cbank[offsets[i]/4+1]=(uint32_t)(ptrs[i]>>32);}
 sim.cbank[OMEGA_FP8_ARG_RG/4]=7;sim.cbank[OMEGA_FP8_ARG_STRIDE/4]=3;
 sim.nranges=5;
 const uintptr_t starts[]={(uintptr_t)table,(uintptr_t)weights,(uintptr_t)scales,(uintptr_t)input,(uintptr_t)output};
 const size_t sizes[]={sizeof table,sizeof weights,sizeof scales,sizeof input,sizeof output};
 for(unsigned i=0;i<5;i++){sim.ranges[i].lo=starts[i];sim.ranges[i].hi=starts[i]+sizes[i];}
 if(ws_run(&sim,1,1)){fprintf(stderr,"MOE%u sim: %s\n",variant,sim.msg);return 0;}
 for(unsigned i=0;i<7;i++)if((output[i]==512.0f)!=(mutant==0)){fprintf(stderr,"MOE%u mutant %d row %u observed %g expected %s512\n",variant,mutant,i,output[i],mutant?"not ":"");return 0;}
 return 1;
}
int main(void){
 const unsigned orders[]={0,1,2654435769u};
 for(unsigned v=1;v<=2;v++)for(int order=0;order<2;order++)for(unsigned w=0;w<3;w++)if(!run(v,0,order,orders[w])||!run(v,1,order,orders[w]))return 1;
 puts("CODEGEN MOE WARP SIM PASS (both builders, fragment/warp orders, scale-drop negative controls)");return 0;
}
