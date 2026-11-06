/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Queue epochs may complete in reverse order until an emitted WFI drains them.
 * Each dispatch executes real IR in the existing warp simulator. No fake values
 * are injected on hazards: missing ordering changes the computed buffer data. */
#include "cudapp_native.h"
#include "bw_warp_sim.h"
#define fadd em_fadd
#define ffma em_ffma
#include "omega_bw_em.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N 32u
#define BUFS 6u
#define LAUNCHES 7u
static uint64_t descriptors[200704],barriers[4096];
static const unsigned ins[LAUNCHES]={0,2,1,3,4,0,5};
static const unsigned outs[LAUNCHES]={1,3,4,5,0,0,2};
static BlackwellIRProgram kernel;
static int build(void) {
    omega_bw_ir_init(&kernel);Em em={0};em.p=&kernel;Em *e=&em;
    int ud=UV64(e),pi=V64(e),po=V64(e),a=V64(e),tid=V(e),v=V(e),one=V(e);
    ldcu64(e,ud,0x358);ldc64(e,pi,0x380);ldc64(e,po,0x388);
    s2r(e,tid,BW_SR_TID_X);movi(e,one,0x3f800000u);
    imadw(e,a,tid,4u,pi);ldg32(e,v,a,ud,1);fadd(e,v,v,one);
    imadw(e,a,tid,4u,po);stg(e,a,0,v,ud);tail(e);
    return em.err || omega_bw_regalloc_solve(&kernel);
}
static int dispatch(float data[BUFS][N],unsigned i,int order) {
    static WsSim s; memset(&s,0,sizeof s);s.p=&kernel;s.threads=N;s.order=order;
    uint64_t pi=(uintptr_t)data[ins[i]],po=(uintptr_t)data[outs[i]];
    s.cbank[0x380/4]=(uint32_t)pi;s.cbank[0x384/4]=(uint32_t)(pi>>32);
    s.cbank[0x388/4]=(uint32_t)po;s.cbank[0x38c/4]=(uint32_t)(po>>32);
    s.nranges=1;s.ranges[0]=(WsRange){(uintptr_t)data,(uintptr_t)(data+BUFS)};
    if(ws_run(&s,1,1)) { fprintf(stderr,"FAIL warp simulator: %s\n",s.msg);return 1; }
    return 0;
}
int main(void) {
    if(build()) return 2;
    for(unsigned i=0;i<LAUNCHES;i++) {
        uint64_t *p=descriptors+i*49;p[0]=2;
        p[1]=4096+ins[i]*sizeof(float[N]);p[2]=sizeof(float[N]);p[3]=1;
        p[4]=4096+outs[i]*sizeof(float[N]);p[5]=sizeof(float[N]);p[6]=2;
    }
    uint64_t a[16]={LAUNCHES,LAUNCHES,0,0,16384},o[16],r=1;
    if(omega_cmdlist_plan(&r,a,16,descriptors,200704,barriers,4096,o,16)||r) return 2;
    for(int order=0;order<2;order++) {
        float serial[BUFS][N],queued[BUFS][N];
        for(unsigned b=0;b<BUFS;b++) for(unsigned j=0;j<N;j++) serial[b][j]=(float)(b*N+j);
        memcpy(queued,serial,sizeof serial);
        for(unsigned i=0;i<LAUNCHES;i++) if(dispatch(serial,i,order)) return 2;
        unsigned begin=0;
        for(unsigned end=1;end<=LAUNCHES;end++) if(end==LAUNCHES || barriers[end]) {
            for(unsigned i=end;i>begin;i--) if(dispatch(queued,i-1,order)) return 2;
            begin=end;
        }
        if(memcmp(serial,queued,sizeof serial)) { fprintf(stderr,"FAIL sim data: queue epochs differ from serialized execution (warp order %d)\n",order);return 1; }
    }
    printf("PASS sim data: seven dispatches, independent overlap and nonadjacent RAW/WAR/WAW, two warp orders\n");
    return 0;
}
