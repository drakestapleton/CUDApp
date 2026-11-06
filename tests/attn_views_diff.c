/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include "attn_views_fixture.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
static void oracle(uint32_t *a,const uint64_t *p,size_t na,size_t nw){
    memset(a,0,32*sizeof *a);
    for(size_t i=0;i<na;i++){a[2*i]=(uint32_t)p[i];a[2*i+1]=(uint32_t)(p[i]>>32);}
    for(size_t i=0;i<nw-2*na;i++)a[2*na+i]=(uint32_t)p[na+i];
}
static int floats_equal(const float *a,const float *b,size_t n){
    for(size_t i=0;i<n;i++){
        uint32_t x,y;memcpy(&x,a+i,4);memcpy(&y,b+i,4);
        int xn=(x&0x7f800000u)==0x7f800000u&&(x&0x7fffffu);
        int yn=(y&0x7f800000u)==0x7f800000u&&(y&0x7fffffu);
        if(xn||yn){if(!xn||!yn)return 0;}else if(x!=y){
            fprintf(stderr,"float mismatch at %zu: %08x != %08x\n",i,x,y);return 0;
        }
    }
    return 1;
}
static int fixture_equal(const AttnViewsFixture *a,const AttnViewsFixture *b){
    size_t tail=offsetof(AttnViewsFixture,result);
    return !memcmp(a,b,offsetof(AttnViewsFixture,ff))&&
        floats_equal(a->ff,b->ff,AV_N)&&floats_equal(a->rope,b->rope,1024)&&
        floats_equal(a->fold,b->fold,32)&&
        !memcmp((const char*)a+tail,(const char*)b+tail,sizeof *a-tail);
}
int main(void){
    uint64_t p[32],r=99;uint32_t got[32],want[32];
    for(size_t na=0;na<=16;na++)for(size_t nw=2*na;nw<=32;nw++){
        for(size_t i=0;i<32;i++)p[i]=UINT64_C(0xdeadbeef12345678)^((uint64_t)i<<40);
        oracle(want,p,na,nw);
        if(omega_av_pack(&r,p,32,got,32,na,nw)||r||memcmp(got,want,sizeof got))return 1;
    }
    if(omega_av_pack(&r,p,32,got,32,17,32)||r!=1)return 2;
    if(omega_av_pack(&r,p,32,got,32,2,3)||r!=1)return 3;
    if(omega_av_pack(&r,p,32,got,32,0,33)||r!=1)return 4;
    if(omega_av_pack(&r,p,31,got,32,0,0)==0)return 5;
    if(omega_av_pack(&r,p,32,(uint32_t*)p,32,0,0)==0)return 6;
    oracle(want,p,4,16);want[1]^=1;
    if(omega_av_pack(&r,p,32,got,32,4,16)||!memcmp(got,want,sizeof got))return 7;
    AttnViewsFixture *a=calloc(1,sizeof *a),*b=calloc(1,sizeof *b);
    if(!a||!b)return 8;
    attn_omega(a);attn_oracle(b);
    if(!fixture_equal(a,b)){
        for(size_t i=0;i<sizeof *a;i++)if(((unsigned char*)a)[i]!=((unsigned char*)b)[i]){
            fprintf(stderr,"first fixture byte mismatch at %zu: %02x != %02x\n",i,((unsigned char*)a)[i],((unsigned char*)b)[i]);break;
        }
        for(size_t i=0;i<32;i++)if(a->result[i]!=b->result[i])
            fprintf(stderr,"result[%zu]: %llu != %llu\n",i,(unsigned long long)a->result[i],(unsigned long long)b->result[i]);
        fputs("FAIL host-data fixture differential\n",stderr);return 9;
    }
    b->args[1]^=1;
    if(fixture_equal(a,b))return 10;
    free(a);free(b);
    unsigned char bytes[32]={0};uint64_t q[16]={0};
    if(omega_av_copy(&r,NULL,0,NULL,0,0)||r)return 11;
    if(omega_av_copy(&r,bytes,16,bytes+1,16,16)!=32)return 12;
    if(omega_av_copy(&r,bytes,16,bytes+16,16,16)||r)return 13;
    if(omega_av_copy((uint64_t*)bytes,bytes,8,bytes+16,8,8)!=32)return 14;
    if(omega_av_copy(&r,(void*)(UINTPTR_MAX-1),3,bytes,3,3)!=32)return 15;
    if(omega_av_copy(&r,bytes,2,bytes+16,2,3)!=3)return 16;
    if(omega_av_gath(&r,bytes,8,bytes+16,8,q,2)!=3)return 17;
    /* Admission above the former fixed capacity, backed by a real unread mapping. */
    size_t large_bytes=(size_t)16777217*sizeof(uint32_t);
    void *large=mmap(NULL,large_bytes,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    if(large==MAP_FAILED)return 18;
    int large_ok=omega_av_max(&r,large,16777217,0)==0 && r==1;
    munmap(large,large_bytes);if(!large_ok)return 18;
    if(omega_av_bfexp(&r,bytes+1,1,got,1,1)!=32)return 19;
    uint16_t nanbf[2]={0x7f81,0xffc1};float nanout[2],nanwant[2];
    uint32_t nanbits[2]={0x7fc00000,0xffc00000};
    memcpy(nanwant,nanbits,sizeof nanbits);
    if(omega_av_bfexp(&r,nanbf,2,nanout,2,2)||r||!floats_equal(nanout,nanwant,2))return 20;
    q[0]=0;q[3]=0;
    if(omega_av_table(&r,got,1,got,1,q,16,0)!=32)return 21;
    if(omega_av_touch(&r,got,1,got+2,1,got,1,0)!=32)return 22;
    puts("PASS attention host typed views differential; red check rejected; runtime bounds and aliases refused");
    return 0;
}
