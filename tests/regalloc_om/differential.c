/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
static BlackwellIRProgram a, b;
static uint8_t ca[BW_MAX_IR_INSNS * 16], cb[BW_MAX_IR_INSNS * 16];
static unsigned checked, refused, encoded;
static void fail(const char *what, unsigned at) { fprintf(stderr,"FAIL %s case=%u\n",what,at); exit(1); }
static int same(void) { return memcmp(&a, &b, sizeof a) == 0; }
static void compare(int ar, int br, unsigned at) {
    if (ar != br) fail("refusal", at);
    if (!same()) fail("program/map/partial-state", at);
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
static int operand(unsigned nv) { return rand() % (nv + 1) - 1; }
static void random_case(unsigned at) {
    omega_bw_ir_init(&a); legacy_omega_bw_ir_init(&b);
    unsigned nv = 1u + (unsigned)rand()%256u, nu=(unsigned)rand()%33u;
    static const unsigned widths[]={1,2,4};
    for (unsigned v=0;v<nv;++v) {
        unsigned w=widths[rand()%3];
        if (alloc_one(&a,w,0,0)!=alloc_one(&b,w,1,0)) fail("alloc",at);
    }
    for (unsigned u=0;u<nu;++u) {
        unsigned w=(rand()%2) ? 1 : 2;
        if (alloc_one(&a,w,0,1)!=alloc_one(&b,w,1,1)) fail("ualloc",at);
    }
    unsigned n=1u+(unsigned)rand()%1024u;
    static const BlackwellIROpcode ops[]={BW_IR_MOV_IMM,BW_IR_IMAD,BW_IR_LDCU,BW_IR_LDCU64,BW_IR_MOV_RZ,BW_IR_IADD3};
    for (unsigned i=0;i<n;++i) {
        BlackwellIRInsn in={0};
        in.op=ops[rand()%6]; in.dst_vreg=operand(nv); in.src1_vreg=operand(nv);
        in.src2_vreg=operand(nv); in.src3_vreg=operand(nv); in.ureg=operand(nu);
        in.imm=(uint32_t)rand(); in.is_uniform=rand()%2;
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
int main(int argc, char **argv) {
    (void)argv; srand(1);
    /* Exhaust every public register-count boundary, including one refusal beyond capacity. */
    omega_bw_ir_init(&a); legacy_omega_bw_ir_init(&b);
    for(unsigned i=0;i<=256;++i) if(alloc_one(&a,1,0,0)!=alloc_one(&b,1,1,0)) fail("capacity",i);
    for(unsigned i=0;i<=32;++i) if(alloc_one(&a,1,0,1)!=alloc_one(&b,1,1,1)) fail("uniform-capacity",i);
    for(unsigned i=0;i<10000;++i) random_case(i);
    if(argc==1) builders();
    b=a; b.insns[0].imm^=1;
    if(same()) fail("negative-record-flip-not-caught",0);
    printf("PASS regalloc differential cases=%u refused=%u encoded=%u record-flip=caught\n",checked,refused,encoded);
    return 0;
}
