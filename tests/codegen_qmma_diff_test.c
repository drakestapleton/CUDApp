/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdio.h>
#include "omega_cubin.h"
#include <string.h>
#define CUDAPP_MIGRATION_PENDING_C_CODEGEN_QMMA 1
#define CUDAPP_MIGRATION_PENDING_C_QMMA_PROBE_REST 1
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
static uint8_t x[BW_MAX_IR_INSNS*16],y[BW_MAX_IR_INSNS*16];
static size_t nx,ny;
static unsigned cases;
static int same(int ca,int cb,const char *label) {
    nx=ny=0;
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
static int refusals(void) {
    static const uint32_t invalid[][4] = {
        {0,1,1,32}, {1,0,1,32}, {1,17,1,32},
        {1,1,0,32}, {1,1,16,32}, {1,1,1,0},
        {1,1,1,31}, {1,1,1,33}, {1,1,1,1056},
        {1,UINT32_MAX,1,32}, {1,1,UINT32_MAX,32},
        {1,1,1,UINT32_MAX}
    };
    for (size_t i=0;i<sizeof invalid/sizeof invalid[0];i++) {
        const uint32_t *v=invalid[i];
        if (!oracle_qmma_peak(v[0],v[1],v[2],v[3],&a) ||
            !omega_blackwell_codegen_qmma_peak_prog(v[0],v[1],v[2],v[3],&b)) return 0;
    }
    if (!oracle_qmma_probe(-1,0,&a) || !omega_blackwell_codegen_qmma_probe_prog(-1,0,&b) ||
        !oracle_qmma_probe(2,0,&a) || !omega_blackwell_codegen_qmma_probe_prog(2,0,&b) ||
        !oracle_qmma_probe(0,17,&a) || !omega_blackwell_codegen_qmma_probe_prog(0,17,&b) ||
        !oracle_qmma_probe(0,0,NULL) || !omega_blackwell_codegen_qmma_probe_prog(0,0,NULL) ||
        !oracle_qmma_peak(1,1,1,32,NULL) || !omega_blackwell_codegen_qmma_peak_prog(1,1,1,32,NULL) ||
        !oracle_f2e4_probe(NULL) || !omega_blackwell_codegen_f2e4_probe_prog(NULL)) return 0;
    return 1;
}
static int patch_refusals(void) {
    static uint8_t x[BW_MAX_IR_INSNS*16],y[BW_MAX_IR_INSNS*16];
    const unsigned corrupt[]={0,5,11};
    for (int kind=0;kind<2;kind++) {
        if (kind ? oracle_f2e4_probe(&a) : oracle_qmma_probe(0,0,&a)) return 0;
        size_t len=0,mark=a.count;
        if (omega_bw_encode_program(&a,x,sizeof x,&len)) return 0;
        for(size_t i=0;i<a.count;i++)
            if (a.insns[i].imm==(kind ? OMEGA_QMMA_MARK_F2E4 : OMEGA_QMMA_MARK)) {mark=i;break;}
        if(mark==a.count) return 0;
        for(size_t q=0;q<sizeof corrupt/sizeof corrupt[0];q++) {
            memcpy(y,x,len);
            y[mark*16+corrupt[q]]^=1;
            memcpy(x,y,len);
            if (oracle_qmma_patch_code(&a,x,len)!=-1 || omega_qmma_patch_code(&a,y,len)!=-1) return 0;
            if (omega_bw_encode_program(&a,x,sizeof x,&len)) return 0;
        }
        if (oracle_qmma_patch_code(&a,x,a.count*16-1)!=-1 || omega_qmma_patch_code(&a,x,a.count*16-1)!=-1) return 0;
    }
    return oracle_qmma_patch_code(NULL,x,0)==-1 && omega_qmma_patch_code(NULL,x,0)==-1 &&
           oracle_qmma_patch_code(&a,NULL,0)==-1 && omega_qmma_patch_code(&a,NULL,0)==-1;
}
/* MOV_IMM stores its imm32 in word 1; see omega_blackwell_codegen.c.
 * Locate the niter=7 MOVI, rather than assuming its instruction index. */
static int iteration_field(const BlackwellIRProgram *p,size_t len,size_t *field) {
    size_t found=p->count;
    for(size_t i=0;i<p->count;i++) if(p->insns[i].op==BW_IR_MOV_IMM && p->insns[i].imm==7) {
        if(found!=p->count) return 0;
        found=i;
    }
    if(found==p->count || found*16+2*sizeof(uint32_t)>len) return 0;
    *field=found*16+sizeof(uint32_t);
    return 1;
}
static int iteration_only(const uint8_t *code,size_t len,const uint8_t *base,size_t base_len,
                          size_t field,uint32_t iterations) {
    if(len!=base_len || field+sizeof(uint32_t)>len) return 0;
    uint32_t decoded=0;
    for(unsigned i=0;i<sizeof(uint32_t);i++) decoded|=(uint32_t)code[field+i]<<(8*i);
    return decoded==iterations && !memcmp(code,base,field) &&
           !memcmp(code+field+sizeof(uint32_t),base+field+sizeof(uint32_t),len-field-sizeof(uint32_t));
}
static void add_iterations(uint32_t *values,size_t *count,uint32_t value) {
    for(size_t i=0;i<*count;i++) if(values[i]==value) return;
    values[(*count)++]=value;
}
static int peak_structure(void) {
    uint32_t values[3*32+3];size_t count=0;
    add_iterations(values,&count,1);
    add_iterations(values,&count,UINT32_MAX);
    add_iterations(values,&count,7);
    for(unsigned bit=0;bit<32;bit++) {
        uint32_t value=UINT32_C(1)<<bit;
        add_iterations(values,&count,value);
        add_iterations(values,&count,value-1);
        add_iterations(values,&count,value+1);
    }
    static uint8_t base_c[BW_MAX_IR_INSNS*16],base_om[BW_MAX_IR_INSNS*16];
    for(unsigned chains=1;chains<=16;chains++) for(unsigned stall=1;stall<=15;stall++)
        for(unsigned threads=32;threads<=1024;threads+=32) {
            int ca=oracle_qmma_peak(7,chains,stall,threads,&a);
            int cb=omega_blackwell_codegen_qmma_peak_prog(7,chains,stall,threads,&b);
            size_t field_c=0,field_om=0,base_len=0;
            if(!same(ca,cb,"peak baseline") || ca || !iteration_field(&a,nx,&field_c) ||
               !iteration_field(&b,ny,&field_om) || field_c!=field_om) return 0;
            base_len=nx;memcpy(base_c,x,nx);memcpy(base_om,y,ny);
            for(size_t v=0;v<count;v++) {
                uint32_t iterations=values[v];
                if(iterations!=7) {
                    ca=oracle_qmma_peak(iterations,chains,stall,threads,&a);
                    cb=omega_blackwell_codegen_qmma_peak_prog(iterations,chains,stall,threads,&b);
                    if(!same(ca,cb,"peak iteration")) return 0;
                }
                if(!iterations) {
                    if(!ca || !cb) return 0;
                    continue;
                }
                /* The baseline is already checked, and the zero case has no output. */
                const uint8_t *cx=iterations==7?base_c:x,*cy=iterations==7?base_om:y;
                size_t lx=iterations==7?base_len:nx,ly=iterations==7?base_len:ny;
                if(!iteration_only(cx,lx,base_c,base_len,field_c,iterations) ||
                   !iteration_only(cy,ly,base_om,base_len,field_om,iterations)) {
                    fprintf(stderr,"QMMA niter confinement failed: chains=%u stall=%u threads=%u niter=%u\n",
                            chains,stall,threads,iterations);
                    return 0;
                }
            }
        }
    printf("CODEGEN QMMA STRUCTURE EXHAUSTIVE NITER BIT-BASIS PASS (%zu values, zero refused)\n",count);
    return 1;
}
static uint32_t dependency_mutant_unit(uint64_t *result,uint64_t k,uint64_t iterations,uint64_t chains,
                                     uint64_t shape,void *ins,uint64_t ni,void *meta,uint64_t nm) {
    uint32_t rc=omega_cg_qmma(result,k,iterations,chains,shape,ins,ni,meta,nm);
    /* A valid but wrong extra dependence: niter parity changes a prologue stall. */
    if(!rc && !*result && k==1 && (iterations&1)) ((uint64_t *)ins)[6]^=UINT64_C(1)<<32;
    return rc;
}
static int dependency_mutant(void) {
    static uint8_t base[BW_MAX_IR_INSNS*16],changed[BW_MAX_IR_INSNS*16];
    const uint64_t shape=UINT64_C(1)|(UINT64_C(32)<<32);
    size_t base_len=0,changed_len=0,field=0;
    if(omega_gpu_om_ir_replay(dependency_mutant_unit,1,7,1,shape,&b,NULL) ||
       omega_bw_encode_program(&b,base,sizeof base,&base_len) ||
       omega_qmma_patch_code(&b,base,base_len)<0 || !iteration_field(&b,base_len,&field) ||
       omega_gpu_om_ir_replay(dependency_mutant_unit,1,2,1,shape,&b,NULL) ||
       omega_bw_encode_program(&b,changed,sizeof changed,&changed_len) ||
       omega_qmma_patch_code(&b,changed,changed_len)<0) return 0;
    if(iteration_only(changed,changed_len,base,base_len,field,2)) {
        fprintf(stderr,"QMMA niter-dependent non-immediate mutant escaped confinement check\n");
        return 0;
    }
    printf("CODEGEN QMMA NITER DEPENDENCE MUTANT CAUGHT\n");
    return 1;
}
int main(void) {
    if(!refusals() || !patch_refusals()){fprintf(stderr,"QMMA refusal differential failed\n");return 1;}
    for(int mut=0;mut<2;mut++) for(unsigned wait=0;wait<=16;wait++) {
        int ca=oracle_qmma_probe(mut,wait,&a),cb=omega_blackwell_codegen_qmma_probe_prog(mut,wait,&b);
        if(!same(ca,cb,"probe"))return 1;
    }
    if(!peak_structure() || !dependency_mutant()) return 1;
    if(!same(oracle_f2e4_probe(&a),omega_blackwell_codegen_f2e4_probe_prog(&b),"f2e4"))return 1;
    if(oracle_qmma_probe(0,0,&a) || omega_gpu_om_ir_replay(mutant_unit,0,0,0,0,&b,NULL))return 1;
    if(same(0,0,"deliberate mutant")){fprintf(stderr,"QMMA mutant escaped\n");return 1;}
    printf("CODEGEN QMMA MUTANT CAUGHT\n");
    printf("CODEGEN QMMA DIFF PASS (%u cases)\n",cases);
    return 0;
}
