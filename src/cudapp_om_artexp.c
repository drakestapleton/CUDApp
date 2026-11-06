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
static uint64_t f0(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f1(uint64_t v0);
static uint64_t f2(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f3(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f4(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f5(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f6(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f7(uint64_t v0);
static uint64_t f8(uint64_t v0);
static uint64_t f9(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f10(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f11(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f12(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f13(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f14(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n);
static uint64_t f15(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f16(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f17(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f18(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f19(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f20(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f21(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f22(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f23(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f24(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f25(void *v0, uint64_t v0_n);
static uint64_t f26(void *v0, uint64_t v0_n);
static uint64_t f27(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n);
static uint64_t f28(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f29(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n);
static uint64_t f30(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f31(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f32(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f33(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n);
static uint64_t f34(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f35(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n);
static uint64_t f0(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
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
static uint64_t f1(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=1ULL;
uint64_t __attribute__((unused)) v2=({uint64_t _a=v0;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(c1++>=20)o_trap(4);
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f2(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
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
static uint64_t f3(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=63ULL;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=v1;uint64_t _v=45ULL;o_put(v0, v0_n, _i,_v,2);}
return o_leave(({uint64_t p1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p2=({uint64_t _a=0ULL;uint64_t _b=v2;o_op(2, 5, _a,_b);});f2(v0, v0_n, p1, p2);}));
}
return o_leave(({uint64_t p1=v1;uint64_t p2=v2;f2(v0, v0_n, p1, p2);}));
}
static uint64_t f4(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=v1;
uint64_t __attribute__((unused)) v5=v3;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=262144ULL;o_op(8, 5, _a,_b);})){
if(c1++>=262144)o_trap(4);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, v5, 2);
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 2, _a,_b);})){
break;
}
{uint64_t _i=v4;uint64_t _v=v6;o_put(v0, v0_n, _i,_v,2);}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v4);
}
static uint64_t f5(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=v1;
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=262144ULL;o_op(8, 5, _a,_b);})){
if(c1++>=262144)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, v2, 2);uint64_t _b=0ULL;o_op(6, 2, _a,_b);})){
return o_leave(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));
}
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v2);
}
static uint64_t f6(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
{
uint64_t v4=0;
uint64_t c1=0;
for(;o_op(8,5, v4, v3);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2));uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
}
}
return o_leave(1ULL);
}
static uint64_t f7(uint64_t v0){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=48ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=57ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=48ULL;o_op(2, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=97ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=102ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=87ULL;o_op(2, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=65ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=70ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=55ULL;o_op(2, 5, _a,_b);}));
}
return o_leave(99ULL);
}
static uint64_t f8(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=32ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=13ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f9(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
if(c1++>=262144)o_trap(4);
if(({uint64_t _a=({uint64_t p0=o_cast(5, 2, o_get(v0, v0_n, v2, 2));f8(p0);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v3=0ULL;
if(({uint64_t _a=v2;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
uint64_t __attribute__((unused)) v4=o_cast(5, 2, o_get(v0, v0_n, v2, 2));
if(({uint64_t _a=v4;uint64_t _b=43ULL;o_op(6, 5, _a,_b);})){
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
if(({uint64_t _a=v4;uint64_t _b=45ULL;o_op(6, 5, _a,_b);})){
v3=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v4=10ULL;
if(({uint64_t _a=v2;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, v2, 2);uint64_t _b=48ULL;o_op(6, 2, _a,_b);})){
v4=8ULL;
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _b=v1;o_op(8, 5, _a,_b);})){
uint64_t __attribute__((unused)) v5=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2));
if(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=120ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=88ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=({uint64_t p0=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2));f7(p0);});uint64_t _b=16ULL;o_op(8, 5, _a,_b);})){
v4=16ULL;
v2=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});
}
}
}
}
}
uint64_t __attribute__((unused)) v5=v2;
uint64_t __attribute__((unused)) v6=0ULL;
uint64_t __attribute__((unused)) v7=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
if(c1++>=262144)o_trap(4);
uint64_t __attribute__((unused)) v8=({uint64_t p0=o_cast(5, 2, o_get(v0, v0_n, v2, 2));f7(p0);});
if(({uint64_t _a=v8;uint64_t _b=v4;o_op(11, 5, _a,_b);})){
break;
}
if(({uint64_t _a=v6;uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=0ULL;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=v8;o_op(2, 5, _a,_b);});uint64_t _b=v4;o_op(4, 5, _a,_b);});o_op(10, 5, _a,_b);})){
v7=1ULL;
}
v6=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=v4;o_op(3, 5, _a,_b);});uint64_t _b=v8;o_op(1, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v5;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=v1;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v6=({uint64_t _a=0ULL;uint64_t _b=v6;o_op(2, 5, _a,_b);});
}
if(({uint64_t _a=v6;uint64_t _b=4294967295ULL;o_op(10, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(v6);
}
static uint64_t f10(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=500186426669ULL;uint64_t p3=5ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=7021802786688019757ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=8ULL;uint64_t p2=431261316195ULL;uint64_t p3=5ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=8388356097741892909ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(3ULL);
}
v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=7154989ULL;uint64_t p3=3ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(4ULL);
}
v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=7023917ULL;uint64_t p3=3ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(5ULL);
}
v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=7220525ULL;uint64_t p3=3ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(6ULL);
}
v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=7233173974647123245ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=8ULL;uint64_t p2=115ULL;uint64_t p3=1ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(7ULL);
}
v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=7018130138796338477ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=8ULL;uint64_t p2=29540ULL;uint64_t p3=2ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(8ULL);
}
return o_leave(0ULL);
}
static uint64_t f11(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=119251603317101ULL;uint64_t p3=6ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=8028075828261975137ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=8ULL;uint64_t p2=110ULL;uint64_t p3=1ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
static uint64_t f12(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=64ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=7162471842286351414ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=8ULL;uint64_t p2=4121692394664059441ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=16ULL;uint64_t p2=7077516104993093940ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=24ULL;uint64_t p2=3906139728515707233ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=32ULL;uint64_t p2=3474018477578727989ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=40ULL;uint64_t p2=7162519142831109729ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=48ULL;uint64_t p2=3616727373085435440ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=56ULL;uint64_t p2=3760566390553469240ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
v2=0ULL;
if(({uint64_t _a=v1;uint64_t _b=64ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=0ULL;uint64_t p2=3775483481970389040ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=8ULL;uint64_t p2=3977302312323527218ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=16ULL;uint64_t p2=7293968145964283956ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=24ULL;uint64_t p2=3906644189691064624ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=32ULL;uint64_t p2=7364007956564697445ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=40ULL;uint64_t p2=7017280228115821107ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=48ULL;uint64_t p2=4063988723899314273ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t p1=56ULL;uint64_t p2=4122257337479160117ULL;uint64_t p3=8ULL;f6(v0, v0_n, p1, p2, p3);});o_op(12, 5, _a,_b);});
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
static uint64_t f13(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
{
uint64_t v2=0;
uint64_t c1=0;
for(;o_op(8,5, v2, v1);v2=o_op(1,5, v2,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v3=o_cast(5, 2, o_get(v0, v0_n, v2, 2));
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=57ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=97ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=102ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
}
}
return o_leave(1ULL);
}
static uint64_t f14(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 4ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 2ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=6ULL;uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=7ULL;uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=v4;uint64_t _b=64ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=8ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
}
{uint64_t _i=9ULL;uint64_t _v=({uint64_t p1=v4;f12(v1, v1_n, p1);});o_put(v0, v0_n, _i,_v,5);}
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=10ULL;uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=v4;uint64_t _b=40ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=11ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
}
{uint64_t _i=12ULL;uint64_t _v=({uint64_t p1=v4;f13(v1, v1_n, p1);});o_put(v0, v0_n, _i,_v,5);}
}
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=({uint64_t _a=9ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=v4;f9(v1, v1_n, p1);});o_put(v0, v0_n, _i,_v,5);}
}
return o_leave(0ULL);
}
static uint64_t f15(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
v1=({uint64_t p1=v1;uint64_t p2=6998658028134298485ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3275352098986357874ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7863412984233621605ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3255293755857400929ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2336010503560437869ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2327833714973548619ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7021802786688019757ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=4992275802573599843ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338623226871881816ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3251696002696303184ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338610025220498221ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=173557064ULL;uint64_t p3=4ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=6998629143283769376ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=3275352098986357874ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=6998721855778486373ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7957695015376221300ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7018130138708389152ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7722878552682558308ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2338323001011350902ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8389765627588190280ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=2334106150831874418ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=8462031896691885384ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=6361693212185206900ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=7596848471616597280ULL;uint64_t p3=8ULL;f0(v0, v0_n, p1, p2, p3);});
v1=({uint64_t p1=v1;uint64_t p2=11374235754612ULL;uint64_t p3=6ULL;f0(v0, v0_n, p1, p2, p3);});
return o_leave(v1);
}
static uint64_t f16(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=99ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=25ULL;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
return o_leave(({uint64_t _a=2ULL;uint64_t _b=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(1, 5, _a,_b);}));
}
static uint64_t f17(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
return o_leave(({uint64_t p1=({uint64_t p1=0ULL;f15(v1, v1_n, p1);});uint64_t p2=2ULL;f16(v0, v0_n, p1, p2);}));
}
static uint64_t f18(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 6ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 7ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 8ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({f17(v0, v0_n, v1, v1_n);}));
}
uint64_t __attribute__((unused)) v2=0ULL;
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 10ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 11ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v2=({uint64_t p1=v2;uint64_t p2=8388356097741892909ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7305437230073736480ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7307140080464847136ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7305808869231632504ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8028074707559739936ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=2670ULL;uint64_t p3=2ULL;f0(v1, v1_n, p1, p2, p3);});
return o_leave(({uint64_t p1=({uint64_t p1=v2;f15(v1, v1_n, p1);});uint64_t p2=2ULL;f16(v0, v0_n, p1, p2);}));
}
if(({uint64_t _a=o_get(v0, v0_n, 12ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v2=({uint64_t p1=v2;uint64_t p2=8388356097741892909ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7305437230073736480ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7017578424494418976ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7307140080464848243ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=2680ULL;uint64_t p3=2ULL;f0(v1, v1_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v2;uint64_t p2=2ULL;f16(v0, v0_n, p1, p2);}));
}
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 5ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 9ULL, 5);uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
v2=({uint64_t p1=v2;uint64_t p2=8386654075301556067ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7935469438779025696ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8461547236690523247ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8515522056226483564ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=2609ULL;uint64_t p3=2ULL;f0(v1, v1_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v2;uint64_t p2=3ULL;f16(v0, v0_n, p1, p2);}));
}
uint64_t __attribute__((unused)) v4=0ULL;
if(({uint64_t _a=o_get(v0, v0_n, 13ULL, 5);uint64_t _b=8192ULL;o_op(10, 5, _a,_b);})){
v4=1ULL;
}
if(({uint64_t _a=o_get(v0, v0_n, 14ULL, 5);uint64_t _b=16384ULL;o_op(10, 5, _a,_b);})){
v4=1ULL;
}
if(({uint64_t _a=o_get(v0, v0_n, 15ULL, 5);uint64_t _b=8192ULL;o_op(10, 5, _a,_b);})){
v4=1ULL;
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v2=({uint64_t p1=v2;uint64_t p2=8462017844901668979ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7020303013988889460ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3559640502715641204ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8462090115260755254ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=4412521754026796142ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=4412519572481585464ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=4354466492506650161ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=2860045451737149ULL;uint64_t p3=7ULL;f0(v1, v1_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v2;uint64_t p2=3ULL;f16(v0, v0_n, p1, p2);}));
}
}
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 9ULL, 5);uint64_t _b=2ULL;o_op(7, 5, _a,_b);})){
v2=({uint64_t p1=v2;uint64_t p2=8386654075301556067ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7935469438779025696ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7146475323396224111ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3564086098625000801ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=10ULL;uint64_t p3=1ULL;f0(v1, v1_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v2;uint64_t p2=3ULL;f16(v0, v0_n, p1, p2);}));
}
uint64_t __attribute__((unused)) v4=0ULL;
if(({uint64_t _a=o_get(v0, v0_n, 16ULL, 5);uint64_t _b=64ULL;o_op(10, 5, _a,_b);})){
v4=1ULL;
}
if(({uint64_t _a=o_get(v0, v0_n, 17ULL, 5);uint64_t _b=64ULL;o_op(10, 5, _a,_b);})){
v4=1ULL;
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v2=({uint64_t p1=v2;uint64_t p2=8462017904829883752ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8171535668735472500ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7809650176819408225ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7959390383684810285ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=4412526427079996260ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=4412531938597549110ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=170472502ULL;uint64_t p3=4ULL;f0(v1, v1_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v2;uint64_t p2=3ULL;f16(v0, v0_n, p1, p2);}));
}
}
return o_leave(0ULL);
}
static uint64_t f19(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 13ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 14ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 15ULL, 5);
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({f17(v0, v0_n, v1, v1_n);}));
}
uint64_t __attribute__((unused)) v5=0ULL;
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v5=1ULL;
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v5=1ULL;
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v5=1ULL;
}
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=0ULL;
v6=({uint64_t p1=v6;uint64_t p2=7074429841272234349ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=7286878580719890790ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=7308814929777094776ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=3904943502533026917ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=7935391128637750560ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=3184080383941556261ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=4412718627017812768ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=2338616595357642784ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=v2;f2(v1, v1_n, p1, p2);});
{uint64_t _i=v6;uint64_t _v=120ULL;o_put(v1, v1_n, _i,_v,2);}
v6=({uint64_t p1=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p2=v3;f2(v1, v1_n, p1, p2);});
{uint64_t _i=v6;uint64_t _v=120ULL;o_put(v1, v1_n, _i,_v,2);}
v6=({uint64_t p1=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p2=v4;f2(v1, v1_n, p1, p2);});
v6=({uint64_t p1=v6;uint64_t p2=2601ULL;uint64_t p3=2ULL;f0(v1, v1_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v6;uint64_t p2=3ULL;f16(v0, v0_n, p1, p2);}));
}
{uint64_t _i=0ULL;uint64_t _v=5ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(3ULL);
}
static uint64_t f20(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 16ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 17ULL, 5);
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({f17(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v3;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({f17(v0, v0_n, v1, v1_n);}));
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=v3;o_op(4, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v5;o_op(15, 5, _a,_b);});uint64_t _b=v4;o_op(8, 5, _a,_b);})){
if(c1++>=64)o_trap(4);
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v5;o_op(15, 5, _a,_b);});uint64_t _b=v4;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=0ULL;
v6=({uint64_t p1=v6;uint64_t p2=7598805614076186951ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=8390045716468801647ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=8243126013477806368ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=751951003437657888ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v6;uint64_t p2=3ULL;f16(v0, v0_n, p1, p2);}));
}
{uint64_t _i=20ULL;uint64_t _v=v5;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=8ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(8ULL);
}
static uint64_t f21(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=({f18(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v2);
}
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 5ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f19(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f20(v0, v0_n, v1, v1_n);}));
}
return o_leave(({f17(v0, v0_n, v1, v1_n);}));
}
static uint64_t f22(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p1=0ULL;uint64_t p2=v2;uint64_t p3=v3;f0(v1, v1_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v4;uint64_t p2=3ULL;f16(v0, v0_n, p1, p2);}));
}
static uint64_t f23(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
{uint64_t _i=({uint64_t _a=64ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f24(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
{uint64_t _i=32ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=33ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=34ULL;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=35ULL;uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=36ULL;uint64_t _v=v4;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=37ULL;uint64_t _v=v5;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=40ULL;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=41ULL;uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=42ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=43ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=44ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=45ULL;uint64_t _v=v5;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=10ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f25(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 13ULL, 5);
uint64_t __attribute__((unused)) v2=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(4, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=o_get(v0, v0_n, 15ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 19ULL, 5);
uint64_t __attribute__((unused)) v5=({uint64_t p1=32ULL;uint64_t p2=v4;uint64_t p3=v2;uint64_t p4=v3;uint64_t p5=1024ULL;f24(v0, v0_n, p1, p2, p3, p4, p5);});
{uint64_t _i=46ULL;uint64_t _v=10ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=47ULL;uint64_t _v=3ULL;o_put(v0, v0_n, _i,_v,5);}
{
uint64_t v6=0;
uint64_t k1=3ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
v5=({uint64_t p1=v6;uint64_t p2=({uint64_t _a=896ULL;uint64_t _b=({uint64_t _a=v6;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t p3=8ULL;uint64_t p4=1ULL;f23(v0, v0_n, p1, p2, p3, p4);});
}
}
return o_leave(7ULL);
}
static uint64_t f26(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 16ULL, 5);
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 21ULL, 5);
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=3ULL;uint64_t _b=v2;o_op(3, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v3=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=512ULL;o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=v2;o_op(4, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=128ULL;o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p1=128ULL;uint64_t p2=v4;uint64_t p3=1ULL;uint64_t p4=v5;uint64_t p5=v3;f24(v0, v0_n, p1, p2, p3, p4, p5);});
{uint64_t _i=46ULL;uint64_t _v=16ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=47ULL;uint64_t _v=12ULL;o_put(v0, v0_n, _i,_v,5);}
{
uint64_t v7=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v7, k1);v7=o_op(1,5, v7,1)){
v6=({uint64_t p1=v7;uint64_t p2=({uint64_t _a=896ULL;uint64_t _b=({uint64_t _a=v7;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t p3=8ULL;uint64_t p4=1ULL;f23(v0, v0_n, p1, p2, p3, p4);});
}
}
{
uint64_t v7=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v7, k1);v7=o_op(1,5, v7,1)){
v6=({uint64_t p1=({uint64_t _a=4ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});uint64_t p2=({uint64_t _a=928ULL;uint64_t _b=({uint64_t _a=v7;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t p3=4ULL;uint64_t p4=0ULL;f23(v0, v0_n, p1, p2, p3, p4);});
}
}
return o_leave(11ULL);
}
static uint64_t f27(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n){
o_enter();
{
uint64_t v2=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t v4=0;
uint64_t k2=8ULL;
for(;o_op(8,5, v4, k2);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=o_cast(5, 2, o_get(v1, v1_n, ({uint64_t _a=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2));
v3=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
}
}
{uint64_t _i=({uint64_t _a=80ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
}
}
return o_leave(0ULL);
}
static uint64_t f28(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
v2=({uint64_t p1=v2;uint64_t p2=8386033877510352239ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3273676477860570484ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7496858609312428147ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=4426309888593125733ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7307195334516027953ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=1030972513ULL;uint64_t p3=4ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 16ULL, 5);f2(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7233173975486327562ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=15731ULL;uint64_t p3=2ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 17ULL, 5);f2(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8171604718975413258ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=15713ULL;uint64_t p3=2ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 20ULL, 5);f2(v1, v1_n, p1, p2);});
{uint64_t _i=v2;uint64_t _v=10ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=0ULL;uint64_t _v=9ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(({uint64_t _a=9ULL;uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(1, 5, _a,_b);}));
}
static uint64_t f29(void *v0, uint64_t v0_n, uint64_t v1, const void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=v1;
{
uint64_t v4=0;
uint64_t k1=32ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, ({uint64_t _a=80ULL;uint64_t _b=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(4, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(5, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});
{
uint64_t v7=0;
uint64_t k2=2ULL;
for(;o_op(8,5, v7, k2);v7=o_op(1,5, v7,1)){
uint64_t __attribute__((unused)) v8=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v7;o_op(2, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});
v8=({uint64_t _a=v8;uint64_t _b=15ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v8;uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
{uint64_t _i=v3;uint64_t _v=o_cast(2, 5, ({uint64_t _a=48ULL;uint64_t _b=v8;o_op(1, 5, _a,_b);}));o_put(v0, v0_n, _i,_v,2);}
}else{
{uint64_t _i=v3;uint64_t _v=o_cast(2, 5, ({uint64_t _a=87ULL;uint64_t _b=v8;o_op(1, 5, _a,_b);}));o_put(v0, v0_n, _i,_v,2);}
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
}
}
{uint64_t _i=v3;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,2);}
return o_leave(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));
}
static uint64_t f30(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
v2=({uint64_t p1=v2;uint64_t p2=7854397001737462125ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7377509083436119137ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7377459352022365745ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=14130306676176433ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3271074843431494241ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3184370909023382069ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=2983183344158859873ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=1030553641ULL;uint64_t p3=4ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 13ULL, 5);f2(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=4025132ULL;uint64_t p3=3ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 14ULL, 5);f2(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=4025900ULL;uint64_t p3=3ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 15ULL, 5);f2(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=3618977893184204032ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=4049640075044875052ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=4121136969678728244ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=50ULL;uint64_t p3=2ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;f29(v1, v1_n, p1, v0, v0_n);});
v2=({uint64_t p1=v2;uint64_t p2=7020372266091638127ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3559640717464006004ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=54ULL;uint64_t p3=2ULL;f0(v1, v1_n, p1, p2, p3);});
return o_leave(v2);
}
static uint64_t f31(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
v2=({uint64_t p1=v2;uint64_t p2=8319662965227876711ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7954892360015244385ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7021506545597770100ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3273098177709957933ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7722884132346278262ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8214620916834238582ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3687714932041477221ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7291676601532703020ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3184393749937533997ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7506427719172972898ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=15729ULL;uint64_t p3=2ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 16ULL, 5);f2(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=263979755564ULL;uint64_t p3=5ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=o_get(v0, v0_n, 17ULL, 5);f2(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=15817785760573484ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7506432086480023912ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7506432086480025195ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8030529705426894180ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=4352011477019489143ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=232887694397ULL;uint64_t p3=6ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;f29(v1, v1_n, p1, v0, v0_n);});
v2=({uint64_t p1=v2;uint64_t p2=8171604920838221167ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8389754706680962913ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8319662966067588969ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=27745ULL;uint64_t p3=3ULL;f0(v1, v1_n, p1, p2, p3);});
return o_leave(v2);
}
static uint64_t f32(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
if(({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v2=({f30(v0, v0_n, v1, v1_n);});
}else{
v2=({f31(v0, v0_n, v1, v1_n);});
}
v2=({uint64_t p1=v2;uint64_t p2=3329065233924765749ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3616776899440685616ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7019251898354904113ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7002374394533471021ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8458604767632779618ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8390875903532491108ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8675390235105912425ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=54079886880624ULL;uint64_t p3=7ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3271142103423805039ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7882833662175308132ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=6004214541099560034ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=3271130008963605563ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=6075160447930167148ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=8531335275003335263ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=7162263158351291241ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=5644508277368639037ULL;uint64_t p3=8ULL;f0(v1, v1_n, p1, p2, p3);});
v2=({uint64_t p1=v2;uint64_t p2=0ULL;uint64_t p3=1ULL;f0(v1, v1_n, p1, p2, p3);});
{uint64_t _i=0ULL;uint64_t _v=14ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(14ULL);
}
static uint64_t f33(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n){
o_enter();
uint64_t __attribute__((unused)) v4=0ULL;
if(({uint64_t _a=o_get(v0, v0_n, 18ULL, 5);uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(7, 5, _a,_b);})){
v4=({uint64_t p1=v4;uint64_t p2=8386654023577793121ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p2=2338619929466004781ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p2=4207599498436175218ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p2=32ULL;uint64_t p3=1ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p3=0ULL;f4(v3, v3_n, p1, v2, v2_n, p3);});
v4=({uint64_t p1=v4;uint64_t p2=8250ULL;uint64_t p3=2ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p3=({uint64_t p1=0ULL;f5(v2, v2_n, p1);});f4(v3, v3_n, p1, v2, v2_n, p3);});
{uint64_t _i=v4;uint64_t _v=10ULL;o_put(v3, v3_n, _i,_v,2);}
return o_leave(({uint64_t p1=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p2=3ULL;f16(v0, v0_n, p1, p2);}));
}
uint64_t __attribute__((unused)) v5=0ULL;
v4=({uint64_t p1=v4;uint64_t p2=8386654023577793121ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p2=1029990751ULL;uint64_t p3=4ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p3=v5;f4(v3, v3_n, p1, v2, v2_n, p3);});
v5=({uint64_t p1=v5;f5(v2, v2_n, p1);});
v4=({uint64_t p1=v4;uint64_t p2=1030778634ULL;uint64_t p3=4ULL;f0(v3, v3_n, p1, p2, p3);});
if(({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t p1=v4;uint64_t p2=119251603317101ULL;uint64_t p3=6ULL;f0(v3, v3_n, p1, p2, p3);});
}else{
v4=({uint64_t p1=v4;uint64_t p2=8319662965227876711ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p2=7954892360015244385ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p2=1852795252ULL;uint64_t p3=4ULL;f0(v3, v3_n, p1, p2, p3);});
}
v4=({uint64_t p1=v4;uint64_t p2=7161130726890365706ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p2=263677632372ULL;uint64_t p3=5ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p3=v5;f4(v3, v3_n, p1, v2, v2_n, p3);});
v5=({uint64_t p1=v5;f5(v2, v2_n, p1);});
v4=({uint64_t p1=v4;uint64_t p2=17291372481831690ULL;uint64_t p3=7ULL;f0(v3, v3_n, p1, p2, p3);});
v4=({uint64_t p1=v4;uint64_t p3=v5;f4(v3, v3_n, p1, v2, v2_n, p3);});
v4=({uint64_t p1=v4;uint64_t p2=2936148669588270ULL;uint64_t p3=7ULL;f0(v3, v3_n, p1, p2, p3);});
{uint64_t _i=0ULL;uint64_t _v=99ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(({uint64_t _a=15ULL;uint64_t _b=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(1, 5, _a,_b);}));
}
static uint64_t f34(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, 3ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=2ULL;o_op(8, 5, _a,_b);})){
return o_leave(({f17(v0, v0_n, v2, v2_n);}));
}
{uint64_t _i=1ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=5ULL;uint64_t _v=({uint64_t p1=o_get(v0, v0_n, 2ULL, 5);f11(v1, v1_n, p1);});o_put(v0, v0_n, _i,_v,5);}
v4=2ULL;
}
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t p1=o_get(v0, v0_n, 2ULL, 5);f10(v1, v1_n, p1);});
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=v5;o_op(11, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({f17(v0, v0_n, v2, v2_n);}));
}
{uint64_t _i=4ULL;uint64_t _v=v6;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=3ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({f14(v0, v0_n, v1, v1_n);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
{uint64_t _i=1ULL;uint64_t _v=v4;o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=v4;uint64_t _b=v5;o_op(8, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=2ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(1ULL);
}
return o_leave(({f21(v0, v0_n, v2, v2_n);}));
}
static uint64_t f35(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 0ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, 18ULL, 5);
uint64_t __attribute__((unused)) v6=0ULL;
if(({uint64_t _a=v4;uint64_t _b=4ULL;o_op(8, 5, _a,_b);})){
return o_leave(({f34(v1, v1_n, v2, v2_n, v3, v3_n);}));
}
if(({uint64_t _a=v4;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=7378429047948537971ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=44634043253ULL;uint64_t p3=5ULL;f0(v3, v3_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v6;uint64_t p2=3ULL;f16(v1, v1_n, p1, p2);}));
}
uint64_t __attribute__((unused)) v7=o_get(v1, v1_n, 13ULL, 5);
uint64_t __attribute__((unused)) v8=o_get(v1, v1_n, 15ULL, 5);
uint64_t __attribute__((unused)) v9=({uint64_t _a=v8;uint64_t _b=8ULL;o_op(4, 5, _a,_b);});
{uint64_t _i=0ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 14ULL, 5);uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=19ULL;uint64_t _v=v9;o_put(v1, v1_n, _i,_v,5);}
return o_leave(4ULL);
}
uint64_t __attribute__((unused)) v10=({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=({uint64_t _a=v7;uint64_t _b=16ULL;o_op(4, 5, _a,_b);});o_op(4, 5, _a,_b);});
if(({uint64_t _a=v10;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v10=1ULL;
}
if(({uint64_t _a=v9;uint64_t _b=v10;o_op(10, 5, _a,_b);})){
v9=v10;
}
{uint64_t _i=19ULL;uint64_t _v=v9;o_put(v1, v1_n, _i,_v,5);}
return o_leave(5ULL);
}
if(({uint64_t _a=v4;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=2336916751408983907ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=748834984615634290ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v6;uint64_t p2=3ULL;f16(v1, v1_n, p1, p2);}));
}
{uint64_t _i=0ULL;uint64_t _v=7ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(6ULL);
}
if(({uint64_t _a=v4;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({f27(v1, v1_n, v2, v2_n);});
return o_leave(({f25(v1, v1_n);}));
}
if(({uint64_t _a=v4;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=8028075828261975137ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=7306920402471821422ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=7310315702801539182ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=2112100ULL;uint64_t p3=3ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=v5;f3(v3, v3_n, p1, p2);});
{uint64_t _i=v6;uint64_t _v=10ULL;o_put(v3, v3_n, _i,_v,2);}
return o_leave(({uint64_t p1=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p2=3ULL;f16(v1, v1_n, p1, p2);}));
}
return o_leave(({f28(v1, v1_n, v3, v3_n);}));
}
if(({uint64_t _a=v4;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({f27(v1, v1_n, v2, v2_n);});
{uint64_t _i=0ULL;uint64_t _v=12ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(10ULL);
}
if(({uint64_t _a=v4;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f26(v1, v1_n);}));
}
if(({uint64_t _a=v4;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=11ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(12ULL);
}
if(({uint64_t _a=v4;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=38ULL;uint64_t _v=o_get(v1, v1_n, 23ULL, 5);o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 24ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=7234314139196417380ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=7310594991672619552ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=8319668472529318770ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=681061ULL;uint64_t p3=3ULL;f0(v3, v3_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v6;uint64_t p2=3ULL;f16(v1, v1_n, p1, p2);}));
}
{uint64_t _i=0ULL;uint64_t _v=13ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(13ULL);
}
if(({uint64_t _a=v4;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v6=({uint64_t p1=v6;uint64_t p2=8101242854141349201ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=7378429047983071596ULL;uint64_t p3=8ULL;f0(v3, v3_n, p1, p2, p3);});
v6=({uint64_t p1=v6;uint64_t p2=44634043253ULL;uint64_t p3=5ULL;f0(v3, v3_n, p1, p2, p3);});
return o_leave(({uint64_t p1=v6;uint64_t p2=3ULL;f16(v1, v1_n, p1, p2);}));
}
return o_leave(({f32(v1, v1_n, v3, v3_n);}));
}
if(({uint64_t _a=v4;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f33(v1, v1_n, v0, v0_n, v2, v2_n, v3, v3_n);}));
}
return o_leave(0ULL);
}
uint32_t omega_artexp(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n){
if(!o_buf(result,1,1,5))return 32;
if(!o_buf(p0, p0_n,8, 5))return 32;
if(o_overlap(result,8,p0, p0_n*8))return 32;
if(!o_buf(p1, p1_n,128, 5))return 32;
if(o_overlap(result,8,p1, p1_n*8))return 32;
if(o_overlap(p1, p1_n*8, p0, p0_n*8))return 32;
if(!o_buf(p2, p2_n,262144, 2))return 32;
if(o_overlap(result,8,p2, p2_n*1))return 32;
if(o_overlap(p2, p2_n*1, p1, p1_n*8))return 32;
if(!o_buf(p3, p3_n,262144, 2))return 32;
if(o_overlap(result,8,p3, p3_n*1))return 32;
if(o_overlap(p3, p3_n*1, p0, p0_n*8))return 32;
if(o_overlap(p3, p3_n*1, p1, p1_n*8))return 32;
if(o_overlap(p3, p3_n*1, p2, p2_n*1))return 32;
if(o_busy)return 33;
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f35(p0, p0_n, p1, p1_n, p2, p2_n, p3, p3_n);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
