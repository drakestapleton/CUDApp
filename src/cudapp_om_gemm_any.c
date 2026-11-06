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
static uint64_t f0(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f1(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f2(uint64_t v0);
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f4(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f5(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f6(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f7(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f8(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f9(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f10(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f11(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f12(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f13(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f14(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f15(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f16(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f17(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f18(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f19(const void *v0, uint64_t v0_n);
static uint64_t f20(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f21(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f22(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4);
static uint64_t f23(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4, uint64_t v5);
static uint64_t f24(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f25(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f26(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f27(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f28(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f29(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4);
static uint64_t f30(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4);
static uint64_t f31(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f32(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f33(uint64_t v0);
static uint64_t f34(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f35(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f36(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f37(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f38(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f39(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f40(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f41(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f42(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f43(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f44(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4);
static uint64_t f45(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f46(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f47(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f48(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f49(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f50(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f51(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f52(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f53(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f54(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n);
static uint64_t f55(const void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n);
static uint64_t f0(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f1(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=256ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(65535ULL);
}
{uint64_t _i=({uint64_t _a=64ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(v2);
}
static uint64_t f2(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=65535ULL;o_op(6, 5, _a,_b);})){
return o_leave(18446744073709551615ULL);
}
return o_leave(({uint64_t _a=v0;uint64_t _b=1023ULL;o_op(12, 5, _a,_b);}));
}
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=14ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=17ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=20ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f4(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v5=({uint64_t p0=6ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v4;f3(p0, p1, p2, p3);});
uint64_t __attribute__((unused)) v6=0ULL;
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=0ULL;uint64_t p2=1ULL;uint64_t p3=v4;f3(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
v6=3ULL;
}
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=2ULL;uint64_t p2=7ULL;uint64_t p3=v4;f3(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
v6=4ULL;
}
if(({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=7ULL;uint64_t p2=1ULL;uint64_t p3=v4;f3(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
v6=2ULL;
}
if(({uint64_t _a=v3;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t p0=14ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v4;f3(p0, p1, p2, p3);});
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=5ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=6ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v5=({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v4;f3(p0, p1, p2, p3);});
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v6=v4;
}
}
if(({uint64_t _a=v3;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=({uint64_t p0=1ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v4;f3(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
}
if(({uint64_t _a=v3;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v4;f3(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
}
if(({uint64_t _a=v3;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});
}
{uint64_t _i=3ULL;uint64_t _v=v6;o_put(v0, v0_n, _i,_v,5);}
return o_leave(v5);
}
static uint64_t f5(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
}
uint64_t __attribute__((unused)) v6=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t p1=v2;uint64_t p2=v4;uint64_t p3=v5;f4(v1, v1_n, p1, p2, p3);});
{uint64_t _i=v6;uint64_t _v=({uint64_t _a=v2;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f2(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f2(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f2(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(16, 5, _a,_b);});f2(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=18446744073709551615ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=7ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f6(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=0ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f7(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=v2;uint64_t p3=({uint64_t p0=v3;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=v4;uint64_t p5=1ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f8(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p2=65543ULL;uint64_t p3=({uint64_t p0=0ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=856ULL;uint64_t p5=2ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=18446744073709551615ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f9(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=1ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f10(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=12ULL;uint64_t p3=v2;uint64_t p4=v3;uint64_t p5=3ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f11(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=28ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;uint64_t p5=4ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f12(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=v5;f0(p0, p1, p2, p3);});uint64_t p4=v4;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f13(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=9ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=v5;f0(p0, p1, p2, p3);});uint64_t p4=v4;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f14(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=v2;uint64_t p3=({uint64_t p0=v3;uint64_t p1=v4;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=v5;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f15(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=v4;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=192ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f16(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=v2;uint64_t p3=v4;uint64_t p4=4294967295ULL;uint64_t p5=v3;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=v2;uint64_t p4=v2;uint64_t p5=31ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f17(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p2=45ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;uint64_t p5=6ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=46ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=4433230883192832ULL;uint64_t p5=9ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f18(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{
uint64_t v3=0;
uint64_t k1=42ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
uint64_t __attribute__((unused)) v4=1ULL;
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=18ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=26ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=30ULL;o_op(8, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);})){
v4=2ULL;
}
{uint64_t _i=v3;uint64_t _v=({uint64_t p1=v4;f1(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
}
}
({f8(v0, v0_n, v1, v1_n);});
{
uint64_t v3=0;
uint64_t k1=7ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
uint64_t __attribute__((unused)) v4=4ULL;
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(8, 5, _a,_b);})){
v4=5ULL;
}
uint64_t __attribute__((unused)) v5=({uint64_t _a=912ULL;uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(2, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(8, 5, _a,_b);})){
v5=({uint64_t _a=896ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
}
({uint64_t p2=v4;uint64_t p3=o_get(v2, v2_n, v3, 5);uint64_t p4=v5;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
{
uint64_t v3=0;
uint64_t k1=3ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
uint64_t __attribute__((unused)) v4=33ULL;
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v4=({uint64_t _a=36ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
}
({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=v3;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p4=v4;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
({uint64_t p2=2ULL;uint64_t p3=o_get(v2, v2_n, 10ULL, 5);uint64_t p4=65535ULL;uint64_t p5=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=o_get(v2, v2_n, 11ULL, 5);uint64_t p4=o_get(v2, v2_n, 7ULL, 5);uint64_t p5=4ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=o_get(v2, v2_n, 12ULL, 5);uint64_t p4=o_get(v2, v2_n, 7ULL, 5);uint64_t p5=2ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 12ULL, 5);uint64_t p4=o_get(v2, v2_n, 12ULL, 5);uint64_t p5=3ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 13ULL, 5);uint64_t p4=o_get(v2, v2_n, 7ULL, 5);uint64_t p5=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=o_get(v2, v2_n, 14ULL, 5);uint64_t p4=o_get(v2, v2_n, 7ULL, 5);uint64_t p5=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 14ULL, 5);uint64_t p4=o_get(v2, v2_n, 14ULL, 5);uint64_t p5=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f19(const void *v0, uint64_t v0_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(32ULL);
}
return o_leave(16ULL);
}
static uint64_t f20(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(o_get(v0, v0_n, 2ULL, 5));
}
return o_leave(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);}));
}
static uint64_t f21(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);}));
}
return o_leave(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=16ULL;o_op(3, 5, _a,_b);}));
}
static uint64_t f22(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4){
o_enter();
({uint64_t p2=o_get(v2, v2_n, 15ULL, 5);uint64_t p3=o_get(v2, v2_n, 9ULL, 5);uint64_t p4=({uint64_t p1=v4;f21(v3, v3_n, p1);});uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=o_get(v2, v2_n, 15ULL, 5);uint64_t p3=o_get(v2, v2_n, 13ULL, 5);uint64_t p4=8ULL;uint64_t p5=o_get(v2, v2_n, 15ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=o_get(v2, v2_n, 8ULL, 5);uint64_t p4=({uint64_t _a=({f19(v3, v3_n);});uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=o_get(v2, v2_n, 12ULL, 5);uint64_t p4=({uint64_t _a=({f19(v3, v3_n);});uint64_t _b=8ULL;o_op(4, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 16ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=o_get(v2, v2_n, 14ULL, 5);uint64_t p4=({uint64_t _a=({f19(v3, v3_n);});uint64_t _b=2ULL;o_op(4, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 16ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}else{
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=o_get(v2, v2_n, 14ULL, 5);uint64_t p4=({f19(v3, v3_n);});uint64_t p5=o_get(v2, v2_n, 16ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=o_get(v2, v2_n, 13ULL, 5);uint64_t p4=({uint64_t _a=({f19(v3, v3_n);});uint64_t _b=2ULL;o_op(4, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 16ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 17ULL, 5);uint64_t p1=o_get(v2, v2_n, 9ULL, 5);uint64_t p2=o_get(v2, v2_n, 6ULL, 5);uint64_t p3=o_get(v2, v2_n, 8ULL, 5);f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 17ULL, 5);uint64_t p3=o_get(v2, v2_n, 17ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t p1=v4;f20(v3, v3_n, p1);});uint64_t _b=128ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 7ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 18ULL, 5);uint64_t p3=o_get(v2, v2_n, 17ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 1ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f23(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=o_get(v2, v2_n, 22ULL, 5);uint64_t p3=o_get(v2, v2_n, 10ULL, 5);uint64_t p4=({uint64_t _a=v4;uint64_t _b=v5;o_op(1, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 16ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=o_get(v2, v2_n, 3ULL, 5);f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=o_get(v2, v2_n, 20ULL, 5);f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 24ULL, 5);uint64_t p1=o_get(v2, v2_n, 22ULL, 5);uint64_t p2=o_get(v2, v2_n, 5ULL, 5);uint64_t p3=o_get(v2, v2_n, 21ULL, 5);f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 24ULL, 5);uint64_t p1=o_get(v2, v2_n, 24ULL, 5);uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=34ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=o_get(v3, v3_n, 0ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=38ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t p4=o_get(v2, v2_n, 24ULL, 5);uint64_t p5=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 24ULL, 5);uint64_t p4=o_get(v2, v2_n, 24ULL, 5);uint64_t p5=4294967294ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=26ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, 24ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}else{
({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=26ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, 24ULL, 5);uint64_t p4=2ULL;uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
return o_leave(0);
}
static uint64_t f24(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=30ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p5=255ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=30ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p5=8ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=o_get(v2, v2_n, 23ULL, 5);uint64_t p5=255ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 23ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=30ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, 23ULL, 5);uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=38ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f25(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, ({uint64_t _a=30ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, ({uint64_t _a=34ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
({uint64_t p2=v4;uint64_t p3=v4;uint64_t p4=v5;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f26(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, ({uint64_t _a=30ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v6=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=v4;uint64_t _b=({uint64_t _a=256ULL;uint64_t _b=v3;o_op(4, 5, _a,_b);});o_op(3, 5, _a,_b);});o_op(15, 5, _a,_b);});
({uint64_t p2=o_get(v2, v2_n, 25ULL, 5);uint64_t p3=v5;uint64_t p4=v6;uint64_t p5=o_get(v2, v2_n, 25ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f27(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
{
uint64_t v5=0;
uint64_t k1=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(4, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p2=19ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=30ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, ({uint64_t _a=26ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f9(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
{
uint64_t v5=0;
uint64_t k1=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(4, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=v3;uint64_t _b=32ULL;o_op(6, 5, _a,_b);})){
({uint64_t p3=v5;f24(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
}
({uint64_t p3=v5;f25(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
}
}
({uint64_t p2=o_get(v2, v2_n, 25ULL, 5);uint64_t p3=o_get(v2, v2_n, 30ULL, 5);uint64_t p4=1ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{
uint64_t v5=0;
uint64_t k1=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(4, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p3=v3;uint64_t p4=v5;f26(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
}
}
}
({uint64_t p2=({uint64_t p0=65535ULL;uint64_t p1=o_get(v2, v2_n, 18ULL, 5);uint64_t p2=o_get(v2, v2_n, 25ULL, 5);uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p3=({uint64_t _a=v4;uint64_t _b=512ULL;o_op(3, 5, _a,_b);});f10(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
static uint64_t f28(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
({uint64_t p2=o_get(v2, v2_n, 19ULL, 5);uint64_t p3=o_get(v2, v2_n, 10ULL, 5);uint64_t p4=o_get(v2, v2_n, 101ULL, 5);uint64_t p5=o_get(v2, v2_n, 15ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=o_get(v2, v2_n, 22ULL, 5);uint64_t p3=o_get(v2, v2_n, 10ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 2ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=o_get(v2, v2_n, 19ULL, 5);uint64_t p4=o_get(v2, v2_n, 22ULL, 5);f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
}else{
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=o_get(v2, v2_n, 19ULL, 5);uint64_t p4=o_get(v2, v2_n, 2ULL, 5);f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 21ULL, 5);uint64_t p1=o_get(v2, v2_n, 19ULL, 5);uint64_t p2=o_get(v2, v2_n, 4ULL, 5);uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f29(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4){
o_enter();
({uint64_t p3=o_get(v3, v3_n, 6ULL, 5);f28(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
{
uint64_t v5=0;
uint64_t k1=({uint64_t _a=({f19(v3, v3_n);});uint64_t _b=8ULL;o_op(4, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p4=o_get(v2, v2_n, 102ULL, 5);uint64_t p5=v5;f23(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n, p4, p5);});
}
}
({uint64_t p3=({f19(v3, v3_n);});uint64_t p4=v4;f27(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
return o_leave(0);
}
static uint64_t f30(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4){
o_enter();
({f18(v0, v0_n, v1, v1_n, v2, v2_n);});
({uint64_t p4=v4;f22(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n, p4);});
{uint64_t _i=100ULL;uint64_t _v=({uint64_t p1=v4;f20(v3, v3_n, p1);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v3, v3_n, 6ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v5=4294967280ULL;
if(({uint64_t _a=o_get(v3, v3_n, 0ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v5=4294967264ULL;
}
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 3ULL, 5);uint64_t p4=o_get(v2, v2_n, 3ULL, 5);uint64_t p5=v5;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
{
uint64_t v5=0;
uint64_t k1=o_get(v2, v2_n, 100ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=101ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=o_get(v3, v3_n, 1ULL, 5);o_op(5, 5, _a,_b);});uint64_t _b=16ULL;o_op(3, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=102ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=o_get(v3, v3_n, 1ULL, 5);o_op(4, 5, _a,_b);});uint64_t _b=({f19(v3, v3_n);});o_op(3, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=101ULL;uint64_t _v=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=102ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
}
({uint64_t p4=v5;f29(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n, p4);});
}
}
({f17(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f31(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=28ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=1ULL;
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=3ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=26ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v5=2ULL;
}
{uint64_t _i=v4;uint64_t _v=({uint64_t p1=v5;f1(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
}
}
{
uint64_t v4=0;
uint64_t k1=({uint64_t _a=o_get(v3, v3_n, 1ULL, 5);uint64_t _b=o_get(v3, v3_n, 2ULL, 5);o_op(3, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=({uint64_t _a=32ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=4ULL;f1(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
}
}
{
uint64_t v4=0;
uint64_t k1=({uint64_t _a=2ULL;uint64_t _b=o_get(v3, v3_n, 1ULL, 5);o_op(3, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=({uint64_t _a=48ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=4ULL;f1(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
}
}
{
uint64_t v4=0;
uint64_t k1=o_get(v3, v3_n, 2ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=({uint64_t _a=64ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=4ULL;f1(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
}
}
({f8(v0, v0_n, v1, v1_n);});
{
uint64_t v4=0;
uint64_t k1=6ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=4ULL;
uint64_t __attribute__((unused)) v6=({uint64_t _a=920ULL;uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=3ULL;o_op(2, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=3ULL;o_op(8, 5, _a,_b);})){
v5=5ULL;
v6=({uint64_t _a=896ULL;uint64_t _b=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
}
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, v4, 5);uint64_t p4=v6;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
{
uint64_t v4=0;
uint64_t k1=3ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=33ULL;
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v5=({uint64_t _a=36ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
}
({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=6ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);uint64_t p4=v5;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
({uint64_t p2=2ULL;uint64_t p3=o_get(v2, v2_n, 13ULL, 5);uint64_t p4=65535ULL;uint64_t p5=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=1ULL;uint64_t p3=o_get(v2, v2_n, 14ULL, 5);uint64_t p4=65535ULL;uint64_t p5=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f32(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
{
uint64_t v3=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t p2=1ULL;uint64_t p3=v2;uint64_t p4=65535ULL;uint64_t p5=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
}
}
return o_leave(0);
}
static uint64_t f33(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=0ULL;
{
uint64_t v2=0;
uint64_t k1=6ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
if(({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v1;o_op(15, 5, _a,_b);});uint64_t _b=v0;o_op(8, 5, _a,_b);})){
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
}
return o_leave(v1);
}
static uint64_t f34(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
if(({uint64_t _a=o_get(v3, v3_n, 7ULL, 5);uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
({uint64_t p2=4ULL;uint64_t p3=o_get(v2, v2_n, 27ULL, 5);uint64_t p4=932ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 17ULL, 5);uint64_t p4=o_get(v2, v2_n, 7ULL, 5);uint64_t p5=({uint64_t _a=o_get(v3, v3_n, 7ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=o_get(v2, v2_n, 8ULL, 5);uint64_t p4=o_get(v3, v3_n, 7ULL, 5);uint64_t p5=o_get(v2, v2_n, 17ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=o_get(v2, v2_n, 8ULL, 5);uint64_t p4=o_get(v2, v2_n, 7ULL, 5);uint64_t p5=({uint64_t p0=o_get(v3, v3_n, 7ULL, 5);f33(p0);});f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=o_get(v2, v2_n, 27ULL, 5);f11(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=131117ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;uint64_t p5=6ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 7ULL, 5);uint64_t p3=o_get(v2, v2_n, 16ULL, 5);uint64_t p4=1ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=17ULL;uint64_t p3=o_get(v2, v2_n, 9ULL, 5);uint64_t p4=o_get(v2, v2_n, 6ULL, 5);uint64_t p5=5ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 10ULL, 5);uint64_t p4=o_get(v2, v2_n, 6ULL, 5);uint64_t p5=31ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=o_get(v2, v2_n, 17ULL, 5);uint64_t p4=o_get(v2, v2_n, 9ULL, 5);uint64_t p5=({uint64_t p0=o_get(v3, v3_n, 4ULL, 5);f33(p0);});f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 15ULL, 5);uint64_t p3=o_get(v2, v2_n, 8ULL, 5);uint64_t p4=o_get(v3, v3_n, 3ULL, 5);uint64_t p5=o_get(v2, v2_n, 17ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 17ULL, 5);uint64_t p4=o_get(v2, v2_n, 9ULL, 5);uint64_t p5=({uint64_t _a=o_get(v3, v3_n, 4ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=o_get(v2, v2_n, 7ULL, 5);uint64_t p4=o_get(v3, v3_n, 4ULL, 5);uint64_t p5=o_get(v2, v2_n, 17ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 17ULL, 5);uint64_t p1=o_get(v2, v2_n, 15ULL, 5);uint64_t p2=o_get(v2, v2_n, 5ULL, 5);uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 0ULL, 5);uint64_t p3=o_get(v2, v2_n, 17ULL, 5);uint64_t p4=({uint64_t _a=o_get(v3, v3_n, 1ULL, 5);uint64_t _b=1024ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 17ULL, 5);uint64_t p1=o_get(v2, v2_n, 16ULL, 5);uint64_t p2=o_get(v2, v2_n, 5ULL, 5);uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 1ULL, 5);uint64_t p3=o_get(v2, v2_n, 17ULL, 5);uint64_t p4=({uint64_t _a=o_get(v3, v3_n, 2ULL, 5);uint64_t _b=512ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 1ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 17ULL, 5);uint64_t p3=o_get(v2, v2_n, 10ULL, 5);uint64_t p4=16ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 0ULL, 5);uint64_t p3=o_get(v2, v2_n, 17ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 1ULL, 5);uint64_t p3=o_get(v2, v2_n, 17ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 1ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{
uint64_t v4=0;
uint64_t k1=({uint64_t _a=o_get(v3, v3_n, 1ULL, 5);uint64_t _b=o_get(v3, v3_n, 2ULL, 5);o_op(3, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=32ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);f32(v0, v0_n, v1, v1_n, p2);});
}
}
({uint64_t p2=o_get(v2, v2_n, 18ULL, 5);uint64_t p3=o_get(v2, v2_n, 5ULL, 5);uint64_t p4=1ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f35(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=({uint64_t _a=o_get(v3, v3_n, 1ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p2=57ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=48ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, 0ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=v4;uint64_t _b=512ULL;o_op(3, 5, _a,_b);});f9(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
{
uint64_t v4=0;
uint64_t k1=o_get(v3, v3_n, 2ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p2=57ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=64ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, 1ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=v4;uint64_t _b=512ULL;o_op(3, 5, _a,_b);});f9(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
({uint64_t p2=o_get(v2, v2_n, 0ULL, 5);uint64_t p3=o_get(v2, v2_n, 13ULL, 5);uint64_t p4=({uint64_t _a=o_get(v3, v3_n, 1ULL, 5);uint64_t _b=1024ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 1ULL, 5);uint64_t p3=o_get(v2, v2_n, 13ULL, 5);uint64_t p4=({uint64_t _a=o_get(v3, v3_n, 2ULL, 5);uint64_t _b=512ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 1ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f36(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
{uint64_t _i=113ULL;uint64_t _v=o_get(v2, v2_n, ({uint64_t _a=32ULL;uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 111ULL, 5);uint64_t _b=o_get(v3, v3_n, 2ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=o_get(v2, v2_n, 112ULL, 5);o_op(1, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=114ULL;uint64_t _v=o_get(v2, v2_n, ({uint64_t _a=48ULL;uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 110ULL, 5);uint64_t _b=o_get(v3, v3_n, 1ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=o_get(v2, v2_n, 111ULL, 5);o_op(1, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=115ULL;uint64_t _v=o_get(v2, v2_n, ({uint64_t _a=64ULL;uint64_t _b=o_get(v2, v2_n, 112ULL, 5);o_op(1, 5, _a,_b);}), 5);o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=116ULL;uint64_t _v=16ULL;o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v3, v3_n, 0ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=116ULL;uint64_t _v=15ULL;o_put(v2, v2_n, _i,_v,5);}
}
{uint64_t _i=117ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v3, v3_n, 0ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=117ULL;uint64_t _v=1362114610ULL;o_put(v2, v2_n, _i,_v,5);}
}
({uint64_t p2=o_get(v2, v2_n, 116ULL, 5);uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 113ULL, 5);uint64_t p1=o_get(v2, v2_n, 114ULL, 5);uint64_t p2=o_get(v2, v2_n, 115ULL, 5);uint64_t p3=o_get(v2, v2_n, 113ULL, 5);f0(p0, p1, p2, p3);});uint64_t p4=o_get(v2, v2_n, 117ULL, 5);uint64_t p5=0ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=118ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=o_get(v2, v2_n, 118ULL, 5);uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 110ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v4=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=o_get(v2, v2_n, 118ULL, 5);uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=32ULL;o_op(16, 5, _a,_b);});
v4=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=4294959615ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v3, v3_n, 5ULL, 5);uint64_t _b=9ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=o_get(v2, v2_n, 118ULL, 5);uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 117ULL, 5);uint64_t _b=({uint64_t _a=v4;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f37(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=o_get(v3, v3_n, 2ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=112ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
({f36(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
}
}
return o_leave(0);
}
static uint64_t f38(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=o_get(v3, v3_n, 1ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=111ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
({f37(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
}
}
return o_leave(0);
}
static uint64_t f39(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=2ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
{uint64_t _i=110ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
({f38(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
}
}
return o_leave(0);
}
static uint64_t f40(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p2=0ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=({uint64_t p0=15ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=o_get(v1, v1_n, 3ULL, 5);f3(p0, p1, p2, p3);});uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t p5=9ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f41(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=v4;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v4, 5);uint64_t _b=18446744069414584320ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=v2;o_op(2, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
v4=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=v4;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v4, 5);uint64_t _b=18446744069414584320ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=v1;o_op(2, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f42(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=o_get(v2, v2_n, 24ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=o_get(v2, v2_n, 4ULL, 5);f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 24ULL, 5);uint64_t p3=o_get(v2, v2_n, 24ULL, 5);uint64_t p4=o_get(v2, v2_n, 23ULL, 5);f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 14ULL, 5);uint64_t p3=o_get(v2, v2_n, 24ULL, 5);f11(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=54ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=2ULL;uint64_t p5=7ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=131118ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=1ULL;uint64_t p5=5ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=14ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 17ULL, 5);uint64_t p1=o_get(v2, v2_n, 25ULL, 5);uint64_t p2=o_get(v2, v2_n, 22ULL, 5);uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 26ULL, 5);uint64_t p3=o_get(v2, v2_n, 17ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 2ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=({uint64_t p0=65535ULL;uint64_t p1=o_get(v2, v2_n, 26ULL, 5);uint64_t p2=v3;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p3=0ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
{uint64_t _i=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v4;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=55ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;uint64_t p5=8ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p1=v5;uint64_t p2=v6;uint64_t p3=v7;f41(v0, v0_n, p1, p2, p3);});
return o_leave(0);
}
static uint64_t f43(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
{uint64_t _i=108ULL;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=32ULL;uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 104ULL, 5);uint64_t _b=v1;o_op(3, 5, _a,_b);});uint64_t _b=o_get(v0, v0_n, 106ULL, 5);o_op(1, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f44(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4){
o_enter();
({uint64_t p2=o_get(v2, v2_n, 22ULL, 5);uint64_t p3=o_get(v2, v2_n, 13ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 106ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 20ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=109ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 105ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=v4;o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v3, v3_n, 6ULL, 5);uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=109ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 105ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=1ULL;uint64_t _b=v4;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
}
({uint64_t p1=o_get(v3, v3_n, 2ULL, 5);f43(v2, v2_n, p1);});
({uint64_t p3=o_get(v2, v2_n, 108ULL, 5);uint64_t p4=o_get(v2, v2_n, 109ULL, 5);f42(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
return o_leave(0);
}
static uint64_t f45(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 104ULL, 5);uint64_t _b=16ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 105ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
({uint64_t p2=o_get(v2, v2_n, 21ULL, 5);uint64_t p3=o_get(v2, v2_n, 13ULL, 5);uint64_t p4=v3;uint64_t p5=o_get(v2, v2_n, 19ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=o_get(v2, v2_n, 3ULL, 5);f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 25ULL, 5);uint64_t p1=o_get(v2, v2_n, 21ULL, 5);uint64_t p2=o_get(v2, v2_n, 4ULL, 5);uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f46(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
({f45(v0, v0_n, v1, v1_n, v2, v2_n);});
{
uint64_t v4=0;
uint64_t k1=o_get(v3, v3_n, 2ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=106ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
({uint64_t p4=0ULL;f44(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n, p4);});
({uint64_t p4=1ULL;f44(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n, p4);});
}
}
return o_leave(0);
}
static uint64_t f47(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=2ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
{uint64_t _i=105ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
({f46(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
}
}
return o_leave(0);
}
static uint64_t f48(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
({uint64_t p2=17ULL;uint64_t p3=o_get(v2, v2_n, 11ULL, 5);uint64_t p4=o_get(v2, v2_n, 10ULL, 5);uint64_t p5=2ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=18ULL;uint64_t p3=o_get(v2, v2_n, 12ULL, 5);uint64_t p4=o_get(v2, v2_n, 10ULL, 5);uint64_t p5=3ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 19ULL, 5);uint64_t p3=o_get(v2, v2_n, 15ULL, 5);uint64_t p4=({uint64_t _a=o_get(v3, v3_n, 1ULL, 5);uint64_t _b=16ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 17ULL, 5);uint64_t p3=o_get(v2, v2_n, 12ULL, 5);uint64_t p4=2ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=o_get(v2, v2_n, 16ULL, 5);uint64_t p4=({uint64_t _a=o_get(v3, v3_n, 2ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 17ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{
uint64_t v4=0;
uint64_t k1=o_get(v3, v3_n, 1ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=104ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
({f47(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
}
}
({f17(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f49(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
({f31(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
({f34(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 0ULL, 5);
({f35(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
({f39(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
({f40(v0, v0_n, v1, v1_n);});
({uint64_t p2=o_get(v2, v2_n, 18ULL, 5);uint64_t p3=o_get(v2, v2_n, 13ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 18ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 18ULL, 5);uint64_t p3=o_get(v2, v2_n, 13ULL, 5);f11(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=131118ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t p5=5ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=5ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=16ULL;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=17ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{
uint64_t v5=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
({f40(v0, v0_n, v1, v1_n);});
}
}
({f48(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n);});
return o_leave(0);
}
static uint64_t f50(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{
uint64_t v3=0;
uint64_t k1=48ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
uint64_t __attribute__((unused)) v4=1ULL;
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=19ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=30ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=31ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
v4=2ULL;
}
{uint64_t _i=v3;uint64_t _v=({uint64_t p1=v4;f1(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
}
}
{uint64_t _i=2ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=320ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
({f8(v0, v0_n, v1, v1_n);});
{
uint64_t v3=0;
uint64_t k1=3ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t p2=5ULL;uint64_t p3=o_get(v2, v2_n, v3, 5);uint64_t p4=({uint64_t _a=896ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 6ULL, 5);uint64_t p4=33ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 7ULL, 5);uint64_t p4=37ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 8ULL, 5);uint64_t p4=38ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=2ULL;uint64_t p3=o_get(v2, v2_n, 9ULL, 5);uint64_t p4=65535ULL;uint64_t p5=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=1ULL;uint64_t p3=o_get(v2, v2_n, 10ULL, 5);uint64_t p4=65535ULL;uint64_t p5=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=1ULL;uint64_t p3=o_get(v2, v2_n, 15ULL, 5);uint64_t p4=65535ULL;uint64_t p5=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f51(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=11ULL;
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v5=19ULL;
}
({uint64_t p2=v5;uint64_t p3=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f9(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=v2;uint64_t p3=v2;uint64_t p4=65536ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
if(({uint64_t _a=v4;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=66ULL;uint64_t p3=v2;uint64_t p4=v2;uint64_t p5=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
return o_leave(0);
}
static uint64_t f52(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=o_get(v2, v2_n, 18ULL, 5);uint64_t p3=o_get(v2, v2_n, 5ULL, 5);uint64_t p4=1ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=o_get(v2, v2_n, 0ULL, 5);uint64_t p4=v3;f51(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 17ULL, 5);uint64_t p3=o_get(v2, v2_n, 1ULL, 5);uint64_t p4=v3;f51(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=o_get(v2, v2_n, 16ULL, 5);uint64_t p4=o_get(v2, v2_n, 22ULL, 5);f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v6=33ULL;
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v6=30ULL;
}
({uint64_t p2=v6;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 15ULL, 5);uint64_t p1=o_get(v2, v2_n, 16ULL, 5);uint64_t p2=o_get(v2, v2_n, 17ULL, 5);uint64_t p3=o_get(v2, v2_n, 15ULL, 5);f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 0ULL, 5);uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 1ULL, 5);uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 1ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 18ULL, 5);uint64_t p3=o_get(v2, v2_n, 9ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 18ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 18ULL, 5);uint64_t p3=o_get(v2, v2_n, 9ULL, 5);f11(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=131118ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t p5=5ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=({uint64_t _a=16ULL;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=v5;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=17ULL;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f53(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
({uint64_t p2=o_get(v2, v2_n, 10ULL, 5);uint64_t p3=o_get(v2, v2_n, 12ULL, 5);f11(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=54ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=2ULL;uint64_t p5=7ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=131118ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=1ULL;uint64_t p5=5ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=({uint64_t p0=65535ULL;uint64_t p1=o_get(v2, v2_n, 19ULL, 5);uint64_t p2=o_get(v2, v2_n, 15ULL, 5);uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p3=0ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=55ULL;uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;uint64_t p5=8ULL;f5(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p1=v3;uint64_t p2=v4;uint64_t p3=v5;f41(v0, v0_n, p1, p2, p3);});
({f17(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f54(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n){
o_enter();
uint64_t v6[128]={0};
uint64_t v6_n=128;
o_fill(v6, v6_n, 0ULL, 5);
{
uint64_t v7=0;
uint64_t k1=512ULL;
for(;o_op(8,5, v7, k1);v7=o_op(1,5, v7,1)){
{uint64_t _i=v7;uint64_t _v=0ULL;o_put(v5, v5_n, _i,_v,5);}
}
}
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
({f50(v4, v4_n, v5, v5_n, v6, v6_n);});
{
uint64_t v7=0;
uint64_t k1=3ULL;
for(;o_op(8,5, v7, k1);v7=o_op(1,5, v7,1)){
({uint64_t p2=4ULL;uint64_t p3=o_get(v6, v6_n, ({uint64_t _a=v7;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p4=({uint64_t _a=920ULL;uint64_t _b=({uint64_t _a=v7;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f7(v4, v4_n, v5, v5_n, p2, p3, p4);});
}
}
({uint64_t p2=o_get(v6, v6_n, 11ULL, 5);uint64_t p3=o_get(v6, v6_n, 7ULL, 5);uint64_t p4=128ULL;uint64_t p5=o_get(v6, v6_n, 6ULL, 5);f12(v4, v4_n, v5, v5_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v6, v6_n, 12ULL, 5);uint64_t p3=o_get(v6, v6_n, 11ULL, 5);uint64_t p4=o_get(v6, v6_n, 4ULL, 5);f16(v4, v4_n, v5, v5_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v6, v6_n, 13ULL, 5);uint64_t p1=o_get(v6, v6_n, 11ULL, 5);uint64_t p2=o_get(v6, v6_n, 12ULL, 5);uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v4, v4_n, v5, v5_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v6, v6_n, 14ULL, 5);uint64_t p1=o_get(v6, v6_n, 8ULL, 5);uint64_t p2=o_get(v6, v6_n, 5ULL, 5);uint64_t p3=65535ULL;f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v4, v4_n, v5, v5_n, p2, p3, p4);});
({uint64_t p2=o_get(v6, v6_n, 0ULL, 5);uint64_t p3=o_get(v6, v6_n, 14ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v6, v6_n, 0ULL, 5);f13(v4, v4_n, v5, v5_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v6, v6_n, 1ULL, 5);uint64_t p3=o_get(v6, v6_n, 13ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v6, v6_n, 1ULL, 5);f13(v4, v4_n, v5, v5_n, p2, p3, p4, p5);});
({uint64_t p2=2ULL;uint64_t p3=o_get(v6, v6_n, 20ULL, 5);uint64_t p4=65535ULL;uint64_t p5=4ULL;f14(v4, v4_n, v5, v5_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v6, v6_n, 21ULL, 5);uint64_t p3=o_get(v6, v6_n, 4ULL, 5);uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(v4, v4_n, v5, v5_n, p2, p3, p4, p5);});
({uint64_t p2=2ULL;uint64_t p3=o_get(v6, v6_n, 22ULL, 5);uint64_t p4=65535ULL;uint64_t p5=4294967295ULL;f14(v4, v4_n, v5, v5_n, p2, p3, p4, p5);});
({uint64_t p3=0ULL;uint64_t p4=v1;f52(v4, v4_n, v5, v5_n, v6, v6_n, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v6, v6_n, 14ULL, 5);uint64_t p1=o_get(v6, v6_n, 8ULL, 5);uint64_t p2=o_get(v6, v6_n, 4ULL, 5);uint64_t p3=o_get(v6, v6_n, 11ULL, 5);f0(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v4, v4_n, v5, v5_n, p2, p3, p4);});
({uint64_t p2=o_get(v6, v6_n, 19ULL, 5);uint64_t p3=o_get(v6, v6_n, 14ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v6, v6_n, 2ULL, 5);f13(v4, v4_n, v5, v5_n, p2, p3, p4, p5);});
({f53(v4, v4_n, v5, v5_n, v6, v6_n);});
return o_leave(o_get(v5, v5_n, 4ULL, 5));
}
static uint64_t f55(const void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n){
o_enter();
uint64_t v4[128]={0};
uint64_t v4_n=128;
o_fill(v4, v4_n, 0ULL, 5);
{
uint64_t v5=0;
uint64_t k1=512ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
{uint64_t _i=v5;uint64_t _v=0ULL;o_put(v3, v3_n, _i,_v,5);}
}
}
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=2ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
{uint64_t _i=2ULL;uint64_t _v=1ULL;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=320ULL;uint64_t _v=2ULL;o_put(v3, v3_n, _i,_v,5);}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({f49(v2, v2_n, v3, v3_n, v4, v4_n, v0, v0_n);});
}else{
({uint64_t p4=v1;f30(v2, v2_n, v3, v3_n, v4, v4_n, v0, v0_n, p4);});
}
return o_leave(o_get(v3, v3_n, 4ULL, 5));
}
uint32_t omega_gemm_any_ir(uint64_t  *result, const void *p0, uint64_t p0_n, volatile uint64_t p1, void *p2, uint64_t p2_n, void *p3, uint64_t p3_n){
  if(!o_buf(result,1,1,5))return 32;
if(!o_buf(p0, p0_n,8, 5))return 32;
  if(o_overlap(result,8,p0, p0_n*8))return 32;
if(!o_valid(5, p1))return 32;
if(!o_buf(p2, p2_n,8192, 5))return 32;
  if(o_overlap(result,8,p2, p2_n*8))return 32;
if(o_overlap(p2, p2_n*8, p0, p0_n*8))return 32;
if(!o_buf(p3, p3_n,512, 5))return 32;
  if(o_overlap(result,8,p3, p3_n*8))return 32;
if(o_overlap(p3, p3_n*8, p0, p0_n*8))return 32;
if(o_overlap(p3, p3_n*8, p2, p2_n*8))return 32;
  if(o_busy)return 33;
  o_busy=1;o_depth=0;int code=setjmp(o_jmp);if(!code){uint64_t _result=f55(p0, p0_n, p1, p2, p2_n, p3, p3_n);o_put(result,1,0,_result,5);}
  o_busy=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
