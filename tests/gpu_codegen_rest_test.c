/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_codegen_rest_wire.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static BlackwellIRProgram oldp, newp, defp;
static int flip_record;
static OmegaOmIrUnit unflipped;
static uint32_t flipped(uint64_t *ret,uint64_t k,uint64_t a,uint64_t b,uint64_t c,void *words,uint64_t nw,void *metadata,uint64_t nm) {
    uint64_t *w=words,*m=metadata;
    uint32_t rc=unflipped(ret,k,a,b,c,w,nw,m,nm);
    if(!rc && !*ret && m[0])w[6]^=1;
    return rc;
}
int legacy_omega_dattn256_shape(const OmegaDecodeAttn256Spec *spec, OmegaDattn256Shape *s);
int legacy_omega_blackwell_codegen_decode_attn256_prog(const OmegaDecodeAttn256Spec *spec, BlackwellIRProgram *prog);
int legacy_omega_decode_attn256_prog_stats(const OmegaDecodeAttn256Spec *spec, uint32_t *insns, uint32_t *gprs);
int legacy_omega_pattn256_shape(const OmegaPrefillAttn256Spec *spec, OmegaPattn256Shape *s);
int legacy_omega_pattn256_spec_ok(const OmegaPrefillAttn256Spec *s);
int legacy_omega_blackwell_codegen_prefill_attn256_prog(const OmegaPrefillAttn256Spec *spec, BlackwellIRProgram *prog);
int legacy_omega_prefill_attn256_prog_stats(const OmegaPrefillAttn256Spec *spec, uint32_t *insns, uint32_t *gprs);
int legacy_omega_router_smem(const OmegaRouterSpec *spec, OmegaRouterSmem *m);
int legacy_omega_blackwell_codegen_router_prog(const OmegaRouterSpec *spec, BlackwellIRProgram *prog);
int legacy_omega_router_prog_stats(const OmegaRouterSpec *spec, uint32_t *insns, uint32_t *gprs);
uint32_t legacy_omega_fp8gemm_steps(const OmegaFp8GemmSpec *spec);
uint32_t legacy_omega_fp8gemm_shared_bytes(const OmegaFp8GemmSpec *spec);
int legacy_omega_blackwell_codegen_fp8gemm_prog(const OmegaFp8GemmSpec *spec, BlackwellIRProgram *prog);
int legacy_omega_fp8gemm_prog_stats(const OmegaFp8GemmSpec *spec, uint32_t *insns, uint32_t *gprs);
uint32_t legacy_omega_bf16_gemv_steps(const OmegaBf16GemvSpec *spec);
uint32_t legacy_omega_bf16_gemv_shared_bytes(const OmegaBf16GemvSpec *spec);
int legacy_omega_blackwell_codegen_bf16_gemv_prog(const OmegaBf16GemvSpec *spec, BlackwellIRProgram *prog);
int legacy_omega_bf16_gemv_prog_stats(const OmegaBf16GemvSpec *spec, uint32_t *insns, uint32_t *gprs);
int legacy_omega_pq_spec_ok(const OmegaPqSpec *s);
int legacy_omega_blackwell_codegen_pq_prog(const OmegaPqSpec *spec, BlackwellIRProgram *prog);
int legacy_omega_qg_spec_ok(const OmegaQgSpec *s);
uint32_t legacy_omega_qg_shared_bytes(const OmegaQgSpec *s);
uint32_t legacy_omega_qg_span(const OmegaQgSpec *s);
int legacy_omega_blackwell_codegen_qgemm_prog(const OmegaQgSpec *spec, BlackwellIRProgram *prog);

