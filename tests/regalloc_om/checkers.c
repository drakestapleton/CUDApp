/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Compile twice: default Omega wrappers and C_EM/C_RECONV reference. The append
 * double injects success/failure after the pure state change, with no device. */
#include "omega_bw_em.h"
#include "omega_bw_em_q.h"
#include "omega_bw_reconv.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
static BlackwellIRProgram p;
static int fail_append;
#define ins (p.insns)
int omega_bw_ir_append(BlackwellIRProgram *prog, const BlackwellIRInsn *in) {
    if (fail_append) return -1;
    if (prog->count >= 64) abort();
    ins[prog->count++] = *in;
    return 0;
}
int omega_bw_ir_alloc_vreg(BlackwellIRProgram *q) { (void)q; return fail_append ? -1 : 0; }
int omega_bw_ir_alloc_vreg64(BlackwellIRProgram *q) { (void)q; return fail_append ? -1 : 0; }
int omega_bw_ir_alloc_vreg128(BlackwellIRProgram *q) { (void)q; return fail_append ? -1 : 0; }
int omega_bw_ir_alloc_uvreg64(BlackwellIRProgram *q) { (void)q; return fail_append ? -1 : 0; }
static void reset(void) { memset(&p, 0, sizeof p); }
static void emout(const Em *e) {
    printf("e %u %u %d %d %zu", e->pending, e->held, e->err, e->depth, p.count);
    for (int d=0;d<MAX_LOOPS;d++) printf(" %d",e->open_start[d]);
    for (size_t j=0;j<p.count;j++) printf(" %u:%u:%u", ins[j].control, ins[j].imm, ins[j].op);
    puts("");
}
static void rout(int rc, const BwRegions *r) {
    printf("r %d %d %d %d",rc,r->depth,r->peak,r->opened);
    for(int d=0;d<BW_RECONV_MAX_DEPTH;d++) {
        printf(" %d:%d",r->bssy_at[d],r->n_exits[d]);
        for(int j=0;j<BW_RECONV_MAX_EXITS;j++) printf(":%d",r->exits[d][j]);
    }
    for(int j=0;j<8;j++) printf(" %u",ins[j].imm);
    puts("");
}
static void emcases(void) {
    const uint32_t bits[]={0,1,2,3,4,63,64,UINT32_MAX};
    for(unsigned j=0;j<sizeof bits/sizeof bits[0];j++)
    for(int k=-1;k<=11;k++) for(int pred=0;pred<2;pred++) for(int fail=0;fail<2;fail++) {
        reset(); Em e={.p=&p,.pending=bits[j],.held=bits[j]}; fail_append=fail;
        BlackwellIRInsn in=I0(BW_IR_NOP); in.predicate_p0=pred; emit(&e,in,k); emout(&e);
    }
    for(unsigned j=0;j<sizeof bits/sizeof bits[0];j++) for(unsigned sb=0;sb<32;sb++) for(int fail=0;fail<2;fail++) {
        reset(); Em e={.p=&p,.pending=bits[j],.held=~bits[j]}; fail_append=fail;
        ldw_pf(&e,BW_IR_LDG_E,1,2,3,4,sb); emout(&e); consume(&e,sb); emout(&e);
    }
    for(unsigned j=0;j<sizeof bits/sizeof bits[0];j++) for(int fail=0;fail<2;fail++) {
        reset(); Em e={.p=&p,.pending=bits[j]}; fail_append=fail;
        hmma(&e,1,2,3,4,bits[j]); nop(&e,bits[j]); qmma_s(&e,1,2,3,1,4,bits[j]);
        qmma(&e,1,2,3,4,bits[j]); f2e4(&e,1,2,1,3,0); emout(&e);
    }
    for (int fail=0;fail<2;fail++) {
        reset(); Em e={.p=&p}; fail_append=fail;
        printf("alloc %d %d %d %d\n",V(&e),V64(&e),V128(&e),UV64(&e));
        ldcu64(&e,1,2); ldc64(&e,1,2); ldc32(&e,1,2); s2r(&e,1,2);
        movi(&e,1,2); movrz(&e,1); imadi(&e,1,2,0,3,4); imadr(&e,1,2,3,4);
        imadw(&e,1,2,3,4); shr(&e,1,2,0,3); andi(&e,1,2,0,3);
        ldg_wide(&e,BW_IR_LDG_E,1,2,3,4,0); ldg_wide(&e,BW_IR_LDG_E,1,2,3,4,1);
        ldg32(&e,1,2,3,0); ldg32(&e,1,2,3,1); stg(&e,1,2,3,4);
        lds128(&e,1,2,3,0); lds128(&e,1,2,3,1); sts32(&e,1,2); bar_sync(&e);
        fadd(&e,1,2,3); ffma(&e,1,2,3,0,4); shfl_down(&e,1,2,3); isetp_ge_u32(&e,1,2);
        lop3(&e,1,2,3,4,255); iadd(&e,1,2,3); shfl_bfly(&e,1,2,63); f2fp_pack(&e,1,2,3);
        bra_if_p0(&e,1); bra(&e,1); (void)bra_if_p0_fwd(&e);
        patch_bra(&e,0,3); (void)bssy_fwd(&e); bsync(&e); tail(&e); emout(&e);
    }
    fail_append=0;
    for(int d=0;d<=5;d++) { reset(); Em e={.p=&p,.depth=d}; loop_begin(&e); emout(&e); }
    for(int d=-1;d<=4;d++) for(int first=-1;first<=4;first++) for(int last=-1;last<=4;last++) {
        reset(); p.count=4; p.regalloc.num_vregs=1;
        p.regalloc.intervals[0].first_def=first; p.regalloc.intervals[0].last_use=last;
        Em e={.p=&p,.depth=d,.open_start={1,1,1,1}}; loop_end(&e); emout(&e);
        printf("iv %d\n",p.regalloc.intervals[0].last_use);
    }
    /* Include the full uint32 domain above the last representable power of two. */
    const uint32_t logs[]={0,1,2,3,4,7,8,9,0x7fffffff,0x80000000,0x80000001,UINT32_MAX};
    for(unsigned j=0;j<sizeof logs/sizeof logs[0];j++) printf("l %d\n",log2u(logs[j]));
}
static void rcases(void) {
    const int ix[]={-1,0,1,INT_MAX};
    for(int d=0;d<=17;d++) for(unsigned j=0;j<sizeof ix/sizeof ix[0];j++) {
        BwRegions r={.depth=d,.peak=j%2 ? 16 : (d?d-1:0),.opened=2}; reset();
        printf("n %d\n",bw_regions_next_bar(&r));
        rout(bw_regions_begin(&r,ix[j]),&r);
    }
    for(int d=-1;d<=1;d++) for(int n=0;n<=9;n++) for(unsigned j=0;j<sizeof ix/sizeof ix[0];j++) {
        BwRegions r={.depth=d}; r.n_exits[0]=n; reset(); rout(bw_regions_exit(&r,ix[j]),&r);
    }
    for(int mode=0;mode<22;mode++) {
        reset(); p.count=8; BwRegions r={.depth=1,.n_exits={2},.bssy_at={1},.exits={{2,3}}};
        ins[1].op=BW_IR_BSSY; ins[4].op=BW_IR_BSYNC; ins[2].op=ins[3].op=BW_IR_BRA;
        for(int j=0;j<8;j++) ins[j].imm=0xfeed;
        int bs=4; BlackwellIRProgram *q=&p;
        switch(mode) {
        case 0:r.depth=0;break; case 1:q=NULL;break; case 2:bs=-1;break; case 3:bs=8;break;
        case 4:r.bssy_at[0]=-1;break; case 5:r.bssy_at[0]=4;break; case 6:ins[1].op=BW_IR_BRA;break;
        case 7:ins[4].op=BW_IR_BRA;break; case 8:ins[1].bar_reg=1;break; case 9:ins[4].bar_reg=1;break;
        case 10:r.exits[0][0]=1;break; case 11:r.exits[0][0]=4;break; case 12:ins[2].op=BW_IR_NOP;break;
        case 13:r.exits[0][1]=1;break; case 14:r.exits[0][1]=4;break; case 15:ins[3].op=BW_IR_NOP;break;
        case 16:r.n_exits[0]=0;break; case 17:p.count=SIZE_MAX;break;
        case 18:r.depth=-1;break; case 19:bs=INT_MAX;break; case 20:r.bssy_at[0]=INT_MAX;break;
        default:break;
        }
        rout(bw_regions_join(&r,q,bs),&r);
    }
    BwRegions r={.depth=INT_MIN}; printf("n %d\n",bw_regions_next_bar(&r));
    r.depth=INT_MAX; printf("n %d\n",bw_regions_next_bar(&r));
}
int main(int argc,char **argv) {
    if(argc>1 && strcmp(argv[1],"--log-overflow")==0) {
        printf("log-overflow %d\n", log2u(UINT32_MAX)); return 0;
    }
    (void)argv; emcases(); rcases();
    if(argc>1) puts("red-control");
    puts("CHECKERS source partition complete"); return 0;
}
