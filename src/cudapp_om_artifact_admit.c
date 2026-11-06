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
static uint64_t f1(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f2(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n);
static uint64_t f3(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n);
static uint64_t f4(uint64_t v0, const void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f5(uint64_t v0);
static uint64_t f6(uint64_t v0);
static uint64_t f7(uint64_t v0);
static uint64_t f8(uint64_t v0);
static uint64_t f9(uint64_t v0);
static uint64_t f10(uint64_t v0);
static uint64_t f11(uint64_t v0);
static uint64_t f12(uint64_t v0);
static uint64_t f13(uint64_t v0);
static uint64_t f14(uint64_t v0);
static uint64_t f15(uint64_t v0);
static uint64_t f16(uint64_t v0);
static uint64_t f17(uint64_t v0);
static uint64_t f18(uint64_t v0);
static uint64_t f19(uint64_t v0);
static uint64_t f20(uint64_t v0);
static uint64_t f21(uint64_t v0);
static uint64_t f22(uint64_t v0);
static uint64_t f23(uint64_t v0);
static uint64_t f24(uint64_t v0);
static uint64_t f25(uint64_t v0);
static uint64_t f26(uint64_t v0);
static uint64_t f27(uint64_t v0);
static uint64_t f28(uint64_t v0);
static uint64_t f29(uint64_t v0);
static uint64_t f30(uint64_t v0);
static uint64_t f31(uint64_t v0);
static uint64_t f32(uint64_t v0);
static uint64_t f33(uint64_t v0);
static uint64_t f34(uint64_t v0);
static uint64_t f35(uint64_t v0);
static uint64_t f36(uint64_t v0);
static uint64_t f37(uint64_t v0);
static uint64_t f38(uint64_t v0);
static uint64_t f39(uint64_t v0);
static uint64_t f40(uint64_t v0);
static uint64_t f41(uint64_t v0, uint64_t v1);
static uint64_t f42(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f43(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f44(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f45(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f46(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f47(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f48(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f49(uint64_t v0, uint64_t v1);
static uint64_t f50(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f51(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f52(uint64_t v0, uint64_t v1);
static uint64_t f53(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f54(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f55(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f56(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f57(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f58(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f59(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f60(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f61(uint64_t v0, uint64_t v1);
static uint64_t f62(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n);
static uint64_t f63(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f64(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f65(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f0(uint64_t v0, uint64_t v1, void *v2, uint64_t v2_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=15ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
v3=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(1, 5, _a,_b);});
}else{
v3=({uint64_t _a=v3;uint64_t _b=87ULL;o_op(1, 5, _a,_b);});
}
if(({uint64_t _a=v4;uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=48ULL;o_op(1, 5, _a,_b);});
}else{
v4=({uint64_t _a=v4;uint64_t _b=87ULL;o_op(1, 5, _a,_b);});
}
return o_leave(({uint64_t _a=v3;uint64_t _b=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=32ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=127ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=64ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(2ULL);
}
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=64ULL;o_op(11, 5, _a,_b);})){
return o_leave(2ULL);
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=48ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=57ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=97ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=102ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
v3=1ULL;
}
if(({uint64_t _a=v3;uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=2ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=3ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v2, v2_n, 5ULL, 5);uint64_t _b=63ULL;o_op(11, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
}
}
{uint64_t _i=5ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 5ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=48ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=57ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=9ULL;o_op(11, 5, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=10ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=48ULL;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 2ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=o_get(v2, v2_n, 3ULL, 5);o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=4ULL;uint64_t _v=o_get(v2, v2_n, 1ULL, 5);o_put(v2, v2_n, _i,_v,5);}
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v3=44ULL;
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v3=46ULL;
}
if(({uint64_t _a=v1;uint64_t _b=v3;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 2ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=o_get(v2, v2_n, 3ULL, 5);o_op(11, 5, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=4ULL;uint64_t _v=o_get(v2, v2_n, 1ULL, 5);o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 2ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(3ULL);
}
static uint64_t f1(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, v2, 2));uint64_t _b=({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2));uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2));uint64_t _b=({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2));uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
return o_leave(({uint64_t _a=v3;uint64_t _b=({uint64_t _a=v4;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f2(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=({uint64_t p1=12ULL;f1(v0, v0_n, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=12ULL;f1(v1, v1_n, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t p1=15ULL;f1(v1, v1_n, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p1=16ULL;f1(v1, v1_n, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=32ULL;f1(v1, v1_n, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t p1=33ULL;f1(v1, v1_n, p1);});uint64_t _b=({uint64_t _a=4294967295ULL;uint64_t _b=({uint64_t _a=10ULL;uint64_t _b=21ULL;o_op(15, 5, _a,_b);});o_op(14, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=42ULL;f1(v1, v1_n, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t p1=43ULL;f1(v1, v1_n, p1);});uint64_t _b=524287ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=44ULL;f1(v1, v1_n, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t p1=45ULL;f1(v1, v1_n, p1);});uint64_t _b=524287ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=52ULL;f1(v1, v1_n, p1);});uint64_t _b=12ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t p1=53ULL;f1(v1, v1_n, p1);});uint64_t _b=524287ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=56ULL;f1(v1, v1_n, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t p1=57ULL;f1(v1, v1_n, p1);});uint64_t _b=524287ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p1=59ULL;f1(v1, v1_n, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
static uint64_t f3(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 6ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=16ULL;o_op(8, 5, _a,_b);})){
v2=16ULL;
}
uint64_t __attribute__((unused)) v3=8ULL;
if(({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v3=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=127ULL;o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=128ULL;o_op(4, 5, _a,_b);});
}
{
uint64_t v4=0;
uint64_t k1=5ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t _a=o_get(v1, v1_n, v4, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
}
}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=65535ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=65535ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=255ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=511ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=2047ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t p1=34ULL;f1(v0, v0_n, p1);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=35ULL;f1(v0, v0_n, p1);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 4ULL, 5);o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t p1=35ULL;f1(v0, v0_n, p1);});uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=511ULL;o_op(12, 5, _a,_b);});uint64_t _b=v2;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=36ULL;f1(v0, v0_n, p1);});uint64_t _b=2047ULL;o_op(12, 5, _a,_b);});uint64_t _b=v3;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t p1=39ULL;f1(v0, v0_n, p1);});uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t p1=40ULL;f1(v0, v0_n, p1);});uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t p1=41ULL;f1(v0, v0_n, p1);});uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
static uint64_t f4(uint64_t v0, const void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f2(v1, v1_n, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f3(v2, v2_n, v3, v3_n);}));
}
return o_leave(0ULL);
}
static uint64_t f5(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
return o_leave(0ULL);
}
static uint64_t f6(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(112ULL);
}
return o_leave(0ULL);
}
static uint64_t f7(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(104ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(112ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
return o_leave(0ULL);
}
static uint64_t f8(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
return o_leave(0ULL);
}
static uint64_t f9(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
return o_leave(0ULL);
}
static uint64_t f10(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
return o_leave(0ULL);
}
static uint64_t f11(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(112ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
return o_leave(0ULL);
}
static uint64_t f12(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
return o_leave(0ULL);
}
static uint64_t f13(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(104ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(112ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
return o_leave(0ULL);
}
static uint64_t f14(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(107ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
return o_leave(0ULL);
}
static uint64_t f15(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(107ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
return o_leave(0ULL);
}
static uint64_t f16(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(120ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(104ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(50ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(53ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(54ULL);
}
return o_leave(0ULL);
}
static uint64_t f17(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(120ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(122ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
return o_leave(0ULL);
}
static uint64_t f18(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(104ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(50ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(53ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(54ULL);
}
return o_leave(0ULL);
}
static uint64_t f19(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(122ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
return o_leave(0ULL);
}
static uint64_t f20(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(113ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(48ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(104ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(50ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(53ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(54ULL);
}
return o_leave(0ULL);
}
static uint64_t f21(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(113ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(49ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(104ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(50ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(53ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(54ULL);
}
return o_leave(0ULL);
}
static uint64_t f22(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(113ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(112ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
return o_leave(0ULL);
}
static uint64_t f23(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(107ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
return o_leave(0ULL);
}
static uint64_t f24(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(107ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(119ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
return o_leave(0ULL);
}
static uint64_t f25(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(107ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(103ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
return o_leave(0ULL);
}
static uint64_t f26(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(107ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(103ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(104ULL);
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(50ULL);
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
return o_leave(53ULL);
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
return o_leave(54ULL);
}
return o_leave(0ULL);
}
static uint64_t f27(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(103ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
return o_leave(0ULL);
}
static uint64_t f28(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(107ULL);
}
return o_leave(0ULL);
}
static uint64_t f29(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(104ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(121ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
return o_leave(0ULL);
}
static uint64_t f30(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(103ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(112ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
return o_leave(0ULL);
}
static uint64_t f31(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(112ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
return o_leave(0ULL);
}
static uint64_t f32(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
return o_leave(0ULL);
}
static uint64_t f33(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
return o_leave(0ULL);
}
static uint64_t f34(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(118ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
return o_leave(0ULL);
}
static uint64_t f35(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(113ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
return o_leave(0ULL);
}
static uint64_t f36(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(113ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
return o_leave(0ULL);
}
static uint64_t f37(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(112ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(118ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
return o_leave(0ULL);
}
static uint64_t f38(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(112ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(118ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
return o_leave(0ULL);
}
static uint64_t f39(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
return o_leave(0ULL);
}
static uint64_t f40(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(103ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(45ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(118ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(45ULL);
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v0;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v0;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v0;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
return o_leave(47ULL);
}
if(({uint64_t _a=v0;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
return o_leave(49ULL);
}
return o_leave(0ULL);
}
static uint64_t f41(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f5(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f6(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f7(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f8(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f9(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f10(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f11(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f12(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f13(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f14(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f15(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f16(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f17(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f18(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f19(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f20(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f21(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f22(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f23(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f24(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f25(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f26(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f27(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f28(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=24ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f29(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=25ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f30(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=26ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f31(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=27ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f32(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=28ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f33(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=29ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f34(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=30ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f35(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=31ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f36(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=32ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f37(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=33ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f38(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=34ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f39(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=35ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f40(p0);}));
}
return o_leave(0ULL);
}
static uint64_t f42(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=({uint64_t _a=32768ULL;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(1, 5, _a,_b);});uint64_t _v=o_cast(2, 5, v2);o_put(v0, v0_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f43(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t p0=v2;uint64_t p1=v3;f41(p0, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(c1++>=64)o_trap(4);
({uint64_t p2=({uint64_t p0=v2;uint64_t p1=v3;f41(p0, p1);});f42(v0, v0_n, v1, v1_n, p2);});
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f44(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t v4[64]={0};
uint64_t v4_n=64;
o_fill(v4, v4_n, 0ULL, 5);
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=v3;o_op(11, 5, _a,_b);})){
if(c1++>=64)o_trap(4);
{uint64_t _i=v5;uint64_t _v=({uint64_t _a=v2;uint64_t _b=v3;o_op(5, 5, _a,_b);});o_put(v4, v4_n, _i,_v,5);}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=v3;o_op(4, 5, _a,_b);});
}
}
{uint64_t _i=v5;uint64_t _v=v2;o_put(v4, v4_n, _i,_v,5);}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(c1++>=64)o_trap(4);
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=o_get(v4, v4_n, v5, 5);
if(({uint64_t _a=v6;uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
({uint64_t p2=({uint64_t _a=v6;uint64_t _b=48ULL;o_op(1, 5, _a,_b);});f42(v0, v0_n, v1, v1_n, p2);});
}else{
({uint64_t p2=({uint64_t _a=v6;uint64_t _b=87ULL;o_op(1, 5, _a,_b);});f42(v0, v0_n, v1, v1_n, p2);});
}
}
}
return o_leave(0);
}
static uint64_t f45(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=34ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=92ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
({uint64_t p2=92ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v2;f42(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=127ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
({uint64_t p2=92ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=117ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=48ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=48ULL;f42(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(16, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
({uint64_t p2=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(1, 5, _a,_b);});f42(v0, v0_n, v1, v1_n, p2);});
}else{
({uint64_t p2=({uint64_t _a=v3;uint64_t _b=87ULL;o_op(1, 5, _a,_b);});f42(v0, v0_n, v1, v1_n, p2);});
}
v3=({uint64_t _a=v2;uint64_t _b=15ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
({uint64_t p2=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(1, 5, _a,_b);});f42(v0, v0_n, v1, v1_n, p2);});
}else{
({uint64_t p2=({uint64_t _a=v3;uint64_t _b=87ULL;o_op(1, 5, _a,_b);});f42(v0, v0_n, v1, v1_n, p2);});
}
return o_leave(0);
}
({uint64_t p2=v2;f42(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f46(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=12ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=14ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
uint64_t __attribute__((unused)) v4=88ULL;
if(({uint64_t _a=v2;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
v4=89ULL;
}
({uint64_t p2=o_get(v1, v1_n, v4, 5);uint64_t p3=10ULL;f44(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=18ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=19ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=24ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=25ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
uint64_t __attribute__((unused)) v4=90ULL;
if(({uint64_t _a=v2;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
v4=91ULL;
}
if(({uint64_t _a=v2;uint64_t _b=24ULL;o_op(6, 5, _a,_b);})){
v4=97ULL;
}
if(({uint64_t _a=v2;uint64_t _b=25ULL;o_op(6, 5, _a,_b);})){
v4=98ULL;
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=18ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
({uint64_t p2=48ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=120ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=o_get(v1, v1_n, v4, 5);uint64_t p3=16ULL;f44(v0, v0_n, v1, v1_n, p2, p3);});
}else{
({uint64_t p2=o_get(v1, v1_n, v4, 5);uint64_t p3=10ULL;f44(v0, v0_n, v1, v1_n, p2, p3);});
}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=22ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=23ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
uint64_t __attribute__((unused)) v4=92ULL;
if(({uint64_t _a=v2;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
v4=94ULL;
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=91ULL;f42(v0, v0_n, v1, v1_n, p2);});
}
({uint64_t p2=o_get(v1, v1_n, v4, 5);uint64_t p3=10ULL;f44(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=44ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=o_get(v1, v1_n, ({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p3=10ULL;f44(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=44ULL;f42(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v2;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=49ULL;f42(v0, v0_n, v1, v1_n, p2);});
}else{
({uint64_t p2=o_get(v1, v1_n, 96ULL, 5);uint64_t p3=10ULL;f44(v0, v0_n, v1, v1_n, p2, p3);});
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=93ULL;f42(v0, v0_n, v1, v1_n, p2);});
}
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=34ULL;f42(v0, v0_n, v1, v1_n, p2);});
}
if(({uint64_t _a=v2;uint64_t _b=34ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=35ULL;f43(v0, v0_n, v1, v1_n, p2);});
}else{
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=48ULL;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=32768ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=32768ULL;uint64_t _b=v4;o_op(2, 5, _a,_b);});o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{
uint64_t v6=0;
uint64_t c2=0;
for(;o_op(8,5, v6, v5);v6=o_op(1,5, v6,1)){
if(c2++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v7=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=v6;o_op(1, 5, _a,_b);}), 2));
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=v7;f45(v0, v0_n, v1, v1_n, p2);});
}else{
({uint64_t p2=v7;f42(v0, v0_n, v1, v1_n, p2);});
}
}
}
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=34ULL;f42(v0, v0_n, v1, v1_n, p2);});
}
return o_leave(0);
}
static uint64_t f47(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=32ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=32ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=34ULL;f42(v0, v0_n, v1, v1_n, p2);});
}
({uint64_t p2=v2;f43(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=34ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=58ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=32ULL;f42(v0, v0_n, v1, v1_n, p2);});
}else{
({uint64_t p2=61ULL;f42(v0, v0_n, v1, v1_n, p2);});
}
({uint64_t p2=v2;uint64_t p3=v3;f46(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
({uint64_t p2=44ULL;f42(v0, v0_n, v1, v1_n, p2);});
}
({uint64_t p2=10ULL;f42(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f48(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 3ULL, 5);
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=23ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=8192ULL;o_op(11, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
{
uint64_t v3=0;
uint64_t c1=0;
for(;o_op(8,5, v3, v2);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v3, 2);uint64_t _b=0ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, v3, 2);uint64_t _b=13ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
}
}
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t p0=35ULL;uint64_t p1=v3;f41(p0, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(c1++>=64)o_trap(4);
if(({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, v3, 2));uint64_t _b=({uint64_t p0=35ULL;uint64_t p1=v3;f41(p0, p1);});o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=o_get(v0, v0_n, v3, 2);uint64_t _b=10ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
{
uint64_t v4=0;
uint64_t k1=34ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c2=0;
while(({uint64_t _a=({uint64_t p0=v4;uint64_t p1=v5;f41(p0, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(c2++>=64)o_trap(4);
if(({uint64_t _a=v3;uint64_t _b=v2;o_op(11, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, v3, 2));uint64_t _b=({uint64_t p0=v4;uint64_t p1=v5;f41(p0, p1);});o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v3;uint64_t _b=v2;o_op(11, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, v3, 2);uint64_t _b=61ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=v3;
{
uint64_t c2=0;
while(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c2++>=8192)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, v3, 2);uint64_t _b=10ULL;o_op(6, 2, _a,_b);})){
break;
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v3;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
{uint64_t _i=({uint64_t _a=v4;uint64_t _b=48ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v3;uint64_t _b=v6;o_op(2, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v3;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f49(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v1;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v1;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v1;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v1;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v1;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v1;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v1;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v1;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(120ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(113ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(48ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(113ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(49ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(107ULL);
}
if(({uint64_t _a=v1;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v1;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v1;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(103ULL);
}
if(({uint64_t _a=v1;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
return o_leave(0ULL);
}
return o_leave(0ULL);
}
static uint64_t f50(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
{
uint64_t v2=0;
uint64_t k1=7ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
uint64_t __attribute__((unused)) v3=0ULL;
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v3=35ULL;
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v3=13ULL;
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v3=11ULL;
}
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v3=15ULL;
}
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
v3=16ULL;
}
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
v3=21ULL;
}
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, ({uint64_t _a=v3;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);
{
uint64_t v5=0;
uint64_t k2=64ULL;
for(;o_op(8,5, v5, k2);v5=o_op(1,5, v5,1)){
({uint64_t p2=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2));f42(v0, v0_n, v1, v1_n, p2);});
}
}
({uint64_t p2=32ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=32ULL;f42(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c2=0;
while(({uint64_t _a=({uint64_t p0=v2;uint64_t p1=v5;f49(p0, p1);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(c2++>=64)o_trap(4);
({uint64_t p2=({uint64_t p0=v2;uint64_t p1=v5;f49(p0, p1);});f42(v0, v0_n, v1, v1_n, p2);});
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
({uint64_t p2=10ULL;f42(v0, v0_n, v1, v1_n, p2);});
}
}
return o_leave(0);
}
static uint64_t f51(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f48(v0, v0_n, v1, v1_n);}));
}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=8191ULL;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
({f50(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=2ULL;uint64_t _v=32768ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=123ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=10ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=34ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=0ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
{
uint64_t v2=0;
uint64_t k2=11ULL;
for(;o_op(8,5, v2, k2);v2=o_op(1,5, v2,1)){
({uint64_t p2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
({uint64_t p2=13ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=12ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=14ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=18ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=19ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=22ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=23ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=24ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=25ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=15ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=16ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=21ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=17ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
{
uint64_t v2=0;
uint64_t k2=5ULL;
for(;o_op(8,5, v2, k2);v2=o_op(1,5, v2,1)){
({uint64_t p2=({uint64_t _a=v2;uint64_t _b=26ULL;o_op(1, 5, _a,_b);});uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
({uint64_t p2=33ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=32ULL;uint64_t p3=1ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=31ULL;uint64_t p3=1ULL;uint64_t p4=1ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=125ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=10ULL;f42(v0, v0_n, v1, v1_n, p2);});
}else{
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=35ULL;f43(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=10ULL;f42(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=0ULL;uint64_t p3=0ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
{
uint64_t v2=0;
uint64_t k2=32ULL;
for(;o_op(8,5, v2, k2);v2=o_op(1,5, v2,1)){
({uint64_t p2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p3=0ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=33ULL;uint64_t p3=0ULL;uint64_t p4=0ULL;f47(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f52(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(95ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(49ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(50ULL);
}
if(({uint64_t _a=v1;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(49ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(118ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(100ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v1;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(45ULL);
}
if(({uint64_t _a=v1;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v1;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v1;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(98ULL);
}
if(({uint64_t _a=v1;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v1;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v1;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(45ULL);
}
if(({uint64_t _a=v1;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v1;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
if(({uint64_t _a=v1;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
return o_leave(102ULL);
}
if(({uint64_t _a=v1;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
return o_leave(54ULL);
}
if(({uint64_t _a=v1;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
return o_leave(52ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(122ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(114ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(45ULL);
}
if(({uint64_t _a=v1;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(118ULL);
}
if(({uint64_t _a=v1;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(45ULL);
}
if(({uint64_t _a=v1;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(118ULL);
}
if(({uint64_t _a=v1;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(49ULL);
}
return o_leave(0ULL);
}
return o_leave(0ULL);
}
static uint64_t f53(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=48ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=64ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);
{
uint64_t v4=0;
uint64_t k1=64ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, ({uint64_t _a=v3;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=48ULL;o_op(11, 2, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=57ULL;o_op(9, 2, _a,_b);});o_op(12, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=97ULL;o_op(11, 2, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=102ULL;o_op(9, 2, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
}
}
return o_leave(1ULL);
}
static uint64_t f54(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);
{
uint64_t v4=0;
uint64_t k1=64ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v3;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);uint64_t _b=o_get(v0, v0_n, ({uint64_t _a=26000ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
}
}
return o_leave(1ULL);
}
static uint64_t f55(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 2ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
{uint64_t _i=({uint64_t _a=110ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v1, v1_n, 4ULL, 5);o_put(v0, v0_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f56(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t v5[8]={0};
uint64_t v5_n=8;
o_fill(v5, v5_n, 0ULL, 5);
{uint64_t _i=3ULL;uint64_t _v=v3;o_put(v5, v5_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=48ULL;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=64ULL;o_op(11, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v7=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);
{
uint64_t v8=0;
uint64_t k1=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v8, k1);v8=o_op(1,5, v8,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v9=0ULL;
if(({uint64_t _a=v8;uint64_t _b=v6;o_op(8, 5, _a,_b);})){
v9=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v7;uint64_t _b=v8;o_op(1, 5, _a,_b);}), 2));
}
uint64_t __attribute__((unused)) v10=({uint64_t p0=v4;uint64_t p1=v9;f0(p0, p1, v5, v5_n);});
if(({uint64_t _a=v10;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
({uint64_t p2=v10;f55(v1, v1_n, v5, v5_n, p2);});
}
}
return o_leave(1ULL);
}
static uint64_t f57(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=48ULL;o_op(1, 5, _a,_b);}), 5);
{
uint64_t v6=0;
uint64_t c1=0;
for(;o_op(8,5, v6, v5);v6=o_op(1,5, v6,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v4;uint64_t _b=v6;o_op(1, 5, _a,_b);}), 2));uint64_t _b=({uint64_t p0=v3;uint64_t p1=v6;f52(p0, p1);});o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
}
}
return o_leave(({uint64_t _a=({uint64_t p0=v3;uint64_t p1=v5;f52(p0, p1);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);}));
}
static uint64_t f58(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=48ULL;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=v5;uint64_t _b=v3;o_op(11, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
static uint64_t f59(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t v2[8]={0};
uint64_t v2_n=8;
o_fill(v2, v2_n, 0ULL, 5);
if(({uint64_t _a=o_get(v1, v1_n, 106ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=3ULL;uint64_t _v=3ULL;o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 107ULL, 5);
{
uint64_t v4=0;
uint64_t k2=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
uint64_t c2=0;
for(;o_op(8,5, v4, k2);v4=o_op(1,5, v4,1)){
if(c2++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v5=0ULL;
if(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
v5=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=25088ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2));
}
uint64_t __attribute__((unused)) v6=({uint64_t p0=3ULL;uint64_t p1=v5;f0(p0, p1, v2, v2_n);});
if(({uint64_t _a=v6;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 2ULL, 5);
if(({uint64_t _a=v6;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v7=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
{uint64_t _i=({uint64_t _a=120ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v2, v2_n, 4ULL, 5);o_put(v1, v1_n, _i,_v,5);}
}
}
}
{
uint64_t v4=0;
uint64_t k2=3ULL;
for(;o_op(8,5, v4, k2);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, ({uint64_t _a=120ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, ({uint64_t _a=110ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=v5;uint64_t _b=v6;o_op(10, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v5;uint64_t _b=v6;o_op(8, 5, _a,_b);})){
return o_leave(8ULL);
}
}
}
}
return o_leave(0ULL);
}
static uint64_t f60(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint8_t v2[256]={0};
uint64_t v2_n=256;
o_fill(v2, v2_n, 0ULL, 2);
uint8_t v3[256]={0};
uint64_t v3_n=256;
o_fill(v3, v3_n, 0ULL, 2);
uint64_t v4[7]={0};
uint64_t v4_n=7;
o_fill(v4, v4_n, 0ULL, 5);
{
uint64_t v5=0;
uint64_t k1=256ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
{uint64_t _i=v5;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=24576ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);o_put(v2, v2_n, _i,_v,2);}
{uint64_t _i=v5;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=24832ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);o_put(v3, v3_n, _i,_v,2);}
}
}
if(({uint64_t _a=({f2(v2, v2_n, v3, v3_n);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(7ULL);
}
if(({uint64_t _a=({uint64_t p2=22ULL;uint64_t p3=64ULL;uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=22ULL;uint64_t p3=3ULL;uint64_t p4=2ULL;f56(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=0ULL;uint64_t _v=o_get(v1, v1_n, 110ULL, 5);o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=o_get(v1, v1_n, 111ULL, 5);o_put(v4, v4_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 112ULL, 5);uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=23ULL;uint64_t p3=64ULL;uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=23ULL;uint64_t p3=3ULL;uint64_t p4=2ULL;f56(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=2ULL;uint64_t _v=o_get(v1, v1_n, 110ULL, 5);o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v1, v1_n, 111ULL, 5);o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v1, v1_n, 112ULL, 5);o_put(v4, v4_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t p2=24ULL;uint64_t p3=64ULL;uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=24ULL;uint64_t p3=1ULL;uint64_t p4=2ULL;f56(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=5ULL;uint64_t _v=o_get(v1, v1_n, 110ULL, 5);o_put(v4, v4_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t p2=25ULL;uint64_t p3=64ULL;uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=25ULL;uint64_t p3=1ULL;uint64_t p4=2ULL;f56(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=6ULL;uint64_t _v=o_get(v1, v1_n, 110ULL, 5);o_put(v4, v4_n, _i,_v,5);}
if(({uint64_t _a=({f3(v3, v3_n, v4, v4_n);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=({uint64_t p2=19ULL;uint64_t p3=64ULL;uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=19ULL;uint64_t p3=1ULL;uint64_t p4=2ULL;f56(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, 110ULL, 5);
if(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=32ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 105ULL, 5);uint64_t _b=({uint64_t _a=v5;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=2ULL;uint64_t p3=128ULL;uint64_t p4=0ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=3ULL;uint64_t p3=128ULL;uint64_t p4=0ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=7ULL;uint64_t p3=128ULL;uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(4ULL);
}
if(({uint64_t _a=({uint64_t p2=7ULL;uint64_t p3=0ULL;f57(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(4ULL);
}
if(({uint64_t _a=({uint64_t p2=9ULL;uint64_t p3=128ULL;uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=9ULL;uint64_t p3=1ULL;f57(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=17ULL;uint64_t p3=128ULL;uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(7ULL);
}
if(({uint64_t _a=({uint64_t p2=17ULL;uint64_t p3=2ULL;f57(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(7ULL);
}
if(({uint64_t _a=({uint64_t p2=5ULL;uint64_t p3=128ULL;uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=({uint64_t p2=5ULL;f53(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=({uint64_t p2=29ULL;uint64_t p3=128ULL;uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=29ULL;uint64_t p3=3ULL;uint64_t p4=3ULL;f56(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(({f59(v0, v0_n, v1, v1_n);}));
}
static uint64_t f61(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(109ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v1;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
}
if(({uint64_t _a=v0;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(103ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(113ULL);
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(45ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(99ULL);
}
if(({uint64_t _a=v1;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(117ULL);
}
if(({uint64_t _a=v1;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(115ULL);
}
if(({uint64_t _a=v1;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(108ULL);
}
if(({uint64_t _a=v1;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(45ULL);
}
if(({uint64_t _a=v1;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(97ULL);
}
if(({uint64_t _a=v1;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v1;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v1;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(101ULL);
}
if(({uint64_t _a=v1;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
if(({uint64_t _a=v1;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
return o_leave(116ULL);
}
if(({uint64_t _a=v1;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
return o_leave(105ULL);
}
if(({uint64_t _a=v1;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
return o_leave(111ULL);
}
if(({uint64_t _a=v1;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
return o_leave(110ULL);
}
}
return o_leave(0ULL);
}
static uint64_t f62(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 9ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 49ULL, 5);
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=6ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=20ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
{
uint64_t v4=0;
uint64_t c1=0;
for(;o_op(8,5, v4, v3);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2));uint64_t _b=({uint64_t p0=v3;uint64_t p1=v4;f61(p0, p1);});o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
}
}
return o_leave(1ULL);
}
static uint64_t f63(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
{
uint64_t v2=0;
uint64_t k1=34ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=10ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=26ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=33ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
continue;
}
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);
{
uint64_t v4=0;
uint64_t k2=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=48ULL;o_op(1, 5, _a,_b);}), 5);
uint64_t c2=0;
for(;o_op(8,5, v4, k2);v4=o_op(1,5, v4,1)){
if(c2++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, ({uint64_t _a=v3;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=32ULL;o_op(8, 2, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=127ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
}
}
}
}
if(({uint64_t _a=({uint64_t p2=29ULL;uint64_t p3=3ULL;uint64_t p4=3ULL;f56(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f64(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint8_t v2[256]={0};
uint64_t v2_n=256;
o_fill(v2, v2_n, 0ULL, 2);
uint8_t v3[256]={0};
uint64_t v3_n=256;
o_fill(v3, v3_n, 0ULL, 2);
uint64_t v4[7]={0};
uint64_t v4_n=7;
o_fill(v4, v4_n, 0ULL, 5);
if(({uint64_t _a=({uint64_t _a=({uint64_t p2=5ULL;f53(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p2=6ULL;f53(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=({f62(v0, v0_n, v1, v1_n);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(9ULL);
}
{
uint64_t v5=0;
uint64_t k1=256ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
{uint64_t _i=v5;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=24576ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);o_put(v2, v2_n, _i,_v,2);}
{uint64_t _i=v5;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=24832ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);o_put(v3, v3_n, _i,_v,2);}
}
}
if(({uint64_t _a=({f2(v2, v2_n, v3, v3_n);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(7ULL);
}
{uint64_t _i=0ULL;uint64_t _v=o_get(v1, v1_n, 92ULL, 5);o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=o_get(v1, v1_n, 93ULL, 5);o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v1, v1_n, 94ULL, 5);o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v1, v1_n, 95ULL, 5);o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v1, v1_n, 96ULL, 5);o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=o_get(v1, v1_n, 97ULL, 5);o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=o_get(v1, v1_n, 98ULL, 5);o_put(v4, v4_n, _i,_v,5);}
if(({uint64_t _a=({f3(v3, v3_n, v4, v4_n);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(3ULL);
}
return o_leave(0ULL);
}
static uint64_t f65(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f63(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f64(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(10ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(9ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 113ULL, 5);uint64_t _b=o_get(v1, v1_n, 114ULL, 5);o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 115ULL, 5);o_op(10, 5, _a,_b);})){
{uint64_t _i=115ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 90ULL, 5);uint64_t _b=896ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 91ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 91ULL, 5);uint64_t _b=32ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(9ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({f48(v0, v0_n, v1, v1_n);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t p2=0ULL;f53(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=108ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 9ULL, 5);uint64_t _b=3ULL;o_op(2, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=109ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 41ULL, 5);uint64_t _b=16ULL;o_op(2, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t p2=0ULL;f54(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(5ULL);
}
{uint64_t _i=0ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=11ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 7ULL, 5);
if(({uint64_t _a=({uint64_t p2=v2;f53(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(6ULL);
}
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 6ULL, 5);
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=100ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t _b=384ULL;o_op(7, 5, _a,_b);})){
return o_leave(5ULL);
}
}
if(({uint64_t _a=({uint64_t p2=v2;f54(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(5ULL);
}
if(({uint64_t _a=v3;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t p2=10ULL;uint64_t p3=64ULL;uint64_t p4=0ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=0ULL;uint64_t _v=3ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=7ULL;uint64_t _v=13ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=7ULL;uint64_t _v=15ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
if(({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=7ULL;uint64_t _v=16ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=7ULL;uint64_t _v=21ULL;o_put(v1, v1_n, _i,_v,5);}
}
}
}
}
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(5ULL);
}
uint64_t __attribute__((unused)) v2=({f60(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v2);
}
{uint64_t _i=0ULL;uint64_t _v=5ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
return o_leave(0ULL);
}
uint32_t omega_artifact_admit(uint64_t *result, void *p0, uint64_t p0_n, void *p1, uint64_t p1_n){
if(!o_buf(result,1,1,5))return 32;
if(!o_buf(p0, p0_n,65536, 2))return 32;
if(o_overlap(result,8,p0, p0_n*1))return 32;
if(!o_buf(p1, p1_n,128, 5))return 32;
if(o_overlap(result,8,p1, p1_n*8))return 32;
if(o_overlap(p1, p1_n*8, p0, p0_n*1))return 32;
if(o_busy)return 33;
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f65(p0, p0_n, p1, p1_n);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
