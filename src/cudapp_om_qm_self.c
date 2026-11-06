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
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f2(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f3(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f4(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f5(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f6(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f7(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n);
static uint64_t f8(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n);
static uint64_t f0(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);}));
}
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=14ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=17ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=20ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f2(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=256ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(18446744073709551615ULL);
}
{uint64_t _i=({uint64_t _a=64ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(v2);
}
static uint64_t f3(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
{
uint64_t v2=0;
uint64_t k1=16ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
{uint64_t _i=v2;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
}
}
{uint64_t _i=0ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{
uint64_t v2=0;
uint64_t k1=5ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=18446744073709551615ULL;o_put(v0, v0_n, _i,_v,5);}
}
}
return o_leave(0);
}
static uint64_t f4(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 3ULL, 5);
uint64_t __attribute__((unused)) v5=6ULL;
uint64_t __attribute__((unused)) v6=7ULL;
uint64_t __attribute__((unused)) v7=7ULL;
uint64_t __attribute__((unused)) v8=0ULL;
uint64_t __attribute__((unused)) v9=0ULL;
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v5=14ULL;
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v5=4ULL;
v6=0ULL;
v7=1ULL;
v8=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=3ULL;o_op(13, 5, _a,_b);});
v9=8192ULL;
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v8=3ULL;
}else{
v4=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});
}
}
if(({uint64_t _a=v3;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v5=4ULL;
v6=2ULL;
v8=4ULL;
v9=8192ULL;
}
if(({uint64_t _a=v3;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
v5=4ULL;
v7=1ULL;
v8=2ULL;
v9=8192ULL;
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=7ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v5=5ULL;
if(({uint64_t _a=o_get(v2, v2_n, 9ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v8=v4;
}
}
if(({uint64_t _a=v3;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
v5=o_get(v2, v2_n, 7ULL, 5);
}
{uint64_t _i=7ULL;uint64_t _v=({uint64_t _a=({uint64_t p0=v5;uint64_t p1=v6;uint64_t p2=v7;uint64_t p3=v4;f1(p0, p1, p2, p3);});uint64_t _b=v9;o_op(13, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=v8;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v10=o_get(v1, v1_n, 0ULL, 5);
if(({uint64_t _a=v10;uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
({uint64_t p2=v10;f6(v0, v0_n, v2, v2_n, p2);});
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v10;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f5(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t v3[16]={0};
uint64_t v3_n=16;
o_fill(v3, v3_n, 0ULL, 5);
uint64_t v4[32]={0};
uint64_t v4_n=32;
o_fill(v4, v4_n, 0ULL, 5);
{
uint64_t v5=0;
uint64_t k1=512ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
{uint64_t _i=v5;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
}
}
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, 0ULL, 5);
if(({uint64_t _a=v5;uint64_t _b=2ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=1ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=16ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
}
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=16ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 3ULL, 5);uint64_t _b=15ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=32ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=1024ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
}
{uint64_t _i=2ULL;uint64_t _v=1ULL;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=320ULL;uint64_t _v=2ULL;o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{
uint64_t v6=0;
uint64_t k2=12ULL;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
uint64_t __attribute__((unused)) v7=2ULL;
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v7=1ULL;
}
if(({uint64_t _a=v6;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
v7=4ULL;
}
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=10ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=11ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v7=4ULL;
}
{uint64_t _i=v6;uint64_t _v=({uint64_t p1=v7;f2(v2, v2_n, p1);});o_put(v4, v4_n, _i,_v,5);}
}
}
}else{
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{
uint64_t v6=0;
uint64_t k3=7ULL;
for(;o_op(8,5, v6, k3);v6=o_op(1,5, v6,1)){
uint64_t __attribute__((unused)) v7=1ULL;
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v7=2ULL;
}
if(({uint64_t _a=v6;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
v7=4ULL;
}
if(({uint64_t _a=v6;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
v7=2ULL;
}
{uint64_t _i=v6;uint64_t _v=({uint64_t p1=v7;f2(v2, v2_n, p1);});o_put(v4, v4_n, _i,_v,5);}
}
}
{
uint64_t v6=0;
uint64_t k3=o_get(v0, v0_n, 2ULL, 5);
uint64_t c3=0;
for(;o_op(8,5, v6, k3);v6=o_op(1,5, v6,1)){
if(c3++>=1000000)o_trap(4);
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=8ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=4ULL;f2(v2, v2_n, p1);});o_put(v4, v4_n, _i,_v,5);}
}
}
}else{
{
uint64_t v6=0;
uint64_t k3=7ULL;
for(;o_op(8,5, v6, k3);v6=o_op(1,5, v6,1)){
uint64_t __attribute__((unused)) v7=1ULL;
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=5ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v7=2ULL;
}
{uint64_t _i=v6;uint64_t _v=({uint64_t p1=v7;f2(v2, v2_n, p1);});o_put(v4, v4_n, _i,_v,5);}
}
}
}
}
({uint64_t p1=7ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=856ULL;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=1ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=4ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=5ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 0ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=896ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=2ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
({uint64_t p1=5ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 1ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=904ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=2ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
{
uint64_t v6=0;
uint64_t k1=2ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
({uint64_t p1=3ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, ({uint64_t _a=v6;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 5);o_put(v3, v3_n, _i,_v,5);}
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, ({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);o_put(v3, v3_n, _i,_v,5);}
}
{uint64_t _i=6ULL;uint64_t _v=33ULL;o_put(v3, v3_n, _i,_v,5);}
if(({uint64_t _a=v6;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=6ULL;uint64_t _v=37ULL;o_put(v3, v3_n, _i,_v,5);}
}
({uint64_t p3=2ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
}
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=9ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 6ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 3ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, 0ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=1280ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=9ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 4ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 2ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, 6ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=16ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=9ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 7ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 2ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, 6ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=8ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=9ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 5ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 3ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, 1ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=512ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=9ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 5ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 2ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, 5ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=16ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
{
uint64_t v6=0;
uint64_t k2=3ULL;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
({uint64_t p1=57ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 8ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 4ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v3, v3_n, _i,_v,5);}
if(({uint64_t _a=v6;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 10ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=768ULL;o_put(v3, v3_n, _i,_v,5);}
}
if(({uint64_t _a=v6;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=56ULL;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 9ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 7ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=512ULL;o_put(v3, v3_n, _i,_v,5);}
}
uint64_t __attribute__((unused)) v7=3ULL;
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v7=2ULL;
}
({uint64_t p3=v7;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
}
({uint64_t p1=16ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 11ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 8ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v4, v4_n, 9ULL, 5);o_put(v3, v3_n, _i,_v,5);}
if(({uint64_t _a=o_get(v0, v0_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, 10ULL, 5);o_put(v3, v3_n, _i,_v,5);}
}
{uint64_t _i=6ULL;uint64_t _v=1362114610ULL;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=15ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=11ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
{
uint64_t v6=0;
uint64_t k2=o_get(v0, v0_n, 2ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
if(c2++>=1000000)o_trap(4);
({uint64_t p1=0ULL;f3(v3, v3_n, p1);});
{uint64_t _i=7ULL;uint64_t _v=15ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=11ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
}
{
uint64_t v6=0;
uint64_t k2=4ULL;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
({uint64_t p1=12ULL;f3(v3, v3_n, p1);});
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 5ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v4, v4_n, 11ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=v6;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=v6;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=5ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
}
}
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=2ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 3ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=1ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
{
uint64_t v6=0;
uint64_t k2=6ULL;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
({uint64_t p1=2ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 5ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=v6;o_put(v3, v3_n, _i,_v,5);}
if(({uint64_t _a=v6;uint64_t _b=4ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 6ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({uint64_t _a=v6;uint64_t _b=4ULL;o_op(2, 5, _a,_b);});o_put(v3, v3_n, _i,_v,5);}
}
{uint64_t _i=6ULL;uint64_t _v=943208504ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
}
{
uint64_t v6=0;
uint64_t k2=o_get(v0, v0_n, 2ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
if(c2++>=1000000)o_trap(4);
{
uint64_t v7=0;
uint64_t k3=4ULL;
for(;o_op(8,5, v7, k3);v7=o_op(1,5, v7,1)){
({uint64_t p1=1ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, ({uint64_t _a=v6;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=v7;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
}
}
}
({uint64_t p1=2ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 4ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=o_get(v0, v0_n, 1ULL, 5);o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
{uint64_t _i=16ULL;uint64_t _v=o_get(v2, v2_n, 0ULL, 5);o_put(v2, v2_n, _i,_v,5);}
{
uint64_t v6=0;
uint64_t k2=o_get(v0, v0_n, 2ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
if(c2++>=1000000)o_trap(4);
({uint64_t p1=16ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, ({uint64_t _a=v6;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 5ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v4, v4_n, 6ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, ({uint64_t _a=v6;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=1362114610ULL;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=o_get(v0, v0_n, 3ULL, 5);o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=11ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
}
({uint64_t p1=8ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 4ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 3ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, 4ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=4294967295ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=28ULL;f3(v3, v3_n, p1);});
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 4ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v4, v4_n, 3ULL, 5);o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=1ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=46ULL;f3(v3, v3_n, p1);});
{uint64_t _i=9ULL;uint64_t _v=1ULL;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 16ULL, 5);uint64_t _b=o_get(v2, v2_n, 0ULL, 5);o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=8ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
{uint64_t _i=5ULL;uint64_t _v=1ULL;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=17ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
{
uint64_t v6=0;
uint64_t k2=4ULL;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
({uint64_t p1=0ULL;f3(v3, v3_n, p1);});
{uint64_t _i=7ULL;uint64_t _v=15ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=11ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
}
if(1ULL){
uint64_t __attribute__((unused)) v6=({uint64_t p1=1ULL;f2(v2, v2_n, p1);});
({uint64_t p1=8ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=v6;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 2ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, 1ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=o_get(v0, v0_n, 4ULL, 5);o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=9ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 0ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=v6;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, 0ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 2ULL, 5);uint64_t _b=16ULL;o_op(3, 5, _a,_b);});o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
{
uint64_t v6=0;
uint64_t k2=o_get(v0, v0_n, 2ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
if(c2++>=1000000)o_trap(4);
{
uint64_t v7=0;
uint64_t k3=4ULL;
for(;o_op(8,5, v7, k3);v7=o_op(1,5, v7,1)){
({uint64_t p1=12ULL;f3(v3, v3_n, p1);});
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 0ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v4, v4_n, ({uint64_t _a=v6;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=v7;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t p0=v6;uint64_t p1=v7;f0(p0, p1);});o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=5ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
}
}
}
}
if(({uint64_t _a=v5;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=8ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 4ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 3ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, 2ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=128ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
{
uint64_t v6=0;
uint64_t k2=2ULL;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
({uint64_t p1=9ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, v6, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 4ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=o_get(v4, v4_n, v6, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=8ULL;o_put(v3, v3_n, _i,_v,5);}
if(({uint64_t _a=v6;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=6ULL;uint64_t _v=4ULL;o_put(v3, v3_n, _i,_v,5);}
}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
}
({uint64_t p1=56ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 5ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 0ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=2ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=37ULL;f3(v3, v3_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=o_get(v4, v4_n, 6ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 5ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v4, v4_n, 5ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=1ULL;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=1178940467ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=12ULL;f3(v3, v3_n, p1);});
{uint64_t _i=2ULL;uint64_t _v=o_get(v4, v4_n, 1ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=o_get(v4, v4_n, 6ULL, 5);o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p3=5ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
}
({uint64_t p1=45ULL;f3(v3, v3_n, p1);});
({uint64_t p3=7ULL;f4(v1, v1_n, v2, v2_n, v3, v3_n, p3);});
({uint64_t p1=46ULL;f3(v3, v3_n, p1);});
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 0ULL, 5);
{uint64_t _i=7ULL;uint64_t _v=1032192ULL;o_put(v3, v3_n, _i,_v,5);}
({uint64_t p2=v6;f6(v1, v1_n, v3, v3_n, p2);});
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f6(void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});
{
uint64_t v4=0;
uint64_t k1=6ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
{uint64_t _i=({uint64_t _a=v3;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v1, v1_n, v4, 5);o_put(v0, v0_n, _i,_v,5);}
}
}
{uint64_t _i=({uint64_t _a=v3;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 6ULL, 5);uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 7ULL, 5);uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v4=({uint64_t _a=o_get(v1, v1_n, 8ULL, 5);uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 9ULL, 5);uint64_t _b=1ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 10ULL, 5);uint64_t _b=2ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);});
{
uint64_t v5=0;
uint64_t k1=5ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
v4=({uint64_t _a=v4;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v5;uint64_t _b=11ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=({uint64_t _a=16ULL;uint64_t _b=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
}
}
{uint64_t _i=({uint64_t _a=v3;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=v4;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f7(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n){
o_enter();
uint64_t v6[8]={0};
uint64_t v6_n=8;
o_fill(v6, v6_n, 0ULL, 5);
{uint64_t _i=0ULL;uint64_t _v=v0;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v1;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=v2;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});o_put(v6, v6_n, _i,_v,5);}
return o_leave(({f5(v6, v6_n, v4, v4_n, v5, v5_n);}));
}
static uint64_t f8(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n){
o_enter();
uint64_t v6[16]={0};
uint64_t v6_n=16;
o_fill(v6, v6_n, 0ULL, 5);
{
uint64_t v7=0;
uint64_t k1=512ULL;
for(;o_op(8,5, v7, k1);v7=o_op(1,5, v7,1)){
{uint64_t _i=v7;uint64_t _v=0ULL;o_put(v5, v5_n, _i,_v,5);}
}
}
{uint64_t _i=2ULL;uint64_t _v=1ULL;o_put(v5, v5_n, _i,_v,5);}
{uint64_t _i=320ULL;uint64_t _v=2ULL;o_put(v5, v5_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=({uint64_t p1=2ULL;f2(v5, v5_n, p1);});
v7=({uint64_t p1=4ULL;f2(v5, v5_n, p1);});
v7=({uint64_t p1=2ULL;f2(v5, v5_n, p1);});
v7=({uint64_t p1=4ULL;f2(v5, v5_n, p1);});
({uint64_t p1=7ULL;f3(v6, v6_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=856ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=1ULL;o_put(v6, v6_n, _i,_v,5);}
({uint64_t p3=4ULL;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
({uint64_t p1=5ULL;f3(v6, v6_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=896ULL;o_put(v6, v6_n, _i,_v,5);}
({uint64_t p3=2ULL;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
{
uint64_t v8=0;
uint64_t k1=3ULL;
for(;o_op(8,5, v8, k1);v8=o_op(1,5, v8,1)){
({uint64_t p1=57ULL;f3(v6, v6_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v8;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=0ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=v8;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});o_put(v6, v6_n, _i,_v,5);}
uint64_t __attribute__((unused)) v9=3ULL;
if(({uint64_t _a=v8;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v9=2ULL;
}
if(({uint64_t _a=v8;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=56ULL;o_put(v6, v6_n, _i,_v,5);}
}
({uint64_t p3=v9;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
}
}
({uint64_t p1=16ULL;f3(v6, v6_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=3ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=1ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=2ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=1362114610ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=15ULL;o_put(v6, v6_n, _i,_v,5);}
({uint64_t p3=11ULL;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
({uint64_t p1=0ULL;f3(v6, v6_n, p1);});
{uint64_t _i=7ULL;uint64_t _v=15ULL;o_put(v6, v6_n, _i,_v,5);}
({uint64_t p3=11ULL;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
v7=({uint64_t p1=1ULL;f2(v5, v5_n, p1);});
({uint64_t p1=37ULL;f3(v6, v6_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=v7;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=3ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=1ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=1178940467ULL;o_put(v6, v6_n, _i,_v,5);}
({uint64_t p3=0ULL;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
({uint64_t p1=12ULL;f3(v6, v6_n, p1);});
{uint64_t _i=2ULL;uint64_t _v=0ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=v7;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=64ULL;o_put(v6, v6_n, _i,_v,5);}
({uint64_t p3=5ULL;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
({uint64_t p1=16ULL;f3(v6, v6_n, p1);});
{uint64_t _i=1ULL;uint64_t _v=3ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=1ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=2ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=3ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=15ULL;o_put(v6, v6_n, _i,_v,5);}
({uint64_t p3=11ULL;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
({uint64_t p1=0ULL;f3(v6, v6_n, p1);});
{uint64_t _i=7ULL;uint64_t _v=15ULL;o_put(v6, v6_n, _i,_v,5);}
({uint64_t p3=11ULL;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
{
uint64_t v8=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v8, k1);v8=o_op(1,5, v8,1)){
({uint64_t p1=12ULL;f3(v6, v6_n, p1);});
{uint64_t _i=2ULL;uint64_t _v=0ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=3ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v6, v6_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=v8;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v6, v6_n, _i,_v,5);}
({uint64_t p3=5ULL;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
}
}
({uint64_t p1=45ULL;f3(v6, v6_n, p1);});
({uint64_t p3=7ULL;f4(v4, v4_n, v5, v5_n, v6, v6_n, p3);});
({uint64_t p1=46ULL;f3(v6, v6_n, p1);});
{uint64_t _i=7ULL;uint64_t _v=1032192ULL;o_put(v6, v6_n, _i,_v,5);}
({uint64_t p2=o_get(v5, v5_n, 0ULL, 5);f6(v4, v4_n, v6, v6_n, p2);});
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=o_get(v5, v5_n, 0ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v5, v5_n, _i,_v,5);}
return o_leave(o_get(v5, v5_n, 4ULL, 5));
}
uint32_t omega_qm_self(uint64_t *result, volatile uint64_t p0, volatile uint64_t p1, volatile uint64_t p2, volatile uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n){
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
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f8(p0, p1, p2, p3, p4, p4_n, p5, p5_n);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
