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
static uint64_t o_get(const void *p,uint64_t n,uint64_t i,unsigned t){if(i>=n)o_trap(3);unsigned w=t==11?32:t==12?64:o_width(t);const uint8_t *b=(const uint8_t*)p+i*(w/8);uint64_t v=0;if(w==8)v=b[0];else if(w==16){uint16_t x;__builtin_memcpy(&x,b,2);v=x;}else if(w==32){uint32_t x;__builtin_memcpy(&x,b,4);v=x;}else if(w==64)__builtin_memcpy(&v,b,8);if(t>=6&&t<=9&&w<64&&(v>> (w-1)))v|=UINT64_MAX<<w;return v;}
static void o_put(void *p,uint64_t n,uint64_t i,uint64_t v,unsigned t){if(i>=n)o_trap(3);unsigned w=t==11?32:t==12?64:o_width(t);if(t<11)o_norm(t,o_value(t,v),5);uint8_t *b=(uint8_t*)p+i*(w/8);if(w==8)b[0]=(uint8_t)v;else if(w==16){uint16_t x=(uint16_t)v;__builtin_memcpy(b,&x,2);}else if(w==32){uint32_t x=(uint32_t)v;__builtin_memcpy(b,&x,4);}else if(w==64)__builtin_memcpy(b,&v,8);}
static void o_fill(void *p,uint64_t n,uint64_t v,unsigned t){for(uint64_t i=0;i<n;i++)o_put(p,n,i,v,t);}
static uint64_t o_io(unsigned op,uint64_t fd,const void *p,uint64_t n,uint64_t off,uint64_t len){if(off>n||len>n-off)o_trap(3);int f=(int)o_value(8,fd);ssize_t r=op==1?read(f,(uint8_t*)p+off,(size_t)len):write(f,(const uint8_t*)p+off,(size_t)len);return r<0?(uint64_t)(-(int64_t)errno):(uint64_t)r;}
static int o_valid(unsigned t,uint64_t v){unsigned w=o_width(t);__int128 x=o_value(t,v);__int128 m=((__int128)1)<<(t>=6?w-1:w);return t>=6?x>=-m&&x<m:x>=0&&x<m;}
static int o_buf(const void *p,uint64_t n,uint64_t want,unsigned t){uintptr_t a=(uintptr_t)p;uint64_t z=t==11?4:t==12?8:o_width(t)/8;return p&&n==want&&a%z==0&&n<=UINTPTR_MAX/z&&a<=UINTPTR_MAX-n*z;}
static int o_overlap(const void *a,uint64_t an,const void *b,uint64_t bn){uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x<y+bn&&y<x+an;}
#if !defined(__BYTE_ORDER__) || __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error seeded C profile requires little-endian storage
#endif
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
static float o_sqrt32(float a){float r;
#if defined(__aarch64__)
__asm__("fsqrt %s0, %s1":"=w"(r):"w"(a));
#elif defined(__x86_64__)
__asm__("sqrtss %1, %0":"=x"(r):"x"(a));
#else
#error unsupported seeded float sqrt target
#endif
return r;}
static double o_sqrt64(double a){double r;
#if defined(__aarch64__)
__asm__("fsqrt %d0, %d1":"=w"(r):"w"(a));
#elif defined(__x86_64__)
__asm__("sqrtsd %1, %0":"=x"(r):"x"(a));
#else
#error unsupported seeded float sqrt target
#endif
return r;}
static uint64_t o_fp(unsigned op,unsigned t,uint64_t a,uint64_t b){if(t==11){uint32_t ua=(uint32_t)a,ub=(uint32_t)b,ur;float x,y;volatile float z;__builtin_memcpy(&x,&ua,4);__builtin_memcpy(&y,&ub,4);switch(op){case 1:z=x+y;break;case 2:z=x-y;break;case 3:z=x*y;break;case 4:z=x/y;break;case 6:return x==y;case 7:return x!=y;case 8:return x<y;case 9:return x<=y;case 10:return x>y;case 11:return x>=y;case 17:return ua^UINT32_C(0x80000000);case 19:return ua&UINT32_C(0x7fffffff);case 20:z=o_sqrt32(x);break;default:__builtin_trap();}__builtin_memcpy(&ur,(const void*)&z,4);return o_fpn(11,ua,ub,ur);}uint64_t ur;double x,y;volatile double z;__builtin_memcpy(&x,&a,8);__builtin_memcpy(&y,&b,8);switch(op){case 1:z=x+y;break;case 2:z=x-y;break;case 3:z=x*y;break;case 4:z=x/y;break;case 6:return x==y;case 7:return x!=y;case 8:return x<y;case 9:return x<=y;case 10:return x>y;case 11:return x>=y;case 17:return a^UINT64_C(0x8000000000000000);case 19:return a&UINT64_C(0x7fffffffffffffff);case 20:z=o_sqrt64(x);break;default:__builtin_trap();}__builtin_memcpy(&ur,(const void*)&z,8);return o_fpn(12,a,b,ur);}
static uint64_t o_fcv(unsigned t,unsigned s,uint64_t a){uint32_t u=(uint32_t)a;float f;double x;if(s==t)return a;if(s<11){if(t==11){volatile float z=s>=6?(float)(int64_t)a:(float)a;__builtin_memcpy(&u,(const void*)&z,4);return u;}volatile double z=s>=6?(double)(int64_t)a:(double)a;__builtin_memcpy(&a,(const void*)&z,8);return a;}if(t==12){if(o_fnan(11,a))return (uint64_t)(u>>31)<<63|UINT64_C(0x7ff8000000000000)|(uint64_t)(u&UINT32_C(0x7fffff))<<29;__builtin_memcpy(&f,&u,4);volatile double z=f;__builtin_memcpy(&a,(const void*)&z,8);return a;}if(t==11){if(o_fnan(12,a))return (a>>63)<<31|UINT64_C(0x7fc00000)|(a>>29&UINT64_C(0x3fffff));__builtin_memcpy(&x,&a,8);volatile float z=(float)x;__builtin_memcpy(&u,(const void*)&z,4);return u;}if(s==11){__builtin_memcpy(&f,&u,4);x=f;}else __builtin_memcpy(&x,&a,8);if(x!=x)return 0;unsigned w=o_width(t);if(t>=6){double m=(double)((uint64_t)1<<(w-1));if(x>=m)return ((uint64_t)1<<(w-1))-1;if(x<-m)return (uint64_t)0-((uint64_t)1<<(w-1));return (uint64_t)(int64_t)x;}if(x<1.0)return 0;if(x>=(w==64?18446744073709551616.0:(double)((uint64_t)1<<w)))return w==64?UINT64_MAX:((uint64_t)1<<w)-1;return (uint64_t)x;}
#include <string.h>
static uint64_t o_arg32(float v){uint32_t b;memcpy(&b,&v,4);return b;}
static uint64_t o_arg64(double v){uint64_t b;memcpy(&b,&v,8);return b;}
static uint64_t f0(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f1(uint64_t v0);
static uint64_t f2(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f3(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f4(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f5(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f6(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f7(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f8(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f9(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f10(uint64_t v0, uint64_t v1, void *v2, uint64_t v2_n);
static uint64_t f11(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f12(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f13(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f14(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f15(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f16(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f17(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f18(uint64_t v0, uint64_t v1, void *v2, uint64_t v2_n);
static uint64_t f19(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f20(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f21(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4);
static uint64_t f22(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f23(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f24(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f25(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f26(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f27(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f28(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f29(uint64_t v0);
static uint64_t f30(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f31(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f32(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f33(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n);
static uint64_t f34(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4);
static uint64_t f35(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n);
static uint64_t f36(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f37(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n);
static uint64_t f38(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n, uint64_t v5);
static uint64_t f39(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n, uint64_t v5);
static uint64_t f40(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n);
static uint64_t f41(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f42(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n);
static uint64_t f43(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n);
static uint64_t f44(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n);
static uint64_t f45(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f46(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f0(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t v4=0;
uint64_t c1=0;
for(;o_op(8,5, v4, v1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v4, 5);uint64_t _b=v2;o_op(3, 5, _a,_b);});uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v4;uint64_t _v=({uint64_t _a=v5;uint64_t _b=1000000000ULL;o_op(5, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
v3=({uint64_t _a=v5;uint64_t _b=1000000000ULL;o_op(4, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v4=v1;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c1++>=130)o_trap(4);
{uint64_t _i=v4;uint64_t _v=({uint64_t _a=v3;uint64_t _b=1000000000ULL;o_op(5, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
v3=({uint64_t _a=v3;uint64_t _b=1000000000ULL;o_op(4, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v4);
}
static uint64_t f1(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=1ULL;
uint64_t __attribute__((unused)) v2=({uint64_t _a=v0;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c1++>=20)o_trap(4);
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f2(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=v3;
{
uint64_t v6=0;
uint64_t c1=0;
for(;o_op(8,5, v6, v2);v6=o_op(1,5, v6,1)){
if(c1++>=1000000)o_trap(4);
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
{uint64_t _i=v4;uint64_t _v=o_cast(2, 5, ({uint64_t _a=48ULL;uint64_t _b=({uint64_t _a=v5;uint64_t _b=10ULL;o_op(5, 5, _a,_b);});o_op(1, 5, _a,_b);}));o_put(v0, v0_n, _i,_v,2);}
v5=({uint64_t _a=v5;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
}
}
return o_leave(({uint64_t _a=v1;uint64_t _b=v2;o_op(1, 5, _a,_b);}));
}
static uint64_t f3(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v5=({uint64_t p1=v3;uint64_t p2=({uint64_t p0=v4;f1(p0);});uint64_t p3=v4;f2(v2, v2_n, p1, p2, p3);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
{
uint64_t c1=0;
while(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c1++>=130)o_trap(4);
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
v5=({uint64_t p1=v5;uint64_t p2=9ULL;uint64_t p3=o_get(v0, v0_n, v6, 5);f2(v2, v2_n, p1, p2, p3);});
}
}
return o_leave(v5);
}
static uint64_t f4(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=52ULL;o_op(16, 5, _a,_b);});uint64_t _b=2047ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=4503599627370495ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=1074ULL;
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v3=({uint64_t _a=v3;uint64_t _b=4503599627370496ULL;o_op(13, 5, _a,_b);});
v4=({uint64_t _a=1075ULL;uint64_t _b=v2;o_op(2, 5, _a,_b);});
}
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=1000000000ULL;o_op(5, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1000000000ULL;o_op(4, 5, _a,_b);});uint64_t _b=1000000000ULL;o_op(5, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v5=2ULL;
if(({uint64_t _a=v2;uint64_t _b=1075ULL;o_op(11, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t _a=v2;uint64_t _b=1075ULL;o_op(2, 5, _a,_b);});
v4=0ULL;
{
uint64_t c2=0;
while(({uint64_t _a=v6;uint64_t _b=29ULL;o_op(11, 5, _a,_b);})){
if(c2++>=40)o_trap(4);
v5=({uint64_t p1=v5;uint64_t p2=536870912ULL;f0(v0, v0_n, p1, p2);});
v6=({uint64_t _a=v6;uint64_t _b=29ULL;o_op(2, 5, _a,_b);});
}
}
v5=({uint64_t p1=v5;uint64_t p2=({uint64_t _a=1ULL;uint64_t _b=v6;o_op(15, 5, _a,_b);});f0(v0, v0_n, p1, p2);});
}
{uint64_t _i=129ULL;uint64_t _v=v4;o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=v4;
{
uint64_t c1=0;
while(({uint64_t _a=v6;uint64_t _b=13ULL;o_op(11, 5, _a,_b);})){
if(c1++>=90)o_trap(4);
v5=({uint64_t p1=v5;uint64_t p2=1220703125ULL;f0(v0, v0_n, p1, p2);});
v6=({uint64_t _a=v6;uint64_t _b=13ULL;o_op(2, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v7=1ULL;
{
uint64_t v8=0;
uint64_t c1=0;
for(;o_op(8,5, v8, v6);v8=o_op(1,5, v8,1)){
if(c1++>=1000000)o_trap(4);
v7=({uint64_t _a=v7;uint64_t _b=5ULL;o_op(3, 5, _a,_b);});
}
}
v5=({uint64_t p1=v5;uint64_t p2=v7;f0(v0, v0_n, p1, p2);});
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
if(c1++>=130)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
break;
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
}
return o_leave(v5);
}
static uint64_t f5(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
uint64_t v2[130]={0};
uint64_t v2_n=130;
o_fill(v2, v2_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=({uint64_t p1=v0;f4(v2, v2_n, p1);});
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 129ULL, 5);
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);}), 5);f1(p0);});uint64_t _b=({uint64_t _a=9ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t p1=v3;uint64_t p3=0ULL;f3(v2, v2_n, p1, v1, v1_n, p3);});
return o_leave(({uint64_t _a=v6;uint64_t _b=16ULL;o_op(15, 5, _a,_b);}));
}
if(({uint64_t _a=v5;uint64_t _b=v4;o_op(10, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t p1=v3;uint64_t p3=0ULL;f3(v2, v2_n, p1, v1, v1_n, p3);});
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=v4;o_op(2, 5, _a,_b);});uint64_t _b=16ULL;o_op(15, 5, _a,_b);});uint64_t _b=v4;o_op(13, 5, _a,_b);}));
}
{uint64_t _i=0ULL;uint64_t _v=48ULL;o_put(v1, v1_n, _i,_v,2);}
uint64_t __attribute__((unused)) v6=({uint64_t _a=v4;uint64_t _b=v5;o_op(2, 5, _a,_b);});
{
uint64_t v7=0;
uint64_t c1=0;
for(;o_op(8,5, v7, v6);v7=o_op(1,5, v7,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=48ULL;o_put(v1, v1_n, _i,_v,2);}
}
}
uint64_t __attribute__((unused)) v7=({uint64_t p1=v3;uint64_t p3=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});f3(v2, v2_n, p1, v1, v1_n, p3);});
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v7;uint64_t _b=v4;o_op(2, 5, _a,_b);});uint64_t _b=16ULL;o_op(15, 5, _a,_b);});uint64_t _b=v4;o_op(13, 5, _a,_b);}));
}
static uint64_t f6(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
return o_leave(o_cast(5, 2, o_get(v0, v0_n, v2, 2)));
}
return o_leave(48ULL);
}
static uint64_t f7(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=v1;o_op(11, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v3=o_cast(5, 2, o_get(v0, v0_n, v2, 2));
if(({uint64_t _a=v3;uint64_t _b=53ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=53ULL;o_op(8, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
if(c1++>=1200)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, v4, 2);uint64_t _b=48ULL;o_op(7, 2, _a,_b);})){
return o_leave(1ULL);
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t _a=({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);}), 2));uint64_t _b=48ULL;o_op(2, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);}));
}
static uint64_t f8(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=v1;
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c1++>=1200)o_trap(4);
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, v2, 2);
if(({uint64_t _a=v3;uint64_t _b=57ULL;o_op(6, 2, _a,_b);})){
{uint64_t _i=v2;uint64_t _v=48ULL;o_put(v0, v0_n, _i,_v,2);}
}else{
{uint64_t _i=v2;uint64_t _v=o_cast(2, 2, ({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 2, _a,_b);}));o_put(v0, v0_n, _i,_v,2);}
return o_leave(0ULL);
}
}
}
return o_leave(1ULL);
}
static uint64_t f9(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=v0;
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=52ULL;o_op(16, 5, _a,_b);});uint64_t _b=2047ULL;o_op(12, 5, _a,_b);});uint64_t _b=2047ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v3=0ULL;
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=63ULL;o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=45ULL;o_put(v1, v1_n, _i,_v,2);}
v3=1ULL;
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=4503599627370495ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=v3;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=102ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(({uint64_t _a=v3;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}));
}
{uint64_t _i=v3;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(({uint64_t _a=v3;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}));
}
static uint64_t f10(uint64_t v0, uint64_t v1, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p0=v0;f9(p0, v2, v2_n);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v3);
}
uint8_t v4[1200]={0};
uint64_t v4_n=1200;
o_fill(v4, v4_n, 0ULL, 2);
v3=({uint64_t p0=v0;f5(p0, v4, v4_n);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=v3;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v5;uint64_t _b=v1;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=0ULL;
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=63ULL;o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=45ULL;o_put(v2, v2_n, _i,_v,2);}
v8=1ULL;
}
if(({uint64_t _a=({uint64_t p1=v6;uint64_t p2=v7;f7(v4, v4_n, p1, p2);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t p1=v7;f8(v4, v4_n, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=v8;uint64_t _v=49ULL;o_put(v2, v2_n, _i,_v,2);}
v8=({uint64_t _a=v8;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
{
uint64_t v9=0;
uint64_t c1=0;
for(;o_op(8,5, v9, v5);v9=o_op(1,5, v9,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=v8;uint64_t _v=o_get(v4, v4_n, v9, 2);o_put(v2, v2_n, _i,_v,2);}
v8=({uint64_t _a=v8;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(v8);
}
{uint64_t _i=v8;uint64_t _v=46ULL;o_put(v2, v2_n, _i,_v,2);}
v8=({uint64_t _a=v8;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
{
uint64_t v9=0;
uint64_t c1=0;
for(;o_op(8,5, v9, v1);v9=o_op(1,5, v9,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=v8;uint64_t _v=o_cast(2, 5, ({uint64_t p1=v6;uint64_t p2=({uint64_t _a=v5;uint64_t _b=v9;o_op(1, 5, _a,_b);});f6(v4, v4_n, p1, p2);}));o_put(v2, v2_n, _i,_v,2);}
v8=({uint64_t _a=v8;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v8);
}
static uint64_t f11(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
if(c1++>=1200)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, v2, 2);uint64_t _b=48ULL;o_op(7, 2, _a,_b);})){
return o_leave(v2);
}
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f12(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
{uint64_t _i=v1;uint64_t _v=101ULL;o_put(v0, v0_n, _i,_v,2);}
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=v4;uint64_t _v=45ULL;o_put(v0, v0_n, _i,_v,2);}
}else{
{uint64_t _i=v4;uint64_t _v=43ULL;o_put(v0, v0_n, _i,_v,2);}
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
{uint64_t _i=v4;uint64_t _v=48ULL;o_put(v0, v0_n, _i,_v,2);}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
uint64_t __attribute__((unused)) v5=({uint64_t p0=v3;f1(p0);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v4;uint64_t _b=v5;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=v3;
{
uint64_t v8=0;
uint64_t c1=0;
for(;o_op(8,5, v8, v5);v8=o_op(1,5, v8,1)){
if(c1++>=1000000)o_trap(4);
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
{uint64_t _i=v6;uint64_t _v=o_cast(2, 5, ({uint64_t _a=48ULL;uint64_t _b=({uint64_t _a=v7;uint64_t _b=10ULL;o_op(5, 5, _a,_b);});o_op(1, 5, _a,_b);}));o_put(v0, v0_n, _i,_v,2);}
v7=({uint64_t _a=v7;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
}
}
return o_leave(({uint64_t _a=v4;uint64_t _b=v5;o_op(1, 5, _a,_b);}));
}
static uint64_t f13(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(49ULL);
}
return o_leave(48ULL);
}
return o_leave(({uint64_t p1=v1;uint64_t p2=({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);});f6(v0, v0_n, p1, p2);}));
}
static uint64_t f14(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v4);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=v5;uint64_t _v=o_cast(2, 5, ({uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=v5;f13(v0, v0_n, p1, p2, p3, p4);}));o_put(v0, v0_n, _i,_v,2);}
}
}
return o_leave(v4);
}
static uint64_t f15(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
{uint64_t _i=v1;uint64_t _v=o_get(v2, v2_n, 0ULL, 2);o_put(v0, v0_n, _i,_v,2);}
uint64_t __attribute__((unused)) v5=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
{uint64_t _i=v5;uint64_t _v=46ULL;o_put(v0, v0_n, _i,_v,2);}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
{
uint64_t v6=0;
uint64_t k2=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t c2=0;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
if(c2++>=1000000)o_trap(4);
{uint64_t _i=v5;uint64_t _v=o_get(v2, v2_n, ({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);o_put(v0, v0_n, _i,_v,2);}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
}
return o_leave(({uint64_t p1=v5;uint64_t p2=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t p3=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});f12(v0, v0_n, p1, p2, p3);}));
}
static uint64_t f16(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=v1;
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=v6;uint64_t _v=48ULL;o_put(v0, v0_n, _i,_v,2);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=46ULL;o_put(v0, v0_n, _i,_v,2);}
v6=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});
{
uint64_t v7=0;
uint64_t k2=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t c2=0;
for(;o_op(8,5, v7, k2);v7=o_op(1,5, v7,1)){
if(c2++>=1000000)o_trap(4);
{uint64_t _i=v6;uint64_t _v=48ULL;o_put(v0, v0_n, _i,_v,2);}
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
{
uint64_t v7=0;
uint64_t c2=0;
for(;o_op(8,5, v7, v3);v7=o_op(1,5, v7,1)){
if(c2++>=1000000)o_trap(4);
{uint64_t _i=v6;uint64_t _v=o_get(v2, v2_n, v7, 2);o_put(v0, v0_n, _i,_v,2);}
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v6);
}
{
uint64_t v7=0;
uint64_t k1=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v7, k1);v7=o_op(1,5, v7,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=v6;uint64_t _v=o_get(v2, v2_n, v7, 2);o_put(v0, v0_n, _i,_v,2);}
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v3;uint64_t _b=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_op(10, 5, _a,_b);})){
{uint64_t _i=v6;uint64_t _v=46ULL;o_put(v0, v0_n, _i,_v,2);}
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
{
uint64_t v7=0;
uint64_t k2=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v5;o_op(2, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t c2=0;
for(;o_op(8,5, v7, k2);v7=o_op(1,5, v7,1)){
if(c2++>=1000000)o_trap(4);
{uint64_t _i=v6;uint64_t _v=o_get(v2, v2_n, ({uint64_t _a=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=v7;o_op(1, 5, _a,_b);}), 2);o_put(v0, v0_n, _i,_v,2);}
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
}
return o_leave(v6);
}
static uint64_t f17(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=0ULL;
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=4ULL;o_op(10, 5, _a,_b);})){
v6=1ULL;
}
}else{
if(({uint64_t _a=v5;uint64_t _b=v3;o_op(11, 5, _a,_b);})){
v6=1ULL;
}
}
uint64_t __attribute__((unused)) v7=v3;
{
uint64_t c1=0;
while(({uint64_t _a=v7;uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
if(c1++>=1200)o_trap(4);
if(({uint64_t _a=o_get(v2, v2_n, ({uint64_t _a=v7;uint64_t _b=1ULL;o_op(2, 5, _a,_b);}), 2);uint64_t _b=48ULL;o_op(7, 2, _a,_b);})){
break;
}
v7=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
}
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=v7;uint64_t p4=v4;f15(v0, v0_n, p1, v2, v2_n, p3, p4);}));
}
return o_leave(({uint64_t p1=v1;uint64_t p3=v7;uint64_t p4=v4;f16(v0, v0_n, p1, v2, v2_n, p3, p4);}));
}
static uint64_t f18(uint64_t v0, uint64_t v1, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p0=v0;f9(p0, v2, v2_n);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v3);
}
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=1ULL;
}
uint8_t v4[1200]={0};
uint64_t v4_n=1200;
o_fill(v4, v4_n, 0ULL, 2);
v3=({uint64_t p0=v0;f5(p0, v4, v4_n);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=v3;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=0ULL;
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=63ULL;o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=45ULL;o_put(v2, v2_n, _i,_v,2);}
v7=1ULL;
}
uint64_t __attribute__((unused)) v8=({uint64_t p1=v6;f11(v4, v4_n, p1);});
if(({uint64_t _a=v8;uint64_t _b=v6;o_op(6, 5, _a,_b);})){
{uint64_t _i=v7;uint64_t _v=48ULL;o_put(v2, v2_n, _i,_v,2);}
return o_leave(({uint64_t _a=v7;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));
}
uint64_t __attribute__((unused)) v9=({uint64_t _a=v8;uint64_t _b=v1;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v10=0ULL;
if(({uint64_t _a=({uint64_t p1=v6;uint64_t p2=v9;f7(v4, v4_n, p1, p2);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v10=({uint64_t p1=v9;f8(v4, v4_n, p1);});
}
uint64_t __attribute__((unused)) v11=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
if(({uint64_t _a=v10;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v8=({uint64_t p1=v9;f11(v4, v4_n, p1);});
if(({uint64_t _a=v8;uint64_t _b=v5;o_op(8, 5, _a,_b);})){
v11=({uint64_t _a=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=v8;o_op(2, 5, _a,_b);});uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
}else{
v11=({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v8;uint64_t _b=v5;o_op(2, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=1ULL;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(13, 5, _a,_b);});
}
}
v3=({uint64_t p1=v6;uint64_t p2=v8;uint64_t p3=v10;uint64_t p4=v1;f14(v4, v4_n, p1, p2, p3, p4);});
return o_leave(({uint64_t p1=v7;uint64_t p3=v3;uint64_t p4=v11;f17(v2, v2_n, p1, v4, v4_n, p3, p4);}));
}
static uint64_t f19(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=9)o_trap(4);
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=o_cast(2, 5, ({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);}));o_put(v0, v0_n, _i,_v,2);}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(({uint64_t _a=v1;uint64_t _b=v3;o_op(1, 5, _a,_b);}));
}
static uint64_t f20(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v4);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=v5;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v2, v2_n, ({uint64_t _a=v3;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);o_put(v0, v0_n, _i,_v,2);}
}
}
return o_leave(({uint64_t _a=v1;uint64_t _b=v4;o_op(1, 5, _a,_b);}));
}
static uint64_t f21(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4){
o_enter();
return o_leave(({uint64_t p1=v1;uint64_t p3=o_get(v2, v2_n, v4, 5);uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);f20(v0, v0_n, p1, v3, v3_n, p3, p4);}));
}
static uint64_t f22(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p0=v2;f1(p0);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=v3;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=v2;
{
uint64_t v6=0;
uint64_t c1=0;
for(;o_op(8,5, v6, v3);v6=o_op(1,5, v6,1)){
if(c1++>=1000000)o_trap(4);
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
{uint64_t _i=v4;uint64_t _v=o_cast(2, 5, ({uint64_t _a=48ULL;uint64_t _b=({uint64_t _a=v5;uint64_t _b=10ULL;o_op(5, 5, _a,_b);});o_op(1, 5, _a,_b);}));o_put(v0, v0_n, _i,_v,2);}
v5=({uint64_t _a=v5;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
}
}
return o_leave(({uint64_t _a=v1;uint64_t _b=v3;o_op(1, 5, _a,_b);}));
}
static uint64_t f23(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
v1=({uint64_t p1=v1;uint64_t p2=2314934085979873312ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p2=2314971646008959008ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p2=2314885532081922080ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
v1=({uint64_t p1=v1;uint64_t p2=2314972694198165536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
static uint64_t f24(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=v3;
uint64_t __attribute__((unused)) v6=({uint64_t _a=v3;uint64_t _b=v4;o_op(1, 5, _a,_b);});
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v6;o_op(8, 5, _a,_b);})){
if(c1++>=262145)o_trap(4);
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
uint64_t __attribute__((unused)) v7=v5;
{
uint64_t c2=0;
while(({uint64_t _a=v7;uint64_t _b=v6;o_op(8, 5, _a,_b);})){
if(c2++>=262145)o_trap(4);
if(({uint64_t _a=o_get(v2, v2_n, v7, 2);uint64_t _b=10ULL;o_op(6, 2, _a,_b);})){
break;
}
v7=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
v1=({uint64_t p1=v1;uint64_t p3=v5;uint64_t p4=({uint64_t _a=v7;uint64_t _b=v5;o_op(2, 5, _a,_b);});f20(v0, v0_n, p1, v2, v2_n, p3, p4);});
{uint64_t _i=v1;uint64_t _v=10ULL;o_put(v0, v0_n, _i,_v,2);}
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v5=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f25(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _b=v2;o_op(10, 5, _a,_b);})){
return o_leave(0ULL);
}
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v4);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2));uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
}
}
return o_leave(1ULL);
}
static uint64_t f26(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
{
uint64_t v3=0;
uint64_t c1=0;
for(;o_op(8,5, v3, v2);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=v4;uint64_t _b=37ULL;o_op(6, 2, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v4;uint64_t _b=10ULL;o_op(6, 2, _a,_b);})){
return o_leave(1ULL);
}
}
}
return o_leave(0ULL);
}
static uint64_t f27(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=v1;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=262145)o_trap(4);
if(({uint64_t _a=({uint64_t p1=v3;uint64_t p2=v2;uint64_t p3=111516500389709ULL;uint64_t p4=6ULL;f25(v0, v0_n, p1, p2, p3, p4);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v3);
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f28(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=v1;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=262145)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t p1=v3;uint64_t p2=v2;uint64_t p3=8243077295634739279ULL;uint64_t p4=8ULL;f25(v0, v0_n, p1, p2, p3, p4);});
uint64_t __attribute__((unused)) v5=({uint64_t p1=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(1, 5, _a,_b);});uint64_t p2=v2;uint64_t p3=8459008377155773806ULL;uint64_t p4=8ULL;f25(v0, v0_n, p1, p2, p3, p4);});
uint64_t __attribute__((unused)) v6=({uint64_t p1=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(1, 5, _a,_b);});uint64_t p2=v2;uint64_t p3=25964ULL;uint64_t p4=2ULL;f25(v0, v0_n, p1, p2, p3, p4);});
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=v5;o_op(12, 5, _a,_b);});uint64_t _b=v6;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0ULL);
}
static uint64_t f29(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=48ULL;o_op(11, 5, _a,_b);})){
if(({uint64_t _a=v0;uint64_t _b=57ULL;o_op(9, 5, _a,_b);})){
return o_leave(1ULL);
}
}
return o_leave(0ULL);
}
static uint64_t f30(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t p1=v1;uint64_t p2=v3;f27(v0, v0_n, p1, p2);});
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=262145)o_trap(4);
if(({uint64_t _a=({uint64_t p0=o_cast(5, 2, o_get(v0, v0_n, v4, 2));f29(p0);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v5=0ULL;
uint64_t __attribute__((unused)) v6=0ULL;
{
uint64_t c3=0;
while(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=v5;o_op(1, 5, _a,_b);});uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c3++>=262145)o_trap(4);
uint64_t __attribute__((unused)) v7=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2));
if(({uint64_t _a=v7;uint64_t _b=46ULL;o_op(6, 5, _a,_b);})){
v6=1ULL;
}else{
if(({uint64_t _a=({uint64_t p0=v7;f29(p0);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v5;uint64_t _b=2ULL;o_op(10, 5, _a,_b);})){
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=64ULL;o_op(8, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});uint64_t _b=v5;o_op(13, 5, _a,_b);}));
}
}
}
v4=({uint64_t _a=v4;uint64_t _b=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0ULL);
}
static uint64_t f31(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=v4;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v2);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);uint64_t _b=o_get(v0, v0_n, ({uint64_t _a=v3;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
}
}
return o_leave(1ULL);
}
static uint64_t f32(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
return o_leave(({uint64_t p1=v1;uint64_t p4=2ULL;f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);}));
}
static uint64_t f33(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n){
o_enter();
v1=({uint64_t p1=v1;uint64_t p3=0ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7719241544247105102ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029109360601756261ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029390620829971812ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7234298746084532340ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=10272ULL;uint64_t p3=2ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p4=0ULL;f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
v1=({uint64_t p1=v1;uint64_t p2=8242765322112888879ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7599382571296061033ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8318823019982186852ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8315174762841010025ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7599935561254530592ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=170488947ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8439793166470894915ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=5629622052994639219ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8319591351411034454ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7305790156121666149ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8242000134883210016ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7575182620743445605ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338330744771059828ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=5279425298211827572ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7958535002297411393ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7815263158106352741ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=667237ULL;uint64_t p3=3ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4211825973867343684ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029109373470339872ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7935454016877785188ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2748920225308371318ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029109373470339872ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7935454016877785188ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8535778565153909110ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2669ULL;uint64_t p3=2ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8246201347450365513ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7236837538582061679ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7237123112283545715ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8317701148704599157ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=5284497113930817908ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8101729933614729540ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7308905025183444581ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7308345369542074476ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=32ULL;uint64_t p3=1ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;f32(v0, v0_n, p1, v2, v2_n, v3, v3_n);});
v1=({uint64_t p1=v1;uint64_t p2=8101729874579040288ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7722958061638086245ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029123654252917349ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7309940825170670948ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7381153934383080812ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8461736434098271264ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8367736664178909805ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2336934389682693480ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7165055622473015661ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7593464113577226100ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=667246ULL;uint64_t p3=3ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
static uint64_t f34(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4){
o_enter();
v1=({uint64_t p1=v1;uint64_t p2=7719241544247105102ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029109360601756261ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7311066480264377700ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=35658687673202ULL;uint64_t p3=6ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p3=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t p4=({uint64_t _a=v4;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});f20(v0, v0_n, p1, v3, v3_n, p3, p4);});
v1=({uint64_t p1=v1;uint64_t p2=7233184986725312800ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4702977657673573477ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7737577168441453355ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=35619916226675ULL;uint64_t p3=6ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;f32(v0, v0_n, p1, v2, v2_n, v3, v3_n);});
v1=({uint64_t p1=v1;uint64_t p2=7018422849374152992ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=681315ULL;uint64_t p3=3ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4923915573298554967ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7811904092957780801ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7214893546864669540ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7017488320073984370ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7885647255505693804ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=139140297760ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;f32(v0, v0_n, p1, v2, v2_n, v3, v3_n);});
v1=({uint64_t p1=v1;uint64_t p2=8029123662842982176ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7018130082676634980ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=44886549860ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8097301097592943656ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7599382571414483553ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7954887939624888676ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=45ULL;uint64_t p3=1ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;f32(v0, v0_n, p1, v2, v2_n, v3, v3_n);});
v1=({uint64_t p1=v1;uint64_t p2=7791338854113357353ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7381153998842067297ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7142820555474367520ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7953747407939595361ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334385705639764768ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2336916743122085218ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8511914812230627181ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3203025184780153445ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=10ULL;uint64_t p3=1ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8248096844079722081ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8032558527420001903ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8317138478892610677ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7021802806838192416ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8320788950816221038ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7307218078066221100ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338053702233842720ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338324178100708722ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334097595258072681ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8319104491599193711ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=43728203369ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338328219631577172ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8749489599637973108ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7957695011148363296ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2317993992531559200ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8462115405134787425ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7166460028376806432ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8031079698469501544ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7935391421923290223ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8603402928459751535ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7811904092961993327ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8675375920160189540ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2336927755350730100ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2865758617564514ULL;uint64_t p3=7ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4211825973867343684ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7812726606361684256ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=139140297760ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;f32(v0, v0_n, p1, v2, v2_n, v3, v3_n);});
v1=({uint64_t p1=v1;uint64_t p2=7307970478859120416ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7237123112232644210ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338053340533189749ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7307218078150061936ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338615488351593248ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338060278192697204ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=751660686516577654ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4776439401847669544ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7236491060045239381ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338619928923480108ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8243124888234369121ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7957695011148363296ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2340025105102420514ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2339450055106586740ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3203029539791922536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7236274654160450080ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7020303052929376300ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338620986089301355ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7308041941661544033ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7434721071324293152ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=43724141936ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
static uint64_t f35(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n){
o_enter();
if(({uint64_t _a=o_get(v2, v2_n, 13ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;f33(v0, v0_n, p1, v2, v2_n, v3, v3_n, v4, v4_n);}));
}
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 14ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 15ULL, 5);
uint64_t __attribute__((unused)) v7=({uint64_t p1=v5;uint64_t p2=v6;f30(v3, v3_n, p1, p2);});
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=0ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7719241544247105102ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029109360601756261ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8028827670876550500ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8458358425190229089ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7311066725328953460ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8026294619592029042ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338616625292668021ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334097595219076450ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=35435316212326ULL;uint64_t p3=6ULL;f19(v0, v0_n, p1, p2, p3);});
uint64_t __attribute__((unused)) v8=v6;
if(({uint64_t _a=v8;uint64_t _b=120ULL;o_op(10, 5, _a,_b);})){
v8=120ULL;
}
v1=({uint64_t p1=v1;uint64_t p3=v5;uint64_t p4=v8;f20(v0, v0_n, p1, v3, v3_n, p3, p4);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453514ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8316293034885989408ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8390891584273653818ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7526756837609466144ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8391735665605768224ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=6926648854367729008ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8030604433265287523ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8243124888234372963ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3414125872510561839ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=6948613773778380150ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7307199665625391392ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=667236ULL;uint64_t p3=3ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
if(({uint64_t _a=({uint64_t p1=({uint64_t _a=v7;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t p2=({uint64_t _a=v7;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t p3=o_get(v2, v2_n, 2ULL, 5);uint64_t p4=o_get(v2, v2_n, 3ULL, 5);f31(v3, v3_n, p1, p2, p3, p4);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=0ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
return o_leave(({uint64_t p1=v1;uint64_t p4=v7;f34(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);}));
}
if(({uint64_t _a=({uint64_t p1=v5;uint64_t p2=({uint64_t _a=v5;uint64_t _b=v6;o_op(1, 5, _a,_b);});f28(v3, v3_n, p1, p2);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=1ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7719241544247105102ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029109360601756261ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7526676312424740196ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7234315298837569637ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334391233313467680ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p3=({uint64_t _a=v7;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t p4=({uint64_t _a=v7;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});f20(v0, v0_n, p1, v3, v3_n, p3, p4);});
v1=({uint64_t p1=v1;uint64_t p2=7233184986725312800ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4702977657673573477ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8151642187988216619ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7234304265016861045ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334386829831073568ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7237123112266199151ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8367800735125302389ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8318823019731771752ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=175009641ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338324178096125001ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7308604917384833140ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=6998720730445734688ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7815275222770148462ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2318908997382905956ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7526774392225818185ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7811882119909043817ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7166468868806814835ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7307218078066286696ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029109369191821088ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7236828442999420260ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2337213314941154080ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338060278192696937ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=733651824152700274ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
v1=({uint64_t p1=v1;uint64_t p3=2ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7719241544247105102ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029109360601756261ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8101729689844610404ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7815263158106353253ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8293ULL;uint64_t p3=2ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p3=({uint64_t _a=v7;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t p4=({uint64_t _a=v7;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});f20(v0, v0_n, p1, v3, v3_n, p3, p4);});
v1=({uint64_t p1=v1;uint64_t p2=3110651729373505568ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338330697593287456ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;f32(v0, v0_n, p1, v2, v2_n, v3, v3_n);});
v1=({uint64_t p1=v1;uint64_t p2=2601ULL;uint64_t p3=2ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
static uint64_t f36(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p2=7599382571363165231ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=119247139334500ULL;uint64_t p3=6ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p2=7599382571363165231ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=811690340ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
v1=({uint64_t p1=v1;uint64_t p2=7599382571363165231ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=30811018106071396ULL;uint64_t p3=7ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
static uint64_t f37(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n){
o_enter();
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t v5=0;
uint64_t k1=3ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, ({uint64_t _a=16ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=v6;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=0ULL;f23(v0, v0_n, p1, v3, v3_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7358993268768925028ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=35435416218729ULL;uint64_t p3=6ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=v5;f36(v0, v0_n, p1, p2);});
v1=({uint64_t p1=v1;uint64_t p2=8319108854100224288ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=174550633ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
if(({uint64_t _a=v5;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4211825973867343684ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029109373470339872ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7935454016877785188ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8535778565153909110ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7310014169164554349ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7306016311923274849ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7019251898354904950ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=6143789756275127597ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8097873589919498564ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7308533321083330675ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8243966800727011181ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2987128040764962159ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=10ULL;uint64_t p3=1ULL;f19(v0, v0_n, p1, p2, p3);});
}else{
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4211825973867343684ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029109373470339872ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7935454016877785188ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885811220539766ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2336934389682958120ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7867051229370218094ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334380212662330479ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7527520677224738415ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7018141364397826921ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334386829831398772ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7358993268768925028ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=11173145963625ULL;uint64_t p3=6ULL;f19(v0, v0_n, p1, p2, p3);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
if(({uint64_t _a=v6;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=0ULL;f23(v0, v0_n, p1, v3, v3_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7358993268768925028ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=35435416218729ULL;uint64_t p3=6ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=v5;f36(v0, v0_n, p1, p2);});
v1=({uint64_t p1=v1;uint64_t p2=2338340670574978336ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8316293034886329698ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7017488320073790752ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7309464380405411438ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8245921732316700782ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7453010313196433952ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7598266502335062304ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=44684962164ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4211825973867343684ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7307218077999128864ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7308895142333738272ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8243116152216122151ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334100875203540000ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8367810652205838201ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8103505641670206824ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8606133097150837792ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7793514839516672878ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2336927441696399475ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7308332182666569844ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7575181530044592928ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=170666356ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8367816086726406767ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7815263158106350952ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7957695015410409573ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4926770202774228512ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7811852429924333157ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3904688382616227173ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=170800694ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=2ULL;f23(v0, v0_n, p1, v3, v3_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7358993268768925028ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7219024163114413161ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7594310829029160549ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7306016311806813025ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7019251898354904950ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7939694845922058288ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8535778565153909110ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7305437204000743533ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334102031707434784ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338328528881219938ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=44868924277ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
}
return o_leave(v1);
}
static uint64_t f38(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, ({uint64_t _a=v5;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, ({uint64_t _a=v5;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v8=({uint64_t _a=v6;uint64_t _b=v7;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _b=o_get(v2, v2_n, ({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v10=v6;
{
uint64_t c1=0;
while(({uint64_t _a=v10;uint64_t _b=v8;o_op(8, 5, _a,_b);})){
if(c1++>=262145)o_trap(4);
if(({uint64_t _a=o_get(v3, v3_n, v10, 2);uint64_t _b=10ULL;o_op(6, 2, _a,_b);})){
break;
}
v10=({uint64_t _a=v10;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v10;uint64_t _b=v8;o_op(11, 5, _a,_b);})){
{uint64_t _i=2ULL;uint64_t _v=2ULL;o_put(v4, v4_n, _i,_v,5);}
return o_leave(v1);
}
if(({uint64_t _a=v9;uint64_t _b=v8;o_op(10, 5, _a,_b);})){
{uint64_t _i=2ULL;uint64_t _v=2ULL;o_put(v4, v4_n, _i,_v,5);}
return o_leave(v1);
}
v1=({uint64_t p1=v1;uint64_t p3=0ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=542462023ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p4=v5;f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
v1=({uint64_t p1=v1;uint64_t p2=8250ULL;uint64_t p3=2ULL;f19(v0, v0_n, p1, p2, p3);});
if(({uint64_t _a=v10;uint64_t _b=v9;o_op(11, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=v9;uint64_t p4=({uint64_t _a=v10;uint64_t _b=v9;o_op(2, 5, _a,_b);});f20(v0, v0_n, p1, v3, v3_n, p3, p4);});
}else{
v1=({uint64_t p1=v1;uint64_t p3=v9;uint64_t p4=({uint64_t _a=v8;uint64_t _b=v9;o_op(2, 5, _a,_b);});f20(v0, v0_n, p1, v3, v3_n, p3, p4);});
}
{uint64_t _i=v1;uint64_t _v=10ULL;o_put(v0, v0_n, _i,_v,2);}
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
return o_leave(({uint64_t p1=v1;uint64_t p3=({uint64_t _a=v10;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p4=({uint64_t _a=v8;uint64_t _b=({uint64_t _a=v10;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_op(2, 5, _a,_b);});f24(v0, v0_n, p1, v3, v3_n, p3, p4);}));
}
static uint64_t f39(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n, uint64_t v5){
o_enter();
v1=({uint64_t p1=v1;uint64_t p3=2ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=542462023ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p4=v5;f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
v1=({uint64_t p1=v1;uint64_t p2=8250ULL;uint64_t p3=2ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p4=({uint64_t _a=v5;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
v1=({uint64_t p1=v1;uint64_t p2=10272ULL;uint64_t p3=2ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p4=({uint64_t _a=v5;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
v1=({uint64_t p1=v1;uint64_t p2=8030041785346567209ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8241996475536864370ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8316292708468811111ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2336349386297208692ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4213524689155093875ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=32ULL;uint64_t p3=1ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p4=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(1, 5, _a,_b);});f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
v1=({uint64_t p1=v1;uint64_t p2=2601ULL;uint64_t p3=2ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2321096865283006834ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7305804402280461925ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7811882189719500064ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029390625124934241ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7954890152741118068ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8388356097741892980ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7594873735819391264ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2936174556766563ULL;uint64_t p3=7ULL;f19(v0, v0_n, p1, p2, p3);});
if(({uint64_t _a=o_get(v2, v2_n, ({uint64_t _a=v5;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=1ULL;uint64_t _v=1ULL;o_put(v4, v4_n, _i,_v,5);}
}
return o_leave(v1);
}
static uint64_t f40(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n){
o_enter();
if(({uint64_t _a=o_get(v2, v2_n, 20ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=0ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7953747406981779527ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7233174017378905966ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=32ULL;uint64_t p3=1ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p4=0ULL;f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
v1=({uint64_t p1=v1;uint64_t p2=8319663842577183535ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8531335223917637679ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=44885697385ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4211825973867343684ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7737585620858794528ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8245937413158084709ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334386829831073568ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334393380863435117ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2318340523933529193ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=6998715218728873838ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7955997369423586080ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8027794332106912357ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3347152483685266549ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=10ULL;uint64_t p3=1ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 19ULL, 5);
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=0ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2337207816325255239ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=5125168158881699406ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7236850772546180432ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8320808432788465952ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7594036745779634735ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8315161591618561071ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=10ULL;uint64_t p3=1ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2314885530818453536ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4211825973867343684ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8367806182301655840ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7575148376516224360ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7234316338103263347ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8606220697354527008ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8317066731434898021ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8243593360416793456ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8533793980014424165ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2865505029940329ULL;uint64_t p3=7ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
if(({uint64_t _a=v5;uint64_t _b=16ULL;o_op(10, 5, _a,_b);})){
v5=16ULL;
}
{
uint64_t v6=0;
uint64_t c1=0;
for(;o_op(8,5, v6, v5);v6=o_op(1,5, v6,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v7=({uint64_t _a=32ULL;uint64_t _b=({uint64_t _a=v6;uint64_t _b=12ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=o_get(v2, v2_n, ({uint64_t _a=v7;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p5=v7;f39(v0, v0_n, p1, v2, v2_n, v3, v3_n, v4, v4_n, p5);});
}else{
v1=({uint64_t p1=v1;uint64_t p5=v7;f38(v0, v0_n, p1, v2, v2_n, v3, v3_n, v4, v4_n, p5);});
}
}
}
return o_leave(v1);
}
static uint64_t f41(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint8_t v3[1600]={0};
uint64_t v3_n=1600;
o_fill(v3, v3_n, 0ULL, 2);
uint64_t __attribute__((unused)) v4=({uint64_t p0=({uint64_t _a=o_fcv(12, 5, v2);uint64_t _b=4697254411347427328ULL;o_fp(4, 12, _a,_b);});uint64_t p1=1ULL;f10(p0, p1, v3, v3_n);});
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v4);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=v5;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v3, v3_n, v5, 2);o_put(v0, v0_n, _i,_v,2);}
}
}
return o_leave(({uint64_t _a=v1;uint64_t _b=v4;o_op(1, 5, _a,_b);}));
}
static uint64_t f42(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n){
o_enter();
if(({uint64_t _a=o_get(v2, v2_n, 21ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=3ULL;f23(v0, v0_n, p1, v3, v3_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2322302090265585005ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8223700941521445219ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8030604433264238949ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7380952757880827747ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2671ULL;uint64_t p3=2ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
if(({uint64_t _a=o_get(v3, v3_n, 1ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=2ULL;f23(v0, v0_n, p1, v3, v3_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2322302090265585005ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=o_get(v2, v2_n, 22ULL, 5);f41(v0, v0_n, p1, p2);});
v1=({uint64_t p1=v1;uint64_t p2=7018986601647261472ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8367821570011587954ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=6998687624145692008ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=5125207875246908526ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=539776336ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=o_get(v2, v2_n, 23ULL, 5);f41(v0, v0_n, p1, p2);});
v1=({uint64_t p1=v1;uint64_t p2=7022907460111910688ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7935454059608239209ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=685935ULL;uint64_t p3=3ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
v1=({uint64_t p1=v1;uint64_t p3=3ULL;f23(v0, v0_n, p1, v3, v3_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2322302090265585005ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=o_get(v2, v2_n, 22ULL, 5);f41(v0, v0_n, p1, p2);});
v1=({uint64_t p1=v1;uint64_t p2=8320808368934045472ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029185230041343348ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=539785586ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=o_get(v2, v2_n, 23ULL, 5);f41(v0, v0_n, p1, p2);});
v1=({uint64_t p1=v1;uint64_t p2=7022907460111910688ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7935454059608239209ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=685935ULL;uint64_t p3=3ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
static uint64_t f43(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 24ULL, 5);
v1=({uint64_t p1=v1;uint64_t p3=3ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p2=8029764342507656515ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029390621081430892ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7234298763096760436ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8390045716217487648ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8746589041877808416ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2317993992531559200ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7955925836743274024ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2982620369575310451ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=10ULL;uint64_t p3=1ULL;f19(v0, v0_n, p1, p2, p3);});
}else{
v1=({uint64_t p1=v1;uint64_t p2=8029764342507656515ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8029390621081430892ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7234298763096760436ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8390045716217487648ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8746589041877808416ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2317993992531559200ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338328219397025576ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7308335461136821865ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=139391475812ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p2=7165064483209180463ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3414120447861550177ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7161698048063465826ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
}else{
v1=({uint64_t p1=v1;uint64_t p2=7955998172649846063ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=426870468143ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
}
v1=({uint64_t p1=v1;uint64_t p2=7575182534950264891ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=732227939313524851ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
}
v1=({uint64_t p1=v1;uint64_t p3=3ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8243113893085146979ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7214893499608735802ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7023117819775837039ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338613306708861043ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2128226ULL;uint64_t p3=3ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p4=4ULL;f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
v1=({uint64_t p1=v1;uint64_t p2=3110651729373503547ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8007527012499811872ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7308041941662919790ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4836972791547715872ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7308332226908742432ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2674ULL;uint64_t p3=2ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
static uint64_t f44(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, void *v4, uint64_t v4_n){
o_enter();
v1=({uint64_t p1=v1;uint64_t p2=7214890231020156259ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7575118836407755631ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7863411845950087283ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8223684444467782497ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8245921732400275813ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4551779407707325216ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=10ULL;uint64_t p3=1ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8391173050692888900ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4213518147948405609ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338603393874486304ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2340027244252129644ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338319731383432053ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7311146985109614446ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8028074745930329458ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=5279151288497414254ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7596838514759846189ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8459766083726439283ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=43728007777ULL;uint64_t p3=5ULL;f19(v0, v0_n, p1, p2, p3);});
if(({uint64_t _a=o_get(v2, v2_n, 6ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p3=3ULL;f23(v0, v0_n, p1, v4, v4_n, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2322288840375761267ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p4=7ULL;f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
{uint64_t _i=v1;uint64_t _v=32ULL;o_put(v0, v0_n, _i,_v,2);}
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v1=({uint64_t p1=v1;uint64_t p4=9ULL;f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
{uint64_t _i=v1;uint64_t _v=32ULL;o_put(v0, v0_n, _i,_v,2);}
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v1=({uint64_t p1=v1;uint64_t p4=11ULL;f21(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
{uint64_t _i=v1;uint64_t _v=10ULL;o_put(v0, v0_n, _i,_v,2);}
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
return o_leave(v1);
}
static uint64_t f45(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p2=5629550495145215314ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7737585619986315365ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3273954666935230565ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=175468647ULL;uint64_t p3=4ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
v1=({uint64_t p1=v1;uint64_t p2=4918288487097323342ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2112089ULL;uint64_t p3=3ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=v2;f22(v0, v0_n, p1, p2);});
v1=({uint64_t p1=v1;uint64_t p2=7882825892814286880ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=v1;uint64_t _v=115ULL;o_put(v0, v0_n, _i,_v,2);}
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
v1=({uint64_t p1=v1;uint64_t p2=2318902417491648800ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334386829831727430ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4703482545360234854ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334393380926147657ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2336934389682040417ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7165055622473015661ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7593464113577226100ULL;uint64_t p3=8ULL;f19(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=667246ULL;uint64_t p3=3ULL;f19(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
static uint64_t f46(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=o_get(v0, v0_n, 2ULL, 5);uint64_t p2=o_get(v0, v0_n, 3ULL, 5);f26(v1, v1_n, p1, p2);});uint64_t _b=({uint64_t p1=o_get(v0, v0_n, 4ULL, 5);uint64_t p2=o_get(v0, v0_n, 5ULL, 5);f26(v1, v1_n, p1, p2);});o_op(13, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=63ULL;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(13, 5, _a,_b);}));
}
uint64_t v3[4]={0};
uint64_t v3_n=4;
o_fill(v3, v3_n, 0ULL, 5);
uint64_t __attribute__((unused)) v4=({uint64_t p1=0ULL;f44(v2, v2_n, p1, v0, v0_n, v1, v1_n, v3, v3_n);});
v4=({uint64_t p1=v4;f35(v2, v2_n, p1, v0, v0_n, v1, v1_n, v3, v3_n);});
v4=({uint64_t p1=v4;f37(v2, v2_n, p1, v0, v0_n, v3, v3_n);});
v4=({uint64_t p1=v4;f40(v2, v2_n, p1, v0, v0_n, v1, v1_n, v3, v3_n);});
if(({uint64_t _a=o_get(v3, v3_n, 2ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=63ULL;o_op(15, 5, _a,_b);});uint64_t _b=2ULL;o_op(13, 5, _a,_b);}));
}
v4=({uint64_t p1=v4;f42(v2, v2_n, p1, v0, v0_n, v3, v3_n);});
v4=({uint64_t p1=v4;f43(v2, v2_n, p1, v0, v0_n, v1, v1_n, v3, v3_n);});
uint64_t __attribute__((unused)) v5=o_get(v3, v3_n, 0ULL, 5);
v4=({uint64_t p1=v4;uint64_t p2=v5;f45(v2, v2_n, p1, p2);});
return o_leave(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t _b=v4;o_op(13, 5, _a,_b);}));
}
uint32_t omega_doctor(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n){
  if(!o_buf(result,1,1,5))return 32;
if(!o_buf(p0, p0_n,256, 5))return 32;
  if(o_overlap(result,8,p0, p0_n*8))return 32;
if(!o_buf(p1, p1_n,262144, 2))return 32;
  if(o_overlap(result,8,p1, p1_n*1))return 32;
if(!o_buf(p2, p2_n,786432, 2))return 32;
  if(o_overlap(result,8,p2, p2_n*1))return 32;
if(o_overlap(p2, p2_n*1, p0, p0_n*8))return 32;
if(o_overlap(p2, p2_n*1, p1, p1_n*1))return 32;
  if(o_busy)return 33;
  o_busy=1;o_depth=0;int code=setjmp(o_jmp);if(!code){uint64_t _result=f46(p0, p0_n, p1, p1_n, p2, p2_n);o_put(result,1,0,_result,5);}
  o_busy=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
