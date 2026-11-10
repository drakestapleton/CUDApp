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
static uint64_t f0(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f1(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f2(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f3(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f4(uint64_t v0);
static uint64_t f5(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f6(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f7(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f8(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f9(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f10(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f11(uint64_t v0);
static uint64_t f12(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f13(uint64_t v0);
static uint64_t f14(uint64_t v0);
static uint64_t f15(uint64_t v0);
static uint64_t f16(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f17(uint64_t v0);
static uint64_t f18(uint64_t v0, uint64_t v1);
static uint64_t f19(uint64_t v0);
static uint64_t f20(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f21(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f22(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f23(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f24(const void *v0, uint64_t v0_n);
static uint64_t f25(const void *v0, uint64_t v0_n);
static uint64_t f26(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f27(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f28(uint64_t v0, uint64_t v1);
static uint64_t f29(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f30(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f31(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f32(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f33(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f34(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f35(uint64_t v0, uint64_t v1, uint64_t v2, void *v3, uint64_t v3_n);
static uint64_t f0(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(16, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v2;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(12, 5, _a,_b);}));
}
static uint64_t f1(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 3ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=255ULL;o_op(8, 5, _a,_b);})){
{uint64_t _i=v3;uint64_t _v=o_cast(2, 5, v2);o_put(v0, v0_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f2(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=v2;
{
uint64_t v4=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
({uint64_t p2=({uint64_t _a=v3;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});f1(v0, v0_n, v1, v1_n, p2);});
v3=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f3(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t v3[20]={0};
uint64_t v3_n=20;
o_fill(v3, v3_n, 0ULL, 5);
uint64_t __attribute__((unused)) v4=v2;
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t v6=0;
uint64_t k1=20ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
{uint64_t _i=v5;uint64_t _v=({uint64_t _a=v4;uint64_t _b=10ULL;o_op(5, 5, _a,_b);});o_put(v3, v3_n, _i,_v,5);}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
}
}
{
uint64_t v6=0;
uint64_t k1=20ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
({uint64_t p2=({uint64_t _a=48ULL;uint64_t _b=o_get(v3, v3_n, v5, 5);o_op(1, 5, _a,_b);});f1(v0, v0_n, v1, v1_n, p2);});
}
}
return o_leave(0);
}
static uint64_t f4(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=48ULL;o_op(1, 5, _a,_b);}));
}
return o_leave(({uint64_t _a=v0;uint64_t _b=87ULL;o_op(1, 5, _a,_b);}));
}
static uint64_t f5(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=1ULL;
{
uint64_t v4=0;
uint64_t k1=15ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
{
uint64_t v4=0;
uint64_t k1=16ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
({uint64_t p2=({uint64_t p0=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=15ULL;o_op(12, 5, _a,_b);});f4(p0);});f1(v0, v0_n, v1, v1_n, p2);});
}
}
return o_leave(0);
}
static uint64_t f6(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({uint64_t p2=30768ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v2;f5(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f7(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=16ULL;
{
uint64_t v4=0;
uint64_t k1=16ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
({uint64_t p2=({uint64_t p0=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=15ULL;o_op(12, 5, _a,_b);});f4(p0);});f1(v0, v0_n, v1, v1_n, p2);});
}
}
return o_leave(0);
}
static uint64_t f8(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(15, 5, _a,_b);});o_op(11, 5, _a,_b);})){
({uint64_t p2=45ULL;f1(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});uint64_t _b=v2;o_op(2, 5, _a,_b);});f6(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
({uint64_t p2=v2;f6(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f9(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=33ULL;f1(v0, v0_n, v1, v1_n, p2);});
}
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=21584ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
({uint64_t p2=80ULL;f1(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v2;f3(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f10(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=v2;f2(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v3;uint64_t _b=255ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=90ULL;f1(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
({uint64_t p2=v3;f3(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f11(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=0ULL;
uint64_t __attribute__((unused)) v2=({uint64_t _a=v0;uint64_t _b=64ULL;o_op(4, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=9223363172042244111ULL;
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v1=18446744069418713119ULL;
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v1=61247ULL;
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v1=9223372036854775808ULL;
}
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=({uint64_t _a=v0;uint64_t _b=64ULL;o_op(5, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);}));
}
static uint64_t f12(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=33ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=6354091206005183059ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=34ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=6426148800043110995ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=35ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=6498206394081038931ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=37ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=4920535898042356307ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=22574ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=38ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=4920535898042356307ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=22830ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=39ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=4920535898042356307ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=23086ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=5279712180522930771ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=68ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=80ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=5423265565250638419ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=20300ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=81ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=5423265565250638419ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=18760ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=82ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=4702408149961757267ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=5714032484829189196ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=83ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=4702408149961757267ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=5280561020694778956ULL;f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t p0=v2;f11(p0);})){
{uint64_t _i=5ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
({uint64_t p2=21075ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v2;f3(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f13(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(70ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(21580ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(20805ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(17740ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(21575ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(17742ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(17735ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(5068110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(5128526ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(5592140ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(5591365ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(5588300ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(5592135ULL);
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(5588302ULL);
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(5588295ULL);
}
return o_leave(84ULL);
}
static uint64_t f14(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(84ULL);
}
return o_leave(({uint64_t p0=v0;f13(p0);}));
}
static uint64_t f15(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(3691822ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(3691310ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(909202734ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(909202222ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(3421742ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(942813486ULL);
}
return o_leave(61788224640302ULL);
}
static uint64_t f16(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=65536ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=256ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _b=v2;o_op(1, 5, _a,_b);}));
}
static uint64_t f17(uint64_t v0){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=100ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=68ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=16ULL;uint64_t p2=8ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=97ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=65ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=24ULL;uint64_t p2=8ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=98ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=66ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=32ULL;uint64_t p2=8ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=99ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=67ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=0ULL;uint64_t p2=8ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=105ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=115ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=102ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=32ULL;uint64_t p2=32ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=106ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=89ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=32ULL;uint64_t p2=32ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=75ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=38ULL;uint64_t p2=21ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=85ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=37ULL;uint64_t p2=22ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=111ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=40ULL;uint64_t p2=24ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=121ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=16ULL;uint64_t p2=4ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=80ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=17ULL;uint64_t p2=3ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=81ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=20ULL;uint64_t p2=3ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=82ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=23ULL;uint64_t p2=4ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=108ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=83ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=8ULL;uint64_t p2=8ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=104ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=53ULL;uint64_t p2=5ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=109ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=40ULL;uint64_t p2=13ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=71ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=12ULL;uint64_t p2=3ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=103ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=12ULL;uint64_t p2=4ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=90ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=9ULL;uint64_t p2=1ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=122ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=9ULL;uint64_t p2=3ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=78ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=8ULL;uint64_t p2=1ULL;f16(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=77ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=63ULL;uint64_t p2=1ULL;f16(p0, p1, p2);}));
}
return o_leave(0ULL);
}
static uint64_t f18(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=v1;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=v0;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});o_op(15, 5, _a,_b);}));
}
static uint64_t f19(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=255ULL;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=30ULL;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=34ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=18ULL;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);}));
}
static uint64_t f20(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v2=o_get(v0, v0_n, 1ULL, 5);
}
return o_leave(({uint64_t p0=v2;uint64_t p1=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t p2=({uint64_t _a=v1;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});f0(p0, p1, p2);}));
}
static uint64_t f21(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=10ULL;o_op(3, 5, _a,_b);});uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(4, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);
return o_leave(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(5, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);}));
}
static uint64_t f22(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t v5=0;
uint64_t k1=64ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
if(({uint64_t _a=v4;uint64_t _b=63ULL;o_op(11, 5, _a,_b);})){
break;
}
uint64_t __attribute__((unused)) v6=({uint64_t p1=v1;uint64_t p2=v4;f21(v0, v0_n, p1, p2);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
if(({uint64_t _a=v6;uint64_t _b=37ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t p1=v1;uint64_t p2=v4;f21(v0, v0_n, p1, p2);});
if(({uint64_t _a=v7;uint64_t _b=84ULL;o_op(6, 5, _a,_b);})){
v3=({uint64_t _a=v3;uint64_t _b=({uint64_t p0=v2;f19(p0);});o_op(13, 5, _a,_b);});
}else{
v3=({uint64_t _a=v3;uint64_t _b=({uint64_t p0=({uint64_t p0=v7;f17(p0);});uint64_t p1=v2;f18(p0, p1);});o_op(13, 5, _a,_b);});
}
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v3);
}
static uint64_t f23(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=10ULL;o_op(3, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=({uint64_t p1=v1;uint64_t p2=0ULL;f22(v0, v0_n, p1, p2);});uint64_t _b=61440ULL;o_op(13, 5, _a,_b);});uint64_t _b=0;o_op(18, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=2199023255551ULL;uint64_t _b=({uint64_t _a=({uint64_t p1=v1;uint64_t p2=1ULL;f22(v0, v0_n, p1, p2);});uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v5;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, v4, 5);uint64_t _b=v5;o_op(12, 5, _a,_b);});o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v6;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=v6;o_op(12, 5, _a,_b);});o_op(6, 5, _a,_b);}));
}
static uint64_t f24(const void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=({uint64_t p0=v1;uint64_t p1=16ULL;uint64_t p2=8ULL;f0(p0, p1, p2);});uint64_t _b=({uint64_t _a=({uint64_t p0=v1;uint64_t p1=34ULL;uint64_t p2=30ULL;f0(p0, p1, p2);});uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, 1ULL, 5);uint64_t p1=0ULL;uint64_t p2=18ULL;f0(p0, p1, p2);});uint64_t _b=38ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=36028797018963968ULL;o_op(11, 5, _a,_b);})){
v2=({uint64_t _a=v2;uint64_t _b=18374686479671623680ULL;o_op(13, 5, _a,_b);});
}
return o_leave(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=16ULL;o_op(1, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);}));
}
static uint64_t f25(const void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t p0=o_get(v0, v0_n, 0ULL, 5);uint64_t p1=32ULL;uint64_t p2=32ULL;f0(p0, p1, p2);});
if(({uint64_t _a=v1;uint64_t _b=2147483648ULL;o_op(11, 5, _a,_b);})){
v1=({uint64_t _a=v1;uint64_t _b=18446744069414584320ULL;o_op(13, 5, _a,_b);});
}
return o_leave(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=16ULL;o_op(1, 5, _a,_b);});uint64_t _b=v1;o_op(1, 5, _a,_b);}));
}
static uint64_t f26(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=({uint64_t p0=v2;f17(p0);});f20(v1, v1_n, p1);});
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=100ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=97ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=98ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=99ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
({uint64_t p2=82ULL;uint64_t p3=v3;f10(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=68ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=65ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=66ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=67ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
({uint64_t p2=21077ULL;uint64_t p3=v3;f10(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=105ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=108ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=104ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=109ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
({uint64_t p2=v3;f6(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=115ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=106ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
({uint64_t p2=v3;uint64_t p3=32ULL;f8(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=75ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=2016435043ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=54ULL;uint64_t p2=5ULL;f0(p0, p1, p2);});f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=2016435037ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=38ULL;uint64_t p2=16ULL;f0(p0, p1, p2);});f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=93ULL;f1(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=85ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=2016435043ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=54ULL;uint64_t p2=5ULL;f0(p0, p1, p2);});f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=2016435037ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=37ULL;uint64_t p2=17ULL;f0(p0, p1, p2);});f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=93ULL;f1(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=111ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=43ULL;f1(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v3;uint64_t p3=24ULL;f8(v0, v0_n, v1, v1_n, p2, p3);});
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=89ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=({f25(v1, v1_n);});f6(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=121ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=66ULL;f1(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v3;f3(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=80ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=81ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
({uint64_t p2=v3;uint64_t p3=0ULL;f9(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=82ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=({uint64_t _a=v3;uint64_t _b=7ULL;o_op(12, 5, _a,_b);});uint64_t p3=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});f9(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=83ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=v3;f12(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=71ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=({uint64_t p0=v3;f14(p0);});f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=103ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=({uint64_t p0=v3;f13(p0);});f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=90ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=842224942ULL;f2(v0, v0_n, v1, v1_n, p2);});
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=122ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=({uint64_t p0=v3;f15(p0);});f2(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=78ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=77ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=45ULL;f1(v0, v0_n, v1, v1_n, p2);});
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=84ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=({f24(v1, v1_n);});f6(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f27(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t v5=0;
uint64_t k1=64ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
if(({uint64_t _a=v4;uint64_t _b=64ULL;o_op(11, 5, _a,_b);})){
break;
}
uint64_t __attribute__((unused)) v6=({uint64_t p1=v3;uint64_t p2=v4;f21(v2, v2_n, p1, p2);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
if(({uint64_t _a=v6;uint64_t _b=37ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=64ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
({uint64_t p2=({uint64_t p1=v3;uint64_t p2=v4;f21(v2, v2_n, p1, p2);});f26(v0, v0_n, v1, v1_n, p2);});
}else{
({uint64_t p2=v6;f1(v0, v0_n, v1, v1_n, p2);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f28(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=255ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(2ULL);
}
if(({uint64_t _a=v1;uint64_t _b=255ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v0;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v2=0ULL;
uint64_t __attribute__((unused)) v3=v0;
{
uint64_t v4=0;
uint64_t k1=32ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
break;
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=31ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(3ULL);
}
static uint64_t f29(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=4095ULL;o_op(12, 5, _a,_b);});uint64_t _b=2084ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=255ULL;uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=126747136ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t p0=v2;uint64_t p1=32ULL;uint64_t p2=32ULL;f0(p0, p1, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v3;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p0=v4;uint64_t p1=v5;f28(p0, p1);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=1145130313ULL;f2(v0, v0_n, v1, v1_n, p2);});
}
if(({uint64_t _a=v6;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=6219274471530909001ULL;f2(v0, v0_n, v1, v1_n, p2);});
}
if(({uint64_t _a=v6;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=4918292731127745865ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=68ULL;f2(v0, v0_n, v1, v1_n, p2);});
}
if(({uint64_t _a=v6;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=5496734803384421705ULL;f2(v0, v0_n, v1, v1_n, p2);});
}
({uint64_t p2=32ULL;f1(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=82ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=16ULL;uint64_t p2=8ULL;f0(p0, p1, p2);});f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=8236ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=82ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=24ULL;uint64_t p2=8ULL;f0(p0, p1, p2);});f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=8236ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v4;uint64_t p3=32ULL;f8(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=8236ULL;f2(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=82ULL;uint64_t p3=v5;f10(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(1ULL);
}
static uint64_t f30(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=2050ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=30722ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=3840ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=2318338018808057677ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=26917ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=514ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=4278219266ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=3840ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=2318338018808057677ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=25125ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=29186ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=3840ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=2318338018808057677ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=25125ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2329ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31001ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=2318338018807788115ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=21285ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2946ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=4278221698ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2048ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=2318338018806809676ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=19237ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=4278221698ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=2560ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=2675195585967113292ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=322745412708ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1964ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=4278220716ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=134219776ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3189715256884806732ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=5580064ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=4278220716ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=134220288ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=2320539280629515340ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=93617941922853ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=548ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29220ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=126747136ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3198722455854206281ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3198159505500218656ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=6497568ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3108ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31780ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=260964864ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3198722455854206281ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3189152306245477664ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=6497568ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2085ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=30757ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=126746624ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=4920559924104219977ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=7216209377912761925ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=8297073263480479788ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=1663377452ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=551ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29223ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=126746624ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3335276362595388745ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=2318338018805691221ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=2318335819782054181ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=25381ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=528ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29200ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=134209536ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=7216209382491832649ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=6075391272087986220ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=2318334721041702956ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=7144151758890421541ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2064ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=30736ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=134209536ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=7216209382491832649ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=6075391272087986220ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=2318334721041702956ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=109011104723749ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=530ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29202ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=917504ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=6076847033519460172ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3197878030523704608ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3198440980476994848ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=23121768204739872ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2066ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=30738ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=917504ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=6076847033519460172ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3197878030523704608ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3198440980477453600ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=23121768204739872ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2073ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=30745ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=71168ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3698913599453284435ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3198722455938543154ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3200129830337193248ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=6497568ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=30745ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=1536ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=3698913573683480659ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=7000036570848108594ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=7144151759007850540ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=537ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29209ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=71168ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3698913599453284435ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3198722455938543154ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3198159505500218656ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=6497568ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=29209ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=1536ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=3698913573683480659ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=7000036570848108594ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=7144151758890410028ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
return o_leave(0ULL);
}
static uint64_t f31(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=524ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29196ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=5126554673377268553ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=2675213281549769253ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=2675186905684913232ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=2675186978699357281ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2060ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=30732ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=5126554673377268553ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=2675213281549769253ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=2675186905684913232ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=2675187013059095649ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3084ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31756ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=134217840ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=5126554673377268553ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=2675213281549769253ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=2675186905684913232ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=2675186841260403809ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=519ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29191ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=2318338018807399763ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=2318335819782054181ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=21029ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2055ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=30727ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=2318338018807399763ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=2318343516363448613ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=21029ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=545ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29217ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3198722455854399814ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=27625367831389472ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=9223372036854805025ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=3198722455854399814ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=7072108406061999392ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=544ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29216ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=4194304ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3198722455989734726ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=27625367831389472ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=547ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29219ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3198722455804659270ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3198159505500218656ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=6497568ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=521ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29193ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=7216209541557275974ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=7072094164835704876ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=1378164780ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=523ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29195ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=7432397682590962502ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3193092956320842030ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3197878030522459424ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=23121768204084512ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=581ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29253ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2101248ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3617312090437857865ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3198722455551300910ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=6432032ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=29253ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=2102272ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=3617312090437857865ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=3198722455551300398ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=6432032ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=773ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29445ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2158592ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3328779515522003526ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=6074843813981278804ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=7072094164886036570ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=574ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29246ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=75526144ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=6071225231052517958ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=4984998065855154502ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3328779451096059188ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=6864120659606454608ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=2324806583807264077ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=2318334720270427173ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=109011104719397ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=29246ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=4351ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=3550598122910921286ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=4706312403745648182ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=7216209446867782467ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=15ULL;uint64_t _v=7072094164835704876ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=572ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29244ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=268288ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=4050478989600181576ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=4768804251234088497ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=2318338018805952838ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=2318335819782054181ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=25381ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=29244ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=6144ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=4050478989600181576ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=2675193374460229169ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=2675186974404389988ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=15ULL;uint64_t _v=425824627810ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=634ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29306ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=11264ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=4050478989600181585ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=4984977033347871283ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3696668340873547060ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=3197878030523704608ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=27906842808165664ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f32(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=776ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29448ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2048ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3627725565148419405ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=27625367831586080ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=29448ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=4096ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=5783556705426494797ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=27625367831586080ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=29448ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=5120ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=22ULL;uint64_t _v=5860117899091793229ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=23ULL;uint64_t _v=27625367831586080ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=30ULL;uint64_t _v=29448ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=31ULL;uint64_t _v=3072ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=32ULL;uint64_t _v=3622948187125732685ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=33ULL;uint64_t _v=27625367831586080ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=40ULL;uint64_t _v=29448ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=41ULL;uint64_t _v=6144ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=42ULL;uint64_t _v=5783556705426494797ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=43ULL;uint64_t _v=2318338018807133238ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=44ULL;uint64_t _v=25125ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=50ULL;uint64_t _v=29448ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=51ULL;uint64_t _v=7168ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=52ULL;uint64_t _v=5860117899091793229ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=53ULL;uint64_t _v=2318338018807133238ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=54ULL;uint64_t _v=25125ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(6ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2433ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31105ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=203297024ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=2340223935431787596ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=7166182912787506213ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3341993472822486363ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=401296667702ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=31105ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=203380992ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=6076251196916974668ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=6005923036305182546ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=8315162376355980576ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=15ULL;uint64_t _v=7000101651917134691ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=16ULL;uint64_t _v=102731946931758ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=31105ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=203364608ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=22ULL;uint64_t _v=5570159188791215180ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=23ULL;uint64_t _v=2675231981938036553ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=24ULL;uint64_t _v=6585233959454518372ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=25ULL;uint64_t _v=3904164740051321381ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=26ULL;uint64_t _v=1567565108ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(3ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2438ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31110ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=202379520ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=2340223935431791699ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=6720256143407670628ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=8008864919659291995ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=421529660509ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=31110ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=202463488ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=6076251196916978771ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=6005923036305182546ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=4838373857375380512ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=15ULL;uint64_t _v=2680827251018521437ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=16ULL;uint64_t _v=107911593090415ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2472ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=9223372036854806952ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=136270080ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3334121982912320577ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=5715723593321825345ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=5773706365401712462ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=7214815411678751828ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=6583492058011628389ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=2318330387017654565ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=25125ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=9223372036854806952ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=144659712ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=3334121982912320577ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=5993785561386469709ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=6436539176233947732ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=15ULL;uint64_t _v=7216209352696275027ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=16ULL;uint64_t _v=2691854549557452844ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=17ULL;uint64_t _v=3762245533488078147ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=18ULL;uint64_t _v=421529660509ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2436ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31108ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=134219776ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=2318338018807858252ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=26249900243101019ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=31108ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=7216209686915400780ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=6732641067110965292ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2440ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31112ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=134219776ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3125820689686418515ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=27625367831135013ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2450ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31122ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=45056ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=4696781901632718157ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=91643112737868ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3977ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=576460752303456137ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=917504ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=6291322170098731091ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=7216209352696275022ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=7504439729063272492ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=1831149612ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=864691128455167881ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=917504ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=5496153161867872339ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=7216209352696275033ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=7504439729063272492ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=15ULL;uint64_t _v=1831149612ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2328ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31000ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=5263182ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2381ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31053ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=58720256ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=1414092869ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2375ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31047ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=58720256ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=92518431674946ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2373ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31045ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=58720768ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=4847370932954813250ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=5639992527027588687ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=6423575818897006676ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2369ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31041ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=58720768ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=4995105803319464770ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=4992049169717677891ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=520313918542ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2845ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=31517ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=65536ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=4849912062518903106ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=4782631786775921710ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=2325913809262432076ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=3176496ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=560ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=2305843013491913264ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=16640ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3694691337732637000ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=5921424460092874802ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3479081711208246362ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=3164255ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=772ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29444ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2099200ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3329903151980884550ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=2318338018805691206ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=25125ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=553ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29225ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3198722455854399812ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=27906842808100128ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=552ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29224ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3198722455989734724ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=27625367831389472ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=555ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29227ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3198722455804659268ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3198159505500218656ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=6497568ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=554ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29226ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=7432397682590962500ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3193092956320842030ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3197878030522459424ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=23121768204084512ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=784ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29456ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2103296ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3329345699585602118ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=2318338018805691206ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=25125ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=29456ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=3149824ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=3328779451097297478ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=2318338018805823046ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=25125ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=786ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29458ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2104320ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=2320539383054611017ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=107911593092133ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=29458ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=2103296ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=3329345699585602121ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=2318338018805691221ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=25125ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=785ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=29457ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=3199232ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3329345699585798726ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=7216209451380200020ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=1646600236ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f33(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t p0=v0;f30(p0, v1, v1_n);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v2);
}
v2=({uint64_t p0=v0;f31(p0, v1, v1_n);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v2);
}
return o_leave(({uint64_t p0=v0;f32(p0, v1, v1_n);}));
}
static uint64_t f34(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=v1;o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=262144ULL;o_op(3, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f35(uint64_t v0, uint64_t v1, uint64_t v2, void *v3, uint64_t v3_n){
o_enter();
uint64_t v4[8]={0};
uint64_t v4_n=8;
o_fill(v4, v4_n, 0ULL, 5);
uint64_t v5[80]={0};
uint64_t v5_n=80;
o_fill(v5, v5_n, 0ULL, 5);
{uint64_t _i=0ULL;uint64_t _v=v0;o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v1;o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=v2;o_put(v4, v4_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=({uint64_t p0=v0;uint64_t p1=12ULL;uint64_t p2=3ULL;f0(p0, p1, p2);});
uint64_t __attribute__((unused)) v7=({uint64_t p0=v0;uint64_t p1=15ULL;uint64_t p2=1ULL;f0(p0, p1, p2);});
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=7ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
({uint64_t p2=64ULL;f1(v3, v3_n, v4, v4_n, p2);});
({uint64_t p2=v6;uint64_t p3=v7;f9(v3, v3_n, v4, v4_n, p2, p3);});
({uint64_t p2=32ULL;f1(v3, v3_n, v4, v4_n, p2);});
}
uint64_t __attribute__((unused)) v8=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=61440ULL;uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=v1;uint64_t _b=2199023255551ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v10=0ULL;
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=4095ULL;o_op(12, 5, _a,_b);});uint64_t _b=2381ULL;o_op(6, 5, _a,_b);})){
v10=131072ULL;
}
if(({uint64_t p2=v8;uint64_t p3=v9;f29(v3, v3_n, v4, v4_n, p2, p3);})){
return o_leave(({uint64_t p1=v10;f34(v4, v4_n, p1);}));
}
uint64_t __attribute__((unused)) v11=({uint64_t p0=({uint64_t _a=v8;uint64_t _b=4095ULL;o_op(12, 5, _a,_b);});f33(p0, v5, v5_n);});
{
uint64_t v12=0;
uint64_t c1=0;
for(;o_op(8,5, v12, v11);v12=o_op(1,5, v12,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t p1=v12;uint64_t p2=v8;uint64_t p3=v9;f23(v5, v5_n, p1, p2, p3);})){
({uint64_t p3=v12;f27(v3, v3_n, v4, v4_n, v5, v5_n, p3);});
if(({uint64_t _a=o_get(v4, v4_n, 5ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v10;f34(v4, v4_n, p1);}));
}
break;
}
}
}
{uint64_t _i=3ULL;uint64_t _v=0ULL;o_put(v4, v4_n, _i,_v,5);}
({uint64_t p2=516203495231ULL;f2(v3, v3_n, v4, v4_n, p2);});
({uint64_t p2=v1;f7(v3, v3_n, v4, v4_n, p2);});
({uint64_t p2=v0;f7(v3, v3_n, v4, v4_n, p2);});
return o_leave(({uint64_t p1=({uint64_t _a=v10;uint64_t _b=65536ULL;o_op(13, 5, _a,_b);});f34(v4, v4_n, p1);}));
}
uint32_t omega_sm121_dis(uint64_t  *result, volatile uint64_t p0, volatile uint64_t p1, volatile uint64_t p2, void *p3, uint64_t p3_n){
  if(!o_buf(result,1,1,5))return 32;
if(!o_valid(5, p0))return 32;
if(!o_valid(5, p1))return 32;
if(!o_valid(5, p2))return 32;
if(!o_buf(p3, p3_n,256, 2))return 32;
  if(o_overlap(result,8,p3, p3_n*1))return 32;
  if(o_busy)return 33;
  o_busy=1;o_depth=0;int code=setjmp(o_jmp);if(!code){uint64_t _result=f35(p0, p1, p2, p3, p3_n);o_put(result,1,0,_result,5);}
  o_busy=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
