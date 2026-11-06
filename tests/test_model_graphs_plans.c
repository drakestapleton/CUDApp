/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cudapp_native.h"
static uint64_t state=0x6a7d9421;
static uint64_t rnd(void){state^=state<<13;state^=state>>7;return state^=state<<17;}
static uint64_t align256(uint64_t x){return(x+255)&~UINT64_C(255);}
static void layout(const uint64_t c[32],uint64_t d[64]){
 uint64_t H=c[4],I=c[5],G=c[6],N=c[1],A=c[3],L=c[2],S=c[15],B=c[14],P=c[20],R=N*G,M=c[16],pad=c[17],lb=align256(c[13]);
 uint64_t b[64]={0};unsigned count=18,split=64;memset(d,0,64*sizeof *d);
 if(c[0]==0){uint64_t x[]={H*4,H*4,c[7]*4,c[8]*4,H*4,G*2*I*4,G*I*4,G*H*4,H*4,A*S*4,S*4,S*4,16,L*H*4,H*4,L*32,c[8]*4,H*4};memcpy(b,x,sizeof x);}
 if(c[0]==1){uint64_t x[]={H*4*N,c[10]*4,H*4*N,A*S*4*N,S*4,S*4,c[19]*N,L*H*4,H*4,L*64+16,c[11]*4,c[18]*4,G*H*4,L*2*B,(H+c[12]+I)*4,H*4*N,c[9]*4,c[11]*4,c[12]*4,H*4,G*2*I*4,G*I*4,G*H*4,M*H*4,N*c[9]*4,N*c[10]*4,N*c[11]*4,N*c[12]*4,M*c[12]*4,N*H*4,N*lb,R*B,R*B,(R+pad)*4,(R+pad)*4,M*H*4,R*2*I*4,R*I*4,R*I*4,R*H*4};memcpy(b,x,sizeof x);count=40;split=15;}
 if(c[0]==2){uint64_t x[]={H*4,c[10]*4,H*4,A*S*4,S*4,S*4,16,L*2*H*4,H*4,L*64+16,c[11]*4,c[18]*4,2*I*4,H*4,c[9]*4,c[11]*4,c[12]*4,H*4,2*I*4,I*4};memcpy(b,x,sizeof x);count=20;split=13;}
 if(c[0]==3){R=P*G;uint64_t x[]={P*H*4,P*c[13],256*B,256*B,B,B,(R+pad)*4,(R+pad)*4,(P+pad)*4,(P+pad)*4,P*H*4,(P+pad)*H*4,P*c[9]*4,P*c[10]*4,P*c[11]*4,P*c[12]*4,(P+pad)*c[12]*4,P*H*4,(P+pad)*H*4,R*2*I*4,R*I*4,(R+pad)*I*4,R*H*4};memcpy(b,x,sizeof x);count=23;}
 uint64_t off=0;for(unsigned i=0;i<count;i++){if(i==split)d[60]=off;if(c[0]==3&&i==10){d[61]=off;off=0;}d[i]=off;off=align256(off+b[i]);}d[c[0]==3?63:61]=off;d[62]=lb;
}
static int plan(unsigned kind,uint64_t q[16]){
 uint64_t got[16]={0},want[16]={0},r=0,expected=0;int rc;
 if(kind==0){if((q[0]|q[1]|q[2])&15)expected=0;else if(q[3]>=q[2])expected=2;else{expected=1;uint64_t n=q[2]-q[3];if(n>q[4])n=q[4];want[0]=q[0]+q[3];want[1]=q[1]+q[3];want[2]=n;want[3]=n/16;want[4]=(n/16+q[5]-1)/q[5];}rc=omega_mg_copy(&r,q,16,got,16);}
 else if(kind==1){if(((q[1]|q[2])&15)||q[1]+q[2]>q[3])expected=0;else if(q[4]>=q[2])expected=2;else{expected=1;uint64_t n=q[2]-q[4];if(n>67108864)n=67108864;want[2]=n;want[3]=q[4];want[0]=q[0]+q[1]+q[4];want[1]=q[5];if(!q[6]){want[1]=want[0];want[0]=q[5];}}rc=omega_mg_xfer(&r,q,16,got,16);}
 else{if(!q[2]||!q[3]||!q[4])expected=0;else if(q[2]*q[1]>65536)expected=2;else if(q[1]%8||q[1]<(uint32_t)(q[7]+2))expected=3;else{expected=1;memcpy(want,q,sizeof want);want[8]=q[2]*q[3]*q[4];want[9]=20000;want[10]=200000;}rc=omega_mg_lauch(&r,q,16,got,16);}
 if(rc||r!=expected||(expected==1&&memcmp(got,want,sizeof got))){fprintf(stderr,"FAIL graph plan kind=%u status=%llu/%llu rc=%d\n",kind,(unsigned long long)r,(unsigned long long)expected,rc);return 1;}return 0;
}
int main(void){
 for(unsigned i=0;i<10032;i++){
  uint64_t c[32]={0,1+rnd()%8,1+rnd()%64,1+rnd()%2048,1024*(1+rnd()%5),256*(1+rnd()%68),1+rnd()%9,5120,4096,12288,64,9216,4096,rnd()%65536,32,2048,8,8,256,256,1+rnd()%128},a[64],b[64],r=0;
  for(unsigned m=0;m<4;m++){c[0]=m;layout(c,a);if(omega_mg_layout(&r,c,32,b,64)||r!=1||memcmp(a,b,sizeof a)){fprintf(stderr,"FAIL graph layout model=%u case=%u\n",m,i);return 1;}b[i%64]^=1;if(!memcmp(a,b,sizeof a))return 1;}
  uint64_t q[16]={rnd()&UINT64_C(0xfffffffffff0),rnd()&65535,((rnd()%134217728)&~UINT64_C(15)),rnd()%134217728,16777216,256,rnd()&1,0};
  if(i<32){q[2]=UINT64_C(1)<<i;q[3]=i?((UINT64_C(1)<<i)-1):0;}if(plan(0,q))return 1;
  q[0]&=~UINT64_C(15);q[1]&=~UINT64_C(15);q[3]=rnd()%268435456;q[4]=rnd()%134217728;q[5]=UINT64_C(0xb00000000);if(plan(1,q))return 1;
  uint64_t ac[16]={2,0,0,0,0,0,256,rnd()%1048576},av[16]={rnd(),rnd(),rnd()},ad[64],ar=0;
  if(omega_mg_aux(&ar,ac,16,av,16,ad,64)||ar!=1||ad[2]!=1||ad[3]!=(ac[7]+255)/256||ad[4]!=ad[3]*256||ad[5]!=6)return 1;
  for(unsigned j=0;j<3;j++)if(ad[32+2*j]!=(uint32_t)av[j]||ad[33+2*j]!=(uint32_t)(av[j]>>32))return 1;
  q[1]=rnd()%513;q[2]=rnd()%1025;q[3]=rnd()%1025;q[4]=rnd()%129;q[7]=rnd()%513;if(plan(2,q))return 1;
 }
 puts("MODEL-GRAPHS plans PASS layouts=40128 prefill=30096 record-flip=CAUGHT");return 0;
}
