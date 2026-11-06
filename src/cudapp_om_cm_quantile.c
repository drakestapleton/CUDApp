#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
/* Emitted by Omega Bridge. GNU C11, POSIX. Scalars use canonical 64-bit bits. */
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <setjmp.h>
static _Thread_local jmp_buf o_jmp;
static _Thread_local int o_busy;
static __attribute__((noreturn)) void o_trap(int c){if(o_busy)longjmp(o_jmp,c);_exit(100+c);}
static _Thread_local unsigned o_depth;
static void o_enter(void){if(++o_depth>128)o_trap(6);}
static uint64_t o_leave(uint64_t v){--o_depth;return v;}
static unsigned o_width(unsigned t){static const unsigned w[]={0,1,8,16,32,64,8,16,32,64};return w[t];}
static __int128 o_value(unsigned t,uint64_t v){return t>=6 && v>>63 ? (__int128)v-(((__int128)1)<<64):(__int128)v;}
static uint64_t o_norm(unsigned t,__int128 v,int code){unsigned w=o_width(t);if(t>=6){__int128 m=((__int128)1)<<(w-1);if(v< -m||v>=m)o_trap(code);}else {if(v<0||(__uint128_t)v>=(((__uint128_t)1)<<w))o_trap(code);}return (uint64_t)v;}
static uint64_t o_cast(unsigned t,unsigned s,uint64_t a){return o_norm(t,o_value(s,a),5);}
static uint64_t o_op(unsigned op,unsigned t,uint64_t a,uint64_t b){
 __int128 x=o_value(t,a),y=o_value(t,b),z=0;unsigned w=o_width(t);uint64_t mask=w==64?UINT64_MAX:(((uint64_t)1<<w)-1);
 if(op>=6&&op<=11){switch(op){case 6:return x==y;case 7:return x!=y;case 8:return x<y;case 9:return x<=y;case 10:return x>y;default:return x>=y;}}
 if(op==12)return a&b;if(op==13)return a|b;if(op==14)return a^b;
 if(op==15||op==16){if(b>=w)o_trap(7);if(op==15)return t>=6?o_norm(t,x*(((__int128)1)<<b),1):(a<<b)&mask;if(t<6)return a>>b;if(!b)return a;return (a>>b)|((a>>63)?(UINT64_MAX<<(64-b)):0);}
 if(op==17){if(t<6)return (0-a)&mask;return o_norm(t,-x,1);}if(op==18)return t==1?!a:(t>=6?~a:(~a)&mask);
 if(op==4||op==5){if(!b)o_trap(2);z=op==4?x/y:x%y;return t>=6?o_norm(t,z,1):(uint64_t)z;}
 if(t<6){if(op==1)return (a+b)&mask;if(op==2)return (a-b)&mask;return (a*b)&mask;}
 if(op==1)z=x+y;if(op==2)z=x-y;if(op==3)z=x*y;return o_norm(t,z,1);
}
static uint64_t o_get(const void *p,uint64_t n,uint64_t i,unsigned t){if(i>=n)o_trap(3);switch(t){case 2:return ((const uint8_t*)p)[i];case 3:return ((const uint16_t*)p)[i];case 4:return ((const uint32_t*)p)[i];case 5:return ((const uint64_t*)p)[i];case 6:return (uint64_t)((const int8_t*)p)[i];case 7:return (uint64_t)((const int16_t*)p)[i];case 8:return (uint64_t)((const int32_t*)p)[i];default:return (uint64_t)((const int64_t*)p)[i];}}
static void o_put(void *p,uint64_t n,uint64_t i,uint64_t v,unsigned t){if(i>=n)o_trap(3);o_norm(t,o_value(t,v),5);switch(t){case 2:((uint8_t*)p)[i]=(uint8_t)v;break;case 3:((uint16_t*)p)[i]=(uint16_t)v;break;case 4:((uint32_t*)p)[i]=(uint32_t)v;break;case 5:((uint64_t*)p)[i]=v;break;case 6:((int8_t*)p)[i]=(int8_t)o_value(t,v);break;case 7:((int16_t*)p)[i]=(int16_t)o_value(t,v);break;case 8:((int32_t*)p)[i]=(int32_t)o_value(t,v);break;default:((int64_t*)p)[i]=(int64_t)o_value(t,v);}}
static void o_fill(void *p,uint64_t n,uint64_t v,unsigned t){for(uint64_t i=0;i<n;i++)o_put(p,n,i,v,t);}
static uint64_t o_io(unsigned op,uint64_t fd,const void *p,uint64_t n,uint64_t off,uint64_t len){if(off>n||len>n-off)o_trap(3);int f=(int)o_value(8,fd);ssize_t r=op==1?read(f,(uint8_t*)p+off,(size_t)len):write(f,(const uint8_t*)p+off,(size_t)len);return r<0?(uint64_t)(-(int64_t)errno):(uint64_t)r;}
static int o_valid(unsigned t,uint64_t v){unsigned w=o_width(t);__int128 x=o_value(t,v);__int128 m=((__int128)1)<<(t>=6?w-1:w);return t>=6?x>=-m&&x<m:x>=0&&x<m;}
static int o_buf(const void *p,uint64_t n,uint64_t want,unsigned t){uintptr_t a=(uintptr_t)p;uint64_t z=t==2?1:8;return p&&n==want&&a%z==0&&n<=UINTPTR_MAX/z&&a<=UINTPTR_MAX-n*z;}
static int o_overlap(const void *a,uint64_t an,const void *b,uint64_t bn){uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x<y+bn&&y<x+an;}
#include <float.h>
#include <fenv.h>
#if FLT_RADIX != 2 || FLT_MANT_DIG != 24 || FLT_MAX_EXP != 128 || DBL_MANT_DIG != 53 || DBL_MAX_EXP != 1024 || FLT_EVAL_METHOD != 0
#error unsupported binary32 or binary64 environment
#endif
#if defined(__FAST_MATH__)
#error fast math is not a supported Omega environment
#endif
static int o_fnan(unsigned t,uint64_t a){return t==11?((a>>23&255)==255&&(a&UINT64_C(0x7fffff))!=0):((a>>52&2047)==2047&&(a&UINT64_C(0xfffffffffffff))!=0);}
static uint64_t o_fpn(unsigned t,uint64_t a,uint64_t b,uint64_t r){uint64_t q=t==11?UINT64_C(0x400000):UINT64_C(0x8000000000000);if(!o_fnan(t,r))return r;if(o_fnan(t,a)&&!(a&q))return a|q;if(o_fnan(t,b)&&!(b&q))return b|q;if(o_fnan(t,a))return a;if(o_fnan(t,b))return b;return t==11?UINT64_C(0x7fc00000):UINT64_C(0x7ff8000000000000);}
static uint64_t o_fp(unsigned op,unsigned t,uint64_t a,uint64_t b){if(t==11){uint32_t ua=(uint32_t)a,ub=(uint32_t)b,ur;float x,y;volatile float z;__builtin_memcpy(&x,&ua,4);__builtin_memcpy(&y,&ub,4);switch(op){case 1:z=x+y;break;case 2:z=x-y;break;case 3:z=x*y;break;case 4:z=x/y;break;case 6:return x==y;case 7:return x!=y;case 8:return x<y;case 9:return x<=y;case 10:return x>y;case 11:return x>=y;case 17:return ua^UINT32_C(0x80000000);case 19:return ua&UINT32_C(0x7fffffff);default:__builtin_trap();}__builtin_memcpy(&ur,(const void*)&z,4);return o_fpn(11,ua,ub,ur);}uint64_t ur;double x,y;volatile double z;__builtin_memcpy(&x,&a,8);__builtin_memcpy(&y,&b,8);switch(op){case 1:z=x+y;break;case 2:z=x-y;break;case 3:z=x*y;break;case 4:z=x/y;break;case 6:return x==y;case 7:return x!=y;case 8:return x<y;case 9:return x<=y;case 10:return x>y;case 11:return x>=y;case 17:return a^UINT64_C(0x8000000000000000);case 19:return a&UINT64_C(0x7fffffffffffffff);default:__builtin_trap();}__builtin_memcpy(&ur,(const void*)&z,8);return o_fpn(12,a,b,ur);}
static uint64_t o_fcv(unsigned t,unsigned s,uint64_t a){uint32_t u=(uint32_t)a;float f;double x;if(s==t)return a;if(s<11){if(t==11){volatile float z=s>=6?(float)(int64_t)a:(float)a;__builtin_memcpy(&u,(const void*)&z,4);return u;}volatile double z=s>=6?(double)(int64_t)a:(double)a;__builtin_memcpy(&a,(const void*)&z,8);return a;}if(t==12){if(o_fnan(11,a))return (uint64_t)(u>>31)<<63|UINT64_C(0x7ff8000000000000)|(uint64_t)(u&UINT32_C(0x7fffff))<<29;__builtin_memcpy(&f,&u,4);volatile double z=f;__builtin_memcpy(&a,(const void*)&z,8);return a;}if(t==11){if(o_fnan(12,a))return (a>>63)<<31|UINT64_C(0x7fc00000)|(a>>29&UINT64_C(0x3fffff));__builtin_memcpy(&x,&a,8);volatile float z=(float)x;__builtin_memcpy(&u,(const void*)&z,4);return u;}if(s==11){__builtin_memcpy(&f,&u,4);x=f;}else __builtin_memcpy(&x,&a,8);if(x!=x)return 0;unsigned w=o_width(t);if(t>=6){double m=(double)((uint64_t)1<<(w-1));if(x>=m)return ((uint64_t)1<<(w-1))-1;if(x<-m)return (uint64_t)0-((uint64_t)1<<(w-1));return (uint64_t)(int64_t)x;}if(x<1.0)return 0;if(x>=(w==64?18446744073709551616.0:(double)((uint64_t)1<<w)))return w==64?UINT64_MAX:((uint64_t)1<<w)-1;return (uint64_t)x;}
static uint64_t f0(uint64_t v0, uint64_t v1, void *v2, uint64_t v2_n);
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n);
static uint64_t f2(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n);
static uint64_t f4(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n);
static uint64_t f5(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n);
static uint64_t f6(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n);
static uint64_t f7(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f8(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f9(uint64_t v0, uint64_t v1);
static uint64_t f10(uint64_t v0);
static uint64_t f11(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f12(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f13(uint64_t v0, uint64_t v1, uint64_t v2, void *v3, uint64_t v3_n);
static uint64_t f14(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f15(uint64_t v0, uint64_t v1);
static uint64_t f16(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f17(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f18(void *v0, uint64_t v0_n);
static uint64_t f19(uint64_t v0);
static uint64_t f20(uint64_t v0);
static uint64_t f21(uint64_t v0);
static uint64_t f22(uint64_t v0, uint64_t v1);
static uint64_t f23(uint64_t v0);
static uint64_t f24(uint64_t v0);
static uint64_t f25(uint64_t v0);
static uint64_t f26(uint64_t v0, uint64_t v1);
static uint64_t f0(uint64_t v0, uint64_t v1, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=v0;uint64_t _b=v1;o_fp(1, 12, _a,_b);});
{uint64_t _i=0ULL;uint64_t _v=v3;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=({uint64_t _a=v3;uint64_t _b=v0;o_fp(2, 12, _a,_b);});o_fp(2, 12, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=v0;uint64_t _b=v2;o_fp(1, 12, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v5;uint64_t _b=v0;o_fp(2, 12, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v5;uint64_t _b=v6;o_fp(2, 12, _a,_b);});o_fp(2, 12, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=v6;o_fp(2, 12, _a,_b);});o_fp(1, 12, _a,_b);});
v7=({uint64_t _a=v7;uint64_t _b=({uint64_t _a=v1;uint64_t _b=v3;o_fp(1, 12, _a,_b);});o_fp(1, 12, _a,_b);});
({uint64_t p0=v5;uint64_t p1=v7;f0(p0, p1, v4, v4_n);});
return o_leave(0);
}
static uint64_t f2(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=4728779608772575232ULL;
uint64_t __attribute__((unused)) v4=({uint64_t _a=v3;uint64_t _b=v0;o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v3;uint64_t _b=v1;o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v4;uint64_t _b=({uint64_t _a=v4;uint64_t _b=v0;o_fp(2, 12, _a,_b);});o_fp(2, 12, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=v5;uint64_t _b=v1;o_fp(2, 12, _a,_b);});o_fp(2, 12, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v0;uint64_t _b=v6;o_fp(2, 12, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=v1;uint64_t _b=v7;o_fp(2, 12, _a,_b);});
uint64_t __attribute__((unused)) v10=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=v7;o_fp(3, 12, _a,_b);});uint64_t _b=v2;o_fp(2, 12, _a,_b);});
v10=({uint64_t _a=v10;uint64_t _b=({uint64_t _a=v6;uint64_t _b=v9;o_fp(3, 12, _a,_b);});o_fp(1, 12, _a,_b);});
v10=({uint64_t _a=v10;uint64_t _b=({uint64_t _a=v8;uint64_t _b=v7;o_fp(3, 12, _a,_b);});o_fp(1, 12, _a,_b);});
return o_leave(({uint64_t _a=v10;uint64_t _b=({uint64_t _a=v8;uint64_t _b=v9;o_fp(3, 12, _a,_b);});o_fp(1, 12, _a,_b);}));
}
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=v0;uint64_t _b=v2;o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p0=v0;uint64_t p1=v2;uint64_t p2=v5;f2(p0, p1, p2);});
v6=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v3;o_fp(3, 12, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=v2;o_fp(3, 12, _a,_b);});o_fp(1, 12, _a,_b);});o_fp(1, 12, _a,_b);});
v6=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=v1;uint64_t _b=v3;o_fp(3, 12, _a,_b);});o_fp(1, 12, _a,_b);});
({uint64_t p0=v5;uint64_t p1=v6;f0(p0, p1, v4, v4_n);});
return o_leave(0);
}
static uint64_t f4(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=v0;uint64_t _b=v2;o_fp(4, 12, _a,_b);});
({uint64_t p0=v5;uint64_t p1=0ULL;uint64_t p2=v2;uint64_t p3=v3;f3(p0, p1, p2, p3, v4, v4_n);});
({uint64_t p0=v0;uint64_t p1=v1;uint64_t p2=({uint64_t _a=o_get(v4, v4_n, 0ULL, 5);uint64_t _b=0;o_fp(17, 12, _a,_b);});uint64_t p3=({uint64_t _a=o_get(v4, v4_n, 1ULL, 5);uint64_t _b=0;o_fp(17, 12, _a,_b);});f1(p0, p1, p2, p3, v4, v4_n);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=o_get(v4, v4_n, 0ULL, 5);uint64_t _b=o_get(v4, v4_n, 1ULL, 5);o_fp(1, 12, _a,_b);});uint64_t _b=v2;o_fp(4, 12, _a,_b);});
({uint64_t p0=v5;uint64_t p1=v6;f0(p0, p1, v4, v4_n);});
return o_leave(0);
}
static uint64_t f5(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n){
o_enter();
({uint64_t p0=o_get(v0, v0_n, 0ULL, 5);uint64_t p1=o_get(v0, v0_n, 1ULL, 5);uint64_t p2=o_get(v1, v1_n, 0ULL, 5);uint64_t p3=o_get(v1, v1_n, 1ULL, 5);f1(p0, p1, p2, p3, v0, v0_n);});
return o_leave(0);
}
static uint64_t f6(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n){
o_enter();
({uint64_t p0=o_get(v0, v0_n, 0ULL, 5);uint64_t p1=o_get(v0, v0_n, 1ULL, 5);uint64_t p2=o_get(v1, v1_n, 0ULL, 5);uint64_t p3=o_get(v1, v1_n, 1ULL, 5);f3(p0, p1, p2, p3, v0, v0_n);});
return o_leave(0);
}
static uint64_t f7(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
({uint64_t p0=o_get(v0, v0_n, 0ULL, 5);uint64_t p1=o_get(v0, v0_n, 1ULL, 5);uint64_t p2=v1;uint64_t p3=0ULL;f3(p0, p1, p2, p3, v0, v0_n);});
return o_leave(0);
}
static uint64_t f8(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
uint64_t v2[2]={0};
uint64_t v2_n=2;
o_fill(v2, v2_n, 0ULL, 5);
uint64_t v3[2]={0};
uint64_t v3_n=2;
o_fill(v3, v3_n, 0ULL, 5);
uint64_t v4[2]={0};
uint64_t v4_n=2;
o_fill(v4, v4_n, 0ULL, 5);
({uint64_t p0=v0;uint64_t p1=0ULL;uint64_t p2=4607182418800017408ULL;uint64_t p3=0ULL;f1(p0, p1, p2, p3, v4, v4_n);});
({uint64_t p0=({uint64_t _a=v0;uint64_t _b=4607182418800017408ULL;o_fp(2, 12, _a,_b);});uint64_t p1=0ULL;uint64_t p2=o_get(v4, v4_n, 0ULL, 5);uint64_t p3=o_get(v4, v4_n, 1ULL, 5);f4(p0, p1, p2, p3, v2, v2_n);});
{uint64_t _i=0ULL;uint64_t _v=o_get(v2, v2_n, 0ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=o_get(v2, v2_n, 1ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=o_get(v2, v2_n, 0ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=o_get(v2, v2_n, 1ULL, 5);o_put(v1, v1_n, _i,_v,5);}
({uint64_t p0=o_get(v2, v2_n, 0ULL, 5);uint64_t p1=o_get(v2, v2_n, 1ULL, 5);uint64_t p2=o_get(v2, v2_n, 0ULL, 5);uint64_t p3=o_get(v2, v2_n, 1ULL, 5);f3(p0, p1, p2, p3, v2, v2_n);});
{
uint64_t v5=0;
uint64_t k1=32ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
({f6(v3, v3_n, v2, v2_n);});
uint64_t __attribute__((unused)) v6=o_fcv(12, 5, ({uint64_t _a=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=3ULL;o_op(1, 5, _a,_b);}));
({uint64_t p0=o_get(v3, v3_n, 0ULL, 5);uint64_t p1=o_get(v3, v3_n, 1ULL, 5);uint64_t p2=v6;uint64_t p3=0ULL;f4(p0, p1, p2, p3, v4, v4_n);});
({f5(v1, v1_n, v4, v4_n);});
}
}
return o_leave(0);
}
static uint64_t f9(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=v0;
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=9223372036854775807ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(18442240474082181120ULL);
}
if(({uint64_t _a=v0;uint64_t _b=v0;o_fp(7, 12, _a,_b);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=v0;o_fp(1, 12, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);})){
return o_leave(9221120237041090560ULL);
}
if(({uint64_t _a=v2;uint64_t _b=9218868437227405312ULL;o_op(6, 5, _a,_b);})){
return o_leave(v0);
}
uint64_t __attribute__((unused)) v3=0ULL;
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=52ULL;o_op(16, 5, _a,_b);});uint64_t _b=2047ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v0=({uint64_t _a=v0;uint64_t _b=4850376798678024192ULL;o_fp(3, 12, _a,_b);});
v2=v0;
v3=18446744073709551562ULL;
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=o_cast(9, 5, ({uint64_t _a=({uint64_t _a=v2;uint64_t _b=52ULL;o_op(16, 5, _a,_b);});uint64_t _b=2047ULL;o_op(12, 5, _a,_b);}));uint64_t _b=1023ULL;o_op(2, 9, _a,_b);});uint64_t _b=v3;o_op(1, 9, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=4503599627370495ULL;o_op(12, 5, _a,_b);});uint64_t _b=4607182418800017408ULL;o_op(13, 5, _a,_b);});
if(({uint64_t _a=v5;uint64_t _b=4609047870845172685ULL;o_fp(10, 12, _a,_b);})){
v5=({uint64_t _a=v5;uint64_t _b=4602678819172646912ULL;o_fp(3, 12, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 9, _a,_b);});
}
uint64_t v6[2]={0};
uint64_t v6_n=2;
o_fill(v6, v6_n, 0ULL, 5);
uint64_t v7[2]={0};
uint64_t v7_n=2;
o_fill(v7, v7_n, 0ULL, 5);
({uint64_t p0=v5;f8(p0, v6, v6_n);});
({uint64_t p0=o_fcv(12, 9, v4);uint64_t p1=0ULL;uint64_t p2=4604418534313441775ULL;uint64_t p3=4358002977218854975ULL;f3(p0, p1, p2, p3, v7, v7_n);});
({uint64_t p1=4611686018427387904ULL;f7(v6, v6_n, p1);});
({f5(v6, v6_n, v7, v7_n);});
if(v1){
({uint64_t p0=o_get(v6, v6_n, 0ULL, 5);uint64_t p1=o_get(v6, v6_n, 1ULL, 5);uint64_t p2=4609176140021203710ULL;uint64_t p3=4357082904033627428ULL;f3(p0, p1, p2, p3, v6, v6_n);});
}
return o_leave(({uint64_t _a=o_get(v6, v6_n, 0ULL, 5);uint64_t _b=o_get(v6, v6_n, 1ULL, 5);o_fp(1, 12, _a,_b);}));
}
static uint64_t f10(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=v0;
if(({uint64_t _a=v1;uint64_t _b=v1;o_fp(7, 12, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=v1;o_fp(1, 12, _a,_b);}));
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=52ULL;o_op(16, 5, _a,_b);});uint64_t _b=2047ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=1075ULL;o_op(11, 5, _a,_b);})){
return o_leave(v0);
}
if(({uint64_t _a=v2;uint64_t _b=1023ULL;o_op(8, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9223372036854775807ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(v0);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9223372036854775808ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(9223372036854775808ULL);
}
return o_leave(4607182418800017408ULL);
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=1075ULL;uint64_t _b=v2;o_op(2, 5, _a,_b);});o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v3;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(v0);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9223372036854775808ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v0=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_op(1, 5, _a,_b);});
}
return o_leave(({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v3;uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);}));
}
static uint64_t f11(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
({uint64_t p0=o_get(v0, v0_n, 0ULL, 5);uint64_t p1=o_get(v0, v0_n, 1ULL, 5);uint64_t p2=v1;uint64_t p3=0ULL;f4(p0, p1, p2, p3, v0, v0_n);});
return o_leave(0);
}
static uint64_t f12(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t v2[2]={0};
uint64_t v2_n=2;
o_fill(v2, v2_n, 0ULL, 5);
{uint64_t _i=0ULL;uint64_t _v=4607182418800017408ULL;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=4607182418800017408ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{
uint64_t v3=0;
uint64_t k1=40ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({f6(v2, v2_n, v0, v0_n);});
({uint64_t p1=o_fcv(12, 5, ({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));f11(v2, v2_n, p1);});
({f5(v1, v1_n, v2, v2_n);});
}
}
return o_leave(0);
}
static uint64_t f13(uint64_t v0, uint64_t v1, uint64_t v2, void *v3, uint64_t v3_n){
o_enter();
uint64_t v4[2]={0};
uint64_t v4_n=2;
o_fill(v4, v4_n, 0ULL, 5);
uint64_t __attribute__((unused)) v5=v0;
uint64_t __attribute__((unused)) v6=0ULL;
if(({uint64_t _a=v2;uint64_t _b=0;o_op(18, 1, _a,_b);})){
v5=({uint64_t _a=v0;uint64_t _b=4609176140021203710ULL;o_fp(3, 12, _a,_b);});
}
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_fp(11, 12, _a,_b);})){
v6=o_fcv(9, 12, ({uint64_t _a=v5;uint64_t _b=4602678819172646912ULL;o_fp(1, 12, _a,_b);}));
}else{
v6=o_fcv(9, 12, ({uint64_t _a=v5;uint64_t _b=4602678819172646912ULL;o_fp(2, 12, _a,_b);}));
}
if(v2){
({uint64_t p0=v0;uint64_t p1=v1;uint64_t p2=({uint64_t _a=o_fcv(12, 9, v6);uint64_t _b=0;o_fp(17, 12, _a,_b);});uint64_t p3=0ULL;f1(p0, p1, p2, p3, v4, v4_n);});
({uint64_t p0=o_get(v4, v4_n, 0ULL, 5);uint64_t p1=o_get(v4, v4_n, 1ULL, 5);uint64_t p2=4604418534313441775ULL;uint64_t p3=4358002977218854975ULL;f3(p0, p1, p2, p3, v4, v4_n);});
}else{
({uint64_t p0=({uint64_t _a=o_fcv(12, 9, v6);uint64_t _b=0;o_fp(17, 12, _a,_b);});uint64_t p1=0ULL;uint64_t p2=4604418534313441775ULL;uint64_t p3=4358002977218854975ULL;f3(p0, p1, p2, p3, v4, v4_n);});
({uint64_t p0=v0;uint64_t p1=v1;uint64_t p2=o_get(v4, v4_n, 0ULL, 5);uint64_t p3=o_get(v4, v4_n, 1ULL, 5);f1(p0, p1, p2, p3, v4, v4_n);});
}
({f12(v4, v4_n, v3, v3_n);});
return o_leave(v6);
}
static uint64_t f14(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
if(({uint64_t _a=v1;uint64_t _b=18446744073709550616ULL;o_op(8, 9, _a,_b);})){
uint64_t __attribute__((unused)) v2=({uint64_t _a=o_cast(5, 9, ({uint64_t _a=v1;uint64_t _b=2097ULL;o_op(1, 9, _a,_b);}));uint64_t _b=52ULL;o_op(15, 5, _a,_b);});
({uint64_t p1=v2;f7(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=o_fcv(12, 9, 4503599627370496ULL);o_fp(9, 12, _a,_b);})){
uint64_t __attribute__((unused)) v5=o_fcv(5, 12, v3);
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=o_fcv(12, 5, v5);o_fp(2, 12, _a,_b);});uint64_t _b=v4;o_fp(1, 12, _a,_b);});
if(({uint64_t _a=v6;uint64_t _b=4602678819172646912ULL;o_fp(10, 12, _a,_b);})){
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}else{
if(({uint64_t _a=v6;uint64_t _b=13826050856027422720ULL;o_fp(8, 12, _a,_b);})){
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}else{
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=4602678819172646912ULL;o_fp(6, 12, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=13826050856027422720ULL;o_fp(6, 12, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
}
}
return o_leave(v5);
}
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v4;o_fp(1, 12, _a,_b);});uint64_t _b=4503599627370496ULL;o_fp(3, 12, _a,_b);});
return o_leave(({uint64_t _a=v5;uint64_t _b=4372995238176751616ULL;o_fp(3, 12, _a,_b);}));
}
if(({uint64_t _a=v1;uint64_t _b=1024ULL;o_op(6, 9, _a,_b);})){
({uint64_t p1=4611686018427387904ULL;f7(v0, v0_n, p1);});
v1=1023ULL;
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=o_cast(5, 9, ({uint64_t _a=v1;uint64_t _b=1023ULL;o_op(1, 9, _a,_b);}));uint64_t _b=52ULL;o_op(15, 5, _a,_b);});
return o_leave(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=o_get(v0, v0_n, 1ULL, 5);o_fp(1, 12, _a,_b);});uint64_t _b=v2;o_fp(3, 12, _a,_b);}));
}
static uint64_t f15(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=v0;
if(({uint64_t _a=v2;uint64_t _b=v2;o_fp(7, 12, _a,_b);})){
return o_leave(({uint64_t _a=v2;uint64_t _b=v2;o_fp(1, 12, _a,_b);}));
}
if(v1){
if(({uint64_t _a=v2;uint64_t _b=4652218415073722368ULL;o_fp(11, 12, _a,_b);})){
return o_leave(9218868437227405312ULL);
}
if(({uint64_t _a=v2;uint64_t _b=13875814752300564480ULL;o_fp(9, 12, _a,_b);})){
return o_leave(0ULL);
}
}else{
if(({uint64_t _a=v2;uint64_t _b=4649456441864749056ULL;o_fp(10, 12, _a,_b);})){
return o_leave(9218868437227405312ULL);
}
if(({uint64_t _a=v2;uint64_t _b=13873145138068324352ULL;o_fp(8, 12, _a,_b);})){
return o_leave(0ULL);
}
}
uint64_t v3[2]={0};
uint64_t v3_n=2;
o_fill(v3, v3_n, 0ULL, 5);
uint64_t __attribute__((unused)) v4=({uint64_t p0=v2;uint64_t p1=0ULL;uint64_t p2=v1;f13(p0, p1, p2, v3, v3_n);});
if(({uint64_t _a=v4;uint64_t _b=1024ULL;o_op(10, 9, _a,_b);})){
return o_leave(9218868437227405312ULL);
}
if(({uint64_t _a=v4;uint64_t _b=18446744073709550541ULL;o_op(8, 9, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t p1=v4;f14(v3, v3_n, p1);}));
}
static uint64_t f16(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
uint64_t v2[2]={0};
uint64_t v2_n=2;
o_fill(v2, v2_n, 0ULL, 5);
uint64_t v3[2]={0};
uint64_t v3_n=2;
o_fill(v3, v3_n, 0ULL, 5);
uint64_t v4[2]={0};
uint64_t v4_n=2;
o_fill(v4, v4_n, 0ULL, 5);
{uint64_t _i=0ULL;uint64_t _v=v0;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=v0;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p0=({uint64_t _a=v0;uint64_t _b=0;o_fp(17, 12, _a,_b);});uint64_t p1=0ULL;uint64_t p2=v0;uint64_t p3=0ULL;f3(p0, p1, p2, p3, v4, v4_n);});
{
uint64_t v5=0;
uint64_t k1=64ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
({f6(v2, v2_n, v4, v4_n);});
({uint64_t p1=o_fcv(12, 5, ({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));f11(v2, v2_n, p1);});
({uint64_t p0=o_get(v2, v2_n, 0ULL, 5);uint64_t p1=o_get(v2, v2_n, 1ULL, 5);uint64_t p2=o_fcv(12, 5, ({uint64_t _a=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=3ULL;o_op(1, 5, _a,_b);}));uint64_t p3=0ULL;f4(p0, p1, p2, p3, v3, v3_n);});
({f5(v1, v1_n, v3, v3_n);});
}
}
({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=o_get(v1, v1_n, 1ULL, 5);uint64_t p2=4603256987541740397ULL;uint64_t p3=4350950281585880448ULL;f3(p0, p1, p2, p3, v1, v1_n);});
({uint64_t p1=13835058055282163712ULL;f7(v1, v1_n, p1);});
({uint64_t p0=4607182418800017408ULL;uint64_t p1=0ULL;uint64_t p2=o_get(v1, v1_n, 0ULL, 5);uint64_t p3=o_get(v1, v1_n, 1ULL, 5);f1(p0, p1, p2, p3, v1, v1_n);});
return o_leave(0);
}
static uint64_t f17(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
uint64_t v2[2]={0};
uint64_t v2_n=2;
o_fill(v2, v2_n, 0ULL, 5);
{uint64_t _i=0ULL;uint64_t _v=v0;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{
uint64_t v3=0;
uint64_t k1=512ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
uint64_t __attribute__((unused)) v4=({uint64_t _a=512ULL;uint64_t _b=v3;o_op(2, 5, _a,_b);});
({uint64_t p0=({uint64_t _a=o_fcv(12, 5, v4);uint64_t _b=4602678819172646912ULL;o_fp(3, 12, _a,_b);});uint64_t p1=0ULL;uint64_t p2=o_get(v1, v1_n, 0ULL, 5);uint64_t p3=o_get(v1, v1_n, 1ULL, 5);f4(p0, p1, p2, p3, v2, v2_n);});
({uint64_t p0=v0;uint64_t p1=0ULL;uint64_t p2=o_get(v2, v2_n, 0ULL, 5);uint64_t p3=o_get(v2, v2_n, 1ULL, 5);f1(p0, p1, p2, p3, v1, v1_n);});
}
}
({uint64_t p0=4603256987541740397ULL;uint64_t p1=4350950281585880448ULL;uint64_t p2=o_get(v1, v1_n, 0ULL, 5);uint64_t p3=o_get(v1, v1_n, 1ULL, 5);f4(p0, p1, p2, p3, v1, v1_n);});
return o_leave(0);
}
static uint64_t f18(void *v0, uint64_t v0_n){
o_enter();
({uint64_t p0=4611686018427387904ULL;uint64_t p1=0ULL;uint64_t p2=({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=0;o_fp(17, 12, _a,_b);});uint64_t p3=({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0;o_fp(17, 12, _a,_b);});f1(p0, p1, p2, p3, v0, v0_n);});
return o_leave(0);
}
static uint64_t f19(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=v0;uint64_t _b=9223372036854775807ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v1;uint64_t _b=v1;o_fp(7, 12, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=v1;o_fp(1, 12, _a,_b);}));
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9223372036854775808ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});
if(({uint64_t _a=v1;uint64_t _b=4628574517030027264ULL;o_fp(11, 12, _a,_b);})){
if(v2){
return o_leave(4611686018427387904ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v1;uint64_t _b=4618441417868443648ULL;o_fp(11, 12, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(4611686018427387904ULL);
}
uint64_t v3[2]={0};
uint64_t v3_n=2;
o_fill(v3, v3_n, 0ULL, 5);
if(({uint64_t _a=v1;uint64_t _b=4611686018427387904ULL;o_fp(8, 12, _a,_b);})){
({uint64_t p0=v1;f16(p0, v3, v3_n);});
if(v2){
({f18(v3, v3_n);});
}
return o_leave(({uint64_t _a=o_get(v3, v3_n, 0ULL, 5);uint64_t _b=o_get(v3, v3_n, 1ULL, 5);o_fp(1, 12, _a,_b);}));
}
uint64_t v4[2]={0};
uint64_t v4_n=2;
o_fill(v4, v4_n, 0ULL, 5);
({uint64_t p0=v1;f17(p0, v4, v4_n);});
({uint64_t p0=({uint64_t _a=v1;uint64_t _b=0;o_fp(17, 12, _a,_b);});uint64_t p1=0ULL;uint64_t p2=v1;uint64_t p3=0ULL;f3(p0, p1, p2, p3, v3, v3_n);});
uint64_t __attribute__((unused)) v5=o_get(v3, v3_n, 0ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v3, v3_n, 1ULL, 5);
uint64_t __attribute__((unused)) v7=({uint64_t p0=v5;uint64_t p1=v6;uint64_t p2=0ULL;f13(p0, p1, p2, v3, v3_n);});
({f6(v3, v3_n, v4, v4_n);});
if(v2){
uint64_t __attribute__((unused)) v8=({uint64_t _a=o_cast(5, 9, ({uint64_t _a=v7;uint64_t _b=1023ULL;o_op(1, 9, _a,_b);}));uint64_t _b=52ULL;o_op(15, 5, _a,_b);});
({uint64_t p1=v8;f7(v3, v3_n, p1);});
({f18(v3, v3_n);});
return o_leave(({uint64_t _a=o_get(v3, v3_n, 0ULL, 5);uint64_t _b=o_get(v3, v3_n, 1ULL, 5);o_fp(1, 12, _a,_b);}));
}
if(({uint64_t _a=v7;uint64_t _b=18446744073709550541ULL;o_op(8, 9, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t p1=v7;f14(v3, v3_n, p1);}));
}
static uint64_t f20(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=v0;
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=4503599627370495ULL;o_op(12, 5, _a,_b);});uint64_t _b=4503599627370496ULL;o_op(13, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=52ULL;uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=52ULL;o_op(16, 5, _a,_b);});uint64_t _b=2047ULL;o_op(12, 5, _a,_b);});uint64_t _b=1023ULL;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=0ULL;
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t v6=0;
uint64_t k1=53ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
uint64_t __attribute__((unused)) v7=({uint64_t _a=105ULL;uint64_t _b=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=0ULL;
if(({uint64_t _a=v7;uint64_t _b=v3;o_op(11, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v7;uint64_t _b=v3;o_op(2, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
}
if(({uint64_t _a=v8;uint64_t _b=v3;o_op(11, 5, _a,_b);})){
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v8;uint64_t _b=v3;o_op(2, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});o_op(13, 5, _a,_b);});
}
v5=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(15, 5, _a,_b);});uint64_t _b=v9;o_op(13, 5, _a,_b);});
uint64_t __attribute__((unused)) v10=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(13, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
if(({uint64_t _a=v5;uint64_t _b=v10;o_op(11, 5, _a,_b);})){
v5=({uint64_t _a=v5;uint64_t _b=v10;o_op(2, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
}
if(({uint64_t _a=v5;uint64_t _b=v4;o_op(10, 5, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
return o_leave(({uint64_t _a=o_fcv(12, 5, v4);uint64_t _b=4372995238176751616ULL;o_fp(3, 12, _a,_b);}));
}
static uint64_t f21(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=v0;
if(({uint64_t _a=v1;uint64_t _b=v1;o_fp(7, 12, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=v1;o_fp(1, 12, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9223372036854775807ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(v0);
}
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);})){
return o_leave(9221120237041090560ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9218868437227405312ULL;o_op(6, 5, _a,_b);})){
return o_leave(v0);
}
uint64_t __attribute__((unused)) v2=0ULL;
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=52ULL;o_op(16, 5, _a,_b);});uint64_t _b=2047ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t _a=v1;uint64_t _b=4850376798678024192ULL;o_fp(3, 12, _a,_b);});
v0=v1;
v2=18446744073709551562ULL;
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=o_cast(9, 5, ({uint64_t _a=({uint64_t _a=v0;uint64_t _b=52ULL;o_op(16, 5, _a,_b);});uint64_t _b=2047ULL;o_op(12, 5, _a,_b);}));uint64_t _b=1023ULL;o_op(2, 9, _a,_b);});uint64_t _b=v2;o_op(1, 9, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=4503599627370495ULL;o_op(12, 5, _a,_b);});uint64_t _b=4607182418800017408ULL;o_op(13, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(12, 9, _a,_b);});uint64_t _b=0ULL;o_op(7, 9, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=4611686018427387904ULL;o_fp(3, 12, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 9, _a,_b);});
}
uint64_t __attribute__((unused)) v5=({uint64_t _a=o_cast(5, 9, ({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(4, 9, _a,_b);});uint64_t _b=1023ULL;o_op(1, 9, _a,_b);}));uint64_t _b=52ULL;o_op(15, 5, _a,_b);});
return o_leave(({uint64_t _a=({uint64_t p0=v4;f20(p0);});uint64_t _b=v5;o_fp(3, 12, _a,_b);}));
}
static uint64_t f22(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=0ULL;f9(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=1ULL;f9(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f10(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=0ULL;f15(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=1ULL;f15(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f21(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=9223372036854775807ULL;o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f19(p0);}));
}
return o_leave(9221120237041090560ULL);
}
static uint64_t f23(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=13798997430714945504ULL;
v1=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=v0;o_fp(3, 12, _a,_b);});uint64_t _b=13822851435045880248ULL;o_fp(1, 12, _a,_b);});
v1=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=v0;o_fp(3, 12, _a,_b);});uint64_t _b=13835960482696009560ULL;o_fp(1, 12, _a,_b);});
v1=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=v0;o_fp(3, 12, _a,_b);});uint64_t _b=13836295942911834650ULL;o_fp(1, 12, _a,_b);});
v1=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=v0;o_fp(3, 12, _a,_b);});uint64_t _b=4616611452376731079ULL;o_fp(1, 12, _a,_b);});
v1=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=v0;o_fp(3, 12, _a,_b);});uint64_t _b=4613798575908835234ULL;o_fp(1, 12, _a,_b);});
uint64_t __attribute__((unused)) v2=4575625165243457492ULL;
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v0;o_fp(3, 12, _a,_b);});uint64_t _b=4599480671287182180ULL;o_fp(1, 12, _a,_b);});
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v0;o_fp(3, 12, _a,_b);});uint64_t _b=4612688371394471446ULL;o_fp(1, 12, _a,_b);});
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v0;o_fp(3, 12, _a,_b);});uint64_t _b=4615636595525398809ULL;o_fp(1, 12, _a,_b);});
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v0;o_fp(3, 12, _a,_b);});uint64_t _b=4607182418800017408ULL;o_fp(1, 12, _a,_b);});
return o_leave(({uint64_t _a=v1;uint64_t _b=v2;o_fp(4, 12, _a,_b);}));
}
static uint64_t f24(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=v0;uint64_t _b=v0;o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v2=13854155686354486565ULL;
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v1;o_fp(3, 12, _a,_b);});uint64_t _b=4641977866302784411ULL;o_fp(1, 12, _a,_b);});
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v1;o_fp(3, 12, _a,_b);});uint64_t _b=13866933838737128345ULL;o_fp(1, 12, _a,_b);});
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v1;o_fp(3, 12, _a,_b);});uint64_t _b=4639072047187312667ULL;o_fp(1, 12, _a,_b);});
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v1;o_fp(3, 12, _a,_b);});uint64_t _b=13852696627858410445ULL;o_fp(1, 12, _a,_b);});
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v1;o_fp(3, 12, _a,_b);});uint64_t _b=4612826843888178297ULL;o_fp(1, 12, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=v0;o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v3=13856235683484533956ULL;
v3=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v1;o_fp(3, 12, _a,_b);});uint64_t _b=4639889312772538999ULL;o_fp(1, 12, _a,_b);});
v3=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v1;o_fp(3, 12, _a,_b);});uint64_t _b=13863054224260258002ULL;o_fp(1, 12, _a,_b);});
v3=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v1;o_fp(3, 12, _a,_b);});uint64_t _b=4634401141363829182ULL;o_fp(1, 12, _a,_b);});
v3=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v1;o_fp(3, 12, _a,_b);});uint64_t _b=13847038013971134502ULL;o_fp(1, 12, _a,_b);});
v3=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v1;o_fp(3, 12, _a,_b);});uint64_t _b=4607182418800017408ULL;o_fp(1, 12, _a,_b);});
return o_leave(({uint64_t _a=v2;uint64_t _b=v3;o_fp(4, 12, _a,_b);}));
}
static uint64_t f25(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_fp(9, 12, _a,_b);})){
return o_leave(18442240474082181120ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4607182418800017408ULL;o_fp(11, 12, _a,_b);})){
return o_leave(9218868437227405312ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4582646808030102946ULL;o_fp(8, 12, _a,_b);})){
uint64_t __attribute__((unused)) v1=({uint64_t _a=13835058055282163712ULL;uint64_t _b=({uint64_t p0=v0;uint64_t p1=0ULL;f9(p0, p1);});o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v2=({uint64_t p0=v1;f21(p0);});
return o_leave(({uint64_t p0=v2;f23(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=4606963994218089939ULL;o_fp(10, 12, _a,_b);})){
uint64_t __attribute__((unused)) v1=({uint64_t _a=13835058055282163712ULL;uint64_t _b=({uint64_t p0=({uint64_t _a=4607182418800017408ULL;uint64_t _b=v0;o_fp(2, 12, _a,_b);});uint64_t p1=0ULL;f9(p0, p1);});o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v2=({uint64_t p0=v1;f21(p0);});
return o_leave(({uint64_t _a=({uint64_t p0=v2;f23(p0);});uint64_t _b=0;o_fp(17, 12, _a,_b);}));
}
return o_leave(({uint64_t p0=({uint64_t _a=v0;uint64_t _b=4602678819172646912ULL;o_fp(2, 12, _a,_b);});f24(p0);}));
}
static uint64_t f26(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=v0;
uint64_t __attribute__((unused)) v3=v1;
uint64_t __attribute__((unused)) v4=({uint64_t p0=({uint64_t _a=4602678819172646912ULL;uint64_t _b=({uint64_t _a=4602678819172646912ULL;uint64_t _b=v2;o_fp(3, 12, _a,_b);});o_fp(1, 12, _a,_b);});f25(p0);});
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(v4);
}
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=v4;o_fp(3, 12, _a,_b);});uint64_t _b=v4;o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=v4;o_fp(3, 12, _a,_b);});uint64_t _b=v4;o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=v4;o_fp(1, 12, _a,_b);});uint64_t _b=({uint64_t _a=4616189618054758400ULL;uint64_t _b=v3;o_fp(3, 12, _a,_b);});o_fp(4, 12, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=({uint64_t _a=4617315517961601024ULL;uint64_t _b=v6;o_fp(3, 12, _a,_b);});uint64_t _b=({uint64_t _a=4625196817309499392ULL;uint64_t _b=v5;o_fp(3, 12, _a,_b);});o_fp(1, 12, _a,_b);});
v8=({uint64_t _a=v8;uint64_t _b=({uint64_t _a=4613937818241073152ULL;uint64_t _b=v4;o_fp(3, 12, _a,_b);});o_fp(1, 12, _a,_b);});
v8=({uint64_t _a=v8;uint64_t _b=({uint64_t _a=({uint64_t _a=4636455816377925632ULL;uint64_t _b=v3;o_fp(3, 12, _a,_b);});uint64_t _b=v3;o_fp(3, 12, _a,_b);});o_fp(4, 12, _a,_b);});
return o_leave(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=v7;o_fp(1, 12, _a,_b);});uint64_t _b=v8;o_fp(1, 12, _a,_b);}));
}
uint32_t omega_cm_quantile(uint64_t *result, volatile uint64_t p0, volatile uint64_t p1){
if(!o_buf(result,1,1,5))return 32;
if(!o_valid(5, p0))return 32;
if(!o_valid(5, p1))return 32;
if(o_busy)return 33;
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f26(p0, p1);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
