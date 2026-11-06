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
static uint64_t f1(uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f0(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=v1;
{
uint64_t v4=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=v3;uint64_t _v=o_cast(2, 5, v5);o_put(v0, v0_n, _i,_v,2);}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
}
return o_leave(v3);
}
static uint64_t f1(uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
{
uint64_t v2=0;
uint64_t k1=64ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
{uint64_t _i=v2;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,2);}
}
}
uint64_t __attribute__((unused)) v2=0ULL;
uint64_t __attribute__((unused)) v3=31ULL;
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v3=0ULL;
v2=({uint64_t p1=v2;uint64_t p2=32496501618079059ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v3=1ULL;
v2=({uint64_t p1=v2;uint64_t p2=7019269490406420053ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=431199250796ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v3=2ULL;
v2=({uint64_t p1=v2;uint64_t p2=2334106421097295433ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8389754676633367105ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v3=3ULL;
v2=({uint64_t p1=v2;uint64_t p2=8102104729526759246ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=431198859887ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v3=4ULL;
v2=({uint64_t p1=v2;uint64_t p2=7163369272552681033ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8243082793294128489ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8317708060499077485ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
v3=5ULL;
v2=({uint64_t p1=v2;uint64_t p2=2340011850872286273ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7596553777019711049ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=6579578ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
v3=6ULL;
v2=({uint64_t p1=v2;uint64_t p2=7959390263430115150ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=100ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
v3=7ULL;
v2=({uint64_t p1=v2;uint64_t p2=7163369272552681033ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8820672744039277929ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=101ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
v3=8ULL;
v2=({uint64_t p1=v2;uint64_t p2=7163369272552681033ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8392533911299581289ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8020946586210038373ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7497079ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
v3=9ULL;
v2=({uint64_t p1=v2;uint64_t p2=5629625314317005380ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7306071626148246639ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=100ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
v3=10ULL;
v2=({uint64_t p1=v2;uint64_t p2=32780218601924948ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
v3=11ULL;
v2=({uint64_t p1=v2;uint64_t p2=8103508940177043017ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8315181416901124212ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=28558111363637364ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
v3=12ULL;
v2=({uint64_t p1=v2;uint64_t p2=7018951417441113678ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8242266043723310450ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=2338616487855092321ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=431349919558ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
v3=13ULL;
v2=({uint64_t p1=v2;uint64_t p2=7957695015192261958ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8462059330058800672ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=25710ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
v3=14ULL;
v2=({uint64_t p1=v2;uint64_t p2=7310591744862744387ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=5715753362816311396ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=77ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
v3=15ULL;
v2=({uint64_t p1=v2;uint64_t p2=7791354250702180423ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7631727ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
v3=16ULL;
v2=({uint64_t p1=v2;uint64_t p2=8462656715649011783ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8315177770710037097ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=29797ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
v3=17ULL;
v2=({uint64_t p1=v2;uint64_t p2=7305792289330516039ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7738151096046089075ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7526676582952363109ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8386109761144168549ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8391184066093215337ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=28005ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
v3=18ULL;
v2=({uint64_t p1=v2;uint64_t p2=7795575260105503300ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8511936827898356329ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7863406374178484837ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=29382749079827305ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
v3=19ULL;
v2=({uint64_t p1=v2;uint64_t p2=7070762961532382793ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7307218417350680697ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8389754659302219890ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
v3=20ULL;
v2=({uint64_t p1=v2;uint64_t p2=7163369272552681033ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7882791524569605481ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7959151ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
v3=21ULL;
v2=({uint64_t p1=v2;uint64_t p2=27431034381430606ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
v3=22ULL;
v2=({uint64_t p1=v2;uint64_t p2=4846793629153380214ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8103509687752683040ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=110386908131184ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
v3=23ULL;
v2=({uint64_t p1=v2;uint64_t p2=7163369272552681033ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8315142586355311977ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=126879297336687ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=24ULL;o_op(6, 5, _a,_b);})){
v3=24ULL;
v2=({uint64_t p1=v2;uint64_t p2=2334106421097295433ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7164775612281090630ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=121ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=25ULL;o_op(6, 5, _a,_b);})){
v3=25ULL;
v2=({uint64_t p1=v2;uint64_t p2=8389754676633367105ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7957695011148363296ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7166460029768920352ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=104ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=26ULL;o_op(6, 5, _a,_b);})){
v3=26ULL;
v2=({uint64_t p1=v2;uint64_t p2=7310594974728414546ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7598807797348048996ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=2340029477534396015ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7954875788945482088ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7017564186559734816ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=6579572ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=27ULL;o_op(6, 5, _a,_b);})){
v3=27ULL;
v2=({uint64_t p1=v2;uint64_t p2=7575174855750023507ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7955925901184475251ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8295763985654182432ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=1702125940ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=28ULL;o_op(6, 5, _a,_b);})){
v3=28ULL;
v2=({uint64_t p1=v2;uint64_t p2=8030041785346584398ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=6147491695809623154ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7358993333126373491ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=1684960623ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=29ULL;o_op(6, 5, _a,_b);})){
v3=29ULL;
v2=({uint64_t p1=v2;uint64_t p2=7305808869231650130ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=2336920844765195808ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=2338603393875079011ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8031079655624766579ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7886488383275757600ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8391162085809418784ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7021786302599160933ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=1852795252ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=30ULL;o_op(6, 5, _a,_b);})){
v3=30ULL;
v2=({uint64_t p1=v2;uint64_t p2=8751655660178072914ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8029390865705231728ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=8245933071047794804ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=2338053640979506548ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=7306000154022537575ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=1701013878ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
v2=({uint64_t p1=v2;uint64_t p2=2336936577129475669ULL;f0(v1, v1_n, p1, p2);});
v2=({uint64_t p1=v2;uint64_t p2=491496043077ULL;f0(v1, v1_n, p1, p2);});
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
uint32_t omega_nvml_errstr(uint64_t  *result, volatile uint64_t p0, void *p1, uint64_t p1_n){
  if(!o_buf(result,1,1,5))return 32;
if(!o_valid(5, p0))return 32;
if(!o_buf(p1, p1_n,64, 2))return 32;
  if(o_overlap(result,8,p1, p1_n*1))return 32;
  if(o_busy)return 33;
  o_busy=1;o_depth=0;int code=setjmp(o_jmp);if(!code){uint64_t _result=f1(p0, p1, p1_n);o_put(result,1,0,_result,5);}
  o_busy=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
