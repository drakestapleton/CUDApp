/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_matmul.h"
#include "omega_blackwell_realize.h"
#include "omega_gpu_code_alloc.h"
#include "sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifndef CUDAPP_MIGRATION_PENDING_BW_MATMUL
#error This differential requires the default-OFF independent C oracle
#endif
int oracle_omega_matmul_spec_init(OmegaMatMulSpec *, uint32_t, uint32_t, uint32_t, OmegaMatMulPrecision);
float oracle_omega_fp16_to_fp32(uint16_t);
float oracle_omega_bf16_to_fp32(uint16_t);
uint16_t oracle_omega_fp32_to_fp16(float);
uint16_t oracle_omega_fp32_to_bf16(float);
int oracle_omega_blackwell_bind_matmul_realization(const OmegaMatMulSpec *, const OmegaBlackwellKernel *, OmegaBlackwellRealizationIdentity *);
int oracle_omega_blackwell_get_machine_id(SemanticId *);
int oracle_omega_blackwell_realization_compute_id(OmegaBlackwellRealization *);
int oracle_omega_blackwell_realize_vector(const OmegaVectorSpec *, OmegaBlackwellRealization *);
int oracle_omega_blackwell_codegen_matmul(const OmegaMatMulSpec *, OmegaBlackwellKernel *);
void oracle_omega_blackwell_kernel_free(OmegaBlackwellKernel *);
int oracle_omega_matmul_cpu_oracle_i32(const uint32_t *, const uint32_t *, uint32_t *, uint32_t, uint32_t, uint32_t);
int oracle_omega_matmul_cpu_oracle_f16(const uint16_t *, const uint16_t *, float *, uint32_t, uint32_t, uint32_t);
int oracle_omega_matmul_cpu_oracle_bf16(const uint16_t *, const uint16_t *, float *, uint32_t, uint32_t, uint32_t);
static unsigned checks;
#define CHECK(c) do { ++checks; if (!(c)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#c); return 1; } } while(0)
static uint32_t bits(float f) { uint32_t b; memcpy(&b,&f,4); return b; }
static float value(uint32_t b) { float f; memcpy(&f,&b,4); return f; }
static int nan16(uint16_t h) { return (h&0x7c00)==0x7c00 && (h&0x3ff); }
static int nanbf(uint16_t h) { return (h&0x7f80)==0x7f80 && (h&0x7f); }
static uint32_t rng = 0x6123abcd;
static uint32_t next(void) { rng ^= rng<<13; rng ^= rng>>17; rng ^= rng<<5; return rng; }
static int same_float(float a, float b) {
    uint32_t x=bits(a), y=bits(b);
    return x==y || ((x&0x7fffffff)>0x7f800000 && (y&0x7fffffff)>0x7f800000);
}
static int cpu_views(void) {
    uint32_t a[81], b[81], c[81], expected[81];
    uint16_t h[81], q[81];
    float out[81], want[81];
    uint64_t result=0;
    uint8_t name[64];
    uint64_t length=0;
    for(unsigned i=0;i<64;i++) {
        memset(name,0x61,sizeof name); name[i]=0;
        CHECK(omega_bw_name(&result,name,sizeof name,&length,1)==0 && result==0);
        CHECK(length==strnlen((const char *)name,sizeof name));
    }
    memset(name,0x61,sizeof name); length=123;
    CHECK(omega_bw_name(&result,name,sizeof name,&length,1)==0 && result==UINT64_MAX && length==123);
    CHECK(omega_bw_name(&result,name,63,&length,1)!=0);
    for(unsigned trial=0;trial<100;trial++) {
        uint32_t m=1+next()%9,k=1+next()%9,n=1+next()%9;
        for(unsigned i=0;i<81;i++) { a[i]=next(); b[i]=next(); h[i]=(uint16_t)next(); q[i]=(uint16_t)next(); }
        CHECK(omega_matmul_cpu_oracle_i32(a,b,c,m,k,n)==0);
        CHECK(oracle_omega_matmul_cpu_oracle_i32(a,b,expected,m,k,n)==0);
        CHECK(memcmp(c,expected,(size_t)m*n*sizeof(*c))==0);
        CHECK(omega_matmul_cpu_oracle_f16(h,q,out,m,k,n)==0);
        CHECK(oracle_omega_matmul_cpu_oracle_f16(h,q,want,m,k,n)==0);
        for(unsigned i=0;i<m*n;i++) CHECK(same_float(out[i],want[i]));
        CHECK(omega_matmul_cpu_oracle_bf16(h,q,out,m,k,n)==0);
        CHECK(oracle_omega_matmul_cpu_oracle_bf16(h,q,want,m,k,n)==0);
        for(unsigned i=0;i<m*n;i++) CHECK(same_float(out[i],want[i]));
        m=next()|1; k=next()|1; n=next()|1;
        CHECK(omega_bw_shape(&result,(uint64_t)m*k,(uint64_t)k*n,(uint64_t)m*n)==0);
        CHECK(result==n);
    }
    CHECK(omega_bw_shape(&result,(uint64_t)UINT32_MAX*UINT32_MAX,(uint64_t)UINT32_MAX*UINT32_MAX,(uint64_t)UINT32_MAX*UINT32_MAX)==0 && result==UINT32_MAX);
    const uint64_t invalid[][3]={{1,2,3},{2,1,8},{3,2,2},{1,UINT64_MAX,2},{UINT64_MAX,UINT64_MAX,1},{UINT64_MAX,1,UINT64_MAX},{0,1,1},{1,0,1},{1,1,0}};
    for(unsigned i=0;i<sizeof invalid/sizeof invalid[0];i++)
        CHECK(omega_bw_shape(&result,invalid[i][0],invalid[i][1],invalid[i][2])==0 && result==0);
    /* Red controls: a non-square quotient and k not divisible by recovered n. */
    for(unsigned i=0;i<2;i++) {
        memset(c,0x5a,sizeof c); memcpy(expected,c,sizeof c);
        memset(out,0x5a,sizeof out); memcpy(want,out,sizeof out);
        CHECK(omega_bw_cpui(&result,a,invalid[i][0],b,invalid[i][1],c,invalid[i][2])==0 && result==UINT64_MAX);
        CHECK(memcmp(c,expected,sizeof c)==0);
        CHECK(omega_bw_cpuf16(&result,h,invalid[i][0],q,invalid[i][1],out,invalid[i][2])==0 && result==UINT64_MAX);
        CHECK(memcmp(out,want,sizeof out)==0);
        CHECK(omega_bw_cpubf16(&result,h,invalid[i][0],q,invalid[i][1],out,invalid[i][2])==0 && result==UINT64_MAX);
        CHECK(memcmp(out,want,sizeof out)==0);
    }
    CHECK(omega_bw_cpui(&result,a,1,b,1,a,1)!=0);
    CHECK(omega_matmul_cpu_oracle_i32(a,b,a,1,1,1)!=0);
    CHECK(omega_matmul_cpu_oracle_f16((const uint16_t *)a,q,(float *)a,1,1,1)!=0);
    CHECK(omega_matmul_cpu_oracle_bf16((const uint16_t *)a,q,(float *)a,1,1,1)!=0);
    CHECK(omega_matmul_cpu_oracle_i32(a,b,a+1,1,2,2)!=0);
    /* Read-only inputs may alias; only writable output must be disjoint. */
    CHECK(omega_matmul_cpu_oracle_i32(a,a,c,1,2,1)==0);
    CHECK(oracle_omega_matmul_cpu_oracle_i32(a,a,expected,1,2,1)==0 && c[0]==expected[0]);
    CHECK(omega_matmul_cpu_oracle_f16(h,h,out,1,2,1)==0);
    CHECK(oracle_omega_matmul_cpu_oracle_f16(h,h,want,1,2,1)==0 && same_float(out[0],want[0]));
    CHECK(omega_matmul_cpu_oracle_bf16(h,h,out,1,2,1)==0);
    CHECK(oracle_omega_matmul_cpu_oracle_bf16(h,h,want,1,2,1)==0 && same_float(out[0],want[0]));
    /* Exercise a dot beyond the implicit one-million runtime-loop guard. */
    size_t count=1000001;
    uint32_t *long_a=calloc(count,sizeof(*long_a)),*long_b=calloc(count,sizeof(*long_b));
    CHECK(long_a && long_b);
    long_a[count-1]=7; long_b[count-1]=9;
    CHECK(omega_matmul_cpu_oracle_i32(long_a,long_b,c,1,(uint32_t)count,1)==0 && c[0]==63);
    free(long_a); free(long_b);
    return 0;
}
static uint64_t policy(unsigned op,uint64_t x,uint64_t y,uint64_t z,uint64_t w,uint64_t v) {
    switch(op) {
    case 4: {
        if (x>UINT64_MAX-y-4095) return 0;
        uint64_t b=(x+y+4095)&~UINT64_C(4095);
        return b<4096?4096:b;
    }
    case 7: return y==0?x:UINT64_MAX;
    case 8: return x==0 && y==0?0:UINT64_MAX;
    case 9: { int64_t s; memcpy(&s,&x,8); return s>0?(uint64_t)(s-1):UINT64_MAX; }
    case 10: return x?2:3;
    case 11: return x?14:13;
    case 12: return y==0?x:0;
    case 13: return x?1:0;
    case 14: return x<=32768 && y<=16384 && z==384 && w==384 && v<=128;
    case 15: return x<=4096;
    default: return UINT64_MAX;
    }
}
int main(void) {
    for (uint32_t h=0;h<65536;h++) {
        CHECK(bits(omega_fp16_to_fp32(h))==bits(oracle_omega_fp16_to_fp32(h)));
        CHECK(bits(omega_bf16_to_fp32(h))==bits(oracle_omega_bf16_to_fp32(h)));
        float f=oracle_omega_fp16_to_fp32(h);
        uint16_t a=omega_fp32_to_fp16(f),b=oracle_omega_fp32_to_fp16(f);
        CHECK(a==b || (nan16(a)&&nan16(b)));
    }
    const uint32_t edges[]={0,0x80000000,1,0x7fffff,0x800000,0x33000000,0x33800000,0x387fc000,0x38800000,0x477fe000,0x47800000,0x7f800000,0xff800000,0x7f800001,0xffc12345,0x3f808000,0x3f818000};
    for(unsigned i=0;i<100000;i++) {
        uint32_t u=i<sizeof edges/sizeof *edges?edges[i]:next();
        float f=value(u);
        uint16_t a=omega_fp32_to_fp16(f),b=oracle_omega_fp32_to_fp16(f);
        CHECK(a==b || (nan16(a)&&nan16(b)));
        a=omega_fp32_to_bf16(f); b=oracle_omega_fp32_to_bf16(f);
        CHECK(a==b || (nanbf(a)&&nanbf(b)));
        CHECK(a==omega_fp32_to_bf16_rne(f));
    }
    const uint64_t limits[]={0,1,127,128,129,383,384,385,2047,2048,2049,4095,4096,4097,16384,16385,32768,32769,UINT64_MAX-6143,UINT64_MAX-6142,INT64_MAX,UINT64_C(1)<<63,UINT64_MAX};
    const unsigned ops[]={4,7,8,9,10,11,12,13,14,15,99};
    for(unsigned o=0;o<sizeof ops/sizeof *ops;o++) for(unsigned i=0;i<sizeof limits/sizeof *limits;i++) {
        uint64_t x=limits[i];
        for(unsigned j=0;j<sizeof limits/sizeof *limits;j++) {
            uint64_t y=ops[o]==4?(j&1?2048:0):limits[j];
            CHECK(omega_bw_value(ops[o],x,y,384,384,128)==policy(ops[o],x,y,384,384,128));
        }
    }
    for(unsigned i=0;i<10000;i++) {
        uint64_t x=next(),y=next(),z=next(),w=next(),v=next();
        unsigned op=ops[i%(sizeof ops/sizeof *ops)];
        CHECK(omega_bw_value(op,x,y,z,w,v)==policy(op,x,y,z,w,v));
    }
    CHECK(omega_bw_value(14,32768,16384,384,384,128)==1);
    CHECK(omega_bw_value(14,32768,16384,383,384,128)==0);
    CHECK(omega_bw_value(14,32768,16384,384,383,128)==0);
    CHECK(omega_bw_value(14,32768,16384,384,384,129)==0);
    uint8_t data[256],hash[32],want[32];
    for(unsigned i=0;i<sizeof data;i++) data[i]=(uint8_t)next();
    for(unsigned n=0;n<=256;n++) {
        CHECK(omega_bw_id(data,n,hash)==0);
        sha256_hash(data,n,want);
        CHECK(memcmp(hash,want,32)==0);
    }
    memset(hash,0xa5,sizeof hash); uint8_t saved[32]; memcpy(saved,hash,32);
    uint64_t result=123;
    CHECK(omega_bw_identity(&result,data,255,16,hash,32)==32);
    CHECK(result==123 && memcmp(saved,hash,32)==0);
    CHECK(omega_bw_id(data,257,hash)==-1);
    CHECK(omega_bw_identity(&result,data,256,16,data,32)==32);
    const uint32_t dims[]={0,1,2,8192,8193,16384,16385,262144,262145,UINT32_MAX};
    for(unsigned i=0;i<sizeof dims/sizeof *dims;i++) for(unsigned j=0;j<sizeof dims/sizeof *dims;j++) for(unsigned k=0;k<sizeof dims/sizeof *dims;k++) {
        OmegaMatMulSpec a={0},b={0};
        int x=omega_matmul_spec_init(&a,dims[i],dims[j],dims[k],(OmegaMatMulPrecision)next());
        int y=oracle_omega_matmul_spec_init(&b,dims[i],dims[j],dims[k],a.precision);
        /* Refused calls need not initialize the precision field. */
        CHECK(x==y);
        if(x==0) CHECK(memcmp(&a,&b,sizeof a)==0);
    }
    CHECK(omega_matmul_spec_init(NULL,1,1,1,0)==-1);
    CHECK(oracle_omega_matmul_spec_init(NULL,1,1,1,0)==-1);
    for(unsigned i=0;i<100;i++) {
        OmegaBlackwellRealization a={0},b={0};
        uint8_t *p=(uint8_t *)&a;
        for(unsigned j=0;j<sizeof a;j++) p[j]=(uint8_t)next();
        b=a;
        CHECK(omega_blackwell_realization_compute_id(&a)==oracle_omega_blackwell_realization_compute_id(&b));
        CHECK(memcmp(&a,&b,sizeof a)==0);
        OmegaMatMulSpec s; OmegaBlackwellKernel kern={0}; OmegaBlackwellRealizationIdentity ia,ib;
        CHECK(omega_matmul_spec_init(&s,1,1,1,0)==0);
        for(unsigned j=0;j<32;j++) kern.code_digest[j]=(uint8_t)next();
        CHECK(omega_blackwell_bind_matmul_realization(&s,&kern,&ia)==0);
        CHECK(oracle_omega_blackwell_bind_matmul_realization(&s,&kern,&ib)==0);
        CHECK(memcmp(&ia,&ib,sizeof ia)==0);
    }
    SemanticId a,b;
    CHECK(omega_blackwell_get_machine_id(&a)==0 && oracle_omega_blackwell_get_machine_id(&b)==0);
    CHECK(memcmp(&a,&b,sizeof a)==0);
    CHECK(omega_blackwell_get_machine_id(NULL)==-1 && oracle_omega_blackwell_get_machine_id(NULL)==-1);
    CHECK(omega_blackwell_realization_compute_id(NULL)==-1 && oracle_omega_blackwell_realization_compute_id(NULL)==-1);
    OmegaVectorSpec vs={0}; OmegaBlackwellRealization ra,rb;
    const uint32_t counts[]={0,1,63,64,65,UINT32_MAX-63,UINT32_MAX};
    for(unsigned i=0;i<sizeof counts/sizeof *counts;i++) {
        vs.element_count=counts[i];
        CHECK(omega_blackwell_realize_vector(&vs,&ra)==oracle_omega_blackwell_realize_vector(&vs,&rb));
        CHECK(memcmp(&ra,&rb,sizeof ra)==0);
    }
    CHECK(omega_blackwell_realize_vector(NULL,&ra)==-1 && oracle_omega_blackwell_realize_vector(NULL,&rb)==-1);
    CHECK(omega_blackwell_realize_vector(&vs,NULL)==-1 && oracle_omega_blackwell_realize_vector(&vs,NULL)==-1);
    OmegaMatMulSpec spec; OmegaBlackwellKernel ka={0},kb={0};
    CHECK(omega_matmul_spec_init(&spec,1,1,1,0)==0);
    CHECK(omega_blackwell_bind_matmul_realization(NULL,&ka,(OmegaBlackwellRealizationIdentity *)&ra)==-1);
    CHECK(omega_blackwell_bind_matmul_realization(&spec,NULL,(OmegaBlackwellRealizationIdentity *)&ra)==-1);
    CHECK(omega_blackwell_bind_matmul_realization(&spec,&ka,NULL)==-1);
    CHECK(oracle_omega_blackwell_bind_matmul_realization(NULL,&ka,(OmegaBlackwellRealizationIdentity *)&ra)==-1);
    CHECK(oracle_omega_blackwell_bind_matmul_realization(&spec,NULL,(OmegaBlackwellRealizationIdentity *)&ra)==-1);
    CHECK(oracle_omega_blackwell_bind_matmul_realization(&spec,&ka,NULL)==-1);
    for(unsigned p=0;p<4;p++) {
        spec.precision=(OmegaMatMulPrecision)p;
        int x=omega_blackwell_codegen_matmul(&spec,&ka),y=oracle_omega_blackwell_codegen_matmul(&spec,&kb);
        CHECK(x==y);
        if(x==0) { CHECK(ka.code_size==kb.code_size); CHECK(memcmp(ka.code,kb.code,ka.code_size)==0); }
        omega_blackwell_kernel_free(&ka); oracle_omega_blackwell_kernel_free(&kb);
    }
    CHECK(omega_blackwell_codegen_matmul(NULL,&ka)==-1 && oracle_omega_blackwell_codegen_matmul(NULL,&kb)==-1);
    CHECK(omega_blackwell_codegen_matmul(&spec,NULL)==-1 && oracle_omega_blackwell_codegen_matmul(&spec,NULL)==-1);
    omega_blackwell_kernel_free(NULL); oracle_omega_blackwell_kernel_free(NULL);
    uint32_t ai[6]={1,2,3,4,UINT32_MAX,6},bi[6]={6,5,4,3,2,1},ci[4],di[4];
    uint16_t ah[6]={0,0x3c00,0xbc00,1,0x4000,0x8000},bh[6]={0x3c00,0x4000,0xbc00,0,0x3c00,1}; float cf[4],df[4];
    CHECK(omega_matmul_cpu_oracle_i32(ai,bi,ci,2,3,2)==oracle_omega_matmul_cpu_oracle_i32(ai,bi,di,2,3,2));
    CHECK(memcmp(ci,di,sizeof ci)==0);
    CHECK(omega_matmul_cpu_oracle_f16(ah,bh,cf,2,3,2)==oracle_omega_matmul_cpu_oracle_f16(ah,bh,df,2,3,2));
    CHECK(memcmp(cf,df,sizeof cf)==0);
    CHECK(omega_matmul_cpu_oracle_bf16(ah,bh,cf,2,3,2)==oracle_omega_matmul_cpu_oracle_bf16(ah,bh,df,2,3,2));
    CHECK(memcmp(cf,df,sizeof cf)==0);
#define BAD_ORACLE(fn,a,b,c) do { CHECK(fn(NULL,b,c,1,1,1)==-1); CHECK(fn(a,NULL,c,1,1,1)==-1); CHECK(fn(a,b,NULL,1,1,1)==-1); CHECK(fn(a,b,c,0,1,1)==-1); CHECK(fn(a,b,c,1,0,1)==-1); CHECK(fn(a,b,c,1,1,0)==-1); } while(0)
    BAD_ORACLE(oracle_omega_matmul_cpu_oracle_i32,ai,bi,ci);
    BAD_ORACLE(oracle_omega_matmul_cpu_oracle_f16,ah,bh,cf);
    BAD_ORACLE(oracle_omega_matmul_cpu_oracle_bf16,ah,bh,cf);
    BAD_ORACLE(omega_matmul_cpu_oracle_i32,ai,bi,ci);
    BAD_ORACLE(omega_matmul_cpu_oracle_f16,ah,bh,cf);
    BAD_ORACLE(omega_matmul_cpu_oracle_bf16,ah,bh,cf);
    CHECK(cpu_views()==0);
#ifdef OMEGA_BW_DIFF_RED
    hash[0] ^= 1;
    CHECK(memcmp(hash,saved,32)==0);
#endif
    printf("PASS bw matmul realization differential checks=%u\n",checks);
    return 0;
}
