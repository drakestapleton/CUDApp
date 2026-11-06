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
static uint64_t f0(uint64_t v0, uint64_t v1);
static uint64_t f1(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f2(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f3(const void *v0, uint64_t v0_n);
static uint64_t f4(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f5(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f6(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f7(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f8(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f9(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f10(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f11(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f12(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f13(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f14(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f15(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f16(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f17(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f18(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f19(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f20(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f21(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f22(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f23(uint64_t v0);
static uint64_t f24(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f25(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f26(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f27(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f28(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f29(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f30(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f31(uint64_t v0);
static uint64_t f32(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f33(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f34(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f35(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f36(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f37(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f38(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f39(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f40(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f41(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f42(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f43(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f44(uint64_t v0, uint64_t v1);
static uint64_t f45(uint64_t v0, uint64_t v1);
static uint64_t f46(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f0(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=31ULL;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}else{
return o_leave(0ULL);
}
}
static uint64_t f1(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v2=600000ULL;
}
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 2ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v3=5000ULL;
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=1000000ULL;o_op(3, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v3;uint64_t _b=1000000ULL;o_op(3, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, 3ULL, 5);
{uint64_t _i=0ULL;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v5;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t _a=v6;uint64_t _b=v4;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t _a=v6;uint64_t _b=v5;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=4ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
if(({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=5ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f2(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=o_get(v0, v0_n, 2ULL, 5);o_op(8, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=o_get(v0, v0_n, 3ULL, 5);o_op(2, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f3(const void *v0, uint64_t v0_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=o_get(v0, v0_n, 17ULL, 5);o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=o_get(v0, v0_n, 18ULL, 5);o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 21ULL, 5);uint64_t _b=o_get(v0, v0_n, 19ULL, 5);o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 22ULL, 5);uint64_t _b=o_get(v0, v0_n, 20ULL, 5);o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f4(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}else{
return o_leave(0ULL);
}
}else{
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f0(p0, p1);}));
}
}
static uint64_t f5(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f6(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=({f3(v0, v0_n);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 23ULL, 5);uint64_t _b=1ULL;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=o_get(v0, v0_n, 23ULL, 5);uint64_t _b=1ULL;o_op(12, 5, _a,_b);});
{uint64_t _i=7ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 16ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=2ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 10ULL, 5);uint64_t _b=o_get(v0, v0_n, 13ULL, 5);o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=2ULL;uint64_t _v=o_get(v0, v0_n, 12ULL, 5);o_put(v1, v1_n, _i,_v,5);}
}
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=3ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 10ULL, 5);uint64_t _b=o_get(v0, v0_n, 15ULL, 5);o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=3ULL;uint64_t _v=o_get(v0, v0_n, 14ULL, 5);o_put(v1, v1_n, _i,_v,5);}
}
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=o_get(v0, v0_n, 10ULL, 5);o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=2ULL;uint64_t _v=o_get(v0, v0_n, 12ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v0, v0_n, 14ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
}else{
{uint64_t _i=2ULL;uint64_t _v=o_get(v0, v0_n, 12ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v0, v0_n, 14ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f7(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 10ULL, 5);
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=o_get(v0, v0_n, 11ULL, 5);o_op(2, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(11, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=5ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f8(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t _a=o_get(v0, v0_n, 10ULL, 5);uint64_t _b=o_get(v0, v0_n, 11ULL, 5);o_op(2, 5, _a,_b);});
if(({uint64_t _a=o_get(v0, v0_n, 9ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=4ULL;uint64_t p2=v2;f5(v1, v1_n, p1, p2);});
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=({uint64_t p0=o_get(v0, v0_n, 1ULL, 5);uint64_t p1=o_get(v0, v0_n, 3ULL, 5);uint64_t p2=o_get(v0, v0_n, 2ULL, 5);f4(p0, p1, p2);});
if(({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=o_get(v0, v0_n, 6ULL, 5);o_op(7, 5, _a,_b);})){
v3=0ULL;
}
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
({f6(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 7ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 8ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=3ULL;uint64_t p2=v2;f5(v1, v1_n, p1, p2);});
({f6(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
}
({f6(v0, v0_n, v1, v1_n);});
({f7(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f9(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=v1;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
static uint64_t f10(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=3ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=4ULL;o_op(3, 5, _a,_b);});
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, 6ULL, 5);uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 5ULL, 5);f9(p0, p1, p2);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, 8ULL, 5);uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 7ULL, 5);f9(p0, p1, p2);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=5ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, 10ULL, 5);uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 9ULL, 5);f9(p0, p1, p2);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 11ULL, 5);uint64_t _b=3ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=7ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, 13ULL, 5);uint64_t p1=o_get(v0, v0_n, 4ULL, 5);uint64_t p2=o_get(v0, v0_n, 12ULL, 5);f9(p0, p1, p2);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=8ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 14ULL, 5);uint64_t _b=18446744073709551614ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=9ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 15ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=10ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 16ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=11ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 17ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=12ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 14ULL, 5);uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 18ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=13ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
}
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f11(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=255ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f12(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f13(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p1=v4;uint64_t p2=v2;uint64_t p3=v3;f11(v1, v1_n, p1, p2, p3);});
}else{
({uint64_t p1=v5;f12(v1, v1_n, p1);});
}
return o_leave(0);
}
static uint64_t f14(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=3ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=255ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
({uint64_t p1=255ULL;f12(v1, v1_n, p1);});
}
return o_leave(0);
}
static uint64_t f15(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=1ULL;uint64_t p2=13ULL;uint64_t p3=3ULL;f11(v1, v1_n, p1, p2, p3);});
}else{
({uint64_t p1=255ULL;f12(v1, v1_n, p1);});
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=1ULL;uint64_t p2=10ULL;uint64_t p3=1ULL;f11(v1, v1_n, p1, p2, p3);});
}else{
if(({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=1ULL;uint64_t p2=10ULL;uint64_t p3=1ULL;f11(v1, v1_n, p1, p2, p3);});
}else{
({uint64_t p1=255ULL;f12(v1, v1_n, p1);});
}
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=1ULL;uint64_t p2=11ULL;uint64_t p3=4ULL;f11(v1, v1_n, p1, p2, p3);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=1ULL;uint64_t p2=11ULL;uint64_t p3=2ULL;f11(v1, v1_n, p1, p2, p3);});
}else{
({uint64_t p1=255ULL;f12(v1, v1_n, p1);});
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=1ULL;uint64_t p2=11ULL;uint64_t p3=20ULL;f11(v1, v1_n, p1, p2, p3);});
}else{
({uint64_t p1=255ULL;f12(v1, v1_n, p1);});
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=1ULL;uint64_t p3=5ULL;uint64_t p4=2ULL;uint64_t p5=255ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=2ULL;uint64_t p3=6ULL;uint64_t p4=2ULL;uint64_t p5=255ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=3ULL;uint64_t p3=7ULL;uint64_t p4=2ULL;uint64_t p5=255ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=4ULL;uint64_t p3=8ULL;uint64_t p4=2ULL;uint64_t p5=255ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=4ULL;uint64_t p3=9ULL;uint64_t p4=2ULL;uint64_t p5=255ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
({uint64_t p1=255ULL;f12(v1, v1_n, p1);});
return o_leave(0);
}
static uint64_t f16(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=4ULL;uint64_t p3=10ULL;uint64_t p4=2ULL;uint64_t p5=1ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=5ULL;uint64_t p3=11ULL;uint64_t p4=2ULL;uint64_t p5=2ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=6ULL;uint64_t p3=12ULL;uint64_t p4=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
({uint64_t p2=6ULL;uint64_t p3=13ULL;uint64_t p4=3ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=7ULL;uint64_t p3=14ULL;uint64_t p4=2ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
}else{
({uint64_t p1=3ULL;f12(v1, v1_n, p1);});
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=8ULL;uint64_t p3=15ULL;uint64_t p4=2ULL;uint64_t p5=255ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=8ULL;uint64_t p3=16ULL;uint64_t p4=2ULL;uint64_t p5=4ULL;f13(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t p1=2ULL;uint64_t p2=9ULL;uint64_t p3=17ULL;f11(v1, v1_n, p1, p2, p3);});
}else{
({uint64_t p1=5ULL;f12(v1, v1_n, p1);});
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=6ULL;f12(v1, v1_n, p1);});
}else{
({uint64_t p1=255ULL;f12(v1, v1_n, p1);});
}
return o_leave(0);
}
({uint64_t p1=255ULL;f12(v1, v1_n, p1);});
return o_leave(0);
}
static uint64_t f17(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
if(({uint64_t _a=v2;uint64_t _b=18014398509481983ULL;o_op(10, 5, _a,_b);})){
v3=18446744073709551615ULL;
}else{
v3=({uint64_t _a=v2;uint64_t _b=1024ULL;o_op(3, 5, _a,_b);});
}
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v5=0ULL;
if(({uint64_t _a=v4;uint64_t _b=17592186044415ULL;o_op(10, 5, _a,_b);})){
v5=18446744073709551615ULL;
}else{
v5=({uint64_t _a=v4;uint64_t _b=20ULL;o_op(15, 5, _a,_b);});
}
{uint64_t _i=1ULL;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=v5;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v6;uint64_t _b=v3;o_op(10, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=11ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v7=({uint64_t _a=v3;uint64_t _b=v6;o_op(2, 5, _a,_b);});
if(({uint64_t _a=v7;uint64_t _b=v5;o_op(8, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=11ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f18(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=4095ULL;o_op(1, 5, _a,_b);});uint64_t _b=18446744073709547520ULL;o_op(12, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f19(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 2ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
{uint64_t _i=2ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=2ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 5ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v3=o_get(v0, v0_n, 4ULL, 5);
}
{uint64_t _i=3ULL;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f20(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=o_get(v0, v0_n, 2ULL, 5);o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f21(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 2ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=v3;o_op(11, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=v3;o_op(2, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f22(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
{uint64_t _i=1ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f23(uint64_t v0){
o_enter();
if(v0){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f24(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, 4ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, 5ULL, 5);
uint64_t __attribute__((unused)) v7=o_get(v0, v0_n, 6ULL, 5);
uint64_t __attribute__((unused)) v8=o_get(v0, v0_n, 7ULL, 5);
uint64_t __attribute__((unused)) v9=0ULL;
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=v4;o_op(6, 5, _a,_b);});f23(p0);});
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t p0=({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v4;o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=v6;o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=12ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v7;uint64_t _b=v8;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});
}
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=v5;o_op(10, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v6;uint64_t _b=v7;o_op(9, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=v6;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v7;uint64_t _b=v8;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=274877906944ULL;o_op(10, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=2147483648ULL;o_op(8, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=v6;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v7;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=v4;o_op(9, 5, _a,_b);});f23(p0);});
}
if(({uint64_t _a=v2;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v6;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=v4;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v9=({uint64_t _a=v9;uint64_t _b=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=v6;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
v9=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
{uint64_t _i=0ULL;uint64_t _v=v9;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f25(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v4=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=63ULL;o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
}
{uint64_t _i=0ULL;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=381747182ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f26(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=o_get(v0, v0_n, 4ULL, 5);o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=o_get(v0, v0_n, 6ULL, 5);o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f27(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 2ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=18446744073709547520ULL;o_op(10, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=4095ULL;o_op(1, 5, _a,_b);});uint64_t _b=18446744073709547520ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=4096ULL;o_op(8, 5, _a,_b);})){
v4=4096ULL;
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=4294967232ULL;o_op(10, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=63ULL;o_op(1, 5, _a,_b);});uint64_t _b=64ULL;o_op(4, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=64ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 7ULL, 5);
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=({uint64_t _a=v4;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v5=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=({uint64_t _a=v4;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=6ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});o_op(12, 5, _a,_b);});
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v5;uint64_t _b=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 6ULL, 5);uint64_t _b=2172715008ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v4=1ULL;
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=49ULL;o_op(6, 5, _a,_b);})){
v4=0ULL;
}
}
{uint64_t _i=0ULL;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=v4;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f28(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=5000ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=5000ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=5000ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=5000ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f29(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 4ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 5ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, 6ULL, 5);
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=v3;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
}
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v4;uint64_t _b=v5;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
}
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 7ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v2;uint64_t _b=v3;o_op(6, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});
v6=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=v5;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});o_op(13, 5, _a,_b);});
if(({uint64_t _a=v6;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f30(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=o_get(v0, v0_n, 11ULL, 5);o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=o_get(v0, v0_n, 12ULL, 5);o_op(8, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 4ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 13ULL, 5);
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v3;o_op(13, 5, _a,_b);});uint64_t _b=18446744073709547520ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=v2;uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=3ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{
uint64_t v4=0;
uint64_t k1=6ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=14ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);uint64_t _b=o_get(v0, v0_n, ({uint64_t _a=5ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);o_op(10, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=11ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
}
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f31(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(12ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(48ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(192ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(768ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(1024ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(2048ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(28672ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(491520ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(524288ULL);
}
return o_leave(3145728ULL);
}
static uint64_t f32(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 4ULL, 5);
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=18446744073709547520ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=18446744073709547520ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v3;o_op(12, 5, _a,_b);});uint64_t _b=v3;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=3ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{
uint64_t v5=0;
uint64_t k1=12ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
uint64_t __attribute__((unused)) v6=({uint64_t p0=v5;f31(p0);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v4;uint64_t _b=v6;o_op(12, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=1ULL;uint64_t _b=v5;o_op(15, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v7;uint64_t _b=v6;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
}else{
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
}
}
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f33(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 6ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 7ULL, 5);
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=({uint64_t p0=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t p0=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=9ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=7ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=o_get(v0, v0_n, 4ULL, 5);o_op(10, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=11ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, 5ULL, 5);
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=8ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=18446744073709551552ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=8ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t p0=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=16ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v7=({uint64_t _a=v7;uint64_t _b=({uint64_t _a=({uint64_t p0=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=40ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=16ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);});
v7=({uint64_t _a=v7;uint64_t _b=({uint64_t p0=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v7=({uint64_t _a=v7;uint64_t _b=({uint64_t _a=({uint64_t p0=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=4ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);});
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=32ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v7=({uint64_t _a=v7;uint64_t _b=({uint64_t _a=({uint64_t p0=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);});
}
if(({uint64_t _a=v7;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=8ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=32ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=10ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f34(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 5ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 6ULL, 5);
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p0=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(8, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v2;uint64_t _b=6ULL;o_op(11, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=9ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=o_get(v0, v0_n, 4ULL, 5);o_op(11, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=11ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=v4;o_op(10, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=11ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 7ULL, 5);uint64_t _b=({uint64_t _a=v4;uint64_t _b=v3;o_op(2, 5, _a,_b);});o_op(10, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=11ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f35(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=4096ULL;o_op(11, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=5ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=o_get(v0, v0_n, 3ULL, 5);o_op(7, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=o_get(v0, v0_n, 6ULL, 5);uint64_t _b=o_get(v0, v0_n, 7ULL, 5);o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=5ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f36(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 4ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 5ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, 6ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
v6=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});o_op(12, 5, _a,_b);});
v6=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=8192ULL;o_op(9, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=16384ULL;o_op(9, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=262144ULL;o_op(9, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});o_op(12, 5, _a,_b);});
{uint64_t _i=0ULL;uint64_t _v=v6;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=15ULL;o_op(1, 5, _a,_b);});uint64_t _b=16ULL;o_op(4, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=15ULL;o_op(1, 5, _a,_b);});uint64_t _b=16ULL;o_op(4, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=16ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=16ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(4, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v6=1ULL;
}
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(4, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v7=1ULL;
}
{uint64_t _i=0ULL;uint64_t _v=v6;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v7;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, 2ULL, 5);
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=({uint64_t p0=({uint64_t _a=v6;uint64_t _b=o_get(v0, v0_n, 3ULL, 5);o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v6;uint64_t _b=o_get(v0, v0_n, 4ULL, 5);o_op(6, 5, _a,_b);});f23(p0);});o_op(13, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f37(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, 4ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, 5ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=({uint64_t p0=({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v4;o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=v6;o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=v4;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=1179010630ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=7ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=369098753ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=1179010630ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v7;uint64_t _b=({uint64_t _a=({uint64_t p0=({uint64_t _a=v5;uint64_t _b=369098754ULL;o_op(6, 5, _a,_b);});f23(p0);});uint64_t _b=({uint64_t p0=({uint64_t _a=v6;uint64_t _b=1179010630ULL;o_op(6, 5, _a,_b);});f23(p0);});o_op(12, 5, _a,_b);});o_op(12, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=7ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=0ULL;uint64_t _v=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=v4;o_op(7, 5, _a,_b);});f23(p0);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f38(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f39(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=o_get(v0, v0_n, 2ULL, 5);o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=o_get(v0, v0_n, 3ULL, 5);o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=o_get(v0, v0_n, 4ULL, 5);o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=o_get(v0, v0_n, 5ULL, 5);o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f40(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=2ULL;uint64_t _b=28ULL;o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v0;uint64_t _b=13ULL;o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v3;uint64_t _b=v4;o_op(13, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v7;uint64_t _b=v5;o_op(13, 5, _a,_b);});
return o_leave(({uint64_t _a=v8;uint64_t _b=v6;o_op(13, 5, _a,_b);}));
}
static uint64_t f41(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=0ULL;uint64_t _v=8ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t p0=1ULL;uint64_t p1=0ULL;uint64_t p2=1ULL;f40(p0, p1, p2);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v0, v0_n, 4ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t p0=0ULL;uint64_t p1=92ULL;uint64_t p2=5ULL;f40(p0, p1, p2);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=32ULL;o_op(16, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=o_get(v0, v0_n, 3ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=1048577ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f42(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({f1(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({f8(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({f10(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
({f15(v0, v0_n, v1, v1_n);});
}else{
({f16(v0, v0_n, v1, v1_n);});
}
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
({f17(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
({f18(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
({f19(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
({f20(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
({f21(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
({f22(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=({uint64_t p0=o_get(v0, v0_n, 1ULL, 5);uint64_t p1=o_get(v0, v0_n, 2ULL, 5);f0(p0, p1);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
({f24(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
({f25(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
({f26(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
({f27(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
({f28(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
({f29(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
({f2(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
({f38(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
({f39(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
({f41(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
({f30(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
({f32(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
({f33(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=24ULL;o_op(6, 5, _a,_b);})){
({f34(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=25ULL;o_op(6, 5, _a,_b);})){
({f35(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=26ULL;o_op(6, 5, _a,_b);})){
({f36(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=27ULL;o_op(6, 5, _a,_b);})){
({f37(v0, v0_n, v1, v1_n);});
return o_leave(0ULL);
}
return o_leave(2ULL);
}
static uint64_t f43(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1000000000ULL;o_op(3, 5, _a,_b);});uint64_t _b=v2;o_op(1, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1000ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=1000000ULL;o_op(4, 5, _a,_b);});o_op(1, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=v2;o_op(1, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=v2;o_op(2, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 4, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=v2;o_op(10, 5, _a,_b);})){
return o_leave(v1);
}
return o_leave(v2);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=255ULL;o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=255ULL;o_op(14, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 4, _a,_b);})){
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(5, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(4096ULL);
}
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(65536ULL);
}
if(({uint64_t _a=v4;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(1048576ULL);
}
if(({uint64_t _a=v4;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(4194304ULL);
}
if(({uint64_t _a=v4;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(16777216ULL);
}
if(({uint64_t _a=v4;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(33554432ULL);
}
return o_leave(67108864ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2000ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});f23(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 4, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));
}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=128ULL;o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(3, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _b=v3;o_op(1, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 4, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=v1;uint64_t _b=v2;o_op(6, 5, _a,_b);});f23(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);});f23(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=1000000ULL;o_op(4, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));
}
return o_leave(18446744073709551615ULL);
}
static uint64_t f44(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(537133066ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(2147483648ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(537198615ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(1, 5, _a,_b);});uint64_t _b=32ULL;o_op(16, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(1179010630ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(1048577ULL);
}
return o_leave(0ULL);
}
static uint64_t f45(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 4, _a,_b);})){
return o_leave(({uint64_t _a=o_fcv(12, 5, v0);uint64_t _b=4742290407621132288ULL;o_fp(4, 12, _a,_b);}));
}
return o_leave(({uint64_t _a=o_fcv(12, 5, v0);uint64_t _b=4697254411347427328ULL;o_fp(4, 12, _a,_b);}));
}
static uint64_t f46(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
{
uint64_t v3=0;
uint64_t k1=512ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(({uint64_t _a=o_get(v0, v0_n, v3, 2);uint64_t _b=0ULL;o_op(6, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=512ULL;uint64_t _b=v3;o_op(2, 5, _a,_b);});o_op(9, 5, _a,_b);})){
uint64_t __attribute__((unused)) v4=1ULL;
{
uint64_t v5=0;
uint64_t c3=0;
for(;o_op(8,5, v5, v2);v5=o_op(1,5, v5,1)){
if(c3++>=1000000)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v3;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);uint64_t _b=o_get(v1, v1_n, v5, 2);o_op(7, 2, _a,_b);})){
v4=0ULL;
}
}
}
if(v4){
return o_leave(1ULL);
}
}
}
}
return o_leave(0ULL);
}
uint32_t omega_fr_text(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, volatile uint64_t p2){
if(!o_buf(result,1,1,5))return 32;
if(!o_buf(p0, p0_n,512, 2))return 32;
if(o_overlap(result,8,p0, p0_n*1))return 32;
if(!o_buf(p1, p1_n,32, 2))return 32;
if(o_overlap(result,8,p1, p1_n*1))return 32;
if(!o_valid(5, p2))return 32;
if(o_busy)return 33;
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f46(p0, p0_n, p1, p1_n, p2);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
