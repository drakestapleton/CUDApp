/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdio.h>
#include <string.h>
#include "omega_cubin.h"
#define CUDAPP_MIGRATION_PENDING_C_CODEGEN_TGEMM 1
#define omega_tgemm_spec_ok oracle_tgemm_spec_ok
#define omega_tgemm_shared_bytes oracle_tgemm_shared_bytes
#define omega_tgemm_span oracle_tgemm_span
#define omega_tgemm_bm oracle_tgemm_bm
#define omega_blackwell_codegen_tgemm_prog oracle_tgemm_prog
#include "../src/omega_blackwell_codegen_tgemm.c"
#undef omega_tgemm_spec_ok
#undef omega_tgemm_shared_bytes
#undef omega_tgemm_span
#undef omega_tgemm_bm
#undef omega_blackwell_codegen_tgemm_prog
extern int omega_tgemm_spec_ok(const OmegaTgemmSpec *);
extern uint32_t omega_tgemm_shared_bytes(const OmegaTgemmSpec *);
extern uint32_t omega_tgemm_span(const OmegaTgemmSpec *);
extern uint32_t omega_tgemm_bm(const OmegaTgemmSpec *);
extern int omega_blackwell_codegen_tgemm_prog(const OmegaTgemmSpec *, BlackwellIRProgram *);
static BlackwellIRProgram a,b;
static unsigned long long cases;
static uint32_t mutant_unit(uint64_t *result,uint64_t k,uint64_t n,uint64_t shape,uint64_t z,void *ins,uint64_t ni,void *meta,uint64_t nm) {
    uint32_t rc=omega_cg_tgemm(result,k,n,shape,z,ins,ni,meta,nm);
    if (!rc && !*result) ((uint64_t *)ins)[6]^=UINT64_C(1)<<32;
    return rc;
}
static int equal_program(int ca,int cb,int pack) {
    if ((ca==0)!=(cb==0)) {fprintf(stderr,"TGEMM refusal C=%d Omega=%d\n",ca,cb);return 0;}
    if (ca) return 1;
    /* Equal complete IR and allocator state are stronger than equal encoded bytes:
     * both branches feed the same deterministic encoder and cubin packer. */
    int equal=a.count==b.count && !memcmp(a.insns,b.insns,a.count*sizeof a.insns[0]) &&
              !memcmp(&a.regalloc,&b.regalloc,sizeof a.regalloc);
    if (!equal || pack) {
        static uint8_t x[BW_MAX_IR_INSNS*16],y[BW_MAX_IR_INSNS*16];
        size_t nx=0,ny=0;
        int ex=omega_bw_encode_program(&a,x,sizeof x,&nx),ey=omega_bw_encode_program(&b,y,sizeof y,&ny);
        if (!equal || ex || ey || nx!=ny || memcmp(x,y,nx)) {
            fprintf(stderr,"TGEMM mismatch count=%zu/%zu encode=%d/%d bytes=%zu/%zu\n",a.count,b.count,ex,ey,nx,ny);
            for(size_t i=0;i<a.count && i<b.count;i++) if(memcmp(&a.insns[i],&b.insns[i],sizeof a.insns[i])) {
                const BlackwellIRInsn *u=&a.insns[i],*v=&b.insns[i];
                fprintf(stderr,"first IR %zu op=%d/%d d=%d/%d a=%d/%d b=%d/%d c=%d/%d imm=%x/%x ctrl=%x/%x subs=%u,%u,%u,%u / %u,%u,%u,%u\n",i,u->op,v->op,u->dst_vreg,v->dst_vreg,u->src1_vreg,v->src1_vreg,u->src2_vreg,v->src2_vreg,u->src3_vreg,v->src3_vreg,u->imm,v->imm,u->control,v->control,u->dst_subreg,u->src1_subreg,u->src2_subreg,u->src3_subreg,v->dst_subreg,v->src1_subreg,v->src2_subreg,v->src3_subreg);break;
            }
            return 0;
        }
        uint8_t *cx=NULL,*cy=NULL;size_t lx=0,ly=0;char error[128];
        const OmegaCubinParam params[]={{OMEGA_CUBIN_PARAM_BASE,8,1},{OMEGA_CUBIN_PARAM_BASE+8,8,1},{OMEGA_CUBIN_PARAM_BASE+16,8,1}};
        ex=omega_cubin_pack("tgemm",x,nx,params,3,OMEGA_CUBIN_PARAM_BASE+24,omega_bw_gpr_declared(a.regalloc.peak_gpr_usage),0,&cx,&lx,error,sizeof error);
        ey=omega_cubin_pack("tgemm",y,ny,params,3,OMEGA_CUBIN_PARAM_BASE+24,omega_bw_gpr_declared(b.regalloc.peak_gpr_usage),0,&cy,&ly,error,sizeof error);
        equal=!ex && !ey && lx==ly && !memcmp(cx,cy,lx);
        omega_cubin_free(cx);omega_cubin_free(cy);
        if(!equal){fprintf(stderr,"TGEMM cubin mismatch %d/%d\n",ex,ey);return 0;}
    }
    cases++;return 1;
}
static int check(OmegaTgemmSpec *s,int pack) {
    if (oracle_tgemm_spec_ok(s)!=omega_tgemm_spec_ok(s) ||
        oracle_tgemm_shared_bytes(s)!=omega_tgemm_shared_bytes(s) ||
        oracle_tgemm_span(s)!=omega_tgemm_span(s) || oracle_tgemm_bm(s)!=omega_tgemm_bm(s)) {
        fprintf(stderr,"TGEMM spec/query mismatch K=%u N=%u RT=%u CT=%u W=%u KC=%u AQN=%u layout=%u epi=%u mutant=%d ok=%d/%d shared=%u/%u span=%u/%u bm=%u/%u\n",s->k,s->n,s->rt,s->ct,s->warps,s->kc,s->aqn,s->layout,s->epi,s->mutant,oracle_tgemm_spec_ok(s),omega_tgemm_spec_ok(s),oracle_tgemm_shared_bytes(s),omega_tgemm_shared_bytes(s),oracle_tgemm_span(s),omega_tgemm_span(s),oracle_tgemm_bm(s),omega_tgemm_bm(s));return 0;
    }
    int ca=oracle_tgemm_prog(s,&a),cb=omega_blackwell_codegen_tgemm_prog(s,&b);
    if (!equal_program(ca,cb,pack)) {
        fprintf(stderr,"K=%u N=%u RT=%u W=%u AQN=%u mutant=%d\n",s->k,s->n,s->rt,s->warps,s->aqn,s->mutant);return 0;
    }
    return 1;
}
static int refusals(void) {
    const OmegaTgemmSpec good={128,128,2,2,8,32,1,0,0,0};
    if (omega_tgemm_spec_ok(NULL) || oracle_tgemm_spec_ok(NULL) ||
        omega_tgemm_shared_bytes(NULL) || omega_tgemm_span(NULL) || omega_tgemm_bm(NULL) ||
        !omega_blackwell_codegen_tgemm_prog(&good,NULL) || !oracle_tgemm_prog(&good,NULL) ||
        !omega_blackwell_codegen_tgemm_prog(NULL,&b) || !oracle_tgemm_prog(NULL,&a)) return 0;
    for(unsigned field=0;field<10;field++) for(unsigned choice=0;choice<4;choice++) {
        OmegaTgemmSpec s=good;
        const uint32_t values[]={0,1,UINT32_MAX,65537};
        switch(field) {
        case 0:s.k=values[choice];break;case 1:s.n=values[choice];break;
        case 2:s.rt=values[choice];break;case 3:s.ct=values[choice];break;
        case 4:s.warps=values[choice];break;case 5:s.kc=values[choice];break;
        case 6:s.aqn=values[choice];break;case 7:s.layout=values[choice];break;
        case 8:s.epi=values[choice];break;case 9:s.mutant=(int)values[choice];break;
        }
        if(!check(&s,0))return 0;
    }
    return 1;
}
int main(int argc,char **argv) {
    int exhaustive=argc==2 && !strcmp(argv[1],"--exhaustive");
    if(argc>2 || (argc==2 && !exhaustive))return 2;
    if(!refusals()){fprintf(stderr,"TGEMM negative controls failed\n");return 1;}
    for(unsigned rt=2;rt<=8;rt*=2) for(unsigned w=8;w<=16;w*=2)
    for(unsigned aqn=1;aqn<=2;aqn++) for(int mut=0;mut<=5;mut++) {
        OmegaTgemmSpec s={128,w*16,rt,2,w,32,aqn,0,0,mut};
        if(!check(&s,1))return 1;
        for(unsigned k=128;k<=65536;k+=128) {
            s.k=k;
            if(exhaustive) {
                for(unsigned n=w*16;n<131072;n+=w*16) {s.n=n;if(!check(&s,0))return 1;}
            } else {s.n=w*16;if(!check(&s,0))return 1;}
        }
        if(!exhaustive)for(unsigned n=w*16;n<131072;n+=w*16) {s.k=128;s.n=n;if(!check(&s,0))return 1;}
    }
    OmegaTgemmSpec s={128,128,2,2,8,32,1,0,0,0};
    uint64_t shape=s.rt|((uint64_t)s.warps<<8)|((uint64_t)s.aqn<<16);
    if(oracle_tgemm_prog(&s,&a) || omega_gpu_om_ir_replay(mutant_unit,s.k,s.n,shape,0,&b,NULL))return 1;
    if(equal_program(0,0,0)){fprintf(stderr,"TGEMM mutant escaped\n");return 1;}
    printf("CODEGEN TGEMM MUTANT CAUGHT\n");
    printf("CODEGEN TGEMM %s DIFF PASS (%llu accepted cases)\n",exhaustive?"EXHAUSTIVE":"AXES",cases);
    return 0;
}