static unsigned long cases, accepted, maxinsns, maxvregs;
static int cmp(const char *name, int oldrc, OmegaOmIrUnit unit, uint64_t kind, const void *spec) {
    uint64_t err=0;
    unflipped=unit;
    int rc=omega_gpu_om_ir_replay(flip_record?flipped:unit,kind,(uintptr_t)spec,0,0,&newp,&err);
    cases++;
    if(oldp.count>maxinsns)maxinsns=oldp.count;
    if(oldp.regalloc.num_vregs>0&&(unsigned long)oldp.regalloc.num_vregs>maxvregs)maxvregs=(unsigned long)oldp.regalloc.num_vregs;
    if ((oldrc==0)!=(rc==0)) { fprintf(stderr,"%s refusal mismatch C=%d OM=%d emit=%llu case=%lu\n",name,oldrc,rc,(unsigned long long)err,cases); return 1; }
    if(oldrc) return 0;
    accepted++;
    if(oldp.count!=newp.count) {fprintf(stderr,"%s count %zu != %zu\n",name,oldp.count,newp.count);return 1;}
    for(size_t i=0;i<oldp.count;i++) if(memcmp(&oldp.insns[i],&newp.insns[i],sizeof(oldp.insns[i]))) {
        const uint32_t *a=(const uint32_t *)&oldp.insns[i],*b=(const uint32_t *)&newp.insns[i];
        fprintf(stderr,"%s IR mismatch insn=%zu\n",name,i);
        for(size_t j=0;j<sizeof(oldp.insns[i])/sizeof(*a);j++) fprintf(stderr,"word%zu C=%u OM=%u\n",j,a[j],b[j]);return 1;
    }
    if(memcmp(&oldp.regalloc,&newp.regalloc,sizeof(oldp.regalloc))) {
        fprintf(stderr,"%s regalloc mismatch peak C=%u OM=%u vregs C=%d OM=%d\n",name,oldp.regalloc.peak_gpr_usage,newp.regalloc.peak_gpr_usage,oldp.regalloc.num_vregs,newp.regalloc.num_vregs);
        for(int v=0;v<oldp.regalloc.num_vregs;v++) if(memcmp(&oldp.regalloc.intervals[v],&newp.regalloc.intervals[v],sizeof(OmegaLiveInterval))) fprintf(stderr,"v%d C first=%d last=%d phys=%d OM first=%d last=%d phys=%d\n",v,oldp.regalloc.intervals[v].first_def,oldp.regalloc.intervals[v].last_use,oldp.regalloc.intervals[v].phys_reg,newp.regalloc.intervals[v].first_def,newp.regalloc.intervals[v].last_use,newp.regalloc.intervals[v].phys_reg);
        return 1;
    }
    uint8_t a[32768],b[32768];size_t na=0,nb=0;
    int ra=omega_bw_encode_program(&oldp,a,sizeof(a),&na),rb=omega_bw_encode_program(&newp,b,sizeof(b),&nb);
    if(ra!=rb||na!=nb||(ra==0&&memcmp(a,b,na))) {fprintf(stderr,"%s encoded byte mismatch\n",name);return 1;}
    return 0;
}
static int check_attn256(const OmegaDecodeAttn256Spec *s) {
    OmegaDattn256Shape a={0},b={0};int r=legacy_omega_dattn256_shape(s,&a),t=omega_dattn256_shape(s,&b);if(r!=t||(!r&&memcmp(&a,&b,sizeof(a))))return 1;
    int cr=legacy_omega_blackwell_codegen_decode_attn256_prog(s,&oldp),dr=omega_blackwell_codegen_decode_attn256_prog(s,&defp);
    if((cr==0)!=(dr==0)){fprintf(stderr,"attn256 default refusal mismatch\n");return 1;}
    if(cmp("attn256",cr,omega_rest_attn256_wire,1,s))return 1;
    if(!cr&&memcmp(&defp,&newp,sizeof(defp))){fprintf(stderr,"attn256 default wrapper mismatch\n");return 1;}
    return 0;
}
static int check_pattn256(const OmegaPrefillAttn256Spec *s) {
    OmegaPattn256Shape a={0},b={0};int r=legacy_omega_pattn256_shape(s,&a),t=omega_pattn256_shape(s,&b);if(r!=t||(!r&&memcmp(&a,&b,sizeof(a)))||legacy_omega_pattn256_spec_ok(s)!=omega_pattn256_spec_ok(s))return 1;
    int cr=legacy_omega_blackwell_codegen_prefill_attn256_prog(s,&oldp),dr=omega_blackwell_codegen_prefill_attn256_prog(s,&defp);
    if((cr==0)!=(dr==0)){fprintf(stderr,"pattn256 default refusal mismatch\n");return 1;}
    if(cmp("pattn256",cr,omega_rest_pattn256_wire,2,s))return 1;
    if(!cr&&memcmp(&defp,&newp,sizeof(defp))){fprintf(stderr,"pattn256 default wrapper mismatch\n");return 1;}
    return 0;
}
static int check_router(const OmegaRouterSpec *s) {
    OmegaRouterSmem a={0},b={0};int r=legacy_omega_router_smem(s,&a),t=omega_router_smem(s,&b);if(r!=t||(!r&&memcmp(&a,&b,sizeof(a))))return 1;
    int cr=legacy_omega_blackwell_codegen_router_prog(s,&oldp),dr=omega_blackwell_codegen_router_prog(s,&defp);
    if((cr==0)!=(dr==0)){fprintf(stderr,"router default refusal mismatch\n");return 1;}
    if(cmp("router",cr,omega_rest_router_wire,1,s))return 1;
    if(!cr&&memcmp(&defp,&newp,sizeof(defp))){fprintf(stderr,"router default wrapper mismatch\n");return 1;}
    return 0;
}
static int check_gemm(const OmegaFp8GemmSpec *s) {
    if(legacy_omega_fp8gemm_steps(s)!=omega_fp8gemm_steps(s)||legacy_omega_fp8gemm_shared_bytes(s)!=omega_fp8gemm_shared_bytes(s))return 1;
    int cr=legacy_omega_blackwell_codegen_fp8gemm_prog(s,&oldp),dr=omega_blackwell_codegen_fp8gemm_prog(s,&defp);
    if((cr==0)!=(dr==0)){fprintf(stderr,"gemm default refusal mismatch\n");return 1;}
    if(cmp("gemm",cr,omega_rest_gemm_wire,2,s))return 1;
    if(!cr&&memcmp(&defp,&newp,sizeof(defp))){fprintf(stderr,"gemm default wrapper mismatch\n");return 1;}
    return 0;
}
static int check_bf16(const OmegaBf16GemvSpec *s) {
    if(legacy_omega_bf16_gemv_steps(s)!=omega_bf16_gemv_steps(s)||legacy_omega_bf16_gemv_shared_bytes(s)!=omega_bf16_gemv_shared_bytes(s))return 1;
    int cr=legacy_omega_blackwell_codegen_bf16_gemv_prog(s,&oldp),dr=omega_blackwell_codegen_bf16_gemv_prog(s,&defp);
    if((cr==0)!=(dr==0)){fprintf(stderr,"bf16 default refusal mismatch\n");return 1;}
    if(cmp("bf16",cr,omega_rest_bf16_wire,2,s))return 1;
    if(!cr&&memcmp(&defp,&newp,sizeof(defp))){fprintf(stderr,"bf16 default wrapper mismatch\n");return 1;}
    return 0;
}
static int check_pq(const OmegaPqSpec *s) {
    if(legacy_omega_pq_spec_ok(s)!=omega_pq_spec_ok(s))return 1;
    int cr=legacy_omega_blackwell_codegen_pq_prog(s,&oldp),dr=omega_blackwell_codegen_pq_prog(s,&defp);
    if((cr==0)!=(dr==0)){fprintf(stderr,"pq default refusal mismatch\n");return 1;}
    if(cmp("pq",cr,omega_rest_qg_wire,1,s))return 1;
    if(!cr&&memcmp(&defp,&newp,sizeof(defp))){fprintf(stderr,"pq default wrapper mismatch\n");return 1;}
    return 0;
}
static int check_qg(const OmegaQgSpec *s) {
    if(legacy_omega_qg_spec_ok(s)!=omega_qg_spec_ok(s)||legacy_omega_qg_shared_bytes(s)!=omega_qg_shared_bytes(s)||legacy_omega_qg_span(s)!=omega_qg_span(s))return 1;
    int cr=legacy_omega_blackwell_codegen_qgemm_prog(s,&oldp),dr=omega_blackwell_codegen_qgemm_prog(s,&defp);
    if((cr==0)!=(dr==0)){cmp("qg",cr,omega_rest_qg_wire,5,s);fprintf(stderr,"qg default refusal mismatch C=%d OM=%d\n",cr,dr);return 1;}
    if(cmp("qg",cr,omega_rest_qg_wire,5,s))return 1;
    if(!cr&&memcmp(&defp,&newp,sizeof(defp))){fprintf(stderr,"qg default wrapper mismatch\n");return 1;}
    return 0;
}
static uint32_t rng=0x9135a71u;
static uint32_t next_random(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static const uint32_t edges[]={0,1,2,3,4,7,8,15,16,31,32,63,64,127,128,255,256,511,512,1023,1024,16383,16384,32767,32768,65535,65536,0x7fffffffu,0x80000000u,0xfffffffeu,0xffffffffu};
/* Refusal constants from the six C admission functions and qg shape policy.
 * Each center is checked on both sides, separately in every scalar field. */
static const uint32_t bounds[]={1,2,4,5,6,8,16,32,64,120,128,256,512,1024,2048,4096,8192,16384,32768,65536,8388607,UINT32_C(1)<<23,UINT32_C(1)<<28,UINT32_C(1)<<31};
#define CHECK(n,s) do{if(check_##n(&(s))) {fprintf(stderr,#n " spec:");for(size_t zz=0;zz<sizeof(s);zz+=4){uint32_t vv;memcpy(&vv,(char*)&(s)+zz,4);fprintf(stderr," %u",vv);}fprintf(stderr,"\n");return 1;}}while(0)
#define BOUND(n,base) do{for(size_t field=0;field<sizeof(base)/4;field++)for(size_t e=0;e<sizeof(bounds)/sizeof(bounds[0]);e++)for(int delta=-1;delta<=1;delta++){__typeof__(base) s=(base);uint32_t val=bounds[e]+delta;memcpy((char*)&s+4*field,&val,4);CHECK(n,s);}}while(0)
#define RANDOM(n,base) do{BOUND(n,base);for(size_t field=0;field<sizeof(base)/4;field++)for(size_t e=0;e<sizeof(edges)/sizeof(edges[0]);e++){__typeof__(base) s=(base);memcpy((char*)&s+4*field,&edges[e],4);CHECK(n,s);}for(unsigned ii=0;ii<10000;ii++){__typeof__(base) s=(base);uint32_t val=next_random();size_t field=next_random()%(sizeof(s)/4);memcpy((char*)&s+4*field,&val,4);CHECK(n,s);}}while(0)
#define NULLS(fn,base) do{if(legacy_##fn(NULL,&oldp)==0||fn(NULL,&newp)==0||legacy_##fn(&(base),NULL)==0||fn(&(base),NULL)==0){fprintf(stderr,"null refusal mismatch: " #fn "\n");return 1;}cases+=4;}while(0)
#define STATS(fn,base) do{for(unsigned present=0;present<=1;present++)for(unsigned outputs=0;outputs<4;outputs++){uint32_t ci=0,cg=0,oi=0,og=0;int cr=legacy_##fn(present?&(base):NULL,outputs&1?&ci:NULL,outputs&2?&cg:NULL),orr=fn(present?&(base):NULL,outputs&1?&oi:NULL,outputs&2?&og:NULL);if(cr!=orr||ci!=oi||cg!=og){fprintf(stderr,"statistics mismatch: " #fn "\n");return 1;}cases++;}}while(0)
static int run(const char *family,int full){
    OmegaDecodeAttn256Spec a={.depth=1};
    OmegaPrefillAttn256Spec p={0};
    OmegaRouterSpec r={.warps=8,.depth=1,.experts=128};
    OmegaFp8GemmSpec g={.k=1024,.lane_bytes=16,.mt=2,.warps=16};
    OmegaBf16GemvSpec b={.k=1024,.rows=1,.warps=8,.depth=1};
    OmegaPqSpec pq={.k=1024};
    OmegaQgSpec q={.k=1024,.rt=4,.warps=8,.n=128};
    if(!strcmp(family,"attn256")){
        STATS(omega_decode_attn256_prog_stats,a);NULLS(omega_blackwell_codegen_decode_attn256_prog,a);CHECK(attn256,a);if(!full)return 0;
        const uint32_t qhs[]={0,1,8,10,16,20,24,30,32,40,48,60,64},kvhs[]={0,1,2,4,8};
        for(unsigned d=1;d<=4;d++)for(unsigned prep=0;prep<=1;prep++)for(unsigned multi=0;multi<=1;multi++)for(unsigned qp=0;qp<=6;qp++)for(size_t h=0;h<sizeof(qhs)/sizeof(qhs[0]);h++)for(size_t k=0;k<sizeof(kvhs)/sizeof(kvhs[0]);k++)for(int m=0;m<=5;m++){OmegaDecodeAttn256Spec s={.depth=d,.mutant=m,.prep=prep,.q_heads=qhs[h],.kv_heads=kvhs[k],.q_per_cta=qp,.multi=multi};CHECK(attn256,s);}
        for(unsigned h=1;h<=6;h++)for(unsigned group=1;group<=2;group++)for(unsigned bit=0;bit<32;bit++){uint64_t kv=UINT64_C(1)<<bit,qh=kv*h*group;if(qh>UINT32_MAX)continue;OmegaDecodeAttn256Spec s=a;s.q_heads=(uint32_t)qh;s.kv_heads=(uint32_t)kv;s.q_per_cta=h;CHECK(attn256,s);}RANDOM(attn256,a);
        const uint32_t wide[]={3,(UINT32_C(1)<<31)-1u,UINT32_C(1)<<31,(UINT32_C(1)<<31)+1u,UINT32_MAX};
        for(unsigned wi=0;wi<sizeof(wide)/sizeof(wide[0]);wi++){OmegaDecodeAttn256Spec s=a;s.q_heads=wide[wi];s.kv_heads=wide[wi];s.q_per_cta=1;s.multi=1;CHECK(attn256,s);}
    }else if(!strcmp(family,"pattn256")){
        STATS(omega_prefill_attn256_prog_stats,p);NULLS(omega_blackwell_codegen_prefill_attn256_prog,p);CHECK(pattn256,p);if(!full)return 0;
        for(uint32_t qh=0;qh<=64;qh++)for(uint32_t kv=0;kv<=16;kv++)for(int m=0;m<=3;m++){OmegaPrefillAttn256Spec s={.q_heads=qh,.kv_heads=kv,.mutant=m};CHECK(pattn256,s);}RANDOM(pattn256,p);
    }else if(!strcmp(family,"router")){
        STATS(omega_router_prog_stats,r);NULLS(omega_blackwell_codegen_router_prog,r);CHECK(router,r);if(!full)return 0;
        const uint32_t ex[]={0,128,256},strides[]={0,1,128,2048,OMEGA_ROUTER_HIDDEN*4u,65536};
        for(uint32_t w=1;w<=32;w++)for(uint32_t d=1;d<=8;d++)for(unsigned e=0;e<3;e++)for(unsigned shared=0;shared<=1;shared++)for(int m=0;m<=6;m++){OmegaRouterSpec s={.warps=w,.depth=d,.experts=ex[e],.shared=shared,.mutant=m};CHECK(router,s);}
        for(uint32_t w=8;w<=32;w*=2)for(uint32_t d=1;d<=8;d*=2)for(unsigned e=0;e<3;e++)for(unsigned shared=0;shared<=1;shared++)for(int m=0;m<=6;m++)for(unsigned x=0;x<sizeof(strides)/sizeof(strides[0]);x++)for(unsigned l=0;l<sizeof(strides)/sizeof(strides[0]);l++){OmegaRouterSpec s={.warps=w,.depth=d,.experts=ex[e],.shared=shared,.mutant=m,.xstride=strides[x],.lbstride=strides[l]};CHECK(router,s);}RANDOM(router,r);
        OmegaRouterSpec rows=r;rows.xstride=OMEGA_ROUTER_HIDDEN*4u;rows.lbstride=16;BOUND(router,rows);
    }else if(!strcmp(family,"gemm")){
        STATS(omega_fp8gemm_prog_stats,g);NULLS(omega_blackwell_codegen_fp8gemm_prog,g);CHECK(gemm,g);if(!full)return 0;
        for(uint32_t k=128;k<=16384;k+=128)for(uint32_t lb=8;lb<=16;lb+=8)for(uint32_t mt=1;mt<=16;mt*=2)for(uint32_t w=1;w<=32;w++)for(int rm=0;rm<=1;rm++){OmegaFp8GemmSpec s={.k=k,.lane_bytes=lb,.mt=mt,.warps=w,.rowmap=rm};CHECK(gemm,s);if(legacy_omega_fp8gemm_steps(&s))for(int m=1;m<=4;m++){s.mutant=m;CHECK(gemm,s);}}
        for(int m=0;m<=8;m++){OmegaFp8GemmSpec s=g;s.mutant=m;CHECK(gemm,s);}RANDOM(gemm,g);
    }else if(!strcmp(family,"bf16")){
        STATS(omega_bf16_gemv_prog_stats,b);NULLS(omega_blackwell_codegen_bf16_gemv_prog,b);CHECK(bf16,b);if(!full)return 0;
        for(uint32_t k=256;k<=16384;k+=256)for(uint32_t row=1;row<=4;row*=2)for(uint32_t w=1;w<=32;w++)for(uint32_t d=1;d<=16;d++){OmegaBf16GemvSpec s={.k=k,.rows=row,.warps=w,.depth=d};CHECK(bf16,s);if(legacy_omega_bf16_gemv_steps(&s))for(unsigned mask=0;mask<=1;mask++)for(unsigned xb=0;xb<=4;xb++)for(unsigned ys=0;ys<=2;ys++)for(int m=0;m<=4;m++){OmegaBf16GemvSpec t=s;t.mask=mask;t.xbatch=xb;t.ystride=ys==0?0:ys==1?4096:UINT32_C(1)<<28;t.mutant=m;CHECK(bf16,t);}}
        for(unsigned mask=0;mask<=1;mask++)for(unsigned xb=0;xb<=5;xb++)for(int m=0;m<=5;m++){OmegaBf16GemvSpec s=b;s.mask=mask;s.xbatch=xb;s.ystride=4096;s.mutant=m;CHECK(bf16,s);}RANDOM(bf16,b);
        OmegaBf16GemvSpec batch=b;batch.xbatch=3;batch.ystride=4096;BOUND(bf16,batch);
    }else if(!strcmp(family,"qg")){
        NULLS(omega_blackwell_codegen_pq_prog,pq);NULLS(omega_blackwell_codegen_qgemm_prog,q);CHECK(pq,pq);CHECK(qg,q);OmegaQgSpec rt8=q;rt8.rt=8;CHECK(qg,rt8);
        /* A conditional second scale register must follow the first initializer. */
        OmegaQgSpec gated=q;gated.rt=2;gated.warps=4;gated.epi=1;gated.n=256;CHECK(qg,gated);if(!full)return 0;
        const uint32_t rts[]={1,2,4,8},ws[]={4,8,16,32},ns[]={128,256,384,512,640,768,1024,2048,4096,8192};
        for(uint32_t k=128;k<=32768;k+=128)for(unsigned rt=0;rt<4;rt++)for(unsigned w=0;w<4;w++)for(unsigned ep=0;ep<=1;ep++)for(unsigned direct=0;direct<=1;direct++){OmegaQgSpec s={.k=k,.rt=rts[rt],.warps=ws[w],.epi=ep,.direct=direct,.n=128};CHECK(qg,s);if(legacy_omega_qg_spec_ok(&s))for(int m=1;m<=4;m++){s.mutant=m;CHECK(qg,s);}}
        for(uint32_t k=128;k<=32768;k+=128)for(unsigned in=0;in<=2;in++)for(unsigned group=0;group<=32;group++)for(int m=0;m<=3;m++){OmegaPqSpec s={.k=k,.in_type=in,.groups=group,.mutant=m};CHECK(pq,s);}
        for(unsigned wait=0;wait<=121;wait++)for(unsigned group=0;group<=6;group++)for(int m=0;m<=6;m++){OmegaQgSpec s=q;s.wait=wait;s.group=group;s.mutant=m;CHECK(qg,s);}
        for(unsigned rt=0;rt<4;rt++)for(unsigned w=0;w<4;w++)for(unsigned ep=0;ep<=1;ep++)for(unsigned direct=0;direct<=1;direct++)for(unsigned ni=0;ni<sizeof(ns)/sizeof(ns[0]);ni++)for(unsigned group=0;group<=6;group++){OmegaQgSpec s=q;s.rt=rts[rt];s.warps=ws[w];s.epi=ep;s.direct=direct;s.n=ns[ni];s.group=group;CHECK(qg,s);}RANDOM(qg,q);RANDOM(pq,pq);
        /* Wolfram-derived exact qg_offset limits; also test adjacent legal spans. */
        const uint32_t orts[]={2,4,8},limit[][2]={{87380,349525},{37448,149796},{17476,69905}};
        for(unsigned rt=0;rt<3;rt++)for(unsigned ep=0;ep<=1;ep++)for(int delta=-1;delta<=1;delta++){
            OmegaQgSpec s=q;s.rt=orts[rt];s.warps=s.rt==2?4:8;s.epi=ep;s.n=limit[rt][ep]+delta;CHECK(qg,s);
            const uint32_t span=s.warps*16u;s.n=(limit[rt][ep]/span+delta)*span;CHECK(qg,s);
        }
    }else{fprintf(stderr,"unknown family %s\n",family);return 1;}
    return 0;
}
int main(int argc,char **argv){
    const char *family=argc>1?argv[1]:"all";int full=argc<=2||strcmp(argv[2],"--smoke");flip_record=argc>3&&!strcmp(argv[3],"--record-flip");
    const char *all[]={"attn256","pattn256","router","gemm","bf16","qg"};
    for(unsigned i=0;i<6;i++)if(!strcmp(family,"all")||!strcmp(family,all[i])){cases=accepted=maxinsns=maxvregs=0;if(run(all[i],full))return 1;printf("PASS codegen-rest %s cases=%lu accepted=%lu max-insns=%lu max-vregs=%lu IR/regalloc/bytes/refusals/default-shell\n",all[i],cases,accepted,maxinsns,maxvregs);fflush(stdout);}
    return 0;
}
