/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_API_PLAN_H
#define OMEGA_API_PLAN_H
#include <stdint.h>
#include <string.h>
#include "cudapp_native.h"
#ifndef CUDAPP_MIGRATION_PENDING_C_API_PLAN
#define CUDAPP_MIGRATION_PENDING_C_API_PLAN 0
#endif
/* Private, fixed-size function-mode ABI. Inputs 0..7; outputs 8..15.
 * All calls are serialized by their existing session shell, not by policy.
 * Refusal numbers match the API's negative return codes. Pointer admission is
 * deliberately performed before unpacking a pointed-to object in the shell. */
enum { AP_MM, AP_AT, AP_EW, AP_BF, AP_DA, AP_RT };
#if CUDAPP_MIGRATION_PENDING_C_API_PLAN || defined(OMEGA_API_PLAN_ORACLE)
static inline uint64_t ap_log(uint64_t v) {
    if (!v || (v & (v - 1))) return 32;
    uint64_t l = 0; while ((v >> l) != 1) l++; return l;
}
static inline uint64_t ap_c(unsigned family, uint64_t mode, uint64_t a[16]) {
    switch (family) {
    case AP_MM:
        if (!mode) {
            uint64_t m=a[0],k=a[1],n=a[2];
            if (!m||!k||!n) return 1;
            if (m>8192||k>16384||n>262144||a[3]>262144||a[4]>128) return 2;
            uint64_t mp=(m+15)/16*16,kp=(k+15)/16*16,np=(n+7)/8*8;
            uint64_t cb=mp*np*4,one=(a[3]+7)/8*8*64;
            if(one>cb)cb=one;
            a[8]=mp;a[9]=kp;a[10]=np;a[11]=mp*kp*2;a[12]=cb;a[13]=kp*np*2;a[14]=k*n;
            return 0;
        }
        if(mode==3){a[8]=(uint32_t)(a[0]+a[1]-1)/a[1]*a[1];return 0;}
        if(mode==1){
            uint64_t mp=a[0],np=a[1],b=a[2]?a[2]:1024,mt=mp/16,nt=np/8;
            uint64_t r=mt%4==0?4:mt%2==0?2:1,c=1;
            for(;r>1;r/=2){c=nt%4==0?4:nt%2==0?2:1;if((r-1)*64*np+(c-1)*32+4<8388608)break;}
            if(r==1)c=1;
            uint64_t rb=mt/r,nb=nt/c,rows=rb<=b?mp:b*16*r,gx=b/(rows/16/r);
            if(!gx)gx=1;
            if(gx>nb)gx=nb;
            a[8]=rows;a[9]=gx;a[10]=r;a[11]=c;a[12]=rows/16/r;return 0;
        }
        if(mode==2){uint64_t c=64;if(a[3]&&262144/(a[0]*8)<c)c=262144/(a[0]*8);a[8]=c;a[9]=a[1]-a[2]<c?a[1]:a[2]+c;return 0;}
        if(mode==4)return a[0]!=a[2]||a[1]!=a[3]?1:0;
        if(mode==5){uint64_t ks=a[0]/16,b=a[1]>1||a[2]>1,u=b?(!(ks%2)&&2*(a[1]+a[2])<=8?2:1):a[3]?a[3]:!(ks%4)?4:!(ks%2)?2:1;
            if(a[4]&&(b||!a[3]))while(u>1&&ks/u<2)u/=2;
            a[8]=b;a[9]=u;return 0;}
        if(mode==6){uint64_t c=a[0]*8,w=a[1]*8;if(w>a[2])w=a[2];a[8]=c;a[9]=w-c;return 0;}
        if(mode==7){uint64_t r=a[0]<a[1]?a[0]:a[1];a[8]=r;a[9]=r/16/a[2];return 0;}
        if(mode==8){a[8]=a[0]?a[0]:1024;return 0;}
        break;
    case AP_AT:
        if(!mode){uint64_t q=a[0],k=a[1],h=a[2];if(!q||!k||!h)return 1;if(h>256||q>1024)return 2;if(q%k)return 1;
            uint64_t l=ap_log(q/k);a[8]=a[9]=0;if(!a[3]&&(h==64||h==128)&&q<=64&&l<32){a[8]=l;a[9]=1;}return 0;}
        if(mode==1){uint64_t l=ap_log(a[0]);if(l==32)return 1;a[8]=l;return 0;}
        if(mode==2){uint64_t p=1024/a[1];if(!p)p=1;if(p>a[0])p=a[0];if(a[4]&&a[3]>=a[2]){uint64_t f=a[3]/a[2];if(f<p)p=f;}
            a[8]=p;a[9]=p*a[1]*a[5]*4;a[10]=p*16+a[1]*4+4;return 0;}
        if(mode==3){uint32_t x=((uint32_t)a[0]+(uint32_t)a[1]-1)/(uint32_t)a[1];uint64_t l=0;while(l<4&&(2u<<l)<=x)l++;a[8]=l;return 0;}
        if(mode==4){uint64_t l=a[0],p=64/a[1];if(!p)p=1;while(l&&((a[1]*p)<<l)>1024)l--;a[8]=l;a[9]=p;a[10]=1ull<<l;return 0;}
        if(mode==5){if(!a[0]||!a[1]||(a[4]&&ap_log(a[4])==32))return 1;
            uint64_t p=a[0]*a[2]*a[3]*8;if(a[4]){uint64_t v=(a[0]+a[4]-1)/a[4]*a[4]*a[2]*a[3]*4;if(v>p)p=v;}
            a[8]=p*a[1];a[9]=a[1]*a[5]*a[3]*4;
            uint64_t launch=64/a[5];if(!launch)launch=1;if(launch>a[1])launch=a[1];
            uint64_t part=launch*a[5]*16*(a[3]+2)*4;a[10]=part>a[9]?part:a[9];
            part=(a[1]*a[0]+a[1])*4;if(!a[6])part+=a[1]*16+a[5]*4;a[11]=part<8?8:part;return 0;}
        if(mode==6)return !a[0]||!a[1]||a[1]>1024||!a[2]||a[2]>256?1:0;
        if(mode==7){union{uint32_t u;float f;}s={.u=a[3]};return !a[0]||(int64_t)a[1]<-1||(int64_t)a[2]<-1||!(s.f>=0.0f)||s.f>3.0e38f?1:0;}
        if(mode==8){if(!a[1])return 1;uint64_t t=a[0]+a[1];if(t>0x7fffffffu)return 2;a[8]=t;a[9]=!a[2]||t>4096;return 0;}
        if(mode==9){a[8]=a[0]&&(!a[1]||a[0]>4096);return 0;}
        if(mode==10){uint64_t p=a[0],hi=a[1],lo=0;if(a[2])hi=p+1;else if((int64_t)a[4]>=0)hi=p+a[4]+1;if(hi>a[1])hi=a[1];if((int64_t)a[3]>=0&&p>a[3])lo=p-a[3];if(lo>hi)lo=hi;a[8]=lo;a[9]=hi;return 0;}
        if(mode==11){uint64_t v=a[0]*a[1]*a[2]*a[3];if(2*v>UINT32_MAX)return 2;a[8]=v;a[9]=a[4]?a[4]:a[1]*a[2];a[10]=a[5]?a[5]:a[2];return 0;}
        if(mode==12){uint64_t n=a[0]<a[1]?a[0]:a[1];a[8]=n;a[9]=n*4;a[10]=n*4+a[2];return 0;}
        if(mode==13)return a[3]!=a[0]*2||a[4]!=a[1]*a[0]*2||a[5]!=a[2]*a[4]||a[6]!=2*a[5]?1:0;
        if(mode==14)return a[0]>=a[1]||a[2]<a[1]*a[3]?1:0;
        if(mode==15){uint64_t cap=a[1]*a[2],ctx=a[0]<cap?a[0]:cap;a[8]=ctx;a[9]=(uint32_t)(ctx+a[2]-1)/a[2];return 0;}
        if(mode==16)return a[0]>=a[1]||a[0]*a[2]+(a[3]+1)*a[4]>a[5]?1:0;
        if(mode==17){a[8]=a[4]?a[0]*a[1]*a[2]*(a[3]+2)*4:4;a[9]=a[0]+a[1];return 0;}
        if(mode==18){uint64_t v=(a[0]?a[0]:1)*a[1];if(v>UINT32_MAX)return 2;a[8]=v;return 0;}
        if(mode==19)return !a[0]||!a[1]?1:0;
        if(mode==20){a[8]=a[0]>a[1]?a[0]:a[1];return 0;}
        if(mode==21){uint64_t v=a[0]==128?1ull<<(a[1]<2?a[1]:2):1;a[8]=v;a[9]=(3*v+1)*a[0]*4;return 0;}
        if(mode==22){float u;uint32_t bits=a[1];memcpy(&u,&bits,4);double v=1.4426950408889634/8.0;
            if(!a[2]){double x=(double)a[0],r=x;if(!(u>0)){for(int i=0;i<64;i++){double n=0.5*(r+x/r);if(!(n<r))break;r=n;}}v=1.4426950408889634*(u>0?(double)u:1.0/r);}
            float f=(float)v;memcpy(&bits,&f,4);a[8]=bits;return 0;}
        if(mode==23){float f=(float)(1.4426950408889634/(a[0]==128?11.313708498984761:8.0));uint32_t b;memcpy(&b,&f,4);a[8]=b;return 0;}
        if(mode==24){a[8]=a[0]<=UINT32_MAX&&a[1]*a[0]+a[2]<=UINT32_MAX;return 0;}
        if(mode==25){a[8]=(uint32_t)((a[4]?a[0]%a[2]:a[0]/(a[1]/a[2]))*a[3]);return 0;}
        if(mode==26){if(!a[0]){if(a[1]>16||a[2]>6)return 1;if(a[3]!=64&&a[3]!=128)return 2;}else{if(a[1]>31||!a[3])return 1;if(a[3]>256)return 2;}return 0;}
        if(mode==27){a[8]=a[0]<=4?a[0]:UINT32_MAX;return 0;}
        break;
    case AP_EW:
        if(!mode){if(!a[0]||!a[1]||a[1]%128)return 1;if(a[1]>16384)return 2;}
        if(mode==1){if(!a[0]||a[1]<2||(a[1]&1))return 1;if(a[1]>2048)return 2;}
        if(mode<=1){uint64_t nr=a[0]<1024?a[0]:1024;a[8]=nr;a[9]=nr*a[1]*4;a[10]=a[1]/(mode?2:128);
            if(!mode){union{float f;uint32_t u;}v={.f=1.0f/(float)a[1]};a[11]=v.u;}return 0;}
        if(mode==2){if(!a[0])return 1;uint64_t n=a[0]<131072?a[0]:131072;a[8]=n;a[9]=(n+127)/128;a[10]=n*4;return 0;}
        if(mode==3)return !a[0]||a[0]%128?1:0;
        if(mode==4)return !a[0]||a[2]>a[3]?1:0;
        if(mode==5)return a[0]>3?1:0;
        if(mode==6){a[8]=!a[0]||a[0]>INT64_MAX?1:a[0]>3?3:a[0];return 0;}
        break;
    case AP_BF:
        if(!mode){uint64_t k=a[0],ks=k/256,w=16,d=ks;if(d)while(d>16||ks%d)d--;
            if(k%(32*w)||k>512*w)w=32;
            a[8]=a[1]?a[1]:1;a[9]=a[2]?a[2]:w;a[10]=a[3]?a[3]:d;a[11]=a[4]?a[4]:96;return 0;}
        if(mode==1){if(!a[1]||!a[2]||a[0]%a[1])return 3;uint64_t g=(uint32_t)(a[0]/a[1]+a[2]-1)/a[2];if(g>a[3])g=a[3];if(!g)g=1;a[8]=g;a[9]=a[0]/a[1];return 0;}
        if(mode==2){if(a[1]&&(!a[0]||a[1]>65535))return 1;if(a[4]&&(!a[0]||a[1]||a[2]>1||!a[5]))return 1;
            if(a[2]>1&&(!a[0]||a[1]||a[2]>4||a[3]<a[6]))return 1;
            return 0;}
        if(mode==3){if(a[0]>1&&(a[0]>4||!a[1]))return 1;a[8]=a[0]<=1;return 0;}
        if(mode==4)return 32*a[0]*a[1]>65536?2:0;
        if(mode==5){if(!a[0])return 1;a[8]=a[0]==1;return 0;}
        if(mode==6){uint64_t r=a[5],s=0,xb=0,gy=1,nr=1;if(a[0]){s=a[4];gy=nr=a[0];}if(a[3])r=1;if(a[1]>1){xb=nr=a[1];s=a[2];}a[8]=r;a[9]=s;a[10]=xb;a[11]=gy;a[12]=a[4]*nr;return 0;}
        break;
    case AP_DA:
        if(!mode){uint64_t l=a[0],p=a[1];union{uint64_t u;double f;}t={.u=a[2]};if(!l||l>64||!p||p>131072||p%256||!(t.f>0))return 1;
            a[8]=p/256;a[9]=4*p*256+(4*8*256+64);a[10]=p*512;a[11]=l*2*128*4;a[12]=4*(p/256)*1040*4;return 0;}
        if(mode==1)return a[0]>a[2]||a[1]>a[2]-a[0]?1:0;
        if(mode==2){uint64_t p=a[1]?a[1]:256;if(p%256)return 1;uint64_t s=(a[0]+p)/p;if(s>a[2])return 2;
            a[8]=p;a[9]=s;a[10]=a[3]?a[3]:4;a[11]=2*4*(a[0]+1)*256;return 0;}
        if(mode==3)return a[0]>=a[1]||a[2]>=4||!a[4]||a[3]>a[5]||a[4]>a[5]-a[3]?1:0;
        if(mode==4)return a[0]*256>65536?2:0;
        break;
    case AP_RT:
        if(!mode){a[7]=0;if(!a[0]||a[0]>64)return 1;a[7]=1;uint64_t e=a[1]?a[1]:128;if((e!=128&&e!=256)||a[2]>1)return 1;
            a[8]=e;a[9]=e+a[2];a[10]=8+a[2];a[11]=a[2]?16656:16640;a[12]=a[2]?16800:16768;
            a[13]=a[0]*a[9]*4096;a[14]=a[0]*8192;a[15]=a[0]*a[9]*32;return 0;}
        if(mode==1){a[8]=a[0]?a[0]:32;a[9]=a[1]?a[1]:4;a[10]=a[2]==128?0:a[2];return 0;}
        if(mode==2){if(!a[0]||a[0]>64)return 1;if(a[0]>1&&(a[1]<8192||(a[1]&15)||a[2]<a[3]||(a[2]&255)))return 1;a[8]=a[0]==1;return 0;}
        if(mode==3){if(a[0]>=a[1])return 1;a[8]=a[9]=0;a[10]=1;if(a[2]>1){if(!a[3]||!a[4])return 1;a[8]=a[5];a[9]=a[6];a[10]=a[2];}return 0;}
        if(mode==4)return 32*a[0]*a[1]>65536?2:0;
        break;
    }
    return 1;
}
#endif
static inline int ap_om(unsigned f, uint64_t mode, uint64_t a[16]) {
    uint64_t rc=1;uint32_t st=OMEGA_BAD_ARG;
    switch(f){
    case AP_MM:st=omega_api_mm_plan(&rc,mode,a,16);break;
    case AP_AT:st=omega_api_at_plan(&rc,mode,a,16);break;
    case AP_EW:st=omega_api_ew_plan(&rc,mode,a,16);break;
    case AP_BF:st=omega_api_bf_plan(&rc,mode,a,16);break;
    case AP_DA:st=omega_api_da_plan(&rc,mode,a,16);break;
    case AP_RT:st=omega_api_rt_plan(&rc,mode,a,16);break;
    }
    return st==OMEGA_OK&&rc<=3?-(int)rc:-1;
}
static inline int ap_plan(unsigned f, uint64_t mode, uint64_t a[16]) {
#if CUDAPP_MIGRATION_PENDING_C_API_PLAN
    return -(int)ap_c(f,mode,a);
#else
    return ap_om(f,mode,a);
#endif
}
#endif
