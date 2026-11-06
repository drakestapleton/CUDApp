/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include "omega_gpu_decode_attn256_api.h"
#include "omega_gpu_matmul_api.h"
#include "omega_native_artifact.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;} }while(0)
int om_cubin(const uint8_t*,size_t,const char*,const uint8_t*,size_t);
int c_cubin(const uint8_t*,size_t,const char*,const uint8_t*,size_t);
int om_layout(const uint8_t*,size_t);int c_layout(const uint8_t*,size_t);
int om_version(const char*,const char*);int c_version(const char*,const char*);
size_t om_json(unsigned char,unsigned char*);size_t c_json(unsigned char,unsigned char*);
void om_oracle(uint32_t,uint32_t,uint32_t,const uint16_t*,const uint16_t*,const float*,OmegaGpuMatmulInfo*);
void c_oracle(uint32_t,uint32_t,uint32_t,const uint16_t*,const uint16_t*,const float*,OmegaGpuMatmulInfo*);
static void le(uint8_t*b,size_t o,uint64_t v,unsigned n){for(unsigned i=0;i<n;i++)b[o+i]=(uint8_t)(v>>(8*i));}
static uint32_t perm(uint32_t i,uint32_t r){uint32_t p=r/2;if(i<128)return i<p?i:r+(i-p);uint32_t j=i-128;return j<p?p+j:i;}
static int scalars(void){
    const uint32_t lows[]={0,1,0x7ffe,0x7fff,0x8000,0x8001,0xfffe,0xffff};
    for(uint32_t sign=0;sign<2;sign++)for(uint32_t e=0;e<256;e++)for(uint32_t hi=0;hi<128;hi++)for(size_t j=0;j<sizeof lows/sizeof*lows;j++){
        uint32_t u=(sign<<31)|(e<<23)|(hi<<16)|lows[j];uint64_t got=0;
        uint16_t want=(uint16_t)((u+0x7fffu+((u>>16)&1u))>>16);
        CHECK(omega_ds_round(&got,u)==OMEGA_OK);CHECK((uint16_t)got==want);
        float f;memcpy(&f,&u,4);CHECK(omega_dattn_bf16_rne(f)==want);
    }
    float s[256],d[256];uint16_t b[256],v[256];
    for(unsigned i=0;i<256;i++){uint32_t bits=0x7f800000u+i;memcpy(s+i,&bits,4);b[i]=(uint16_t)i;}
    for(uint32_t r=2;r<=256;r+=2){omega_dattn256_perm_vec(s,d,r);omega_dattn256_perm_vec_bf16(b,v,r);for(uint32_t i=0;i<256;i++){
        CHECK(omega_dattn256_perm_slot(i,r)==perm(i,r));CHECK(!memcmp(d+i,s+perm(i,r),4));CHECK(v[i]==b[perm(i,r)]);
    }}
    const uint32_t edge[]={0,1,127,128,129,255,256,UINT32_MAX};
    for(size_t i=0;i<sizeof edge/sizeof*edge;i++)for(size_t j=0;j<sizeof edge/sizeof*edge;j++)CHECK(omega_dattn256_perm_slot(edge[i],edge[j])==perm(edge[i],edge[j]));
    puts("PASS decode all exponent/sign/high-payload classes and rounding/NaN payload edges; permutation full valid domain");return 0;
}
static int artifacts(void){
    unsigned char x[32],y[32];for(unsigned c=0;c<256;c++){size_t a=om_json(c,x),b=c_json(c,y);CHECK(a==b&&!memcmp(x,y,a));}
    const char*vs[]={"0.0.0","0.0.1","1.0.0","1.2.3","1.2.4","4294967295.0.0","4294967296.0.0","01.2.3","1.2","1.2.3x","-1.2.3",""};
    for(size_t i=0;i<sizeof vs/sizeof*vs;i++)for(size_t j=0;j<sizeof vs/sizeof*vs;j++)CHECK(om_version(vs[i],vs[j])==c_version(vs[i],vs[j]));
    const char*keys[]={"artifact_id","op","shape","bounds","contract","contract_id","semantic_spec_id","isa","machine_profile","kernel_format","kernel_function","text_sha256","text_size","cubin_sha256","cubin_size","qmd0_sha256","qmd1_sha256","qmd_template","cbank_offset","cbank_words","cbank_args","cbank_args_sha256","grid","block","shared_bytes","gpr_count","precision","accumulation","tolerance","min_driver","requires","qualification","provenance_source_commit","provenance_tool"};
    uint8_t manifest[8193]={0};size_t len=(size_t)sprintf((char*)manifest,OMEGA_ARTIFACT_FORMAT "\n");
    for(size_t i=0;i<sizeof keys/sizeof*keys;i++)len+=(size_t)sprintf((char*)manifest+len,"%s=v\n",keys[i]);
    CHECK(om_layout(manifest,len)==1&&c_layout(manifest,len)==1);
    for(size_t n=0;n<=len+1;n++)CHECK(om_layout(manifest,n)==c_layout(manifest,n));
    for(size_t i=0;i<len;i++){uint8_t old=manifest[i];const uint8_t changes[]={0,13,10,'='};for(size_t j=0;j<sizeof changes;j++){manifest[i]=changes[j];CHECK(om_layout(manifest,len)==c_layout(manifest,len));}manifest[i]=old;}
    CHECK(!om_layout(manifest,8192));
    uint8_t cub[340]={0},text[]={1,2,3,4};memcpy(cub,"\177ELF",4);le(cub,18,190,2);le(cub,40,64,8);le(cub,58,64,2);le(cub,60,3,2);le(cub,62,1,2);
    le(cub,128+24,256,8);le(cub,128+32,32,8);memcpy(cub+256,"\0.text.foo\0",11);le(cub,192,1,4);le(cub,192+24,320,8);le(cub,192+32,4,8);memcpy(cub+320,text,4);
    CHECK(om_cubin(cub,sizeof cub,"foo",text,4)==1&&c_cubin(cub,sizeof cub,"foo",text,4)==1);
    for(size_t n=0;n<=sizeof cub;n++)CHECK(om_cubin(cub,n,"foo",text,4)==c_cubin(cub,n,"foo",text,4));
    for(size_t i=0;i<sizeof cub;i++){uint8_t old=cub[i];const uint8_t vals[]={0,1,63,64,127,128,254,255};for(size_t j=0;j<sizeof vals;j++){cub[i]=vals[j];CHECK(om_cubin(cub,sizeof cub,"foo",text,4)==c_cubin(cub,sizeof cub,"foo",text,4));}cub[i]=old;}
    char fn[130];memset(fn,'f',sizeof fn);for(size_t n=120;n<sizeof fn;n++){fn[n]=0;CHECK(om_cubin(cub,sizeof cub,fn,text,4)==c_cubin(cub,sizeof cub,fn,text,4));fn[n]='f';}
    for(unsigned n=2;n<=8;n*=2){uint64_t r=0;CHECK(omega_na_read(&r,cub,n,n)==OMEGA_OK);uint64_t want=0;for(unsigned i=0;i<n;i++)want|=(uint64_t)cub[i]<<(8*i);CHECK(r==want);}
    text[0]^=1;CHECK(!om_cubin(cub,sizeof cub,"foo",text,4));
    puts("PASS artifact differential: JSON bytes, strict versions, manifest boundaries, ELF bounds/name/text mutations; red text mismatch refused");return 0;
}
static int matmul(void){
    uint16_t a[96],b[96];float c[9];const uint16_t vals[]={0,0x8000,1,0x7f,0x80,0x3f80,0xbf80,0x7f7f,0xff7f,0x7f80,0xff80,0x7f81,0x7fc0,0xffff};
    for(unsigned kind=0;kind<sizeof vals/sizeof*vals;kind++)for(unsigned k=0;k<=32;k++){
        for(unsigned i=0;i<96;i++){a[i]=vals[(kind+i)% (sizeof vals/sizeof*vals)];b[i]=vals[(kind+i*3)%(sizeof vals/sizeof*vals)];}
        for(unsigned i=0;i<9;i++)c[i]=(float)i;
        OmegaGpuMatmulInfo x={0},y={0};om_oracle(3,k,3,a,b,c,&x);c_oracle(3,k,3,a,b,c,&y);
        CHECK(!memcmp(&x.max_abs_err,&y.max_abs_err,sizeof(float)));CHECK(!memcmp(&x.max_rel_err,&y.max_rel_err,sizeof(float)));
        CHECK(x.mismatch_count==y.mismatch_count&&x.parity_verified==y.parity_verified&&x.oracle_ran==y.oracle_ran);
    }
    for(unsigned i=0;i<96;i++)a[i]=b[i]=0x3f80;c[0]=1.0f;
    OmegaGpuMatmulInfo x={0},y={0};om_oracle(1,1,1,a,b,c,&x);CHECK(x.parity_verified);c[0]=2.0f;om_oracle(1,1,1,a,b,c,&y);CHECK(!y.parity_verified&&y.mismatch_count==1);
    const uint32_t near[]={0x3f800053,0x3f800054,0x3f800055};
    for(unsigned i=0;i<sizeof near/sizeof*near;i++){
        memcpy(c,&near[i],4);memset(&x,0,sizeof x);memset(&y,0,sizeof y);
        om_oracle(1,1,1,a,b,c,&x);c_oracle(1,1,1,a,b,c,&y);
        CHECK(x.mismatch_count==y.mismatch_count&&x.mismatch_count==(i!=0));
        CHECK(!memcmp(&x.max_abs_err,&y.max_abs_err,sizeof(float)));CHECK(!memcmp(&x.max_rel_err,&y.max_rel_err,sizeof(float)));
    }
    puts("PASS matmul double oracle float classes and error reduction; red wrong output rejected");return 0;
}
int main(void){CHECK(!scalars());CHECK(!artifacts());CHECK(!matmul());puts("API-REST-DIFF PASS");return 0;}
