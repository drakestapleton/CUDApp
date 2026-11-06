/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Independent migration oracle: retain the original generator in this TU only. */
#define CUDAPP_MIGRATION_PENDING_C_API_KGEN 1
#include "../src/omega_gpu_attention_api.c"
#include "omega_gpu_om_ir.h"
#include <stdio.h>
#include <stdlib.h>

extern uint32_t omega_attention_ir(uint64_t *,uint64_t,uint64_t,uint64_t,uint64_t,void *,uint64_t,void *,uint64_t);
static unsigned cases;
static int compare(const KSpec *k, int red) {
    BlackwellIRProgram *cp=calloc(1,sizeof *cp), *op=calloc(1,sizeof *op);
    uint64_t words[8192]={0},meta[512]={0},res=99,err=0;
    uint64_t kind=(uint64_t)k->f32|((uint64_t)k->general<<1)|((uint64_t)k->divergent<<2)|((uint64_t)k->mutant<<8)|((uint64_t)(k->split+1)<<40);
    if(!cp||!op) exit(2);
    omega_bw_ir_init(cp);
    Em e={0};e.p=cp;e.rg_mutant=k->divergent==2;
    if(k->general)gen_attention_general(&e,k);else gen_attention(&e,k);
    uint32_t st=omega_attention_ir(&res,kind,k->hd,k->log2bs,k->log2gqa,words,8192,meta,512);
    if(st||res||meta[4]||meta[6]||meta[7]||e.err||e.depth||e.rg.depth||meta[0]!=cp->count||meta[1]!=(uint64_t)cp->regalloc.num_vregs||meta[2]!=(uint64_t)cp->regalloc.num_uvregs||meta[5]!=(uint64_t)e.n_loops)goto bad;
    if(red)words[6]^=1;
    for(size_t i=0;i<cp->count;i++) {
        BlackwellIRInsn *in=&cp->insns[i];uint64_t *w=words+8*i;
        uint64_t flags=in->is_uniform|((uint64_t)in->predicate_p0<<1)|((uint64_t)in->predicate_not<<2)|((uint64_t)in->dst_subreg<<16)|((uint64_t)in->src1_subreg<<24)|((uint64_t)in->src2_subreg<<32)|((uint64_t)in->src3_subreg<<40)|((uint64_t)in->bar_reg<<48);
        if(w[0]!=(uint64_t)in->op||w[1]!=(uint64_t)(int64_t)in->dst_vreg||w[2]!=(uint64_t)(int64_t)in->src1_vreg||w[3]!=(uint64_t)(int64_t)in->src2_vreg||w[4]!=(uint64_t)(int64_t)in->src3_vreg||w[5]!=(uint64_t)(int64_t)in->ureg||w[6]!=((uint64_t)in->imm|((uint64_t)in->control<<32))||w[7]!=flags){fprintf(stderr,"record %zu mismatch op=%u/%llu\n",i,in->op,(unsigned long long)w[0]);goto bad;}
    }
    for(int v=0;v<cp->regalloc.num_vregs;v++)if(meta[64+v]!=cp->regalloc.intervals[v].bundle_size){fprintf(stderr,"width v%d %llu/%u\n",v,(unsigned long long)meta[64+v],cp->regalloc.intervals[v].bundle_size);goto bad;}
    for(int v=0;v<cp->regalloc.num_uvregs;v++)if(meta[320+v]!=(cp->regalloc.uintervals[v].is_pair?2u:1u)){fprintf(stderr,"width u%d %llu/%u\n",v,(unsigned long long)meta[320+v],cp->regalloc.uintervals[v].is_pair?2u:1u);goto bad;}
    for(int l=0;l<e.n_loops;l++)if(meta[16+2*l]!=(uint64_t)e.loop_start[l]||meta[17+2*l]!=(uint64_t)e.loop_end[l]){fprintf(stderr,"loop %d %llu,%llu/%d,%d\n",l,(unsigned long long)meta[16+2*l],(unsigned long long)meta[17+2*l],e.loop_start[l],e.loop_end[l]);goto bad;}
    int cr=e.err?-1:omega_bw_regalloc_solve(cp);
    int orc=omega_gpu_om_ir_replay(omega_attention_ir,kind,k->hd,k->log2bs,k->log2gqa,op,&err);
    if((cr==0)!=(orc==0)){fprintf(stderr,"allocation result %d/%d replay_error=%llu\n",cr,orc,(unsigned long long)err);goto bad;}
    if(cr==0){
        if(check_loop_invariant(&e)!=0){fprintf(stderr,"C oracle loop invariant failed\n");goto bad;}
        uint8_t cb[BW_MAX_IR_INSNS*16],ob[BW_MAX_IR_INSNS*16];size_t cn=0,on=0;
        int ce=omega_bw_encode_program(cp,cb,sizeof cb,&cn),oe=omega_bw_encode_program(op,ob,sizeof ob,&on);
        if(ce!=oe||cn!=on||(ce==0&&memcmp(cb,ob,cn))||cp->regalloc.peak_gpr_usage!=op->regalloc.peak_gpr_usage){fprintf(stderr,"encode %d/%d bytes=%zu/%zu gpr=%u/%u\n",ce,oe,cn,on,cp->regalloc.peak_gpr_usage,op->regalloc.peak_gpr_usage);goto bad;}
    }
    cases++;free(cp);free(op);return 0;
bad:
    fprintf(stderr,"attention parity failed f32=%d general=%d hd=%u bs=%u gqa=%u split=%d mutant=%d divergent=%d status=%u result=%llu err=%llu n=%llu/%zu\n",k->f32,k->general,k->hd,k->log2bs,k->log2gqa,k->split,k->mutant,k->divergent,st,(unsigned long long)res,(unsigned long long)meta[4],(unsigned long long)meta[0],cp->count);
    fprintf(stderr,"vregs=%llu/%d uvregs=%llu/%d loops=%llu/%d metadata=%llu,%llu Cerror=%d depth=%d region=%d\n",(unsigned long long)meta[1],cp->regalloc.num_vregs,(unsigned long long)meta[2],cp->regalloc.num_uvregs,(unsigned long long)meta[5],e.n_loops,(unsigned long long)meta[6],(unsigned long long)meta[7],e.err,e.depth,e.rg.depth);
    free(cp);free(op);return 1;
}
static int refused(uint64_t kind,uint64_t hd,uint64_t bs,uint64_t gqa){
    uint64_t r[8192]={0},m[512],out=99;memset(m,0xa5,sizeof m);
    uint32_t st=omega_attention_ir(&out,kind,hd,bs,gqa,r,8192,m,512);
    if(st||out!=1||m[6]||m[7]||m[0]){fprintf(stderr,"refusal failed kind=%llu hd=%llu bs=%llu gqa=%llu\n",(unsigned long long)kind,(unsigned long long)hd,(unsigned long long)bs,(unsigned long long)gqa);return 1;}
    cases++;return 0;
}
int main(int argc,char **argv){
    (void)argv;
    KSpec k={.hd=64,.split=-1};
    if(argc>1)return compare(&k,1);
    /* All hd residues, warp boundaries, D4/DT branches. */
    for(unsigned f=0;f<2;f++)for(unsigned d=1;d<=256;d++){
        k=(KSpec){.f32=f,.hd=d,.general=true,.split=-1,.log2bs=31};if(compare(&k,0))return 1;
    }
    /* Full source-derived shift/GQA/split partitions, not a sampled Cartesian product. */
    for(unsigned f=0;f<2;f++)for(unsigned h=64;h<=128;h+=64)for(unsigned b=0;b<=31;b++)for(unsigned g=0;g<=31;g++)for(int s=-1;s<=4;s++){
        k=(KSpec){.f32=f,.hd=h,.log2bs=b,.log2gqa=g,.split=s};if(compare(&k,0))return 1;
    }
    for(unsigned f=0;f<2;f++)for(unsigned h=64;h<=128;h+=64)for(unsigned g=0;g<=3;g++)for(int s=-1;s<=4;s++)for(int m=0;m<=6;m++)for(int dv=0;dv<=2;dv++){
        k=(KSpec){.f32=f,.hd=h,.log2bs=4,.log2gqa=g,.split=s,.mutant=m,.divergent=dv};if(compare(&k,0))return 1;
    }
    const unsigned general_bs[]={0,1,30,31};
    for(unsigned f=0;f<2;f++)for(unsigned d=1;d<=256;d++)for(int m=0;m<=5;m++)for(size_t b=0;b<sizeof general_bs/sizeof *general_bs;b++){
        k=(KSpec){.f32=f,.hd=d,.general=true,.split=-1,.log2bs=general_bs[b],.mutant=m};if(compare(&k,0))return 1;
    }
    const uint32_t hd_bad[]={0,1,2,3,4,5,31,32,33,63,65,127,129,255,256,257,UINT32_MAX};
    for(size_t i=0;i<sizeof hd_bad/sizeof *hd_bad;i++)if(refused(0,hd_bad[i],0,0))return 1;
    if(refused(2,0,0,0)||refused(2,257,0,0)||refused(2,UINT32_MAX,0,0))return 1;
    const uint32_t sh_bad[]={32,33,UINT32_MAX};
    for(size_t i=0;i<sizeof sh_bad/sizeof *sh_bad;i++)if(refused(0,64,sh_bad[i],0)||refused(0,64,0,sh_bad[i]))return 1;
    if(refused(UINT32_MAX,64,0,0)||refused(1ull<<44,64,0,0)||refused(6ull<<40,64,0,0)||refused(7ull<<8,64,0,0)||refused(3ull<<2,64,0,0))return 1;
    printf("ATTENTION-IR DIFFERENTIAL PASS cases=%u records+encoded-bytes identical\n",cases);return 0;
}
