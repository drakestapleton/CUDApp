#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif
/* Omega Bridge freestanding checked runtime views. GNU C11; no hosted imports. */
#include <stdint.h>
#include <stddef.h>
struct ombr_fs_ctx { unsigned code, depth; };
static uint64_t fs_error(struct ombr_fs_ctx *fs,unsigned code){fs->code=code;return 0;}
static uint64_t fs_leave(struct ombr_fs_ctx *fs,uint64_t v){--fs->depth;return v;}
static unsigned o_width(unsigned t){static const unsigned w[]={0,1,8,16,32,64,8,16,32,64};return w[t];}
static int o_valid(unsigned t,uint64_t v){unsigned w=o_width(t);if(t<6)return w==64||v<((uint64_t)1<<w);if(w==64)return 1;uint64_t m=(uint64_t)1<<(w-1);return v<m||v>=UINT64_MAX-m+1;}
static uint64_t fs_cast(struct ombr_fs_ctx *fs,unsigned t,unsigned s,uint64_t a){if((s>=6&&a>>63&&t<6)||(s<6&&t>=6&&a>>63)||!o_valid(t,a))return fs_error(fs,5);return a;}
static uint64_t fs_op(struct ombr_fs_ctx *fs,unsigned op,unsigned t,uint64_t a,uint64_t b){
unsigned w=o_width(t);uint64_t mask=UINT64_MAX>>(64-w),r=0;
if(op>=6&&op<=11){uint64_t x=t>=6?a^((uint64_t)1<<63):a,y=t>=6?b^((uint64_t)1<<63):b;switch(op){case 6:return x==y;case 7:return x!=y;case 8:return x<y;case 9:return x<=y;case 10:return x>y;default:return x>=y;}}
if(op==12)return a&b;if(op==13)return a|b;if(op==14)return a^b;
if(op==18)return t==1?!a:(t>=6?~a:(~a)&mask);
if(op==15||op==16){if(b>=w)return fs_error(fs,7);if(op==16){if(t<6||!b)return a>>b;return (a>>b)|((a>>63)?(UINT64_MAX<<(64-b)):0);}if(t<6)return (a<<b)&mask;uint64_t m=(uint64_t)1<<(w-1),x=a>>63?0-a:a,lim=a>>63?m:m-1;if(x>(lim>>b))return fs_error(fs,1);return a<<b;}
if(op==4||op==5){if(!b)return fs_error(fs,2);if(t<6)return op==4?a/b:a%b;uint64_t x=a>>63?0-a:a,y=b>>63?0-b:b;int neg=op==4?(int)((a^b)>>63):(int)(a>>63);r=op==4?x/y:x%y;if(neg)r=0-r;if(!o_valid(t,r)||(!neg&&r>>63))return fs_error(fs,1);return r;}
if(t<6){if(op==17)return (0-a)&mask;if(op==1)return (a+b)&mask;if(op==2)return (a-b)&mask;return (a*b)&mask;}
if(op==17){r=0-a;if(a&&r==a)return fs_error(fs,1);}
if(op==1){r=a+b;if((~(a^b)&(a^r))>>63)return fs_error(fs,1);}
if(op==2){r=a-b;if(((a^b)&(a^r))>>63)return fs_error(fs,1);}
if(op==3){uint64_t x=a>>63?0-a:a,y=b>>63?0-b:b,m=(uint64_t)1<<(w-1),lim=(a^b)>>63?m:m-1;if(y&&x>lim/y)return fs_error(fs,1);r=a*b;}
if(!o_valid(t,r))return fs_error(fs,1);return r;
}
static uint64_t fs_get(struct ombr_fs_ctx *fs,const void *p,uint64_t n,uint64_t i,unsigned t){if(i>=n)return fs_error(fs,3);unsigned w=t==11?32:t==12?64:o_width(t);const uint8_t *b=(const uint8_t*)p+i*(w/8);uint64_t v=0;if(w==8)v=b[0];else if(w==16){uint16_t x;__builtin_memcpy(&x,b,2);v=x;}else if(w==32){uint32_t x;__builtin_memcpy(&x,b,4);v=x;}else if(w==64)__builtin_memcpy(&v,b,8);if(t>=6&&t<=9&&w<64&&(v>>(w-1)))v|=UINT64_MAX<<w;return v;}
static void fs_put(struct ombr_fs_ctx *fs,void *p,uint64_t n,uint64_t i,uint64_t v,unsigned t){if(i>=n){fs_error(fs,3);return;}unsigned w=t==11?32:t==12?64:o_width(t);if(t<11&&!o_valid(t,v)){fs_error(fs,5);return;}uint8_t *b=(uint8_t*)p+i*(w/8);if(w==8)b[0]=(uint8_t)v;else if(w==16){uint16_t x=(uint16_t)v;__builtin_memcpy(b,&x,2);}else if(w==32){uint32_t x=(uint32_t)v;__builtin_memcpy(b,&x,4);}else if(w==64)__builtin_memcpy(b,&v,8);}
static void fs_fill(struct ombr_fs_ctx *fs,void *p,uint64_t n,uint64_t v,unsigned t){for(uint64_t i=0;i<n;i++){fs_put(fs,p,n,i,v,t);if(fs->code)return;}}
static int o_buf(const void *p,uint64_t n,uint64_t want,unsigned t){uintptr_t a=(uintptr_t)p;uint64_t z=t==11?4:t==12?8:o_width(t)/8;return p&&n==want&&a%z==0&&n<=UINTPTR_MAX/z&&a<=UINTPTR_MAX-n*z;}
static int o_overlap(const void *a,uint64_t an,const void *b,uint64_t bn){uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x<y+bn&&y<x+an;}
#define o_cast(t,s,a) ({uint64_t fs_r=fs_cast(fs,t,s,a);if(fs->code)goto fs_fail;fs_r;})
#define o_op(op,t,a,b) ({uint64_t fs_r=fs_op(fs,op,t,a,b);if(fs->code)goto fs_fail;fs_r;})
#define o_get(p,n,i,t) ({uint64_t fs_r=fs_get(fs,p,n,i,t);if(fs->code)goto fs_fail;fs_r;})
#define o_put(p,n,i,v,t) do{fs_put(fs,p,n,i,v,t);if(fs->code)goto fs_fail;}while(0)
#define o_fill(p,n,v,t) do{fs_fill(fs,p,n,v,t);if(fs->code)goto fs_fail;}while(0)
#define o_leave(v) fs_leave(fs,(v))
#define o_enter() do{if(fs->depth>=128){fs->code=6;return 0;}++fs->depth;}while(0)
#define o_trap(c) do{fs->code=(c);goto fs_fail;}while(0)
static uint64_t o_arg32(float v){uint32_t b;__builtin_memcpy(&b,&v,4);return b;}
static uint64_t o_arg64(double v){uint64_t b;__builtin_memcpy(&b,&v,8);return b;}
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
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
static int o_rbuf(const void *p,uint64_t n,uint64_t want,unsigned t){uintptr_t a=(uintptr_t)p;uint64_t z=t==11?4:o_width(t)/8;if(n>UINTPTR_MAX/z)return 0;if(want&&n>want)return 0;if(!n)return 1;if(!p)return 0;return a%z==0&&a<=UINTPTR_MAX-n*z;}
static int o_rover(const void *a,uint64_t an,const void *b,uint64_t bn){return an&&bn&&o_overlap(a,an,b,bn);}
static uint64_t f0(struct ombr_fs_ctx *fs, uint64_t v0);
static uint64_t f1(struct ombr_fs_ctx *fs, uint64_t v0, void *v1, uint64_t v1_n);
static uint64_t f2(struct ombr_fs_ctx *fs, const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, const void *v3, uint64_t v3_n);
static uint64_t f3(struct ombr_fs_ctx *fs, const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f0(struct ombr_fs_ctx *fs, uint64_t v0){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v0;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(32ULL);
}
uint64_t __attribute__((unused)) v1=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
if(c1++>=32)o_trap(4);
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v1);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f1(struct ombr_fs_ctx *fs, uint64_t v0, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 1ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 2ULL, 5);
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=256ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=1024ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v3;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v5=({uint64_t fs_value=(({uint64_t p0=({uint64_t _a=v2;uint64_t _b=v3;o_op(4, 5, _a,_b);});f0(fs, p0);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=64ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=128ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(12, 1, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=64ULL;o_op(9, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=32ULL;o_op(8, 5, _a,_b);});o_op(12, 1, _a,_b);})){
{uint64_t _i=8ULL;uint64_t _v=v5;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=({uint64_t fs_value=(({uint64_t p0=o_get(v1, v1_n, 0ULL, 5);f0(fs, p0);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=v2;uint64_t _b=32ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=({uint64_t _a=1024ULL;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(4, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
}
if(({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(10, 5, _a,_b);})){
v2=o_get(v1, v1_n, 0ULL, 5);
}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(11, 5, _a,_b);});o_op(12, 1, _a,_b);})){
uint64_t __attribute__((unused)) v3=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(4, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
v2=v3;
}
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 5ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _b=4ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(4, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t c2=0;
while(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=2ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});uint64_t _b=v2;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(c2++>=4)o_trap(4);
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
{uint64_t _i=8ULL;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=({uint64_t _a=64ULL;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(4, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v3=1ULL;
}
{
uint64_t c2=0;
while(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=v3;o_op(3, 5, _a,_b);});uint64_t _b=v2;o_op(15, 5, _a,_b);});uint64_t _b=1024ULL;o_op(10, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(c2++>=4)o_trap(4);
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({uint64_t _a=1ULL;uint64_t _b=v2;o_op(15, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t fs_value=(({uint64_t p0=o_get(v1, v1_n, 4ULL, 5);f0(fs, p0);}));if(fs->code)goto fs_fail;fs_value;});uint64_t _b=32ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 3ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=1ULL;o_op(3, 5, _a,_b);});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v3=({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
v3=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=v3;o_op(1, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=o_get(v1, v1_n, 4ULL, 5);o_op(4, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=o_get(v1, v1_n, 4ULL, 5);o_op(3, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(3, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=o_get(v1, v1_n, 3ULL, 5);o_op(3, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=v2;o_op(10, 5, _a,_b);})){
v2=v3;
}
}
{uint64_t _i=8ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(3, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=o_get(v1, v1_n, 5ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 3ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v3=({uint64_t _a=64ULL;uint64_t _b=o_get(v1, v1_n, 5ULL, 5);o_op(4, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v3=1ULL;
}
if(({uint64_t _a=v3;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(10, 5, _a,_b);})){
v3=o_get(v1, v1_n, 1ULL, 5);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=v3;uint64_t _b=o_get(v1, v1_n, 5ULL, 5);o_op(3, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=2ULL;o_op(1, 5, _a,_b);});o_op(3, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=o_get(v1, v1_n, 9ULL, 5);o_op(8, 5, _a,_b);})){
v4=o_get(v1, v1_n, 9ULL, 5);
}
{uint64_t _i=10ULL;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
v4=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(3, 5, _a,_b);});
v4=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(1, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});
if(({uint64_t _a=o_get(v1, v1_n, 6ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=16ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
}
if(({uint64_t _a=v4;uint64_t _b=8ULL;o_op(8, 5, _a,_b);})){
v4=8ULL;
}
{uint64_t _i=11ULL;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=1024ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=256ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=9223372036854775807ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=18446744073709551615ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=9223372036854775807ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=18446744073709551615ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v2=o_cast(4, 5, o_get(v1, v1_n, 3ULL, 5));
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=o_fcv(11, 9, 0ULL);o_fp(11, 11, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v2;uint64_t _b=2137108966ULL;o_fp(10, 11, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(1, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=2147483647ULL;o_op(10, 5, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=4096ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=9ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=4096ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(12, 1, _a,_b);})){
{uint64_t _i=8ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 1ULL, 5);
if(({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v3=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}else{
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=9223372036854775807ULL;o_op(9, 5, _a,_b);})){
v3=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 4ULL, 5);o_op(1, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v3;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(10, 5, _a,_b);})){
v3=o_get(v1, v1_n, 1ULL, 5);
}
uint64_t __attribute__((unused)) v4=0ULL;
if(({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=9223372036854775807ULL;o_op(9, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 3ULL, 5);o_op(10, 5, _a,_b);})){
v4=({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 3ULL, 5);o_op(2, 5, _a,_b);});
}
}
if(({uint64_t _a=v4;uint64_t _b=v3;o_op(10, 5, _a,_b);})){
v4=v3;
}
{uint64_t _i=8ULL;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 3ULL, 5);o_op(3, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(10, 5, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
v2=o_get(v1, v1_n, 4ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v2=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(3, 5, _a,_b);});
}
{uint64_t _i=9ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
v2=o_get(v1, v1_n, 5ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v2=o_get(v1, v1_n, 2ULL, 5);
}
{uint64_t _i=10ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(10, 5, _a,_b);})){
v2=o_get(v1, v1_n, 1ULL, 5);
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=o_get(v1, v1_n, 4ULL, 5);o_op(3, 5, _a,_b);});o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 6ULL, 5);uint64_t _b=({uint64_t _a=2ULL;uint64_t _b=o_get(v1, v1_n, 5ULL, 5);o_op(3, 5, _a,_b);});o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(11, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=o_get(v1, v1_n, 3ULL, 5);o_op(3, 5, _a,_b);});o_op(8, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(3, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=v3;o_op(10, 5, _a,_b);})){
v2=v3;
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(4, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(11, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(3, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 4ULL, 5);o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 5ULL, 5);o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=4ULL;
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v2=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(3, 5, _a,_b);});
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=2ULL;o_op(1, 5, _a,_b);});o_op(3, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v2=1ULL;
}
v2=({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(3, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=4294967295ULL;o_op(10, 5, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 1ULL, 5);o_op(8, 5, _a,_b);})){
v2=o_get(v1, v1_n, 1ULL, 5);
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=1ULL;
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=128ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 1ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(10, 5, _a,_b);})){
v3=2ULL;
}
v2=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=3ULL;uint64_t _b=v2;o_op(3, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=({uint64_t _a=4609176140021203710ULL;uint64_t _b=4620693217682128896ULL;o_fp(4, 12, _a,_b);});
if(({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v3=o_cast(4, 5, o_get(v1, v1_n, 1ULL, 5));
v2=o_fcv(12, 11, v3);
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=o_fcv(11, 9, 0ULL);o_fp(10, 11, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
uint64_t __attribute__((unused)) v4=o_fcv(12, 5, o_get(v1, v1_n, 0ULL, 5));
uint64_t __attribute__((unused)) v5=v4;
uint64_t __attribute__((unused)) v6=0ULL;
{
uint64_t c4=0;
while(({uint64_t _a=v6;uint64_t _b=64ULL;o_op(8, 5, _a,_b);})){
if(c4++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v7=({uint64_t _a=4602678819172646912ULL;uint64_t _b=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=v4;uint64_t _b=v5;o_fp(4, 12, _a,_b);});o_fp(1, 12, _a,_b);});o_fp(3, 12, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v7;uint64_t _b=v5;o_fp(8, 12, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
break;
}
v5=v7;
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
v2=({uint64_t _a=o_fcv(12, 9, 1ULL);uint64_t _b=v5;o_fp(4, 12, _a,_b);});
}
v2=({uint64_t _a=4609176140021203710ULL;uint64_t _b=v2;o_fp(3, 12, _a,_b);});
}
{uint64_t _i=8ULL;uint64_t _v=o_cast(5, 4, o_fcv(11, 12, v2));o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=4620693217682128896ULL;
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=128ULL;o_op(6, 5, _a,_b);})){
v2=4622558669727284173ULL;
}
{uint64_t _i=8ULL;uint64_t _v=o_cast(5, 4, o_fcv(11, 12, ({uint64_t _a=4609176140021203710ULL;uint64_t _b=v2;o_fp(4, 12, _a,_b);})));o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=24ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=4294967295ULL;o_op(9, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(1, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=4294967295ULL;o_op(9, 5, _a,_b);})){
{uint64_t _i=8ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=25ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(4, 5, _a,_b);});o_op(4, 5, _a,_b);});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v2=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=o_get(v1, v1_n, 2ULL, 5);o_op(5, 5, _a,_b);});
}
{uint64_t _i=8ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 3ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=26ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=16ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 2ULL, 5);uint64_t _b=6ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=64ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=128ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(2ULL);
}
}else{
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 1ULL, 5);uint64_t _b=31ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=256ULL;o_op(10, 5, _a,_b);})){
return o_leave(2ULL);
}
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=27ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(10, 5, _a,_b);})){
v2=4294967295ULL;
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
return o_leave(1ULL);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f2(struct ombr_fs_ctx *fs, const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, const void *v3, uint64_t v3_n){
o_enter();
uint64_t v4[16];
uint64_t v4_n=16;
o_fill(v4, v4_n, 0ULL, 5);
{uint64_t _i=2ULL;uint64_t _v=v2;o_put(v4, v4_n, _i,_v,5);}
{
uint64_t v5=0;
uint64_t k1=o_get(v3, v3_n, 1ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t _a=o_get(v3, v3_n, 0ULL, 5);uint64_t _b=v5;o_op(1, 5, _a,_b);});
{uint64_t _i=0ULL;uint64_t _v=o_cast(5, 4, o_get(v1, v1_n, v6, 4));o_put(v4, v4_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=o_cast(5, 4, o_get(v0, v0_n, v6, 4));o_put(v4, v4_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=({uint64_t fs_value=(({uint64_t p0=15ULL;f1(fs, p0, v4, v4_n);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v7);
}
{uint64_t _i=v6;uint64_t _v=o_cast(4, 5, o_get(v4, v4_n, 8ULL, 5));o_put(v1, v1_n, _i,_v,4);}
}
}
return o_leave(0ULL);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f3(struct ombr_fs_ctx *fs, const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t v3[4];
uint64_t v3_n=4;
o_fill(v3, v3_n, 0ULL, 5);
{
uint64_t v4=0;
uint64_t k1=({uint64_t _a=({uint64_t _a=v0_n;uint64_t _b=1000000ULL;o_op(4, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v4;uint64_t _b=1000000ULL;o_op(3, 5, _a,_b);});o_put(v3, v3_n, _i,_v,5);}
uint64_t __attribute__((unused)) v5=({uint64_t _a=v0_n;uint64_t _b=o_get(v3, v3_n, 0ULL, 5);o_op(2, 5, _a,_b);});
if(({uint64_t _a=v5;uint64_t _b=1000000ULL;o_op(10, 5, _a,_b);})){
v5=1000000ULL;
}
{uint64_t _i=1ULL;uint64_t _v=v5;o_put(v3, v3_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=({uint64_t fs_value=(({uint64_t p2=v2;f2(fs, v0, v0_n, v1, v1_n, p2, v3, v3_n);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v6);
}
}
}
return o_leave(0ULL);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
uint32_t omega_av_eff(uint64_t  *result, const void *p0, volatile uint64_t p0_n, void *p1, volatile uint64_t p1_n, volatile uint64_t p2){
  if(!o_buf(result,1,1,5))return 32;
if(!o_rbuf(p0, p0_n,0, 4))return 32;
  if(o_rover(result,8,p0, p0_n*4))return 32;
if(!o_rbuf(p1, p1_n,0, 4))return 32;
  if(o_rover(result,8,p1, p1_n*4))return 32;
if(o_rover(p1, p1_n*4, p0, p0_n*4))return 32;
if(!o_valid(5, p2))return 32;
  struct ombr_fs_ctx fs_state={0,0};struct ombr_fs_ctx *fs=&fs_state;uint64_t _result=f3(fs, p0, p0_n, p1, p1_n, p2);if(!fs->code)fs_put(fs,result,1,0,_result,5);return fs->code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
