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
static uint64_t f1(uint64_t v0);
static uint64_t f2(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f4(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f5(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f6(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f7(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f8(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f9(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f10(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f11(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f12(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f13(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f14(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f15(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f16(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f17(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f18(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f19(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f20(void *v0, uint64_t v0_n);
static uint64_t f21(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f22(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f23(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f24(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f25(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f26(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f27(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f28(void *v0, uint64_t v0_n);
static uint64_t f29(void *v0, uint64_t v0_n);
static uint64_t f30(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f31(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f32(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f33(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f34(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f35(uint64_t v0);
static uint64_t f36(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f37(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f38(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f39(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f40(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f41(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f42(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f43(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f44(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f45(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f46(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f47(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f48(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f49(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f50(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f51(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f52(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f53(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f54(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f55(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f56(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f57(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f58(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n);
static uint64_t f59(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n);
static uint64_t f60(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f61(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n);
static uint64_t f0(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=14ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=17ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=20ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f1(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=65535ULL;o_op(6, 5, _a,_b);})){
return o_leave(18446744073709551615ULL);
}
return o_leave(v0);
}
static uint64_t f2(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=40ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f4(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=14ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v2;f0(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=0ULL;uint64_t p2=7ULL;uint64_t p3=v2;f0(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=1ULL;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=2ULL;uint64_t p2=7ULL;uint64_t p3=v2;f0(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=4ULL;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=7ULL;uint64_t p2=1ULL;uint64_t p3=v2;f0(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=2ULL;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v2;f0(p0, p1, p2, p3);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v2;f0(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t p0=1ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v2;f0(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v2;f0(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t p0=6ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v2;f0(p0, p1, p2, p3);}));
}
static uint64_t f5(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
{uint64_t _i=v1;uint64_t _v=({uint64_t _a=v2;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f1(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f1(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f1(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(16, 5, _a,_b);});f1(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=40ULL;o_op(16, 5, _a,_b);});uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f1(p0);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f6(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, 0ULL, 5);
if(({uint64_t _a=v5;uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v6=({uint64_t p0=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t p1=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t p2=o_get(v1, v1_n, 3ULL, 5);f4(p0, p1, p2);});
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
v5=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});
({uint64_t p1=v5;uint64_t p2=v2;uint64_t p3=v3;f5(v0, v0_n, p1, p2, p3);});
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v5;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t _a=v6;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f7(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=({uint64_t p0=v2;uint64_t p1=0ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=v3;uint64_t p4=v4;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f8(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=({uint64_t p0=v2;uint64_t p1=2ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=v3;uint64_t p4=v4;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f9(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=256ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=2ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(65535ULL);
}
{uint64_t _i=({uint64_t _a=64ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(v2);
}
static uint64_t f10(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=2ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=v3;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f11(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({uint64_t p2=1ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f12(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=v5;f2(p0, p1, p2, p3);});uint64_t p4=v4;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f13(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=v4;uint64_t p3=v5;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f14(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=14ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=v4;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f15(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=9ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=v4;f2(p0, p1, p2, p3);});uint64_t p4=4ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f16(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=({uint64_t p0=11ULL;uint64_t p1=2ULL;uint64_t p2=0ULL;uint64_t p3=0ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f17(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=({uint64_t p0=12ULL;uint64_t p1=4ULL;uint64_t p2=0ULL;uint64_t p3=0ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f18(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=4ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=v3;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f19(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=({uint64_t p0=28ULL;uint64_t p1=1ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f20(void *v0, uint64_t v0_n){
o_enter();
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 13ULL, 5);uint64_t _b=0ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 14ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 15ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=3ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f21(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({f20(v1, v1_n);})){
return o_leave(0);
}
({uint64_t p2=42ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=v4;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f22(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({f20(v1, v1_n);})){
return o_leave(0);
}
({uint64_t p2=({uint64_t p0=48ULL;uint64_t p1=5ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f23(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 13ULL, 5);uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=14ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=({uint64_t p0=45ULL;uint64_t p1=6ULL;uint64_t p2=2ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f24(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=({uint64_t p0=46ULL;uint64_t p1=7ULL;uint64_t p2=v3;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f25(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=({uint64_t p0=46ULL;uint64_t p1=7ULL;uint64_t p2=2ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=1ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v2);
}
static uint64_t f26(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(11, 5, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=5ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=v3;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v3, 5);uint64_t _b=({uint64_t _a=4294967295ULL;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=v2;o_op(2, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f27(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p2=({uint64_t p0=45ULL;uint64_t p1=6ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=({uint64_t p0=46ULL;uint64_t p1=7ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=1032192ULL;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f28(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 8ULL, 5);
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=6ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=({uint64_t _a=9ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, 0ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f29(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 8ULL, 5);
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 5ULL, 5);
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(11, 5, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=7ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
{uint64_t _i=8ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=16ULL;uint64_t _b=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=9ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);}), 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=17ULL;uint64_t _b=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f30(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 13ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=16ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=8ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=({uint64_t _a=352ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v1, v1_n, 0ULL, 5);o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=({uint64_t p0=54ULL;uint64_t p1=8ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=2ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v2;uint64_t _b=48ULL;o_op(15, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f31(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=0ULL;
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v3=2ULL;
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v3=6ULL;
}
}
({uint64_t p2=({uint64_t p0=46ULL;uint64_t p1=7ULL;uint64_t p2=v3;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=1ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 13ULL, 5);
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=9ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=7ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=v5;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, v5, 5);uint64_t _b=({uint64_t _a=v4;uint64_t _b=56ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f32(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 13ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=10ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 0ULL, 5);
uint64_t __attribute__((unused)) v4=55ULL;
if(({uint64_t _a=o_get(v1, v1_n, 368ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v4=0ULL;
}
({uint64_t p2=({uint64_t p0=v4;uint64_t p1=9ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v2;uint64_t _b=48ULL;o_op(15, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, ({uint64_t _a=352ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=v5;o_op(2, 5, _a,_b);});
if(({uint64_t _a=o_get(v1, v1_n, 368ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v7=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
{uint64_t _i=v6;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v6, 5);uint64_t _b=({uint64_t _a=4294967295ULL;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=v7;o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
({uint64_t p1=v2;uint64_t p2=v3;f33(v0, v0_n, p1, p2);});
{uint64_t _i=13ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f33(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
{
uint64_t v3=0;
uint64_t c1=0;
for(;o_op(8,5, v3, v2);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=7ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v4, 5);uint64_t _b=56ULL;o_op(16, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_op(6, 5, _a,_b);})){
{uint64_t _i=v4;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, v4, 5);uint64_t _b=({uint64_t _a=({uint64_t _a=4294967295ULL;uint64_t _b=56ULL;o_op(15, 5, _a,_b);});uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
v4=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=v4;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v4, 5);uint64_t _b=({uint64_t _a=4294967295ULL;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=v3;o_op(2, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
}
}
}
return o_leave(0);
}
static uint64_t f34(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
{uint64_t _i=2ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=320ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
{
uint64_t v2=0;
uint64_t k1=5ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
uint64_t __attribute__((unused)) v3=1ULL;
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(8, 5, _a,_b);})){
v3=2ULL;
}
uint64_t __attribute__((unused)) v4=({uint64_t p1=v3;f9(v1, v1_n, p1);});
}
}
({uint64_t p2=({uint64_t p0=7ULL;uint64_t p1=3ULL;uint64_t p2=1ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=0ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=856ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=5ULL;uint64_t p3=({uint64_t p0=0ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=896ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=5ULL;uint64_t p3=({uint64_t p0=1ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=904ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=5ULL;uint64_t p3=({uint64_t p0=2ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=912ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=3ULL;uint64_t p3=({uint64_t p0=3ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=33ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=3ULL;uint64_t p3=({uint64_t p0=4ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=37ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f35(uint64_t v0){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=29ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=51ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=52ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=53ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=30ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=31ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=32ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=33ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
static uint64_t f36(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(8, 5, _a,_b);})){
return o_leave(0);
}
{
uint64_t v3=0;
uint64_t k1=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, v4, 5);
uint64_t __attribute__((unused)) v6=({uint64_t p0=v5;f35(p0);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=32ULL;o_op(16, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=14ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(({uint64_t _a=v6;uint64_t _b=({uint64_t p0=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);f35(p0);});o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=15ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v7;uint64_t _b=9ULL;o_op(16, 5, _a,_b);});uint64_t _b=15ULL;o_op(12, 5, _a,_b);});uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=v4;uint64_t p2=v7;f37(v0, v0_n, p1, p2);});
}
}
}
}
}
}
}
return o_leave(0);
}
static uint64_t f37(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=({uint64_t _a=15ULL;uint64_t _b=9ULL;o_op(15, 5, _a,_b);});uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=5ULL;uint64_t _b=9ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=v3;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v3, 5);uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f38(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
{
uint64_t v2=0;
uint64_t c1=0;
for(;o_op(8,5, v2, v1);v2=o_op(1,5, v2,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v3=({uint64_t p1=1ULL;f9(v0, v0_n, p1);});
}
}
return o_leave(0);
}
static uint64_t f39(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p1=5ULL;f38(v1, v1_n, p1);});
{uint64_t _i=71ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=5ULL;uint64_t p3=920ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=6ULL;uint64_t p3=4ULL;uint64_t p4=128ULL;uint64_t p5=3ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=6ULL;uint64_t p3=5ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({f23(v0, v0_n, v1, v1_n);});
({uint64_t p2=7ULL;uint64_t p3=6ULL;uint64_t p4=0ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=7ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=o_get(v1, v1_n, 369ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p1=1ULL;f38(v1, v1_n, p1);});
({uint64_t p2=10ULL;uint64_t p3=o_cast(5, 4, 1065353216ULL);f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=8ULL;uint64_t p1=8ULL;uint64_t p2=10ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
({uint64_t p2=47ULL;uint64_t p3=({uint64_t p0=9ULL;uint64_t p1=8ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=7ULL;uint64_t p3=6ULL;uint64_t p4=2ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=7ULL;uint64_t p3=9ULL;f17(v0, v0_n, v1, v1_n, p2, p3);});
({f27(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f40(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p1=7ULL;f38(v1, v1_n, p1);});
{uint64_t _i=70ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=5ULL;uint64_t p3=4ULL;uint64_t p4=128ULL;uint64_t p5=3ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=6ULL;uint64_t p3=5ULL;uint64_t p4=0ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=7ULL;uint64_t p3=6ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=8ULL;uint64_t p3=3ULL;uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=({uint64_t p0=50ULL;uint64_t p1=4ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=8ULL;uint64_t p2=7ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({f22(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=o_get(v1, v1_n, 369ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=49ULL;uint64_t p3=({uint64_t p0=11ULL;uint64_t p1=8ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
}else{
({uint64_t p2=10ULL;uint64_t p3=127ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=29ULL;uint64_t p3=({uint64_t p0=9ULL;uint64_t p1=3ULL;uint64_t p2=10ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=9ULL;uint64_t p3=9ULL;uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=49ULL;uint64_t p3=({uint64_t p0=11ULL;uint64_t p1=9ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
({uint64_t p2=6ULL;uint64_t p3=5ULL;uint64_t p4=2ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=6ULL;uint64_t p3=11ULL;f17(v0, v0_n, v1, v1_n, p2, p3);});
({f27(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f41(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p1=14ULL;f38(v1, v1_n, p1);});
{uint64_t _i=73ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=74ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=5ULL;uint64_t p3=920ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=6ULL;uint64_t p3=924ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=7ULL;uint64_t p3=4ULL;uint64_t p4=5ULL;uint64_t p5=3ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=8ULL;uint64_t p3=7ULL;uint64_t p4=6ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=9ULL;uint64_t p3=7ULL;uint64_t p4=0ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=11ULL;uint64_t p3=9ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=10ULL;uint64_t p3=8ULL;uint64_t p4=0ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=12ULL;uint64_t p3=10ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=9ULL;uint64_t p3=7ULL;uint64_t p4=1ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=13ULL;uint64_t p3=9ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=10ULL;uint64_t p3=8ULL;uint64_t p4=1ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=14ULL;uint64_t p3=10ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=15ULL;uint64_t p1=11ULL;uint64_t p2=13ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=16ULL;uint64_t p1=12ULL;uint64_t p2=14ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v2=31ULL;
if(({uint64_t _a=o_get(v1, v1_n, 369ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v2=30ULL;
}
({uint64_t p2=v2;uint64_t p3=({uint64_t p0=17ULL;uint64_t p1=15ULL;uint64_t p2=16ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=15ULL;uint64_t p1=11ULL;uint64_t p2=14ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=16ULL;uint64_t p1=12ULL;uint64_t p2=13ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=18ULL;uint64_t p1=15ULL;uint64_t p2=16ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=9ULL;uint64_t p3=7ULL;uint64_t p4=2ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=9ULL;uint64_t p3=17ULL;f17(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=10ULL;uint64_t p3=8ULL;uint64_t p4=2ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=10ULL;uint64_t p3=18ULL;f17(v0, v0_n, v1, v1_n, p2, p3);});
({f27(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f42(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p1=12ULL;f38(v1, v1_n, p1);});
{uint64_t _i=71ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=5ULL;uint64_t p3=920ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=6ULL;uint64_t p3=4ULL;uint64_t p4=128ULL;uint64_t p5=3ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=6ULL;uint64_t p3=5ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({f23(v0, v0_n, v1, v1_n);});
({uint64_t p2=7ULL;uint64_t p3=6ULL;uint64_t p4=0ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=7ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=7ULL;uint64_t p3=6ULL;uint64_t p4=1ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=9ULL;uint64_t p3=7ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=o_get(v1, v1_n, 369ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=10ULL;uint64_t p3=o_cast(5, 4, 1069066811ULL);f10(v0, v0_n, v1, v1_n, p2, p3);});
}else{
({uint64_t p2=10ULL;uint64_t p3=o_cast(5, 4, 3216550459ULL);f10(v0, v0_n, v1, v1_n, p2, p3);});
}
({uint64_t p2=15ULL;uint64_t p3=o_cast(5, 4, 1065353216ULL);f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=11ULL;uint64_t p1=8ULL;uint64_t p2=10ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=47ULL;uint64_t p3=({uint64_t p0=12ULL;uint64_t p1=11ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=13ULL;uint64_t p1=12ULL;uint64_t p2=15ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=40ULL;uint64_t p3=({uint64_t p0=14ULL;uint64_t p1=13ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=16ULL;uint64_t p1=8ULL;uint64_t p2=14ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=16ULL;uint64_t p1=16ULL;uint64_t p2=9ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=7ULL;uint64_t p3=6ULL;uint64_t p4=2ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=7ULL;uint64_t p3=16ULL;f17(v0, v0_n, v1, v1_n, p2, p3);});
({f27(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f43(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p1=14ULL;f38(v1, v1_n, p1);});
{uint64_t _i=80ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=5ULL;uint64_t p3=920ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=6ULL;uint64_t p3=924ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=7ULL;uint64_t p3=928ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=8ULL;uint64_t p3=932ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=9ULL;uint64_t p3=4ULL;uint64_t p4=8ULL;uint64_t p5=65535ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=10ULL;uint64_t p3=9ULL;uint64_t p4=3ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=13ULL;f11(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=12ULL;f11(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=14ULL;uint64_t p3=128ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=15ULL;uint64_t p3=1ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({f28(v1, v1_n);});
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=16ULL;uint64_t p3=10ULL;uint64_t p4=0ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=17ULL;uint64_t p3=16ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=33ULL;uint64_t p3=({uint64_t p0=13ULL;uint64_t p1=17ULL;uint64_t p2=17ULL;uint64_t p3=13ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=10ULL;uint64_t p3=10ULL;uint64_t p4=14ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=12ULL;uint64_t p3=12ULL;uint64_t p4=15ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=12ULL;uint64_t p3=5ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v2;uint64_t p3=6ULL;f24(v0, v0_n, v1, v1_n, p2, p3);});
({f29(v1, v1_n);});
uint64_t __attribute__((unused)) v3=5ULL;
if(({uint64_t _a=o_get(v1, v1_n, 369ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v3=4ULL;
}
{
uint64_t v4=0;
uint64_t c1=0;
for(;o_op(8,5, v4, v3);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p2=18ULL;uint64_t p3=13ULL;uint64_t p4=({uint64_t _a=16ULL;uint64_t _b=v4;o_op(16, 5, _a,_b);});f21(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=13ULL;uint64_t p1=13ULL;uint64_t p2=18ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
({uint64_t p1=10ULL;f38(v1, v1_n, p1);});
({uint64_t p2=19ULL;uint64_t p3=3ULL;uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=({uint64_t p0=50ULL;uint64_t p1=4ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=19ULL;uint64_t p2=13ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({f22(v0, v0_n, v1, v1_n);});
({uint64_t p2=20ULL;f11(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=21ULL;uint64_t p3=128ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=22ULL;uint64_t p3=256ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=23ULL;uint64_t p3=384ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=49ULL;uint64_t p3=({uint64_t p0=24ULL;uint64_t p1=20ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=49ULL;uint64_t p3=({uint64_t p0=25ULL;uint64_t p1=21ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=49ULL;uint64_t p3=({uint64_t p0=26ULL;uint64_t p1=22ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=49ULL;uint64_t p3=({uint64_t p0=27ULL;uint64_t p1=23ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=28ULL;uint64_t p1=24ULL;uint64_t p2=25ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=28ULL;uint64_t p1=28ULL;uint64_t p2=26ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=28ULL;uint64_t p1=28ULL;uint64_t p2=27ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p1=5ULL;f38(v1, v1_n, p1);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=29ULL;uint64_t p1=28ULL;uint64_t p2=6ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=29ULL;uint64_t p1=29ULL;uint64_t p2=7ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=41ULL;uint64_t p3=({uint64_t p0=30ULL;uint64_t p1=29ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=o_cast(5, 4, 3204448256ULL);f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=33ULL;uint64_t p3=o_cast(5, 4, 1069547520ULL);f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=31ULL;uint64_t p1=29ULL;uint64_t p2=30ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=31ULL;uint64_t p1=31ULL;uint64_t p2=30ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=33ULL;uint64_t p3=({uint64_t p0=31ULL;uint64_t p1=31ULL;uint64_t p2=32ULL;uint64_t p3=33ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=30ULL;uint64_t p1=30ULL;uint64_t p2=31ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p1=4ULL;f38(v1, v1_n, p1);});
{uint64_t _i=98ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=99ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=10ULL;uint64_t p3=9ULL;uint64_t p4=3ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=11ULL;uint64_t p3=3ULL;uint64_t p4=1ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=12ULL;f11(v0, v0_n, v1, v1_n, p2);});
({f28(v1, v1_n);});
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=16ULL;uint64_t p3=10ULL;uint64_t p4=0ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=17ULL;uint64_t p3=16ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=34ULL;uint64_t p3=11ULL;uint64_t p4=1ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=36ULL;uint64_t p3=34ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=37ULL;uint64_t p1=17ULL;uint64_t p2=30ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=37ULL;uint64_t p1=37ULL;uint64_t p2=36ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=35ULL;uint64_t p3=10ULL;uint64_t p4=2ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=35ULL;uint64_t p3=37ULL;f17(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=10ULL;uint64_t p3=10ULL;uint64_t p4=14ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=11ULL;uint64_t p3=11ULL;uint64_t p4=14ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=12ULL;uint64_t p3=12ULL;uint64_t p4=15ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=12ULL;uint64_t p3=5ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v4;uint64_t p3=6ULL;f24(v0, v0_n, v1, v1_n, p2, p3);});
({f29(v1, v1_n);});
({f27(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f44(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p1=13ULL;f38(v1, v1_n, p1);});
{uint64_t _i=81ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=5ULL;uint64_t p3=920ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=12ULL;uint64_t p3=1ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=13ULL;uint64_t p3=4294967295ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=6ULL;uint64_t p3=4ULL;uint64_t p4=128ULL;uint64_t p5=3ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=14ULL;uint64_t p3=5ULL;uint64_t p4=13ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=15ULL;uint64_t p3=6ULL;uint64_t p4=4294967295ULL;uint64_t p5=14ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=({uint64_t p0=16ULL;uint64_t p1=15ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=31ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=7ULL;uint64_t p3=16ULL;uint64_t p4=15ULL;uint64_t p5=6ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=7ULL;uint64_t p4=0ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=17ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=17ULL;uint64_t p3=7ULL;uint64_t p4=1ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=9ULL;uint64_t p3=17ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
static uint64_t f45(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t p1=1ULL;f9(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v3=({uint64_t p1=2ULL;f9(v1, v1_n, p1);});
({uint64_t p2=11ULL;uint64_t p3=10ULL;uint64_t p4=1ULL;f21(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v2;uint64_t p3=11ULL;uint64_t p4=4099ULL;uint64_t p5=10ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=6ULL;uint64_t p3=5ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({f23(v0, v0_n, v1, v1_n);});
({uint64_t p2=v3;uint64_t p3=6ULL;uint64_t p4=2ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v3;uint64_t p3=v2;f17(v0, v0_n, v1, v1_n, p2, p3);});
({f27(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f46(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({f44(v0, v0_n, v1, v1_n);});
({uint64_t p1=2ULL;f38(v1, v1_n, p1);});
({f30(v0, v0_n, v1, v1_n);});
({uint64_t p2=8ULL;uint64_t p3=9ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v2=({f25(v0, v0_n, v1, v1_n);});
({uint64_t p2=10ULL;uint64_t p3=8ULL;uint64_t p4=12ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=0ULL;f31(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v2;f26(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=18ULL;uint64_t p3=8ULL;uint64_t p4=3ULL;uint64_t p5=9ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=29ULL;uint64_t p3=({uint64_t p0=19ULL;uint64_t p1=18ULL;uint64_t p2=8ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=10ULL;uint64_t p3=19ULL;uint64_t p4=5ULL;uint64_t p5=12ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=({uint64_t p0=18ULL;uint64_t p1=10ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=3ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=10ULL;uint64_t p3=10ULL;uint64_t p4=18ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=o_get(v1, v1_n, 369ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=10ULL;uint64_t p3=10ULL;uint64_t p4=12ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
({f32(v0, v0_n, v1, v1_n);});
({f45(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f47(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t p1=1ULL;f9(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v3=({uint64_t p1=1ULL;f9(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v4=({uint64_t p1=1ULL;f9(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v5=({uint64_t p1=1ULL;f9(v1, v1_n, p1);});
({uint64_t p2=v4;uint64_t p3=255ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v5;uint64_t p3=15ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=o_get(v1, v1_n, 369ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=v3;uint64_t p3=255ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
}else{
({uint64_t p2=v3;uint64_t p3=256ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
}
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=18ULL;uint64_t p1=8ULL;uint64_t p2=v4;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=192ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=9ULL;uint64_t p2=v5;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=192ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v2;uint64_t p3=v2;uint64_t p4=12ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=19ULL;f11(v0, v0_n, v1, v1_n, p2);});
({f30(v0, v0_n, v1, v1_n);});
({f28(v1, v1_n);});
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=18ULL;uint64_t p3=v3;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=1ULL;f31(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=18ULL;uint64_t p3=18ULL;uint64_t p4=v2;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=19ULL;uint64_t p3=19ULL;uint64_t p4=12ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v6;uint64_t p3=0ULL;f24(v0, v0_n, v1, v1_n, p2, p3);});
({f29(v1, v1_n);});
({f32(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f48(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({f44(v0, v0_n, v1, v1_n);});
({uint64_t p1=2ULL;f38(v1, v1_n, p1);});
({f47(v0, v0_n, v1, v1_n);});
({uint64_t p2=10ULL;uint64_t p3=19ULL;uint64_t p4=256ULL;uint64_t p5=18ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({f45(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f49(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({f44(v0, v0_n, v1, v1_n);});
({uint64_t p1=4ULL;f38(v1, v1_n, p1);});
({uint64_t p2=10ULL;uint64_t p3=7ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=21ULL;uint64_t p3=100ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=20ULL;uint64_t p1=8ULL;uint64_t p2=12ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=192ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({f30(v0, v0_n, v1, v1_n);});
({uint64_t p2=20ULL;uint64_t p3=12ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=1ULL;f31(v0, v0_n, v1, v1_n, p2);});
({f47(v0, v0_n, v1, v1_n);});
({uint64_t p2=10ULL;uint64_t p3=19ULL;uint64_t p4=256ULL;uint64_t p5=18ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=10ULL;uint64_t p3=10ULL;uint64_t p4=21ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({f32(v0, v0_n, v1, v1_n);});
({f45(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f50(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({f44(v0, v0_n, v1, v1_n);});
({uint64_t p1=5ULL;f38(v1, v1_n, p1);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 370ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
{uint64_t _i=15ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}
({uint64_t p2=6ULL;uint64_t p3=5ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({f23(v0, v0_n, v1, v1_n);});
}
({uint64_t p2=21ULL;uint64_t p3=8ULL;uint64_t p4=1ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=22ULL;uint64_t p1=8ULL;uint64_t p2=12ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=192ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({f30(v0, v0_n, v1, v1_n);});
({uint64_t p2=22ULL;uint64_t p3=12ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=1ULL;f31(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=21ULL;uint64_t p3=21ULL;uint64_t p4=12ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({f32(v0, v0_n, v1, v1_n);});
({uint64_t p2=18ULL;uint64_t p3=3ULL;uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=({uint64_t p0=50ULL;uint64_t p1=4ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=18ULL;uint64_t p2=21ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({f22(v0, v0_n, v1, v1_n);});
({uint64_t p2=29ULL;uint64_t p3=({uint64_t p0=19ULL;uint64_t p1=3ULL;uint64_t p2=12ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=19ULL;uint64_t p3=19ULL;uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=49ULL;uint64_t p3=({uint64_t p0=20ULL;uint64_t p1=19ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f8(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=10ULL;uint64_t p3=8ULL;uint64_t p4=3ULL;uint64_t p5=20ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({f45(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f51(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({f44(v0, v0_n, v1, v1_n);});
({uint64_t p2=10ULL;uint64_t p3=8ULL;uint64_t p4=1ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{
uint64_t v3=0;
uint64_t c1=0;
for(;o_op(8,5, v3, v2);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({f30(v0, v0_n, v1, v1_n);});
({uint64_t p2=8ULL;uint64_t p3=9ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=1ULL;f31(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=10ULL;uint64_t p3=10ULL;uint64_t p4=12ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
{
uint64_t v3=0;
uint64_t c1=0;
for(;o_op(8,5, v3, v2);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({f32(v0, v0_n, v1, v1_n);});
}
}
({f45(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f52(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({f44(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=v2;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
({f30(v0, v0_n, v1, v1_n);});
({uint64_t p2=6ULL;uint64_t p3=5ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({f23(v0, v0_n, v1, v1_n);});
({f32(v0, v0_n, v1, v1_n);});
}
if(({uint64_t _a=v2;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
({f30(v0, v0_n, v1, v1_n);});
({f22(v0, v0_n, v1, v1_n);});
({f32(v0, v0_n, v1, v1_n);});
}
if(({uint64_t _a=v2;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
({f30(v0, v0_n, v1, v1_n);});
({uint64_t p2=11ULL;uint64_t p3=8ULL;uint64_t p4=1ULL;f21(v0, v0_n, v1, v1_n, p2, p3, p4);});
({f32(v0, v0_n, v1, v1_n);});
}
if(({uint64_t _a=v2;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
({f30(v0, v0_n, v1, v1_n);});
({uint64_t p2=8ULL;uint64_t p3=9ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=1ULL;f31(v0, v0_n, v1, v1_n, p2);});
}
if(({uint64_t _a=v2;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
({f32(v0, v0_n, v1, v1_n);});
}
({uint64_t p2=10ULL;uint64_t p3=8ULL;uint64_t p4=1ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({f45(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f53(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({uint64_t p1=8ULL;f38(v1, v1_n, p1);});
{uint64_t _i=79ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=13ULL;uint64_t p3=936ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=18ULL;uint64_t p3=4ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=19ULL;uint64_t p3=8ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=20ULL;uint64_t p3=12ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=14ULL;uint64_t p3=3ULL;uint64_t p4=1ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({f30(v0, v0_n, v1, v1_n);});
}
({f28(v1, v1_n);});
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=14ULL;uint64_t p3=13ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v4=18446744073709551615ULL;
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=1ULL;f31(v0, v0_n, v1, v1_n, p2);});
}else{
v4=({f25(v0, v0_n, v1, v1_n);});
}
({uint64_t p2=15ULL;uint64_t p3=14ULL;uint64_t p4=0ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=16ULL;uint64_t p3=15ULL;f16(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=17ULL;uint64_t p3=14ULL;uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=({uint64_t p0=50ULL;uint64_t p1=4ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=17ULL;uint64_t p2=16ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=14ULL;uint64_t p3=9ULL;uint64_t p4=128ULL;uint64_t p5=14ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v3;uint64_t p3=0ULL;f24(v0, v0_n, v1, v1_n, p2, p3);});
({f29(v1, v1_n);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({f32(v0, v0_n, v1, v1_n);});
}else{
({uint64_t p2=v4;f26(v0, v0_n, v1, v1_n, p2);});
}
({f22(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f54(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
{
uint64_t v3=0;
uint64_t k1=30ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
uint64_t __attribute__((unused)) v4=1ULL;
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=5ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=24ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v4=2ULL;
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=24ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
v4=1ULL;
}
}
{uint64_t _i=({uint64_t _a=v3;uint64_t _b=13ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=v4;f9(v0, v0_n, p1);});o_put(v1, v1_n, _i,_v,5);}
}
}
return o_leave(0);
}
static uint64_t f55(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=9ULL;uint64_t p3=({uint64_t p0=v2;uint64_t p1=9ULL;uint64_t p2=65535ULL;uint64_t p3=v4;f2(p0, p1, p2, p3);});uint64_t p4=v3;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f56(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=11ULL;
uint64_t __attribute__((unused)) v5=0ULL;
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v4=49ULL;
v5=65535ULL;
}
({uint64_t p2=({uint64_t p0=v4;uint64_t p1=2ULL;uint64_t p2=0ULL;uint64_t p3=v5;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 21ULL, 5);uint64_t p1=o_get(v2, v2_n, 17ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=3ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=({uint64_t p0=v4;uint64_t p1=2ULL;uint64_t p2=0ULL;uint64_t p3=v5;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 19ULL, 5);uint64_t p1=o_get(v2, v2_n, 16ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=3ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=({uint64_t p0=v4;uint64_t p1=2ULL;uint64_t p2=0ULL;uint64_t p3=v5;f3(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 20ULL, 5);uint64_t p1=o_get(v2, v2_n, 18ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f6(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f57(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({f30(v0, v0_n, v1, v1_n);});
({f28(v1, v1_n);});
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=10ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=1ULL;f31(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=52ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 33ULL, 5);uint64_t p1=9ULL;uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 15ULL, 5);uint64_t p1=o_get(v2, v2_n, 15ULL, 5);uint64_t p2=o_get(v2, v2_n, 33ULL, 5);uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=252ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=o_get(v2, v2_n, 30ULL, 5);uint64_t p4=o_get(v2, v2_n, 19ULL, 5);f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v4;uint64_t p3=0ULL;f24(v0, v0_n, v1, v1_n, p2, p3);});
({f29(v1, v1_n);});
({f32(v0, v0_n, v1, v1_n);});
}else{
({uint64_t p2=o_get(v2, v2_n, 38ULL, 5);f11(v0, v0_n, v1, v1_n, p2);});
({f28(v1, v1_n);});
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=o_get(v2, v2_n, 38ULL, 5);uint64_t p3=10ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v5=({f25(v0, v0_n, v1, v1_n);});
({uint64_t p2=o_get(v2, v2_n, 39ULL, 5);uint64_t p3=o_get(v2, v2_n, 30ULL, 5);uint64_t p4=o_get(v2, v2_n, 38ULL, 5);f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 40ULL, 5);uint64_t p1=o_get(v2, v2_n, 39ULL, 5);uint64_t p2=o_get(v2, v2_n, 41ULL, 5);uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=192ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=52ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 33ULL, 5);uint64_t p1=9ULL;uint64_t p2=o_get(v2, v2_n, 40ULL, 5);uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=o_get(v2, v2_n, 39ULL, 5);uint64_t p4=o_get(v2, v2_n, 42ULL, 5);f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=17ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 32ULL, 5);uint64_t p1=o_get(v2, v2_n, 31ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=31ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 35ULL, 5);uint64_t p3=o_get(v2, v2_n, 32ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 15ULL, 5);uint64_t p1=o_get(v2, v2_n, 15ULL, 5);uint64_t p2=o_get(v2, v2_n, 33ULL, 5);uint64_t p3=o_get(v2, v2_n, 35ULL, 5);f2(p0, p1, p2, p3);});uint64_t p4=248ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 38ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=o_get(v2, v2_n, 19ULL, 5);f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v4;uint64_t p3=0ULL;f24(v0, v0_n, v1, v1_n, p2, p3);});
({f29(v1, v1_n);});
({uint64_t p2=v5;f26(v0, v0_n, v1, v1_n, p2);});
}
return o_leave(0);
}
static uint64_t f58(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n){
o_enter();
({uint64_t p2=o_get(v2, v2_n, 22ULL, 5);uint64_t p3=o_get(v2, v2_n, 19ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 13ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=17ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 24ULL, 5);uint64_t p1=o_get(v2, v2_n, 23ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=31ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 25ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 26ULL, 5);uint64_t p3=o_get(v2, v2_n, 24ULL, 5);uint64_t p4=o_get(v2, v2_n, 25ULL, 5);uint64_t p5=o_get(v2, v2_n, 23ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 27ULL, 5);uint64_t p3=o_get(v2, v2_n, 24ULL, 5);uint64_t p4=o_get(v2, v2_n, 25ULL, 5);uint64_t p5=65535ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=53ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 28ULL, 5);uint64_t p1=o_get(v2, v2_n, 26ULL, 5);uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 29ULL, 5);uint64_t p3=o_get(v2, v2_n, 28ULL, 5);uint64_t p4=o_get(v2, v2_n, 22ULL, 5);uint64_t p5=o_get(v2, v2_n, 26ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=o_get(v2, v2_n, 29ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 19ULL, 5);f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=o_get(v1, v1_n, 369ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=o_get(v2, v2_n, 29ULL, 5);uint64_t p4=11ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=17ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 32ULL, 5);uint64_t p1=o_get(v2, v2_n, 31ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=31ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=o_get(v2, v2_n, 32ULL, 5);uint64_t p4=o_get(v2, v2_n, 22ULL, 5);uint64_t p5=o_get(v2, v2_n, 30ULL, 5);f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=o_get(v2, v2_n, 30ULL, 5);uint64_t p4=o_get(v2, v2_n, 27ULL, 5);f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f59(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n){
o_enter();
({uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=12ULL;uint64_t p4=11ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=17ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 32ULL, 5);uint64_t p1=o_get(v2, v2_n, 31ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=31ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 15ULL, 5);uint64_t p1=o_get(v2, v2_n, 15ULL, 5);uint64_t p2=o_get(v2, v2_n, 32ULL, 5);uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=252ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=29ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 34ULL, 5);uint64_t p1=12ULL;uint64_t p2=7ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=0ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=o_get(v2, v2_n, 34ULL, 5);uint64_t p4=11ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=17ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 32ULL, 5);uint64_t p1=o_get(v2, v2_n, 31ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=31ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 35ULL, 5);uint64_t p3=o_get(v2, v2_n, 32ULL, 5);uint64_t p4=8ULL;uint64_t p5=65535ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 36ULL, 5);uint64_t p1=o_get(v2, v2_n, 15ULL, 5);uint64_t p2=o_get(v2, v2_n, 35ULL, 5);uint64_t p3=65535ULL;f2(p0, p1, p2, p3);});uint64_t p4=3ULL;f7(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 37ULL, 5);uint64_t p3=12ULL;uint64_t p4=2ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 37ULL, 5);uint64_t p3=o_get(v2, v2_n, 36ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=12ULL;uint64_t p3=6ULL;uint64_t p4=9ULL;uint64_t p5=12ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=12ULL;uint64_t p3=5ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
static uint64_t f60(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p1=8ULL;f38(v1, v1_n, p1);});
({uint64_t p2=5ULL;uint64_t p3=920ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=6ULL;uint64_t p3=924ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=7ULL;uint64_t p3=928ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=8ULL;uint64_t p3=932ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=9ULL;uint64_t p3=1ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=10ULL;uint64_t p3=32ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=11ULL;uint64_t p3=4294967295ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=v3;f53(v0, v0_n, v1, v1_n, p2);});
}
({uint64_t p2=12ULL;uint64_t p3=4ULL;uint64_t p4=128ULL;uint64_t p5=3ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=12ULL;uint64_t p3=5ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({f23(v0, v0_n, v1, v1_n);});
uint64_t v4[64]={0};
uint64_t v4_n=64;
o_fill(v4, v4_n, 0ULL, 5);
({uint64_t p2=v2;f54(v1, v1_n, v4, v4_n, p2);});
({uint64_t p2=o_get(v4, v4_n, 41ULL, 5);uint64_t p3=31ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v4, v4_n, 42ULL, 5);uint64_t p3=4294967264ULL;f10(v0, v0_n, v1, v1_n, p2, p3);});
({f28(v1, v1_n);});
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=o_get(v4, v4_n, 13ULL, 5);uint64_t p3=12ULL;uint64_t p4=32ULL;uint64_t p5=65535ULL;f12(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v4, v4_n, 14ULL, 5);uint64_t p3=o_get(v4, v4_n, 13ULL, 5);uint64_t p4=10ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v4, v4_n, 15ULL, 5);f11(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=o_get(v4, v4_n, 16ULL, 5);f11(v0, v0_n, v1, v1_n, p2);});
}else{
({uint64_t p2=o_get(v4, v4_n, 16ULL, 5);uint64_t p3=0ULL;uint64_t p4=0ULL;f55(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(11, 5, _a,_b);})){
({f30(v0, v0_n, v1, v1_n);});
}
({f28(v1, v1_n);});
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, 0ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=o_get(v4, v4_n, 17ULL, 5);uint64_t p3=o_get(v4, v4_n, 16ULL, 5);uint64_t p4=19ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v4, v4_n, 18ULL, 5);uint64_t p3=o_get(v4, v4_n, 16ULL, 5);uint64_t p4=18ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
}else{
({uint64_t p2=o_get(v4, v4_n, 17ULL, 5);uint64_t p3=8ULL;uint64_t p4=o_get(v4, v4_n, 16ULL, 5);f55(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v4, v4_n, 18ULL, 5);uint64_t p3=4ULL;uint64_t p4=o_get(v4, v4_n, 16ULL, 5);f55(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
({uint64_t p3=v2;f56(v0, v0_n, v1, v1_n, v4, v4_n, p3);});
({uint64_t p2=o_get(v4, v4_n, 21ULL, 5);uint64_t p3=o_get(v4, v4_n, 14ULL, 5);f19(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v7=18446744073709551615ULL;
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(11, 5, _a,_b);})){
({uint64_t p2=1ULL;f31(v0, v0_n, v1, v1_n, p2);});
}else{
v7=({f25(v0, v0_n, v1, v1_n);});
}
({f58(v0, v0_n, v1, v1_n, v4, v4_n);});
({uint64_t p3=v3;f57(v0, v0_n, v1, v1_n, v4, v4_n, p3);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=o_get(v4, v4_n, 16ULL, 5);uint64_t p3=o_get(v4, v4_n, 16ULL, 5);uint64_t p4=20ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
}else{
({uint64_t p2=o_get(v4, v4_n, 16ULL, 5);uint64_t p3=12ULL;uint64_t p4=o_get(v4, v4_n, 16ULL, 5);f55(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
({uint64_t p2=v6;uint64_t p3=0ULL;f24(v0, v0_n, v1, v1_n, p2, p3);});
({f29(v1, v1_n);});
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(11, 5, _a,_b);})){
({f32(v0, v0_n, v1, v1_n);});
}else{
({uint64_t p2=v7;f26(v0, v0_n, v1, v1_n, p2);});
}
({f59(v0, v0_n, v1, v1_n, v4, v4_n);});
({uint64_t p2=v5;uint64_t p3=6ULL;f24(v0, v0_n, v1, v1_n, p2, p3);});
({f29(v1, v1_n);});
({f27(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f61(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n){
o_enter();
{
uint64_t v6=0;
uint64_t k1=512ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
{uint64_t _i=v6;uint64_t _v=0ULL;o_put(v5, v5_n, _i,_v,5);}
}
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=1ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=23ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=3ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=369ULL;uint64_t _v=1ULL;o_put(v5, v5_n, _i,_v,5);}
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=368ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v5, v5_n, _i,_v,5);}
}
{uint64_t _i=370ULL;uint64_t _v=v2;o_put(v5, v5_n, _i,_v,5);}
({f34(v4, v4_n, v5, v5_n);});
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({f39(v4, v4_n, v5, v5_n);});
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({f40(v4, v4_n, v5, v5_n);});
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
({f43(v4, v4_n, v5, v5_n);});
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
({f41(v4, v4_n, v5, v5_n);});
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
({f42(v4, v4_n, v5, v5_n);});
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=6ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=11ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=6ULL;o_op(2, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=6ULL;o_op(2, 5, _a,_b);});uint64_t _b=2ULL;o_op(4, 5, _a,_b);});
({uint64_t p2=v6;uint64_t p3=v7;f60(v4, v4_n, v5, v5_n, p2, p3);});
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
({f46(v4, v4_n, v5, v5_n);});
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
({f48(v4, v4_n, v5, v5_n);});
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
({f49(v4, v4_n, v5, v5_n);});
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=o_get(v5, v5_n, 369ULL, 5);f50(v4, v4_n, v5, v5_n, p2);});
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=16ULL;f51(v4, v4_n, v5, v5_n, p2);});
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=17ULL;f51(v4, v4_n, v5, v5_n, p2);});
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=2ULL;f50(v4, v4_n, v5, v5_n, p2);});
}
if(({uint64_t _a=v0;uint64_t _b=19ULL;o_op(11, 5, _a,_b);})){
({uint64_t p2=v0;f52(v4, v4_n, v5, v5_n, p2);});
}
if(({uint64_t _a=o_get(v5, v5_n, 13ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=11ULL;o_put(v5, v5_n, _i,_v,5);}
}
({f36(v4, v4_n, v5, v5_n);});
if(({uint64_t _a=o_get(v5, v5_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
uint32_t omega_ew_ir(uint64_t  *result, volatile uint64_t p0, volatile uint64_t p1, volatile uint64_t p2, volatile uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n){
  if(!o_buf(result,1,1,5))return 32;
if(!o_valid(5, p0))return 32;
if(!o_valid(5, p1))return 32;
if(!o_valid(5, p2))return 32;
if(!o_valid(5, p3))return 32;
if(!o_buf(p4, p4_n,8192, 5))return 32;
  if(o_overlap(result,8,p4, p4_n*8))return 32;
if(!o_buf(p5, p5_n,512, 5))return 32;
  if(o_overlap(result,8,p5, p5_n*8))return 32;
if(o_overlap(p5, p5_n*8, p4, p4_n*8))return 32;
  if(o_busy)return 33;
  o_busy=1;o_depth=0;int code=setjmp(o_jmp);if(!code){uint64_t _result=f61(p0, p1, p2, p3, p4, p4_n, p5, p5_n);o_put(result,1,0,_result,5);}
  o_busy=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
