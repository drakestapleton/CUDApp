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
static uint64_t f0(uint64_t v0);
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f2(uint64_t v0);
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, void *v3, uint64_t v3_n);
static uint64_t f4(uint64_t v0, uint64_t v1);
static uint64_t f5(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f6(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f7(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f8(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f9(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f10(uint64_t v0, uint64_t v1);
static uint64_t f11(uint64_t v0, const void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f0(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=81ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(2ULL);
}
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=18446744073709551615ULL;o_op(6, 5, _a,_b);})){
return o_leave(4294967296ULL);
}
if(({uint64_t _a=v0;uint64_t _b=65ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);})){
return o_leave(4294967296ULL);
}
return o_leave(({uint64_t _a=v1;uint64_t _b=8589934592ULL;o_op(13, 5, _a,_b);}));
}
static uint64_t f2(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=v0;
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=1ULL;
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=v1;uint64_t _b=4294967295ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t _a=v2;uint64_t _b=18446744069414584320ULL;o_op(12, 5, _a,_b);}));
}
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, void *v3, uint64_t v3_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v0;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=v0;o_op(8, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=v0;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v2;uint64_t _b=v0;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p0=v4;f2(p0);});
uint64_t __attribute__((unused)) v7=({uint64_t p0=v5;f2(p0);});
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=v7;uint64_t _b=v6;o_op(9, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v8=({uint64_t _a=v0;uint64_t _b=v6;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v8;uint64_t _b=v0;o_op(8, 5, _a,_b);})){
return o_leave(2ULL);
}
uint64_t __attribute__((unused)) v9=({uint64_t _a=v0;uint64_t _b=v7;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v9;uint64_t _b=v0;o_op(8, 5, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=2ULL;uint64_t _v=v8;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=v9;o_put(v3, v3_n, _i,_v,5);}
return o_leave(1ULL);
}
static uint64_t f4(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=v0;uint64_t _b=v2;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=v0;o_op(8, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t _a=v3;uint64_t _b=({uint64_t _a=v2;uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);}));
}
static uint64_t f5(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=3ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2097151ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v0;uint64_t _b=v4;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v5;uint64_t _b=v0;o_op(8, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v5;uint64_t _b=v2;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v6=({uint64_t _a=18446744073709551615ULL;uint64_t _b=v0;o_op(2, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=v6;o_op(10, 5, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
static uint64_t f6(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=v4;o_op(11, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f7(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v4;uint64_t _b=v5;o_op(10, 5, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
static uint64_t f8(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=v1;uint64_t _b=v2;o_op(11, 5, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(1ULL);
}
static uint64_t f9(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=2ULL;uint64_t _b=28ULL;o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v0;uint64_t _b=13ULL;o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(16, 5, _a,_b);});
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v4;o_op(13, 5, _a,_b);});uint64_t _b=v5;o_op(13, 5, _a,_b);});uint64_t _b=v6;o_op(13, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));
}
static uint64_t f10(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t _a=v0;uint64_t _b=18446744073709551612ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=2097151ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v3;uint64_t _b=42ULL;o_op(15, 5, _a,_b);});
return o_leave(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v4;o_op(13, 5, _a,_b);});uint64_t _b=2199023255552ULL;o_op(13, 5, _a,_b);}));
}
static uint64_t f11(uint64_t v0, const void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);f0(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=o_get(v1, v1_n, 1ULL, 5);uint64_t p2=o_get(v1, v1_n, 2ULL, 5);f1(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);f2(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=o_get(v1, v1_n, 1ULL, 5);uint64_t p2=o_get(v1, v1_n, 2ULL, 5);f3(p0, p1, p2, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=o_get(v1, v1_n, 1ULL, 5);f4(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=o_get(v1, v1_n, 1ULL, 5);uint64_t p2=o_get(v1, v1_n, 2ULL, 5);uint64_t p3=o_get(v1, v1_n, 3ULL, 5);f5(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=o_get(v1, v1_n, 1ULL, 5);uint64_t p2=o_get(v1, v1_n, 2ULL, 5);f6(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=o_get(v1, v1_n, 1ULL, 5);uint64_t p2=o_get(v1, v1_n, 2ULL, 5);uint64_t p3=o_get(v1, v1_n, 3ULL, 5);uint64_t p4=o_get(v1, v1_n, 4ULL, 5);uint64_t p5=o_get(v1, v1_n, 5ULL, 5);f7(p0, p1, p2, p3, p4, p5);}));
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=o_get(v1, v1_n, 1ULL, 5);uint64_t p2=o_get(v1, v1_n, 2ULL, 5);uint64_t p3=o_get(v1, v1_n, 3ULL, 5);f8(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=o_get(v1, v1_n, 1ULL, 5);uint64_t p2=o_get(v1, v1_n, 2ULL, 5);f9(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);uint64_t p1=o_get(v1, v1_n, 1ULL, 5);f10(p0, p1);}));
}
return o_leave(0ULL);
}
uint32_t omega_nvrm_policy(uint64_t  *result, volatile uint64_t p0, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n){
  if(!o_buf(result,1,1,5))return 32;
if(!o_valid(5, p0))return 32;
if(!o_buf(p1, p1_n,8, 5))return 32;
  if(o_overlap(result,8,p1, p1_n*8))return 32;
if(!o_buf(p2, p2_n,4, 5))return 32;
  if(o_overlap(result,8,p2, p2_n*8))return 32;
if(o_overlap(p2, p2_n*8, p1, p1_n*8))return 32;
  if(o_busy)return 33;
  o_busy=1;o_depth=0;int code=setjmp(o_jmp);if(!code){uint64_t _result=f11(p0, p1, p1_n, p2, p2_n);o_put(result,1,0,_result,5);}
  o_busy=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
