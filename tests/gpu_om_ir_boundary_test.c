/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_om_ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t boundary(uint64_t *result,uint64_t op,uint64_t a,uint64_t b,uint64_t c,
                         void *iv,uint64_t ni,void *mv,uint64_t nm) {
    uint64_t *ins=iv,*meta=mv;
    (void)a;(void)b;(void)c;
    if(ni!=OMEGA_OM_IR_WORDS||nm!=OMEGA_OM_IR_META)return 1;
    memset(ins,0,ni*sizeof *ins);memset(meta,0,nm*sizeof *meta);
    for(unsigned i=0;i<3;i++)for(unsigned f=1;f<=5;f++)ins[i*8+f]=UINT64_MAX;
    ins[0]=BW_IR_MOV_IMM;ins[1]=0;ins[6]=0x3f800000u;
    ins[8]=op;ins[9]=1;ins[10]=0;
    ins[16]=BW_IR_EXIT;
    meta[0]=3;meta[1]=2;meta[64]=meta[65]=1;
    *result=0;return 0;
}
int main(void) {
    BlackwellIRProgram *p=malloc(sizeof *p);if(!p)return 2;
    int last=omega_gpu_om_ir_replay(boundary,BW_IR_LAST,0,0,0,p,NULL);
    int past=omega_gpu_om_ir_replay(boundary,(uint64_t)BW_IR_LAST+1,0,0,0,p,NULL);
    free(p);
    if(last!=OMEGA_OM_IR_OK||past!=OMEGA_OM_IR_REPLAY) {
        fprintf(stderr,"om-ir-boundary: FAIL (last=%d, past=%d)\n",last,past);
        return 1;
    }
    puts("om-ir-boundary: PASS (BW_IR_LAST accepted; BW_IR_LAST+1 refused)");
    return 0;
}
