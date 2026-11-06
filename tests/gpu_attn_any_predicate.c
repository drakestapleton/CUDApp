/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen.h"
#include "bw_warp_sim_k.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void put64(uint32_t *p,uint64_t n) { memcpy(p,&n,sizeof n); }
static void add(BlackwellIRProgram *p, BlackwellIROpcode op, int d, int a, int b, int c, uint32_t imm, int guard) {
    BlackwellIRInsn in={.op=op,.dst_vreg=d,.src1_vreg=a,.src2_vreg=b,.src3_vreg=c,.ureg=-1,.imm=imm,
        .predicate_p0=guard!=0,.predicate_not=guard==2};
    if(omega_bw_ir_append(p,&in)<0) abort();
}
static void setup(BlackwellIRProgram *p) {
    omega_bw_ir_init(p);
    for(int i=0;i<6;i++) if((i==1||i==2?omega_bw_ir_alloc_vreg64(p):omega_bw_ir_alloc_vreg(p))!=i) abort();
    add(p,BW_IR_S2R,0,-1,-1,-1,BW_SR_TID_X,0);
    add(p,BW_IR_LDC64,1,-1,-1,-1,896,0);
    add(p,BW_IR_IMAD_WIDE,2,0,-1,1,4,0);
    add(p,BW_IR_MOV_IMM,3,-1,-1,-1,1,0);
    add(p,BW_IR_ISETP_GE_U32,-1,0,3,-1,0,0);
}
static int memory_case(int store,int guard) {
    BlackwellIRProgram *p=malloc(sizeof *p); if(!p) abort(); setup(p);
    add(p,BW_IR_MOV_IMM,4,-1,-1,-1,123,0);
    add(p,store?BW_IR_STG_E:BW_IR_LDG_E,store?-1:4,2,store?4:-1,-1,0,guard);
    add(p,BW_IR_EXIT,-1,-1,-1,-1,0,0);
    if(omega_bw_regalloc_solve(p)) abort();
    uint32_t word=7; WsSim s={0};s.p=p;s.threads=32;s.nranges=1;
    s.ranges[0]=(WsRange){(uintptr_t)&word,(uintptr_t)&word+4};put64(&s.cbank[896/4],(uintptr_t)&word);
    int rc=ws_run(&s,1,1);
    int okay=guard?(rc==0 && word==(store?123u:7u)):(rc!=0 && s.err==WS_E_MEM);
    free(p);return okay;
}
static int shuffle_case(int guard) {
    BlackwellIRProgram *p=malloc(sizeof *p);if(!p)abort();setup(p);
    add(p,BW_IR_MOV_IMM,4,-1,-1,-1,0xa5,0);
    add(p,BW_IR_SHFL_BFLY,4,0,-1,-1,1,guard);
    add(p,BW_IR_STG_E,-1,2,4,-1,0,0);
    add(p,BW_IR_EXIT,-1,-1,-1,-1,0,0);
    if(omega_bw_regalloc_solve(p))abort();
    uint32_t out[32]={0};WsSim s={0};s.p=p;s.threads=32;s.nranges=1;
    s.ranges[0]=(WsRange){(uintptr_t)out,(uintptr_t)(out+32)};put64(&s.cbank[896/4],(uintptr_t)out);
    int okay=ws_run(&s,1,1)==0;
    for(unsigned i=0;i<32;i++) {
        int executes=guard==2?i==0:i!=0;
        if(out[i]!=(executes?(i^1u):0xa5u))okay=0;
    }
    free(p);return okay;
}
int main(void) {
    int okay=memory_case(0,2)&memory_case(1,2)&memory_case(0,0)&memory_case(1,0)&shuffle_case(1)&shuffle_case(2);
    puts(okay?"warp-sim predicates: PASS (masked LDG/ST, unmasked negatives, guarded SHFL)":"warp-sim predicates: FAIL");
    return !okay;
}
