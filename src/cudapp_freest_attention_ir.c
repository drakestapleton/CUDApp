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
static uint64_t f0(struct ombr_fs_ctx *fs, uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f1(struct ombr_fs_ctx *fs, uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f2(struct ombr_fs_ctx *fs, uint64_t v0);
static uint64_t f3(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f4(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f5(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f6(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f7(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f8(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f9(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f10(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f11(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f12(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f13(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f14(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f15(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f16(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f17(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f18(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f19(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f20(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f21(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f22(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f23(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f24(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f25(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f26(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f27(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f28(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n);
static uint64_t f29(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n);
static uint64_t f30(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f31(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f32(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f33(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f34(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f35(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f36(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f37(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f38(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f39(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f40(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f41(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f42(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f43(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f44(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f45(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f46(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f47(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f48(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f49(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f50(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f51(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f52(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f53(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f54(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f55(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f56(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f57(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f58(struct ombr_fs_ctx *fs, uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f59(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f60(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n);
static uint64_t f61(struct ombr_fs_ctx *fs, uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n);
static uint64_t f0(struct ombr_fs_ctx *fs, uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=14ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=17ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=20ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f1(struct ombr_fs_ctx *fs, uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f2(struct ombr_fs_ctx *fs, uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=65535ULL;o_op(6, 5, _a,_b);})){
return o_leave(18446744073709551615ULL);
}
return o_leave(v0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f3(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=256ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(65535ULL);
}
{uint64_t _i=({uint64_t _a=64ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(v2);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f4(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
{uint64_t _i=v2;uint64_t _v=({uint64_t fs_value=(({uint64_t p1=v3;f3(fs, v1, v1_n, p1);}));if(fs->code)goto fs_fail;fs_value;});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f5(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
{
uint64_t v4=0;
uint64_t k1=({uint64_t _a=v3;uint64_t _b=v2;o_op(2, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t p3=1ULL;f4(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f6(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v6=({uint64_t fs_value=(({uint64_t p0=6ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v5;f0(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=v4;uint64_t _b=10ULL;o_op(8, 5, _a,_b);})){
{uint64_t _i=32ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{
uint64_t v7=0;
uint64_t k2=8ULL;
for(;o_op(8,5, v7, k2);v7=o_op(1,5, v7,1)){
{uint64_t _i=({uint64_t _a=33ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
}
}
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v6=({uint64_t fs_value=(({uint64_t p0=13ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v5;f0(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
{uint64_t _i=3ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=v4;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v6=({uint64_t _a=({uint64_t fs_value=(({uint64_t p0=4ULL;uint64_t p1=0ULL;uint64_t p2=7ULL;uint64_t p3=v5;f0(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
{uint64_t _i=3ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
}
if(({uint64_t _a=v4;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v6=({uint64_t _a=({uint64_t fs_value=(({uint64_t p0=4ULL;uint64_t p1=2ULL;uint64_t p2=7ULL;uint64_t p3=v5;f0(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
{uint64_t _i=3ULL;uint64_t _v=4ULL;o_put(v0, v0_n, _i,_v,5);}
}
if(({uint64_t _a=v4;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v6=({uint64_t _a=({uint64_t fs_value=(({uint64_t p0=4ULL;uint64_t p1=7ULL;uint64_t p2=1ULL;uint64_t p3=v5;f0(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
{uint64_t _i=3ULL;uint64_t _v=2ULL;o_put(v0, v0_n, _i,_v,5);}
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=6ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=7ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v6=({uint64_t fs_value=(({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v5;f0(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=3ULL;uint64_t _v=v5;o_put(v0, v0_n, _i,_v,5);}
}
}
if(({uint64_t _a=v4;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
v6=({uint64_t _a=({uint64_t fs_value=(({uint64_t p0=1ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v5;f0(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
}
if(({uint64_t _a=v4;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
v6=({uint64_t _a=({uint64_t fs_value=(({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v5;f0(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
}
}else{
if(({uint64_t _a=v4;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t _a=v1;uint64_t _b=24ULL;o_op(16, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v7;uint64_t _b=8ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=33ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);}), 5);uint64_t _b=4ULL;o_op(11, 5, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=3ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
v6=({uint64_t _a=({uint64_t fs_value=(({uint64_t p0=4ULL;uint64_t p1=v7;uint64_t p2=1ULL;uint64_t p3=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 32ULL, 5);uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});f0(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v7;o_op(15, 5, _a,_b);});uint64_t _b=2ULL;o_op(13, 5, _a,_b);});
{uint64_t _i=32ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 32ULL, 5);uint64_t _b=v8;o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=33ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=33ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);}), 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t _a=v5;uint64_t _b=v8;o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
}else{
if(({uint64_t _a=v4;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 32ULL, 5);uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});
if(({uint64_t _a=o_get(v0, v0_n, 32ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v7=({uint64_t _a=v7;uint64_t _b=2ULL;o_op(13, 5, _a,_b);});
}
v6=({uint64_t fs_value=(({uint64_t p0=6ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v7;f0(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
v6=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});
}
}
}
return o_leave(v6);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f7(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, 0ULL, 5);
if(({uint64_t _a=v6;uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v7=({uint64_t fs_value=(({uint64_t p1=v2;uint64_t p2=v4;uint64_t p3=v5;f6(fs, v1, v1_n, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v1, v1_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
v6=({uint64_t _a=v6;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});
{uint64_t _i=v6;uint64_t _v=({uint64_t _a=v2;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t fs_value=(({uint64_t p0=({uint64_t _a=v3;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f2(fs, p0);}));if(fs->code)goto fs_fail;fs_value;});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t fs_value=(({uint64_t p0=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f2(fs, p0);}));if(fs->code)goto fs_fail;fs_value;});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t fs_value=(({uint64_t p0=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f2(fs, p0);}));if(fs->code)goto fs_fail;fs_value;});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t fs_value=(({uint64_t p0=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(16, 5, _a,_b);});f2(fs, p0);}));if(fs->code)goto fs_fail;fs_value;});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=18446744073709551615ULL;o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=65536ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v6;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=v5;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f8(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t fs_value=(({uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f9(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t fs_value=(({uint64_t p2=2ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v2;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=v3;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f10(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({uint64_t fs_value=(({uint64_t p2=1ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v2;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f11(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=v2;uint64_t _b=512ULL;o_op(13, 5, _a,_b);});uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v3;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=v4;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f12(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t fs_value=(({uint64_t p2=8ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=v5;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=v4;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f13(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t fs_value=(({uint64_t p2=8ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=v4;uint64_t p3=v5;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f14(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t fs_value=(({uint64_t p2=9ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=v5;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=v4;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f15(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t fs_value=(({uint64_t p2=v2;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v3;uint64_t p1=v4;uint64_t p2=v5;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f16(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t fs_value=(({uint64_t p2=17ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=v4;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f17(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t fs_value=(({uint64_t p2=18ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=v4;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f18(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t fs_value=(({uint64_t p2=66059ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f19(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t fs_value=(({uint64_t p2=66572ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f20(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t fs_value=(({uint64_t p2=561ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f21(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t fs_value=(({uint64_t p2=1074ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f22(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 41ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
({uint64_t fs_value=(({uint64_t p2=1328ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f23(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 41ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
({uint64_t fs_value=(({uint64_t p2=554ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v2;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=v4;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f24(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t fs_value=(({uint64_t p2=284ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f25(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
({uint64_t fs_value=(({uint64_t p2=1838ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t p5=6ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f26(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
({uint64_t fs_value=(({uint64_t p2=1838ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=2ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(v2);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f27(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=v3;uint64_t _v=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v3, 5);uint64_t _b=({uint64_t _a=4294967295ULL;uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=v2;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f28(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 6ULL, 5);
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=8ULL;o_op(11, 5, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=5ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=({uint64_t _a=8ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, 0ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f29(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 6ULL, 5);
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=8ULL;o_op(11, 5, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=5ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
{uint64_t _i=6ULL;uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v2=({uint64_t _a=16ULL;uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{uint64_t _i=v2;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=8ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);}), 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 5ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f30(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t fs_value=(({uint64_t p2=1581ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=3118ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=({uint64_t _a=1032192ULL;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f31(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2560ULL;o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=24ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=58ULL;o_op(7, 5, _a,_b);})){
v6=({uint64_t _a=v6;uint64_t _b=65536ULL;o_op(13, 5, _a,_b);});
}
({uint64_t fs_value=(({uint64_t p2=v6;uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f32(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t fs_value=(({uint64_t p2=33ULL;uint64_t p3=v2;uint64_t p4=0ULL;uint64_t p5=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=24ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f33(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=9ULL;
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v5=({uint64_t _a=v5;uint64_t _b=2816ULL;o_op(13, 5, _a,_b);});
}
({uint64_t fs_value=(({uint64_t p2=v5;uint64_t p3=v2;uint64_t p4=1ULL;uint64_t p5=({uint64_t _a=v3;uint64_t _b=24ULL;o_op(15, 5, _a,_b);});f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f34(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 41ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=41ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=42ULL;uint64_t _v=o_get(v1, v1_n, 0ULL, 5);o_put(v1, v1_n, _i,_v,5);}
({uint64_t fs_value=(({uint64_t p2=2102ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=2ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v1, v1_n, 44ULL, 5);uint64_t p3=o_get(v1, v1_n, 45ULL, 5);f24(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=43ULL;uint64_t _v=o_get(v1, v1_n, 0ULL, 5);o_put(v1, v1_n, _i,_v,5);}
({uint64_t fs_value=(({uint64_t p2=1838ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=1ULL;uint64_t p5=2ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f35(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 41ULL, 5);uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 0ULL, 5);
uint64_t __attribute__((unused)) v4=55ULL;
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v4=0ULL;
}
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=v4;uint64_t _b=2304ULL;o_op(13, 5, _a,_b);});uint64_t p3=({uint64_t fs_value=(({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v1, v1_n, 42ULL, 5);f27(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 43ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=o_get(v0, v0_n, v5, 5);uint64_t _b=({uint64_t _a=4294967295ULL;uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=o_get(v1, v1_n, 43ULL, 5);o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
{uint64_t _i=v5;uint64_t _v=({uint64_t _a=v6;uint64_t _b=v7;o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=41ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f36(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 72ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=v3;uint64_t p5=o_get(v2, v2_n, 71ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=29ULL;uint64_t p3=o_get(v2, v2_n, 72ULL, 5);uint64_t p4=o_get(v2, v2_n, 72ULL, 5);uint64_t p5=o_get(v2, v2_n, 50ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 64ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=o_get(v2, v2_n, 45ULL, 5);uint64_t p5=o_get(v2, v2_n, 72ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 65ULL, 5);uint64_t p3=o_get(v2, v2_n, 64ULL, 5);uint64_t p4=o_get(v2, v2_n, 9ULL, 5);uint64_t p5=o_get(v2, v2_n, 63ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 66ULL, 5);uint64_t p3=o_get(v2, v2_n, 65ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 32ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 67ULL, 5);uint64_t p3=o_get(v2, v2_n, 66ULL, 5);f18(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 73ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=({uint64_t _a=v3;uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 63ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 73ULL, 5);uint64_t p3=o_get(v2, v2_n, 67ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f37(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=2ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=320ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
{
uint64_t v3=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=32ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=36ULL;uint64_t p3=47ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=775ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=0ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=856ULL;uint64_t p5=1ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=5ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=32ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=({uint64_t _a=896ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v3=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=4ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=39ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=({uint64_t _a=928ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 36ULL, 5);uint64_t p4=33ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 37ULL, 5);uint64_t p4=37ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=38ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 17ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=47ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 14ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 47ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=({uint64_t _a=o_get(v2, v2_n, 13ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f17(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 38ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=o_get(v2, v2_n, 14ULL, 5);f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 47ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=50ULL;uint64_t p3=59ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 50ULL, 5);uint64_t p3=1ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 51ULL, 5);uint64_t p3=4ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 52ULL, 5);uint64_t p3=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=2ULL;o_op(4, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 53ULL, 5);uint64_t p3=o_get(v2, v2_n, 1ULL, 5);f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 54ULL, 5);uint64_t p3=o_get(v2, v2_n, 10ULL, 5);f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 55ULL, 5);uint64_t p3=o_get(v2, v2_n, 11ULL, 5);f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 56ULL, 5);uint64_t p3=o_get(v2, v2_n, 12ULL, 5);f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 57ULL, 5);uint64_t p3=({uint64_t _a=o_get(v2, v2_n, 12ULL, 5);uint64_t _b=128ULL;o_op(1, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 58ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=128ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=59ULL;uint64_t p3=63ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 59ULL, 5);uint64_t p3=({uint64_t _a=o_get(v2, v2_n, 12ULL, 5);uint64_t _b=256ULL;o_op(1, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 60ULL, 5);uint64_t p3=({uint64_t _a=o_get(v2, v2_n, 12ULL, 5);uint64_t _b=384ULL;o_op(1, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=63ULL;uint64_t p3=66ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=66ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=67ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=68ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
{uint64_t _i=69ULL;uint64_t _v=o_get(v2, v2_n, 37ULL, 5);o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=70ULL;uint64_t _v=o_get(v2, v2_n, 37ULL, 5);o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=5ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=69ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=29ULL;uint64_t p3=o_get(v2, v2_n, 69ULL, 5);uint64_t p4=o_get(v2, v2_n, 37ULL, 5);uint64_t p5=o_get(v2, v2_n, 50ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=6ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=70ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=29ULL;uint64_t p3=o_get(v2, v2_n, 70ULL, 5);uint64_t p4=o_get(v2, v2_n, 37ULL, 5);uint64_t p5=o_get(v2, v2_n, 50ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 63ULL, 5);uint64_t p3=o_get(v2, v2_n, 36ULL, 5);uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 64ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=o_get(v2, v2_n, 45ULL, 5);uint64_t p5=o_get(v2, v2_n, 69ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 65ULL, 5);uint64_t p3=o_get(v2, v2_n, 64ULL, 5);uint64_t p4=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 63ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 66ULL, 5);uint64_t p3=o_get(v2, v2_n, 65ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 32ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 67ULL, 5);uint64_t p3=o_get(v2, v2_n, 66ULL, 5);f18(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 63ULL, 5);uint64_t p3=o_get(v2, v2_n, 67ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 17ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v2, v2_n, 70ULL, 5);uint64_t _b=o_get(v2, v2_n, 69ULL, 5);o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 68ULL, 5);uint64_t p3=o_get(v2, v2_n, 65ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 34ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=186ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=187ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 186ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=o_get(v2, v2_n, 45ULL, 5);uint64_t p5=o_get(v2, v2_n, 70ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 187ULL, 5);uint64_t p3=o_get(v2, v2_n, 186ULL, 5);uint64_t p4=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 63ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 68ULL, 5);uint64_t p3=o_get(v2, v2_n, 187ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 34ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}else{
({uint64_t fs_value=(({uint64_t p2=71ULL;uint64_t p3=74ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 71ULL, 5);uint64_t p3=o_get(v2, v2_n, 37ULL, 5);uint64_t p4=o_get(v2, v2_n, 8ULL, 5);uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k2=o_get(v2, v2_n, 8ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
if(c2++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p3=v3;f36(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
({uint64_t fs_value=(({uint64_t p2=74ULL;uint64_t p3=77ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=77ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=78ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=79ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=80ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 74ULL, 5);uint64_t p3=o_get(v2, v2_n, 37ULL, 5);uint64_t p4=({uint64_t _a=o_get(v2, v2_n, 3ULL, 5);uint64_t _b=o_get(v2, v2_n, 24ULL, 5);o_op(2, 5, _a,_b);});f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=29ULL;uint64_t p3=o_get(v2, v2_n, 74ULL, 5);uint64_t p4=o_get(v2, v2_n, 74ULL, 5);uint64_t p5=o_get(v2, v2_n, 50ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 75ULL, 5);uint64_t p3=o_get(v2, v2_n, 74ULL, 5);uint64_t p4=o_get(v2, v2_n, 44ULL, 5);uint64_t p5=65535ULL;f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 76ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=o_get(v2, v2_n, 39ULL, 5);uint64_t p5=65535ULL;f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 76ULL, 5);uint64_t p3=o_get(v2, v2_n, 76ULL, 5);uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 77ULL, 5);uint64_t p3=o_get(v2, v2_n, 76ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 35ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 76ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 46ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 78ULL, 5);uint64_t p3=o_get(v2, v2_n, 76ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 35ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 79ULL, 5);uint64_t p3=o_get(v2, v2_n, 78ULL, 5);f18(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f38(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 87ULL, 5);uint64_t p3=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 83ULL, 5);uint64_t p4=o_get(v2, v2_n, 79ULL, 5);uint64_t p5=o_get(v2, v2_n, 87ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 83ULL, 5);uint64_t p3=o_get(v2, v2_n, 83ULL, 5);uint64_t p4=v3;f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 83ULL, 5);uint64_t p4=o_get(v2, v2_n, 83ULL, 5);uint64_t p5=o_get(v2, v2_n, 86ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 83ULL, 5);uint64_t p3=o_get(v2, v2_n, 83ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(14, 5, _a,_b);});f17(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 87ULL, 5);uint64_t p3=({uint64_t _a=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});uint64_t _b=0;o_op(17, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 84ULL, 5);uint64_t p4=o_get(v2, v2_n, 82ULL, 5);uint64_t p5=o_get(v2, v2_n, 87ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 85ULL, 5);uint64_t p3=o_get(v2, v2_n, 84ULL, 5);uint64_t p4=31ULL;f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 84ULL, 5);uint64_t p3=o_get(v2, v2_n, 83ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 81ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 81ULL, 5);uint64_t p3=o_get(v2, v2_n, 85ULL, 5);uint64_t p4=o_get(v2, v2_n, 84ULL, 5);uint64_t p5=o_get(v2, v2_n, 83ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f39(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
if(({uint64_t _a=o_get(v2, v2_n, 17ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=81ULL;uint64_t p3=88ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=48ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=49ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 86ULL, 5);uint64_t p3=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 81ULL, 5);uint64_t p4=o_get(v2, v2_n, 79ULL, 5);uint64_t p5=o_get(v2, v2_n, 86ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 81ULL, 5);uint64_t p3=o_get(v2, v2_n, 81ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(14, 5, _a,_b);});f17(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 82ULL, 5);uint64_t p3=o_get(v2, v2_n, 81ULL, 5);uint64_t p4=o_get(v2, v2_n, 15ULL, 5);f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k2=o_get(v2, v2_n, 14ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
if(c2++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});f38(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 48ULL, 5);uint64_t p3=o_get(v2, v2_n, 47ULL, 5);uint64_t p4=o_get(v2, v2_n, 81ULL, 5);uint64_t p5=65535ULL;f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 49ULL, 5);uint64_t p4=o_get(v2, v2_n, 48ULL, 5);uint64_t p5=o_get(v2, v2_n, 81ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 84ULL, 5);uint64_t p3=o_get(v2, v2_n, 49ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 79ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 85ULL, 5);uint64_t p3=o_get(v2, v2_n, 84ULL, 5);uint64_t p4=31ULL;f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 49ULL, 5);uint64_t p3=o_get(v2, v2_n, 85ULL, 5);uint64_t p4=o_get(v2, v2_n, 84ULL, 5);uint64_t p5=o_get(v2, v2_n, 49ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=16ULL;uint64_t _v=o_get(v2, v2_n, 49ULL, 5);o_put(v2, v2_n, _i,_v,5);}
}else{
{uint64_t _i=16ULL;uint64_t _v=o_get(v2, v2_n, 79ULL, 5);o_put(v2, v2_n, _i,_v,5);}
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 80ULL, 5);uint64_t p3=o_get(v2, v2_n, 36ULL, 5);uint64_t p4=o_get(v2, v2_n, 25ULL, 5);uint64_t p5=o_get(v2, v2_n, 33ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 80ULL, 5);uint64_t p3=o_get(v2, v2_n, 43ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 80ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=88ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=92ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=96ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=100ULL;uint64_t p3=104ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=88ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=4286578688ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=92ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=92ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=1065353216ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=96ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
}
}
if(({uint64_t _a=o_get(v2, v2_n, 17ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 100ULL, 5);uint64_t p4=o_get(v2, v2_n, 48ULL, 5);uint64_t p5=o_get(v2, v2_n, 58ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 100ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 101ULL, 5);uint64_t p4=o_get(v2, v2_n, 63ULL, 5);uint64_t p5=o_get(v2, v2_n, 54ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 102ULL, 5);uint64_t p4=o_get(v2, v2_n, 63ULL, 5);uint64_t p5=o_get(v2, v2_n, 55ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 103ULL, 5);uint64_t p4=o_get(v2, v2_n, 63ULL, 5);uint64_t p5=o_get(v2, v2_n, 56ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f22(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 17ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 48ULL, 5);uint64_t p3=o_get(v2, v2_n, 49ULL, 5);f24(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 58ULL, 5);uint64_t p3=o_get(v2, v2_n, 79ULL, 5);f24(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
{uint64_t _i=189ULL;uint64_t _v=({uint64_t fs_value=(({f26(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f40(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
({uint64_t fs_value=(({uint64_t p2=104ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=109ULL;uint64_t p3=112ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=112ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=113ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=114ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=115ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=119ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=120ULL;uint64_t p3=132ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=132ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 128ULL, 5);uint64_t p3=4294967295ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 127ULL, 5);uint64_t p4=o_get(v2, v2_n, 16ULL, 5);uint64_t p5=o_get(v2, v2_n, 128ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 129ULL, 5);uint64_t p3=4286578688ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f28(fs, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=191ULL;uint64_t _v=o_get(v1, v1_n, 0ULL, 5);o_put(v2, v2_n, _i,_v,5);}
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 104ULL, 5);uint64_t p4=o_get(v2, v2_n, 100ULL, 5);uint64_t p5=o_get(v2, v2_n, 36ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=4286578688ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 109ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 5ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=44ULL;uint64_t _v=o_get(v2, v2_n, 104ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=45ULL;uint64_t _v=o_get(v2, v2_n, 16ULL, 5);o_put(v1, v1_n, _i,_v,5);}
({uint64_t fs_value=(({f34(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 124ULL, 5);uint64_t p3=o_get(v2, v2_n, 104ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 127ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 125ULL, 5);uint64_t p3=o_get(v2, v2_n, 124ULL, 5);uint64_t p4=31ULL;f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 126ULL, 5);uint64_t p3=o_get(v2, v2_n, 125ULL, 5);uint64_t p4=o_get(v2, v2_n, 36ULL, 5);uint64_t p5=65535ULL;f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 104ULL, 5);uint64_t p3=o_get(v2, v2_n, 126ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 104ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 110ULL, 5);uint64_t p3=o_get(v2, v2_n, 104ULL, 5);uint64_t p4=o_get(v2, v2_n, 2ULL, 5);f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 111ULL, 5);uint64_t p3=o_get(v2, v2_n, 104ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=o_get(v2, v2_n, 2ULL, 5);o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f17(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 112ULL, 5);uint64_t p3=o_get(v2, v2_n, 110ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 77ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 113ULL, 5);uint64_t p3=o_get(v2, v2_n, 112ULL, 5);f18(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 109ULL, 5);uint64_t p3=o_get(v2, v2_n, 113ULL, 5);uint64_t p4=o_get(v2, v2_n, 41ULL, 5);uint64_t p5=65535ULL;f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=133ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 123ULL, 5);uint64_t p4=o_get(v2, v2_n, 104ULL, 5);uint64_t p5=o_get(v2, v2_n, 50ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 133ULL, 5);uint64_t p3=o_get(v2, v2_n, 123ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=o_get(v2, v2_n, 2ULL, 5);o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f17(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 133ULL, 5);uint64_t p3=o_get(v2, v2_n, 133ULL, 5);uint64_t p4=o_get(v2, v2_n, 42ULL, 5);uint64_t p5=o_get(v2, v2_n, 109ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 133ULL, 5);uint64_t p4=o_get(v2, v2_n, 133ULL, 5);uint64_t p5=o_get(v2, v2_n, 75ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 114ULL, 5);uint64_t p3=o_get(v2, v2_n, 133ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 33ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 109ULL, 5);uint64_t p3=o_get(v2, v2_n, 111ULL, 5);uint64_t p4=o_get(v2, v2_n, 42ULL, 5);uint64_t p5=o_get(v2, v2_n, 109ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 109ULL, 5);uint64_t p4=o_get(v2, v2_n, 109ULL, 5);uint64_t p5=o_get(v2, v2_n, 75ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 109ULL, 5);uint64_t p3=o_get(v2, v2_n, 111ULL, 5);uint64_t p4=o_get(v2, v2_n, 42ULL, 5);uint64_t p5=o_get(v2, v2_n, 109ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 109ULL, 5);uint64_t p4=o_get(v2, v2_n, 109ULL, 5);uint64_t p5=o_get(v2, v2_n, 75ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 114ULL, 5);uint64_t p3=o_get(v2, v2_n, 109ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 33ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=134ULL;uint64_t p3=4ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=134ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p3=4ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 19ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=136ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=4ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=115ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 119ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f28(fs, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=192ULL;uint64_t _v=o_get(v1, v1_n, 0ULL, 5);o_put(v2, v2_n, _i,_v,5);}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 122ULL, 5);uint64_t p3=o_get(v2, v2_n, 119ULL, 5);uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=57ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 134ULL, 5);uint64_t p1=o_get(v2, v2_n, 114ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f31(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=57ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=134ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, 114ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=16ULL;uint64_t p5=0ULL;f31(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=27ULL;uint64_t _v=v3;o_put(v2, v2_n, _i,_v,5);}
{
uint64_t v4=0;
uint64_t k2=2ULL;
for(;o_op(8,5, v4, k2);v4=o_op(1,5, v4,1)){
{uint64_t _i=28ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
({uint64_t fs_value=(({f50(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 114ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=32ULL;uint64_t p5=o_get(v2, v2_n, 114ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 119ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=o_get(v2, v2_n, 18ULL, 5);uint64_t p5=o_get(v2, v2_n, 119ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 119ULL, 5);uint64_t p3=o_get(v2, v2_n, 53ULL, 5);f24(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 192ULL, 5);f25(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f29(fs, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=32ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=115ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p5=o_get(v2, v2_n, 40ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
if(({uint64_t _a=o_get(v2, v2_n, 5ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=o_get(v2, v2_n, 5ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f35(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 130ULL, 5);uint64_t p3=o_get(v2, v2_n, 125ULL, 5);uint64_t p4=2147483648ULL;uint64_t p5=o_get(v2, v2_n, 129ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k2=o_get(v2, v2_n, 8ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
if(c2++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=31ULL;uint64_t p3=o_get(v2, v2_n, 131ULL, 5);uint64_t p4=o_get(v2, v2_n, 58ULL, 5);uint64_t p5=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, 131ULL, 5);uint64_t p4=o_get(v2, v2_n, 131ULL, 5);uint64_t p5=o_get(v2, v2_n, 130ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=31ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=o_get(v2, v2_n, 58ULL, 5);uint64_t p5=o_get(v2, v2_n, 131ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 101ULL, 5);uint64_t p3=o_get(v2, v2_n, 105ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=({uint64_t _a=v3;uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 101ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 102ULL, 5);uint64_t p3=o_get(v2, v2_n, 109ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f41(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 142ULL, 5);uint64_t p3=o_get(v2, v2_n, 56ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 143ULL, 5);uint64_t p3=o_get(v2, v2_n, 57ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=({uint64_t _a=v3;uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 56ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 142ULL, 5);uint64_t p3=o_get(v2, v2_n, 132ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=({uint64_t _a=v3;uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 57ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 143ULL, 5);uint64_t p3=o_get(v2, v2_n, 132ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=v4;uint64_t p3=v5;uint64_t p4=o_get(v2, v2_n, 142ULL, 5);uint64_t p5=o_get(v2, v2_n, 143ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=128ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 61ULL, 5);uint64_t p3=o_get(v2, v2_n, 59ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 62ULL, 5);uint64_t p3=o_get(v2, v2_n, 60ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=({uint64_t _a=v3;uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 59ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 61ULL, 5);uint64_t p3=o_get(v2, v2_n, 132ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=({uint64_t _a=v3;uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 60ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 62ULL, 5);uint64_t p3=o_get(v2, v2_n, 132ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=v4;uint64_t p3=v5;uint64_t p4=v5;uint64_t p5=o_get(v2, v2_n, 61ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=v4;uint64_t p3=v5;uint64_t p4=v5;uint64_t p5=o_get(v2, v2_n, 62ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f42(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 152ULL, 5);
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
v4=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 101ULL, 5);uint64_t p3=v4;f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=({uint64_t _a=v3;uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 101ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=v4;f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 140ULL, 5);uint64_t p3=v4;uint64_t p4=16ULL;f23(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=30ULL;uint64_t p3=o_get(v2, v2_n, 153ULL, 5);uint64_t p4=v4;uint64_t p5=o_get(v2, v2_n, 140ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v5=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 140ULL, 5);uint64_t p3=o_get(v2, v2_n, 153ULL, 5);uint64_t p4=({uint64_t _a=8ULL;uint64_t _b=v5;o_op(16, 5, _a,_b);});f23(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=30ULL;uint64_t p3=o_get(v2, v2_n, 153ULL, 5);uint64_t p4=o_get(v2, v2_n, 153ULL, 5);uint64_t p5=o_get(v2, v2_n, 140ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 103ULL, 5);uint64_t p3=o_get(v2, v2_n, 153ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=({uint64_t _a=v3;uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 103ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, 153ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f43(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
({uint64_t fs_value=(({uint64_t p2=140ULL;uint64_t p3=({uint64_t _a=147ULL;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=151ULL;uint64_t p3=155ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=({uint64_t _a=147ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 140ULL, 5);uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=16ULL;f23(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, 141ULL, 5);uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p5=o_get(v2, v2_n, 140ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v4=0;
uint64_t k2=4ULL;
for(;o_op(8,5, v4, k2);v4=o_op(1,5, v4,1)){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 140ULL, 5);uint64_t p3=o_get(v2, v2_n, 141ULL, 5);uint64_t p4=({uint64_t _a=8ULL;uint64_t _b=v4;o_op(16, 5, _a,_b);});f23(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, 141ULL, 5);uint64_t p4=o_get(v2, v2_n, 141ULL, 5);uint64_t p5=o_get(v2, v2_n, 140ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 103ULL, 5);uint64_t p3=o_get(v2, v2_n, 141ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=({uint64_t _a=v3;uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 103ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 132ULL, 5);uint64_t p3=o_get(v2, v2_n, 141ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
({uint64_t fs_value=(({f22(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 152ULL, 5);
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
v4=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
}
({uint64_t fs_value=(({uint64_t p3=v3;uint64_t p4=37ULL;uint64_t p5=o_get(v2, v2_n, 144ULL, 5);f41(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, 145ULL, 5);uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=88ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p5=o_get(v2, v2_n, 144ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=147ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=1065353216ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=559ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v4;uint64_t p1=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=88ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=o_get(v2, v2_n, 145ULL, 5);uint64_t p5=o_get(v2, v2_n, 144ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=31ULL;uint64_t p3=o_get(v2, v2_n, 146ULL, 5);uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=88ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p5=o_get(v2, v2_n, 145ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=559ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=147ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, 146ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=88ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=o_get(v2, v2_n, 145ULL, 5);uint64_t p5=o_get(v2, v2_n, 144ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=31ULL;uint64_t p3=o_get(v2, v2_n, 151ULL, 5);uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=105ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p5=o_get(v2, v2_n, 145ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=559ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=v4;uint64_t p1=o_get(v2, v2_n, 151ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
({uint64_t fs_value=(({f22(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p3=v3;f42(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({f22(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=o_get(v2, v2_n, 8ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p3=v3;uint64_t p4=30ULL;uint64_t p5=o_get(v2, v2_n, 154ULL, 5);f41(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p3=v3;f51(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f44(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
({uint64_t fs_value=(({uint64_t p2=155ULL;uint64_t p3=159ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 7ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=163ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
{
uint64_t v3=0;
uint64_t k1=2ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=159ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=4ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=161ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=4ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=164ULL;uint64_t p3=172ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 155ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f28(fs, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=193ULL;uint64_t _v=o_get(v1, v1_n, 0ULL, 5);o_put(v2, v2_n, _i,_v,5);}
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 156ULL, 5);uint64_t p4=o_get(v2, v2_n, 100ULL, 5);uint64_t p5=o_get(v2, v2_n, 155ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 156ULL, 5);uint64_t p3=o_get(v2, v2_n, 16ULL, 5);f24(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=190ULL;uint64_t _v=({uint64_t fs_value=(({f26(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});o_put(v2, v2_n, _i,_v,5);}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 157ULL, 5);uint64_t p3=o_get(v2, v2_n, 155ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 55ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 158ULL, 5);uint64_t p3=o_get(v2, v2_n, 155ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 54ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=2ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=58ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=159ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, 157ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});uint64_t p5=4ULL;f31(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v3=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 163ULL, 5);uint64_t p1=o_get(v2, v2_n, ({uint64_t _a=159ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(4, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);uint64_t p2=65535ULL;uint64_t p3=o_get(v2, v2_n, 80ULL, 5);f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p3=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(5, 5, _a,_b);});uint64_t p4=v3;f33(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
uint64_t __attribute__((unused)) v4=0ULL;
if(({uint64_t _a=v3;uint64_t _b=4ULL;o_op(11, 5, _a,_b);})){
v4=3ULL;
}
uint64_t __attribute__((unused)) v5=19ULL;
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v5=11ULL;
}
({uint64_t fs_value=(({uint64_t p2=v5;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, 163ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=v4;f31(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{
uint64_t v3=0;
uint64_t k2=2ULL;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=58ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=161ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, 158ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});uint64_t p5=4ULL;f31(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v3=0;
uint64_t k2=8ULL;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=65536ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 96ULL, 5);uint64_t p1=o_get(v2, v2_n, ({uint64_t _a=161ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(4, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, 96ULL, 5);f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p3=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(5, 5, _a,_b);});uint64_t p4=0ULL;f32(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}else{
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{
uint64_t v3=0;
uint64_t k3=8ULL;
for(;o_op(8,5, v3, k3);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=65536ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
{
uint64_t v3=0;
uint64_t k2=o_get(v2, v2_n, 8ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
if(c2++>=1000000)o_trap(4);
{uint64_t _i=27ULL;uint64_t _v=v3;o_put(v2, v2_n, _i,_v,5);}
{
uint64_t v4=0;
uint64_t k3=2ULL;
for(;o_op(8,5, v4, k3);v4=o_op(1,5, v4,1)){
({uint64_t fs_value=(({uint64_t p2=58ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=161ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, 158ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t p5=4ULL;f31(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v4=0;
uint64_t k3=8ULL;
for(;o_op(8,5, v4, k3);v4=o_op(1,5, v4,1)){
({uint64_t fs_value=(({uint64_t p3=v4;f52(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
}
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 155ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=8ULL;uint64_t p5=o_get(v2, v2_n, 155ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 155ULL, 5);uint64_t p3=o_get(v2, v2_n, 53ULL, 5);f24(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 193ULL, 5);f25(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f29(fs, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 190ULL, 5);f27(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f22(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 100ULL, 5);uint64_t p4=o_get(v2, v2_n, 100ULL, 5);uint64_t p5=o_get(v2, v2_n, 53ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 7ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=199ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 199ULL, 5);uint64_t p4=o_get(v2, v2_n, 100ULL, 5);uint64_t p5=o_get(v2, v2_n, 53ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 199ULL, 5);uint64_t p4=o_get(v2, v2_n, 199ULL, 5);uint64_t p5=o_get(v2, v2_n, 128ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 199ULL, 5);uint64_t p3=o_get(v2, v2_n, 16ULL, 5);f24(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 100ULL, 5);uint64_t p3=o_get(v2, v2_n, 16ULL, 5);f24(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 191ULL, 5);f25(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f29(fs, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 189ULL, 5);f27(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f45(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 178ULL, 5);uint64_t p3=o_get(v2, v2_n, 173ULL, 5);uint64_t p4=o_get(v2, v2_n, 176ULL, 5);uint64_t p5=o_get(v2, v2_n, 175ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 178ULL, 5);uint64_t p3=o_get(v2, v2_n, 178ULL, 5);uint64_t p4=o_get(v2, v2_n, 13ULL, 5);uint64_t p5=o_get(v2, v2_n, 174ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 179ULL, 5);uint64_t p3=o_get(v2, v2_n, 178ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 177ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 180ULL, 5);uint64_t p3=o_get(v2, v2_n, 179ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 183ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 180ULL, 5);uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=96ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);f19(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 179ULL, 5);uint64_t p3=o_get(v2, v2_n, 178ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 181ULL, 5);uint64_t p3=o_get(v2, v2_n, 179ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 183ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 181ULL, 5);uint64_t p3=o_get(v2, v2_n, 53ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 181ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 181ULL, 5);uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=88ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);f19(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 182ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 181ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 182ULL, 5);uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=92ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);f19(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f46(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
if(({uint64_t _a=o_get(v2, v2_n, 17ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=172ULL;uint64_t p3=176ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=72ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=176ULL;uint64_t p3=180ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k2=4ULL;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=180ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 172ULL, 5);uint64_t p4=38ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 175ULL, 5);uint64_t p4=37ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=4ULL;uint64_t p3=o_get(v2, v2_n, 176ULL, 5);uint64_t p4=952ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=5ULL;uint64_t p3=o_get(v2, v2_n, 183ULL, 5);uint64_t p4=912ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=29ULL;uint64_t p3=o_get(v2, v2_n, 175ULL, 5);uint64_t p4=o_get(v2, v2_n, 175ULL, 5);uint64_t p5=o_get(v2, v2_n, 50ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 175ULL, 5);uint64_t p3=o_get(v2, v2_n, 175ULL, 5);uint64_t p4=o_get(v2, v2_n, 8ULL, 5);uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
if(({uint64_t _a=o_get(v2, v2_n, 14ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 174ULL, 5);uint64_t p3=o_get(v2, v2_n, 172ULL, 5);uint64_t p4=({uint64_t _a=o_get(v2, v2_n, 13ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f17(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 173ULL, 5);uint64_t p3=o_get(v2, v2_n, 172ULL, 5);uint64_t p4=o_get(v2, v2_n, 14ULL, 5);f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 174ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 173ULL, 5);uint64_t p4=o_get(v2, v2_n, 172ULL, 5);uint64_t p5=o_get(v2, v2_n, 58ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 178ULL, 5);uint64_t p3=o_get(v2, v2_n, 173ULL, 5);uint64_t p4=o_get(v2, v2_n, 176ULL, 5);uint64_t p5=o_get(v2, v2_n, 175ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 178ULL, 5);uint64_t p3=o_get(v2, v2_n, 178ULL, 5);uint64_t p4=o_get(v2, v2_n, 13ULL, 5);uint64_t p5=o_get(v2, v2_n, 174ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 177ULL, 5);uint64_t p3=o_get(v2, v2_n, 36ULL, 5);uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 179ULL, 5);uint64_t p3=o_get(v2, v2_n, 178ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 177ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 180ULL, 5);uint64_t p3=o_get(v2, v2_n, 179ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 183ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 180ULL, 5);uint64_t p3=o_get(v2, v2_n, 96ULL, 5);f19(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 179ULL, 5);uint64_t p3=o_get(v2, v2_n, 178ULL, 5);uint64_t p4=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 181ULL, 5);uint64_t p3=o_get(v2, v2_n, 179ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 183ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 181ULL, 5);uint64_t p3=o_get(v2, v2_n, 53ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 181ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 181ULL, 5);uint64_t p3=o_get(v2, v2_n, 88ULL, 5);f19(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 182ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 181ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 182ULL, 5);uint64_t p3=o_get(v2, v2_n, 92ULL, 5);f19(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 177ULL, 5);uint64_t p3=o_get(v2, v2_n, 36ULL, 5);uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k3=o_get(v2, v2_n, 8ULL, 5);
uint64_t c3=0;
for(;o_op(8,5, v3, k3);v3=o_op(1,5, v3,1)){
if(c3++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 72ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=v3;uint64_t p5=o_get(v2, v2_n, 175ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=29ULL;uint64_t p3=o_get(v2, v2_n, 72ULL, 5);uint64_t p4=o_get(v2, v2_n, 72ULL, 5);uint64_t p5=o_get(v2, v2_n, 50ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 175ULL, 5);
{uint64_t _i=175ULL;uint64_t _v=o_get(v2, v2_n, 72ULL, 5);o_put(v2, v2_n, _i,_v,5);}
({uint64_t fs_value=(({uint64_t p3=v3;f45(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=175ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
}
}
}
}else{
({uint64_t fs_value=(({uint64_t p2=184ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=185ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 8ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=552ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 184ULL, 5);uint64_t p1=o_get(v2, v2_n, 92ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=32ULL;uint64_t p3=o_get(v2, v2_n, 185ULL, 5);uint64_t p4=o_get(v2, v2_n, 96ULL, 5);uint64_t p5=o_get(v2, v2_n, 184ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 68ULL, 5);uint64_t p3=o_get(v2, v2_n, 185ULL, 5);f19(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({uint64_t p2=172ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=71ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=72ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=176ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=177ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=186ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=187ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=188ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=183ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 172ULL, 5);uint64_t p4=38ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 71ULL, 5);uint64_t p4=37ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=4ULL;uint64_t p3=o_get(v2, v2_n, 176ULL, 5);uint64_t p4=952ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=5ULL;uint64_t p3=o_get(v2, v2_n, 183ULL, 5);uint64_t p4=912ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 71ULL, 5);uint64_t p3=o_get(v2, v2_n, 71ULL, 5);uint64_t p4=o_get(v2, v2_n, 8ULL, 5);uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 177ULL, 5);uint64_t p3=o_get(v2, v2_n, 36ULL, 5);uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k3=o_get(v2, v2_n, 8ULL, 5);
uint64_t c3=0;
for(;o_op(8,5, v3, k3);v3=o_op(1,5, v3,1)){
if(c3++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 72ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=v3;uint64_t p5=o_get(v2, v2_n, 71ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=29ULL;uint64_t p3=o_get(v2, v2_n, 72ULL, 5);uint64_t p4=o_get(v2, v2_n, 72ULL, 5);uint64_t p5=o_get(v2, v2_n, 50ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 186ULL, 5);uint64_t p3=o_get(v2, v2_n, 172ULL, 5);uint64_t p4=o_get(v2, v2_n, 176ULL, 5);uint64_t p5=o_get(v2, v2_n, 72ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 187ULL, 5);uint64_t p3=o_get(v2, v2_n, 186ULL, 5);uint64_t p4=o_get(v2, v2_n, 9ULL, 5);uint64_t p5=o_get(v2, v2_n, 177ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 188ULL, 5);uint64_t p3=o_get(v2, v2_n, 187ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 183ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=552ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 184ULL, 5);uint64_t p1=o_get(v2, v2_n, ({uint64_t _a=92ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=32ULL;uint64_t p3=o_get(v2, v2_n, 185ULL, 5);uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=96ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p5=o_get(v2, v2_n, 184ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 188ULL, 5);uint64_t p3=o_get(v2, v2_n, 185ULL, 5);f19(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
}
({uint64_t fs_value=(({f30(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f47(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
({uint64_t fs_value=(({f37(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f39(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f40(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f43(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f44(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f46(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f48(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=o_get(v2, v2_n, 19ULL, 5);uint64_t _b=2ULL;o_op(4, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 28ULL, 5);uint64_t _b=v4;o_op(3, 5, _a,_b);});uint64_t _b=v3;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=136ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, 122ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 27ULL, 5);uint64_t _b=o_get(v2, v2_n, 9ULL, 5);o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=16ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
({uint64_t fs_value=(({uint64_t p2=58ULL;uint64_t p3=v6;uint64_t p4=v7;uint64_t p5=4ULL;f31(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f49(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, ({uint64_t _a=115ULL;uint64_t _b=o_get(v2, v2_n, 27ULL, 5);o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, ({uint64_t _a=134ULL;uint64_t _b=o_get(v2, v2_n, 28ULL, 5);o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, ({uint64_t _a=136ULL;uint64_t _b=o_get(v2, v2_n, 28ULL, 5);o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v7=({uint64_t fs_value=(({uint64_t p0=v4;uint64_t p1=v6;uint64_t p2=v5;uint64_t p3=v4;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=v7;uint64_t p3=v3;uint64_t p4=v3;f32(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}else{
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 28ULL, 5);uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t _b=v3;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 120ULL, 5);uint64_t p1=v5;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=8ULL;uint64_t p3=v7;uint64_t p4=65536ULL;uint64_t p5=({uint64_t _a=v3;uint64_t _b=24ULL;o_op(15, 5, _a,_b);});f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
v7=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 121ULL, 5);uint64_t p1=v5;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=18ULL;uint64_t p3=v7;uint64_t p4=4294901760ULL;uint64_t p5=({uint64_t _a=v3;uint64_t _b=24ULL;o_op(15, 5, _a,_b);});f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
uint64_t __attribute__((unused)) v8=o_get(v2, v2_n, ({uint64_t _a=136ULL;uint64_t _b=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(4, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);
v7=({uint64_t fs_value=(({uint64_t p0=v4;uint64_t p1=v8;uint64_t p2=o_get(v2, v2_n, 120ULL, 5);uint64_t p3=v4;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=v7;uint64_t p3=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(5, 5, _a,_b);});uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t p4=0ULL;f32(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
v7=({uint64_t fs_value=(({uint64_t p0=v4;uint64_t p1=v8;uint64_t p2=o_get(v2, v2_n, 121ULL, 5);uint64_t p3=v4;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=v7;uint64_t p3=({uint64_t _a=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(5, 5, _a,_b);});uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p4=0ULL;f32(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f50(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{
uint64_t v3=0;
uint64_t k1=({uint64_t _a=o_get(v2, v2_n, 19ULL, 5);uint64_t _b=2ULL;o_op(4, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p3=v3;f48(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v3=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p3=v3;f49(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f51(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, ({uint64_t _a=92ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, ({uint64_t _a=147ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, ({uint64_t _a=96ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v7=({uint64_t fs_value=(({uint64_t p0=v4;uint64_t p1=v4;uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, 154ULL, 5);f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=33ULL;uint64_t p3=v7;uint64_t p4=0ULL;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=32ULL;uint64_t p3=v6;uint64_t p4=v6;uint64_t p5=v5;f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f52(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, ({uint64_t _a=96ULL;uint64_t _b=o_get(v2, v2_n, 27ULL, 5);o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, ({uint64_t _a=161ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(4, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v7=({uint64_t fs_value=(({uint64_t p0=v4;uint64_t p1=v5;uint64_t p2=v6;uint64_t p3=v4;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=v7;uint64_t p3=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(5, 5, _a,_b);});uint64_t p4=0ULL;f32(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f53(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=2ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=320ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
{
uint64_t v3=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=32ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=36ULL;uint64_t p3=46ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=775ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=0ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=856ULL;uint64_t p5=1ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=5ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=32ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=({uint64_t _a=896ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}
}
{
uint64_t v3=0;
uint64_t k1=7ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=4ULL;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=39ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=({uint64_t _a=928ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 36ULL, 5);uint64_t p4=33ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 37ULL, 5);uint64_t p4=37ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=3ULL;uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=38ULL;f11(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=50ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=58ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=128ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=129ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=53ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=54ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=55ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=56ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=76ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 50ULL, 5);uint64_t p3=1ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 58ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 128ULL, 5);uint64_t p3=4294967295ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 129ULL, 5);uint64_t p3=4286578688ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 53ULL, 5);uint64_t p3=o_get(v2, v2_n, 22ULL, 5);f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 54ULL, 5);uint64_t p3=o_get(v2, v2_n, 10ULL, 5);f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 55ULL, 5);uint64_t p3=o_get(v2, v2_n, 11ULL, 5);f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 56ULL, 5);uint64_t p3=o_get(v2, v2_n, 12ULL, 5);f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=194ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=84ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=125ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=195ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 76ULL, 5);uint64_t p3=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 84ULL, 5);uint64_t p3=o_get(v2, v2_n, 36ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 76ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 125ULL, 5);uint64_t p3=o_get(v2, v2_n, 84ULL, 5);uint64_t p4=31ULL;f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 194ULL, 5);uint64_t p3=o_get(v2, v2_n, 125ULL, 5);uint64_t p4=o_get(v2, v2_n, 84ULL, 5);uint64_t p5=o_get(v2, v2_n, 36ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 195ULL, 5);uint64_t p3=o_get(v2, v2_n, 194ULL, 5);uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=64ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=65ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=66ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=67ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=68ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 64ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=o_get(v2, v2_n, 45ULL, 5);uint64_t p5=o_get(v2, v2_n, 37ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 65ULL, 5);uint64_t p3=o_get(v2, v2_n, 64ULL, 5);uint64_t p4=({uint64_t _a=o_get(v2, v2_n, 1ULL, 5);uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 195ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 66ULL, 5);uint64_t p3=o_get(v2, v2_n, 65ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 32ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 67ULL, 5);uint64_t p3=o_get(v2, v2_n, 66ULL, 5);f18(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 195ULL, 5);uint64_t p3=o_get(v2, v2_n, 67ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 68ULL, 5);uint64_t p3=o_get(v2, v2_n, 65ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 34ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=142ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=120ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=121ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=196ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=77ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=75ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 76ULL, 5);uint64_t p3=o_get(v2, v2_n, 38ULL, 5);uint64_t p4=16ULL;uint64_t p5=o_get(v2, v2_n, 39ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 142ULL, 5);uint64_t p3=o_get(v2, v2_n, 76ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 35ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 120ULL, 5);uint64_t p3=o_get(v2, v2_n, 142ULL, 5);f18(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 142ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 142ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 121ULL, 5);uint64_t p3=o_get(v2, v2_n, 142ULL, 5);f18(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 142ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 142ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 196ULL, 5);uint64_t p3=o_get(v2, v2_n, 142ULL, 5);f18(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 77ULL, 5);uint64_t p3=o_get(v2, v2_n, 196ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 35ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 76ULL, 5);uint64_t p3=o_get(v2, v2_n, 37ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 44ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 142ULL, 5);uint64_t p3=o_get(v2, v2_n, 76ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 35ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 75ULL, 5);uint64_t p3=o_get(v2, v2_n, 142ULL, 5);f18(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=82ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=85ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 82ULL, 5);uint64_t p3=o_get(v2, v2_n, 121ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 120ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 85ULL, 5);uint64_t p3=o_get(v2, v2_n, 82ULL, 5);uint64_t p4=31ULL;f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 121ULL, 5);uint64_t p3=o_get(v2, v2_n, 85ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 121ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=80ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 80ULL, 5);uint64_t p3=o_get(v2, v2_n, 194ULL, 5);uint64_t p4=o_get(v2, v2_n, 25ULL, 5);uint64_t p5=o_get(v2, v2_n, 33ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 80ULL, 5);uint64_t p3=o_get(v2, v2_n, 43ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 80ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=88ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=92ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=96ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=100ULL;uint64_t p3=104ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=63ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=127ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 88ULL, 5);uint64_t p3=4286578688ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 92ULL, 5);uint64_t p3=1065353216ULL;f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 96ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 100ULL, 5);uint64_t p4=o_get(v2, v2_n, 120ULL, 5);uint64_t p5=o_get(v2, v2_n, 58ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 63ULL, 5);uint64_t p3=o_get(v2, v2_n, 36ULL, 5);uint64_t p4=4ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 101ULL, 5);uint64_t p4=o_get(v2, v2_n, 63ULL, 5);uint64_t p5=o_get(v2, v2_n, 54ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 102ULL, 5);uint64_t p4=o_get(v2, v2_n, 63ULL, 5);uint64_t p5=o_get(v2, v2_n, 55ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 103ULL, 5);uint64_t p4=o_get(v2, v2_n, 63ULL, 5);uint64_t p5=o_get(v2, v2_n, 56ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 127ULL, 5);uint64_t p4=o_get(v2, v2_n, 121ULL, 5);uint64_t p5=o_get(v2, v2_n, 128ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f22(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 120ULL, 5);uint64_t p3=o_get(v2, v2_n, 121ULL, 5);f24(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=189ULL;uint64_t _v=({uint64_t fs_value=(({f26(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=16ULL;uint64_t _v=o_get(v2, v2_n, 121ULL, 5);o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f54(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
{
uint64_t v4=0;
uint64_t c1=0;
for(;o_op(8,5, v4, v3);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 114ULL, 5);
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=2825ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 163ULL, 5);uint64_t p1=o_get(v2, v2_n, 50ULL, 5);uint64_t p2=65535ULL;uint64_t p3=o_get(v2, v2_n, 114ULL, 5);f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=({uint64_t _a=v4;uint64_t _b=o_get(v2, v2_n, 25ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
v5=o_get(v2, v2_n, 163ULL, 5);
}
uint64_t __attribute__((unused)) v6=19ULL;
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v6=11ULL;
}
({uint64_t fs_value=(({uint64_t p2=v6;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);uint64_t p1=v5;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f31(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f55(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
({uint64_t fs_value=(({uint64_t p2=104ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=124ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=125ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=126ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=110ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=111ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=112ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=113ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=109ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=114ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=115ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=105ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=130ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=131ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=163ULL;uint64_t p3=2ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
}
if(({uint64_t _a=o_get(v2, v2_n, 21ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=122ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=119ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=197ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=136ULL;uint64_t p3=4ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 197ULL, 5);uint64_t p3=({uint64_t _a=o_get(v2, v2_n, 21ULL, 5);uint64_t _b=4ULL;o_op(3, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
if(({uint64_t _a=o_get(v2, v2_n, 20ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=198ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({f28(fs, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=191ULL;uint64_t _v=o_get(v1, v1_n, 0ULL, 5);o_put(v2, v2_n, _i,_v,5);}
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 104ULL, 5);uint64_t p4=o_get(v2, v2_n, 100ULL, 5);uint64_t p5=o_get(v2, v2_n, 36ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 124ULL, 5);uint64_t p3=o_get(v2, v2_n, 104ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 127ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 125ULL, 5);uint64_t p3=o_get(v2, v2_n, 124ULL, 5);uint64_t p4=31ULL;f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 126ULL, 5);uint64_t p3=o_get(v2, v2_n, 125ULL, 5);uint64_t p4=o_get(v2, v2_n, 36ULL, 5);uint64_t p5=65535ULL;f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 104ULL, 5);uint64_t p3=o_get(v2, v2_n, 126ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 104ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 110ULL, 5);uint64_t p3=o_get(v2, v2_n, 104ULL, 5);uint64_t p4=o_get(v2, v2_n, 2ULL, 5);f16(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 111ULL, 5);uint64_t p3=o_get(v2, v2_n, 104ULL, 5);uint64_t p4=o_get(v2, v2_n, 26ULL, 5);f17(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 112ULL, 5);uint64_t p3=o_get(v2, v2_n, 110ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 77ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 113ULL, 5);uint64_t p3=o_get(v2, v2_n, 112ULL, 5);f18(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 109ULL, 5);uint64_t p3=o_get(v2, v2_n, 113ULL, 5);uint64_t p4=o_get(v2, v2_n, 41ULL, 5);uint64_t p5=65535ULL;f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 109ULL, 5);uint64_t p3=o_get(v2, v2_n, 111ULL, 5);uint64_t p4=o_get(v2, v2_n, 42ULL, 5);uint64_t p5=o_get(v2, v2_n, 109ULL, 5);f13(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=14ULL;uint64_t p3=o_get(v2, v2_n, 109ULL, 5);uint64_t p4=o_get(v2, v2_n, 109ULL, 5);uint64_t p5=o_get(v2, v2_n, 75ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 114ULL, 5);uint64_t p3=o_get(v2, v2_n, 109ULL, 5);uint64_t p4=1ULL;uint64_t p5=o_get(v2, v2_n, 33ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 115ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 21ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 119ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 122ULL, 5);f10(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f28(fs, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
{uint64_t _i=192ULL;uint64_t _v=o_get(v1, v1_n, 0ULL, 5);o_put(v2, v2_n, _i,_v,5);}
({uint64_t fs_value=(({uint64_t p3=4ULL;f54(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=58ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 136ULL, 5);uint64_t p1=o_get(v2, v2_n, 122ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=4ULL;f31(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k2=4ULL;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=65536ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 115ULL, 5);uint64_t p1=o_get(v2, v2_n, 136ULL, 5);uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, 115ULL, 5);f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p3=v3;uint64_t p4=0ULL;f32(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 114ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=({uint64_t _a=4ULL;uint64_t _b=o_get(v2, v2_n, 25ULL, 5);o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 114ULL, 5);f14(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 122ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=16ULL;uint64_t p5=o_get(v2, v2_n, 122ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 119ULL, 5);uint64_t p3=o_get(v2, v2_n, 50ULL, 5);uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 119ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 119ULL, 5);uint64_t p3=o_get(v2, v2_n, 197ULL, 5);f24(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 192ULL, 5);f25(fs, v0, v0_n, v1, v1_n, p2);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f29(fs, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
}
if(({uint64_t _a=o_get(v2, v2_n, 4ULL, 5);uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p3=o_get(v2, v2_n, 20ULL, 5);f54(fs, v0, v0_n, v1, v1_n, v2, v2_n, p3);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k2=o_get(v2, v2_n, 20ULL, 5);
uint64_t c2=0;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
if(c2++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 76ULL, 5);uint64_t p3=({uint64_t _a=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 21ULL, 5);uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 198ULL, 5);uint64_t p3=o_get(v2, v2_n, 76ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v2, v2_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p4=65536ULL;uint64_t p5=65535ULL;f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
({uint64_t fs_value=(({uint64_t p2=33ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 115ULL, 5);uint64_t p1=o_get(v2, v2_n, 198ULL, 5);uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=164ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, 115ULL, 5);f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
}
}
}
({uint64_t fs_value=(({uint64_t p2=32ULL;uint64_t p3=o_get(v2, v2_n, 105ULL, 5);uint64_t p4=o_get(v2, v2_n, 115ULL, 5);uint64_t p5=o_get(v2, v2_n, 40ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 130ULL, 5);uint64_t p3=o_get(v2, v2_n, 125ULL, 5);uint64_t p4=2147483648ULL;uint64_t p5=o_get(v2, v2_n, 129ULL, 5);f12(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=31ULL;uint64_t p3=o_get(v2, v2_n, 131ULL, 5);uint64_t p4=o_get(v2, v2_n, 58ULL, 5);uint64_t p5=o_get(v2, v2_n, 105ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, 131ULL, 5);uint64_t p4=o_get(v2, v2_n, 131ULL, 5);uint64_t p5=o_get(v2, v2_n, 130ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=31ULL;uint64_t p3=o_get(v2, v2_n, 105ULL, 5);uint64_t p4=o_get(v2, v2_n, 58ULL, 5);uint64_t p5=o_get(v2, v2_n, 131ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 101ULL, 5);uint64_t p3=o_get(v2, v2_n, 105ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 102ULL, 5);uint64_t p3=o_get(v2, v2_n, 109ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f56(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
({uint64_t fs_value=(({uint64_t p2=140ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=141ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=200ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=144ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=145ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=146ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=147ULL;uint64_t p3=1ULL;f4(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=151ULL;uint64_t p3=155ULL;f5(fs, v2, v2_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 140ULL, 5);uint64_t p3=o_get(v2, v2_n, 105ULL, 5);uint64_t p4=16ULL;f23(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, 141ULL, 5);uint64_t p4=o_get(v2, v2_n, 105ULL, 5);uint64_t p5=o_get(v2, v2_n, 140ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 140ULL, 5);uint64_t p3=o_get(v2, v2_n, 141ULL, 5);uint64_t p4=({uint64_t _a=8ULL;uint64_t _b=v3;o_op(16, 5, _a,_b);});f23(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, 141ULL, 5);uint64_t p4=o_get(v2, v2_n, 141ULL, 5);uint64_t p5=o_get(v2, v2_n, 140ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 103ULL, 5);uint64_t p3=o_get(v2, v2_n, 141ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f22(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 144ULL, 5);uint64_t p3=o_get(v2, v2_n, 56ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=({uint64_t _a=o_get(v2, v2_n, 23ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 76ULL, 5);uint64_t p3=({uint64_t _a=o_get(v2, v2_n, 12ULL, 5);uint64_t _b=({uint64_t _a=128ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 200ULL, 5);uint64_t p3=o_get(v2, v2_n, 76ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, 144ULL, 5);uint64_t p4=o_get(v2, v2_n, 144ULL, 5);uint64_t p5=o_get(v2, v2_n, 200ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, 145ULL, 5);uint64_t p4=o_get(v2, v2_n, 88ULL, 5);uint64_t p5=o_get(v2, v2_n, 144ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=31ULL;uint64_t p3=o_get(v2, v2_n, 146ULL, 5);uint64_t p4=o_get(v2, v2_n, 88ULL, 5);uint64_t p5=o_get(v2, v2_n, 145ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=559ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 147ULL, 5);uint64_t p1=o_get(v2, v2_n, 146ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=37ULL;uint64_t p3=o_get(v2, v2_n, 88ULL, 5);uint64_t p4=o_get(v2, v2_n, 145ULL, 5);uint64_t p5=o_get(v2, v2_n, 144ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=31ULL;uint64_t p3=o_get(v2, v2_n, 151ULL, 5);uint64_t p4=o_get(v2, v2_n, 105ULL, 5);uint64_t p5=o_get(v2, v2_n, 145ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=559ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 152ULL, 5);uint64_t p1=o_get(v2, v2_n, 151ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;uint64_t p5=0ULL;f7(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f22(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 101ULL, 5);uint64_t p3=o_get(v2, v2_n, 152ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 140ULL, 5);uint64_t p3=o_get(v2, v2_n, 152ULL, 5);uint64_t p4=16ULL;f23(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=30ULL;uint64_t p3=o_get(v2, v2_n, 153ULL, 5);uint64_t p4=o_get(v2, v2_n, 152ULL, 5);uint64_t p5=o_get(v2, v2_n, 140ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 140ULL, 5);uint64_t p3=o_get(v2, v2_n, 153ULL, 5);uint64_t p4=({uint64_t _a=8ULL;uint64_t _b=v3;o_op(16, 5, _a,_b);});f23(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=30ULL;uint64_t p3=o_get(v2, v2_n, 153ULL, 5);uint64_t p4=o_get(v2, v2_n, 153ULL, 5);uint64_t p5=o_get(v2, v2_n, 140ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 103ULL, 5);uint64_t p3=o_get(v2, v2_n, 153ULL, 5);f21(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f22(fs, v0, v0_n, v1, v1_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 154ULL, 5);uint64_t p3=o_get(v2, v2_n, 56ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
{
uint64_t v3=0;
uint64_t k1=({uint64_t _a=o_get(v2, v2_n, 23ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t c1=0;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 76ULL, 5);uint64_t p3=({uint64_t _a=o_get(v2, v2_n, 12ULL, 5);uint64_t _b=({uint64_t _a=128ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f9(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=o_get(v2, v2_n, 200ULL, 5);uint64_t p3=o_get(v2, v2_n, 76ULL, 5);f20(fs, v0, v0_n, v1, v1_n, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=30ULL;uint64_t p3=o_get(v2, v2_n, 154ULL, 5);uint64_t p4=o_get(v2, v2_n, 154ULL, 5);uint64_t p5=o_get(v2, v2_n, 200ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
}
}
({uint64_t fs_value=(({uint64_t p2=33ULL;uint64_t p3=({uint64_t fs_value=(({uint64_t p0=o_get(v2, v2_n, 92ULL, 5);uint64_t p1=o_get(v2, v2_n, 92ULL, 5);uint64_t p2=o_get(v2, v2_n, 147ULL, 5);uint64_t p3=o_get(v2, v2_n, 154ULL, 5);f1(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t p4=0ULL;f8(fs, v0, v0_n, v1, v1_n, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({uint64_t p2=32ULL;uint64_t p3=o_get(v2, v2_n, 96ULL, 5);uint64_t p4=o_get(v2, v2_n, 96ULL, 5);uint64_t p5=o_get(v2, v2_n, 147ULL, 5);f15(fs, v0, v0_n, v1, v1_n, p2, p3, p4, p5);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f57(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
({uint64_t fs_value=(({f53(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f55(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f56(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f44(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
({uint64_t fs_value=(({f46(fs, v0, v0_n, v1, v1_n, v2, v2_n);}));if(fs->code)goto fs_fail;fs_value;});
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f58(struct ombr_fs_ctx *fs, uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=31ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=31ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=2ULL;o_op(16, 5, _a,_b);});uint64_t _b=3ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v0;uint64_t _b=40ULL;o_op(16, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=7696581459727ULL;o_op(12, 5, _a,_b);});uint64_t _b=v0;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v7;uint64_t _b=5ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=64ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=128ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=6ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
}else{
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=256ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v6;uint64_t _b=5ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
}
return o_leave(0ULL);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f59(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=v4;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(16, 5, _a,_b);});uint64_t _b=3ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=40ULL;o_op(16, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=12ULL;o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=15ULL;uint64_t _v=6ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=17ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=18ULL;uint64_t _v=16ULL;o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=18ULL;uint64_t _v=8ULL;o_put(v0, v0_n, _i,_v,5);}
}
{uint64_t _i=19ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 18ULL, 5);uint64_t _b=4ULL;o_op(4, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(5, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=21ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(4, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=22ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=31ULL;o_op(1, 5, _a,_b);});uint64_t _b=32ULL;o_op(4, 5, _a,_b);});uint64_t _b=32ULL;o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=23ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 22ULL, 5);uint64_t _b=32ULL;o_op(4, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=24ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=25ULL;uint64_t _v=2ULL;o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=25ULL;uint64_t _v=4ULL;o_put(v0, v0_n, _i,_v,5);}
}
{uint64_t _i=26ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=o_get(v0, v0_n, 7ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=128ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=15ULL;uint64_t _v=7ULL;o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v5=v4;
if(({uint64_t _a=v5;uint64_t _b=2ULL;o_op(10, 5, _a,_b);})){
v5=2ULL;
}
{uint64_t _i=24ULL;uint64_t _v=v5;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=({uint64_t _a=1ULL;uint64_t _b=v5;o_op(15, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
}
{uint64_t _i=10ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 8ULL, 5);uint64_t _b=o_get(v0, v0_n, 9ULL, 5);o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({uint64_t _a=2ULL;uint64_t _b=o_get(v0, v0_n, 10ULL, 5);o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 11ULL, 5);uint64_t _b=o_get(v0, v0_n, 9ULL, 5);o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=o_get(v0, v0_n, 6ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
{uint64_t _i=17ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 6ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=13ULL;uint64_t _v=({uint64_t _a=1ULL;uint64_t _b=o_get(v0, v0_n, 14ULL, 5);o_op(15, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
}
}else{
{uint64_t _i=10ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 22ULL, 5);uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=11ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 22ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=12ULL;uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 22ULL, 5);uint64_t _b=12ULL;o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
}
return o_leave(0);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f60(struct ombr_fs_ctx *fs, void *v0, uint64_t v0_n){
o_enter();
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, 6ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 41ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(2ULL);
}
{uint64_t _i=6ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
static uint64_t f61(struct ombr_fs_ctx *fs, uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n){
o_enter();
{
uint64_t v6=0;
uint64_t k1=512ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
{uint64_t _i=v6;uint64_t _v=0ULL;o_put(v5, v5_n, _i,_v,5);}
}
}
if(({uint64_t _a=({uint64_t fs_value=(({uint64_t p0=v0;uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;f58(fs, p0, p1, p2, p3);}));if(fs->code)goto fs_fail;fs_value;});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t v6[256];
uint64_t v6_n=256;
o_fill(v6, v6_n, 65535ULL, 5);
({uint64_t fs_value=(({uint64_t p1=v0;uint64_t p2=v1;uint64_t p3=v2;uint64_t p4=v3;f59(fs, v6, v6_n, p1, p2, p3, p4);}));if(fs->code)goto fs_fail;fs_value;});
if(({uint64_t _a=o_get(v6, v6_n, 7ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t fs_value=(({f47(fs, v4, v4_n, v5, v5_n, v6, v6_n);}));if(fs->code)goto fs_fail;fs_value;});
}else{
({uint64_t fs_value=(({f57(fs, v4, v4_n, v5, v5_n, v6, v6_n);}));if(fs->code)goto fs_fail;fs_value;});
}
return o_leave(({uint64_t fs_value=(({f60(fs, v5, v5_n);}));if(fs->code)goto fs_fail;fs_value;}));
fs_fail: __attribute__((unused));--fs->depth;return 0;
}
uint32_t omega_attention_ir(uint64_t  *result, volatile uint64_t p0, volatile uint64_t p1, volatile uint64_t p2, volatile uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n){
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
  struct ombr_fs_ctx fs_state={0,0};struct ombr_fs_ctx *fs=&fs_state;uint64_t _result=f61(fs, p0, p1, p2, p3, p4, p4_n, p5, p5_n);if(!fs->code)fs_put(fs,result,1,0,_result,5);return fs->code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
