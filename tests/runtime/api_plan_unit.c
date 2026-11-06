/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define OMEGA_API_PLAN_ORACLE 1
#include "omega_api_plan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
static uint64_t cases[6];
static int selected=-1;
static void check(unsigned f,unsigned m,uint64_t a[16]){
    if(selected>=0&&(unsigned)selected!=f)return;
    uint64_t b[16],c[16];memcpy(b,a,sizeof b);memcpy(c,a,sizeof c);
    for(unsigned i=8;i<16;i++)b[i]=c[i]=0x9e3779b9u+i;
    int want=-(int)ap_c(f,m,b),got=ap_om(f,m,c);
    if(want!=got||memcmp(b,c,sizeof b)){
        fprintf(stderr,"API PLAN differential FAIL family=%u mode=%u want=%d got=%d\n",f,m,want,got);
        for(unsigned i=0;i<16;i++)fprintf(stderr,"%u: in=%"PRIu64" C=%"PRIu64" OM=%"PRIu64"\n",i,a[i],b[i],c[i]);
        exit(1);
    }
    cases[f]++;
}
#define C(F,M,...) do{uint64_t a[16]={__VA_ARGS__};check(F,M,a);}while(0)
int main(int argc,char **argv){
    if(argc==2)selected=atoi(argv[1]);
    const uint32_t edge[]={0,1,2,3,7,8,15,16,31,32,63,64,127,128,255,256,511,512,1023,1024,2047,2048,8191,8192,16383,16384,65535,65536,131071,131072,262143,262144,UINT32_MAX-1,UINT32_MAX};
    const size_t en=sizeof edge/sizeof edge[0];
    for(uint32_t x=0;x<=262145;x++){C(AP_MM,0,1,1,x,0,0);if(x<=16385)C(AP_MM,0,1,x,1,0,0);if(x<=8193)C(AP_MM,0,x,1,1,0,0);}
    for(size_t i=0;i<en;i++)for(size_t j=0;j<en;j++)for(size_t k=0;k<en;k++)C(AP_MM,0,edge[i],edge[j],edge[k],edge[j],edge[i]);
    const uint32_t mts[]={1,2,3,4,5,6,7,8,255,256,511,512};
    const uint32_t bs[]={0,1,2,3,4,7,8,15,16,63,64,95,96,511,512,1023,1024};
    for(size_t m=0;m<sizeof mts/sizeof mts[0];m++)for(uint32_t nt=1;nt<=32768;nt++)for(size_t b=0;b<sizeof bs/sizeof bs[0];b++)C(AP_MM,1,mts[m]*16,nt*8,bs[b]);
    for(uint32_t mt=1;mt<=512;mt++)for(uint32_t b=0;b<=1024;b++)for(uint32_t nt=1;nt<=8;nt++)C(AP_MM,1,mt*16,nt*8,b);
    for(uint32_t k=1;k<=16384;k++)for(unsigned b=0;b<2;b++){C(AP_MM,2,k,32768,0,b);C(AP_MM,2,k,32768,32767,b);}
    for(size_t i=0;i<en;i++)for(unsigned q=8;q<=16;q+=8)C(AP_MM,3,edge[i],q);
    for(size_t i=0;i<en;i++)for(size_t j=0;j<en;j++){C(AP_MM,4,edge[i],edge[j],edge[i],edge[j]);C(AP_MM,4,edge[i],edge[j],edge[i]^1,edge[j]);}
    for(unsigned ks=1;ks<=1024;ks++)for(unsigned r=1;r<=4;r*=2)for(unsigned c=1;c<=4;c*=2)for(unsigned u=0;u<=128;u++)for(unsigned m=0;m<2;m++)C(AP_MM,5,ks*16,r,c,u,m);
    for(unsigned n=1;n<=262144;n++){unsigned last=(n+7)/8;C(AP_MM,6,last>64?last-64:0,last,n);}
    for(unsigned mt=1;mt<=512;mt++)for(unsigned per=1;per<=512;per++)for(unsigned r=1;r<=4;r*=2)C(AP_MM,7,mt*16,per*16,r);
    for(unsigned b=0;b<=1025;b++)C(AP_MM,8,b);
    for(size_t i=0;i<en;i++)C(AP_MM,8,edge[i]);
    for(uint32_t q=0;q<=1025;q++)for(uint32_t k=0;k<=1025;k++){
        const unsigned hs[]={0,1,63,64,65,127,128,129,255,256,257};
        for(size_t h=0;h<sizeof hs/sizeof hs[0];h++)for(unsigned g=0;g<2;g++)C(AP_AT,0,q,k,hs[h],g);
    }
    for(uint32_t h=1;h<=256;h++)for(uint32_t q=1;q<=1024;q++)C(AP_AT,0,q,1,h,0);
    for(uint32_t v=0;v<=131073;v++){C(AP_AT,1,v);C(AP_AT,3,v,64);C(AP_AT,3,v,128);}
    for(unsigned p=0;p<32;p++){uint32_t v=UINT32_C(1)<<p;C(AP_AT,1,v);C(AP_AT,1,v-1);C(AP_AT,1,v+1);}
    for(uint32_t q=1;q<=1024;q++)for(size_t i=0;i<en;i++){
        for(unsigned l=0;l<=4;l++)C(AP_AT,4,l,q);
        for(unsigned r=0;r<2;r++)C(AP_AT,2,edge[i],q,q*256,edge[i],r,64);
        C(AP_AT,5,edge[i],1,1,64,256,q,0);
    }
    for(unsigned q=0;q<=1025;q++)for(unsigned h=0;h<=257;h++)for(unsigned r=0;r<2;r++)C(AP_AT,6,r,q,h);
    const int64_t win[]={-2,-1,0,1,2,INT32_MAX};
    const uint32_t scales[]={0,0x80000000u,0x3f800000u,0x7f7fffffu,0x7f800000u,0xff800000u,0x7fc00000u,0xbf800000u};
    for(unsigned r=0;r<2;r++)for(unsigned l=0;l<6;l++)for(unsigned w=0;w<6;w++)for(unsigned s=0;s<8;s++)C(AP_AT,7,r,win[l],win[w],scales[s]);
    for(unsigned p=0;p<=128;p++)for(unsigned k=0;k<=128;k++)for(unsigned l=0;l<6;l++)for(unsigned w=0;w<6;w++)for(unsigned c=0;c<2;c++)C(AP_AT,10,p,k,c,win[l],win[w]);
    for(size_t i=0;i<en;i++)for(size_t j=0;j<en;j++){
        for(unsigned l=0;l<2;l++){C(AP_AT,8,edge[i],edge[j],l);C(AP_AT,9,edge[i],l);C(AP_AT,17,edge[i],edge[j],16,128,l);}
        C(AP_AT,12,edge[i],edge[j],64);C(AP_AT,14,edge[i],64,edge[j],128);
        C(AP_AT,15,edge[i],edge[j],256);C(AP_AT,18,edge[i],edge[j]);C(AP_AT,19,edge[i],edge[j]);
        C(AP_AT,16,edge[i],edge[j],65536,1,16384,UINT32_MAX);
        for(unsigned elem=2;elem<=4;elem+=2)C(AP_AT,11,edge[i],edge[j],128,elem,0,0);
        for(unsigned bad=0;bad<7;bad++){uint64_t a[16]={128,4,256,256,1024,262144,524288};if(bad<6)a[bad]++;check(AP_AT,13,a);}
    }
    for(size_t i=0;i<en;i++)for(size_t j=0;j<en;j++){C(AP_AT,20,edge[i],edge[j]);for(size_t k=0;k<en;k++)C(AP_AT,24,edge[i],edge[j],edge[k]);}
    for(unsigned h=1;h<=256;h++){
        C(AP_AT,23,h);for(unsigned l=0;l<=32;l++)C(AP_AT,21,h,l);
        for(unsigned sign=0;sign<2;sign++)for(unsigned ex=0;ex<256;ex++)for(unsigned man=0;man<3;man++){
            uint32_t b=sign*0x80000000u+ex*0x800000u+(man==0?0:man==1?1:0x7fffff);
            C(AP_AT,7,1,UINT64_MAX,UINT64_MAX,b);C(AP_AT,22,h,b,0);C(AP_AT,22,h,b,1);
        }
        for(uint32_t b=0x7f61b1e4u;b<=0x7f61b1e8u;b++)C(AP_AT,7,1,UINT64_MAX,UINT64_MAX,b);
    }
    for(unsigned q=1;q<=1024;q++)for(unsigned k=1;k<=q;k++)if(q%k==0)for(unsigned h=0;h<q;h++)for(unsigned m=0;m<2;m++)C(AP_AT,25,h,q,k,256,m);
    for(size_t i=0;i<en;i++){C(AP_AT,3,edge[i],64);C(AP_AT,3,edge[i],128);}
    for(unsigned g=0;g<2;g++)for(unsigned bs=0;bs<=33;bs++)for(unsigned q=0;q<=8;q++)for(unsigned h=0;h<=257;h++)C(AP_AT,26,g,bs,q,h);
    for(int s=-2;s<=6;s++)C(AP_AT,27,(uint64_t)(int64_t)s);
    C(AP_AT,27,(uint64_t)(int64_t)INT32_MIN);C(AP_AT,27,INT32_MAX);
    for(size_t i=0;i<en;i++)for(unsigned g=0;g<2;g++)C(AP_AT,26,g,edge[i],edge[i],edge[i]);
    for(uint32_t n=0;n<=262145;n++){C(AP_EW,2,n);C(AP_EW,3,n);}
    for(uint32_t d=0;d<=16385;d++)for(size_t n=0;n<en;n++){C(AP_EW,0,edge[n],d);C(AP_EW,1,edge[n],d);}
    for(size_t i=0;i<en;i++){C(AP_EW,2,edge[i]);for(unsigned op=0;op<7;op++)C(AP_EW,4,edge[i],0,op,4);}
    for(int m=-2;m<=5;m++){C(AP_EW,5,(uint64_t)(int64_t)m);C(AP_EW,6,(uint64_t)(int64_t)m);}
    C(AP_EW,5,(uint64_t)(int64_t)INT32_MIN);C(AP_EW,5,INT32_MAX);
    C(AP_EW,6,(uint64_t)(int64_t)INT32_MIN);C(AP_EW,6,INT32_MAX);
    for(uint32_t k=0;k<=16385;k++)for(unsigned over=0;over<16;over++)C(AP_BF,0,k,over&1?3:0,over&2?32:0,over&4?4:0,over&8?64:0);
    for(uint32_t n=0;n<=65536;n++)for(unsigned rows=1;rows<=4;rows++)for(unsigned w=16;w<=32;w+=16)C(AP_BF,1,n,rows,w,96);
    for(unsigned r=0;r<2;r++)for(unsigned xr=0;xr<=65536;xr++)for(unsigned xb=0;xb<=5;xb++)for(unsigned mask=0;mask<2;mask++)for(unsigned tau=0;tau<2;tau++)for(unsigned ys=127;ys<=129;ys++)C(AP_BF,2,r,xr,xb,ys,mask,tau,128);
    for(unsigned x=0;x<=5;x++)for(size_t j=0;j<en;j++)C(AP_BF,3,x,edge[j]);
    for(unsigned w=0;w<=1024;w++)for(unsigned d=0;d<=512;d++)C(AP_BF,4,w,d);
    for(size_t i=0;i<en;i++){C(AP_BF,1,1,0,16,96);C(AP_BF,1,1,1,0,96);C(AP_BF,4,edge[i],256);}
    for(size_t i=0;i<en;i++){C(AP_BF,5,edge[i]);for(size_t j=0;j<en;j++)for(unsigned b=0;b<=5;b++)for(unsigned m=0;m<2;m++)for(unsigned r=1;r<=4;r++)C(AP_BF,6,edge[i],b,edge[j],m,128,r);}
    for(size_t i=0;i<en;i++)for(unsigned field=1;field<5;field++){uint64_t a[16]={8192};a[field]=edge[i];check(AP_BF,0,a);}
    for(uint32_t p=0;p<=131073;p++)for(unsigned l=0;l<=65;l++)C(AP_DA,0,l,p,1);
    for(uint32_t p=0;p<131072;p++)for(unsigned per=0;per<=131072;per+=256)C(AP_DA,2,p,per,512,0);
    for(uint64_t per=131328;per<=0xffffff00u;per+=256)C(AP_DA,2,131071,per,512,0);
    for(size_t i=0;i<en;i++)for(size_t j=0;j<en;j++){C(AP_DA,1,edge[i],edge[j],131072);C(AP_DA,2,edge[i],edge[j],512,0);}
    const uint64_t theta[]={0,1,0x3ff0000000000000ull,0x7fefffffffffffffull,0x7ff0000000000000ull,0x7ff8000000000000ull,0x8000000000000000ull,0xbff0000000000000ull};
    for(unsigned t=0;t<8;t++)C(AP_DA,0,1,256,theta[t]);
    for(unsigned l=0;l<=65;l++)for(unsigned h=0;h<=4;h++)for(size_t i=0;i<en;i++)for(size_t j=0;j<en;j++)C(AP_DA,3,l,64,h,edge[i],edge[j],131072);
    for(unsigned g=0;g<=1024;g++)C(AP_DA,4,g);
    C(AP_DA,2,131071,0xffffff00u,512,0);
    for(unsigned l=0;l<=65;l++)for(unsigned e=0;e<=257;e++)for(unsigned s=0;s<=2;s++)C(AP_RT,0,l,e,s);
    for(unsigned r=0;r<=65;r++)for(unsigned x=8176;x<=8224;x++)for(unsigned lb=16640;lb<=17152;lb++){C(AP_RT,2,r,x,lb,16768);C(AP_RT,2,r,x,lb,16800);}
    for(size_t i=0;i<en;i++)for(size_t j=0;j<en;j++){C(AP_RT,1,edge[i],edge[j],128);C(AP_RT,1,edge[i],edge[j],256);}
    for(unsigned l=0;l<=65;l++)for(unsigned rows=0;rows<=65;rows++)for(unsigned rec=0;rec<2;rec++)for(unsigned lb=0;lb<2;lb++)for(size_t i=0;i<en;i++)C(AP_RT,3,l,64,rows,rec,lb,edge[i],edge[en-1-i]);
    for(unsigned w=0;w<=1024;w++)for(unsigned g=0;g<=512;g++)C(AP_RT,4,w,g);
    /* Negative controls: rejected domain and binding-size refusal leave output intact. */
    for(unsigned f=0;f<6;f++)C(f,99,0);
    uint64_t result=77,a[16]={0};
    if(omega_api_mm_plan(&result,0,a,15)!=OMEGA_BAD_ARG||result!=77){fputs("API PLAN negative control FAIL\n",stderr);return 1;}
    for(unsigned f=0;f<6;f++)printf("API PLAN family %u differential PASS (%"PRIu64" cases)\n",f,cases[f]);
    puts("API PLAN negative control PASS");puts("API PLAN differential PASS");return 0;
}
