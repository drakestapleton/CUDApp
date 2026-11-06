/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdio.h>
#include "omega_cubin.h"
#include <string.h>
#define CUDAPP_MIGRATION_PENDING_C_CODEGEN_QMMA 1
#define omega_qmma_is_mark oracle_qmma_is_mark
#define omega_qmma_patch_code oracle_qmma_patch_code
#define omega_qmma_patch_selftest oracle_qmma_patch_selftest
#define omega_blackwell_codegen_qmma_probe_prog oracle_qmma_probe
#define omega_blackwell_codegen_qmma_peak_prog oracle_qmma_peak
#define omega_blackwell_codegen_f2e4_probe_prog oracle_f2e4_probe
#include "../src/omega_blackwell_codegen_qmma.c"
#undef omega_blackwell_codegen_qmma_probe_prog
#undef omega_blackwell_codegen_qmma_peak_prog
#undef omega_blackwell_codegen_f2e4_probe_prog
extern int omega_blackwell_codegen_qmma_probe_prog(int, uint32_t, BlackwellIRProgram *);
extern int omega_blackwell_codegen_qmma_peak_prog(uint32_t, uint32_t, uint32_t, uint32_t, BlackwellIRProgram *);
extern int omega_blackwell_codegen_f2e4_probe_prog(BlackwellIRProgram *);
#undef omega_qmma_patch_code
extern int omega_qmma_patch_code(const BlackwellIRProgram *, uint8_t *, size_t);
static uint32_t mutant_unit(uint64_t *result,uint64_t k,uint64_t x,uint64_t y,uint64_t z,void *ins,uint64_t ni,void *meta,uint64_t nm) {
    uint32_t rc=omega_cg_qmma(result,k,x,y,z,ins,ni,meta,nm);
    if(!rc && !*result) ((uint64_t *)ins)[6]^=UINT64_C(1)<<32;
    return rc;
}
static BlackwellIRProgram a,b;
static unsigned cases;
static int same(int ca,int cb,const char *label) {
    static uint8_t x[BW_MAX_IR_INSNS*16],y[BW_MAX_IR_INSNS*16];
    size_t nx=0,ny=0;
    if ((ca==0)!=(cb==0)) {fprintf(stderr,"%s refusal C=%d Omega=%d\n",label,ca,cb);return 0;}
    if(ca) return 1;
    if(omega_bw_encode_program(&a,x,sizeof x,&nx)||omega_bw_encode_program(&b,y,sizeof y,&ny)||nx!=ny||memcmp(x,y,nx)) {
        fprintf(stderr,"%s differs: count %zu/%zu bytes %zu/%zu\n",label,a.count,b.count,nx,ny);
        for(size_t i=0;i<a.count && i<b.count;i++) if(memcmp(x+i*16,y+i*16,16)){fprintf(stderr,"first instruction %zu op %d/%d ctrl %x/%x\n",i,a.insns[i].op,b.insns[i].op,a.insns[i].control,b.insns[i].control);break;}
        return 0;
    }
    int px=oracle_qmma_patch_code(&a,x,nx),py=omega_qmma_patch_code(&b,y,ny);
    if(px!=py || memcmp(x,y,nx)){fprintf(stderr,"%s patch differs C=%d Omega=%d\n",label,px,py);return 0;}
    uint8_t *cx=NULL,*cy=NULL;size_t lx=0,ly=0;char error[128];
    const OmegaCubinParam params[]={{OMEGA_CUBIN_PARAM_BASE,8,1},{OMEGA_CUBIN_PARAM_BASE+8,8,1}};
    int ex=omega_cubin_pack("qmma",x,nx,params,2,OMEGA_CUBIN_PARAM_BASE+16,omega_bw_gpr_declared(a.regalloc.peak_gpr_usage),0,&cx,&lx,error,sizeof error);
    int ey=omega_cubin_pack("qmma",y,ny,params,2,OMEGA_CUBIN_PARAM_BASE+16,omega_bw_gpr_declared(b.regalloc.peak_gpr_usage),0,&cy,&ly,error,sizeof error);
    int equal=!ex && !ey && lx==ly && !memcmp(cx,cy,lx);
    omega_cubin_free(cx);omega_cubin_free(cy);
    if(!equal){fprintf(stderr,"%s cubin differs (%d/%d)\n",label,ex,ey);return 0;}
    cases++; return 1;
}
int main(void) {
    for(int mut=0;mut<2;mut++) for(unsigned wait=0;wait<=16;wait++) {
        int ca=oracle_qmma_probe(mut,wait,&a),cb=omega_blackwell_codegen_qmma_probe_prog(mut,wait,&b);
        if(!same(ca,cb,"probe"))return 1;
    }
    for(unsigned chains=1;chains<=16;chains++) for(unsigned stall=1;stall<=15;stall++) for(unsigned threads=32;threads<=1024;threads+=32) {
        int ca=oracle_qmma_peak(7,chains,stall,threads,&a),cb=omega_blackwell_codegen_qmma_peak_prog(7,chains,stall,threads,&b);
        if(!same(ca,cb,"peak")){fprintf(stderr,"chains=%u stall=%u threads=%u\n",chains,stall,threads);return 1;}
    }
    for(unsigned bit=0;bit<32;bit++) {
        uint32_t iterations=UINT32_C(1)<<bit;
        if(!same(oracle_qmma_peak(iterations,1,1,32,&a),omega_blackwell_codegen_qmma_peak_prog(iterations,1,1,32,&b),"iteration bit"))return 1;
    }
    if(!same(oracle_qmma_peak(UINT32_MAX,16,15,1024,&a),omega_blackwell_codegen_qmma_peak_prog(UINT32_MAX,16,15,1024,&b),"iteration max"))return 1;
    if(!same(oracle_f2e4_probe(&a),omega_blackwell_codegen_f2e4_probe_prog(&b),"f2e4"))return 1;
    if(oracle_qmma_probe(0,0,&a) || omega_gpu_om_ir_replay(mutant_unit,0,0,0,0,&b,NULL))return 1;
    if(same(0,0,"deliberate mutant")){fprintf(stderr,"QMMA mutant escaped\n");return 1;}
    printf("CODEGEN QMMA MUTANT CAUGHT\n");
    printf("CODEGEN QMMA DIFF PASS (%u cases)\n",cases);
    return 0;
}
