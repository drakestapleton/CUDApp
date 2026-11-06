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
static uint64_t f2(uint64_t v0);
static uint64_t f3(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f4(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f5(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f6(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f7(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f8(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f9(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f10(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f11(void *v0, uint64_t v0_n);
static uint64_t f12(void *v0, uint64_t v0_n);
static uint64_t f13(uint64_t v0, uint64_t v1);
static uint64_t f14(void *v0, uint64_t v0_n);
static uint64_t f15(void *v0, uint64_t v0_n);
static uint64_t f16(void *v0, uint64_t v0_n);
static uint64_t f17(void *v0, uint64_t v0_n);
static uint64_t f18(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f19(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f20(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f21(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f22(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f23(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f24(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f25(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f26(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f27(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f28(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f29(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f30(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f31(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f32(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f33(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f34(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f35(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f36(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f37(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f38(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f39(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f40(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f41(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f42(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f43(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f44(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f45(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f46(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f47(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f48(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f49(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f50(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f51(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f52(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f53(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f54(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f55(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f56(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f57(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f58(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f59(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f60(void *v0, uint64_t v0_n);
static uint64_t f61(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f62(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f63(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f64(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f65(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f66(uint64_t v0);
static uint64_t f67(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f68(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f69(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f70(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f71(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f72(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f73(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f74(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f75(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f76(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f77(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f78(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f79(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f80(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f81(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f82(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f83(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f84(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f85(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f86(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f87(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f88(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f89(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f90(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n);
static uint64_t f0(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, ({uint64_t _a=1ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=256ULL;o_op(11, 5, _a,_b);});o_op(12, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(11, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
{uint64_t _i=({uint64_t _a=1ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=({uint64_t _a=64ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
}else{
{uint64_t _i=({uint64_t _a=320ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
}
return o_leave(v3);
}
static uint64_t f1(uint64_t v0){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);})){
return o_leave(18446744073709551615ULL);
}
return o_leave(({uint64_t _a=v0;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));
}
static uint64_t f2(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=18446744073709551615ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t _a=v0;uint64_t _b=32ULL;o_op(16, 5, _a,_b);}));
}
static uint64_t f3(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
return o_leave(0);
}
{uint64_t _i=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f4(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({uint64_t p2=0ULL;uint64_t p3=v2;f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=1ULL;uint64_t p3=18446744073709551615ULL;f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=18446744073709551615ULL;f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=3ULL;uint64_t p3=18446744073709551615ULL;f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=4ULL;uint64_t p3=18446744073709551615ULL;f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=5ULL;uint64_t p3=18446744073709551615ULL;f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=6ULL;uint64_t p3=0ULL;f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=7ULL;uint64_t p3=0ULL;f3(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
static uint64_t f5(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({uint64_t p2=6ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});f3(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
static uint64_t f6(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=v3;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v3;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f7(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
return o_leave(0);
}
({uint64_t p2=v2;uint64_t p3=({uint64_t p0=v3;f1(p0);});f3(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=7ULL;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=16ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=v4;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, v4, 5);uint64_t _b=({uint64_t _a=({uint64_t p0=v3;f2(p0);});uint64_t _b=v5;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f8(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=14ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=17ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=20ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f9(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 7ULL, 5);
uint64_t __attribute__((unused)) v4=({uint64_t p0=6ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v3;f8(p0, p1, p2, p3);});
uint64_t __attribute__((unused)) v5=0ULL;
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t p0=14ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v3;f8(p0, p1, p2, p3);});
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v3=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});
}
v4=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=0ULL;uint64_t p2=1ULL;uint64_t p3=v3;f8(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
v5=3ULL;
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=o_get(v1, v1_n, 7ULL, 5);uint64_t _b=3ULL;o_op(13, 5, _a,_b);});
}
}
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=2ULL;uint64_t p2=7ULL;uint64_t p3=v3;f8(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
v5=4ULL;
}
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=7ULL;uint64_t p2=1ULL;uint64_t p3=v3;f8(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
v5=2ULL;
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=7ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v4=({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v3;f8(p0, p1, p2, p3);});
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v5=v3;
}
}
if(({uint64_t _a=v2;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=({uint64_t p0=1ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v3;f8(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v3;f8(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
}
({uint64_t p2=v4;f6(v0, v0_n, v1, v1_n, p2);});
{uint64_t _i=7ULL;uint64_t _v=v5;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f10(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v1;uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=v3;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v3, 5);uint64_t _b=18446744069414584320ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v1;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f11(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 9ULL, 5);
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=({uint64_t _a=10ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, 0ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f12(void *v0, uint64_t v0_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 9ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v1=({uint64_t _a=o_get(v0, v0_n, 9ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
{uint64_t _i=9ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 5ULL, 5);
{uint64_t _i=({uint64_t _a=16ULL;uint64_t _b=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=10ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);}), 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=17ULL;uint64_t _b=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f13(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f14(void *v0, uint64_t v0_n){
o_enter();
return o_leave(({uint64_t p1=1ULL;uint64_t p2=0ULL;f0(v0, v0_n, p1, p2);}));
}
static uint64_t f15(void *v0, uint64_t v0_n){
o_enter();
return o_leave(({uint64_t p1=2ULL;uint64_t p2=0ULL;f0(v0, v0_n, p1, p2);}));
}
static uint64_t f16(void *v0, uint64_t v0_n){
o_enter();
return o_leave(({uint64_t p1=4ULL;uint64_t p2=0ULL;f0(v0, v0_n, p1, p2);}));
}
static uint64_t f17(void *v0, uint64_t v0_n){
o_enter();
return o_leave(({uint64_t p1=2ULL;uint64_t p2=1ULL;f0(v0, v0_n, p1, p2);}));
}
static uint64_t f18(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=7ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v3;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=7ULL;uint64_t p3=1ULL;f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=4ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f19(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=5ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v3;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=2ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f20(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=4ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v3;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=2ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f21(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=3ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v3;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=2ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f22(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=2ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v3;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f23(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({uint64_t p2=1ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f24(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=8ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v4;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=4ULL;uint64_t p3=v5;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f25(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=8ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=3ULL;uint64_t p3=v4;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=4ULL;uint64_t p3=v5;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f26(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=9ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v4;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=4ULL;uint64_t p3=v5;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f27(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=1ULL;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f28(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=17ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v4;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f29(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=18ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v4;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f30(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=29ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=3ULL;uint64_t p3=v4;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f31(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=51ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=3ULL;uint64_t p3=v4;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v5;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f32(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=v2;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v4;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v5;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=5ULL;uint64_t p3=o_get(v1, v1_n, 3ULL, 5);f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f33(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=v2;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v4;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v5;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=5ULL;uint64_t p3=o_get(v1, v1_n, 3ULL, 5);f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=3ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f34(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=12ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=2ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=3ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=5ULL;uint64_t p3=o_get(v1, v1_n, 3ULL, 5);f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=5ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f35(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=58ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v4;f5(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=2ULL;f9(v0, v0_n, v1, v1_n, p2);});
}else{
({uint64_t p2=3ULL;f9(v0, v0_n, v1, v1_n, p2);});
}
return o_leave(0);
}
static uint64_t f36(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=49ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f37(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=50ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=2ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=3ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=5ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f38(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p2=48ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=6ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f39(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=v2;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v4;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=3ULL;uint64_t p3=v5;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f40(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=33ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=3ULL;uint64_t p3=v4;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=4ULL;uint64_t p3=v5;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=0ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f41(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=v2;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v4;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f42(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=42ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=2ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v4;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=2ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f43(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=28ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=2ULL;uint64_t p3=v2;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=3ULL;uint64_t p3=v3;f7(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=1ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f44(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=46ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=({uint64_t _a=v2;uint64_t _b=v3;o_op(2, 5, _a,_b);});f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=8ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f45(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=46ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1ULL;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=7ULL;uint64_t p3=2ULL;f3(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=8ULL;f9(v0, v0_n, v1, v1_n, p2);});
return o_leave(v2);
}
static uint64_t f46(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=54ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=2ULL;f5(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=9ULL;f9(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v3=({f45(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=v3;uint64_t _b=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_op(7, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(v2);
}
static uint64_t f47(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=55ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=10ULL;f9(v0, v0_n, v1, v1_n, p2);});
({uint64_t p1=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p2=v3;f10(v0, v0_n, p1, p2);});
({uint64_t p1=v2;uint64_t p2=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});f10(v0, v0_n, p1, p2);});
return o_leave(0);
}
static uint64_t f48(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
({uint64_t p2=45ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=7ULL;f9(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
({uint64_t p2=46ULL;f4(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=1032192ULL;f6(v0, v0_n, v1, v1_n, p2);});
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f49(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
return o_leave(({uint64_t p0=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(4, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);uint64_t p1=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(5, 5, _a,_b);});f13(p0, p1);}));
}
static uint64_t f50(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 24ULL, 5);
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 25ULL, 5);
({uint64_t p2=v6;uint64_t p3=v4;uint64_t p4=v5;f30(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v7;uint64_t p3=v6;uint64_t p4=4294967295ULL;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;uint64_t p3=v6;uint64_t p4=v7;uint64_t p5=252ULL;f31(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v3;uint64_t p3=v6;uint64_t p4=31ULL;f28(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f51(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=16ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c1++>=5)o_trap(4);
({uint64_t p2=v4;uint64_t p3=v3;uint64_t p4=v5;f42(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=v3;uint64_t p5=v4;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
v5=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(4, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f52(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=v2;uint64_t p3=v3;f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v6=({f46(v0, v0_n, v1, v1_n);});
({uint64_t p2=v4;uint64_t p3=v5;f37(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v6;f47(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f53(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({f16(v1, v1_n);});
uint64_t __attribute__((unused)) v7=({f16(v1, v1_n);});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v8=o_get(v2, v2_n, 26ULL, 5);
({uint64_t p2=v8;uint64_t p3=v4;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v6;uint64_t p3=v8;uint64_t p4=0ULL;uint64_t p5=1ULL;f35(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v7;uint64_t p3=v8;uint64_t p4=16ULL;uint64_t p5=0ULL;f35(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v3;uint64_t p3=({uint64_t p0=v6;uint64_t p1=0ULL;f13(p0, p1);});f27(v0, v0_n, v1, v1_n, p2, p3);});
{
uint64_t v9=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v9, k1);v9=o_op(1,5, v9,1)){
if(({uint64_t _a=v9;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
uint64_t __attribute__((unused)) v10=({uint64_t p0=v7;uint64_t p1=({uint64_t _a=v9;uint64_t _b=4ULL;o_op(2, 5, _a,_b);});f13(p0, p1);});
if(({uint64_t _a=v9;uint64_t _b=4ULL;o_op(8, 5, _a,_b);})){
v10=({uint64_t p0=v6;uint64_t p1=v9;f13(p0, p1);});
}
({uint64_t p2=v5;uint64_t p3=v3;uint64_t p4=v3;uint64_t p5=v10;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
}
}
return o_leave(0);
}
static uint64_t f54(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
{
uint64_t v6=0;
uint64_t c1=0;
for(;o_op(8,5, v6, v4);v6=o_op(1,5, v6,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=57ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=v3;uint64_t _b=v6;o_op(1, 5, _a,_b);}), 5);uint64_t p4=v5;uint64_t p5=({uint64_t _a=16ULL;uint64_t _b=v6;o_op(3, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}else{
({uint64_t p2=57ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=v3;uint64_t _b=v6;o_op(1, 5, _a,_b);}), 5);uint64_t p4=v5;uint64_t p5=({uint64_t _a=16ULL;uint64_t _b=v6;o_op(3, 5, _a,_b);});f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
}
}
return o_leave(0);
}
static uint64_t f55(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
{
uint64_t v4=0;
uint64_t c1=0;
for(;o_op(8,5, v4, v3);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=({f16(v0, v0_n);});o_put(v1, v1_n, _i,_v,5);}
}
}
return o_leave(0);
}
static uint64_t f56(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=v2;uint64_t p3=v4;uint64_t p4=4294967295ULL;uint64_t p5=v3;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v2;uint64_t p3=v2;uint64_t p4=31ULL;f28(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f57(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=1ULL;uint64_t p5=v4;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v2;uint64_t p3=v2;uint64_t p4=1ULL;f28(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f58(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 55ULL, 5);
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=v4;uint64_t p5=v3;f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;uint64_t p3=v5;f34(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
static uint64_t f59(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 2ULL, 5);
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=256ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
{uint64_t _i=3ULL;uint64_t _v=({f17(v1, v1_n);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=42ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=32ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=43ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=23ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=39ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=40ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=44ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=45ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
({uint64_t p2=o_get(v1, v1_n, 3ULL, 5);uint64_t p3=856ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 10ULL, 5);uint64_t p3=896ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 11ULL, 5);uint64_t p3=904ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 42ULL, 5);uint64_t p3=912ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=33ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 21ULL, 5);uint64_t p3=37ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 32ULL, 5);uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=256ULL;uint64_t p5=o_get(v2, v2_n, 20ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 39ULL, 5);uint64_t p3=o_get(v2, v2_n, 32ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 40ULL, 5);uint64_t p3=o_get(v2, v2_n, 32ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 43ULL, 5);f23(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=1ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 0ULL, 5);
({f11(v1, v1_n);});
({uint64_t p2=o_get(v2, v2_n, 43ULL, 5);uint64_t p3=o_get(v2, v2_n, 42ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v5=({f45(v0, v0_n, v1, v1_n);});
({uint64_t p2=11ULL;uint64_t p3=o_get(v2, v2_n, 44ULL, 5);uint64_t p4=o_get(v2, v2_n, 39ULL, 5);uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=11ULL;uint64_t p3=o_get(v2, v2_n, 45ULL, 5);uint64_t p4=o_get(v2, v2_n, 40ULL, 5);uint64_t p5=0ULL;f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=30ULL;uint64_t p3=o_get(v2, v2_n, 44ULL, 5);uint64_t p4=o_get(v2, v2_n, 44ULL, 5);uint64_t p5=o_get(v2, v2_n, 45ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 39ULL, 5);uint64_t p3=o_get(v2, v2_n, 44ULL, 5);f34(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
({uint64_t p2=o_get(v2, v2_n, 39ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=v6;uint64_t p5=o_get(v2, v2_n, 39ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 40ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=v6;uint64_t p5=o_get(v2, v2_n, 40ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 43ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 43ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v4;f44(v0, v0_n, v1, v1_n, p2);});
({f12(v1, v1_n);});
({uint64_t p1=v5;uint64_t p2=o_get(v1, v1_n, 0ULL, 5);f10(v0, v0_n, p1, p2);});
({f48(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
static uint64_t f60(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 4ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v2=2048ULL;
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=v2;uint64_t _b=256ULL;o_op(4, 5, _a,_b);});
{uint64_t _i=6ULL;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(4, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=7ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=15ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 2ULL, 5);
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=16ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
}
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 5ULL, 5);
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=9ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=9ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=9ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1024ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=2048ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
static uint64_t f61(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=3ULL;uint64_t _v=({f17(v1, v1_n);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=36ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=35ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=27ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=28ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=23ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=37ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=24ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=25ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=26ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=39ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=120ULL;uint64_t p3=o_get(v2, v2_n, 8ULL, 5);f55(v1, v1_n, v2, v2_n, p2, p3);});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
({uint64_t p2=o_get(v1, v1_n, 3ULL, 5);uint64_t p3=856ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 10ULL, 5);uint64_t p3=896ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 11ULL, 5);uint64_t p3=904ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 12ULL, 5);uint64_t p3=912ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 13ULL, 5);uint64_t p3=920ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 14ULL, 5);uint64_t p3=928ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 36ULL, 5);uint64_t p3=936ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 35ULL, 5);uint64_t p3=940ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=33ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=1ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 27ULL, 5);uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=5ULL;f28(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 28ULL, 5);uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=31ULL;f29(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 37ULL, 5);uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=({uint64_t _a=4ULL;uint64_t _b=o_get(v2, v2_n, 7ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=38ULL;uint64_t _v=o_get(v2, v2_n, 37ULL, 5);o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=8ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=22ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=38ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
({uint64_t p2=o_get(v2, v2_n, 22ULL, 5);uint64_t p3=38ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 38ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=4ULL;uint64_t _b=o_get(v2, v2_n, 6ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 37ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=o_get(v2, v2_n, 39ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p3=120ULL;uint64_t p4=o_get(v2, v2_n, 8ULL, 5);uint64_t p5=o_get(v2, v2_n, 39ULL, 5);f54(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f62(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=40ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=128ULL;uint64_t p3=o_get(v2, v2_n, 8ULL, 5);f55(v1, v1_n, v2, v2_n, p2, p3);});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
({uint64_t p2=o_get(v2, v2_n, 40ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p3=128ULL;uint64_t p4=o_get(v2, v2_n, 8ULL, 5);uint64_t p5=o_get(v2, v2_n, 40ULL, 5);f54(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 7ULL, 5);
{
uint64_t v4=0;
uint64_t c1=0;
for(;o_op(8,5, v4, v3);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t p1=120ULL;uint64_t p2=v4;f49(v2, v2_n, p1, p2);});
({uint64_t p2=30ULL;uint64_t p3=v5;uint64_t p4=v5;uint64_t p5=({uint64_t p1=128ULL;uint64_t p2=v4;f49(v2, v2_n, p1, p2);});f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
}
return o_leave(0);
}
static uint64_t f63(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
{
uint64_t v5=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
uint64_t __attribute__((unused)) v6=({uint64_t p1=146ULL;uint64_t p2=v5;f49(v2, v2_n, p1, p2);});
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, ({uint64_t _a=160ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=32ULL;uint64_t p3=v7;uint64_t p4=v3;uint64_t p5=v6;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}else{
({uint64_t p2=32ULL;uint64_t p3=o_get(v2, v2_n, 24ULL, 5);uint64_t p4=v3;uint64_t p5=v6;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=30ULL;uint64_t p3=v7;uint64_t p4=v7;uint64_t p5=o_get(v2, v2_n, 24ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
}
}
return o_leave(0);
}
static uint64_t f64(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 5ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v3=8ULL;
}
{uint64_t _i=144ULL;uint64_t _v=({f16(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=144ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({f16(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=v3;uint64_t _b=8ULL;o_op(10, 5, _a,_b);})){
{uint64_t _i=90ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
}
{uint64_t _i=89ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=146ULL;uint64_t _v=({f16(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=146ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({f16(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{
uint64_t v4=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
{uint64_t _i=({uint64_t _a=160ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
}
}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
{uint64_t _i=91ULL;uint64_t _v=o_get(v2, v2_n, 12ULL, 5);o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=92ULL;uint64_t _v=o_get(v2, v2_n, 37ULL, 5);o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=8ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=91ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=92ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
({uint64_t p2=o_get(v2, v2_n, 91ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=o_get(v2, v2_n, 2ULL, 5);uint64_t p5=o_get(v2, v2_n, 12ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 92ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=({uint64_t _a=v3;uint64_t _b=8192ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 37ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=57ULL;uint64_t p3=o_get(v2, v2_n, 144ULL, 5);uint64_t p4=o_get(v2, v2_n, 91ULL, 5);uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=57ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=144ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p4=o_get(v2, v2_n, 91ULL, 5);uint64_t p5=16ULL;f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=v3;uint64_t _b=8ULL;o_op(10, 5, _a,_b);})){
{uint64_t _i=93ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
({uint64_t p2=o_get(v2, v2_n, 93ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=32ULL;uint64_t p5=o_get(v2, v2_n, 91ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=11ULL;uint64_t p3=o_get(v2, v2_n, 90ULL, 5);uint64_t p4=o_get(v2, v2_n, 93ULL, 5);uint64_t p5=0ULL;f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=o_get(v2, v2_n, 89ULL, 5);uint64_t p3=o_get(v2, v2_n, 92ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v4=v3;
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 3ULL, 5);
if(({uint64_t _a=v5;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v4=7ULL;
}
if(({uint64_t _a=v5;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v4=8ULL;
}
{
uint64_t v6=0;
uint64_t c1=0;
for(;o_op(8,5, v6, v4);v6=o_op(1,5, v6,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p2=57ULL;uint64_t p3=o_get(v2, v2_n, 146ULL, 5);uint64_t p4=o_get(v2, v2_n, 89ULL, 5);uint64_t p5=({uint64_t _a=v6;uint64_t _b=8192ULL;o_op(3, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=57ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=146ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p4=o_get(v2, v2_n, 89ULL, 5);uint64_t p5=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=8192ULL;o_op(3, 5, _a,_b);});uint64_t _b=16ULL;o_op(1, 5, _a,_b);});f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 90ULL, 5);
if(({uint64_t _a=v6;uint64_t _b=8ULL;o_op(8, 5, _a,_b);})){
v7=({uint64_t p1=144ULL;uint64_t p2=v6;f49(v2, v2_n, p1, p2);});
}
({uint64_t p3=v7;uint64_t p4=v6;f63(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
}
}
{
uint64_t v6=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
uint64_t __attribute__((unused)) v7=({uint64_t p1=120ULL;uint64_t p2=v6;f49(v2, v2_n, p1, p2);});
({uint64_t p2=30ULL;uint64_t p3=v7;uint64_t p4=v7;uint64_t p5=o_get(v2, v2_n, ({uint64_t _a=160ULL;uint64_t _b=v6;o_op(1, 5, _a,_b);}), 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
}
return o_leave(0);
}
static uint64_t f65(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=41ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 41ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 7ULL, 5);
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v4);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=v3;uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}else{
({uint64_t p2=v3;uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=4ULL;uint64_t p5=v3;f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=v3;uint64_t p3=({uint64_t p1=120ULL;uint64_t p2=v5;f49(v2, v2_n, p1, p2);});f34(v0, v0_n, v1, v1_n, p2, p3);});
}
}
return o_leave(0);
}
static uint64_t f66(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=2048ULL;o_op(6, 5, _a,_b);})){
return o_leave(973078528ULL);
}
return o_leave(o_cast(5, 4, ({uint64_t _a=1065353216ULL;uint64_t _b=o_fcv(11, 5, v0);o_fp(4, 11, _a,_b);})));
}
static uint64_t f67(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=84ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=85ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=86ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=136ULL;uint64_t p3=o_get(v2, v2_n, 8ULL, 5);f55(v1, v1_n, v2, v2_n, p2, p3);});
{uint64_t _i=87ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=88ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 84ULL, 5);
({uint64_t p2=o_get(v2, v2_n, 86ULL, 5);uint64_t p3=o_get(v2, v2_n, 37ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 13ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p3=136ULL;uint64_t p4=o_get(v2, v2_n, 8ULL, 5);uint64_t p5=o_get(v2, v2_n, 86ULL, 5);f54(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
uint64_t __attribute__((unused)) v4=({uint64_t p1=120ULL;uint64_t p2=0ULL;f49(v2, v2_n, p1, p2);});
({uint64_t p2=32ULL;uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=v4;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 7ULL, 5);
{
uint64_t v6=0;
uint64_t c1=0;
for(;o_op(8,5, v6, v5);v6=o_op(1,5, v6,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t p1=120ULL;uint64_t p2=v6;f49(v2, v2_n, p1, p2);});
({uint64_t p2=v3;uint64_t p3=v7;uint64_t p4=v7;uint64_t p5=v3;f40(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
}
}
({uint64_t p2=30ULL;uint64_t p3=v3;uint64_t p4=o_get(v2, v2_n, 24ULL, 5);f51(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 26ULL, 5);
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 27ULL, 5);uint64_t p4=4ULL;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=0ULL;uint64_t p5=v6;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 28ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=v6;uint64_t p5=v3;f52(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({f38(v0, v0_n, v1, v1_n);});
({uint64_t p3=v3;uint64_t p4=0ULL;uint64_t p5=30ULL;f53(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 25ULL, 5);uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 6ULL, 5);f66(p0);});f22(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 85ULL, 5);
({uint64_t p2=v7;uint64_t p3=v3;uint64_t p4=o_get(v2, v2_n, 25ULL, 5);uint64_t p5=o_get(v2, v2_n, 36ULL, 5);f40(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=41ULL;uint64_t p3=v7;uint64_t p4=v7;f41(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v8=o_get(v2, v2_n, 87ULL, 5);
uint64_t __attribute__((unused)) v9=o_get(v2, v2_n, 88ULL, 5);
({uint64_t p2=v8;uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 14ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{
uint64_t v10=0;
uint64_t c1=0;
for(;o_op(8,5, v10, v5);v10=o_op(1,5, v10,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p2=32ULL;uint64_t p3=v9;uint64_t p4=({uint64_t p1=120ULL;uint64_t p2=v10;f49(v2, v2_n, p1, p2);});uint64_t p5=v7;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=32ULL;uint64_t p3=v9;uint64_t p4=v9;uint64_t p5=({uint64_t p1=136ULL;uint64_t p2=v10;f49(v2, v2_n, p1, p2);});f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=32ULL;uint64_t p3=v9;uint64_t p4=v9;uint64_t p5=o_get(v2, v2_n, 35ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=v10;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
({uint64_t p2=v8;uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=4ULL;uint64_t p5=v8;f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=v8;uint64_t p3=v9;f34(v0, v0_n, v1, v1_n, p2, p3);});
}
}
return o_leave(0);
}
static uint64_t f68(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
if(({uint64_t _a=({f60(v2, v2_n);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(1ULL);
}
({f61(v0, v0_n, v1, v1_n, v2, v2_n);});
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 1ULL, 5);
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({f62(v0, v0_n, v1, v1_n, v2, v2_n);});
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({f64(v0, v0_n, v1, v1_n, v2, v2_n);});
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({f65(v0, v0_n, v1, v1_n, v2, v2_n);});
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({f67(v0, v0_n, v1, v1_n, v2, v2_n);});
}
({f48(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
static uint64_t f69(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 4ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v3=768ULL;
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=256ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
{uint64_t _i=3ULL;uint64_t _v=({f17(v1, v1_n);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=35ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=22ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=51ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=49ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=50ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=52ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=53ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=40ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=46ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=47ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=24ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=29ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=48ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
({uint64_t p2=o_get(v1, v1_n, 3ULL, 5);uint64_t p3=856ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 10ULL, 5);uint64_t p3=896ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 11ULL, 5);uint64_t p3=904ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 35ULL, 5);uint64_t p3=912ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=33ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 21ULL, 5);uint64_t p3=37ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 22ULL, 5);uint64_t p3=38ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 51ULL, 5);uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=256ULL;uint64_t p5=o_get(v2, v2_n, 20ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 49ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=2ULL;uint64_t _b=v3;o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 51ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 50ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=v3;uint64_t p5=o_get(v2, v2_n, 51ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v4=v3;
if(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
({uint64_t p2=o_get(v2, v2_n, 29ULL, 5);uint64_t p3=v4;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 48ULL, 5);uint64_t p3=o_get(v2, v2_n, 29ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 49ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 52ULL, 5);uint64_t p3=o_get(v2, v2_n, 49ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 53ULL, 5);uint64_t p3=o_get(v2, v2_n, 48ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 40ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=11ULL;uint64_t p3=o_get(v2, v2_n, 46ULL, 5);uint64_t p4=o_get(v2, v2_n, 52ULL, 5);uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=11ULL;uint64_t p3=o_get(v2, v2_n, 47ULL, 5);uint64_t p4=o_get(v2, v2_n, 53ULL, 5);uint64_t p5=0ULL;f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 24ULL, 5);
({uint64_t p2=o_get(v2, v2_n, 29ULL, 5);uint64_t p3=3216550459ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=32ULL;uint64_t p3=v5;uint64_t p4=o_get(v2, v2_n, 46ULL, 5);uint64_t p5=o_get(v2, v2_n, 29ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=47ULL;uint64_t p3=v5;uint64_t p4=v5;f41(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 29ULL, 5);uint64_t p3=1065353216ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=30ULL;uint64_t p3=v5;uint64_t p4=v5;uint64_t p5=o_get(v2, v2_n, 29ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=40ULL;uint64_t p3=v5;uint64_t p4=v5;f41(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=v5;uint64_t p4=o_get(v2, v2_n, 46ULL, 5);uint64_t p5=v5;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=32ULL;uint64_t p3=v5;uint64_t p4=v5;uint64_t p5=o_get(v2, v2_n, 47ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=32ULL;uint64_t p3=v5;uint64_t p4=v5;uint64_t p5=o_get(v2, v2_n, 35ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 40ULL, 5);uint64_t p3=v5;f34(v0, v0_n, v1, v1_n, p2, p3);});
({f48(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
static uint64_t f70(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
{uint64_t _i=3ULL;uint64_t _v=({f17(v1, v1_n);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=27ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=28ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=23ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=32ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=33ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=24ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=25ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=26ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=62ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=148ULL;uint64_t _v=({f16(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=148ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({f16(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=63ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=64ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=65ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=66ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=67ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=68ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=57ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(v3){
{uint64_t _i=150ULL;uint64_t _v=({f16(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=150ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({f16(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
({uint64_t p2=o_get(v1, v1_n, 3ULL, 5);uint64_t p3=856ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 10ULL, 5);uint64_t p3=896ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 11ULL, 5);uint64_t p3=904ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 12ULL, 5);uint64_t p3=912ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 13ULL, 5);uint64_t p3=920ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=33ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 21ULL, 5);uint64_t p3=37ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=1ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 68ULL, 5);uint64_t p3=1258291200ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 27ULL, 5);uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=5ULL;f28(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 28ULL, 5);uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=31ULL;f29(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 32ULL, 5);uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=256ULL;uint64_t p5=o_get(v2, v2_n, 20ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 33ULL, 5);uint64_t p3=o_get(v2, v2_n, 32ULL, 5);uint64_t p4=32ULL;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 62ULL, 5);uint64_t p3=o_get(v2, v2_n, 33ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=57ULL;uint64_t p3=o_get(v2, v2_n, 148ULL, 5);uint64_t p4=o_get(v2, v2_n, 62ULL, 5);uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=57ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=148ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p4=o_get(v2, v2_n, 62ULL, 5);uint64_t p5=16ULL;f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(v3){
({uint64_t p2=o_get(v2, v2_n, 62ULL, 5);uint64_t p3=o_get(v2, v2_n, 33ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=57ULL;uint64_t p3=o_get(v2, v2_n, 150ULL, 5);uint64_t p4=o_get(v2, v2_n, 62ULL, 5);uint64_t p5=0ULL;f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=57ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=150ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p4=o_get(v2, v2_n, 62ULL, 5);uint64_t p5=16ULL;f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
return o_leave(0);
}
static uint64_t f71(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 63ULL, 5);
({uint64_t p2=v4;uint64_t p3=({uint64_t p1=148ULL;uint64_t p2=0ULL;f49(v2, v2_n, p1, p2);});f27(v0, v0_n, v1, v1_n, p2, p3);});
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v3);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
({uint64_t p2=37ULL;uint64_t p3=v4;uint64_t p4=v4;uint64_t p5=({uint64_t p1=148ULL;uint64_t p2=v5;f49(v2, v2_n, p1, p2);});f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
}
}
({uint64_t p2=37ULL;uint64_t p3=v4;uint64_t p4=o_get(v2, v2_n, 24ULL, 5);f51(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 26ULL, 5);
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, 27ULL, 5);uint64_t p4=4ULL;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=0ULL;uint64_t p5=v5;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 28ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=v5;uint64_t p5=v4;f52(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({f38(v0, v0_n, v1, v1_n);});
({uint64_t p3=o_get(v2, v2_n, 64ULL, 5);uint64_t p4=0ULL;uint64_t p5=37ULL;f53(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f72(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 57ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 67ULL, 5);
{
uint64_t v7=0;
uint64_t c1=0;
for(;o_op(8,5, v7, v3);v7=o_op(1,5, v7,1)){
if(c1++>=1000000)o_trap(4);
if(v4){
({uint64_t p2=v5;uint64_t p3=({uint64_t p1=150ULL;uint64_t p2=v7;f49(v2, v2_n, p1, p2);});f27(v0, v0_n, v1, v1_n, p2, p3);});
}else{
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, 32ULL, 5);uint64_t p4=8ULL;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=v7;uint64_t p5=v5;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=v5;uint64_t p3=v5;uint64_t p4=o_get(v2, v2_n, 68ULL, 5);uint64_t p5=252ULL;f31(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p3=o_get(v2, v2_n, 65ULL, 5);uint64_t p4=({uint64_t p1=148ULL;uint64_t p2=v7;f49(v2, v2_n, p1, p2);});uint64_t p5=o_get(v2, v2_n, 64ULL, 5);f50(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 66ULL, 5);uint64_t p3=o_get(v2, v2_n, 65ULL, 5);uint64_t p4=805306368ULL;uint64_t p5=v5;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 66ULL, 5);f27(v0, v0_n, v1, v1_n, p2, p3);});
}else{
({uint64_t p2=36ULL;uint64_t p3=v6;uint64_t p4=v6;uint64_t p5=o_get(v2, v2_n, 66ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
}
}
({uint64_t p2=36ULL;uint64_t p3=v6;uint64_t p4=o_get(v2, v2_n, 24ULL, 5);f51(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 26ULL, 5);
({uint64_t p2=v7;uint64_t p3=o_get(v2, v2_n, 27ULL, 5);uint64_t p4=4ULL;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v7;uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=64ULL;uint64_t p5=v7;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 28ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=v7;uint64_t p5=v6;f52(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({f38(v0, v0_n, v1, v1_n);});
({uint64_t p3=v6;uint64_t p4=64ULL;uint64_t p5=36ULL;f53(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p2=v6;uint64_t p3=v6;uint64_t p4=8388607ULL;f29(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v8=({f46(v0, v0_n, v1, v1_n);});
uint64_t __attribute__((unused)) v9=({f15(v1, v1_n);});
({uint64_t p2=v9;uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 12ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v9;uint64_t p3=o_get(v2, v2_n, 64ULL, 5);f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v9;uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 13ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v9;uint64_t p3=v6;f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v8;f47(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f73(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=8ULL;
if(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v4=7ULL;
}
({uint64_t p3=v3;f70(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
({uint64_t p3=v4;f71(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p3=v4;uint64_t p4=v3;f72(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({f48(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
static uint64_t f74(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 4ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 2ULL, 5);
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
{uint64_t _i=3ULL;uint64_t _v=({f17(v1, v1_n);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=22ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=31ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=30ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=55ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=56ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=57ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=58ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=59ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=60ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=54ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
({uint64_t p2=o_get(v1, v1_n, 3ULL, 5);uint64_t p3=856ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 10ULL, 5);uint64_t p3=896ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 11ULL, 5);uint64_t p3=904ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 12ULL, 5);uint64_t p3=912ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=33ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 21ULL, 5);uint64_t p3=37ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 22ULL, 5);uint64_t p3=38ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=256ULL;uint64_t p5=o_get(v2, v2_n, 20ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=v3;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=o_get(v2, v2_n, 30ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v5=({f46(v0, v0_n, v1, v1_n);});
({uint64_t p2=o_get(v2, v2_n, 55ULL, 5);uint64_t p3=o_get(v2, v2_n, 31ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=11ULL;uint64_t p3=o_get(v2, v2_n, 56ULL, 5);uint64_t p4=o_get(v2, v2_n, 55ULL, 5);uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 57ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=v4;uint64_t p5=o_get(v2, v2_n, 56ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=1ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 57ULL, 5);uint64_t p3=o_get(v2, v2_n, 30ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 57ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=o_get(v2, v2_n, 58ULL, 5);uint64_t p3=o_get(v2, v2_n, 57ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=11ULL;uint64_t p3=o_get(v2, v2_n, 59ULL, 5);uint64_t p4=o_get(v2, v2_n, 58ULL, 5);uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 60ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);uint64_t p4=v3;uint64_t p5=o_get(v2, v2_n, 31ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 54ULL, 5);uint64_t p3=o_get(v2, v2_n, 60ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 12ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 54ULL, 5);uint64_t p3=o_get(v2, v2_n, 59ULL, 5);f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v5;f47(v0, v0_n, v1, v1_n, p2);});
({f48(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
static uint64_t f75(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 31ULL, 5);
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=22ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=61ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 22ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 61ULL, 5);
({uint64_t p2=v4;uint64_t p3=38ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v5;uint64_t p3=v4;uint64_t p4=o_get(v2, v2_n, 4ULL, 5);uint64_t p5=v3;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 40ULL, 5);uint64_t p3=v5;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 54ULL, 5);uint64_t p3=v5;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 12ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v5;uint64_t p3=v4;uint64_t p4=o_get(v2, v2_n, 2ULL, 5);uint64_t p5=v3;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 52ULL, 5);uint64_t p3=v5;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}else{
({uint64_t p2=o_get(v2, v2_n, 40ULL, 5);uint64_t p3=v3;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 52ULL, 5);uint64_t p3=v3;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 54ULL, 5);uint64_t p3=v3;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 12ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
return o_leave(0);
}
static uint64_t f76(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=3ULL;uint64_t _v=({f17(v1, v1_n);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=30ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=31ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=40ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=52ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=54ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=45ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=46ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=24ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=29ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
({uint64_t p2=o_get(v1, v1_n, 3ULL, 5);uint64_t p3=856ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 10ULL, 5);uint64_t p3=896ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 11ULL, 5);uint64_t p3=904ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 12ULL, 5);uint64_t p3=912ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=920ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=33ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 21ULL, 5);uint64_t p3=37ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=256ULL;uint64_t p5=o_get(v2, v2_n, 20ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=o_get(v2, v2_n, 30ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v3=({f46(v0, v0_n, v1, v1_n);});
({f75(v0, v0_n, v1, v1_n, v2, v2_n);});
({uint64_t p2=11ULL;uint64_t p3=o_get(v2, v2_n, 45ULL, 5);uint64_t p4=o_get(v2, v2_n, 40ULL, 5);uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=11ULL;uint64_t p3=o_get(v2, v2_n, 46ULL, 5);uint64_t p4=o_get(v2, v2_n, 52ULL, 5);uint64_t p5=0ULL;f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 24ULL, 5);
if(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=v4;uint64_t p3=o_get(v2, v2_n, 45ULL, 5);f27(v0, v0_n, v1, v1_n, p2, p3);});
}else{
({uint64_t p2=o_get(v2, v2_n, 29ULL, 5);uint64_t p3=3216550459ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=32ULL;uint64_t p3=v4;uint64_t p4=o_get(v2, v2_n, 46ULL, 5);uint64_t p5=o_get(v2, v2_n, 29ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=47ULL;uint64_t p3=v4;uint64_t p4=v4;f41(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 29ULL, 5);uint64_t p3=1065353216ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=30ULL;uint64_t p3=v4;uint64_t p4=v4;uint64_t p5=o_get(v2, v2_n, 29ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=40ULL;uint64_t p3=v4;uint64_t p4=v4;f41(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=v4;uint64_t p4=o_get(v2, v2_n, 45ULL, 5);uint64_t p5=v4;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=o_get(v2, v2_n, 54ULL, 5);uint64_t p3=v4;f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v3;f47(v0, v0_n, v1, v1_n, p2);});
({f48(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
static uint64_t f77(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=3ULL;uint64_t _v=({f17(v1, v1_n);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=30ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=31ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=41ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=70ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=69ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=29ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
({uint64_t p2=o_get(v1, v1_n, 3ULL, 5);uint64_t p3=856ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 10ULL, 5);uint64_t p3=896ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 11ULL, 5);uint64_t p3=920ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=936ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=33ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 21ULL, 5);uint64_t p3=37ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=256ULL;uint64_t p5=o_get(v2, v2_n, 20ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=o_get(v2, v2_n, 30ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v3=({f46(v0, v0_n, v1, v1_n);});
({uint64_t p2=o_get(v2, v2_n, 41ULL, 5);uint64_t p3=o_get(v2, v2_n, 31ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 70ULL, 5);uint64_t p3=o_get(v2, v2_n, 31ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 69ULL, 5);
({uint64_t p2=11ULL;uint64_t p3=v4;uint64_t p4=o_get(v2, v2_n, 41ULL, 5);uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 29ULL, 5);uint64_t p3=1602224128ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=32ULL;uint64_t p3=v4;uint64_t p4=v4;uint64_t p5=o_get(v2, v2_n, 29ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 70ULL, 5);uint64_t p3=v4;f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v3;f47(v0, v0_n, v1, v1_n, p2);});
({f48(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
static uint64_t f78(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=3ULL;uint64_t _v=({f17(v1, v1_n);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=30ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=27ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=28ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=23ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=24ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=26ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=80ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=81ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=82ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=83ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=29ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=31ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=71ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=41ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=72ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=70ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=73ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=74ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=75ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=76ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=77ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=78ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=79ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=152ULL;uint64_t p3=2ULL;f55(v1, v1_n, v2, v2_n, p2, p3);});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
({uint64_t p2=o_get(v1, v1_n, 3ULL, 5);uint64_t p3=856ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 10ULL, 5);uint64_t p3=896ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 11ULL, 5);uint64_t p3=904ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 12ULL, 5);uint64_t p3=912ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 13ULL, 5);uint64_t p3=920ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 14ULL, 5);uint64_t p3=928ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=936ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=33ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 21ULL, 5);uint64_t p3=37ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=1ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 27ULL, 5);uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=5ULL;f28(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 28ULL, 5);uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=31ULL;f29(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 71ULL, 5);uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=32ULL;uint64_t p5=o_get(v2, v2_n, 12ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p3=152ULL;uint64_t p4=2ULL;uint64_t p5=o_get(v2, v2_n, 71ULL, 5);f54(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 80ULL, 5);
({uint64_t p2=v3;f23(v0, v0_n, v1, v1_n, p2);});
{
uint64_t v4=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=({uint64_t p1=152ULL;uint64_t p2=v4;f49(v2, v2_n, p1, p2);});
({uint64_t p2=v3;uint64_t p3=v5;uint64_t p4=v5;uint64_t p5=v3;f40(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
}
({uint64_t p2=30ULL;uint64_t p3=v3;uint64_t p4=o_get(v2, v2_n, 24ULL, 5);f51(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 26ULL, 5);
({uint64_t p2=v4;uint64_t p3=o_get(v2, v2_n, 27ULL, 5);uint64_t p4=4ULL;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 28ULL, 5);uint64_t p3=o_get(v2, v2_n, 23ULL, 5);uint64_t p4=v4;uint64_t p5=v3;f52(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({f38(v0, v0_n, v1, v1_n);});
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 81ULL, 5);
({uint64_t p3=v5;uint64_t p4=0ULL;uint64_t p5=30ULL;f53(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 29ULL, 5);
({uint64_t p2=v6;uint64_t p3=226492416ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=30ULL;uint64_t p3=v5;uint64_t p4=v5;uint64_t p5=v6;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=41ULL;uint64_t p3=o_get(v2, v2_n, 82ULL, 5);uint64_t p4=v5;f41(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=o_get(v2, v2_n, 83ULL, 5);uint64_t p4=v5;uint64_t p5=o_get(v2, v2_n, 82ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;uint64_t p3=1065361408ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=32ULL;uint64_t p3=o_get(v2, v2_n, 83ULL, 5);uint64_t p4=o_get(v2, v2_n, 83ULL, 5);uint64_t p5=v6;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f79(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 31ULL, 5);
({uint64_t p2=v3;uint64_t p3=o_get(v2, v2_n, 21ULL, 5);uint64_t p4=256ULL;uint64_t p5=o_get(v2, v2_n, 20ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v3;uint64_t p3=o_get(v2, v2_n, 30ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v4=({f46(v0, v0_n, v1, v1_n);});
({uint64_t p2=o_get(v2, v2_n, 41ULL, 5);uint64_t p3=v3;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 72ULL, 5);uint64_t p3=v3;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 11ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 70ULL, 5);uint64_t p3=v3;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 13ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 73ULL, 5);uint64_t p3=v3;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 14ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 74ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 76ULL, 5);
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 29ULL, 5);
({uint64_t p2=11ULL;uint64_t p3=v5;uint64_t p4=o_get(v2, v2_n, 41ULL, 5);uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=11ULL;uint64_t p3=o_get(v2, v2_n, 75ULL, 5);uint64_t p4=o_get(v2, v2_n, 72ULL, 5);uint64_t p5=0ULL;f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
if(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=v6;f23(v0, v0_n, v1, v1_n, p2);});
}else{
({uint64_t p2=32ULL;uint64_t p3=v6;uint64_t p4=o_get(v2, v2_n, 75ULL, 5);uint64_t p5=o_get(v2, v2_n, 83ULL, 5);f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=o_get(v2, v2_n, 77ULL, 5);uint64_t p3=v5;uint64_t p4=2147483647ULL;f29(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v7;uint64_t p3=897581056ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 77ULL, 5);uint64_t p4=v7;uint64_t p5=v6;f40(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v7;uint64_t p3=226492416ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=30ULL;uint64_t p3=v6;uint64_t p4=v6;uint64_t p5=v7;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=30ULL;uint64_t p3=o_get(v2, v2_n, 79ULL, 5);uint64_t p4=v5;uint64_t p5=v6;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v7;uint64_t p3=2147483648ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v6;uint64_t p3=v6;uint64_t p4=v7;f30(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=30ULL;uint64_t p3=o_get(v2, v2_n, 78ULL, 5);uint64_t p4=v5;uint64_t p5=v6;f39(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 70ULL, 5);uint64_t p3=o_get(v2, v2_n, 78ULL, 5);f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 73ULL, 5);uint64_t p3=o_get(v2, v2_n, 79ULL, 5);f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v4;f47(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f80(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 1ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 3ULL, 5);
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f77(v0, v0_n, v1, v1_n, v2, v2_n);}));
}
({f78(v0, v0_n, v1, v1_n, v2, v2_n);});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
({f79(v0, v0_n, v1, v1_n, v2, v2_n);});
({f48(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
static uint64_t f81(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
{uint64_t _i=3ULL;uint64_t _v=({f17(v1, v1_n);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=15ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=16ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=94ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=95ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=96ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=23ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=34ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=97ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=98ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=99ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=33ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=100ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=101ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=41ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=102ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=103ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=104ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=105ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=106ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=107ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=25ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=108ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=109ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=110ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=111ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=112ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=113ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=114ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=115ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=116ULL;uint64_t _v=({f14(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=58ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=54ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=55ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=117ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=74ULL;uint64_t _v=({f15(v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
({uint64_t p2=o_get(v1, v1_n, 3ULL, 5);uint64_t p3=856ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 10ULL, 5);uint64_t p3=896ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 11ULL, 5);uint64_t p3=904ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 12ULL, 5);uint64_t p3=912ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 13ULL, 5);uint64_t p3=920ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 14ULL, 5);uint64_t p3=928ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 15ULL, 5);uint64_t p3=936ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 16ULL, 5);uint64_t p3=944ULL;f19(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 94ULL, 5);uint64_t p3=952ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 95ULL, 5);uint64_t p3=956ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 20ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 96ULL, 5);
({uint64_t p2=v5;uint64_t p3=33ULL;f21(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 23ULL, 5);uint64_t p3=1ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 34ULL, 5);f23(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=o_get(v2, v2_n, 97ULL, 5);uint64_t p3=512ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 95ULL, 5);uint64_t p4=v3;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 103ULL, 5);uint64_t p3=v5;uint64_t p4=v6;f56(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 98ULL, 5);
({uint64_t p2=v7;uint64_t p3=v5;uint64_t p4=v4;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v7;uint64_t p3=v7;uint64_t p4=16ULL;f28(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 99ULL, 5);uint64_t p3=v7;uint64_t p4=({uint64_t _a=4294967296ULL;uint64_t _b=v3;o_op(2, 5, _a,_b);});uint64_t p5=v5;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 100ULL, 5);uint64_t p3=4294967295ULL;f22(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 41ULL, 5);uint64_t p3=v5;uint64_t p4=4ULL;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f82(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 20ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 96ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 33ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 100ULL, 5);
({uint64_t p2=v3;uint64_t p3=v4;f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v7=({f46(v0, v0_n, v1, v1_n);});
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, 98ULL, 5);uint64_t p4=o_get(v2, v2_n, 94ULL, 5);uint64_t p5=18446744073709551615ULL;f25(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, 99ULL, 5);uint64_t p4=4ULL;uint64_t p5=v5;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 58ULL, 5);uint64_t p3=v5;uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 10ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=11ULL;uint64_t p3=v6;uint64_t p4=o_get(v2, v2_n, 58ULL, 5);uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 41ULL, 5);uint64_t p3=v6;f37(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v7;f47(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=o_get(v2, v2_n, 101ULL, 5);uint64_t p3=v6;uint64_t p4=128ULL;uint64_t p5=18446744073709551615ULL;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}else{
({uint64_t p2=o_get(v2, v2_n, 101ULL, 5);uint64_t p3=v6;uint64_t p4=128ULL;uint64_t p5=v3;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({f38(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f83(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 23ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 104ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 102ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 107ULL, 5);
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 108ULL, 5);
({uint64_t p2=o_get(v2, v2_n, 109ULL, 5);f23(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=o_get(v2, v2_n, 110ULL, 5);f23(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=o_get(v2, v2_n, 111ULL, 5);f23(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v4;f23(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v5;f23(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v8=o_get(v1, v1_n, 0ULL, 5);
({f11(v1, v1_n);});
({uint64_t p2=v4;uint64_t p3=o_get(v2, v2_n, 96ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v9=({f45(v0, v0_n, v1, v1_n);});
({uint64_t p2=o_get(v2, v2_n, 105ULL, 5);uint64_t p3=v5;f36(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 106ULL, 5);uint64_t p3=o_get(v2, v2_n, 105ULL, 5);uint64_t p4=128ULL;uint64_t p5=v4;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 106ULL, 5);uint64_t p4=o_get(v2, v2_n, 101ULL, 5);f56(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 109ULL, 5);uint64_t p3=v6;uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 109ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 105ULL, 5);uint64_t p4=o_get(v2, v2_n, 100ULL, 5);f56(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 25ULL, 5);uint64_t p3=o_get(v2, v2_n, 100ULL, 5);uint64_t p4=o_get(v2, v2_n, 105ULL, 5);f56(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v7;uint64_t p3=v6;uint64_t p4=4294967295ULL;uint64_t p5=v3;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v7;uint64_t p3=o_get(v2, v2_n, 25ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=v7;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 110ULL, 5);uint64_t p3=v7;uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 110ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;uint64_t p3=v4;uint64_t p4=o_get(v2, v2_n, 20ULL, 5);f56(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v6;uint64_t p3=v6;uint64_t p4=v7;f57(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 111ULL, 5);uint64_t p3=v6;uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 111ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v4;uint64_t p3=v3;uint64_t p4=1ULL;uint64_t p5=v4;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v5;uint64_t p3=v3;uint64_t p4=4ULL;uint64_t p5=v5;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v8;f44(v0, v0_n, v1, v1_n, p2);});
({f12(v1, v1_n);});
({uint64_t p1=v9;uint64_t p2=o_get(v1, v1_n, 0ULL, 5);f10(v0, v0_n, p1, p2);});
({f84(v0, v0_n, v1, v1_n, v2, v2_n);});
return o_leave(0);
}
static uint64_t f84(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 23ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 112ULL, 5);
({uint64_t p2=v4;uint64_t p3=v3;uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 111ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v4;uint64_t p3=v4;uint64_t p4=31ULL;f28(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v4;uint64_t p3=v4;uint64_t p4=o_get(v2, v2_n, 103ULL, 5);f57(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 41ULL, 5);uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 97ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=o_get(v2, v2_n, 96ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v5=({f46(v0, v0_n, v1, v1_n);});
({uint64_t p2=o_get(v2, v2_n, 41ULL, 5);uint64_t p3=v4;f37(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v5;f47(v0, v0_n, v1, v1_n, p2);});
({f38(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f85(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 23ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 104ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 102ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 107ULL, 5);
({uint64_t p2=o_get(v2, v2_n, 114ULL, 5);f23(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=o_get(v2, v2_n, 115ULL, 5);f23(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v4;f23(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v5;f23(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v7=o_get(v1, v1_n, 0ULL, 5);
({f11(v1, v1_n);});
({uint64_t p2=v4;uint64_t p3=o_get(v2, v2_n, 96ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v8=({f45(v0, v0_n, v1, v1_n);});
({uint64_t p2=o_get(v2, v2_n, 105ULL, 5);uint64_t p3=v5;f36(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 41ULL, 5);uint64_t p3=v3;uint64_t p4=512ULL;uint64_t p5=v5;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 113ULL, 5);uint64_t p3=o_get(v2, v2_n, 41ULL, 5);f36(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=o_get(v2, v2_n, 115ULL, 5);uint64_t p3=o_get(v2, v2_n, 113ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 115ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 105ULL, 5);uint64_t p4=o_get(v2, v2_n, 100ULL, 5);f56(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v6;uint64_t p3=v6;uint64_t p4=o_get(v2, v2_n, 113ULL, 5);f57(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 114ULL, 5);uint64_t p3=v6;uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 114ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v4;uint64_t p3=v3;uint64_t p4=1ULL;uint64_t p5=v4;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v5;uint64_t p3=v3;uint64_t p4=4ULL;uint64_t p5=v5;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v7;f44(v0, v0_n, v1, v1_n, p2);});
({f12(v1, v1_n);});
({uint64_t p1=v8;uint64_t p2=o_get(v1, v1_n, 0ULL, 5);f10(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v9=o_get(v2, v2_n, 54ULL, 5);
({uint64_t p2=o_get(v2, v2_n, 20ULL, 5);uint64_t p3=o_get(v2, v2_n, 96ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v10=({f46(v0, v0_n, v1, v1_n);});
({uint64_t p2=v9;uint64_t p3=o_get(v2, v2_n, 109ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 15ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v9;uint64_t p3=o_get(v2, v2_n, 98ULL, 5);f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v9;uint64_t p3=o_get(v2, v2_n, 109ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 16ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v9;uint64_t p3=o_get(v2, v2_n, 20ULL, 5);f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v10;f47(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f86(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 58ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 54ULL, 5);
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 117ULL, 5);
uint64_t __attribute__((unused)) v8=o_get(v2, v2_n, 74ULL, 5);
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, 33ULL, 5);uint64_t p4=1ULL;uint64_t p5=v3;f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=56ULL;uint64_t p3=v7;uint64_t p4=v5;uint64_t p5=0ULL;f32(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=56ULL;uint64_t p3=v8;uint64_t p4=v5;uint64_t p5=8ULL;f33(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 114ULL, 5);uint64_t p4=32ULL;uint64_t p5=v4;f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p3=v6;uint64_t p4=0ULL;uint64_t p5=({uint64_t p0=v7;uint64_t p1=0ULL;f13(p0, p1);});f58(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p3=v6;uint64_t p4=4ULL;uint64_t p5=({uint64_t p0=v7;uint64_t p1=1ULL;f13(p0, p1);});f58(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p3=v6;uint64_t p4=8ULL;uint64_t p5=({uint64_t p0=v8;uint64_t p1=0ULL;f13(p0, p1);});f58(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p3=v6;uint64_t p4=12ULL;uint64_t p5=({uint64_t p0=v8;uint64_t p1=1ULL;f13(p0, p1);});f58(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p3=v6;uint64_t p4=16ULL;uint64_t p5=o_get(v2, v2_n, 109ULL, 5);f58(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p3=v6;uint64_t p4=20ULL;uint64_t p5=o_get(v2, v2_n, 110ULL, 5);f58(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p3=v6;uint64_t p4=24ULL;uint64_t p5=o_get(v2, v2_n, 34ULL, 5);f58(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p3=v6;uint64_t p4=28ULL;uint64_t p5=o_get(v2, v2_n, 34ULL, 5);f58(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f87(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 54ULL, 5);
({uint64_t p2=v4;uint64_t p3=o_get(v2, v2_n, 20ULL, 5);uint64_t p4=32ULL;uint64_t p5=v3;f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{
uint64_t v5=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
({uint64_t p3=v4;uint64_t p4=({uint64_t _a=v5;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 34ULL, 5);f58(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
}
}
return o_leave(0);
}
static uint64_t f88(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 23ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 34ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 33ULL, 5);
if(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=o_get(v2, v2_n, 110ULL, 5);uint64_t p3=v3;uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 110ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
({uint64_t p2=v4;uint64_t p3=o_get(v2, v2_n, 112ULL, 5);f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v6=({f46(v0, v0_n, v1, v1_n);});
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, 98ULL, 5);uint64_t p4=o_get(v2, v2_n, 94ULL, 5);uint64_t p5=18446744073709551615ULL;f25(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, 99ULL, 5);uint64_t p4=16ULL;uint64_t p5=v5;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p3=o_get(v2, v2_n, 11ULL, 5);uint64_t p4=o_get(v2, v2_n, 13ULL, 5);f86(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=o_get(v2, v2_n, 12ULL, 5);uint64_t p4=o_get(v2, v2_n, 14ULL, 5);f86(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p2=v6;f47(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 107ULL, 5);
uint64_t __attribute__((unused)) v8=o_get(v2, v2_n, 25ULL, 5);
uint64_t __attribute__((unused)) v9=o_get(v2, v2_n, 116ULL, 5);
uint64_t __attribute__((unused)) v10=o_get(v2, v2_n, 20ULL, 5);
if(({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=v7;uint64_t p3=v10;uint64_t p4=o_get(v2, v2_n, 115ULL, 5);f56(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v7;uint64_t p3=v7;uint64_t p4=4294967295ULL;uint64_t p5=v3;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v9;uint64_t p3=v7;uint64_t p4=o_get(v2, v2_n, 103ULL, 5);f57(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v4;uint64_t p3=v9;f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v11=({f46(v0, v0_n, v1, v1_n);});
({uint64_t p3=o_get(v2, v2_n, 13ULL, 5);f87(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p3=o_get(v2, v2_n, 14ULL, 5);f87(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=v11;f47(v0, v0_n, v1, v1_n, p2);});
}
({uint64_t p2=v8;uint64_t p3=v3;uint64_t p4=8ULL;uint64_t p5=o_get(v2, v2_n, 96ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v7;uint64_t p3=v10;uint64_t p4=v8;f56(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v8;uint64_t p3=o_get(v2, v2_n, 103ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=v3;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v9;uint64_t p3=v7;uint64_t p4=v8;f57(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v4;uint64_t p3=v9;f43(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v11=({f46(v0, v0_n, v1, v1_n);});
uint64_t __attribute__((unused)) v12=o_get(v2, v2_n, 54ULL, 5);
({uint64_t p2=v12;uint64_t p3=v10;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 15ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v12;uint64_t p3=v4;f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v12;uint64_t p3=v10;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 16ULL, 5);f26(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v12;uint64_t p3=v4;f34(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v11;f47(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f89(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 5ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v3=8ULL;
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 2ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=({uint64_t _a=65536ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=v3;o_op(4, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=v3;o_op(3, 5, _a,_b);});uint64_t _b=65536ULL;o_op(2, 5, _a,_b);});uint64_t _b=256ULL;o_op(11, 5, _a,_b);})){
return o_leave(1ULL);
}
({uint64_t p3=v3;uint64_t p4=v4;f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
({f82(v0, v0_n, v1, v1_n, v2, v2_n);});
({f83(v0, v0_n, v1, v1_n, v2, v2_n);});
({f85(v0, v0_n, v1, v1_n, v2, v2_n);});
({f88(v0, v0_n, v1, v1_n, v2, v2_n);});
({f48(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
static uint64_t f90(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=4294967295ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=4294967295ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
uint64_t v6[256]={0};
uint64_t v6_n=256;
o_fill(v6, v6_n, 0ULL, 5);
{uint64_t _i=0ULL;uint64_t _v=v0;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=v3;o_put(v6, v6_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=1ULL;
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v7=({f59(v4, v4_n, v5, v5_n, v6, v6_n);});
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v7=({f68(v4, v4_n, v5, v5_n, v6, v6_n);});
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v7=({f69(v4, v4_n, v5, v5_n, v6, v6_n);});
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v7=({f73(v4, v4_n, v5, v5_n, v6, v6_n);});
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
v7=({f74(v4, v4_n, v5, v5_n, v6, v6_n);});
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
v7=({f76(v4, v4_n, v5, v5_n, v6, v6_n);});
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
v7=({f89(v4, v4_n, v5, v5_n, v6, v6_n);});
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
v7=({f80(v4, v4_n, v5, v5_n, v6, v6_n);});
}
if(({uint64_t _a=v7;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v7;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v5, v5_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=o_get(v5, v5_n, 9ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
uint32_t omega_cg_dec(uint64_t  *result, volatile uint64_t p0, volatile uint64_t p1, volatile uint64_t p2, volatile uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n){
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
  o_busy=1;o_depth=0;int code=setjmp(o_jmp);if(!code){uint64_t _result=f90(p0, p1, p2, p3, p4, p4_n, p5, p5_n);o_put(result,1,0,_result,5);}
  o_busy=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
