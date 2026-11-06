/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_cubin.h"
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define CUBIN_OM_WORDS 16777216u
#include "core_cubin_refusal_ref.inc"
static uint64_t state=UINT64_C(0x56f0138093aac175);
static uint64_t next(void){state^=state<<13;state^=state>>7;state^=state<<17;return state;}
int main(int argc,char **argv){
    int flip=argc==2&&!strcmp(argv[1],"--flip");
    OmegaCubinParam params[64];
    for(unsigned t=0;t<10000;t++){
        for(size_t i=0;i<64;i++){params[i].offset=(uint32_t)next();params[i].size=(uint32_t)next();params[i].is_pointer=(uint32_t)next();}
        uint64_t m[16]={0},d[3]={next()%8,next()%70,next()},r;
        m[0]=t%8;m[1]=t&1?OMEGA_CUBIN_SM_86:OMEGA_CUBIN_SM_121;
        m[2]=d[0];m[3]=d[1];m[4]=d[2];m[5]=next();m[6]=t%65;
        m[7]=(uint32_t)next();m[8]=(uint32_t)next();m[9]=(uint32_t)next();m[12]=t%301;
        if(d[1]<m[6]){m[10]=params[d[1]].offset;m[11]=params[d[1]].size;}
        char actual[512],expected[512],staging[256];memset(actual,0x5a,sizeof actual);memcpy(expected,actual,sizeof expected);
        ref_cubin_message((uint32_t)m[1],(int)m[0],d,(size_t)m[5],params,(size_t)m[6],(uint32_t)m[7],(uint32_t)m[8],(uint32_t)m[9],expected,(size_t)m[12]);
        uint32_t status=omega_cubmsg(&r,m,16,staging,sizeof staging);
        if(status||r>=sizeof staging){fprintf(stderr,"cubin message status=%u case=%u\n",status,t);return 1;}
        if(m[12])memcpy(actual,staging,(size_t)r+1);
        if(flip&&t==1)actual[0]^=1;
        if(memcmp(actual,expected,sizeof actual)){fprintf(stderr,"cubin message mismatch case=%u rc=%llu detail=%llu cap=%llu\n",t,(unsigned long long)m[0],(unsigned long long)d[0],(unsigned long long)m[12]);return 1;}
    }
    for(uint32_t s=0;s<64;s++){
        uint64_t m[16]={8,0,s},r;m[12]=256;
        char actual[256],expected[256];snprintf(expected,sizeof expected,"cubin_elf.om stopped with status %u",s);
        if(omega_cubmsg(&r,m,16,actual,sizeof actual)||strcmp(actual,expected)){fputs("cubin bridge status message mismatch\n",stderr);return 1;}
    }
    puts("CORE-UTIL cubin messages PASS: 10000 differential cases, truncation and bridge status boundaries");return 0;
}
