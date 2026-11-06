/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Reuse, rather than copy, the repository's canonical builder corpus. */
#define main regalloc_canonical_fixture_driver
#include "../test_omega_codegen_freestanding.c"
#undef main
void legacy_omega_bw_ir_init(BlackwellIRProgram *);
int legacy_omega_bw_ir_alloc_vreg(BlackwellIRProgram *);
int legacy_omega_bw_ir_alloc_vreg64(BlackwellIRProgram *);
int legacy_omega_bw_ir_alloc_vreg128(BlackwellIRProgram *);
int legacy_omega_bw_ir_alloc_uvreg(BlackwellIRProgram *);
int legacy_omega_bw_ir_alloc_uvreg64(BlackwellIRProgram *);
int legacy_omega_bw_ir_append(BlackwellIRProgram *, const BlackwellIRInsn *);
int legacy_omega_bw_regalloc_solve(BlackwellIRProgram *);
int legacy_omega_bw_encode_program(const BlackwellIRProgram *, uint8_t *, size_t, size_t *);
int legacy_omega_blackwell_codegen_matmul_i32_prog(const OmegaMatMulSpec *, BlackwellIRProgram *);
int legacy_omega_blackwell_codegen_matmul_tensor_prog(const OmegaMatMulSpec *, BlackwellIRProgram *);
int legacy_omega_blackwell_codegen_matmul_tensor_loop_prog(const OmegaMatMulSpec *, uint32_t, int, BlackwellIRProgram *);
int legacy_omega_blackwell_codegen_matmul_frag_prog(const OmegaMatMulSpec *, uint32_t, uint32_t, int, BlackwellIRProgram *);
int legacy_omega_blackwell_codegen_matmul_frag_block_prog(const OmegaMatMulSpec *, uint32_t, uint32_t, uint32_t, uint32_t, int, BlackwellIRProgram *);
uint32_t legacy_omega_bw_gpr_declared(uint32_t);
int legacy_omega_bw_gpr_declared_ok(uint32_t, uint32_t);
int legacy_omega_bw_ir_set_gpr_limit(BlackwellIRProgram *, uint32_t);
void legacy_omega_matmul_frag_block_shape(uint32_t, uint32_t, uint32_t *, uint32_t *);
uint32_t legacy_omega_matmul_frag_block_unroll(uint32_t, uint32_t, uint32_t);
uint32_t legacy_omega_matmul_frag_unroll(uint32_t);
static BlackwellIRProgram a, b;
static uint8_t ca[BW_MAX_IR_INSNS * 16], cb[BW_MAX_IR_INSNS * 16];
static unsigned checked, refused, encoded;
static void fail(const char *what, unsigned at) { fprintf(stderr,"FAIL %s case=%u\n",what,at); exit(1); }
static int same(void) { return memcmp(&a, &b, sizeof a) == 0; }
static void compare(int ar, int br, unsigned at) {
    if (ar != br) fail("refusal", at);
    if (!same()) {
        for (unsigned i=0;i<BW_MAX_IR_INSNS;++i)
            if(memcmp(&a.insns[i],&b.insns[i],sizeof a.insns[i])) {
                fprintf(stderr,"insn=%u op=%d/%d imm=%u/%u dst=%d/%d src=%d,%d,%d/%d,%d,%d ctrl=%x/%x\n",
                        i,a.insns[i].op,b.insns[i].op,a.insns[i].imm,b.insns[i].imm,
                        a.insns[i].dst_vreg,b.insns[i].dst_vreg,a.insns[i].src1_vreg,a.insns[i].src2_vreg,
                        a.insns[i].src3_vreg,b.insns[i].src1_vreg,b.insns[i].src2_vreg,b.insns[i].src3_vreg,
                        a.insns[i].control,b.insns[i].control);break;
            }
        for(unsigned v=0;v<BW_MAX_VREGS;++v)
            if(memcmp(&a.regalloc.intervals[v],&b.regalloc.intervals[v],sizeof a.regalloc.intervals[v])) {
                fprintf(stderr,"interval=%u first=%d/%d last=%d/%d phys=%d/%d\n",v,
                        a.regalloc.intervals[v].first_def,b.regalloc.intervals[v].first_def,
                        a.regalloc.intervals[v].last_use,b.regalloc.intervals[v].last_use,
                        a.regalloc.intervals[v].phys_reg,b.regalloc.intervals[v].phys_reg);break;
            }
        fail("program/map/partial-state",at);
    }
    if (ar != 0) { ++refused; return; }
    size_t na = 0, nb = 0;
    ar = omega_bw_encode_program(&a, ca, sizeof ca, &na);
    br = legacy_omega_bw_encode_program(&b, cb, sizeof cb, &nb);
    if (ar != br) fail("encode-refusal", at);
    if (ar == 0) {
        if (na != nb || memcmp(ca,cb,na)) fail("encoded-bytes",at);
        ++encoded;
    }
}
static int alloc_one(BlackwellIRProgram *p, unsigned w, int old, int uni) {
    if (uni) return old ? (w==2 ? legacy_omega_bw_ir_alloc_uvreg64(p) : legacy_omega_bw_ir_alloc_uvreg(p))
                              : (w==2 ? omega_bw_ir_alloc_uvreg64(p) : omega_bw_ir_alloc_uvreg(p));
    if (old) return w==4 ? legacy_omega_bw_ir_alloc_vreg128(p) : w==2 ? legacy_omega_bw_ir_alloc_vreg64(p) : legacy_omega_bw_ir_alloc_vreg(p);
    return w==4 ? omega_bw_ir_alloc_vreg128(p) : w==2 ? omega_bw_ir_alloc_vreg64(p) : omega_bw_ir_alloc_vreg(p);
}
/* Fixed-seed 64-bit LCG (Knuth MMIX constants), high 31 bits: the same case stream on
 * every libc, unlike rand(). Values span rand()'s [0, 2^31-1] range used below. */
