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
static uint64_t f0(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f1(uint64_t v0, uint64_t v1);
static uint64_t f2(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f3(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f4(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f5(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f6(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f7(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f8(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f9(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f10(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f11(uint64_t v0);
static uint64_t f12(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f13(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f14(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f15(uint64_t v0);
static uint64_t f16(uint64_t v0);
static uint64_t f17(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f18(const void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n);
static uint64_t f19(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f20(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f21(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f22(void *v0, uint64_t v0_n);
static uint64_t f23(void *v0, uint64_t v0_n);
static uint64_t f24(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f25(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f26(uint64_t v0);
static uint64_t f27(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f28(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f29(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f30(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f31(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f32(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f33(uint64_t v0, uint64_t v1);
static uint64_t f34(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f35(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n, uint64_t v4);
static uint64_t f36(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n);
static uint64_t f37(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f38(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n, void *v4, uint64_t v4_n, uint64_t v5);
static uint64_t f39(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f40(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f41(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f42(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n);
static uint64_t f43(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f44(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f45(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4);
static uint64_t f46(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4);
static uint64_t f47(uint64_t v0);
static uint64_t f48(uint64_t v0, uint64_t v1);
static uint64_t f49(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f50(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n);
static uint64_t f51(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f52(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n);
static uint64_t f53(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f54(void *v0, uint64_t v0_n);
static uint64_t f55(void *v0, uint64_t v0_n);
static uint64_t f56(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f0(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(16, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v2;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(12, 5, _a,_b);}));
}
static uint64_t f1(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v1;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=v0;o_op(15, 5, _a,_b);}));
}
static uint64_t f2(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
{uint64_t _i=v1;uint64_t _v=o_cast(2, 5, v2);o_put(v0, v0_n, _i,_v,2);}
return o_leave(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));
}
static uint64_t f3(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(16, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=7ULL;o_op(12, 5, _a,_b);});uint64_t _b=3ULL;o_op(15, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);}));
}
static uint64_t f4(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=v1;
{
uint64_t v5=0;
uint64_t k1=64ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
uint64_t __attribute__((unused)) v6=({uint64_t p1=v3;uint64_t p2=v5;f3(v2, v2_n, p1, p2);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
v4=({uint64_t p1=v4;uint64_t p2=v6;f2(v0, v0_n, p1, p2);});
}
}
return o_leave(v4);
}
static uint64_t f5(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=1ULL;
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v3;o_op(4, 5, _a,_b);});uint64_t _b=10ULL;o_op(11, 5, _a,_b);})){
if(c1++>=20)o_trap(4);
v3=({uint64_t _a=v3;uint64_t _b=10ULL;o_op(3, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v4=v1;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c1++>=21)o_trap(4);
v4=({uint64_t p1=v4;uint64_t p2=({uint64_t _a=48ULL;uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v3;o_op(4, 5, _a,_b);});uint64_t _b=10ULL;o_op(5, 5, _a,_b);});o_op(1, 5, _a,_b);});f2(v0, v0_n, p1, p2);});
v3=({uint64_t _a=v3;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
}
}
return o_leave(v4);
}
static uint64_t f6(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=({uint64_t _a=48ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});f2(v0, v0_n, p1, p2);}));
}
return o_leave(({uint64_t p1=v1;uint64_t p2=({uint64_t _a=87ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});f2(v0, v0_n, p1, p2);}));
}
static uint64_t f7(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=1ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=16ULL;o_op(8, 5, _a,_b);})){
if(c1++>=16)o_trap(4);
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v4=v1;
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v3);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
v4=({uint64_t p1=v4;uint64_t p2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=v5;o_op(2, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=15ULL;o_op(12, 5, _a,_b);});f6(v0, v0_n, p1, p2);});
}
}
return o_leave(v4);
}
static uint64_t f8(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=v1;
{
uint64_t v4=0;
uint64_t k1=16ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
v3=({uint64_t p1=v3;uint64_t p2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=({uint64_t _a=15ULL;uint64_t _b=v4;o_op(2, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=15ULL;o_op(12, 5, _a,_b);});f6(v0, v0_n, p1, p2);});
}
}
return o_leave(v3);
}
static uint64_t f9(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=v1;uint64_t p2=48ULL;f2(v0, v0_n, p1, p2);});
v3=({uint64_t p1=v3;uint64_t p2=120ULL;f2(v0, v0_n, p1, p2);});
return o_leave(({uint64_t p1=v3;uint64_t p2=v2;f7(v0, v0_n, p1, p2);}));
}
static uint64_t f10(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=9223372036854775808ULL;o_op(11, 5, _a,_b);})){
uint64_t __attribute__((unused)) v3=({uint64_t p1=v1;uint64_t p2=45ULL;f2(v0, v0_n, p1, p2);});
return o_leave(({uint64_t p1=v3;uint64_t p2=({uint64_t _a=0ULL;uint64_t _b=v2;o_op(2, 5, _a,_b);});f9(v0, v0_n, p1, p2);}));
}
return o_leave(({uint64_t p1=v1;uint64_t p2=v2;f9(v0, v0_n, p1, p2);}));
}
static uint64_t f11(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=v0;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v1;uint64_t _b=2147483648ULL;o_op(11, 5, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=18446744069414584320ULL;o_op(13, 5, _a,_b);}));
}
return o_leave(v1);
}
static uint64_t f12(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=v1;
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t p1=v4;uint64_t p2=85ULL;f2(v0, v0_n, p1, p2);});
}
v4=({uint64_t p1=v4;uint64_t p2=82ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v3;uint64_t _b=255ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v4;uint64_t p2=90ULL;f2(v0, v0_n, p1, p2);}));
}
return o_leave(({uint64_t p1=v4;uint64_t p2=v3;f5(v0, v0_n, p1, p2);}));
}
static uint64_t f13(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=v1;
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t p1=v4;uint64_t p2=33ULL;f2(v0, v0_n, p1, p2);});
}
v4=({uint64_t p1=v4;uint64_t p2=80ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v4;uint64_t p2=84ULL;f2(v0, v0_n, p1, p2);}));
}
return o_leave(({uint64_t p1=v4;uint64_t p2=v2;f5(v0, v0_n, p1, p2);}));
}
static uint64_t f14(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=16777216ULL;uint64_t _b=({uint64_t _a=v0;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});uint64_t _b=v2;o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f15(uint64_t v0){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=100ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=68ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=16ULL;uint64_t p2=8ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=121ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=16ULL;uint64_t p2=4ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=97ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=65ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=24ULL;uint64_t p2=8ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=98ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=66ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=32ULL;uint64_t p2=8ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=99ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=67ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=0ULL;uint64_t p2=8ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=105ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=115ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=102ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=106ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=89ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=32ULL;uint64_t p2=32ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=75ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=38ULL;uint64_t p2=21ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=85ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=37ULL;uint64_t p2=22ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=111ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=40ULL;uint64_t p2=24ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=80ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=17ULL;uint64_t p2=3ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=81ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=20ULL;uint64_t p2=3ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=82ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=23ULL;uint64_t p2=4ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=108ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=83ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=8ULL;uint64_t p2=8ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=104ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=53ULL;uint64_t p2=5ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=109ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=40ULL;uint64_t p2=13ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=71ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=12ULL;uint64_t p2=3ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=103ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=12ULL;uint64_t p2=4ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=90ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=9ULL;uint64_t p2=1ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=122ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=9ULL;uint64_t p2=3ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=78ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;uint64_t p1=8ULL;uint64_t p2=1ULL;f14(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=77ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;uint64_t p1=63ULL;uint64_t p2=1ULL;f14(p0, p1, p2);}));
}
return o_leave(0ULL);
}
static uint64_t f16(uint64_t v0){
o_enter();
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t p1=({uint64_t _a=v0;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});f1(p0, p1);}));
}
static uint64_t f17(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p0=v0;f15(p0);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t p2=({uint64_t _a=v3;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});f0(p0, p1, p2);}));
}
return o_leave(({uint64_t p0=v1;uint64_t p1=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t p2=({uint64_t _a=v3;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});f0(p0, p1, p2);}));
}
static uint64_t f18(const void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=64ULL;o_op(8, 5, _a,_b);})){
if(c1++>=64)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t p1=v1;uint64_t p2=v3;f3(v0, v0_n, p1, p2);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=37ULL;o_op(7, 5, _a,_b);})){
continue;
}
uint64_t __attribute__((unused)) v5=({uint64_t p1=v1;uint64_t p2=v3;f3(v0, v0_n, p1, p2);});
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v5;uint64_t _b=84ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=18446744056546394112ULL;o_op(13, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=262143ULL;o_op(13, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
continue;
}
uint64_t __attribute__((unused)) v6=({uint64_t p0=v5;f15(p0);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=({uint64_t p0=v6;f16(p0);});o_op(13, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=({uint64_t p0=v6;f16(p0);});o_op(13, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
}
}
}
}
return o_leave(0);
}
static uint64_t f19(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
{
uint64_t v2=0;
uint64_t k1=16ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
{uint64_t _i=v2;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
}
}
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f20(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
{
uint64_t v3=0;
uint64_t k1=16ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v3, 5);uint64_t _b=v1;o_op(3, 5, _a,_b);});uint64_t _b=v2;o_op(1, 5, _a,_b);});
{uint64_t _i=v3;uint64_t _v=({uint64_t _a=v4;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
v2=({uint64_t _a=v4;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f21(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
{
uint64_t v3=0;
uint64_t c1=0;
for(;o_op(8,5, v3, v2);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p1=v1;f20(v0, v0_n, p1);});
}
}
return o_leave(0);
}
static uint64_t f22(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=0ULL;
{
uint64_t v2=0;
uint64_t k1=16ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
uint64_t __attribute__((unused)) v3=({uint64_t _a=15ULL;uint64_t _b=v2;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t _b=o_get(v0, v0_n, v3, 5);o_op(13, 5, _a,_b);});
{uint64_t _i=v3;uint64_t _v=({uint64_t _a=v4;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
v1=({uint64_t _a=v4;uint64_t _b=10ULL;o_op(5, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f23(void *v0, uint64_t v0_n){
o_enter();
{
uint64_t v1=0;
uint64_t k1=16ULL;
for(;o_op(8,5, v1, k1);v1=o_op(1,5, v1,1)){
if(({uint64_t _a=o_get(v0, v0_n, v1, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
}
}
return o_leave(1ULL);
}
static uint64_t f24(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
{
uint64_t v2=0;
uint64_t k1=16ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
uint64_t __attribute__((unused)) v3=({uint64_t _a=15ULL;uint64_t _b=v2;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, v3, 5);
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, v3, 5);
if(({uint64_t _a=v4;uint64_t _b=v5;o_op(8, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v4;uint64_t _b=v5;o_op(10, 5, _a,_b);})){
return o_leave(2ULL);
}
}
}
return o_leave(1ULL);
}
static uint64_t f25(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=({f23(v0, v0_n);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(c1++>=128)o_trap(4);
{uint64_t _i=v2;uint64_t _v=o_cast(2, 5, ({f22(v0, v0_n);}));o_put(v1, v1_n, _i,_v,2);}
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v2);
}
static uint64_t f26(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=1ULL;
{
uint64_t v2=0;
uint64_t c1=0;
for(;o_op(8,5, v2, v0);v2=o_op(1,5, v2,1)){
if(c1++>=1000000)o_trap(4);
v1=({uint64_t _a=v1;uint64_t _b=10ULL;o_op(3, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f27(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=v1;o_op(11, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=v2;o_op(2, 5, _a,_b);}), 2)));
}
static uint64_t f28(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t v4=0;
uint64_t c1=0;
for(;o_op(8,5, v4, v2);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
v3=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=10ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t p1=v1;uint64_t p2=v4;f27(v0, v0_n, p1, p2);});o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v1;uint64_t _b=v2;o_op(9, 5, _a,_b);})){
return o_leave(v3);
}
uint64_t __attribute__((unused)) v4=({uint64_t p1=v1;uint64_t p2=v2;f27(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v4;uint64_t _b=5ULL;o_op(10, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=0ULL;
{
uint64_t v7=0;
uint64_t k2=({uint64_t _a=v1;uint64_t _b=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_op(2, 5, _a,_b);});
uint64_t c2=0;
for(;o_op(8,5, v7, k2);v7=o_op(1,5, v7,1)){
if(c2++>=1000000)o_trap(4);
if(({uint64_t _a=({uint64_t p1=v1;uint64_t p2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=v7;o_op(1, 5, _a,_b);});f27(v0, v0_n, p1, p2);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v6=1ULL;
}
}
}
if(({uint64_t _a=v6;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v5=1ULL;
}else{
v5=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);});
}
}
if(v5){
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
return o_leave(v3);
}
static uint64_t f29(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p1=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});f19(v0, v0_n, p1);});
({uint64_t p1=v4;f19(v1, v1_n, p1);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(11, 9, _a,_b);})){
({uint64_t p1=5ULL;uint64_t p2=o_cast(5, 9, v3);f21(v0, v0_n, p1, p2);});
}else{
({uint64_t p1=5ULL;uint64_t p2=o_cast(5, 9, ({uint64_t _a=v3;uint64_t _b=0;o_op(17, 9, _a,_b);}));f21(v1, v1_n, p1, p2);});
}
uint64_t __attribute__((unused)) v6=({uint64_t _a=v3;uint64_t _b=v5;o_op(2, 9, _a,_b);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(11, 9, _a,_b);})){
({uint64_t p1=2ULL;uint64_t p2=o_cast(5, 9, v6);f21(v0, v0_n, p1, p2);});
}else{
({uint64_t p1=2ULL;uint64_t p2=o_cast(5, 9, ({uint64_t _a=v6;uint64_t _b=0;o_op(17, 9, _a,_b);}));f21(v1, v1_n, p1, p2);});
}
return o_leave(({f24(v0, v0_n, v1, v1_n);}));
}
static uint64_t f30(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=v4;o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f31(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t p5=v5;f29(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t _b=2ULL;o_op(2, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=8388608ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=18446744073709551467ULL;o_op(10, 9, _a,_b);});o_op(12, 1, _a,_b);})){
v8=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
uint64_t __attribute__((unused)) v9=({uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=v8;uint64_t p5=v5;f29(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(({uint64_t p0=v9;uint64_t p1=v7;uint64_t p2=v6;f30(p0, p1, p2);}));
}
static uint64_t f32(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t p0=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=v2;o_op(2, 5, _a,_b);});f26(p0);});o_op(4, 5, _a,_b);});uint64_t _b=10ULL;o_op(5, 5, _a,_b);}));
}
static uint64_t f33(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=v1;
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p0=v0;uint64_t p1=v1;uint64_t p2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f32(p0, p1, p2);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(c1++>=20)o_trap(4);
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
}
return o_leave(v2);
}
static uint64_t f34(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p0=v2;uint64_t p1=v3;f33(p0, p1);});
uint64_t __attribute__((unused)) v6=v1;
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=18446744073709551612ULL;o_op(8, 9, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=o_cast(9, 5, v3);o_op(11, 9, _a,_b);});o_op(13, 1, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=({uint64_t _a=48ULL;uint64_t _b=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=0ULL;f32(p0, p1, p2);});o_op(1, 5, _a,_b);});f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=46ULL;f2(v0, v0_n, p1, p2);});
{
uint64_t v7=0;
uint64_t k3=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t c3=0;
for(;o_op(8,5, v7, k3);v7=o_op(1,5, v7,1)){
if(c3++>=1000000)o_trap(4);
v6=({uint64_t p1=v6;uint64_t p2=({uint64_t _a=48ULL;uint64_t _b=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});f32(p0, p1, p2);});o_op(1, 5, _a,_b);});f2(v0, v0_n, p1, p2);});
}
}
}
v6=({uint64_t p1=v6;uint64_t p2=101ULL;f2(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v7=v4;
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=45ULL;f2(v0, v0_n, p1, p2);});
v7=({uint64_t _a=v4;uint64_t _b=0;o_op(17, 9, _a,_b);});
}else{
v6=({uint64_t p1=v6;uint64_t p2=43ULL;f2(v0, v0_n, p1, p2);});
}
if(({uint64_t _a=v7;uint64_t _b=10ULL;o_op(8, 9, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=48ULL;f2(v0, v0_n, p1, p2);});
}
return o_leave(({uint64_t p1=v6;uint64_t p2=o_cast(5, 9, v7);f5(v0, v0_n, p1, p2);}));
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=48ULL;f2(v0, v0_n, p1, p2);});
v6=({uint64_t p1=v6;uint64_t p2=46ULL;f2(v0, v0_n, p1, p2);});
{
uint64_t v7=0;
uint64_t k2=({uint64_t _a=o_cast(5, 9, ({uint64_t _a=v4;uint64_t _b=0;o_op(17, 9, _a,_b);}));uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t c2=0;
for(;o_op(8,5, v7, k2);v7=o_op(1,5, v7,1)){
if(c2++>=1000000)o_trap(4);
v6=({uint64_t p1=v6;uint64_t p2=48ULL;f2(v0, v0_n, p1, p2);});
}
}
{
uint64_t v7=0;
uint64_t c2=0;
for(;o_op(8,5, v7, v5);v7=o_op(1,5, v7,1)){
if(c2++>=1000000)o_trap(4);
v6=({uint64_t p1=v6;uint64_t p2=({uint64_t _a=48ULL;uint64_t _b=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=v7;f32(p0, p1, p2);});o_op(1, 5, _a,_b);});f2(v0, v0_n, p1, p2);});
}
}
return o_leave(v6);
}
uint64_t __attribute__((unused)) v7=({uint64_t _a=o_cast(5, 9, v4);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
{
uint64_t v8=0;
uint64_t c1=0;
for(;o_op(8,5, v8, v7);v8=o_op(1,5, v8,1)){
if(c1++>=1000000)o_trap(4);
v6=({uint64_t p1=v6;uint64_t p2=({uint64_t _a=48ULL;uint64_t _b=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=v8;f32(p0, p1, p2);});o_op(1, 5, _a,_b);});f2(v0, v0_n, p1, p2);});
}
}
if(({uint64_t _a=v5;uint64_t _b=v7;o_op(10, 5, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=46ULL;f2(v0, v0_n, p1, p2);});
{
uint64_t v8=0;
uint64_t k2=({uint64_t _a=v5;uint64_t _b=v7;o_op(2, 5, _a,_b);});
uint64_t c2=0;
for(;o_op(8,5, v8, k2);v8=o_op(1,5, v8,1)){
if(c2++>=1000000)o_trap(4);
v6=({uint64_t p1=v6;uint64_t p2=({uint64_t _a=48ULL;uint64_t _b=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=({uint64_t _a=v7;uint64_t _b=v8;o_op(1, 5, _a,_b);});f32(p0, p1, p2);});o_op(1, 5, _a,_b);});f2(v0, v0_n, p1, p2);});
}
}
}
return o_leave(v6);
}
static uint64_t f35(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p1=o_get(v3, v3_n, 0ULL, 5);uint64_t p2=v4;f28(v2, v2_n, p1, p2);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=o_cast(9, 5, o_get(v3, v3_n, 3ULL, 5));uint64_t _b=1000ULL;o_op(2, 9, _a,_b);});
if(({uint64_t _a=v5;uint64_t _b=({uint64_t p0=v4;f26(p0);});o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=v5;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 9, _a,_b);});
}
{uint64_t _i=4ULL;uint64_t _v=o_cast(5, 9, ({uint64_t _a=v6;uint64_t _b=1000ULL;o_op(1, 9, _a,_b);}));o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=v5;o_put(v3, v3_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=o_cast(9, 5, v4);o_op(2, 9, _a,_b);});uint64_t _b=1ULL;o_op(1, 9, _a,_b);});
return o_leave(({uint64_t p2=v5;uint64_t p3=v7;uint64_t p4=o_get(v3, v3_n, 1ULL, 5);uint64_t p5=({uint64_t _a=o_cast(9, 5, o_get(v3, v3_n, 2ULL, 5));uint64_t _b=1000ULL;o_op(2, 9, _a,_b);});f31(v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));
}
static uint64_t f36(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=9ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=({uint64_t p4=v5;f35(v0, v0_n, v1, v1_n, v2, v2_n, v3, v3_n, p4);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(v5);
}
}
}
return o_leave(9ULL);
}
static uint64_t f37(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=23ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v3;uint64_t _b=8388607ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=18446744073709551467ULL;
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v5=({uint64_t _a=v5;uint64_t _b=8388608ULL;o_op(13, 5, _a,_b);});
v6=({uint64_t _a=o_cast(9, 5, v4);uint64_t _b=150ULL;o_op(2, 9, _a,_b);});
}
{uint64_t _i=1ULL;uint64_t _v=v5;o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0);
}
({uint64_t p1=v5;f19(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v7=0ULL;
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(11, 9, _a,_b);})){
({uint64_t p1=2ULL;uint64_t p2=o_cast(5, 9, v6);f21(v0, v0_n, p1, p2);});
}else{
({uint64_t p1=5ULL;uint64_t p2=o_cast(5, 9, ({uint64_t _a=v6;uint64_t _b=0;o_op(17, 9, _a,_b);}));f21(v0, v0_n, p1, p2);});
v7=v6;
}
{uint64_t _i=0ULL;uint64_t _v=({f25(v0, v0_n, v1, v1_n);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_cast(5, 9, ({uint64_t _a=v6;uint64_t _b=1000ULL;o_op(1, 9, _a,_b);}));o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_cast(5, 9, ({uint64_t _a=({uint64_t _a=({uint64_t _a=o_cast(9, 5, o_get(v2, v2_n, 0ULL, 5));uint64_t _b=1ULL;o_op(2, 9, _a,_b);});uint64_t _b=v7;o_op(1, 9, _a,_b);});uint64_t _b=1000ULL;o_op(1, 9, _a,_b);}));o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f38(void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n, void *v4, uint64_t v4_n, uint64_t v5){
o_enter();
uint64_t v6[8]={0};
uint64_t v6_n=8;
o_fill(v6, v6_n, 0ULL, 5);
({uint64_t p3=v5;f37(v2, v2_n, v4, v4_n, v6, v6_n, p3);});
if(({uint64_t _a=o_get(v6, v6_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=48ULL;f2(v0, v0_n, p1, p2);}));
}
uint64_t __attribute__((unused)) v7=({f36(v2, v2_n, v3, v3_n, v4, v4_n, v6, v6_n);});
return o_leave(({uint64_t p1=v1;uint64_t p2=o_get(v6, v6_n, 5ULL, 5);uint64_t p3=v7;uint64_t p4=({uint64_t _a=o_cast(9, 5, o_get(v6, v6_n, 4ULL, 5));uint64_t _b=1000ULL;o_op(2, 9, _a,_b);});f34(v0, v0_n, p1, p2, p3, p4);}));
}
static uint64_t f39(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t v4[16]={0};
uint64_t v4_n=16;
o_fill(v4, v4_n, 0ULL, 5);
uint64_t v5[16]={0};
uint64_t v5_n=16;
o_fill(v5, v5_n, 0ULL, 5);
uint8_t v6[128]={0};
uint64_t v6_n=128;
o_fill(v6, v6_n, 0ULL, 2);
uint64_t __attribute__((unused)) v7=v1;
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2139095040ULL;o_op(12, 5, _a,_b);});uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=31ULL;o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v7=({uint64_t p1=v7;uint64_t p2=45ULL;f2(v0, v0_n, p1, p2);});
}else{
v7=({uint64_t p1=v7;uint64_t p2=43ULL;f2(v0, v0_n, p1, p2);});
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=8388607ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t p1=v7;uint64_t p3=109ULL;f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
return o_leave(({uint64_t p1=v7;uint64_t p3=110ULL;f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=31ULL;o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v7=({uint64_t p1=v7;uint64_t p2=45ULL;f2(v0, v0_n, p1, p2);});
}
return o_leave(({uint64_t p1=v7;uint64_t p5=v3;f38(v0, v0_n, p1, v4, v4_n, v5, v5_n, v6, v6_n, p5);}));
}
static uint64_t f40(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p1=v1;uint64_t p3=112ULL;f4(v0, v0_n, p1, v2, v2_n, p3);});
v5=({uint64_t p1=v5;uint64_t p2=({uint64_t p0=v3;uint64_t p1=54ULL;uint64_t p2=5ULL;f0(p0, p1, p2);});f7(v0, v0_n, p1, p2);});
v5=({uint64_t p1=v5;uint64_t p3=113ULL;f4(v0, v0_n, p1, v2, v2_n, p3);});
v5=({uint64_t p1=v5;uint64_t p2=({uint64_t p0=v3;uint64_t p1=v4;uint64_t p2=({uint64_t _a=54ULL;uint64_t _b=v4;o_op(2, 5, _a,_b);});f0(p0, p1, p2);});f7(v0, v0_n, p1, p2);});
return o_leave(({uint64_t p1=v5;uint64_t p2=93ULL;f2(v0, v0_n, p1, p2);}));
}
static uint64_t f41(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=33ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=35ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=({uint64_t _a=64ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=33ULL;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);});f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=37ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=39ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=({uint64_t _a=64ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=34ULL;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);});f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=({uint64_t _a=64ULL;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=80ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=({uint64_t _a=64ULL;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=81ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=({uint64_t _a=64ULL;uint64_t _b=8ULL;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
uint64_t __attribute__((unused)) v4=({uint64_t p1=v1;uint64_t p3=115ULL;f4(v0, v0_n, p1, v2, v2_n, p3);});
return o_leave(({uint64_t p1=v4;uint64_t p2=v3;f5(v0, v0_n, p1, p2);}));
}
static uint64_t f42(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 0ULL, 5);
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=({uint64_t p0=v3;uint64_t p1=16ULL;uint64_t p2=8ULL;f0(p0, p1, p2);});uint64_t _b=({uint64_t _a=({uint64_t p0=v3;uint64_t p1=34ULL;uint64_t p2=30ULL;f0(p0, p1, p2);});uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p0=o_get(v2, v2_n, 1ULL, 5);uint64_t p1=0ULL;uint64_t p2=18ULL;f0(p0, p1, p2);});uint64_t _b=38ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=36028797018963968ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=18374686479671623680ULL;o_op(13, 5, _a,_b);});
}
return o_leave(({uint64_t p1=v1;uint64_t p2=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 2ULL, 5);uint64_t _b=16ULL;o_op(1, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f9(v0, v0_n, p1, p2);}));
}
static uint64_t f43(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(v1);
}
uint64_t __attribute__((unused)) v3=v2;
if(({uint64_t _a=v2;uint64_t _b=8388608ULL;o_op(11, 5, _a,_b);})){
v3=({uint64_t _a=v2;uint64_t _b=18446744073692774400ULL;o_op(13, 5, _a,_b);});
}
uint64_t __attribute__((unused)) v4=({uint64_t p1=v1;uint64_t p2=43ULL;f2(v0, v0_n, p1, p2);});
return o_leave(({uint64_t p1=v4;uint64_t p2=v3;f10(v0, v0_n, p1, p2);}));
}
static uint64_t f44(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=100ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=97ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=98ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=99ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=0ULL;uint64_t p3=v4;f12(v0, v0_n, p1, p2, p3);}));
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=68ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=65ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=66ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=67ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=1ULL;uint64_t p3=v4;f12(v0, v0_n, p1, p2, p3);}));
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=105ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=108ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=104ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=109ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v4;f9(v0, v0_n, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=115ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=106ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=({uint64_t p0=v4;f11(p0);});f10(v0, v0_n, p1, p2);}));
}
if(({uint64_t _a=v3;uint64_t _b=111ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v4;f43(v0, v0_n, p1, p2);}));
}
if(({uint64_t _a=v3;uint64_t _b=89ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 2ULL, 5);uint64_t _b=16ULL;o_op(1, 5, _a,_b);});uint64_t _b=({uint64_t p0=v4;f11(p0);});o_op(1, 5, _a,_b);});f9(v0, v0_n, p1, p2);}));
}
if(({uint64_t _a=v3;uint64_t _b=121ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v5=({uint64_t p1=v1;uint64_t p2=66ULL;f2(v0, v0_n, p1, p2);});
return o_leave(({uint64_t p1=v5;uint64_t p2=v4;f5(v0, v0_n, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=80ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=81ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v4;uint64_t p3=0ULL;f13(v0, v0_n, p1, p2, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=82ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=({uint64_t _a=v4;uint64_t _b=7ULL;o_op(12, 5, _a,_b);});uint64_t p3=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=3ULL;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});f13(v0, v0_n, p1, p2, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=84ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;f42(v0, v0_n, p1, v2, v2_n);}));
}
return o_leave(v1);
}
static uint64_t f45(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p0=v4;uint64_t p1=o_get(v3, v3_n, 0ULL, 5);uint64_t p2=o_get(v3, v3_n, 1ULL, 5);f17(p0, p1, p2);});
if(({uint64_t _a=v4;uint64_t _b=102ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=v5;f39(v0, v0_n, p1, v2, v2_n, p3);}));
}
if(({uint64_t _a=v4;uint64_t _b=75ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=o_get(v3, v3_n, 0ULL, 5);uint64_t p4=38ULL;f40(v0, v0_n, p1, v2, v2_n, p3, p4);}));
}
if(({uint64_t _a=v4;uint64_t _b=85ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=o_get(v3, v3_n, 0ULL, 5);uint64_t p4=37ULL;f40(v0, v0_n, p1, v2, v2_n, p3, p4);}));
}
if(({uint64_t _a=v4;uint64_t _b=83ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=v5;f41(v0, v0_n, p1, v2, v2_n, p3);}));
}
if(({uint64_t _a=v4;uint64_t _b=71ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=({uint64_t _a=73ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
if(({uint64_t _a=v4;uint64_t _b=103ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=({uint64_t _a=81ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
if(({uint64_t _a=v4;uint64_t _b=122ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=({uint64_t _a=97ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);});f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
if(({uint64_t _a=v4;uint64_t _b=90ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p3=114ULL;f4(v0, v0_n, p1, v2, v2_n, p3);}));
}
return o_leave(v1);
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=78ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=77ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=45ULL;f2(v0, v0_n, p1, p2);}));
}
return o_leave(v1);
}
return o_leave(({uint64_t p1=v1;uint64_t p3=v4;uint64_t p4=v5;f44(v0, v0_n, p1, v3, v3_n, p3, p4);}));
}
static uint64_t f46(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=v1;
uint64_t __attribute__((unused)) v6=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v6;uint64_t _b=64ULL;o_op(8, 5, _a,_b);})){
if(c1++>=64)o_trap(4);
uint64_t __attribute__((unused)) v7=({uint64_t p1=v4;uint64_t p2=v6;f3(v2, v2_n, p1, p2);});
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
if(({uint64_t _a=v7;uint64_t _b=37ULL;o_op(7, 5, _a,_b);})){
v5=({uint64_t p1=v5;uint64_t p2=v7;f2(v0, v0_n, p1, p2);});
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
continue;
}
v5=({uint64_t p1=v5;uint64_t p4=({uint64_t p1=v4;uint64_t p2=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});f3(v2, v2_n, p1, p2);});f45(v0, v0_n, p1, v2, v2_n, v3, v3_n, p4);});
v6=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v5);
}
static uint64_t f47(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(c1++>=32)o_trap(4);
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f48(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=105ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=255ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=105ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));
}
return o_leave(({uint64_t _a=105ULL;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=255ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v0;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
uint64_t __attribute__((unused)) v2=({uint64_t p0=v0;f47(p0);});
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=31ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(105ULL);
}
return o_leave(({uint64_t _a=105ULL;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}));
}
return o_leave(105ULL);
}
static uint64_t f49(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=4095ULL;o_op(12, 5, _a,_b);});uint64_t _b=2084ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=({uint64_t _a=255ULL;uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=126747136ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(v1);
}
uint64_t __attribute__((unused)) v5=({uint64_t p0=v3;uint64_t p1=32ULL;uint64_t p2=32ULL;f0(p0, p1, p2);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v4;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t p1=v1;uint64_t p3=({uint64_t p0=v5;uint64_t p1=v6;f48(p0, p1);});f4(v0, v0_n, p1, v2, v2_n, p3);});
v7=({uint64_t p1=v7;uint64_t p2=32ULL;f2(v0, v0_n, p1, p2);});
v7=({uint64_t p1=v7;uint64_t p2=0ULL;uint64_t p3=({uint64_t p0=v3;uint64_t p1=16ULL;uint64_t p2=8ULL;f0(p0, p1, p2);});f12(v0, v0_n, p1, p2, p3);});
v7=({uint64_t p1=v7;uint64_t p2=44ULL;f2(v0, v0_n, p1, p2);});
v7=({uint64_t p1=v7;uint64_t p2=32ULL;f2(v0, v0_n, p1, p2);});
v7=({uint64_t p1=v7;uint64_t p2=0ULL;uint64_t p3=({uint64_t p0=v3;uint64_t p1=24ULL;uint64_t p2=8ULL;f0(p0, p1, p2);});f12(v0, v0_n, p1, p2, p3);});
v7=({uint64_t p1=v7;uint64_t p2=44ULL;f2(v0, v0_n, p1, p2);});
v7=({uint64_t p1=v7;uint64_t p2=32ULL;f2(v0, v0_n, p1, p2);});
v7=({uint64_t p1=v7;uint64_t p2=({uint64_t p0=v5;f11(p0);});f10(v0, v0_n, p1, p2);});
v7=({uint64_t p1=v7;uint64_t p2=44ULL;f2(v0, v0_n, p1, p2);});
v7=({uint64_t p1=v7;uint64_t p2=32ULL;f2(v0, v0_n, p1, p2);});
return o_leave(({uint64_t p1=v7;uint64_t p2=0ULL;uint64_t p3=v6;f12(v0, v0_n, p1, p2, p3);}));
}
static uint64_t f50(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=o_get(v4, v4_n, 0ULL, 5);uint64_t _b=61440ULL;o_op(13, 5, _a,_b);});uint64_t _b=0;o_op(18, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=2199023255551ULL;uint64_t _b=({uint64_t _a=o_get(v4, v4_n, 1ULL, 5);uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v5;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=v5;o_op(12, 5, _a,_b);});o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=v6;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=v6;o_op(12, 5, _a,_b);});o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f51(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t v4[2]={0};
uint64_t v4_n=2;
o_fill(v4, v4_n, 0ULL, 5);
{
uint64_t v5=0;
uint64_t k1=61ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, ({uint64_t _a=2ULL;uint64_t _b=v5;o_op(3, 5, _a,_b);}), 5);
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=4095ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=4095ULL;o_op(12, 5, _a,_b);});o_op(7, 5, _a,_b);})){
continue;
}
({uint64_t p1=v5;f18(v1, v1_n, p1, v4, v4_n);});
if(({uint64_t _a=({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=v6;uint64_t p3=o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=2ULL;uint64_t _b=v5;o_op(3, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);f50(p0, p1, p2, p3, v4, v4_n);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(v5);
}
}
}
return o_leave(61ULL);
}
static uint64_t f52(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=0ULL;uint64_t p3=111ULL;f4(v0, v0_n, p1, v1, v1_n, p3);});
v3=({uint64_t p1=v3;uint64_t p2=o_get(v2, v2_n, 1ULL, 5);f8(v0, v0_n, p1, p2);});
v3=({uint64_t p1=v3;uint64_t p2=o_get(v2, v2_n, 0ULL, 5);f8(v0, v0_n, p1, p2);});
return o_leave(({uint64_t _a=v3;uint64_t _b=65536ULL;o_op(1, 5, _a,_b);}));
}
static uint64_t f53(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t p0=v1;uint64_t p1=12ULL;uint64_t p2=3ULL;f0(p0, p1, p2);});
uint64_t __attribute__((unused)) v3=({uint64_t p0=v1;uint64_t p1=15ULL;uint64_t p2=1ULL;f0(p0, p1, p2);});
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=7ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t p1=0ULL;uint64_t p2=64ULL;f2(v0, v0_n, p1, p2);});
v4=({uint64_t p1=v4;uint64_t p2=v2;uint64_t p3=v3;f13(v0, v0_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v4;uint64_t p2=32ULL;f2(v0, v0_n, p1, p2);}));
}
static uint64_t f54(void *v0, uint64_t v0_n){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=2318338018808057677ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=26917ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=2318338018808057677ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=25125ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=16ULL;uint64_t _v=2318338018808057677ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=17ULL;uint64_t _v=25125ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=24ULL;uint64_t _v=2318338018807788115ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=25ULL;uint64_t _v=21285ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=32ULL;uint64_t _v=2318338018806809676ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=33ULL;uint64_t _v=19237ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=40ULL;uint64_t _v=2675195585967113292ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=41ULL;uint64_t _v=322745412708ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=48ULL;uint64_t _v=3189715256884806732ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=49ULL;uint64_t _v=5580064ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=56ULL;uint64_t _v=2320539280629515340ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=57ULL;uint64_t _v=93617941922853ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=64ULL;uint64_t _v=3198722455854206281ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=65ULL;uint64_t _v=3198159505500218656ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=66ULL;uint64_t _v=6497568ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=72ULL;uint64_t _v=3198722455854206281ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=73ULL;uint64_t _v=3189152306245477664ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=74ULL;uint64_t _v=6497568ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=80ULL;uint64_t _v=4920559924104219977ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=81ULL;uint64_t _v=7216209377912761925ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=82ULL;uint64_t _v=7576497323101200428ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=83ULL;uint64_t _v=1663377452ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=88ULL;uint64_t _v=3335276362595388745ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=89ULL;uint64_t _v=2318338018805691221ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=90ULL;uint64_t _v=2318335819782054181ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=91ULL;uint64_t _v=25381ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=96ULL;uint64_t _v=7216209382491832649ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=97ULL;uint64_t _v=6075391272087986220ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=98ULL;uint64_t _v=2318334721041702956ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=99ULL;uint64_t _v=7144151758890421541ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=104ULL;uint64_t _v=7216209382491832649ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=105ULL;uint64_t _v=6075391272087986220ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=106ULL;uint64_t _v=2318334721041702956ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=107ULL;uint64_t _v=109011104723749ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=112ULL;uint64_t _v=6076847033519460172ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=113ULL;uint64_t _v=3197878030523704608ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=114ULL;uint64_t _v=3198440980476994848ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=115ULL;uint64_t _v=23121768204739872ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=120ULL;uint64_t _v=6076847033519460172ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=121ULL;uint64_t _v=3197878030523704608ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=122ULL;uint64_t _v=3198440980477453600ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=123ULL;uint64_t _v=23121768204739872ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=128ULL;uint64_t _v=3698913599453284435ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=129ULL;uint64_t _v=3198722455938543154ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=130ULL;uint64_t _v=3200129830337193248ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=131ULL;uint64_t _v=6497568ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=136ULL;uint64_t _v=3698913599453284435ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=137ULL;uint64_t _v=3198722455938543154ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=138ULL;uint64_t _v=3198159505500218656ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=139ULL;uint64_t _v=6497568ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=144ULL;uint64_t _v=3698913573683480659ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=145ULL;uint64_t _v=7000036570848108594ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=146ULL;uint64_t _v=7144151758890410028ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=152ULL;uint64_t _v=3698913573683480659ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=153ULL;uint64_t _v=7000036570848108594ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=154ULL;uint64_t _v=7144151759007850540ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=160ULL;uint64_t _v=5126554673377268553ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=161ULL;uint64_t _v=2675213281549769253ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=162ULL;uint64_t _v=2675186905684913232ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=163ULL;uint64_t _v=2675186978699357281ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=164ULL;uint64_t _v=82ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=168ULL;uint64_t _v=5126554673377268553ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=169ULL;uint64_t _v=2675213281549769253ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=170ULL;uint64_t _v=2675186905684913232ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=171ULL;uint64_t _v=2675187013059095649ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=172ULL;uint64_t _v=82ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=176ULL;uint64_t _v=5126554673377268553ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=177ULL;uint64_t _v=2675213281549769253ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=178ULL;uint64_t _v=2675186905684913232ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=179ULL;uint64_t _v=2675186841260403809ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=180ULL;uint64_t _v=82ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=184ULL;uint64_t _v=2318338018807399763ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=185ULL;uint64_t _v=2318335819782054181ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=186ULL;uint64_t _v=21029ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=192ULL;uint64_t _v=2318338018807399763ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=193ULL;uint64_t _v=2318343516363448613ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=194ULL;uint64_t _v=21029ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=200ULL;uint64_t _v=3198722455854399814ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=201ULL;uint64_t _v=27625367831389472ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=208ULL;uint64_t _v=3198722455854399814ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=209ULL;uint64_t _v=7072108406061999392ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=216ULL;uint64_t _v=3198722455989734726ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=217ULL;uint64_t _v=27625367831389472ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=224ULL;uint64_t _v=3198722455804659270ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=225ULL;uint64_t _v=3198159505500218656ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=226ULL;uint64_t _v=6497568ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=232ULL;uint64_t _v=7216209541557275974ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=233ULL;uint64_t _v=7072094164835704876ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=234ULL;uint64_t _v=1378164780ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=240ULL;uint64_t _v=7432397682590962502ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=241ULL;uint64_t _v=3193092956320842030ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=242ULL;uint64_t _v=3197878030522459424ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=243ULL;uint64_t _v=23121768204084512ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=248ULL;uint64_t _v=3617312090437857865ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=249ULL;uint64_t _v=3198722455551300910ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=250ULL;uint64_t _v=6432032ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=256ULL;uint64_t _v=3617312090437857865ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=257ULL;uint64_t _v=3198722455551300398ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=258ULL;uint64_t _v=6432032ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=264ULL;uint64_t _v=3328779515522003526ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=265ULL;uint64_t _v=6074843813981278804ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=266ULL;uint64_t _v=7072094164886036570ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=272ULL;uint64_t _v=6071225231052517958ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=273ULL;uint64_t _v=4984998065855154502ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=274ULL;uint64_t _v=3328779451096059188ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=275ULL;uint64_t _v=6864120659606454608ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=276ULL;uint64_t _v=2324806583807264077ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=277ULL;uint64_t _v=2318334720270427173ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=278ULL;uint64_t _v=109011104719397ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=280ULL;uint64_t _v=3550598122910921286ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=281ULL;uint64_t _v=4706312403745648182ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=282ULL;uint64_t _v=7216209446867782467ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=283ULL;uint64_t _v=7072094164835704876ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=288ULL;uint64_t _v=4050478989600181576ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=289ULL;uint64_t _v=4768804251234088497ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=290ULL;uint64_t _v=2318338018805952838ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=291ULL;uint64_t _v=2318335819782054181ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=292ULL;uint64_t _v=25381ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=296ULL;uint64_t _v=4050478989600181576ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=297ULL;uint64_t _v=2675193374460229169ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=298ULL;uint64_t _v=2675186974404389988ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=299ULL;uint64_t _v=425824627810ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=304ULL;uint64_t _v=4050478989600181585ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=305ULL;uint64_t _v=4984977033347871283ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=306ULL;uint64_t _v=3696668340873547060ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=307ULL;uint64_t _v=3197878030523704608ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=308ULL;uint64_t _v=27906842808165664ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=312ULL;uint64_t _v=3627725565148419405ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=313ULL;uint64_t _v=27625367831586080ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=320ULL;uint64_t _v=5783556705426494797ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=321ULL;uint64_t _v=27625367831586080ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=328ULL;uint64_t _v=5860117899091793229ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=329ULL;uint64_t _v=27625367831586080ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=336ULL;uint64_t _v=3622948187125732685ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=337ULL;uint64_t _v=27625367831586080ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=344ULL;uint64_t _v=2340223935431787596ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=345ULL;uint64_t _v=7166182912787506213ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=346ULL;uint64_t _v=3341993472822486363ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=347ULL;uint64_t _v=401296667702ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=352ULL;uint64_t _v=2340223935431791699ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=353ULL;uint64_t _v=6720256143407670628ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=354ULL;uint64_t _v=8008864919659291995ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=355ULL;uint64_t _v=421529660509ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=360ULL;uint64_t _v=6076251196916974668ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=361ULL;uint64_t _v=6005923036305182546ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=362ULL;uint64_t _v=8315162376355980576ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=363ULL;uint64_t _v=7000101651917134691ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=364ULL;uint64_t _v=102731946931758ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=368ULL;uint64_t _v=5570159188791215180ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=369ULL;uint64_t _v=2675231981938036553ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=370ULL;uint64_t _v=6585233959454518372ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=371ULL;uint64_t _v=3904164740051321381ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=372ULL;uint64_t _v=1567565108ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=376ULL;uint64_t _v=6076251196916978771ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=377ULL;uint64_t _v=6005923036305182546ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=378ULL;uint64_t _v=4838373857375380512ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=379ULL;uint64_t _v=2680827251018521437ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=380ULL;uint64_t _v=107911593090415ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=384ULL;uint64_t _v=3334121982912320577ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=385ULL;uint64_t _v=5715723593321825345ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=386ULL;uint64_t _v=5773706365401712462ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=387ULL;uint64_t _v=7214815411678751828ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=388ULL;uint64_t _v=6583492058011628389ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=389ULL;uint64_t _v=2318330387017654565ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=390ULL;uint64_t _v=25125ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=392ULL;uint64_t _v=2318338018807858252ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=393ULL;uint64_t _v=26249900243101019ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=400ULL;uint64_t _v=3125820689686418515ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=401ULL;uint64_t _v=27625367831135013ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=408ULL;uint64_t _v=7216209686915400780ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=409ULL;uint64_t _v=6732641067110965292ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=416ULL;uint64_t _v=4696781901632718157ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=417ULL;uint64_t _v=91643112737868ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=424ULL;uint64_t _v=6291322170098731091ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=425ULL;uint64_t _v=7216209352696275022ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=426ULL;uint64_t _v=7504439729063272492ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=427ULL;uint64_t _v=1831149612ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=432ULL;uint64_t _v=5496153161867872339ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=433ULL;uint64_t _v=7216209352696275033ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=434ULL;uint64_t _v=7504439729063272492ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=435ULL;uint64_t _v=1831149612ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=440ULL;uint64_t _v=5263182ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=448ULL;uint64_t _v=1414092869ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=456ULL;uint64_t _v=92518431674946ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=464ULL;uint64_t _v=4847370932954813250ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=465ULL;uint64_t _v=5639992527027588687ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=466ULL;uint64_t _v=6423575818897006676ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=472ULL;uint64_t _v=4995105803319464770ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=473ULL;uint64_t _v=4992049169717677891ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=474ULL;uint64_t _v=520313918542ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=480ULL;uint64_t _v=4849912062518903106ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=481ULL;uint64_t _v=4782631786775921710ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=482ULL;uint64_t _v=2325913809262432076ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=483ULL;uint64_t _v=3176496ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=512ULL;uint64_t _v=6354091206005183059ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=520ULL;uint64_t _v=6426148800043110995ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=528ULL;uint64_t _v=6498206394081038931ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=536ULL;uint64_t _v=4920535898042356307ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=537ULL;uint64_t _v=22574ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=544ULL;uint64_t _v=4920535898042356307ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=545ULL;uint64_t _v=22830ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=552ULL;uint64_t _v=4920535898042356307ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=553ULL;uint64_t _v=23086ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=560ULL;uint64_t _v=5279712180522930771ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=561ULL;uint64_t _v=68ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=568ULL;uint64_t _v=5423265565250638419ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=569ULL;uint64_t _v=20300ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=576ULL;uint64_t _v=5423265565250638419ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=577ULL;uint64_t _v=18760ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=584ULL;uint64_t _v=70ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=592ULL;uint64_t _v=21580ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=600ULL;uint64_t _v=20805ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=608ULL;uint64_t _v=17740ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=616ULL;uint64_t _v=21575ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=624ULL;uint64_t _v=17742ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=632ULL;uint64_t _v=17735ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=640ULL;uint64_t _v=84ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=648ULL;uint64_t _v=70ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=656ULL;uint64_t _v=21580ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=664ULL;uint64_t _v=20805ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=672ULL;uint64_t _v=17740ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=680ULL;uint64_t _v=21575ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=688ULL;uint64_t _v=17742ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=696ULL;uint64_t _v=17735ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=704ULL;uint64_t _v=5068110ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=712ULL;uint64_t _v=5128526ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=720ULL;uint64_t _v=5592140ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=728ULL;uint64_t _v=5591365ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=736ULL;uint64_t _v=5588300ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=744ULL;uint64_t _v=5592135ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=752ULL;uint64_t _v=5588302ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=760ULL;uint64_t _v=5588295ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=768ULL;uint64_t _v=84ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=776ULL;uint64_t _v=3691822ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=784ULL;uint64_t _v=3691310ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=792ULL;uint64_t _v=909202734ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=800ULL;uint64_t _v=909202222ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=816ULL;uint64_t _v=3421742ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=824ULL;uint64_t _v=942813486ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=832ULL;uint64_t _v=61788224640302ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=840ULL;uint64_t _v=1145130313ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=848ULL;uint64_t _v=6219274471530909001ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=856ULL;uint64_t _v=4918292731127745865ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=857ULL;uint64_t _v=68ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=864ULL;uint64_t _v=5496734803384421705ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=872ULL;uint64_t _v=1312902737ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=880ULL;uint64_t _v=4607561ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=888ULL;uint64_t _v=516203495231ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=896ULL;uint64_t _v=2016435043ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=904ULL;uint64_t _v=2016435037ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=912ULL;uint64_t _v=842224942ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=920ULL;uint64_t _v=21075ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f55(void *v0, uint64_t v0_n){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=30722ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=3840ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=4278219266ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3840ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=29186ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=3840ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=31001ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=4278221698ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=2048ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=4278221698ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=2560ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=4278220716ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=134219776ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=4278220716ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=15ULL;uint64_t _v=134220288ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=16ULL;uint64_t _v=29220ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=17ULL;uint64_t _v=126747136ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=18ULL;uint64_t _v=31780ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=19ULL;uint64_t _v=260964864ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=30757ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=126746624ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=22ULL;uint64_t _v=29223ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=23ULL;uint64_t _v=126746624ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=24ULL;uint64_t _v=29200ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=25ULL;uint64_t _v=134209536ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=26ULL;uint64_t _v=30736ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=27ULL;uint64_t _v=134209536ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=28ULL;uint64_t _v=29202ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=29ULL;uint64_t _v=917504ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=30ULL;uint64_t _v=30738ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=31ULL;uint64_t _v=917504ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=32ULL;uint64_t _v=30745ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=33ULL;uint64_t _v=71168ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=34ULL;uint64_t _v=29209ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=35ULL;uint64_t _v=71168ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=36ULL;uint64_t _v=29209ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=37ULL;uint64_t _v=1536ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=38ULL;uint64_t _v=30745ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=39ULL;uint64_t _v=1536ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=40ULL;uint64_t _v=29196ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=41ULL;uint64_t _v=112ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=42ULL;uint64_t _v=30732ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=43ULL;uint64_t _v=112ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=44ULL;uint64_t _v=31756ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=45ULL;uint64_t _v=134217840ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=46ULL;uint64_t _v=29191ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=47ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=48ULL;uint64_t _v=30727ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=49ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=50ULL;uint64_t _v=29217ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=51ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=52ULL;uint64_t _v=9223372036854805025ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=53ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=54ULL;uint64_t _v=29216ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=55ULL;uint64_t _v=4194304ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=56ULL;uint64_t _v=29219ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=57ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=58ULL;uint64_t _v=29193ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=59ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=60ULL;uint64_t _v=29195ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=61ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=62ULL;uint64_t _v=29253ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=63ULL;uint64_t _v=2101248ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=64ULL;uint64_t _v=29253ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=65ULL;uint64_t _v=2102272ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=66ULL;uint64_t _v=29445ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=67ULL;uint64_t _v=2158592ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=68ULL;uint64_t _v=29246ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=69ULL;uint64_t _v=75526144ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=70ULL;uint64_t _v=29246ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=71ULL;uint64_t _v=4351ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=72ULL;uint64_t _v=29244ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=73ULL;uint64_t _v=268288ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=74ULL;uint64_t _v=29244ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=75ULL;uint64_t _v=6144ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=76ULL;uint64_t _v=29306ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=77ULL;uint64_t _v=11264ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=78ULL;uint64_t _v=29448ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=79ULL;uint64_t _v=2048ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=80ULL;uint64_t _v=29448ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=81ULL;uint64_t _v=4096ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=82ULL;uint64_t _v=29448ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=83ULL;uint64_t _v=5120ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=84ULL;uint64_t _v=29448ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=85ULL;uint64_t _v=3072ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=86ULL;uint64_t _v=31105ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=87ULL;uint64_t _v=203297024ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=88ULL;uint64_t _v=31110ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=89ULL;uint64_t _v=202379520ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=90ULL;uint64_t _v=31105ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=91ULL;uint64_t _v=203380992ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=92ULL;uint64_t _v=31105ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=93ULL;uint64_t _v=203364608ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=94ULL;uint64_t _v=31110ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=95ULL;uint64_t _v=202463488ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=96ULL;uint64_t _v=9223372036854806952ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=97ULL;uint64_t _v=136270080ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=98ULL;uint64_t _v=31108ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=99ULL;uint64_t _v=134219776ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=100ULL;uint64_t _v=31112ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=101ULL;uint64_t _v=134219776ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=102ULL;uint64_t _v=31108ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=103ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=104ULL;uint64_t _v=31122ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=105ULL;uint64_t _v=45056ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=106ULL;uint64_t _v=576460752303456137ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=107ULL;uint64_t _v=917504ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=108ULL;uint64_t _v=864691128455167881ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=109ULL;uint64_t _v=917504ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=110ULL;uint64_t _v=31000ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=111ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=112ULL;uint64_t _v=31053ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=113ULL;uint64_t _v=58720256ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=114ULL;uint64_t _v=31047ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=115ULL;uint64_t _v=58720256ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=116ULL;uint64_t _v=31045ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=117ULL;uint64_t _v=58720768ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=118ULL;uint64_t _v=31041ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=119ULL;uint64_t _v=58720768ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=120ULL;uint64_t _v=31517ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=121ULL;uint64_t _v=65536ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f56(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t v2[1024]={0};
uint64_t v2_n=1024;
o_fill(v2, v2_n, 0ULL, 5);
uint64_t v3[122]={0};
uint64_t v3_n=122;
o_fill(v3, v3_n, 0ULL, 5);
({f54(v2, v2_n);});
({f55(v3, v3_n);});
uint64_t __attribute__((unused)) v4=({uint64_t p1=o_get(v0, v0_n, 0ULL, 5);f53(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=({uint64_t _a=61440ULL;uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=2199023255551ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t p1=v4;uint64_t p3=v5;uint64_t p4=v6;f49(v1, v1_n, p1, v2, v2_n, p3, p4);});
if(({uint64_t _a=v7;uint64_t _b=v4;o_op(10, 5, _a,_b);})){
return o_leave(v7);
}
uint64_t __attribute__((unused)) v8=({uint64_t p2=v5;uint64_t p3=v6;f51(v3, v3_n, v2, v2_n, p2, p3);});
if(({uint64_t _a=v8;uint64_t _b=61ULL;o_op(8, 5, _a,_b);})){
return o_leave(({uint64_t p1=v4;uint64_t p4=v8;f46(v1, v1_n, p1, v2, v2_n, v0, v0_n, p4);}));
}
return o_leave(({f52(v1, v1_n, v2, v2_n, v0, v0_n);}));
}
uint32_t omega_sm121_dis(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n){
if(!o_buf(result,1,1,5))return 32;
if(!o_buf(p0, p0_n,4, 5))return 32;
if(o_overlap(result,8,p0, p0_n*8))return 32;
if(!o_buf(p1, p1_n,256, 2))return 32;
if(o_overlap(result,8,p1, p1_n*1))return 32;
if(o_overlap(p1, p1_n*1, p0, p0_n*8))return 32;
if(o_busy)return 33;
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f56(p0, p0_n, p1, p1_n);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
