/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host-only matched work. Nanosecond samples, not chip timings. */
#include "cudapp_native.h"
#include "omega_blackwell_codegen.h"
#define CUDAPP_MIGRATION_PENDING_C_API_SHELLS 1
#include "omega_api_shells.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static volatile unsigned sink;
static uint64_t now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (uint64_t)t.tv_sec*1000000000+t.tv_nsec;}
int main(void){
 uint16_t src[16*128],dst[16*128];float rows[16*24],compact[16*17];
 for(unsigned i=0;i<16*128;i++)src[i]=(uint16_t)i;
 for(unsigned i=0;i<16*24;i++)rows[i]=(float)i;
 uint64_t p[12]={0,16,128,16,128},r=0;if(omega_cg_pklen(&r,p,12)||r)abort();
 for(unsigned sample=0;sample<3;sample++)for(unsigned om=0;om<2;om++) {
  uint64_t t=now();for(unsigned i=0;i<1000;i++){
   if(om){if(omega_cg_pkh(&r,p,12,src,p[8],dst,p[9])||r)abort();}
   else omega_matmul_frag_pack_a(src,16,128,16,128,dst);
   sink^=dst[i%2048];
  }printf("pack-a,%s,%u,%llu\n",om?"omega":"c",sample,(unsigned long long)(now()-t));
  uint64_t q[3]={16,17,24};t=now();for(unsigned i=0;i<1000;i++){
   if(om){if(omega_mrow(&r,q,3,rows,16*24,compact,16*17)||r)abort();}
   else for(unsigned j=0;j<16;j++)memcpy(compact+j*17,rows+j*24,17*sizeof(float));
   sink^=(unsigned)compact[i%272];
  }printf("compact,%s,%u,%llu\n",om?"omega":"c",sample,(unsigned long long)(now()-t));
  uint64_t l[16]={0x123456789abcdef0ull,128,32,1,16,16,8192,0xfedcba9876543210ull,8,20000,256,0,0};uint32_t d[20];
  t=now();for(unsigned i=0;i<1000;i++){
   l[6]=i;
   if(om){if(omega_arlaunch(&r,l,16,d,20)||r)abort();}
   else {memset(d,0,sizeof d);d[0]=(uint32_t)l[0];d[1]=l[0]>>32;d[2]=l[1];d[3]=l[2];d[4]=l[3];d[5]=l[4];d[6]=l[5];d[8]=l[6];d[9]=l[6]>>32;d[10]=l[7];d[11]=l[7]>>32;d[12]=l[8];d[14]=l[9];d[15]=l[9]>>32;d[16]=l[10];d[17]=l[11];d[18]=l[12];}
   sink^=d[i%20];
  }printf("descriptor,%s,%u,%llu\n",om?"omega":"c",sample,(unsigned long long)(now()-t));
  unsigned char w[128];uint16_t sc[1]={0x3f80};float x[128];for(unsigned i=0;i<128;i++){w[i]=(unsigned char)(i%0x70);x[i]=(float)(i%17);}
  t=now();for(unsigned it=0;it<1000;it++){
   uint64_t a,mag;
   if(om){if(omega_aracc(&a,w,128,sc,1,x,128)||omega_armag(&mag,w,128,sc,1,x,128))abort();}
   else {double acc=0,m=0;for(unsigned i=0;i<128;i++){uint32_t u=omega_api_refb(w[i]);float f;memcpy(&f,&u,4);double p=(double)f*x[i];acc+=p;m+=p<0?-p:p;}memcpy(&a,&acc,8);memcpy(&mag,&m,8);}
   sink^=(unsigned)a^(unsigned)mag;
  }printf("fp8-row,%s,%u,%llu\n",om?"omega":"c",sample,(unsigned long long)(now()-t));
  uint32_t ids[16],gsrc[16*32],gdst[16*32];float px[512],py[512];uint16_t ph[512];for(unsigned i=0;i<512;i++){gsrc[i]=i;px[i]=(float)(i%17);}for(unsigned i=0;i<16;i++)ids[i]=15-i;
  t=now();for(unsigned it=0;it<1000;it++){
   if(om){if(omega_arperm(&r,16,px,512,py,512)||r||omega_artcperm(&r,px,512,ph,512)||r||omega_argath32(&r,ids,16,gsrc,512,gdst,512)||r)abort();}
   else {float scale;uint32_t bits=1602224128;memcpy(&scale,&bits,4);for(unsigned i=0;i<512;i++){unsigned a=((i/4)%32)*16+(i/128)*4+i%4;py[i]=px[a]*scale;unsigned j=(i&~3u)|((i&3u)==1?2:(i&3u)==2?1:i&3u);uint32_t b;float v=px[j]*scale;memcpy(&b,&v,4);ph[i]=(uint16_t)((b+0x7fffu+((b>>16)&1u))>>16);}for(unsigned j=0;j<16;j++)memcpy(gdst+j*32,gsrc+ids[j]*32,32*sizeof(uint32_t));}
   uint32_t bits;memcpy(&bits,py+it%512,4);sink^=gdst[it%512]^ph[it%512]^bits;
  }printf("perm-gather,%s,%u,%llu\n",om?"omega":"c",sample,(unsigned long long)(now()-t));
  int8_t tw[16*32];uint32_t td[32];uint64_t tq[4]={16,32,0,2};for(unsigned i=0;i<512;i++)tw[i]=(int8_t[]){0,1,-1}[i%3];
  t=now();for(unsigned it=0;it<1000;it++){
   if(om){if(omega_artern(&r,tq,4,tw,512,td,32)||r)abort();}
   else for(unsigned row=0;row<16;row++)for(unsigned c=0;c<2;c++){uint32_t word=0;for(unsigned i=0;i<16;i++){int v=tw[row*32+c*16+i];word|=(uint32_t)(v==1?1:v==-1?2:0)<<(i*2);}td[row*2+c]=word;}
   sink^=td[it%32];
  }printf("tern-pack,%s,%u,%llu\n",om?"omega":"c",sample,(unsigned long long)(now()-t));
  t=now();for(unsigned it=0;it<1000;it++){
   uint64_t sq[8]={5,0,1,3,11},sr[3]={2,11,11},ss[7]={0},rec[16]={2,0,11,11,5,0,1};
   if(om){if(omega_arslot(&r,sq,8,sr,3,ss,7))abort();sink^=(unsigned)sr[0]^(unsigned)r;}
   else {int rc=omega_api_slot(3,11,rec);sink^=(unsigned)(rec[0]|(rec[1]<<32))^(unsigned)rc;}
  }printf("slot-pin,%s,%u,%llu\n",om?"omega":"c",sample,(unsigned long long)(now()-t));
  uint8_t ps[256*256],pd[256*256];float scales[4]={1,2,3,4},ratios[96];uint64_t pq[4]={256,256,1};for(unsigned i=0;i<sizeof ps;i++)ps[i]=(uint8_t)(i/256);
  t=now();for(unsigned it=0;it<1000;it++){
   if(om){if(omega_arpfcopy(&r,pq,4,ps,sizeof ps,pd,sizeof pd)||r||omega_arpfrat(&r,pq,4,scales,4,ratios,96)||r)abort();}
   else for(unsigned row=0;row<256;row++){unsigned p=omega_api_swsrc(row,256);memcpy(pd+row*256,ps+p*256,256);if(row%8==0){float *rt=ratios+(row/16)*6+(row/8)%2;float *sc=scales+(p/128)*2;rt[0]=1;rt[2]=sc[0]/sc[1];rt[4]=sc[1];}}
   uint32_t bits;memcpy(&bits,ratios+it%96,4);sink^=pd[it%sizeof pd]^bits;
  }printf("prefill-copy-ratios,%s,%u,%llu\n",om?"omega":"c",sample,(unsigned long long)(now()-t));
 }
 return sink==0xdeadbeefu;
}
