/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "omega_blackwell_codegen_tc.h"
#include "bw_warp_sim_tc.h"

static BlackwellIRProgram p;
static WsSim sim;

static void pointer(unsigned at, const void *p) {
    uint64_t v=(uintptr_t)p;
    sim.cbank[at/4]=(uint32_t)v;
    sim.cbank[at/4+1]=(uint32_t)(v>>32);
}
static void range(unsigned at,const void *p,size_t bytes) {
    sim.ranges[at].lo=(uintptr_t)p;
    sim.ranges[at].hi=(uintptr_t)p+bytes;
    sim.nranges=at+1;
}
static int probe(void) {
    static uint32_t input[OMEGA_TC_PROBE_IN_BYTES/4];
    static float output[OMEGA_TC_PROBE_OUT_BYTES/4];
    for(unsigned i=0;i<192;i++)input[i]=0x3f803f80u;
    for(unsigned i=0;i<128;i++) {
        float c=(float)(i%17);
        memcpy(&input[192+i],&c,4);
    }
    for(int mutant=0;mutant<=1;mutant++) {
        if(omega_blackwell_codegen_tc_probe_prog(mutant,&p))return 1;
        memset(&sim,0,sizeof sim);sim.p=&p;sim.threads=32;
        memset(output,0xee,sizeof output);
        pointer(0x380,input);pointer(0x388,output);
        range(0,input,sizeof input);range(1,output,sizeof output);
        if(ws_run(&sim,1,1)){fprintf(stderr,"probe sim: %s\n",sim.msg);return 1;}
        for(unsigned i=0;i<128;i++) {
            float want=mutant?0.0f:(float)(i%17);
            for(unsigned k=0;k<16;k++)want+=1.0f;
            if(output[i]!=want){fprintf(stderr,"probe output[%u] differs\n",i);return 1;}
        }
    }
    puts("CODEGEN TC PROBE SIM PASS (accumulate and zero accumulator)");return 0;
}
static int bandwidth(void) {
    enum { THREADS=64, ITERS=3, GRID=2, REGION=ITERS*THREADS*128/4 };
    static uint32_t input[GRID*REGION],output[GRID*THREADS];
    for(unsigned i=0;i<GRID*REGION;i++)input[i]=i*2654435761u;
    for(unsigned groups=1;groups<=2;groups++) {
        if(omega_blackwell_codegen_tc_bw_prog(ITERS,groups,THREADS,&p))return 1;
        memset(&sim,0,sizeof sim);sim.p=&p;sim.threads=THREADS;
        memset(output,0xee,sizeof output);
        pointer(0x380,input);pointer(0x388,output);
        range(0,input,sizeof input);range(1,output,sizeof output);
        if(ws_run(&sim,GRID,1)){fprintf(stderr,"bandwidth sim: %s\n",sim.msg);return 1;}
        for(unsigned c=0;c<GRID;c++)for(unsigned t=0;t<THREADS;t++) {
            uint32_t want=0;
            const uint32_t *r=input+(groups==1?0:c)*REGION;
            for(unsigned it=0;it<ITERS;it++)for(unsigned q=0;q<8;q++)for(unsigned j=0;j<4;j++)
                want^=r[(it*THREADS*128+q*THREADS*16+t*16)/4+j];
            if(output[c*THREADS+t]!=want){fprintf(stderr,"bandwidth output differs\n");return 1;}
        }
    }
    puts("CODEGEN TC BANDWIDTH SIM PASS (shared and distinct CTA regions)");return 0;
}
int main(void) {
    if(probe() || bandwidth())return 1;
    puts("CODEGEN TC SIM PASS");return 0;
}
