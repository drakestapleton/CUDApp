/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen.h"
#include "omega_gpu_om_ir.h"
#include "bw_warp_sim.h"
#include <float.h>
#include <fenv.h>
#include <stdlib.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"f64 check line %d: %s\n",__LINE__,#x); return 0; } } while (0)
static uint64_t bits(double d) { uint64_t u; memcpy(&u,&d,8); return u; }
static double value(uint64_t u) { double d; memcpy(&d,&u,8); return d; }
static BlackwellIRInsn instruction(unsigned op) {
    BlackwellIRInsn in={.op=(BlackwellIROpcode)op,.dst_vreg=0,.src1_vreg=1,.src2_vreg=2,.src3_vreg=3,.ureg=-1};
    if(op==BW_IR_DSETP)in.dst_vreg=-1;
    return in;
}
static int encoding(BlackwellIRProgram *p) {
    /* Independent fields extracted from our small nvcc 13.0.88 / nvdisasm probes.
     * DADD's second source is in word 2, NOT word 1. All register forms,
     * subregister offsets, aliasing, register extremes, and three guard forms. */
    const unsigned maps[][4]={{0,2,4,6},{8,8,8,8},{250,252,0,2},{4,12,18,22}};
    const uint32_t base[]={0x7229,0x7228,0x722b,0x722a,0x7308,0x7308,0x7310,0x7310,0x7312,0x7312,0x7311};
    const uint32_t mod[]={0,0,0,0x03f06000,0x1800,0x1c00,0x201800,0x301000,0x201c00,0x201800,0x30d100};
    uint8_t code[16384];size_t n;
    for(unsigned op=BW_IR_DADD;op<=BW_IR_LAST;op++)for(unsigned m=0;m<4;m++)for(unsigned g=0;g<3;g++)for(unsigned sub=0;sub<2;sub++) {
        if(sub&&(maps[m][0]<2||maps[m][1]<2||maps[m][2]<2||maps[m][3]<2))continue;
        omega_bw_ir_init(p);
        for(unsigned k=0;k<4;k++){CHECK(omega_bw_ir_alloc_vreg64(p)==(int)k);p->regalloc.vreg_to_phys[k]=(int)maps[m][k]-(int)(sub*2);}
        BlackwellIRInsn in=instruction(op);in.control=0x000e2400;
        in.dst_subreg=in.src1_subreg=in.src2_subreg=in.src3_subreg=sub*2;
        in.predicate_p0=g!=0;in.predicate_not=g==2;
        CHECK(!omega_bw_ir_append(p,&in));CHECK(!omega_bw_encode_program(p,code,sizeof code,&n));
        uint32_t w[4];memcpy(w,code,16);
        unsigned d=maps[m][0],a=maps[m][1],b=maps[m][2],c=maps[m][3];
        uint32_t want[4]={base[op-BW_IR_DADD],0,mod[op-BW_IR_DADD],in.control};
        if(op<=BW_IR_DFMA){want[0]|=(d<<16)|(a<<24);if(op==BW_IR_DADD)want[2]=b;else {want[1]=b;if(op==BW_IR_DFMA)want[2]=c;}}
        else if(op==BW_IR_DSETP){want[0]|=a<<24;want[1]=b;}
        else {want[0]|=d<<16;want[1]=a;}
        want[0]=(want[0]&~0xf000u)|((g==0?7u:g==1?0u:8u)<<12);
        CHECK(!memcmp(w,want,16));
        if(op==BW_IR_DSETP)for(unsigned mode=0;mode<16;mode++) {
            p->insns[0].imm=0x03f00000u|(mode<<12);
            CHECK(!omega_bw_encode_program(p,code,sizeof code,&n));memcpy(w,code,16);
            CHECK(w[2]==p->insns[0].imm);
        }
    }
    /* Scalar halves need not be even; MUFU operates explicitly on high words. */
    for(unsigned op=BW_IR_MUFU_RCP64H;op<=BW_IR_S32_F64;op++) {
        omega_bw_ir_init(p);for(unsigned k=0;k<4;k++){CHECK(omega_bw_ir_alloc_vreg64(p)==(int)k);p->regalloc.vreg_to_phys[k]=(int)(k*2+4);}
        BlackwellIRInsn in=instruction(op);in.control=0;unsigned d=4,a=6;
        if(op==BW_IR_MUFU_RCP64H||op==BW_IR_MUFU_RSQ64H){in.dst_subreg=in.src1_subreg=1;d++;a++;}
        else if(op==BW_IR_F64_F32||op==BW_IR_F64_S32||op==BW_IR_F64_U32){in.src1_subreg=1;a++;}
        else {in.dst_subreg=1;d++;}
        CHECK(!omega_bw_ir_append(p,&in));CHECK(!omega_bw_encode_program(p,code,sizeof code,&n));
        uint32_t w[4];memcpy(w,code,16);
        CHECK(w[0]==(base[op-BW_IR_DADD]|(d<<16))&&w[1]==a&&w[2]==mod[op-BW_IR_DADD]&&w[3]==0x000e2400);
    }
    /* The implicit DFMA addend is RZ; P0 is the only exposed predicate output. */
    omega_bw_ir_init(p);for(unsigned k=0;k<4;k++){CHECK(omega_bw_ir_alloc_vreg64(p)==(int)k);p->regalloc.vreg_to_phys[k]=(int)(k*2);}
    BlackwellIRInsn zero=instruction(BW_IR_DFMA);zero.src3_vreg=-1;CHECK(!omega_bw_ir_append(p,&zero));
    CHECK(!omega_bw_encode_program(p,code,sizeof code,&n));uint32_t zw[4];memcpy(zw,code,16);CHECK(zw[2]==255);
    p->insns[0]=instruction(BW_IR_DSETP);p->insns[0].dst_vreg=0;
    CHECK(omega_bw_encode_program(p,code,sizeof code,&n)!=0);
    /* Encoder-side pair validation also protects direct/replayed physical maps. */
    for(unsigned op=BW_IR_DADD;op<=BW_IR_LAST;op++) {
        if(op==BW_IR_MUFU_RCP64H||op==BW_IR_MUFU_RSQ64H)continue;
        for(unsigned operand=0;operand<4;operand++) {
            int pair=op<=BW_IR_DFMA?(operand<3||(op==BW_IR_DFMA&&operand==3)):op==BW_IR_DSETP?(operand==1||operand==2):
                     (op==BW_IR_F64_F32||op==BW_IR_F64_S32||op==BW_IR_F64_U32)?operand==0:operand==1;
            if(!pair)continue;
            for(unsigned invalid=0;invalid<4;invalid++) {
                omega_bw_ir_init(p);for(unsigned k=0;k<4;k++){CHECK(omega_bw_ir_alloc_vreg64(p)==(int)k);p->regalloc.vreg_to_phys[k]=(int)(k*2);}
                const int bad[]={3,254,256,-2};p->regalloc.vreg_to_phys[operand]=bad[invalid];
                BlackwellIRInsn in=instruction(op);CHECK(!omega_bw_ir_append(p,&in));
                CHECK(omega_bw_encode_program(p,code,sizeof code,&n)!=0);
            }
        }
    }
    omega_bw_ir_init(p);for(unsigned k=0;k<4;k++){CHECK(omega_bw_ir_alloc_vreg64(p)==(int)k);p->regalloc.vreg_to_phys[k]=(int)(k*2);}
    BlackwellIRInsn unknown=instruction(BW_IR_LAST+1);CHECK(!omega_bw_ir_append(p,&unknown));
    CHECK(omega_bw_encode_program(p,code,sizeof code,&n)!=0);
    return 1;
}
static int arithmetic(BlackwellIRProgram *p) {
    omega_bw_ir_init(p);for(unsigned i=0;i<4;i++){CHECK(omega_bw_ir_alloc_vreg64(p)==(int)i);p->regalloc.vreg_to_phys[i]=(int)(i*2);}
    WsSim s={.p=p};WsLane l={0};
    const double v[]={0.0,-0.0,1.0,-1.0,DBL_MAX,-DBL_MAX,DBL_MIN,-DBL_MIN,DBL_TRUE_MIN,-DBL_TRUE_MIN,INFINITY,-INFINITY,NAN,0x1.0000000000001p0,0x1.fffffffffffffp-1,0x1p-53};
    for(unsigned i=0;i<sizeof v/sizeof *v;i++)for(unsigned j=0;j<sizeof v/sizeof *v;j++)for(unsigned op=BW_IR_DADD;op<=BW_IR_DSETP;op++) {
        double a=v[i],b=v[j],c=-1.0,want=op==BW_IR_DADD?a+b:op==BW_IR_DMUL?a*b:fma(a,b,c);
        ws_wr64(&l,1,bits(a),p);ws_wr64(&l,2,bits(b),p);ws_wr64(&l,3,bits(c),p);
        BlackwellIRInsn in=instruction(op);
        if(op==BW_IR_DSETP)for(unsigned mode=0;mode<16;mode++) {
            in.imm=0x03f00000u|(mode<<12);ws_lane_exec(&s,&l,&in,0,0,0,0);
            unsigned relation=(isnan(a)||isnan(b))?8u:a<b?1u:a>b?4u:2u;
            CHECK(!s.err&&l.p0==!!(relation&mode));
        } else {
            ws_lane_exec(&s,&l,&in,0,0,0,0);double got=value(ws_rd64(&l,0,p));
            CHECK(!s.err&&(isnan(want)?isnan(got):bits(got)==bits(want)));
        }
    }
    /* A fused cancellation must differ from separately rounded multiply/add. */
    ws_wr64(&l,1,bits(0x1.0000000000001p0),p);ws_wr64(&l,2,bits(0x1.ffffffffffffep-1),p);ws_wr64(&l,3,bits(-1.0),p);
    BlackwellIRInsn in=instruction(BW_IR_DFMA);ws_lane_exec(&s,&l,&in,0,0,0,0);
    CHECK(value(ws_rd64(&l,0,p))==-0x1p-104);
    for(unsigned i=0;i<sizeof v/sizeof *v;i++) {
        ws_wr64(&l,1,bits(v[i]),p);
        in=instruction(BW_IR_F32_F64);ws_lane_exec(&s,&l,&in,0,0,0,0);
        float want=(float)v[i],got=ws_u2f(l.r[0]);CHECK(isnan(want)?isnan(got):ws_f2u(want)==ws_f2u(got));
        in=instruction(BW_IR_S32_F64);ws_lane_exec(&s,&l,&in,0,0,0,0);
        int32_t iwant=isnan(v[i])?INT32_MIN:v[i]>=INT32_MAX?INT32_MAX:v[i]<=INT32_MIN?INT32_MIN:(int32_t)v[i];
        CHECK(l.r[0]==(uint32_t)iwant&&!s.err);
    }
    const double ties[]={0x1.000001p0,0x1.000003p0,-0x1.000001p0,0x1p-150,0x1.8p-149,2147483647.5,-2147483648.5};
    for(unsigned i=0;i<sizeof ties/sizeof *ties;i++) {
        ws_wr64(&l,1,bits(ties[i]),p);in=instruction(BW_IR_F32_F64);ws_lane_exec(&s,&l,&in,0,0,0,0);
        CHECK(l.r[0]==ws_f2u((float)ties[i]));
    }
    const uint32_t ints[]={0,1,0xffffffffu,0x80000000u,0x7fffffffu,0x01000001u};
    for(unsigned i=0;i<sizeof ints/sizeof *ints;i++)for(unsigned op=BW_IR_F64_F32;op<=BW_IR_F64_U32;op++)if(op!=BW_IR_F32_F64) {
        l.r[2]=ints[i];in=instruction(op);ws_lane_exec(&s,&l,&in,0,0,0,0);
        double want=op==BW_IR_F64_F32?(double)ws_u2f(ints[i]):op==BW_IR_F64_S32?(double)(int32_t)ints[i]:(double)ints[i];
        double got=value(ws_rd64(&l,0,p));CHECK(!s.err&&(isnan(want)?isnan(got):bits(got)==bits(want)));
    }
    /* MUFU is a scalar HIGH word op; the low destination word must survive.
     * PTX ftz f64 semantics: ignored low input, signed flush, canonical NaN.
     * We require the structural contract and exact special values, not chip
     * bit identity for the deliberately approximate normal high word. */
    const uint32_t high[]={0,0x80000000u,1,0x80000001u,0x7ff00000u,0xfff00000u,0x7ff00001u,0x3ff00000u,0x40100000u};
    for(unsigned op=BW_IR_MUFU_RCP64H;op<=BW_IR_MUFU_RSQ64H;op++)for(unsigned i=0;i<sizeof high/sizeof *high;i++) {
        l.r[0]=0x12345678;l.r[2]=0xffffffff;l.r[3]=high[i];
        in=instruction(op);in.dst_subreg=in.src1_subreg=1;ws_lane_exec(&s,&l,&in,0,0,0,0);
        CHECK(!s.err&&l.r[0]==0x12345678);
        uint32_t want=0;
        if(i<=3)want=(high[i]&0x80000000u)|0x7ff00000u;
        else if(i==4)want=0;
        else if(i==5)want=op==BW_IR_MUFU_RCP64H?0x80000000u:0x7fffffffu;
        else if(i==6)want=0x7fffffffu;
        else if(i==7)want=0x3ff00000u;
        else want=op==BW_IR_MUFU_RCP64H?0x3fd00000u:0x3fe00000u;
        CHECK(l.r[1]==want);
    }
    /* Exercise full simulator conductor as well as its per-lane path. */
    omega_bw_ir_init(p);in=instruction(BW_IR_EXIT);CHECK(!omega_bw_ir_append(p,&in));s.threads=32;
    CHECK(!ws_run(&s,1,1));return 1;
}
static int pressure(BlackwellIRProgram *p) {
    /* Simultaneous live DADD inputs consume aligned pairs, not scalar GPRs. */
    for(unsigned count=2;count<=128;count++) {
        omega_bw_ir_init(p);for(unsigned i=0;i<count;i++)CHECK(omega_bw_ir_alloc_vreg64(p)==(int)i);
        for(unsigned i=0;i<count;i++){BlackwellIRInsn in=instruction(BW_IR_MOV_IMM);in.dst_vreg=(int)i;in.src1_vreg=in.src2_vreg=in.src3_vreg=-1;CHECK(omega_bw_ir_append(p,&in)>=0);}
        for(unsigned i=1;i<count;i++){BlackwellIRInsn in=instruction(BW_IR_DADD);in.dst_vreg=0;in.src1_vreg=0;in.src2_vreg=(int)i;in.src3_vreg=-1;CHECK(omega_bw_ir_append(p,&in)>=0);}
        int rc=omega_bw_regalloc_solve(p);
        if(rc){CHECK(count==(BW_PHYS_GPR_MAX-BW_PHYS_GPR_START)/2+1);return 1;}
        for(unsigned i=0;i<count;i++){int r=p->regalloc.vreg_to_phys[i];CHECK(r>=0&&!(r&1)&&r+1<BW_PHYS_GPR_MAX);for(unsigned j=0;j<i;j++)CHECK(r!=p->regalloc.vreg_to_phys[j]);}
    }
    return 0;
}
static uint32_t boundary(uint64_t *result,uint64_t op,uint64_t a,uint64_t b,uint64_t c,void *iv,uint64_t ni,void *mv,uint64_t nm) {
    uint64_t *ir=iv,*meta=mv;(void)a;(void)b;(void)c;
    if(ni!=OMEGA_OM_IR_WORDS||nm!=OMEGA_OM_IR_META)return 1;
    memset(ir,0,ni*sizeof *ir);memset(meta,0,nm*sizeof *meta);
    for(unsigned i=0;i<3;i++)for(unsigned f=1;f<=5;f++)ir[i*8+f]=UINT64_MAX;
    ir[0]=BW_IR_MOV_IMM;ir[1]=0;ir[6]=0x3f800000u;
    ir[8]=op;ir[9]=1;ir[10]=0;ir[16]=BW_IR_EXIT;
    meta[0]=3;meta[1]=2;meta[64]=meta[65]=1;*result=0;return 0;
}
int main(void) {
    if(fesetround(FE_TONEAREST))return 2;
    BlackwellIRProgram *p=malloc(sizeof *p);if(!p)return 2;
    int okay=encoding(p)&&arithmetic(p)&&pressure(p);
    if(okay)okay=omega_gpu_om_ir_replay(boundary,BW_IR_LAST,0,0,0,p,NULL)==OMEGA_OM_IR_OK&&omega_gpu_om_ir_replay(boundary,BW_IR_LAST+1,0,0,0,p,NULL)==OMEGA_OM_IR_REPLAY;
    free(p);puts(okay?"codegen-f64: PASS (oracle bytes, IEEE simulation, pair pressure, replay boundary)":"codegen-f64: FAIL");return !okay;
}
