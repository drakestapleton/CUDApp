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
static uint64_t f0(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f1(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f2(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f3(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f4(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f5(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f6(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f7(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f8(uint64_t v0);
static uint64_t f9(uint64_t v0);
static uint64_t f10(uint64_t v0, uint64_t v1);
static uint64_t f11(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f12(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f13(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f14(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f15(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f16(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f17(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f18(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f19(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f20(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f21(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f22(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f23(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f24(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f25(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f26(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f27(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f28(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f29(uint64_t v0);
static uint64_t f30(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f31(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f32(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f33(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f34(uint64_t v0);
static uint64_t f35(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f36(uint64_t v0);
static uint64_t f37(uint64_t v0);
static uint64_t f38(void *v0, uint64_t v0_n);
static uint64_t f39(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f40(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f41(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f42(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f43(void *v0, uint64_t v0_n);
static uint64_t f44(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f45(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f46(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f47(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f48(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f49(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f50(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f51(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f52(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f53(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f54(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f55(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f56(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f57(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f58(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f59(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f60(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f61(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f62(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f63(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f64(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f65(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f66(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f67(void *v0, uint64_t v0_n);
static uint64_t f68(const void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n);
static uint64_t f0(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=118ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=98ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=11ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=98ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=9ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=10ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=98ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=102ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=102ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=103ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=118ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=118ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f1(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=9ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=95ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=98ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=118ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=107ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=10ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=118ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=95ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=9ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=24ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=49ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f2(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=25ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=26ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=107ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=27ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=28ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=29ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=30ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=31ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=103ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=32ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=95ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=33ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=98ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=34ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=98ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=119ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=35ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=9ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=95ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=36ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f3(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=37ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=77ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=38ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=78ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=39ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=40ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=41ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=102ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=42ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=43ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=44ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=10ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=9ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=45ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=98ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=46ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=106ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=47ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=119ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=95ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=119ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=48ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f4(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=49ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=102ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=50ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=102ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=51ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=107ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=119ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=52ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=9ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=53ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=102ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=54ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=55ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=9ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=102ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=56ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=57ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=9ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=98ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=119ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=58ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=59ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=60ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f5(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=61ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=118ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=62ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=98ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=63ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=103ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=64ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=65ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=10ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=9ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=66ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=67ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=13ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=99ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=103ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=9ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=95ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=10ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=11ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=12ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=68ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=102ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=69ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=109ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=118ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=70ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=115ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=104ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=97ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=114ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=100ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=71ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=108ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=107ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=117ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=112ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
if(({uint64_t _a=v3;uint64_t _b=72ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=111ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=101ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f6(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=73ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=0ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=49ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f7(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=12ULL;o_op(9, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;f0(v0, v0_n, p1, p2, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=24ULL;o_op(9, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;f1(v0, v0_n, p1, p2, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=36ULL;o_op(9, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;f2(v0, v0_n, p1, p2, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=48ULL;o_op(9, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;f3(v0, v0_n, p1, p2, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=60ULL;o_op(9, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;f4(v0, v0_n, p1, p2, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=72ULL;o_op(9, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;f5(v0, v0_n, p1, p2, p3);}));
}
if(({uint64_t _a=v3;uint64_t _b=73ULL;o_op(9, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;f6(v0, v0_n, p1, p2, p3);}));
}
return o_leave(0ULL);
}
static uint64_t f8(uint64_t v0){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=32ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);}));
}
static uint64_t f9(uint64_t v0){
o_enter();
return o_leave(({uint64_t _a=({uint64_t p0=v0;f8(p0);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);}));
}
static uint64_t f10(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
return o_leave(v0);
}
return o_leave(v1);
}
static uint64_t f11(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
{uint64_t _i=14012ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=o_get(v1, v1_n, 14006ULL, 5);o_op(8, 5, _a,_b);})){
if(c1++>=24)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t _a=14100ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=5ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t p1=o_get(v1, v1_n, v4, 5);uint64_t p2=o_get(v1, v1_n, ({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p3=v2;f7(v0, v0_n, p1, p2, p3);})){
{uint64_t _i=({uint64_t _a=v4;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14010ULL;uint64_t _v=o_get(v1, v1_n, ({uint64_t _a=v4;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14011ULL;uint64_t _v=o_get(v1, v1_n, ({uint64_t _a=v4;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14012ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0ULL);
}
static uint64_t f12(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
return o_leave(({uint64_t p1=o_get(v1, v1_n, 14010ULL, 5);uint64_t p2=o_get(v1, v1_n, 14011ULL, 5);uint64_t p3=v2;f7(v0, v0_n, p1, p2, p3);}));
}
static uint64_t f13(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
{uint64_t _i=v1;uint64_t _v=o_get(v0, v0_n, 14010ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=o_get(v0, v0_n, 14011ULL, 5);uint64_t p1=v2;f10(p0, p1);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f14(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=({uint64_t p2=v2;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
({uint64_t p1=v3;uint64_t p2=v4;f13(v1, v1_n, p1, p2);});
return o_leave(1ULL);
}
static uint64_t f15(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=v4;o_op(7, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=1024)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);uint64_t _b=o_get(v0, v0_n, ({uint64_t _a=v3;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(1ULL);
}
static uint64_t f16(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=64ULL;
uint64_t __attribute__((unused)) v6=12ULL;
uint64_t __attribute__((unused)) v7=o_get(v1, v1_n, 11ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v5=448ULL;
v6=26ULL;
v7=o_get(v1, v1_n, 12ULL, 5);
}
uint64_t __attribute__((unused)) v8=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v8;uint64_t _b=v7;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v9=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=v8;uint64_t _b=v6;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t p1=v3;uint64_t p2=v4;uint64_t p3=o_get(v1, v1_n, v9, 5);uint64_t p4=o_get(v1, v1_n, ({uint64_t _a=v9;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);f15(v0, v0_n, p1, p2, p3, p4);})){
return o_leave(v8);
}
v8=({uint64_t _a=v8;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(4294967295ULL);
}
static uint64_t f17(const void *v0, uint64_t v0_n, const void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
return o_leave(({uint64_t p2=v2;uint64_t p3=o_get(v1, v1_n, 14010ULL, 5);uint64_t p4=o_get(v1, v1_n, 14011ULL, 5);f16(v0, v0_n, v1, v1_n, p2, p3, p4);}));
}
static uint64_t f18(uint64_t v0, uint64_t v1, uint64_t v2){
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
static uint64_t f19(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=0ULL;
uint64_t __attribute__((unused)) v5=0ULL;
uint64_t __attribute__((unused)) v6=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=128)o_trap(4);
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);f9(p0);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
break;
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=v7;uint64_t _b=43ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);})){
v6=({uint64_t _a=v7;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v7=18446744073709551615ULL;
if(({uint64_t _a=v3;uint64_t _b=0;o_op(18, 1, _a,_b);})){
v7=({uint64_t _a=9223372036854775808ULL;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
if(v6){
v7=9223372036854775808ULL;
}
}
uint64_t __attribute__((unused)) v8=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=128)o_trap(4);
uint64_t __attribute__((unused)) v9=o_cast(5, 2, o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2));
if(({uint64_t _a=({uint64_t _a=v9;uint64_t _b=48ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v9;uint64_t _b=57ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
break;
}
v9=({uint64_t _a=v9;uint64_t _b=48ULL;o_op(2, 5, _a,_b);});
if(({uint64_t p0=v5;uint64_t p1=v7;uint64_t p2=v9;f18(p0, p1, p2);})){
v8=1ULL;
}
if(({uint64_t _a=v8;uint64_t _b=0;o_op(18, 1, _a,_b);})){
v5=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=10ULL;o_op(3, 5, _a,_b);});uint64_t _b=v9;o_op(1, 5, _a,_b);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(v8){
if(v3){
return o_leave(4294967295ULL);
}
v5=v7;
}
if(v6){
v5=({uint64_t _a=0ULL;uint64_t _b=v5;o_op(2, 5, _a,_b);});
}
return o_leave(({uint64_t _a=v5;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));
}
static uint64_t f20(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=({uint64_t p2=v2;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
{uint64_t _i=v3;uint64_t _v=({uint64_t p1=o_get(v1, v1_n, 14010ULL, 5);uint64_t p2=o_get(v1, v1_n, 14011ULL, 5);uint64_t p3=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=37ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=40ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);});f19(v0, v0_n, p1, p2, p3);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
static uint64_t f21(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 14004ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 14005ULL, 5);
{
uint64_t v4=0;
uint64_t k1=11ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});f7(v0, v0_n, p1, p2, p3);})){
return o_leave(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));
}
}
}
return o_leave(0ULL);
}
static uint64_t f22(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
{uint64_t _i=14006ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14007ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v4=1ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=1024)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, v2, 2);uint64_t _b=35ULL;o_op(6, 2, _a,_b);})){
break;
}
if(({uint64_t p0=o_get(v0, v0_n, v2, 2);f8(p0);})){
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
continue;
}
uint64_t __attribute__((unused)) v5=v2;
uint64_t __attribute__((unused)) v6=v3;
{
uint64_t c2=0;
while(({uint64_t _a=v2;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c2++>=1024)o_trap(4);
uint64_t __attribute__((unused)) v7=o_get(v0, v0_n, v2, 2);
if(({uint64_t _a=({uint64_t p0=v7;f8(p0);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=35ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);})){
break;
}
if(({uint64_t _a=({uint64_t _a=v7;uint64_t _b=61ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=v3;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
v6=v2;
}
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v7=({uint64_t _a=v2;uint64_t _b=v5;o_op(2, 5, _a,_b);});
if(v4){
{uint64_t _i=14004ULL;uint64_t _v=v5;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14005ULL;uint64_t _v=({uint64_t p0=v7;uint64_t p1=31ULL;f10(p0, p1);});o_put(v1, v1_n, _i,_v,5);}
v4=0ULL;
}else{
if(({uint64_t _a=v6;uint64_t _b=v3;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v8=o_get(v1, v1_n, 14007ULL, 5);
if(({uint64_t _a=v8;uint64_t _b=8ULL;o_op(8, 5, _a,_b);})){
uint64_t __attribute__((unused)) v9=({uint64_t _a=14220ULL;uint64_t _b=({uint64_t _a=v8;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{uint64_t _i=v9;uint64_t _v=v5;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v9;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=v7;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14007ULL;uint64_t _v=({uint64_t _a=v8;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}
}else{
uint64_t __attribute__((unused)) v8=o_get(v1, v1_n, 14006ULL, 5);
if(({uint64_t _a=v8;uint64_t _b=24ULL;o_op(11, 5, _a,_b);})){
return o_leave(2ULL);
}
uint64_t __attribute__((unused)) v9=({uint64_t _a=14100ULL;uint64_t _b=({uint64_t _a=v8;uint64_t _b=5ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{uint64_t _i=v9;uint64_t _v=v5;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v9;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=v6;uint64_t _b=v5;o_op(2, 5, _a,_b);});uint64_t p1=31ULL;f10(p0, p1);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v9;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v9;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_op(2, 5, _a,_b);});uint64_t p1=127ULL;f10(p0, p1);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v9;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14006ULL;uint64_t _v=({uint64_t _a=v8;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}
}
}
}
if(v4){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f23(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v1, 2);uint64_t _b=32ULL;o_op(13, 2, _a,_b);});uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=32ULL;o_op(13, 2, _a,_b);});uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=32ULL;o_op(13, 2, _a,_b);});uint64_t _b=102ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=32ULL;o_op(13, 2, _a,_b);});uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=32ULL;o_op(13, 2, _a,_b);});uint64_t _b=110ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=32ULL;o_op(13, 2, _a,_b);});uint64_t _b=105ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=32ULL;o_op(13, 2, _a,_b);});uint64_t _b=116ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 2);uint64_t _b=32ULL;o_op(13, 2, _a,_b);});uint64_t _b=121ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
}
return o_leave(1ULL);
}
static uint64_t f24(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
{
uint64_t v2=0;
uint64_t k1=64ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
}
}
return o_leave(0);
}
static uint64_t f25(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=v3;
{
uint64_t v5=0;
uint64_t k1=64ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
uint64_t __attribute__((unused)) v6=({uint64_t _a=v1;uint64_t _b=v5;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v6, 5);uint64_t _b=v2;o_op(3, 5, _a,_b);});uint64_t _b=v4;o_op(1, 5, _a,_b);});
{uint64_t _i=v6;uint64_t _v=({uint64_t _a=v7;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
v4=({uint64_t _a=v7;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f26(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=64ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c1++>=64)o_trap(4);
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=v4;uint64_t _b=v5;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v4;uint64_t _b=v5;o_op(8, 5, _a,_b);})){
return o_leave(18446744073709551615ULL);
}
}
}
return o_leave(0ULL);
}
static uint64_t f27(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t v4=0;
uint64_t k1=64ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=({uint64_t _a=v1;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, v5, 5);
uint64_t __attribute__((unused)) v7=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);uint64_t _b=v3;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v6;uint64_t _b=v7;o_op(2, 5, _a,_b);});
{uint64_t _i=v5;uint64_t _v=({uint64_t _a=v8;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
v3=0ULL;
if(({uint64_t _a=v6;uint64_t _b=v7;o_op(8, 5, _a,_b);})){
v3=1ULL;
}
}
}
return o_leave(0);
}
static uint64_t f28(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
if(({uint64_t _a=({uint64_t p1=14400ULL;uint64_t p2=14600ULL;f26(v0, v0_n, p1, p2);});uint64_t _b=0ULL;o_op(6, 9, _a,_b);})){
if(v1){
return o_leave(9223372036854775808ULL);
}
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v2=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t p1=14400ULL;uint64_t p2=14500ULL;f26(v0, v0_n, p1, p2);});uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
if(c1++>=2048)o_trap(4);
({uint64_t p1=14400ULL;uint64_t p2=2ULL;uint64_t p3=0ULL;f25(v0, v0_n, p1, p2, p3);});
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 9, _a,_b);});
}
}
{
uint64_t c1=0;
while(1ULL){
if(c1++>=2048)o_trap(4);
{
uint64_t v3=0;
uint64_t k2=64ULL;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
{uint64_t _i=({uint64_t _a=14600ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=14500ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);o_put(v0, v0_n, _i,_v,5);}
}
}
({uint64_t p1=14600ULL;uint64_t p2=2ULL;uint64_t p3=0ULL;f25(v0, v0_n, p1, p2, p3);});
if(({uint64_t _a=({uint64_t p1=14400ULL;uint64_t p2=14600ULL;f26(v0, v0_n, p1, p2);});uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
break;
}
{
uint64_t v3=0;
uint64_t k2=64ULL;
for(;o_op(8,5, v3, k2);v3=o_op(1,5, v3,1)){
{uint64_t _i=({uint64_t _a=14500ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=14600ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);o_put(v0, v0_n, _i,_v,5);}
}
}
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 9, _a,_b);});
}
}
if(({uint64_t _a=v2;uint64_t _b=1023ULL;o_op(10, 9, _a,_b);})){
return o_leave(9218868437227405312ULL);
}
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=53ULL;
if(({uint64_t _a=v2;uint64_t _b=18446744073709550594ULL;o_op(8, 9, _a,_b);})){
v4=({uint64_t _a=v2;uint64_t _b=1075ULL;o_op(1, 9, _a,_b);});
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
if(v1){
return o_leave(9223372036854775808ULL);
}
return o_leave(0ULL);
}
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(10, 9, _a,_b);})){
if(c1++>=53)o_trap(4);
v3=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});
if(({uint64_t _a=({uint64_t p1=14400ULL;uint64_t p2=14500ULL;f26(v0, v0_n, p1, p2);});uint64_t _b=0ULL;o_op(11, 9, _a,_b);})){
({uint64_t p1=14400ULL;uint64_t p2=14500ULL;f27(v0, v0_n, p1, p2);});
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
({uint64_t p1=14400ULL;uint64_t p2=2ULL;uint64_t p3=0ULL;f25(v0, v0_n, p1, p2, p3);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(2, 9, _a,_b);});
}
}
uint64_t __attribute__((unused)) v5=({uint64_t p1=14400ULL;uint64_t p2=14500ULL;f26(v0, v0_n, p1, p2);});
if(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=0ULL;o_op(10, 9, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 9, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);})){
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
if(({uint64_t _a=v3;uint64_t _b=9007199254740992ULL;o_op(6, 5, _a,_b);})){
v3=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(4, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 9, _a,_b);});
}
uint64_t __attribute__((unused)) v6=v3;
if(({uint64_t _a=v2;uint64_t _b=18446744073709550594ULL;o_op(11, 9, _a,_b);})){
v6=({uint64_t _a=({uint64_t _a=o_cast(5, 9, ({uint64_t _a=v2;uint64_t _b=1023ULL;o_op(1, 9, _a,_b);}));uint64_t _b=52ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=4503599627370496ULL;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);});
}
if(v1){
v6=({uint64_t _a=v6;uint64_t _b=9223372036854775808ULL;o_op(13, 5, _a,_b);});
}
return o_leave(v6);
}
static uint64_t f29(uint64_t v0){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=48ULL;o_op(11, 2, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=57ULL;o_op(9, 2, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=o_cast(5, 2, v0);uint64_t _b=48ULL;o_op(2, 5, _a,_b);}));
}
v0=({uint64_t _a=v0;uint64_t _b=32ULL;o_op(13, 2, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=97ULL;o_op(11, 2, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=102ULL;o_op(9, 2, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=o_cast(5, 2, v0);uint64_t _b=97ULL;o_op(2, 5, _a,_b);});uint64_t _b=10ULL;o_op(1, 5, _a,_b);}));
}
return o_leave(16ULL);
}
static uint64_t f30(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p1=14400ULL;f24(v1, v1_n, p1);});
({uint64_t p1=14500ULL;f24(v1, v1_n, p1);});
({uint64_t p1=14600ULL;f24(v1, v1_n, p1);});
{uint64_t _i=14500ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14301ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14302ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14303ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v5=2ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=128)o_trap(4);
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=v6;uint64_t _b=46ULL;o_op(6, 2, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 14302ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
{uint64_t _i=14302ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
uint64_t __attribute__((unused)) v7=({uint64_t p0=v6;f29(p0);});
if(({uint64_t _a=v7;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
break;
}
({uint64_t p1=14400ULL;uint64_t p2=16ULL;uint64_t p3=v7;f25(v1, v1_n, p1, p2, p3);});
{uint64_t _i=14303ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 14303ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=o_get(v1, v1_n, 14302ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=14301ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 14301ULL, 5);uint64_t _b=4ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=o_get(v1, v1_n, 14303ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v6=({uint64_t _a=o_cast(9, 5, o_get(v1, v1_n, 14301ULL, 5));uint64_t _b=0;o_op(17, 9, _a,_b);});
if(({uint64_t _a=v5;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v7;uint64_t _b=112ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=80ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=0ULL;
if(({uint64_t _a=v5;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
v7=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=v7;uint64_t _b=43ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);})){
v8=({uint64_t _a=v7;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v5;uint64_t _b=v3;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v9=0ULL;
{
uint64_t c2=0;
while(({uint64_t _a=v5;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c2++>=128)o_trap(4);
v7=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=v7;uint64_t _b=48ULL;o_op(8, 2, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=57ULL;o_op(10, 2, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v9;uint64_t _b=10000ULL;o_op(8, 9, _a,_b);})){
v9=({uint64_t _a=({uint64_t _a=v9;uint64_t _b=10ULL;o_op(3, 9, _a,_b);});uint64_t _b=({uint64_t _a=o_cast(9, 2, v7);uint64_t _b=48ULL;o_op(2, 9, _a,_b);});o_op(1, 9, _a,_b);});
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(v8){
v9=({uint64_t _a=v9;uint64_t _b=0;o_op(17, 9, _a,_b);});
}
v6=({uint64_t _a=v6;uint64_t _b=v9;o_op(1, 9, _a,_b);});
}
if(({uint64_t _a=({uint64_t p1=14400ULL;uint64_t p2=14600ULL;f26(v1, v1_n, p1, p2);});uint64_t _b=0ULL;o_op(6, 9, _a,_b);})){
v6=0ULL;
}
if(({uint64_t _a=v6;uint64_t _b=1100ULL;o_op(10, 9, _a,_b);})){
if(v4){
return o_leave(0ULL);
}
{uint64_t _i=14309ULL;uint64_t _v=9218868437227405312ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v6;uint64_t _b=18446744073709549916ULL;o_op(8, 9, _a,_b);})){
{uint64_t _i=14309ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
if(v4){
{uint64_t _i=14309ULL;uint64_t _v=9223372036854775808ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(1ULL);
}
{
uint64_t c1=0;
while(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(10, 9, _a,_b);})){
if(c1++>=1100)o_trap(4);
({uint64_t p1=14400ULL;uint64_t p2=2ULL;uint64_t p3=0ULL;f25(v1, v1_n, p1, p2, p3);});
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(2, 9, _a,_b);});
}
}
{
uint64_t c1=0;
while(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
if(c1++>=1700)o_trap(4);
({uint64_t p1=14500ULL;uint64_t p2=2ULL;uint64_t p3=0ULL;f25(v1, v1_n, p1, p2, p3);});
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 9, _a,_b);});
}
}
uint64_t __attribute__((unused)) v7=({uint64_t p1=v4;f28(v1, v1_n, p1);});
if(({uint64_t _a=v4;uint64_t _b=({uint64_t _a=({uint64_t _a=v7;uint64_t _b=9223372036854775807ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(0ULL);
}
{uint64_t _i=14309ULL;uint64_t _v=v7;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
static uint64_t f31(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=0ULL;
uint64_t __attribute__((unused)) v5=0ULL;
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=128)o_trap(4);
if(({uint64_t _a=({uint64_t p0=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);f9(p0);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
break;
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v4;uint64_t _b=v3;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=43ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);})){
v5=({uint64_t _a=v6;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=v4;o_op(2, 5, _a,_b);});uint64_t _b=2ULL;o_op(11, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);uint64_t _b=48ULL;o_op(6, 2, _a,_b);})){
v6=o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=32ULL;o_op(13, 2, _a,_b);});uint64_t _b=120ULL;o_op(6, 2, _a,_b);})){
return o_leave(({uint64_t p2=({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t p3=({uint64_t _a=v3;uint64_t _b=v4;o_op(2, 5, _a,_b);});uint64_t p4=v5;f30(v0, v0_n, v1, v1_n, p2, p3, p4);}));
}
}
}
if(({uint64_t p1=({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t p2=({uint64_t _a=v3;uint64_t _b=v4;o_op(2, 5, _a,_b);});f23(v0, v0_n, p1, p2);})){
if(v5){
return o_leave(0ULL);
}
{uint64_t _i=14309ULL;uint64_t _v=9218868437227405312ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
({uint64_t p1=14400ULL;f24(v1, v1_n, p1);});
({uint64_t p1=14500ULL;f24(v1, v1_n, p1);});
({uint64_t p1=14600ULL;f24(v1, v1_n, p1);});
{uint64_t _i=14500ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14301ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14302ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14303ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=128)o_trap(4);
v6=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=v6;uint64_t _b=46ULL;o_op(6, 2, _a,_b);})){
if(({uint64_t _a=o_get(v1, v1_n, 14302ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
{uint64_t _i=14302ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}else{
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=48ULL;o_op(8, 2, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=57ULL;o_op(10, 2, _a,_b);});o_op(13, 1, _a,_b);})){
break;
}
{uint64_t _i=14303ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 14303ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
({uint64_t p1=14400ULL;uint64_t p2=10ULL;uint64_t p3=({uint64_t _a=o_cast(5, 2, v6);uint64_t _b=48ULL;o_op(2, 5, _a,_b);});f25(v1, v1_n, p1, p2, p3);});
if(({uint64_t _a=o_get(v1, v1_n, 14302ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=14301ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 14301ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=o_get(v1, v1_n, 14303ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v7=({uint64_t _a=o_cast(9, 5, o_get(v1, v1_n, 14301ULL, 5));uint64_t _b=0;o_op(17, 9, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
v6=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=101ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=69ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=0ULL;
if(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
v6=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=43ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);})){
v8=({uint64_t _a=v6;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v4;uint64_t _b=v3;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v9=0ULL;
{
uint64_t c2=0;
while(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c2++>=128)o_trap(4);
v6=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 2);
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=48ULL;o_op(8, 2, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=57ULL;o_op(10, 2, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v9;uint64_t _b=10000ULL;o_op(8, 9, _a,_b);})){
v9=({uint64_t _a=({uint64_t _a=v9;uint64_t _b=10ULL;o_op(3, 9, _a,_b);});uint64_t _b=({uint64_t _a=o_cast(9, 2, v6);uint64_t _b=48ULL;o_op(2, 9, _a,_b);});o_op(1, 9, _a,_b);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(v8){
v9=({uint64_t _a=v9;uint64_t _b=0;o_op(17, 9, _a,_b);});
}
v7=({uint64_t _a=v7;uint64_t _b=v9;o_op(1, 9, _a,_b);});
}
if(({uint64_t _a=({uint64_t p1=14400ULL;uint64_t p2=14600ULL;f26(v1, v1_n, p1, p2);});uint64_t _b=0ULL;o_op(6, 9, _a,_b);})){
v7=0ULL;
}
if(({uint64_t _a=v7;uint64_t _b=400ULL;o_op(10, 9, _a,_b);})){
if(v5){
return o_leave(0ULL);
}
{uint64_t _i=14309ULL;uint64_t _v=9218868437227405312ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
if(({uint64_t _a=v7;uint64_t _b=18446744073709551116ULL;o_op(8, 9, _a,_b);})){
{uint64_t _i=14309ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
if(v5){
{uint64_t _i=14309ULL;uint64_t _v=9223372036854775808ULL;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(1ULL);
}
{
uint64_t c1=0;
while(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(10, 9, _a,_b);})){
if(c1++>=400)o_trap(4);
({uint64_t p1=14400ULL;uint64_t p2=10ULL;uint64_t p3=0ULL;f25(v1, v1_n, p1, p2, p3);});
v7=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(2, 9, _a,_b);});
}
}
{
uint64_t c1=0;
while(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
if(c1++>=500)o_trap(4);
({uint64_t p1=14500ULL;uint64_t p2=10ULL;uint64_t p3=0ULL;f25(v1, v1_n, p1, p2, p3);});
v7=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(1, 9, _a,_b);});
}
}
uint64_t __attribute__((unused)) v8=({uint64_t p1=v5;f28(v1, v1_n, p1);});
if(({uint64_t _a=v5;uint64_t _b=({uint64_t _a=({uint64_t _a=v8;uint64_t _b=9223372036854775807ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(0ULL);
}
{uint64_t _i=14309ULL;uint64_t _v=v8;o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
static uint64_t f32(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=({uint64_t p2=v2;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t p2=o_get(v1, v1_n, 14010ULL, 5);uint64_t p3=o_get(v1, v1_n, 14011ULL, 5);f31(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(2ULL);
}
{uint64_t _i=v3;uint64_t _v=o_get(v1, v1_n, 14309ULL, 5);o_put(v1, v1_n, _i,_v,5);}
return o_leave(1ULL);
}
static uint64_t f33(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
return o_leave(o_get(v0, v0_n, v1, 5));
}
static uint64_t f34(uint64_t v0){
o_enter();
return o_leave(({uint64_t p0=v0;f9(p0);}));
}
static uint64_t f35(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 8ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 9ULL, 5);
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
if(({uint64_t _a=({uint64_t p0=o_get(v1, v1_n, v2, 2);f34(p0);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
break;
}
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v4=0ULL;
if(({uint64_t _a=v2;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, v2, 2);
if(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=43ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);})){
v4=({uint64_t _a=v5;uint64_t _b=45ULL;o_op(6, 2, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v5=v2;
uint64_t __attribute__((unused)) v6=0ULL;
uint64_t __attribute__((unused)) v7=({uint64_t _a=9223372036854775808ULL;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
if(v4){
v7=9223372036854775808ULL;
}
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v8=o_get(v1, v1_n, v2, 2);
if(({uint64_t _a=({uint64_t _a=v8;uint64_t _b=48ULL;o_op(8, 2, _a,_b);});uint64_t _b=({uint64_t _a=v8;uint64_t _b=57ULL;o_op(10, 2, _a,_b);});o_op(13, 1, _a,_b);})){
break;
}
if(({uint64_t p0=v6;uint64_t p1=v7;uint64_t p2=({uint64_t _a=o_cast(5, 2, v8);uint64_t _b=48ULL;o_op(2, 5, _a,_b);});f18(p0, p1, p2);})){
v6=v7;
}else{
v6=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=10ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_cast(5, 2, v8);uint64_t _b=48ULL;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);});
}
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v5;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
if(v4){
v6=({uint64_t _a=({uint64_t _a=0ULL;uint64_t _b=v6;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
}
{uint64_t _i=8ULL;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=10ULL;uint64_t _v=({uint64_t _a=v6;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(1ULL);
}
static uint64_t f36(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=2147483648ULL;o_op(11, 5, _a,_b);})){
return o_leave(({uint64_t _a=o_cast(9, 5, v0);uint64_t _b=4294967296ULL;o_op(2, 9, _a,_b);}));
}
return o_leave(o_cast(9, 5, v0));
}
static uint64_t f37(uint64_t v0){
o_enter();
v0=({uint64_t _a=v0;uint64_t _b=4294967296ULL;o_op(5, 9, _a,_b);});
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
v0=({uint64_t _a=v0;uint64_t _b=4294967296ULL;o_op(1, 9, _a,_b);});
}
return o_leave(o_cast(5, 9, v0));
}
static uint64_t f38(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t p0=o_get(v0, v0_n, 0ULL, 5);f36(p0);});
uint64_t __attribute__((unused)) v2=({uint64_t p0=o_get(v0, v0_n, 1ULL, 5);f36(p0);});
uint64_t __attribute__((unused)) v3=({uint64_t p0=o_get(v0, v0_n, 2ULL, 5);f36(p0);});
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
uint64_t __attribute__((unused)) v7=({uint64_t p0=v6;f37(p0);});
v7=({uint64_t _a=({uint64_t _a=({uint64_t _a=v7;uint64_t _b=153ULL;o_op(3, 5, _a,_b);});uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
v7=({uint64_t _a=v7;uint64_t _b=5ULL;o_op(4, 5, _a,_b);});
v7=({uint64_t _a=v7;uint64_t _b=({uint64_t p0=v3;f37(p0);});o_op(1, 5, _a,_b);});
v7=({uint64_t _a=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=o_cast(5, 9, v5);uint64_t _b=365ULL;o_op(3, 5, _a,_b);});
v8=({uint64_t _a=v8;uint64_t _b=({uint64_t _a=o_cast(5, 9, v5);uint64_t _b=4ULL;o_op(4, 5, _a,_b);});o_op(1, 5, _a,_b);});
v8=({uint64_t _a=v8;uint64_t _b=({uint64_t _a=o_cast(5, 9, v5);uint64_t _b=100ULL;o_op(4, 5, _a,_b);});o_op(2, 5, _a,_b);});
v8=({uint64_t _a=({uint64_t _a=v8;uint64_t _b=v7;o_op(1, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=146097ULL;o_op(3, 9, _a,_b);});uint64_t _b=719468ULL;o_op(2, 9, _a,_b);});uint64_t _b=({uint64_t p0=v8;f36(p0);});o_op(1, 9, _a,_b);});
return o_leave(({uint64_t _a=({uint64_t p0=v9;f37(p0);});uint64_t _b=4294967296ULL;o_op(13, 5, _a,_b);}));
}
static uint64_t f39(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
{uint64_t _i=8ULL;uint64_t _v=0ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=9ULL;uint64_t _v=o_get(v0, v0_n, 0ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{
uint64_t v2=0;
uint64_t k1=3ULL;
for(;o_op(8,5, v2, k1);v2=o_op(1,5, v2,1)){
if(({uint64_t _a=({f35(v0, v0_n, v1, v1_n);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
{uint64_t _i=v2;uint64_t _v=o_get(v0, v0_n, 10ULL, 5);o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(8, 5, _a,_b);})){
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 8ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=o_get(v0, v0_n, 9ULL, 5);o_op(11, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, v3, 2);uint64_t _b=45ULL;o_op(7, 2, _a,_b);})){
return o_leave(0ULL);
}
{uint64_t _i=8ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
}
}
}
return o_leave(({f38(v0, v0_n);}));
}
static uint64_t f40(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint8_t v2[256]={0};
uint64_t v2_n=256;
o_fill(v2, v2_n, 0ULL, 2);
uint64_t v3[16]={0};
uint64_t v3_n=16;
o_fill(v3, v3_n, 0ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 14011ULL, 5);
{uint64_t _i=0ULL;uint64_t _v=v4;o_put(v3, v3_n, _i,_v,5);}
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v4);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=v5;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=o_get(v1, v1_n, 14010ULL, 5);uint64_t _b=v5;o_op(1, 5, _a,_b);}), 2);o_put(v2, v2_n, _i,_v,2);}
}
}
return o_leave(({f39(v3, v3_n, v2, v2_n);}));
}
static uint64_t f41(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 14020ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=255ULL;o_op(8, 5, _a,_b);})){
{uint64_t _i=v3;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,2);}
{uint64_t _i=14020ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
}
return o_leave(0);
}
static uint64_t f42(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=14330ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, v4, 5);
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, ({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);
{
uint64_t v7=0;
uint64_t c1=0;
for(;o_op(8,5, v7, v6);v7=o_op(1,5, v7,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p2=o_get(v0, v0_n, ({uint64_t _a=v5;uint64_t _b=v7;o_op(1, 5, _a,_b);}), 2);f41(v1, v1_n, v2, v2_n, p2);});
}
}
return o_leave(0);
}
static uint64_t f43(void *v0, uint64_t v0_n){
o_enter();
{uint64_t _i=14330ULL;uint64_t _v=o_get(v0, v0_n, 14010ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=14331ULL;uint64_t _v=o_get(v0, v0_n, 14011ULL, 5);o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f44(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=49ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=113ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=113ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=124ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=124ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=124ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=124ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=60ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=52ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=48ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=120ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
return o_leave(0);
}
static uint64_t f45(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=89ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=89ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=89ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=89ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=45ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=77ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=77ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=45ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=68ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=68ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=49ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=40ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=120ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=51ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=50ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=41ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=49ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
return o_leave(0);
}
static uint64_t f46(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=48ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=24ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=60ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=60ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=25ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=48ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=26ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=27ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=40ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=120ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=50ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=53ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=54ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=41ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=28ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=29ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=30ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
return o_leave(0);
}
static uint64_t f47(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=31ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=32ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=33ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=34ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=60ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=35ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=36ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=37ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=60ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=38ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=39ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=60ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=40ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=77ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=78ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=40ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=41ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
return o_leave(0);
}
static uint64_t f48(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=41ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=42ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=60ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=60ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=43ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=44ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=58ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=1ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=45ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=46ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=47ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=48ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=48ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=60ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=62ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=49ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=50ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
return o_leave(0);
}
static uint64_t f49(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=51ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=52ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=53ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=44ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=44ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=46ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=46ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=46ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=54ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=55ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=56ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=57ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=106ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=58ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=106ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=59ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=106ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=60ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=40ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=41ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=59ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=106ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
return o_leave(0);
}
static uint64_t f50(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=61ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=95ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=62ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=63ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=64ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=112ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=68ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=65ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=71ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=65ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=66ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=67ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=39ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=39ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=68ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=107ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=119ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=0ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=39ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=1ULL;f42(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=39ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=69ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=39ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=118ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=49ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=39ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=70ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=97ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=104ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
return o_leave(0);
}
static uint64_t f51(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=71ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=98ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=72ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=103ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=121ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=99ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=100ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=40ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=115ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=117ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=109ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=116ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=61ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=102ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=111ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=114ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=85ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=78ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=77ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=69ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=65ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=83ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=85ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=82ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=69ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=68ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=41ULL;f41(v1, v1_n, v2, v2_n, p2);});
return o_leave(0);
}
return o_leave(0);
}
static uint64_t f52(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
{uint64_t _i=14020ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p2=108ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=105ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=110ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=101ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 14002ULL, 5);
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(1ULL){
if(c1++>=20)o_trap(4);
{uint64_t _i=({uint64_t _a=14350ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v4;uint64_t _b=10ULL;o_op(5, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=10ULL;o_op(4, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
}
}
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c1++>=20)o_trap(4);
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
({uint64_t p2=o_cast(2, 5, ({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=14350ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t _b=48ULL;o_op(1, 5, _a,_b);}));f41(v1, v1_n, v2, v2_n, p2);});
}
}
({uint64_t p2=58ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p2=32ULL;f41(v1, v1_n, v2, v2_n, p2);});
({uint64_t p3=v3;f44(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p3=v3;f45(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p3=v3;f46(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p3=v3;f47(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p3=v3;f48(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p3=v3;f49(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p3=v3;f50(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p3=v3;f51(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
{uint64_t _i=o_get(v1, v1_n, 14020ULL, 5);uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,2);}
return o_leave(1ULL);
}
static uint64_t f53(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p2=20ULL;f11(v0, v0_n, v1, v1_n, p2);});
({f43(v1, v1_n);});
uint64_t __attribute__((unused)) v6=4294967295ULL;
if(v5){
{
uint64_t v7=0;
uint64_t k2=5ULL;
for(;o_op(8,5, v7, k2);v7=o_op(1,5, v7,1)){
if(({uint64_t p2=({uint64_t _a=v7;uint64_t _b=51ULL;o_op(1, 5, _a,_b);});f12(v0, v0_n, v1, v1_n, p2);})){
v6=v7;
break;
}
}
}
if(({uint64_t _a=v6;uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);})){
return o_leave(7ULL);
}
}
uint64_t __attribute__((unused)) v7=({uint64_t p2=21ULL;uint64_t p3=v3;uint64_t p4=127ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=v5;uint64_t _b=0;o_op(18, 1, _a,_b);})){
if(v4){
return o_leave(6ULL);
}
}else{
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=0;o_op(18, 1, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(8ULL);
}
}
{uint64_t _i=v2;uint64_t _v=v6;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f54(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=({uint64_t p2=13ULL;uint64_t p3=17ULL;uint64_t p4=39ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(4ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t p2=14ULL;uint64_t p3=19ULL;uint64_t p4=39ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
v2=({uint64_t p2=15ULL;uint64_t p3=1ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=({uint64_t p2=16ULL;uint64_t p3=2ULL;f32(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(5ULL);
}
uint64_t __attribute__((unused)) v3=({uint64_t p2=14310ULL;uint64_t p3=14311ULL;uint64_t p4=1ULL;f53(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v3);
}
{uint64_t _i=16ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 16ULL, 5);uint64_t _b=2ULL;o_op(13, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f55(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t p2=17ULL;uint64_t p3=21ULL;uint64_t p4=39ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=({uint64_t p2=18ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(9ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 14011ULL, 5);uint64_t _b=40ULL;o_op(7, 5, _a,_b);})){
return o_leave(9ULL);
}
({uint64_t p1=23ULL;uint64_t p2=47ULL;f13(v1, v1_n, p1, p2);});
v2=({uint64_t p2=19ULL;uint64_t p3=25ULL;uint64_t p4=47ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=16ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 16ULL, 5);uint64_t _b=4ULL;o_op(13, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f56(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(10ULL);
}
if(({uint64_t _a=({uint64_t p2=13ULL;uint64_t p3=27ULL;uint64_t p4=39ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(11ULL);
}
if(({uint64_t _a=({uint64_t p2=22ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(12ULL);
}
uint64_t __attribute__((unused)) v2=({f40(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=4294967296ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(12ULL);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=2147483648ULL;o_op(11, 5, _a,_b);})){
return o_leave(12ULL);
}
{uint64_t _i=4ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t p2=23ULL;uint64_t p3=5ULL;f20(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(13ULL);
}
if(({uint64_t _a=({uint64_t p2=24ULL;uint64_t p3=6ULL;f32(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(14ULL);
}
if(({uint64_t _a=({uint64_t p2=21ULL;uint64_t p3=29ULL;uint64_t p4=127ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(15ULL);
}
uint64_t __attribute__((unused)) v3=({uint64_t p2=25ULL;uint64_t p3=31ULL;uint64_t p4=127ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=3ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f57(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 11ULL, 5);uint64_t _b=32ULL;o_op(11, 5, _a,_b);})){
return o_leave(16ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=64ULL;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 11ULL, 5);uint64_t _b=12ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=({uint64_t p2=13ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(17ULL);
}
({f43(v1, v1_n);});
if(({uint64_t _a=({uint64_t p2=0ULL;f17(v0, v0_n, v1, v1_n, p2);});uint64_t _b=4294967295ULL;o_op(7, 5, _a,_b);})){
return o_leave(18ULL);
}
({uint64_t p1=v2;uint64_t p2=39ULL;f13(v1, v1_n, p1, p2);});
if(({uint64_t _a=({uint64_t p2=26ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(19ULL);
}
({f43(v1, v1_n);});
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t v4=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t p2=({uint64_t _a=v4;uint64_t _b=56ULL;o_op(1, 5, _a,_b);});f12(v0, v0_n, v1, v1_n, p2);})){
v3=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
break;
}
}
}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(26ULL);
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t p2=27ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});f20(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(20ULL);
}
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=2147483648ULL;o_op(11, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(20ULL);
}
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
uint64_t __attribute__((unused)) v4=({uint64_t p2=28ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});uint64_t _b=0ULL;o_fp(9, 12, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(21ULL);
}
return o_leave(25ULL);
}
}
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t p2=29ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(22ULL);
}
if(({uint64_t _a=({uint64_t p2=30ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(23ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});uint64_t _b=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});o_fp(10, 12, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});uint64_t _b=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});o_fp(8, 12, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(24ULL);
}
}
uint64_t __attribute__((unused)) v4=({uint64_t p2=({uint64_t _a=v2;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t p3=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);});uint64_t p4=1ULL;f53(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v4);
}
{uint64_t _i=11ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 11ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f58(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=({uint64_t p2=v2;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(0ULL);
}
({f43(v1, v1_n);});
uint64_t __attribute__((unused)) v5=({uint64_t p2=0ULL;f17(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v5;uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=({uint64_t _a=64ULL;uint64_t _b=({uint64_t _a=v5;uint64_t _b=12ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=v4;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
{uint64_t _i=v3;uint64_t _v=v5;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f59(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p2=32ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(33ULL);
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);});uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
}
v3=({uint64_t p2=29ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});uint64_t _b=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});o_fp(10, 12, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(34ULL);
}
}
v3=({uint64_t p2=30ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});uint64_t _b=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});o_fp(8, 12, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(35ULL);
}
}
v3=({uint64_t p2=33ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=9ULL;o_op(1, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(36ULL);
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t p2=34ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(37ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t p2=0ULL;f17(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v4;uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);})){
return o_leave(37ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=({uint64_t _a=64ULL;uint64_t _b=({uint64_t _a=v4;uint64_t _b=12ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=2ULL;o_op(7, 5, _a,_b);})){
return o_leave(37ULL);
}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=10ULL;o_op(1, 5, _a,_b);});uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
}
v3=({uint64_t p2=35ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=11ULL;o_op(1, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(38ULL);
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t p2=36ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(39ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t p2=0ULL;f17(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v4;uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);})){
return o_leave(39ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=({uint64_t _a=64ULL;uint64_t _b=({uint64_t _a=v4;uint64_t _b=12ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
return o_leave(39ULL);
}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=12ULL;o_op(1, 5, _a,_b);});uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(0ULL);
}
static uint64_t f60(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 12ULL, 5);uint64_t _b=256ULL;o_op(11, 5, _a,_b);})){
return o_leave(27ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=448ULL;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 12ULL, 5);uint64_t _b=26ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=4294967295ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=10ULL;o_op(1, 5, _a,_b);});uint64_t _v=4294967295ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=12ULL;o_op(1, 5, _a,_b);});uint64_t _v=4294967295ULL;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t p2=13ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(28ULL);
}
({f43(v1, v1_n);});
if(({uint64_t _a=({uint64_t p2=1ULL;f17(v0, v0_n, v1, v1_n, p2);});uint64_t _b=4294967295ULL;o_op(7, 5, _a,_b);})){
return o_leave(29ULL);
}
({uint64_t p1=v2;uint64_t p2=39ULL;f13(v1, v1_n, p1, p2);});
if(({uint64_t _a=({uint64_t p2=26ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(30ULL);
}
({f43(v1, v1_n);});
uint64_t __attribute__((unused)) v3=0ULL;
if(({uint64_t p2=56ULL;f12(v0, v0_n, v1, v1_n, p2);})){
v3=1ULL;
}
{
uint64_t v4=0;
uint64_t k1=5ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t p2=({uint64_t _a=v4;uint64_t _b=60ULL;o_op(1, 5, _a,_b);});f12(v0, v0_n, v1, v1_n, p2);})){
v3=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});
}
}
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(31ULL);
}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t p2=31ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t p4=1ULL;f58(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(32ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t p2=v2;f59(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v4);
}
uint64_t __attribute__((unused)) v5=({uint64_t p2=37ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=13ULL;o_op(1, 5, _a,_b);});f20(v0, v0_n, v1, v1_n, p2, p3);});
v5=({uint64_t p2=38ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=14ULL;o_op(1, 5, _a,_b);});f20(v0, v0_n, v1, v1_n, p2, p3);});
v5=({uint64_t p2=39ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=15ULL;o_op(1, 5, _a,_b);});f20(v0, v0_n, v1, v1_n, p2, p3);});
v5=({uint64_t p2=40ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(1, 5, _a,_b);});f20(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=13ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=14ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(40ULL);
}
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
{uint64_t _i=14330ULL;uint64_t _v=o_get(v1, v1_n, v2, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14331ULL;uint64_t _v=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);o_put(v1, v1_n, _i,_v,5);}
return o_leave(41ULL);
}
uint64_t __attribute__((unused)) v6=({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=8ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});
v6=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=9ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});uint64_t _b=0ULL;o_fp(10, 12, _a,_b);});o_op(13, 1, _a,_b);});
v6=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=({uint64_t p1=({uint64_t _a=v2;uint64_t _b=11ULL;o_op(1, 5, _a,_b);});f33(v1, v1_n, p1);});uint64_t _b=0ULL;o_fp(10, 12, _a,_b);});o_op(13, 1, _a,_b);});
v6=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});
v4=({uint64_t p2=({uint64_t _a=v2;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t p3=({uint64_t _a=v2;uint64_t _b=17ULL;o_op(1, 5, _a,_b);});uint64_t p4=v6;f53(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v4);
}
if(({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
}
{uint64_t _i=12ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 12ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f61(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, 14007ULL, 5);uint64_t _b=2ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 14006ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(42ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 14ULL, 5);uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
return o_leave(43ULL);
}
{uint64_t _i=14330ULL;uint64_t _v=o_get(v1, v1_n, 14220ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14331ULL;uint64_t _v=o_get(v1, v1_n, 14221ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14332ULL;uint64_t _v=o_get(v1, v1_n, 14222ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14333ULL;uint64_t _v=o_get(v1, v1_n, 14223ULL, 5);o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v2=({uint64_t p2=1ULL;uint64_t p3=o_get(v1, v1_n, 14220ULL, 5);uint64_t p4=o_get(v1, v1_n, 14221ULL, 5);f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v3=({uint64_t p2=1ULL;uint64_t p3=o_get(v1, v1_n, 14222ULL, 5);uint64_t p4=o_get(v1, v1_n, 14223ULL, 5);f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(44ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=7104ULL;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 14ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{uint64_t _i=v4;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 14ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f62(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 13ULL, 5);uint64_t _b=256ULL;o_op(11, 5, _a,_b);})){
return o_leave(45ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=9152ULL;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 13ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t _v=4294967295ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=4294967295ULL;o_put(v1, v1_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t p2=13ULL;uint64_t p3=v2;uint64_t p4=39ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(46ULL);
}
if(({uint64_t _a=({uint64_t p2=33ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});f32(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(47ULL);
}
if(({uint64_t _a=({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=9223372036854775807ULL;o_op(12, 5, _a,_b);});uint64_t _b=9218868437227405312ULL;o_op(11, 5, _a,_b);})){
return o_leave(47ULL);
}
if(({uint64_t _a=({uint64_t p2=28ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(48ULL);
}
uint64_t __attribute__((unused)) v3=({uint64_t p2=0ULL;f17(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v3;uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);})){
return o_leave(48ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, ({uint64_t _a=({uint64_t _a=64ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=12ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(48ULL);
}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
{
uint64_t v4=0;
uint64_t k1=2ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t p2=({uint64_t _a=v4;uint64_t _b=41ULL;o_op(1, 5, _a,_b);});f11(v0, v0_n, v1, v1_n, p2);})){
({f43(v1, v1_n);});
v3=({uint64_t p2=1ULL;f17(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v3;uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=v4;uint64_t _b=49ULL;o_op(1, 5, _a,_b);}));
}
{uint64_t _i=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
}
}
}
{uint64_t _i=13ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 13ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f63(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 12ULL, 5);uint64_t _b=256ULL;o_op(11, 5, _a,_b);})){
return o_leave(51ULL);
}
if(({uint64_t _a=({uint64_t p2=13ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(52ULL);
}
({f43(v1, v1_n);});
if(({uint64_t _a=({uint64_t p2=1ULL;f17(v0, v0_n, v1, v1_n, p2);});uint64_t _b=4294967295ULL;o_op(7, 5, _a,_b);})){
return o_leave(29ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=448ULL;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 12ULL, 5);uint64_t _b=26ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
({uint64_t p1=v2;uint64_t p2=39ULL;f13(v1, v1_n, p1, p2);});
if(({uint64_t _a=({uint64_t p2=43ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(53ULL);
}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=4294967295ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=10ULL;o_op(1, 5, _a,_b);});uint64_t _v=4294967295ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=12ULL;o_op(1, 5, _a,_b);});uint64_t _v=4294967295ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 12ULL, 5);
{uint64_t _i=12ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v4=o_get(v1, v1_n, 14010ULL, 5);
uint64_t __attribute__((unused)) v5=({uint64_t _a=v4;uint64_t _b=o_get(v1, v1_n, 14011ULL, 5);o_op(1, 5, _a,_b);});
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v5;o_op(8, 5, _a,_b);})){
if(c1++>=128)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, v4, 2);uint64_t _b=44ULL;o_op(6, 2, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
continue;
}
uint64_t __attribute__((unused)) v6=v4;
{
uint64_t c2=0;
while(({uint64_t _a=v4;uint64_t _b=v5;o_op(8, 5, _a,_b);})){
if(c2++>=128)o_trap(4);
if(({uint64_t _a=o_get(v0, v0_n, v4, 2);uint64_t _b=44ULL;o_op(6, 2, _a,_b);})){
break;
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v7=({uint64_t _a=v4;uint64_t _b=v6;o_op(2, 5, _a,_b);});
{uint64_t _i=14330ULL;uint64_t _v=v6;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14331ULL;uint64_t _v=v7;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v8=({uint64_t p2=1ULL;uint64_t p3=v6;uint64_t p4=v7;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=({uint64_t _a=v8;uint64_t _b=4294967295ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v8;uint64_t _b=v3;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(54ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 14ULL, 5);uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
return o_leave(43ULL);
}
v2=({uint64_t _a=7104ULL;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 14ULL, 5);uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{uint64_t _i=v2;uint64_t _v=v8;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 14ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
}
}
return o_leave(0ULL);
}
static uint64_t f64(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 7ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(55ULL);
}
if(({uint64_t _a=({uint64_t p2=44ULL;uint64_t p3=33ULL;uint64_t p4=39ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(56ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t p2=45ULL;uint64_t p3=35ULL;uint64_t p4=39ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v3=({uint64_t p2=46ULL;f11(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v4=({uint64_t p2=47ULL;f11(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t p2=10ULL;uint64_t p3=14311ULL;uint64_t p4=({uint64_t _a=v3;uint64_t _b=v4;o_op(13, 1, _a,_b);});f53(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v5);
}
if(v3){
if(({uint64_t _a=({uint64_t p1=o_get(v1, v1_n, 33ULL, 5);uint64_t p2=o_get(v1, v1_n, 34ULL, 5);uint64_t p3=72ULL;f7(v0, v0_n, p1, p2, p3);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 36ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(57ULL);
}
if(({uint64_t _a=o_get(v1, v1_n, 10ULL, 5);uint64_t _b=3ULL;o_op(7, 5, _a,_b);})){
return o_leave(58ULL);
}
if(({uint64_t _a=({uint64_t p2=46ULL;uint64_t p3=8ULL;f32(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(59ULL);
}
}
if(v4){
if(({uint64_t _a=o_get(v1, v1_n, 10ULL, 5);uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(60ULL);
}
if(({uint64_t _a=({uint64_t p2=47ULL;uint64_t p3=9ULL;f32(v0, v0_n, v1, v1_n, p2, p3);});uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(61ULL);
}
}
{uint64_t _i=7ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f65(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
if(({uint64_t _a=o_get(v1, v1_n, 15ULL, 5);uint64_t _b=256ULL;o_op(11, 5, _a,_b);})){
return o_leave(62ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=11200ULL;uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 15ULL, 5);uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=({uint64_t p2=6ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(63ULL);
}
({f43(v1, v1_n);});
({uint64_t p1=v2;uint64_t p2=39ULL;f13(v1, v1_n, p1, p2);});
if(({uint64_t _a=({uint64_t p2=1ULL;f17(v0, v0_n, v1, v1_n, p2);});uint64_t _b=4294967295ULL;o_op(7, 5, _a,_b);})){
return o_leave(64ULL);
}
if(({uint64_t _a=({uint64_t p2=48ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(65ULL);
}
({f43(v1, v1_n);});
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t v4=0;
uint64_t k1=7ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
if(({uint64_t p2=({uint64_t _a=v4;uint64_t _b=65ULL;o_op(1, 5, _a,_b);});f12(v0, v0_n, v1, v1_n, p2);})){
v3=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(66ULL);
}
{uint64_t _i=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v4=({uint64_t p2=49ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t p4=39ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
v4=({uint64_t p2=50ULL;uint64_t p3=({uint64_t _a=v2;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t p4=127ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=15ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 15ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f66(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=({f21(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t p2=12ULL;f11(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=({uint64_t p2=73ULL;f12(v0, v0_n, v1, v1_n, p2);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(3ULL);
}
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=16ULL;uint64_t _v=({uint64_t _a=o_get(v1, v1_n, 16ULL, 5);uint64_t _b=1ULL;o_op(13, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f54(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f55(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f56(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=v2;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f57(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=v2;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f60(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=v2;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f61(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=v2;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f62(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=v2;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f63(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=v2;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f64(v0, v0_n, v1, v1_n);}));
}
if(({uint64_t _a=v2;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(({f65(v0, v0_n, v1, v1_n);}));
}
{uint64_t _i=14330ULL;uint64_t _v=o_get(v1, v1_n, 14004ULL, 5);o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=14331ULL;uint64_t _v=o_get(v1, v1_n, 14005ULL, 5);o_put(v1, v1_n, _i,_v,5);}
return o_leave(67ULL);
}
static uint64_t f67(void *v0, uint64_t v0_n){
o_enter();
{
uint64_t v1=0;
uint64_t k1=o_get(v0, v0_n, 14006ULL, 5);
uint64_t c1=0;
for(;o_op(8,5, v1, k1);v1=o_op(1,5, v1,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v2=({uint64_t _a=14100ULL;uint64_t _b=({uint64_t _a=v1;uint64_t _b=5ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
if(({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=4ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=14330ULL;uint64_t _v=o_get(v0, v0_n, v2, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=14331ULL;uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=14332ULL;uint64_t _v=o_get(v0, v0_n, 14004ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=14333ULL;uint64_t _v=o_get(v0, v0_n, 14005ULL, 5);o_put(v0, v0_n, _i,_v,5);}
return o_leave(1ULL);
}
}
}
return o_leave(0ULL);
}
static uint64_t f68(const void *v0, uint64_t v0_n, uint64_t v1, void *v2, uint64_t v2_n, void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=16384ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
{uint64_t _i=v4;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
}
}
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v3, v3_n, _i,_v,2);}
if(({uint64_t _a=v1;uint64_t _b=1048576ULL;o_op(11, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v4=0ULL;
{uint64_t _i=10ULL;uint64_t _v=4294967295ULL;o_put(v2, v2_n, _i,_v,5);}
{
uint64_t v5=0;
uint64_t k1=1048576ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
if(({uint64_t _a=v4;uint64_t _b=v1;o_op(11, 5, _a,_b);})){
break;
}
if(({uint64_t _a=o_get(v0, v0_n, v4, 2);uint64_t _b=0ULL;o_op(6, 2, _a,_b);})){
break;
}
{uint64_t _i=14002ULL;uint64_t _v=({uint64_t _a=o_get(v2, v2_n, 14002ULL, 5);uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=v4;
{
uint64_t c2=0;
while(({uint64_t _a=v6;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
if(c2++>=1024)o_trap(4);
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, v6, 2);uint64_t _b=10ULL;o_op(6, 2, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, v6, 2);uint64_t _b=0ULL;o_op(6, 2, _a,_b);});o_op(13, 1, _a,_b);})){
break;
}
v6=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=v4;o_op(2, 5, _a,_b);});uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
return o_leave(({uint64_t p3=1ULL;f52(v0, v0_n, v2, v2_n, v3, v3_n, p3);}));
}
}
}
uint64_t __attribute__((unused)) v7=({uint64_t p2=v4;uint64_t p3=v6;f22(v0, v0_n, v2, v2_n, p2, p3);});
v4=v6;
if(({uint64_t _a=v4;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, v4, 2);uint64_t _b=10ULL;o_op(6, 2, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v7;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
continue;
}
if(({uint64_t _a=v7;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p3=2ULL;f52(v0, v0_n, v2, v2_n, v3, v3_n, p3);}));
}
v7=({f66(v0, v0_n, v2, v2_n);});
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t p3=v7;f52(v0, v0_n, v2, v2_n, v3, v3_n, p3);}));
}
if(({f67(v2, v2_n);})){
return o_leave(({uint64_t p3=68ULL;f52(v0, v0_n, v2, v2_n, v3, v3_n, p3);}));
}
}
}
{uint64_t _i=14002ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 16ULL, 5);uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p3=69ULL;f52(v0, v0_n, v2, v2_n, v3, v3_n, p3);}));
}
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 16ULL, 5);uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p3=70ULL;f52(v0, v0_n, v2, v2_n, v3, v3_n, p3);}));
}
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 16ULL, 5);uint64_t _b=4ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p3=71ULL;f52(v0, v0_n, v2, v2_n, v3, v3_n, p3);}));
}
if(({uint64_t _a=o_get(v2, v2_n, 7ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p3=72ULL;f52(v0, v0_n, v2, v2_n, v3, v3_n, p3);}));
}
return o_leave(0ULL);
}
uint32_t omega_dag_parse(uint64_t *result, const void *p0, uint64_t p0_n, volatile uint64_t p1, void *p2, uint64_t p2_n, void *p3, uint64_t p3_n){
if(!o_buf(result,1,1,5))return 32;
if(!o_buf(p0, p0_n,1048576, 2))return 32;
if(o_overlap(result,8,p0, p0_n*1))return 32;
if(!o_valid(5, p1))return 32;
if(!o_buf(p2, p2_n,16384, 5))return 32;
if(o_overlap(result,8,p2, p2_n*8))return 32;
if(o_overlap(p2, p2_n*8, p0, p0_n*1))return 32;
if(!o_buf(p3, p3_n,256, 2))return 32;
if(o_overlap(result,8,p3, p3_n*1))return 32;
if(o_overlap(p3, p3_n*1, p0, p0_n*1))return 32;
if(o_overlap(p3, p3_n*1, p2, p2_n*8))return 32;
if(o_busy)return 33;
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f68(p0, p0_n, p1, p2, p2_n, p3, p3_n);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