static int rnd(void) {
    static uint64_t s = 1;
    s = s * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)(s >> 33);
}
static int operand(unsigned nv) { return rnd() % (nv + 1) - 1; }
static void random_case(unsigned at) {
    omega_bw_ir_init(&a); legacy_omega_bw_ir_init(&b);
    unsigned nv = 1u + (unsigned)rnd()%256u, nu=(unsigned)rnd()%33u;
    static const unsigned widths[]={1,2,4};
    for (unsigned v=0;v<nv;++v) {
        unsigned w=widths[rnd()%3];
        if (alloc_one(&a,w,0,0)!=alloc_one(&b,w,1,0)) fail("alloc",at);
    }
    for (unsigned u=0;u<nu;++u) {
        unsigned w=(rnd()%2) ? 1 : 2;
        if (alloc_one(&a,w,0,1)!=alloc_one(&b,w,1,1)) fail("ualloc",at);
    }
    unsigned n=1u+(unsigned)rnd()%1024u;
    static const BlackwellIROpcode ops[]={BW_IR_MOV_IMM,BW_IR_IMAD,BW_IR_LDCU,BW_IR_LDCU64,BW_IR_MOV_RZ,BW_IR_IADD3};
    for (unsigned i=0;i<n;++i) {
        BlackwellIRInsn in={0};
        in.op=ops[rnd()%6]; in.dst_vreg=operand(nv); in.src1_vreg=operand(nv);
        in.src2_vreg=operand(nv); in.src3_vreg=operand(nv); in.ureg=operand(nu);
        in.imm=(uint32_t)rnd(); in.is_uniform=rnd()%2;
        if (in.op==BW_IR_LDCU || in.op==BW_IR_LDCU64) in.dst_vreg=operand(nu);
        int ar=omega_bw_ir_append(&a,&in),br=legacy_omega_bw_ir_append(&b,&in);
        if(ar!=br) fail("append",at);
    }
    if (!same()) fail("intervals",at);
    if (at%4==0) a.regalloc.gpr_limit=b.regalloc.gpr_limit=246;
    if (at%8==0) a.regalloc.gpr_limit=b.regalloc.gpr_limit=3;
    compare(omega_bw_regalloc_solve(&a),legacy_omega_bw_regalloc_solve(&b),at);
    ++checked;
}
static void builders(void) {
    for(unsigned k=0;k<=64;++k) for(unsigned n=0;n<=32;n+=8) for(unsigned p=0;p<3;++p) {
        OmegaMatMulSpec s={0}; s.m=16; s.k=k; s.n=n; s.precision=p;
        memset(&a,0,sizeof a); memset(&b,0,sizeof b);
        compare(omega_blackwell_codegen_matmul_i32_prog(&s,&a),legacy_omega_blackwell_codegen_matmul_i32_prog(&s,&b),checked++);
        memset(&a,0,sizeof a); memset(&b,0,sizeof b);
        compare(omega_blackwell_codegen_matmul_tensor_prog(&s,&a),legacy_omega_blackwell_codegen_matmul_tensor_prog(&s,&b),checked++);
        for(unsigned grid=0;grid<=2;++grid) for(int mut=0;mut<=1;++mut) {
            memset(&a,0,sizeof a); memset(&b,0,sizeof b);
            compare(omega_blackwell_codegen_matmul_tensor_loop_prog(&s,grid,mut,&a),legacy_omega_blackwell_codegen_matmul_tensor_loop_prog(&s,grid,mut,&b),checked++);
            for(unsigned u=1;u<=8;u*=2) {
                memset(&a,0,sizeof a); memset(&b,0,sizeof b);
                compare(omega_blackwell_codegen_matmul_frag_prog(&s,grid,u,mut,&a),legacy_omega_blackwell_codegen_matmul_frag_prog(&s,grid,u,mut,&b),checked++);
            }
            for(unsigned rt=1;rt<=8;rt*=2) for(unsigned ct=1;ct<=4;ct*=2) for(unsigned u=1;u<=2;++u) {
                memset(&a,0,sizeof a); memset(&b,0,sizeof b);
                compare(omega_blackwell_codegen_matmul_frag_block_prog(&s,grid,u,rt,ct,mut,&a),legacy_omega_blackwell_codegen_matmul_frag_block_prog(&s,grid,u,rt,ct,mut,&b),checked++);
            }
        }
    }
}
static int fixture_program(const Fixture *f, BlackwellIRProgram *p, int legacy) {
    OmegaMatMulSpec s;
    if (omega_matmul_spec_init(&s, f->m, f->k, f->n, f->prec)) return -1;
    switch (f->kind) {
    case FX_I32: return legacy ? legacy_omega_blackwell_codegen_matmul_i32_prog(&s,p) : omega_blackwell_codegen_matmul_i32_prog(&s,p);
    case FX_TENSOR: return legacy ? legacy_omega_blackwell_codegen_matmul_tensor_prog(&s,p) : omega_blackwell_codegen_matmul_tensor_prog(&s,p);
    case FX_LOOP: return legacy ? legacy_omega_blackwell_codegen_matmul_tensor_loop_prog(&s,f->grid_x,f->mutant,p) : omega_blackwell_codegen_matmul_tensor_loop_prog(&s,f->grid_x,f->mutant,p);
    case FX_FRAG: return legacy ? legacy_omega_blackwell_codegen_matmul_frag_prog(&s,f->grid_x,fx_unroll(f),f->mutant,p) : omega_blackwell_codegen_matmul_frag_prog(&s,f->grid_x,fx_unroll(f),f->mutant,p);
    case FX_BLOCK: return legacy ? legacy_omega_blackwell_codegen_matmul_frag_block_prog(&s,f->grid_x,fx_unroll(f),f->rt,f->ct,f->mutant,p) : omega_blackwell_codegen_matmul_frag_block_prog(&s,f->grid_x,fx_unroll(f),f->rt,f->ct,f->mutant,p);
    }
    return -1;
}
static void canonical_builders(void) {
    for (size_t i=0;i<NFX;++i) {
        memset(&a,0,sizeof a); memset(&b,0,sizeof b);
        int ar=fixture_program(&FX[i],&a,0), br=fixture_program(&FX[i],&b,1);
        compare(ar,br,checked++);
    }
}
static void policies(void) {
    static const uint32_t edge[]={0,1,2,7,8,127,128,129,245,246,247,UINT32_MAX-9,UINT32_MAX-2,UINT32_MAX};
    for (unsigned i=0;i<10000;++i) {
        uint32_t x=i<sizeof edge/sizeof edge[0] ? edge[i] : ((uint32_t)rnd()<<1)^(uint32_t)rnd();
        uint32_t y=((uint32_t)rnd()<<1)^(uint32_t)rnd(), z=(uint32_t)rnd();
        if(omega_bw_gpr_declared(x)!=legacy_omega_bw_gpr_declared(x)) fail("declared-gpr",i);
        if(omega_bw_gpr_declared_ok(x,y)!=legacy_omega_bw_gpr_declared_ok(x,y)) fail("declared-gpr-refusal",i);
        omega_bw_ir_init(&a); legacy_omega_bw_ir_init(&b);
        if(omega_bw_ir_set_gpr_limit(&a,x)!=legacy_omega_bw_ir_set_gpr_limit(&b,x)||!same()) fail("gpr-limit",i);
        uint32_t ar,ac,br,bc;
        omega_matmul_frag_block_shape(x,y,&ar,&ac);
        legacy_omega_matmul_frag_block_shape(x,y,&br,&bc);
        if(ar!=br||ac!=bc) fail("block-shape",i);
        if(omega_matmul_frag_block_unroll(x,y,z)!=legacy_omega_matmul_frag_block_unroll(x,y,z)) fail("block-unroll",i);
        if(omega_matmul_frag_unroll(x)!=legacy_omega_matmul_frag_unroll(x)) fail("frag-unroll",i);
    }
    puts("PASS scalar register and matmul policies cases=10000 including uint32 wrap boundaries");
}
int main(int argc, char **argv) {
    (void)argv;
    /* Exhaust every public register-count boundary, including one refusal beyond capacity. */
    omega_bw_ir_init(&a); legacy_omega_bw_ir_init(&b);
    for(unsigned i=0;i<=256;++i) if(alloc_one(&a,1,0,0)!=alloc_one(&b,1,1,0)) fail("capacity",i);
    for(unsigned i=0;i<=32;++i) if(alloc_one(&a,1,0,1)!=alloc_one(&b,1,1,1)) fail("uniform-capacity",i);
    for(unsigned i=0;i<10000;++i) random_case(i);
    if(argc==1) { builders(); canonical_builders(); }
    policies();
    b=a; b.insns[0].imm^=1;
    if(same()) fail("negative-record-flip-not-caught",0);
    printf("PASS regalloc differential cases=%u refused=%u encoded=%u record-flip=caught\n",checked,refused,encoded);
    return 0;
}
