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
static uint64_t f0(const void *v0, uint64_t v0_n);
static uint64_t f1(uint64_t v0);
static uint64_t f2(uint64_t v0);
static uint64_t f3(uint64_t v0);
static uint64_t f4(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f5(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f6(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f7(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f8(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f9(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f10(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f11(const void *v0, uint64_t v0_n);
static uint64_t f12(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f13(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f14(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f15(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f16(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n);
static uint64_t f17(uint64_t v0, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n, void *v4, uint64_t v4_n);
static uint64_t f0(const void *v0, uint64_t v0_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 13ULL, 5);uint64_t _b=110ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 14ULL, 5);uint64_t _b=111ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 15ULL, 5);uint64_t _b=110ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 16ULL, 5);uint64_t _b=101ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 17ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
static uint64_t f1(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=o_cast(9, 5, ({uint64_t _a=v0;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));
if(({uint64_t _a=v1;uint64_t _b=2147483648ULL;o_op(11, 9, _a,_b);})){
v1=({uint64_t _a=v1;uint64_t _b=4294967296ULL;o_op(2, 9, _a,_b);});
}
return o_leave(v1);
}
static uint64_t f2(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
return o_leave(o_cast(5, 9, ({uint64_t _a=v0;uint64_t _b=4294967296ULL;o_op(1, 9, _a,_b);})));
}
return o_leave(o_cast(5, 9, v0));
}
static uint64_t f3(uint64_t v0){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);}));
}
static uint64_t f4(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
return o_leave(o_get(v0, v0_n, v1, 5));
}
static uint64_t f5(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
{
uint64_t v3=0;
uint64_t k1=40ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=v4;uint64_t _b=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 2);o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 2, _a,_b);})){
return o_leave(1ULL);
}
}
}
return o_leave(0ULL);
}
static uint64_t f6(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=v2;uint64_t _b=8192ULL;o_op(3, 5, _a,_b);});uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v6=({uint64_t _a=v2;uint64_t _b=65536ULL;o_op(3, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v6=({uint64_t _a=v6;uint64_t _b=10240ULL;o_op(1, 5, _a,_b);});
}
{
uint64_t v7=0;
uint64_t c1=0;
for(;o_op(8,5, v7, v5);v7=o_op(1,5, v7,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t p1=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=v7;uint64_t _b=40ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t p2=v4;f5(v1, v1_n, p1, p2);})){
return o_leave(o_cast(9, 5, v7));
}
}
}
return o_leave(18446744073709551615ULL);
}
static uint64_t f7(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
return o_leave(4ULL);
}
static uint64_t f8(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=32ULL;uint64_t _b=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
return o_leave(({uint64_t p1=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1);}));
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(10, 9, _a,_b);})){
return o_leave(({uint64_t p1=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1);}));
}
return o_leave(({uint64_t p1=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1);}));
}
static uint64_t f9(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);f1(p0);});uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, v2, 5);uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t p1=5ULL;uint64_t p2=v3;uint64_t p3=6ULL;f7(v1, v1_n, p1, p2, p3);}));
}
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=10ULL;o_op(1, 5, _a,_b);}), 5);f1(p0);});uint64_t _b=0ULL;o_op(11, 9, _a,_b);})){
return o_leave(({uint64_t p1=6ULL;uint64_t p2=v3;uint64_t p3=6ULL;f7(v1, v1_n, p1, p2, p3);}));
}
}
return o_leave(0ULL);
}
static uint64_t f10(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
{uint64_t _i=16ULL;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=11ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=17ULL;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=12ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 16ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 17ULL, 5);
uint64_t __attribute__((unused)) v5=({uint64_t _a=v3;uint64_t _b=v4;o_op(3, 5, _a,_b);});
v5=({uint64_t _a=v5;uint64_t _b=v3;o_op(1, 5, _a,_b);});
v5=({uint64_t _a=v5;uint64_t _b=v4;o_op(1, 5, _a,_b);});
{uint64_t _i=18ULL;uint64_t _v=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=19ULL;uint64_t _v=1048576ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=13ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=o_cast(5, 4, ({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=o_cast(4, 5, o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=14ULL;o_op(1, 5, _a,_b);}), 5));uint64_t _b=31ULL;o_op(12, 4, _a,_b);});o_op(15, 4, _a,_b);}));o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=22ULL;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=14ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f11(const void *v0, uint64_t v0_n){
o_enter();
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, 24ULL, 5);f1(p0);});uint64_t _b=18446744073709551612ULL;o_op(6, 9, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 25ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 26ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(({uint64_t _a=o_get(v0, v0_n, 27ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);}));
}
static uint64_t f12(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=288ULL;uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=16ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=32ULL;uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=10ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({f11(v2, v2_n);})){
return o_leave(({uint64_t p1=3ULL;uint64_t p2=o_get(v2, v2_n, 3ULL, 5);uint64_t p3=1ULL;f7(v2, v2_n, p1, p2, p3);}));
}
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 9ULL, 5);
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=4607182418800017408ULL;o_fp(8, 12, _a,_b);});o_op(12, 1, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
v5=4606281698874543309ULL;
}
{uint64_t _i=9ULL;uint64_t _v=v5;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=2ULL;o_put(v2, v2_n, _i,_v,5);}
return o_leave(2ULL);
}
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v5=0ULL;
uint64_t __attribute__((unused)) v6=({uint64_t p0=o_get(v2, v2_n, 5ULL, 5);f1(p0);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
v5=({uint64_t _a=o_get(v2, v2_n, 31ULL, 5);uint64_t _b=o_get(v2, v2_n, 29ULL, 5);o_fp(3, 12, _a,_b);});
}
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(10, 9, _a,_b);})){
v5=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 31ULL, 5);uint64_t _b=0;o_fp(17, 12, _a,_b);});uint64_t _b=o_get(v2, v2_n, 29ULL, 5);o_fp(3, 12, _a,_b);});
}
{uint64_t _i=32ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 28ULL, 5);uint64_t _b=v5;o_fp(1, 12, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=3ULL;o_put(v2, v2_n, _i,_v,5);}
return o_leave(3ULL);
}
{uint64_t _i=v4;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 33ULL, 5);uint64_t _b=4652007308841189376ULL;o_fp(4, 12, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(1, 5, _a,_b);});uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v5=({uint64_t p2=v3;uint64_t p3=o_get(v2, v2_n, 3ULL, 5);f9(v0, v0_n, v2, v2_n, p2, p3);});
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v5);
}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f13(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=10ULL;uint64_t p2=0ULL;uint64_t p3=6ULL;f7(v2, v2_n, p1, p2, p3);}));
}
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v3=({f12(v0, v0_n, v1, v1_n, v2, v2_n);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v3);
}
}
if(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{
uint64_t v3=0;
uint64_t k2=o_get(v0, v0_n, 1ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
if(c2++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t _a=32ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=1ULL;uint64_t p2=v3;uint64_t p3=1ULL;f7(v2, v2_n, p1, p2, p3);}));
}
if(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v2, v2_n, _i,_v,5);}
}
}
}
}
{
uint64_t c1=0;
while(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=o_get(v0, v0_n, 0ULL, 5);o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 3ULL, 5);
uint64_t __attribute__((unused)) v4=({uint64_t _a=288ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=32ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=10ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=9ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);f1(p0);});uint64_t _b=0ULL;o_op(11, 9, _a,_b);})){
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t p1=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p2=({uint64_t p0=o_get(v2, v2_n, 5ULL, 5);f1(p0);});f8(v0, v0_n, p1, p2);});uint64_t _b=4472406533629990549ULL;o_fp(3, 12, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=10ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, v4, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v4, 5);uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(({uint64_t _a=o_get(v2, v2_n, 7ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=2ULL;uint64_t p2=v3;uint64_t p3=1ULL;f7(v2, v2_n, p1, p2, p3);}));
}
({uint64_t p2=v4;f10(v0, v0_n, v2, v2_n, p2);});
return o_leave(1ULL);
}else{
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=4ULL;uint64_t p2=v3;uint64_t p3=1ULL;f7(v2, v2_n, p1, p2, p3);}));
}
uint64_t __attribute__((unused)) v6=({uint64_t p1=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1);});
if(({uint64_t _a=({uint64_t p0=o_get(v2, v2_n, 5ULL, 5);f1(p0);});uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
v6=({uint64_t p1=({uint64_t _a=v4;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1);});
}
if(({uint64_t _a=({uint64_t p0=o_get(v2, v2_n, 5ULL, 5);f1(p0);});uint64_t _b=0ULL;o_op(10, 9, _a,_b);})){
v6=({uint64_t p1=({uint64_t _a=v4;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1);});
}
{uint64_t _i=v5;uint64_t _v=v6;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t p0=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 5);f3(p0);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v2, v2_n, _i,_v,5);}
}
}
uint64_t __attribute__((unused)) v6=({uint64_t p2=v4;uint64_t p3=v3;f9(v0, v0_n, v2, v2_n, p2, p3);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v6);
}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
}
}
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=7ULL;uint64_t p2=0ULL;uint64_t p3=3ULL;f7(v2, v2_n, p1, p2, p3);}));
}
if(({uint64_t _a=({uint64_t p0=o_get(v2, v2_n, 6ULL, 5);f1(p0);});uint64_t _b=0ULL;o_op(9, 9, _a,_b);})){
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 35ULL, 5);uint64_t _b=86400ULL;o_op(4, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, 5ULL, 5);f1(p0);});uint64_t _b=({uint64_t p0=o_get(v0, v0_n, 6ULL, 5);f1(p0);});o_op(1, 9, _a,_b);});
if(({uint64_t _a=({uint64_t p0=o_get(v2, v2_n, 6ULL, 5);f1(p0);});uint64_t _b=v3;o_op(10, 9, _a,_b);})){
{uint64_t _i=34ULL;uint64_t _v=({uint64_t p0=({uint64_t _a=({uint64_t _a=({uint64_t p0=o_get(v2, v2_n, 6ULL, 5);f1(p0);});uint64_t _b=({uint64_t p0=o_get(v0, v0_n, 5ULL, 5);f1(p0);});o_op(2, 9, _a,_b);});uint64_t _b=({uint64_t p0=o_get(v0, v0_n, 6ULL, 5);f1(p0);});o_op(2, 9, _a,_b);});f2(p0);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(({uint64_t p1=8ULL;uint64_t p2=0ULL;uint64_t p3=2ULL;f7(v2, v2_n, p1, p2, p3);}));
}
}
{uint64_t _i=0ULL;uint64_t _v=o_get(v0, v0_n, 0ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=o_get(v0, v0_n, 1ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v0, v0_n, 2ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v0, v0_n, 12ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=o_get(v0, v0_n, 7ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}
{uint64_t _i=5ULL;uint64_t _v=o_get(v0, v0_n, 8ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=o_get(v0, v0_n, 9ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=o_get(v0, v0_n, 10ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=({f0(v0, v0_n);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{
uint64_t v3=0;
uint64_t k1=o_get(v0, v0_n, 1ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t _a=32ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=2592ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{uint64_t _i=v5;uint64_t _v=o_get(v0, v0_n, v4, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
}
}
return o_leave(0ULL);
}
static uint64_t f14(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=0ULL;o_op(8, 9, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=o_cast(9, 5, o_get(v0, v0_n, 1ULL, 5));o_op(11, 9, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(6ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=32ULL;uint64_t _b=({uint64_t _a=o_cast(5, 9, v1);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, v2, 5);
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{
uint64_t v4=0;
uint64_t k2=3ULL;
for(;o_op(8,5, v4, k2);v4=o_op(1,5, v4,1)){
{uint64_t _i=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t p1=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _b=v4;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1);});uint64_t _b=4611686018427387904ULL;o_fp(3, 12, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
}
}
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_cast(5, 5, ({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));o_put(v0, v0_n, _i,_v,5);}
}
if(({uint64_t _a=v3;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
{
uint64_t v4=0;
uint64_t k2=o_get(v0, v0_n, 0ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v4, k2);v4=o_op(1,5, v4,1)){
if(c2++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=288ULL;uint64_t _b=({uint64_t _a=v4;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, ({uint64_t _a=v5;uint64_t _b=10ULL;o_op(1, 5, _a,_b);}), 5);f1(p0);});uint64_t _b=v1;o_op(6, 9, _a,_b);})){
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=9ULL;o_op(1, 5, _a,_b);});uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
}
}
}
}
if(({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1);});uint64_t _b=4611686018427387904ULL;o_fp(3, 12, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
}
return o_leave(0ULL);
}
static uint64_t f15(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=o_get(v0, v0_n, 0ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v5=1ULL;
{
uint64_t v6=0;
uint64_t k2=40ULL;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
uint64_t __attribute__((unused)) v7=o_get(v1, v1_n, ({uint64_t _a=({uint64_t _a=v4;uint64_t _b=40ULL;o_op(3, 5, _a,_b);});uint64_t _b=v6;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(6, 2, _a,_b);})){
if(({uint64_t _a=v6;uint64_t _b=o_get(v3, v3_n, 40ULL, 5);o_op(7, 5, _a,_b);})){
v5=0ULL;
}
break;
}
if(({uint64_t _a=o_cast(5, 2, v7);uint64_t _b=o_get(v2, v2_n, ({uint64_t _a=8000ULL;uint64_t _b=v6;o_op(1, 5, _a,_b);}), 5);o_op(7, 5, _a,_b);})){
v5=0ULL;
}
}
}
if(({uint64_t _a=o_get(v3, v3_n, 40ULL, 5);uint64_t _b=39ULL;o_op(10, 5, _a,_b);})){
v5=0ULL;
}
if(v5){
{uint64_t _i=({uint64_t _a=7600ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
}
}
}
return o_leave(0);
}
static uint64_t f16(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=o_get(v3, v3_n, 41ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, ({uint64_t _a=130816ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=v5;uint64_t _b=44ULL;o_op(6, 2, _a,_b);})){
({f15(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
{uint64_t _i=40ULL;uint64_t _v=0ULL;o_put(v3, v3_n, _i,_v,5);}
}else{
if(({uint64_t _a=o_get(v3, v3_n, 40ULL, 5);uint64_t _b=40ULL;o_op(8, 5, _a,_b);})){
{uint64_t _i=({uint64_t _a=8000ULL;uint64_t _b=o_get(v3, v3_n, 40ULL, 5);o_op(1, 5, _a,_b);});uint64_t _v=o_cast(5, 2, v5);o_put(v2, v2_n, _i,_v,5);}
}
if(({uint64_t _a=o_get(v3, v3_n, 40ULL, 5);uint64_t _b=40ULL;o_op(8, 5, _a,_b);})){
{uint64_t _i=40ULL;uint64_t _v=({uint64_t _a=o_get(v3, v3_n, 40ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v3, v3_n, _i,_v,5);}
}
}
}
}
if(({uint64_t _a=o_get(v3, v3_n, 42ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({f15(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
}
return o_leave(0ULL);
}
static uint64_t f17(uint64_t v0, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n, void *v4, uint64_t v4_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f13(v1, v1_n, v2, v2_n, v4, v4_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=({uint64_t p0=o_get(v4, v4_n, 1ULL, 5);f1(p0);});f14(v1, v1_n, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p2=o_get(v4, v4_n, 0ULL, 5);uint64_t p3=o_get(v4, v4_n, 1ULL, 5);uint64_t p4=130816ULL;f6(v1, v1_n, v3, v3_n, p2, p3, p4);});f2(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f16(v1, v1_n, v3, v3_n, v2, v2_n, v4, v4_n);}));
}
return o_leave(6ULL);
}
uint32_t omega_dag_res(uint64_t  *result, volatile uint64_t p0, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n){
  if(!o_buf(result,1,1,5))return 32;
if(!o_valid(5, p0))return 32;
if(!o_buf(p1, p1_n,16384, 5))return 32;
  if(o_overlap(result,8,p1, p1_n*8))return 32;
if(!o_buf(p2, p2_n,8192, 5))return 32;
  if(o_overlap(result,8,p2, p2_n*8))return 32;
if(o_overlap(p2, p2_n*8, p1, p1_n*8))return 32;
if(!o_buf(p3, p3_n,131072, 2))return 32;
  if(o_overlap(result,8,p3, p3_n*1))return 32;
if(o_overlap(p3, p3_n*1, p1, p1_n*8))return 32;
if(o_overlap(p3, p3_n*1, p2, p2_n*8))return 32;
if(!o_buf(p4, p4_n,64, 5))return 32;
  if(o_overlap(result,8,p4, p4_n*8))return 32;
if(o_overlap(p4, p4_n*8, p1, p1_n*8))return 32;
if(o_overlap(p4, p4_n*8, p2, p2_n*8))return 32;
if(o_overlap(p4, p4_n*8, p3, p3_n*1))return 32;
  if(o_busy)return 33;
  o_busy=1;o_depth=0;int code=setjmp(o_jmp);if(!code){uint64_t _result=f17(p0, p1, p1_n, p2, p2_n, p3, p3_n, p4, p4_n);o_put(result,1,0,_result,5);}
  o_busy=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
