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
static uint64_t f0(uint64_t v0);
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f2(uint64_t v0);
static uint64_t f3(uint64_t v0);
static uint64_t f4(void *v0, uint64_t v0_n);
static uint64_t f5(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f6(void *v0, uint64_t v0_n);
static uint64_t f7(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f8(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f9(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f10(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f11(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f12(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f13(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f14(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f15(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f16(uint64_t v0);
static uint64_t f17(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f18(void *v0, uint64_t v0_n);
static uint64_t f19(void *v0, uint64_t v0_n);
static uint64_t f20(void *v0, uint64_t v0_n);
static uint64_t f21(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f22(uint64_t v0, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f0(uint64_t v0){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=32ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);}));
}
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
if(({uint64_t _a=v0;uint64_t _b=v3;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=v3;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v1;uint64_t _b=10ULL;o_op(5, 5, _a,_b);});o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
}
return o_leave(0ULL);
}
static uint64_t f2(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=2147483648ULL;o_op(11, 5, _a,_b);})){
return o_leave(({uint64_t _a=o_cast(9, 5, v0);uint64_t _b=4294967296ULL;o_op(2, 9, _a,_b);}));
}
return o_leave(o_cast(9, 5, v0));
}
static uint64_t f3(uint64_t v0){
o_enter();
v0=({uint64_t _a=v0;uint64_t _b=4294967296ULL;o_op(5, 9, _a,_b);});
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
v0=({uint64_t _a=v0;uint64_t _b=4294967296ULL;o_op(1, 9, _a,_b);});
}
return o_leave(o_cast(5, 9, v0));
}
static uint64_t f4(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t p0=o_get(v0, v0_n, 0ULL, 5);f2(p0);});
uint64_t __attribute__((unused)) v2=({uint64_t p0=o_get(v0, v0_n, 1ULL, 5);f2(p0);});
uint64_t __attribute__((unused)) v3=({uint64_t p0=o_get(v0, v0_n, 2ULL, 5);f2(p0);});
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(9, 9, _a,_b);})){
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 9, _a,_b);});
}
uint64_t __attribute__((unused)) v4=v1;
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
v4=({uint64_t _a=v1;uint64_t _b=399ULL;o_op(2, 9, _a,_b);});
}
v4=({uint64_t _a=v4;uint64_t _b=400ULL;o_op(4, 9, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v1;uint64_t _b=({uint64_t _a=v4;uint64_t _b=400ULL;o_op(3, 9, _a,_b);});o_op(2, 9, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v2;uint64_t _b=9ULL;o_op(1, 9, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(10, 9, _a,_b);})){
v6=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(2, 9, _a,_b);});
}
uint64_t __attribute__((unused)) v7=({uint64_t p0=v6;f3(p0);});
v7=({uint64_t _a=({uint64_t _a=({uint64_t _a=v7;uint64_t _b=153ULL;o_op(3, 5, _a,_b);});uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
v7=({uint64_t _a=v7;uint64_t _b=5ULL;o_op(4, 5, _a,_b);});
v7=({uint64_t _a=v7;uint64_t _b=({uint64_t p0=v3;f3(p0);});o_op(1, 5, _a,_b);});
v7=({uint64_t _a=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=o_cast(5, 9, v5);uint64_t _b=365ULL;o_op(3, 5, _a,_b);});
v8=({uint64_t _a=v8;uint64_t _b=({uint64_t _a=o_cast(5, 9, v5);uint64_t _b=4ULL;o_op(4, 5, _a,_b);});o_op(1, 5, _a,_b);});
v8=({uint64_t _a=v8;uint64_t _b=({uint64_t _a=o_cast(5, 9, v5);uint64_t _b=100ULL;o_op(4, 5, _a,_b);});o_op(2, 5, _a,_b);});
v8=({uint64_t _a=({uint64_t _a=v8;uint64_t _b=v7;o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=146097ULL;o_op(3, 9, _a,_b);});uint64_t _b=719468ULL;o_op(2, 9, _a,_b);});uint64_t _b=({uint64_t p0=v8;f2(p0);});o_op(1, 9, _a,_b);});
return o_leave(({uint64_t _a=({uint64_t p0=v9;f3(p0);});uint64_t _b=4294967296ULL;o_op(13, 5, _a,_b);}));
}
static uint64_t f5(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=9223372036854775807ULL;
if(({uint64_t _a=o_get(v0, v0_n, 6ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v2=9223372036854775808ULL;
}
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 7ULL, 5);
uint64_t __attribute__((unused)) v4=({uint64_t _a=o_cast(5, 2, v1);uint64_t _b=48ULL;o_op(2, 5, _a,_b);});
if(({uint64_t p0=v3;uint64_t p1=v2;uint64_t p2=v4;f1(p0, p1, p2);})){
return o_leave(v2);
}
return o_leave(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=10ULL;o_op(3, 5, _a,_b);});uint64_t _b=v4;o_op(1, 5, _a,_b);}));
}
static uint64_t f6(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 7ULL, 5);
if(({uint64_t _a=o_get(v0, v0_n, 6ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v1=({uint64_t _a=0ULL;uint64_t _b=v1;o_op(2, 5, _a,_b);});
}
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 4ULL, 5);
{uint64_t _i=({uint64_t _a=8ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v1;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=o_get(v0, v0_n, 8ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=o_get(v0, v0_n, 9ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=o_get(v0, v0_n, 10ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=({f4(v0, v0_n);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(1ULL);
}
{uint64_t _i=4ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=3ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f7(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=256ULL;o_op(10, 5, _a,_b);})){
return o_leave(2ULL);
}
{
uint64_t v3=0;
uint64_t c1=0;
for(;o_op(8,5, v3, v2);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, v3, 2);
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, 5ULL, 5);
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t p0=v4;f0(p0);})){
continue;
}
{uint64_t _i=5ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
v5=1ULL;
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=43ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=v4;uint64_t _b=45ULL;o_op(6, 2, _a,_b);})){
{uint64_t _i=6ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
}
continue;
}
}
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=48ULL;o_op(8, 2, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=57ULL;o_op(10, 2, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=5ULL;uint64_t _v=2ULL;o_put(v0, v0_n, _i,_v,5);}
v5=2ULL;
}
if(({uint64_t _a=v5;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=48ULL;o_op(11, 2, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=57ULL;o_op(9, 2, _a,_b);});o_op(12, 1, _a,_b);})){
{uint64_t _i=7ULL;uint64_t _v=({uint64_t p1=v4;f5(v0, v0_n, p1);});o_put(v0, v0_n, _i,_v,5);}
continue;
}
if(({f6(v0, v0_n);})){
return o_leave(1ULL);
}
v5=3ULL;
}
if(({uint64_t _a=v5;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v4;uint64_t _b=45ULL;o_op(7, 2, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
}
}
}
if(({uint64_t _a=o_get(v0, v0_n, 13ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(({f6(v0, v0_n);})){
return o_leave(1ULL);
}
}
return o_leave(2ULL);
}
return o_leave(0ULL);
}
static uint64_t f8(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
{
uint64_t v2=0;
uint64_t k1=10ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
}
}
{uint64_t _i=13ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=({f7(v0, v0_n, v1, v1_n);});uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(o_get(v0, v0_n, 12ULL, 5));
}
static uint64_t f9(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=79ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=75ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=70ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=78ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=75ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=78ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=79ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=87ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=78ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(15ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=70ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=84ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=65ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=76ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(13ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=70ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=78ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=79ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=67ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=65ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=76ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=73ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=66ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=17ULL;uint64_t _v=65ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=18ULL;uint64_t _v=84ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=19ULL;uint64_t _v=73ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=20ULL;uint64_t _v=79ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=21ULL;uint64_t _v=78ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=22ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(22ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=70ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=67ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=65ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=80ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=65ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=67ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=73ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=84ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=89ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(16ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=70ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=67ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=89ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=67ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=76ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(13ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=70ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=78ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=80ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=80ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=79ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=84ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=17ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=18ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=19ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(19ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=70ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=77ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=79ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=86ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=17ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=18ULL;uint64_t _v=81ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=19ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=20ULL;uint64_t _v=73ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=21ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=22ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=23ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=24ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(24ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=70ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=76ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=73ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=70ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=84ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=73ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=77ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(16ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=80ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=65ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=79ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(11ULL);
}
{uint64_t _i=0ULL;uint64_t _v=63ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(1ULL);
}
static uint64_t f10(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=107ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=119ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(7ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=121ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(9ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=102ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(6ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=109ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(8ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=102ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(9ULL);
}
{uint64_t _i=0ULL;uint64_t _v=63ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(1ULL);
}
static uint64_t f11(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=109ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(10ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(6ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=103ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(13ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=102ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(8ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=109ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=118ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(5ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(6ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=107ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(6ULL);
}
{uint64_t _i=0ULL;uint64_t _v=63ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(1ULL);
}
static uint64_t f12(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=63ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=109ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(7ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=98ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=119ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(9ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=121ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(8ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(6ULL);
}
{uint64_t _i=0ULL;uint64_t _v=63ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(1ULL);
}
static uint64_t f13(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=109ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(7ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=98ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=119ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(9ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(10ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(6ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=122ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(13ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=121ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(4ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(8ULL);
}
{uint64_t _i=0ULL;uint64_t _v=63ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(1ULL);
}
static uint64_t f14(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=78ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=77ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=65ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(10ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=77ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=65ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=85ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=82ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(8ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=89ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=78ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=84ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=72ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=84ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=73ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=67ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=69ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=78ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=83ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=73ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=84ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=73ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=17ULL;uint64_t _v=86ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=18ULL;uint64_t _v=73ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=19ULL;uint64_t _v=84ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=20ULL;uint64_t _v=89ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=21ULL;uint64_t _v=95ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=22ULL;uint64_t _v=79ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=23ULL;uint64_t _v=78ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=24ULL;uint64_t _v=76ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=25ULL;uint64_t _v=89ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=26ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(26ULL);
}
{uint64_t _i=0ULL;uint64_t _v=63ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(1ULL);
}
static uint64_t f15(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=109ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=98ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=17ULL;uint64_t _v=119ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=18ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=19ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=20ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=21ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=22ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=23ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=24ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=25ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=26ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=27ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=28ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=29ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=30ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=31ULL;uint64_t _v=68ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=32ULL;uint64_t _v=65ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=33ULL;uint64_t _v=71ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=34ULL;uint64_t _v=39ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=35ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=36ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=37ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=38ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=39ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=40ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=41ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=42ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=43ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=44ULL;uint64_t _v=40ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=45ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=46ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=47ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=48ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=49ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=50ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=51ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=52ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=53ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=54ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=55ULL;uint64_t _v=102ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=56ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=57ULL;uint64_t _v=103ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=58ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=59ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=60ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=61ULL;uint64_t _v=44ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=62ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=63ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=64ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=65ULL;uint64_t _v=118ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=66ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=67ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=68ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=69ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=70ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=71ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=72ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=73ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=74ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=75ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=76ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=77ULL;uint64_t _v=41ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=78ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(78ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=98ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=119ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=17ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=18ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=19ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=20ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=21ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=22ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=23ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=24ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=25ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=26ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=27ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=28ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=29ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=30ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=31ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=32ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=33ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=34ULL;uint64_t _v=113ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=35ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=36ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=37ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=38ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=39ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=40ULL;uint64_t _v=98ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=41ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=42ULL;uint64_t _v=120ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=43ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=44ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=45ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=46ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=47ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=48ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=49ULL;uint64_t _v=103ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=50ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=51ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=52ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=53ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=54ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=55ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=56ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=57ULL;uint64_t _v=47ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=58ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=59ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=60ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(60ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=109ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=119ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=17ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=18ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=19ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=20ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=21ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=22ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=23ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=24ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=25ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=26ULL;uint64_t _v=118ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=27ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=28ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=29ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=30ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=31ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=32ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=33ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=34ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=35ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=36ULL;uint64_t _v=120ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=37ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=38ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=39ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=40ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=41ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=42ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=43ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=44ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=45ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=46ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=47ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=48ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=49ULL;uint64_t _v=40ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=50ULL;uint64_t _v=77ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=51ULL;uint64_t _v=49ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=52ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=53ULL;uint64_t _v=98ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=54ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=55ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=56ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=57ULL;uint64_t _v=107ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=58ULL;uint64_t _v=50ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=59ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=60ULL;uint64_t _v=102ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=61ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=62ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=63ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=64ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=65ULL;uint64_t _v=119ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=66ULL;uint64_t _v=45ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=67ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=68ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=69ULL;uint64_t _v=41ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=70ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(70ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=109ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=117ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=17ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=18ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=19ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=20ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=21ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=22ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=23ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=24ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=25ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=26ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=27ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=28ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=29ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=30ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=31ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=32ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=33ULL;uint64_t _v=120ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=34ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=35ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=36ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=37ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=38ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=39ULL;uint64_t _v=103ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=40ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=41ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=42ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=43ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=44ULL;uint64_t _v=104ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=45ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=46ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=47ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=48ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(48ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=2ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=3ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=4ULL;uint64_t _v=58ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=5ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=6ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=7ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=8ULL;uint64_t _v=112ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=9ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=10ULL;uint64_t _v=99ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=11ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=12ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=13ULL;uint64_t _v=121ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=15ULL;uint64_t _v=105ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=16ULL;uint64_t _v=115ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=17ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=18ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=19ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=20ULL;uint64_t _v=108ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=21ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=22ULL;uint64_t _v=100ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=23ULL;uint64_t _v=103ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=24ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=25ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=26ULL;uint64_t _v=44ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=27ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=28ULL;uint64_t _v=110ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=29ULL;uint64_t _v=111ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=30ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=31ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=32ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=33ULL;uint64_t _v=32ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=34ULL;uint64_t _v=114ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=35ULL;uint64_t _v=97ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=36ULL;uint64_t _v=116ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=37ULL;uint64_t _v=101ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=38ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(38ULL);
}
{uint64_t _i=0ULL;uint64_t _v=63ULL;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=1ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
return o_leave(1ULL);
}
static uint64_t f16(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=v0;uint64_t _b=9223372036854775807ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v1;uint64_t _b=4831355200913801216ULL;o_op(11, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=v1;uint64_t _b=52ULL;o_op(16, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=1023ULL;o_op(8, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v2;uint64_t _b=1075ULL;o_op(11, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=1075ULL;uint64_t _b=v2;o_op(2, 5, _a,_b);});o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=v3;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
static uint64_t f17(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
return o_leave(o_get(v0, v0_n, v1, 5));
}
static uint64_t f18(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=({uint64_t _a=({uint64_t p1=3ULL;f17(v0, v0_n, p1);});uint64_t _b=({uint64_t p1=0ULL;f17(v0, v0_n, p1);});o_fp(9, 12, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p1=4ULL;f17(v0, v0_n, p1);});uint64_t _b=({uint64_t p1=1ULL;f17(v0, v0_n, p1);});o_fp(9, 12, _a,_b);});o_op(12, 1, _a,_b);});
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=({uint64_t p1=3ULL;f17(v0, v0_n, p1);});uint64_t _b=({uint64_t p1=0ULL;f17(v0, v0_n, p1);});o_fp(8, 12, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p1=4ULL;f17(v0, v0_n, p1);});uint64_t _b=({uint64_t p1=1ULL;f17(v0, v0_n, p1);});o_fp(8, 12, _a,_b);});o_op(13, 1, _a,_b);});
if(({uint64_t _a=o_get(v0, v0_n, 6ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v1=({uint64_t _a=v1;uint64_t _b=({uint64_t _a=({uint64_t p1=5ULL;f17(v0, v0_n, p1);});uint64_t _b=({uint64_t p1=2ULL;f17(v0, v0_n, p1);});o_fp(9, 12, _a,_b);});o_op(12, 1, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=({uint64_t p1=5ULL;f17(v0, v0_n, p1);});uint64_t _b=({uint64_t p1=2ULL;f17(v0, v0_n, p1);});o_fp(8, 12, _a,_b);});o_op(13, 1, _a,_b);});
}
if(({uint64_t _a=v1;uint64_t _b=v2;o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f19(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=0ULL;
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p1=1ULL;f17(v0, v0_n, p1);});uint64_t _b=0ULL;o_fp(10, 12, _a,_b);});o_op(12, 1, _a,_b);})){
v1=({uint64_t _a=({uint64_t _a=({uint64_t p1=3ULL;f17(v0, v0_n, p1);});uint64_t _b=({uint64_t p1=2ULL;f17(v0, v0_n, p1);});o_fp(2, 12, _a,_b);});uint64_t _b=({uint64_t p1=1ULL;f17(v0, v0_n, p1);});o_fp(4, 12, _a,_b);});
}
if(({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v1=4607182418800017408ULL;
}
return o_leave(v1);
}
static uint64_t f20(void *v0, uint64_t v0_n){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=1ULL;f17(v0, v0_n, p1);});uint64_t _b=0ULL;o_fp(11, 12, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v1=({uint64_t p1=2ULL;f17(v0, v0_n, p1);});
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
v1=4587366580439587226ULL;
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t p1=0ULL;f17(v0, v0_n, p1);});uint64_t _b=({uint64_t p1=1ULL;f17(v0, v0_n, p1);});o_fp(2, 12, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=9223372036854775807ULL;o_op(12, 5, _a,_b);});
return o_leave(({uint64_t _a=v2;uint64_t _b=v1;o_fp(3, 12, _a,_b);}));
}
static uint64_t f21(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=32ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t v3[32]={0};
uint64_t v3_n=32;
o_fill(v3, v3_n, 0ULL, 5);
uint64_t v4[32]={0};
uint64_t v4_n=32;
o_fill(v4, v4_n, 0ULL, 5);
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v2);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v6=0ULL;
{
uint64_t v7=0;
uint64_t k2=8ULL;
for(;o_op(8,5, v7, k2);v7=o_op(1,5, v7,1)){
v6=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=o_cast(5, 2, o_get(v1, v1_n, ({uint64_t _a=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=v7;o_op(1, 5, _a,_b);}), 2));uint64_t _b=({uint64_t _a=v7;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
}
}
{uint64_t _i=v5;uint64_t _v=v6;o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=v5;uint64_t _v=v5;o_put(v4, v4_n, _i,_v,5);}
}
}
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v2);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v6=o_get(v4, v4_n, v5, 5);
uint64_t __attribute__((unused)) v7=v5;
{
uint64_t c2=0;
while(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c2++>=32)o_trap(4);
uint64_t __attribute__((unused)) v8=o_get(v4, v4_n, ({uint64_t _a=v7;uint64_t _b=1ULL;o_op(2, 5, _a,_b);}), 5);
if(({uint64_t _a=({uint64_t _a=o_get(v3, v3_n, v8, 5);uint64_t _b=o_get(v3, v3_n, v6, 5);o_fp(8, 12, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
break;
}
{uint64_t _i=v7;uint64_t _v=v8;o_put(v4, v4_n, _i,_v,5);}
v7=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
}
{uint64_t _i=v7;uint64_t _v=v6;o_put(v4, v4_n, _i,_v,5);}
}
}
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v2);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=v5;uint64_t _v=o_cast(2, 5, o_get(v4, v4_n, v5, 5));o_put(v1, v1_n, _i,_v,2);}
}
}
return o_leave(0ULL);
}
static uint64_t f22(uint64_t v0, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f7(v1, v1_n, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);f9(p0, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);f10(p0, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);f11(p0, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);f12(p0, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);f13(p0, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f4(v1, v1_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f8(v1, v1_n, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);f16(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=0ULL;f17(v1, v1_n, p1);});uint64_t _b=0ULL;o_fp(10, 12, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p1=1ULL;f17(v1, v1_n, p1);});uint64_t _b=0ULL;o_fp(10, 12, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=0ULL;f17(v1, v1_n, p1);});uint64_t _b=4517329193108106637ULL;o_fp(1, 12, _a,_b);});uint64_t _b=({uint64_t p1=1ULL;f17(v1, v1_n, p1);});o_fp(11, 12, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t p0=1ULL;f14(p0, v2, v2_n);}));
}
if(({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t p0=2ULL;f14(p0, v2, v2_n);}));
}
return o_leave(({uint64_t p0=0ULL;f14(p0, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=0ULL;f15(p0, v2, v2_n);}));
}
return o_leave(({uint64_t p0=1ULL;f15(p0, v2, v2_n);}));
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=2ULL;f15(p0, v2, v2_n);}));
}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=3ULL;f15(p0, v2, v2_n);}));
}
return o_leave(({uint64_t p0=4ULL;f15(p0, v2, v2_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(o_get(v1, v1_n, 2ULL, 5));
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f18(v1, v1_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f19(v1, v1_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f20(v1, v1_n);}));
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t p1=0ULL;f17(v1, v1_n, p1);});uint64_t _b=({uint64_t p1=1ULL;f17(v1, v1_n, p1);});o_fp(2, 12, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t p1=0ULL;f17(v1, v1_n, p1);});uint64_t _b=4472406533629990549ULL;o_fp(3, 12, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t p1=1ULL;f17(v1, v1_n, p1);});uint64_t _b=({uint64_t p1=0ULL;f17(v1, v1_n, p1);});o_fp(10, 12, _a,_b);})){
return o_leave(o_get(v1, v1_n, 1ULL, 5));
}
return o_leave(o_get(v1, v1_n, 0ULL, 5));
}
if(({uint64_t _a=v0;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f21(v1, v1_n, v2, v2_n);}));
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,2);}
return o_leave(0ULL);
}
uint32_t omega_dag_fmt(uint64_t *result, volatile uint64_t p0, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n){
if(!o_buf(result,1,1,5))return 32;
if(!o_valid(5, p0))return 32;
if(!o_buf(p1, p1_n,16, 5))return 32;
if(o_overlap(result,8,p1, p1_n*8))return 32;
if(!o_buf(p2, p2_n,256, 2))return 32;
if(o_overlap(result,8,p2, p2_n*1))return 32;
if(o_overlap(p2, p2_n*1, p1, p1_n*8))return 32;
if(o_busy)return 33;
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f22(p0, p1, p1_n, p2, p2_n);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
