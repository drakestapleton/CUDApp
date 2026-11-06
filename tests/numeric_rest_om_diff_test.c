/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* This TU exercises the default IEEE inlines; oracle TUs retain C predicates. */
#undef CUDAPP_MIGRATION_PENDING_C_NUMERIC_REST_DIFF
#include "omega_numeric.h"
#include "omega_numeric_reduce.h"
#include "omega_blackwell_qmd.h"
#include <assert.h>
#include <fenv.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define DECL_BULK(n) int numeric_rest_c_##n(OmegaNumericOp,const float*,const float*,const float*,float*,size_t)
DECL_BULK(reference); DECL_BULK(cpu_realize); DECL_BULK(reference_ftz);
int numeric_rest_c_patch_words(OmegaNumericOp,OmegaNumericPatchInsn*);
int numeric_rest_c_check_patch(OmegaNumericOp,const OmegaNumericPatchInsn*,int,const uint32_t*,char*,size_t);
int numeric_rest_c_submit_check(const char*,const float*,const float*,const float*,const float*,size_t,char*,size_t);
bool numeric_rest_c_element_checked(OmegaNumericOp,size_t);
int numeric_rest_c_parity(OmegaNumericOp,const float*,const float*,size_t,OmegaParityTrace*);
int numeric_rest_c_seed_bound(OmegaNumericOp,const float*,const float*,size_t,OmegaParityTrace*);
int numeric_rest_c_reduce_reference(OmegaReduceOp,const float*,size_t,float*);
int numeric_rest_c_reduce_cpu(OmegaReduceOp,const float*,size_t,float*);
int numeric_rest_c_reduce_rows_reference(OmegaReduceOp,const float*,size_t,size_t,size_t,float*);
int numeric_rest_c_reduce_rows_cpu(OmegaReduceOp,const float*,size_t,size_t,size_t,float*);
float numeric_rest_c_reduce_identity(OmegaReduceOp);
float numeric_rest_c_reduce_sequential_sum_not_contract(const float*,size_t);
static unsigned checks;
static void equal(int ok,const char *where,unsigned op,size_t n) {
    ++checks;
    if (!ok) { fprintf(stderr,"FAIL %s op=%u n=%zu\n",where,op,n); exit(1); }
}
static int beq(float a,float b) {
    uint32_t x=omega_float_to_bits(a),y=omega_float_to_bits(b);
    return x==y || ((x&0x7fffffff)>0x7f800000 && (y&0x7fffffff)>0x7f800000);
}
static void traceeq(const OmegaParityTrace *a,const OmegaParityTrace *b,unsigned op,size_t n) {
    equal(a->checked==b->checked && a->mismatches==b->mismatches && a->first_index==b->first_index && a->first_expect==b->first_expect && a->first_got==b->first_got && a->subnormal_expected==b->subnormal_expected && a->out_of_bound==b->out_of_bound && a->bound_skipped==b->bound_skipped,"trace",op,n);
}
int main(void) {
    static const uint32_t edge[]={0,0x80000000,1,0x80000001,0x007fffff,0x807fffff,0x00800000,0x80800000,0x3f7fffff,0x3f800000,0x3f800001,0xbf800000,0x40000000,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00000,0xffc00000,0x7f800001,0x4b7fffff,0x4b800000,0x4b800001,0x4effffff,0x4f000000,0xcf000000,0x477fe000,0x47800000};
    static const size_t lengths[]={0,1,31,32,33,63,64,65,1023,1024,1025};
    float a[1089],b[1089],c[1089],x[1089],y[1089];
    char err[512];
    for (unsigned shift=0;shift<sizeof(edge)/sizeof(edge[0]);++shift) {
        for(size_t i=0;i<1089;++i) {
            a[i]=omega_bits_to_float(edge[(i+shift)%28]);
            b[i]=omega_bits_to_float(edge[(i*3+shift+1)%28]);
            c[i]=omega_bits_to_float(edge[(i*5+shift+2)%28]);
        }
        for(unsigned op=0;op<OMEGA_NOP_COUNT;++op) {
            for(size_t k=0;k<sizeof(lengths)/sizeof(lengths[0]);++k) {
                size_t n=lengths[k];
                for(unsigned tier=0;tier<3;++tier) {
                    int r=tier==0?numeric_rest_c_reference(op,a,b,c,x,n):tier==1?numeric_rest_c_cpu_realize(op,a,b,c,x,n):numeric_rest_c_reference_ftz(op,a,b,c,x,n);
                    int s=tier==0?omega_numeric_reference(op,a,b,c,y,n):tier==1?omega_numeric_cpu_realize(op,a,b,c,y,n):omega_numeric_reference_ftz(op,a,b,c,y,n);
                    equal(r==s,"bulk rc",op,n);
                    if(!r) for(size_t i=0;i<n;++i) equal(beq(x[i],y[i]),tier==0?"reference bits":tier==1?"CPU bits":"FTZ bits",op,i);
                }
                OmegaParityTrace p,q;
                memset(&p,0xa5,sizeof(p)); memset(&q,0xa5,sizeof(q));
                int r=numeric_rest_c_parity(op,a,b,n,&p),s=omega_numeric_parity(op,a,b,n,&q);
                equal(r==s,"parity rc",op,n); traceeq(&p,&q,op,n);
                memset(&p,0xa5,sizeof(p)); memset(&q,0xa5,sizeof(q));
                r=numeric_rest_c_seed_bound(op,a,b,n,&p); s=omega_numeric_seed_bound(op,a,b,n,&q);
                equal(r==s,"seed rc",op,n); traceeq(&p,&q,op,n);
            }
        }
        for(unsigned op=0;op<OMEGA_RED_COUNT;++op) {
            equal(beq(numeric_rest_c_reduce_identity(op),omega_reduce_identity(op)),"identity",op,0);
            for(size_t k=0;k<sizeof(lengths)/sizeof(lengths[0]);++k) {
                size_t n=lengths[k];
                int r=numeric_rest_c_reduce_reference(op,a,n,x),s=omega_reduce_reference(op,a,n,y);
                equal(r==s && (r || beq(x[0],y[0])),"reduce reference",op,n);
                r=numeric_rest_c_reduce_cpu(op,a,n,x);s=omega_reduce_cpu(op,a,n,y);
                equal(r==s && (r || beq(x[0],y[0])),"reduce CPU",op,n);
                equal(beq(numeric_rest_c_reduce_sequential_sum_not_contract(a,n),omega_reduce_sequential_sum_not_contract(a,n)),"sequential control",op,n);
            }
            int r=numeric_rest_c_reduce_rows_reference(op,a,3,33,64,x),s=omega_reduce_rows_reference(op,a,3,33,64,y);
            equal(r==s,"rows rc",op,33);
            if(!r)for(unsigned i=0;i<3;++i)equal(beq(x[i],y[i]),"rows bits",op,i);
            r=numeric_rest_c_reduce_rows_cpu(op,a,3,33,64,x);s=omega_reduce_rows_cpu(op,a,3,33,64,y);
            equal(r==s,"rows CPU rc",op,33);
            if(!r)for(unsigned i=0;i<3;++i)equal(beq(x[i],y[i]),"rows CPU bits",op,i);
        }
    }
    for(unsigned op=0;op<OMEGA_NOP_COUNT;++op) {
        OmegaNumericPatchInsn p[OMEGA_NUMERIC_PATCH_MAX],q[OMEGA_NUMERIC_PATCH_MAX];
        int n=numeric_rest_c_patch_words(op,p),m=omega_numeric_patch_words(op,q);
        equal(n==m,"patch count",op,0);
        if(n<=0)continue;
        for(int i=0;i<n;++i) {
            equal(!memcmp(p[i].w,q[i].w,sizeof(p[i].w)),"patch words",op,i);
            equal((!p[i].provenance_key&&!q[i].provenance_key)||(p[i].provenance_key&&q[i].provenance_key&&!strcmp(p[i].provenance_key,q[i].provenance_key)),"patch key",op,i);
        }
        uint32_t launch[OMEGA_BW_QMD_WORDS]; OmegaBlackwellQmdConfig cfg={0};
        cfg.num_elements=64; omega_numeric_launch_shape(64,&cfg.threads_per_block,&cfg.grid_width);
        equal(!omega_blackwell_build_qmd1(launch,&cfg),"qmd",op,0);
        equal(numeric_rest_c_check_patch(op,p,n,launch,err,sizeof(err))==omega_numeric_check_patch(op,q,n,launch,err,sizeof(err)),"check valid",op,n);
        for(int i=0;i<n;++i)for(unsigned w=0;w<4;++w)for(unsigned bit=0;bit<32;++bit) {
            p[i].w[w]^=1u<<bit; q[i].w[w]^=1u<<bit;
            equal(numeric_rest_c_check_patch(op,p,n,launch,NULL,0)==omega_numeric_check_patch(op,q,n,launch,NULL,0),"check patch bit",op,i*128+w*32+bit);
            p[i].w[w]^=1u<<bit; q[i].w[w]^=1u<<bit;
        }
        for(unsigned w=0;w<OMEGA_BW_QMD_WORDS;++w)for(unsigned bit=0;bit<32;++bit) {
            launch[w]^=1u<<bit;
            equal(numeric_rest_c_check_patch(op,p,n,launch,NULL,0)==omega_numeric_check_patch(op,q,n,launch,NULL,0),"check QMD bit",op,w*32+bit);
            launch[w]^=1u<<bit;
        }
        const OmegaNumericOpInfo *info=omega_numeric_op_at(op);
        for(size_t k=0;k<sizeof(lengths)/sizeof(lengths[0]);++k) {
            size_t n0=lengths[k];
            equal(numeric_rest_c_submit_check(info->name,a,b,c,x,n0,NULL,0)==omega_numeric_submit_check(info->name,a,b,c,x,n0,NULL,0),"submit",op,n0);
        }
        for(unsigned flags=0;flags<16;++flags) {
            const float *aa=flags&1?a:NULL,*bb=flags&2?b:NULL,*cc=flags&4?c:NULL;
            float *oo=flags&8?x:NULL;
            equal(numeric_rest_c_submit_check(info->name,aa,bb,cc,oo,1,NULL,0)==omega_numeric_submit_check(info->name,aa,bb,cc,oo,1,NULL,0),"submit null admission",op,flags);
        }
        for(size_t i=0;i<65;++i)
            equal(numeric_rest_c_element_checked(op,i)==omega_numeric_element_checked(op,i),"checked lane",op,i);
        equal(numeric_rest_c_check_patch(op,p,0,launch,NULL,0)==omega_numeric_check_patch(op,q,0,launch,NULL,0),"check zero count",op,0);
        equal(numeric_rest_c_check_patch(op,NULL,n,launch,NULL,0)==omega_numeric_check_patch(op,NULL,n,launch,NULL,0),"check null patch",op,0);
        equal(numeric_rest_c_check_patch(op,p,n,NULL,NULL,0)==omega_numeric_check_patch(op,q,n,NULL,NULL,0),"check null QMD",op,0);
    }
    for(unsigned op=0;op<=OMEGA_NOP_COUNT;++op)for(unsigned flags=0;flags<16;++flags) {
        const float *aa=flags&1?a:NULL,*bb=flags&2?b:NULL,*cc=flags&4?c:NULL;float *oo=flags&8?x:NULL;
        equal(numeric_rest_c_reference(op,aa,bb,cc,oo,0)==omega_numeric_reference(op,aa,bb,cc,oo,0),"null admission",op,flags);
    }
    equal(omega_numeric_reference((OmegaNumericOp)UINT32_MAX,a,b,c,x,0)==OMEGA_NUMERIC_ERR_BAD_ARGS,"invalid op",UINT32_MAX,0);
    equal(omega_reduce_reference(OMEGA_RED_MEAN,NULL,OMEGA_REDUCE_MEAN_MAX_N+1,x)==OMEGA_NUMERIC_ERR_BAD_ARGS,"mean null priority",3,0);
    equal(omega_reduce_reference(OMEGA_RED_MEAN,a,OMEGA_REDUCE_MEAN_MAX_N+1,x)==OMEGA_NUMERIC_ERR_OPERANDS,"mean bound",3,0);
    equal(omega_reduce_reference(OMEGA_RED_SUM,a,SIZE_MAX,x)==OMEGA_NUMERIC_ERR_OPERANDS,"size overflow",0,0);
    equal(omega_reduce_rows_reference(OMEGA_RED_SUM,a,2,SIZE_MAX,SIZE_MAX,x)==OMEGA_NUMERIC_ERR_OPERANDS,"rows overflow",0,0);
    equal(omega_reduce_rows_reference(OMEGA_RED_SUM,NULL,0,SIZE_MAX,SIZE_MAX,NULL)==0,"zero rows",0,0);
    for(size_t n=OMEGA_REDUCE_MEAN_MAX_N-1;n<=OMEGA_REDUCE_MEAN_MAX_N+1;++n)
        equal(omega_numeric_rest(22,OMEGA_RED_MEAN,n,25,0,0)==(n>OMEGA_REDUCE_MEAN_MAX_N?3u:0u),"mean admission boundary",3,n);
    a[0]=1.0f;
    for(unsigned j=7;j<=9;++j) {
        OmegaParityTrace p,q;b[0]=omega_bits_to_float(0x3f800000+j);
        int r=numeric_rest_c_seed_bound(OMEGA_NOP_MUFU_RCP,a,b,1,&p);
        int s=omega_numeric_seed_bound(OMEGA_NOP_MUFU_RCP,a,b,1,&q);
        equal(r==s,"seed threshold rc",OMEGA_NOP_MUFU_RCP,j);traceeq(&p,&q,OMEGA_NOP_MUFU_RCP,j);
        equal(q.out_of_bound==(j==9),"seed threshold",OMEGA_NOP_MUFU_RCP,j);
    }
    equal(numeric_rest_c_reference(OMEGA_NOP_FADD,a,b,(const float *)(uintptr_t)1,x,1)==omega_numeric_reference(OMEGA_NOP_FADD,a,b,(const float *)(uintptr_t)1,y,1) && beq(x[0],y[0]),"unused c not dereferenced",0,1);
    equal(omega_numeric_rest(35,OMEGA_NOP_F32_TO_BF16,1,1,0,0)==1 && omega_numeric_rest(35,OMEGA_NOP_F32_TO_BF16,0,1,0,0)==0 && omega_numeric_rest(35,OMEGA_NOP_F32_TO_BF16,1,0,0,0)==0,"CPU BF16 feature admission",38,1);
    equal(omega_numeric_rest(36,0,0,0,0,0)==2 && omega_numeric_rest(36,1,0,0,0,0)==0,"CPU BF16 unavailable red",38,1);
    equal(numeric_rest_c_submit_check(NULL,a,b,c,x,1,NULL,0)==omega_numeric_submit_check(NULL,a,b,c,x,1,NULL,0),"null name",0,1);
    equal(numeric_rest_c_submit_check("NO_SUCH_OP",a,b,c,x,1,NULL,0)==omega_numeric_submit_check("NO_SUCH_OP",a,b,c,x,1,NULL,0),"unknown name",0,1);
    for(size_t j=0;j<omega_numeric_refused_variant_count();++j) {
        const char *name=omega_numeric_refused_variant_at(j);
        equal(numeric_rest_c_submit_check(name,a,b,c,x,1,NULL,0)==omega_numeric_submit_check(name,a,b,c,x,1,NULL,0),"refused variant",0,j);
    }
    for(unsigned j=0;j<32;++j)c[j]=1.0f;
    equal(numeric_rest_c_submit_check(omega_numeric_op_at(OMEGA_NOP_FFMA)->name,a,b,c,x,32,NULL,0)==0 && omega_numeric_submit_check(omega_numeric_op_at(OMEGA_NOP_FFMA)->name,a,b,c,x,32,NULL,0)==0,"uniform FFMA",3,32);
    equal(numeric_rest_c_reduce_rows_reference(OMEGA_RED_SUM,a,3,33,32,x)==omega_reduce_rows_reference(OMEGA_RED_SUM,a,3,33,32,y),"short row stride",0,32);
    equal(numeric_rest_c_reduce_rows_cpu(OMEGA_RED_MEAN,NULL,3,0,0,x)==omega_reduce_rows_cpu(OMEGA_RED_MEAN,NULL,3,0,0,y) && beq(x[0],y[0]) && beq(x[2],y[2]),"empty CPU rows",3,0);
    for(size_t j=0;j<sizeof(edge)/sizeof(edge[0]);++j) {
        uint32_t u=edge[j], mag=u&0x7fffffff; float f=omega_bits_to_float(u);
        equal(omega_float_to_bits(omega_fabs(f))==mag,"fabs bits",0,j);
        equal(omega_isnan(f)==(mag>0x7f800000),"isnan class",0,j);
        equal(omega_isinf(f)==(mag==0x7f800000),"isinf class",0,j);
        equal(omega_signbit(f)==((u>>31)!=0),"signbit",0,j);
        equal(omega_issubnormal(f)==(mag>0 && mag<0x00800000),"subnormal class",0,j);
        equal(omega_iszero(f)==(mag==0),"zero class",0,j);
    }
    const size_t launch_edges[]={0,65535,65536,65537,UINT32_MAX};
    const OmegaNumericOp launch_ops[]={OMEGA_NOP_FADD,OMEGA_NOP_LDS_STS,OMEGA_NOP_SHFL_DOWN,OMEGA_NOP_REDUCE_SUM};
    for(size_t j=0;j<sizeof(launch_ops)/sizeof(launch_ops[0]);++j)
        for(size_t k=0;k<sizeof(launch_edges)/sizeof(launch_edges[0]);++k) {
            const OmegaNumericOpInfo *info=omega_numeric_op_at(launch_ops[j]);
            size_t n=launch_edges[k];
            equal(numeric_rest_c_submit_check(info->name,a,b,NULL,x,n,NULL,0)==omega_numeric_submit_check(info->name,a,b,NULL,x,n,NULL,0),"launch boundary",info->op,n);
        }
    /* Wolfram Real32 fold/tree query in the evidence gives {0.,1.}. */
    for(unsigned j=0;j<32;++j)a[j]=0.0f;
    a[0]=16777216.0f;a[1]=1.0f;a[2]=-16777216.0f;
    equal(!omega_reduce_reference(OMEGA_RED_SUM,a,32,x),"tree red setup",0,32);
    equal(omega_float_to_bits(x[0])==0x3f800000 && omega_float_to_bits(omega_reduce_sequential_sum_not_contract(a,32))==0,"tree order red control",0,32);
    fesetround(FE_DOWNWARD);
    equal(omega_numeric_reference(OMEGA_NOP_FADD,a,b,c,x,1)==numeric_rest_c_reference(OMEGA_NOP_FADD,a,b,c,y,1),"FPenv",0,0);
    fesetround(FE_TONEAREST);
    puts("NUMERIC REST OM DIFF PASS; host only; chip NOT_RUN");
    printf("checks=%u\n",checks);
    return 0;
}
