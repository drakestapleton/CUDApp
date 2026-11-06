/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_om_ir.h"
#include <stdio.h>
#include <string.h>
static uint32_t emit(uint64_t *ret,uint64_t kind,uint64_t a,uint64_t b,uint64_t c,void *ip,uint64_t ni,void *mp,uint64_t nm) {
    (void)a;(void)b;(void)c;
    if(ni!=8192 || nm!=512) return 1;
    uint64_t *i=ip,*m=mp;
    m[0]=4;m[1]=kind==5?161:160;m[7]=kind==0?0:kind==2?2:1;
    for(unsigned v=0;v<m[1];++v) m[64+v]=1;
    for(unsigned k=0;k<4;++k) for(unsigned f=1;f<=5;++f) i[k*8+f]=UINT64_MAX;
    i[0]=BW_IR_MOV_RZ;i[1]=159;
    i[8]=BW_IR_NOP;
    i[16]=BW_IR_MOV_RZ;i[17]=159;
    i[24]=BW_IR_EXIT;
    m[511]=kind==3?1:kind==4?4:3;
    *ret=0;return 0;
}
int main(void) {
    BlackwellIRProgram a,b;
    int expect[]={0,0,OMEGA_OM_IR_LIVE_FLAG,OMEGA_OM_IR_LIVE_RANGE,OMEGA_OM_IR_LIVE_RANGE,OMEGA_OM_IR_LIVE_RANGE};
    for(unsigned k=0;k<6;++k) {
        int rc=omega_gpu_om_ir_replay(emit,k,0,0,0,&a,NULL);
        if(rc!=expect[k]) {fprintf(stderr,"FAIL liveness case=%u rc=%d expected=%d\n",k,rc,expect[k]);return 1;}
        if(k==0) b=a;
        if(k==1 && (a.regalloc.intervals[159].last_use!=3 || b.regalloc.intervals[159].last_use!=2)) return 2;
    }
    BlackwellIRProgram direct;
    omega_bw_ir_init(&direct);
    for(unsigned v=0;v<160;++v) omega_bw_ir_alloc_vreg(&direct);
    BlackwellIRInsn in={0};in.dst_vreg=in.src1_vreg=in.src2_vreg=in.src3_vreg=in.ureg=-1;
    in.op=BW_IR_MOV_RZ;in.dst_vreg=159;omega_bw_ir_append(&direct,&in);
    in.op=BW_IR_NOP;in.dst_vreg=-1;omega_bw_ir_append(&direct,&in);
    in.op=BW_IR_MOV_RZ;in.dst_vreg=159;omega_bw_ir_append(&direct,&in);
    in.op=BW_IR_EXIT;in.dst_vreg=-1;omega_bw_ir_append(&direct,&in);
    omega_bw_regalloc_solve(&direct);
    if(memcmp(&direct,&b,sizeof b)) {fputs("FAIL zero flag changed default replay\n",stderr);return 3;}
    puts("PASS exact-liveness flag-zero flag-invalid shortened past-end v159 v160");
    return 0;
}
