/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_core.h"
#include "omega_canonical.h"
#include "omega_vector.h"
#include "omega_gpu_hostmath.h"
#include "omega_gpu_fmt.h"
#include "cudapp_native.h"
#include "core_util_refs.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint64_t state=UINT64_C(0xa13e7b965240c8df);
static uint64_t rnd(void){state^=state<<13;state^=state>>7;state^=state<<17;return state;}
#define CHECK(c) do{if(!(c)){fprintf(stderr,"CORE-UTIL FAIL line %d iteration %u\n",__LINE__,t);return 1;}}while(0)
static int canonical(void){
    OmegaObject a,b;uint8_t x[32768],y[32768],data[512];SemanticId id;
    for(unsigned t=0;t<10000;t++){
        memset(&a,0,sizeof a);a.kind=(SemanticKind)(rnd()%256);b=a;
        unsigned attrs=t==9999?32:(unsigned)(rnd()%9),rels=t==9999?64:(unsigned)(rnd()%9),cons=t==9999?32:(unsigned)(rnd()%9);
        for(unsigned j=0;j<attrs;j++){
            char key[64];snprintf(key,sizeof key,"key%02u",(unsigned)(rnd()%12));
            for(unsigned k=0;k<sizeof data;k++)data[k]=(uint8_t)rnd();
            uint16_t n=(uint16_t)(rnd()%513);
            CHECK(omega_object_add_attribute(&a,key,data,n)==ref_omega_object_add_attribute(&b,key,data,n));
        }
        for(unsigned j=0;j<rels;j++){
            for(unsigned k=0;k<32;k++)id.bytes[k]=(uint8_t)rnd();
            RelationKind k=(RelationKind)(rnd()%16);
            CHECK(omega_object_add_relation(&a,k,&id)==ref_omega_object_add_relation(&b,k,&id));
        }
        for(unsigned j=0;j<cons;j++){
            uint16_t n=(uint16_t)(rnd()%129);ConstraintKind k=(ConstraintKind)(rnd()%16);
            CHECK(omega_object_add_constraint(&a,k,data,n)==ref_omega_object_add_constraint(&b,k,data,n));
        }
        a.payload_len=(uint32_t)(rnd()%1025);for(unsigned j=0;j<a.payload_len;j++)a.payload[j]=(uint8_t)rnd();
        b.payload_len=a.payload_len;memcpy(b.payload,a.payload,sizeof a.payload);
        CHECK(!memcmp(&a,&b,sizeof a));
        memset(x,0xa5,sizeof x);memset(y,0xa5,sizeof y);
        size_t nx=99,ny=99,cap=t%2?sizeof x:(size_t)(rnd()%4097);
        int rx=omega_canonical_encode(&a,x,cap,&nx),ry=ref_omega_canonical_encode(&b,y,cap,&ny);
        CHECK(rx==ry&&nx==ny&&!memcmp(x,y,sizeof x));
        if(t<32){omega_compute_semantic_id(&a);ref_omega_compute_semantic_id(&b);CHECK(!memcmp(&a,&b,sizeof a));}
    }
    for(unsigned t=0;t<65536;t++)CHECK(omega_canonical_check_header((uint8_t)t,(uint8_t)(t>>8))==ref_omega_canonical_check_header((uint8_t)t,(uint8_t)(t>>8)));
    for(unsigned t=0;t<16384;t++){
        char text[65];memset(text,'a',64);text[64]=0;text[0]=(char)(t&127);text[1]=(char)(t>>7);
        SemanticId a0,b0;memset(&a0,0xa5,sizeof a0);b0=a0;
        CHECK(omega_parse_hex_semantic_id(text,&a0)==ref_omega_parse_hex_semantic_id(text,&b0));CHECK(!memcmp(&a0,&b0,sizeof a0));
        char ax[65],bx[65];omega_hex_semantic_id(&a0,ax);ref_omega_hex_semantic_id(&b0,bx);CHECK(!memcmp(ax,bx,65));
        int cmp=omega_compare_semantic_id(&a0,&b0),ref=ref_omega_compare_semantic_id(&a0,&b0);CHECK((cmp>0)==(ref>0)&&(cmp<0)==(ref<0));
    }
    puts("CORE-UTIL canonical PASS: 10000 objects, all header pairs, 16384 parser pairs");return 0;
}
static int effects_eval(void){
    for(unsigned t=0;t<10000;t++){
        EffectPayload p,a,b;uint8_t x[178],y[178];for(unsigned j=0;j<sizeof p;j++)((uint8_t*)&p)[j]=(uint8_t)rnd();
        if(t%2){p.resource_class=1;p.param_len=(uint16_t)(rnd()%129);memset(p.param_bytes+p.param_len,0,128-p.param_len);}
        CHECK(omega_effect_payload_encode(&p,x)==ref_omega_effect_payload_encode(&p,y));CHECK(!memcmp(x,y,sizeof x));
        size_t n=t%17==0?176:t%19==0?179:178;
        CHECK(omega_effect_payload_decode(x,n,&a)==ref_omega_effect_payload_decode(y,n,&b));CHECK(!memcmp(&a,&b,sizeof a));
        uint64_t out=UINT64_C(0x9382716455aa55aa),want=out,av=rnd(),bv=rnd();
        OpCode op=(OpCode)(t%18);OverflowPolicy ov=(OverflowPolicy)(t%5);uint16_t width=(uint16_t)(t%128);
        CHECK(omega_eval_pure_binary_uint(op,ov,width,av,bv,&out)==ref_omega_eval_pure_binary_uint(op,ov,width,av,bv,&want));CHECK(out==want);
    }
    puts("CORE-UTIL effects/eval PASS: 10000 generated cases");return 0;
}
static int vectors(void){
    uint32_t a[2050],b[2050],c[2050],ar[2050],br[2050],cr[2050];
    for(unsigned t=0;t<10000;t++){
        uint32_t n=t<2051?t:(uint32_t)(rnd()%2051);
        memset(a,0xa5,sizeof a);memcpy(b,a,sizeof a);memcpy(c,a,sizeof a);memcpy(ar,a,sizeof a);memcpy(br,a,sizeof a);memcpy(cr,a,sizeof a);
        omega_vector_generate_deterministic(a,b,c,n);ref_omega_vector_generate_deterministic(ar,br,cr,n);
        CHECK(!memcmp(a,ar,sizeof a)&&!memcmp(b,br,sizeof b)&&!memcmp(c,cr,sizeof c));
        omega_vector_oracle_u32(a,b,c,n);ref_omega_vector_oracle_u32(ar,br,cr,n);CHECK(!memcmp(c,cr,sizeof c));
        if(n&&t%2)c[n/2]^=1;
        size_t ix=SIZE_MAX,iy=SIZE_MAX;
        CHECK(omega_vector_verify_oracle(a,b,c,n,&ix)==ref_omega_vector_verify_oracle(ar,br,c,n,&iy));CHECK(ix==iy);
        if(t<64){OmegaVectorSpec x,y;memset(&x,0xa5,sizeof x);y=x;char name[100];memset(name,'x',sizeof name);name[t]=0;
            CHECK(omega_vector_spec_init(&x,name,n)==ref_omega_vector_spec_init(&y,name,n));CHECK(!memcmp(&x,&y,sizeof x));}
    }
    for(unsigned t=0;t<64;t++){
        uint32_t x[32],y[32];unsigned ai=t&3,bi=(t>>2)&3,ci=(t>>4)&3;
        memset(x,0xa5,sizeof x);memcpy(y,x,sizeof x);
        omega_vector_generate_deterministic(x+ai,x+bi,x+ci,16);
        ref_omega_vector_generate_deterministic(y+ai,y+bi,y+ci,16);CHECK(!memcmp(x,y,sizeof x));
        omega_vector_oracle_u32(x+ai,x+bi,x+ci,16);
        ref_omega_vector_oracle_u32(y+ai,y+bi,y+ci,16);CHECK(!memcmp(x,y,sizeof x));
    }
    puts("CORE-UTIL vector PASS: 10000 cases, chunk and refusal boundaries");return 0;
}
static int formatting(void){
    int (*f)(char*,size_t,const char*,...)=omega_gpu_fmt;
    int (*r)(char*,size_t,const char*,...)=ref_omega_gpu_fmt;
    for(unsigned t=0;t<10000;t++){
        char a[256],b[256];memset(a,0xa5,sizeof a);memset(b,0xa5,sizeof b);size_t cap=t<257?t:(size_t)(rnd()%257);
        uint64_t u=rnd();int d=(int)rnd();const char *s=t%3?"Omega formatter":NULL;
        const char *fmt="%s/%c/%d/%u/%x/%ld/%lu/%lld/%llu/%zu/%%/%q/%llx/%zx";
        int x=f(a,cap,fmt,s,'A',d,(unsigned)u,(unsigned)(u>>32),(long)u,(unsigned long)u,(long long)u,(unsigned long long)u,(size_t)u,(unsigned long long)u,(size_t)u);
        int y=r(b,cap,fmt,s,'A',d,(unsigned)u,(unsigned)(u>>32),(long)u,(unsigned long)u,(long long)u,(unsigned long long)u,(size_t)u,(unsigned long long)u,(size_t)u);
        CHECK(x==y&&!memcmp(a,b,sizeof a));
    }
    const char *tails[]={"%","%l","%ll","%z","%q","%lll","plain", "%%"};
    for(unsigned t=0;t<sizeof tails/sizeof tails[0];t++){char a[32],b[32];memset(a,0xa5,sizeof a);memset(b,0xa5,sizeof b);CHECK(f(a,32,tails[t])==r(b,32,tails[t]));CHECK(!memcmp(a,b,sizeof a));}
    puts("CORE-UTIL fmt PASS: 10000 cases and truncated format tokens");return 0;
}
static uint64_t bits(double d){uint64_t u;memcpy(&u,&d,8);return u;}
static double real(uint64_t u){double d;memcpy(&d,&u,8);return d;}
static int same_double(double a,double b){return bits(a)==bits(b);}
static int hostmath(void){
    const uint64_t edges[]={0,UINT64_C(0x8000000000000000),1,UINT64_C(0x0010000000000000),UINT64_C(0x3ff0000000000000),
        UINT64_C(0x7fefffffffffffff),UINT64_C(0x7ff0000000000000),UINT64_C(0xfff0000000000000),UINT64_C(0x7ff8000000000001),UINT64_C(0xc008236e94ed2a03),
        UINT64_C(4609047870845172684),UINT64_C(4609047870845172685),UINT64_C(4609047870845172686),
        UINT64_C(4649454506724284169),UINT64_C(4649454506724284170),UINT64_C(4649454506724284171),
        UINT64_C(13873138101193906585),UINT64_C(13873138101193906586),UINT64_C(13873138101193906587),
        UINT64_C(4503599627370495),UINT64_C(4503599627370497)};
    for(unsigned t=0;t<10000;t++){
        uint64_t u=t<sizeof edges/sizeof edges[0]?edges[t]:rnd();double x=real(u);
        CHECK(same_double(omega_hostmath_log(x),ref_omega_hostmath_log(x)));
        CHECK(same_double(omega_hostmath_exp(x),ref_omega_hostmath_exp(x)));
        if((u&UINT64_C(0x7ff0000000000000))<UINT64_C(0x4130000000000000)){
            uint64_t ignored,out[2];uint32_t status=omega_omhost(&ignored,3,u,0,out,2);
            if(status){fprintf(stderr,"hostmath sincos input=%016llx status=%u case=%u\n",(unsigned long long)u,status,t);return 1;}
            double a,b,c,d;omega_hostmath_sincos(x,&a,&b);ref_omega_hostmath_sincos(x,&c,&d);CHECK(same_double(a,c)&&same_double(b,d));
        }
        double base=real(UINT64_C(0x3ff0000000000000)|(rnd()&UINT64_C(0x000fffffffffffff)));
        double power=real(UINT64_C(0x4000000000000000)|(rnd()&UINT64_C(0x000fffffffffffff)));
        CHECK(same_double(omega_hostmath_pow(base,power),ref_omega_hostmath_pow(base,power)));
    }
    for(unsigned t=0;t<1024;t++){int e=(int)t-512;float a=omega_hostmath_pow2f(e),b=ref_omega_hostmath_pow2f(e);CHECK(!memcmp(&a,&b,4));}
    puts("CORE-UTIL hostmath PASS: 10000 bitwise non-contracting cases and exponent boundaries");return 0;
}
static uint32_t widen(uint16_t h){
    uint32_t sign=(uint32_t)(h&0x8000)<<16,exp=(h>>10)&31,m=h&1023;
    if(exp==31)return sign|UINT32_C(0x7f800000)|(m<<13);
    if(!exp){if(!m)return sign;exp=113;while(!(m&1024)){m<<=1;exp--;}return sign|(exp<<23)|((m&1023)<<13);}
    return sign|((exp+112)<<23)|(m<<13);
}
static int f16(void){
    for(unsigned t=0;t<65536;t++){uint64_t out;uint16_t want=(uint16_t)t;if((t&0x7c00)==0x7c00&&(t&1023))want|=0x200;
        CHECK(omega_native_f16(&out,widen((uint16_t)t))==0);CHECK(out==want);}
    puts("CORE-UTIL f16 PASS: all 65536 binary16 inputs, NaNs quieted with payload preserved");return 0;
}
static int builders(void){
    OmegaGraph *a=omega_graph_create(),*b=ref_omega_graph_create();if(!a||!b)return 1;
    for(unsigned t=0;t<256;t++){
        SemanticId id;for(unsigned i=0;i<32;i++)id.bytes[i]=(uint8_t)rnd();OmegaObject *x,*y;
        switch(t%16){
#define PAIR(name,...) do{x=omega_##name(a,##__VA_ARGS__);y=ref_omega_##name(b,##__VA_ARGS__);}while(0)
            case 0:PAIR(build_type_unit);break;case 1:PAIR(build_type_bool);break;
            case 2:PAIR(build_type_uint,(uint16_t)t);break;case 3:PAIR(build_type_signed_int,(uint16_t)t);break;
            case 4:PAIR(build_type_fp32);break;case 5:PAIR(build_type_bitvector,(uint16_t)t);break;
            case 6:PAIR(build_type_byte);break;case 7:PAIR(build_type_sequence,&id,t);break;
            case 8:PAIR(build_type_cap_ref);break;case 9:PAIR(build_val_bool,&id,t%2);break;
            case 10:PAIR(build_val_uint,&id,(uint16_t)(t*3),state);break;case 11:PAIR(build_op_binary,OP_ADD,OVERFLOW_WRAP,&id);break;
            case 12:PAIR(build_apply,&id,&id,&id);break;case 13:PAIR(build_param,&id,(uint16_t)t);break;
            case 14:PAIR(build_effect,1,2,3,state);break;default:PAIR(graph_add_object,KIND_INVALID);break;
#undef PAIR
        }
        CHECK(x&&y&&!memcmp(x,y,sizeof *x));
        OmegaObject *px=omega_graph_find_object(a,&x->id),*py=ref_omega_graph_find_object(b,&y->id);
        CHECK((px==NULL)==(py==NULL));if(px)CHECK(px-a->objects==py-b->objects);
    }
    {unsigned t=256;CHECK(omega_graph_add_object(a,KIND_TYPE)==NULL);CHECK(ref_omega_graph_add_object(b,KIND_TYPE)==NULL);}
    omega_graph_destroy(a);ref_omega_graph_destroy(b);
    puts("CORE-UTIL builders PASS: graph full boundary, every constructor and aliasing ID lookup");return 0;
}
int main(int argc,char **argv){
    setvbuf(stdout,NULL,_IONBF,0);
    if(argc>1&&!strcmp(argv[1],"--hostmath"))return hostmath();
    if(argc>1&&!strcmp(argv[1],"--quick"))return canonical()||formatting()||f16();
    return canonical()||effects_eval()||vectors()||formatting()||hostmath()||f16()||builders();
}
