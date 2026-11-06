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
static uint64_t f1(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f2(uint64_t v0, uint64_t v1);
static uint64_t f3(uint64_t v0);
static uint64_t f4(uint64_t v0, uint64_t v1);
static uint64_t f0(uint64_t v0){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=48ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=57ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=48ULL;o_op(2, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=65ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=70ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=55ULL;o_op(2, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=97ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=102ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=87ULL;o_op(2, 5, _a,_b);}));
}
return o_leave(256ULL);
}
static uint64_t f1(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_cast(5, 2, o_get(v0, v0_n, 64ULL, 2));uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
{
uint64_t v2=0;
uint64_t k1=32ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
uint64_t __attribute__((unused)) v3=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=2ULL;o_op(3, 5, _a,_b);}), 2));
uint64_t __attribute__((unused)) v4=({uint64_t p0=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2));f0(p0);});
if(({uint64_t _a=v4;uint64_t _b=256ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v5=({uint64_t p0=v3;f0(p0);});
if(({uint64_t _a=v5;uint64_t _b=256ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=v2;uint64_t _v=o_cast(2, 5, ({uint64_t _a=({uint64_t _a=v5;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});uint64_t _b=v4;o_op(1, 5, _a,_b);}));o_put(v1, v1_n, _i,_v,2);}
}else{
if(({uint64_t _a=v3;uint64_t _b=45ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=v2;uint64_t _v=o_cast(2, 5, ({uint64_t _a=({uint64_t _a=0ULL;uint64_t _b=v4;o_op(2, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);}));o_put(v1, v1_n, _i,_v,2);}
}else{
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=43ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=9ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=13ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=v2;uint64_t _v=o_cast(2, 5, v4);o_put(v1, v1_n, _i,_v,2);}
}else{
return o_leave(1ULL);
}
}
}
}
}
return o_leave(0ULL);
}
static uint64_t f2(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=1ULL;
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
v2=2ULL;
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(2ULL);
}
}
if(({uint64_t _a=v1;uint64_t _b=v2;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f3(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(1ULL);
}
static uint64_t f4(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(({uint64_t p0=v0;uint64_t p1=v1;f2(p0, p1);}));
}
uint32_t omega_omcheck(uint64_t *result, volatile uint64_t p0, volatile uint64_t p1){
if(!o_buf(result,1,1,5))return 32;
if(!o_valid(5, p0))return 32;
if(!o_valid(5, p1))return 32;
if(o_busy)return 33;
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f4(p0, p1);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
