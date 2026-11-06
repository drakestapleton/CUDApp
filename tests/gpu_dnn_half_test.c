/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Native words derive from omega_numeric.c E1_CVT and its sm_121 oracle.
 * Independent integer references live in omega_numeric.c, not this simulator. */
#include "omega_blackwell_codegen.h"
#include "omega_numeric.h"
#include "bw_warp_sim_k.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int words(BlackwellIRProgram *p, unsigned op, unsigned d, unsigned a,
                 int guard, uint8_t *code, size_t *bytes) {
    omega_bw_ir_init(p);
    if (omega_bw_ir_alloc_vreg(p) != 0 || omega_bw_ir_alloc_vreg(p) != 1) abort();
    p->regalloc.vreg_to_phys[0] = (int)d;
    p->regalloc.vreg_to_phys[1] = (int)a;
    BlackwellIRInsn in = {.op=(BlackwellIROpcode)op,.dst_vreg=0,.src1_vreg=1,
        .src2_vreg=-1,.src3_vreg=-1,.ureg=-1,.control=0x010fca00u,
        .predicate_p0=guard!=0,.predicate_not=guard==2};
    if (omega_bw_ir_append(p,&in)) abort();
    return omega_bw_encode_program(p,code,16384,bytes);
}
static int golden(BlackwellIRProgram *p, const char *dump) {
    uint8_t code[16384];size_t bytes=0;
    unsigned regs[][2]={{9,2},{0,0},{13,27},{254,253}};
    FILE *f=dump?fopen(dump,"wb"):NULL;
    if(dump&&!f)return 0;
    for(unsigned op=66;op<=67;op++)for(unsigned j=0;j<4;j++)for(int g=0;g<3;g++) {
        unsigned d=regs[j][0],a=regs[j][1];uint32_t w[4];
        if(words(p,op,d,a,g,code,&bytes)){if(f)fclose(f);return 0;}
        memcpy(w,code,sizeof w);
        uint32_t pred=(g==0?7u:g==1?0u:8u)<<12;
        uint32_t base=op==66?0xff007230u:0x7304u;
        uint32_t want0=(base&~0xf000u)|(d<<16)|pred;
        uint32_t want1=(op==66?0x20000000u:0u)|a;
        if(w[0]!=want0||w[1]!=want1||w[2]!=(op==66?0x4100u:0x200800u)||w[3]!=0x010fca00u) {
            if(f)fclose(f);
            return 0;
        }
        if(f&&fwrite(code,1,16,f)!=16){fclose(f);return 0;}
    }
    if(f&&fclose(f))return 0;
    /* Unknown op is rejected, not silently encoded as a neighbouring conversion. */
    return words(p,BW_IR_LAST+1,9,2,0,code,&bytes)!=0;
}
static int simulation(BlackwellIRProgram *p) {
    WsSim s={0};WsLane lane={0};s.p=p;
    p->regalloc.vreg_to_phys[0]=0;p->regalloc.vreg_to_phys[1]=1;
    BlackwellIRInsn in={.op=(BlackwellIROpcode)66,.dst_vreg=0,.src1_vreg=1,
        .src2_vreg=-1,.src3_vreg=-1,.ureg=-1};
    for(unsigned h=0;h<65536;h++) {
        lane.r[1]=h;s.err=0;ws_lane_exec(&s,&lane,&in,0,0,0,0);
        float want=omega_ref_f16_to_f32((uint16_t)h),got=ws_u2f(lane.r[0]);
        if(s.err||(!isnan(want)&&ws_f2u(want)!=ws_f2u(got))||(isnan(want)&&!isnan(got)))return 0;
        if((h&0x7c00u)!=0x7c00u) {
            in.op=(BlackwellIROpcode)67;lane.r[1]=ws_f2u(want);s.err=0;
            ws_lane_exec(&s,&lane,&in,0,0,0,0);
            if(s.err||lane.r[0]!=h)return 0;
            in.op=(BlackwellIROpcode)66;
        }
    }
    in.op=(BlackwellIROpcode)67;
    uint32_t seed=1;
    for(unsigned i=0;i<65536;i++) {
        seed=seed*1664525u+1013904223u;lane.r[1]=seed;s.err=0;
        ws_lane_exec(&s,&lane,&in,0,0,0,0);
        uint16_t want=omega_ref_f32_to_f16(ws_u2f(seed));
        if(s.err||((want&0x7fffu)>0x7c00u?((lane.r[0]&0x7fffu)<=0x7c00u):lane.r[0]!=want))return 0;
    }
    omega_bw_ir_init(p);
    in.op=BW_IR_EXIT;
    in.dst_vreg=in.src1_vreg=in.src2_vreg=in.src3_vreg=-1;
    if(omega_bw_ir_append(p,&in))abort();
    s.threads=32;
    return ws_run(&s,1,1)==0;
}
int main(int argc,char **argv) {
    BlackwellIRProgram *p=malloc(sizeof *p);if(!p)return 2;
    int okay=golden(p,argc==2?argv[1]:NULL);
    if(okay)okay=simulation(p);
    free(p);
    puts(okay?"dnn-half: PASS (native words, predicates, exhaustive half reference, narrowing controls)":"dnn-half: FAIL (native encoding or independent reference)");
    return !okay;
}
