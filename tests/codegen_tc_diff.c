/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "omega_blackwell_codegen_tc.h"
#include "omega_blackwell_codegen_tc2.h"
#include "omega_gpu_om_ir.h"
#include "cudapp_native.h"
extern int oracle_omega_blackwell_codegen_tc_probe_prog(int,BlackwellIRProgram *);
extern int oracle_omega_blackwell_codegen_tc_peak_prog(uint32_t,uint32_t,uint32_t,uint32_t,BlackwellIRProgram *);
extern int oracle_omega_blackwell_codegen_tc_bw_prog(uint32_t,uint32_t,uint32_t,BlackwellIRProgram *);
extern int oracle_omega_blackwell_codegen_tcgemm_prog(const OmegaTcGemmSpec *,BlackwellIRProgram *);
extern int oracle_omega_blackwell_codegen_tc2_prog(const OmegaTc2Spec *,BlackwellIRProgram *);
static BlackwellIRProgram a,b;
static unsigned cases,refused;
static int equal(int ca,int cb,const char *label) {
    static uint8_t x[BW_MAX_IR_INSNS*16],y[BW_MAX_IR_INSNS*16]; size_t nx=0,ny=0;
    if ((ca==0)!=(cb==0)) {fprintf(stderr,"%s refusal C=%d Omega=%d counts=%zu/%zu\n",label,ca,cb,a.count,b.count);return 0;}
    if(ca){refused++;return 1;}
    if(a.count!=b.count || memcmp(a.insns,b.insns,a.count*sizeof a.insns[0]) || memcmp(&a.regalloc,&b.regalloc,sizeof a.regalloc)) {
        fprintf(stderr,"%s IR differs %zu/%zu\n",label,a.count,b.count);
        for(size_t i=0;i<a.count && i<b.count;i++) {
            if(memcmp(&a.insns[i],&b.insns[i],sizeof a.insns[0])) {
                fprintf(stderr,"first record %zu op %d/%d ctrl %x/%x imm %x/%x\n",i,a.insns[i].op,b.insns[i].op,a.insns[i].control,b.insns[i].control,a.insns[i].imm,b.insns[i].imm);
                break;
            }
        }
        return 0;
    }
    int ex=omega_bw_encode_program(&a,x,sizeof x,&nx),ey=omega_bw_encode_program(&b,y,sizeof y,&ny);
    if(ex || ey || nx!=ny || memcmp(x,y,nx)){fprintf(stderr,"%s encoded bytes differ or refused (%d/%d)\n",label,ex,ey);return 0;}
    cases++;return 1;
}
static int variant(int v) {
    if(v==0)return equal(oracle_omega_blackwell_codegen_tc_probe_prog(0,&a),omega_blackwell_codegen_tc_probe_prog(0,&b),"probe");
    if(v==1)return equal(oracle_omega_blackwell_codegen_tc_peak_prog(7,8,8,128,&a),omega_blackwell_codegen_tc_peak_prog(7,8,8,128,&b),"peak");
    if(v==2)return equal(oracle_omega_blackwell_codegen_tc_bw_prog(7,2,128,&a),omega_blackwell_codegen_tc_bw_prog(7,2,128,&b),"bandwidth");
    if(v==3) {
        OmegaTcGemmSpec s={2048,4,2,8,0};
        return equal(oracle_omega_blackwell_codegen_tcgemm_prog(&s,&a),omega_blackwell_codegen_tcgemm_prog(&s,&b),"tcgemm");
    }
    OmegaTc2Spec s={2048,8,2,16,1,v==4?0u:1u,0,1536,0};
    return equal(oracle_omega_blackwell_codegen_tc2_prog(&s,&a),omega_blackwell_codegen_tc2_prog(&s,&b),"tc2");
}
int main(int argc,char **argv) {
    if(argc==3 && !strcmp(argv[1],"--variant"))return variant(atoi(argv[2])) ? 0 : 1;
    const uint32_t edges[]={0,1,2,31,32,33,1024,1025,UINT32_MAX};
    for(unsigned i=0;i<sizeof edges/sizeof edges[0];i++) {
        uint32_t e=edges[i];
        if(!equal(oracle_omega_blackwell_codegen_tc_peak_prog(e,8,8,128,&a),omega_blackwell_codegen_tc_peak_prog(e,8,8,128,&b),"peak iters") ||
           !equal(oracle_omega_blackwell_codegen_tc_peak_prog(7,e,8,128,&a),omega_blackwell_codegen_tc_peak_prog(7,e,8,128,&b),"peak chains") ||
           !equal(oracle_omega_blackwell_codegen_tc_peak_prog(7,8,e,128,&a),omega_blackwell_codegen_tc_peak_prog(7,8,e,128,&b),"peak stall") ||
           !equal(oracle_omega_blackwell_codegen_tc_peak_prog(7,8,8,e,&a),omega_blackwell_codegen_tc_peak_prog(7,8,8,e,&b),"peak threads") ||
           !equal(oracle_omega_blackwell_codegen_tc_bw_prog(e,2,128,&a),omega_blackwell_codegen_tc_bw_prog(e,2,128,&b),"bw iters") ||
           !equal(oracle_omega_blackwell_codegen_tc_bw_prog(7,e,128,&a),omega_blackwell_codegen_tc_bw_prog(7,e,128,&b),"bw groups") ||
           !equal(oracle_omega_blackwell_codegen_tc_bw_prog(7,2,e,&a),omega_blackwell_codegen_tc_bw_prog(7,2,e,&b),"bw threads"))return 1;
    }
    if(!equal(oracle_omega_blackwell_codegen_tc_probe_prog(-1,&a),omega_blackwell_codegen_tc_probe_prog(-1,&b),"probe negative") ||
       !equal(oracle_omega_blackwell_codegen_tc_probe_prog(2,&a),omega_blackwell_codegen_tc_probe_prog(2,&b),"probe excess") ||
       omega_blackwell_codegen_tc_probe_prog(0,NULL)!=-1 ||
       omega_blackwell_codegen_tc_peak_prog(1,1,1,32,NULL)!=-1 ||
       omega_blackwell_codegen_tc_bw_prog(1,1,32,NULL)!=-1 ||
       omega_blackwell_codegen_tcgemm_prog(NULL,&b)!=-1 ||
       omega_blackwell_codegen_tc2_prog(NULL,&b)!=-1)return 1;
    for(unsigned i=0;i<sizeof edges/sizeof edges[0];i++) {
        uint32_t e=edges[i];
        OmegaTcGemmSpec g={2048,4,2,8,0},gt;
        OmegaTc2Spec t={2048,8,2,16,1,0,0,1536,0},tt;
#define CHECK_G(field) do { gt=g;gt.field=e;if(!equal(oracle_omega_blackwell_codegen_tcgemm_prog(&gt,&a),omega_blackwell_codegen_tcgemm_prog(&gt,&b),"tcgemm " #field))return 1; } while(0)
#define CHECK_T(field) do { tt=t;tt.field=e;if(!equal(oracle_omega_blackwell_codegen_tc2_prog(&tt,&a),omega_blackwell_codegen_tc2_prog(&tt,&b),"tc2 " #field))return 1; } while(0)
        CHECK_G(k);CHECK_G(rt);CHECK_G(ct);CHECK_G(warps);CHECK_G(mutant);
        CHECK_T(k);CHECK_T(rt);CHECK_T(ct);CHECK_T(warps);CHECK_T(aqn);
        CHECK_T(epi);CHECK_T(direct);CHECK_T(n);CHECK_T(mutant);
#undef CHECK_G
#undef CHECK_T
        if(omega_blackwell_codegen_tcgemm_prog(&g,NULL)!=-1 || omega_blackwell_codegen_tc2_prog(&t,NULL)!=-1)return 1;
    }
    for(int m=0;m<2;m++)if(!equal(oracle_omega_blackwell_codegen_tc_probe_prog(m,&a),omega_blackwell_codegen_tc_probe_prog(m,&b),"probe"))return 1;
    for(uint32_t ch=1;ch<=16;ch++)for(uint32_t st=1;st<=15;st++)for(uint32_t th=32;th<=1024;th+=32)
        if(!equal(oracle_omega_blackwell_codegen_tc_peak_prog(7,ch,st,th,&a),omega_blackwell_codegen_tc_peak_prog(7,ch,st,th,&b),"peak"))return 1;
    const uint32_t its[]={1,2,UINT32_MAX};
    for(unsigned ii=0;ii<3;ii++)for(uint32_t th=32;th<=1024;th+=32)for(uint32_t g=1;g<=2;g++)
        if(!equal(oracle_omega_blackwell_codegen_tc_bw_prog(its[ii],g,th,&a),omega_blackwell_codegen_tc_bw_prog(its[ii],g,th,&b),"bandwidth"))return 1;
    for(uint32_t k=128;k<=32768;k+=128)for(uint32_t rt=1;rt<=4;rt*=2)for(uint32_t ct=1;ct<=4;ct*=2)for(uint32_t w=1;w<=8;w*=2)for(int m=0;m<=4;m++){
        OmegaTcGemmSpec s={k,rt,ct,w,m};if(!omega_tcgemm_spec_ok(&s))continue;
        if(!equal(oracle_omega_blackwell_codegen_tcgemm_prog(&s,&a),omega_blackwell_codegen_tcgemm_prog(&s,&b),"tcgemm")){fprintf(stderr,"k=%u rt=%u ct=%u w=%u m=%d\n",k,rt,ct,w,m);return 1;}
    }
    /* n changes only affine store offsets after the same epilogue. Enumerate every accepted width at k=128;
       all other k values cover both endpoint widths, all shapes, epilogues, direct modes and mutants. */
    for(uint32_t k=128;k<=16384;k+=128)for(uint32_t rt=2;rt<=8;rt*=2)for(uint32_t w=4;w<=16;w*=2)for(uint32_t aq=1;aq<=2;aq++)for(uint32_t ep=0;ep<=1;ep++)for(uint32_t di=0;di<=1;di++)for(int m=0;m<=4;m++){
        uint32_t span=w*16,limit=((1u<<23)-1)/(64*rt),maxn=limit/span*span;
        for(uint32_t n=span;n<=maxn;n+=(k==128 ? span : maxn-span ? maxn-span : span)){
            OmegaTc2Spec s={k,rt,2,w,aq,ep,di,n,m};if(!omega_tc2_spec_ok(&s))continue;
            if(!equal(oracle_omega_blackwell_codegen_tc2_prog(&s,&a),omega_blackwell_codegen_tc2_prog(&s,&b),"tc2")){fprintf(stderr,"k=%u rt=%u w=%u aq=%u ep=%u di=%u n=%u m=%d\n",k,rt,w,aq,ep,di,n,m);return 1;}
        }
    }
    if(omega_blackwell_codegen_tc_probe_prog(0,&b))return 1;
    a=b;b.insns[0].control^=1;
    if(equal(0,0,"negative record flip")){fprintf(stderr,"negative control escaped\n");return 1;}
    puts("CODEGEN TC RECORD NEGATIVE CAUGHT");
    printf("CODEGEN TC DIFF PASS (%u cases, %u matching refusals)\n",cases,refused);return 0;
}
