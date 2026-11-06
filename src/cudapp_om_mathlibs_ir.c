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
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f2(uint64_t v0);
static uint64_t f3(uint64_t v0);
static uint64_t f4(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f5(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f6(uint64_t v0, uint64_t v1);
static uint64_t f7(uint64_t v0, uint64_t v1);
static uint64_t f8(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f9(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f10(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f11(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f12(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f13(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f14(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f15(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f16(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f17(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f18(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f19(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f20(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f21(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f22(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f23(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f24(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f25(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f26(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f27(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f28(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f29(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f30(void *v0, uint64_t v0_n);
static uint64_t f31(void *v0, uint64_t v0_n);
static uint64_t f32(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f33(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f34(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f35(uint64_t v0);
static uint64_t f36(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f37(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f38(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f39(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f40(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f41(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f42(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f43(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f44(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n);
static uint64_t f45(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f46(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f47(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f48(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f49(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f50(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n);
static uint64_t f51(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f52(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f53(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4, uint64_t v5);
static uint64_t f54(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f55(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f56(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f57(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f58(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f59(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f60(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f61(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f62(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n);
static uint64_t f63(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f64(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f65(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n);
static uint64_t f66(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f67(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f68(uint64_t v0);
static uint64_t f69(uint64_t v0);
static uint64_t f70(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f71(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f72(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f73(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f74(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f75(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f76(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f77(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f78(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f79(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f80(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f81(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f82(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f83(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f84(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f85(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f86(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f87(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f88(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f89(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f90(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f91(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f92(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f93(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f94(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f95(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f96(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f97(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f98(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f99(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f100(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f101(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f102(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f103(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f104(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f105(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n, uint64_t v4);
static uint64_t f106(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f107(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f108(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3);
static uint64_t f109(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n);
static uint64_t f110(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4);
static uint64_t f111(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f112(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n);
static uint64_t f113(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f114(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f115(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f116(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f117(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f118(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f119(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n);
static uint64_t f0(uint64_t v0){
o_enter();
return o_leave(o_cast(5, 4, v0));
}
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=9ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=14ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=17ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=20ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f2(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=65535ULL;o_op(6, 5, _a,_b);})){
return o_leave(18446744073709551615ULL);
}
return o_leave(({uint64_t _a=v0;uint64_t _b=1023ULL;o_op(12, 5, _a,_b);}));
}
static uint64_t f3(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=65535ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(16, 5, _a,_b);}));
}
static uint64_t f4(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=48ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f5(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=40ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f6(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v1;uint64_t _b=10ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f7(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f8(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=14ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v2;f1(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=0ULL;uint64_t p2=7ULL;uint64_t p3=v2;f1(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});uint64_t _b=4294967296ULL;o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=2ULL;uint64_t p2=7ULL;uint64_t p3=v2;f1(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});uint64_t _b=17179869184ULL;o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=7ULL;uint64_t p2=1ULL;uint64_t p3=v2;f1(p0, p1, p2, p3);});uint64_t _b=8192ULL;o_op(13, 5, _a,_b);});uint64_t _b=8589934592ULL;o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v2;f1(p0, p1, p2, p3);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t p0=5ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v2;f1(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t p0=6ULL;uint64_t p1=7ULL;uint64_t p2=7ULL;uint64_t p3=v2;f1(p0, p1, p2, p3);}));
}
static uint64_t f9(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=v2;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v2;uint64_t _b=48ULL;o_op(16, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=v3;f2(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=v4;f2(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=v5;f2(p0);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=v6;f2(p0);});o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=({uint64_t p0=v3;f3(p0);});uint64_t _b=16ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p0=v4;f3(p0);});uint64_t _b=24ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=({uint64_t _a=({uint64_t p0=v5;f3(p0);});uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t p0=v6;f3(p0);});uint64_t _b=40ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v7;uint64_t _b=v8;o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f10(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=40ULL;o_op(16, 5, _a,_b);});uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=18446744073709551615ULL;
if(({uint64_t _a=v5;uint64_t _b=65535ULL;o_op(7, 5, _a,_b);})){
v6=v5;
}
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=5ULL;o_op(1, 5, _a,_b);});uint64_t _v=v6;o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=({uint64_t _a=v4;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=6ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
uint64_t __attribute__((unused)) v8=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 5);
{uint64_t _i=({uint64_t _a=v1;uint64_t _b=7ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=v8;uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=7ULL;o_op(12, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f11(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v1, v1_n, 0ULL, 5);
if(({uint64_t _a=v5;uint64_t _b=1024ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v6;uint64_t _b=8ULL;o_op(10, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v7=({uint64_t p0=v6;uint64_t p1=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t p2=o_get(v1, v1_n, 3ULL, 5);uint64_t p3=v4;f8(p0, p1, p2, p3);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});
{uint64_t _i=v8;uint64_t _v=({uint64_t _a=v2;uint64_t _b=255ULL;o_op(12, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
({uint64_t p1=v8;uint64_t p2=v3;f9(v0, v0_n, p1, p2);});
({uint64_t p1=v8;uint64_t p2=v2;uint64_t p3=v4;uint64_t p4=v7;f10(v0, v0_n, p1, p2, p3, p4);});
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t _a=v7;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f12(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=256ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=3ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
{uint64_t _i=({uint64_t _a=64ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(v2);
}
static uint64_t f13(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 2ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=32ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=3ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
{uint64_t _i=({uint64_t _a=320ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=v1;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(v2);
}
static uint64_t f14(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
({uint64_t p2=({uint64_t p0=v2;uint64_t p1=0ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=v3;uint64_t p4=v4;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f15(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=v2;uint64_t p3=({uint64_t p0=v5;uint64_t p1=v3;uint64_t p2=v4;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v5);
}
static uint64_t f16(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v5;uint64_t p1=v2;uint64_t p2=65535ULL;uint64_t p3=v4;f4(p0, p1, p2, p3);});uint64_t p4=v3;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v5);
}
static uint64_t f17(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p1=2ULL;f12(v1, v1_n, p1);});
({uint64_t p2=9ULL;uint64_t p3=({uint64_t p0=v5;uint64_t p1=v2;uint64_t p2=65535ULL;uint64_t p3=v4;f4(p0, p1, p2, p3);});uint64_t p4=v3;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v5);
}
static uint64_t f18(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=17ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=v2;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=v3;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v4);
}
static uint64_t f19(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=2ULL;uint64_t p3=({uint64_t p0=v3;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=v2;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v3);
}
static uint64_t f20(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=33ULL;uint64_t p3=({uint64_t p0=v5;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=v4;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v5);
}
static uint64_t f21(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=v6;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=v4;f4(p0, p1, p2, p3);});uint64_t p4=v5;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v6);
}
static uint64_t f22(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t p1=v3;f12(v1, v1_n, p1);});
({uint64_t p2=({uint64_t p0=v2;uint64_t p1=2ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=v6;uint64_t p1=v4;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=v5;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v6);
}
static uint64_t f23(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p1=2ULL;f12(v1, v1_n, p1);});
({uint64_t p2=({uint64_t p0=56ULL;uint64_t p1=2ULL;uint64_t p2=0ULL;uint64_t p3=v4;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=v5;uint64_t p1=v2;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=v3;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v5);
}
static uint64_t f24(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
({uint64_t p2=({uint64_t p0=12ULL;uint64_t p1=4ULL;uint64_t p2=0ULL;uint64_t p3=v5;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=v4;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f25(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=({uint64_t p0=50ULL;uint64_t p1=4ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f26(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p2=({uint64_t p0=48ULL;uint64_t p1=5ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f27(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
({uint64_t p2=({uint64_t p0=28ULL;uint64_t p1=1ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f28(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p2=({uint64_t p0=45ULL;uint64_t p1=6ULL;uint64_t p2=2ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f29(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({uint64_t p2=({uint64_t p0=45ULL;uint64_t p1=6ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=({uint64_t p0=46ULL;uint64_t p1=8ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=1032192ULL;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f30(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 6ULL, 5);
if(({uint64_t _a=v1;uint64_t _b=4ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=4ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
{uint64_t _i=({uint64_t _a=8ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, 0ULL, 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f31(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 6ULL, 5);
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 5ULL, 5);
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=24ULL;o_op(11, 5, _a,_b);});o_op(13, 1, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=5ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=16ULL;uint64_t _b=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=o_get(v0, v0_n, ({uint64_t _a=8ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=17ULL;uint64_t _b=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=v3;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f32(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
({uint64_t p2=({uint64_t p0=46ULL;uint64_t p1=7ULL;uint64_t p2=2ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=1ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v2);
}
static uint64_t f33(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, v3, 5);
{uint64_t _i=v3;uint64_t _v=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=18446744069414584320ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v1, v1_n, 0ULL, 5);uint64_t _b=v2;o_op(2, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f34(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 6ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, ({uint64_t _a=7ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);}), 5);
({uint64_t p2=({uint64_t p0=46ULL;uint64_t p1=7ULL;uint64_t p2=6ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=65535ULL;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=o_get(v1, v1_n, 0ULL, 5);o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f35(uint64_t v0){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=29ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=51ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=52ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=53ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=30ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=31ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=32ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=33ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
static uint64_t f36(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, ({uint64_t _a=v1;uint64_t _b=8ULL;o_op(3, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v3=({uint64_t p0=v2;f35(p0);});
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=({uint64_t p0=o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);}), 5);f35(p0);});o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0);
}
if(({uint64_t _a=v2;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
if(({uint64_t _a=({uint64_t _a=o_get(v0, v0_n, ({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=7ULL;o_op(1, 5, _a,_b);}), 5);uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=6ULL;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, v4, 5);
uint64_t __attribute__((unused)) v6=({uint64_t _a=v5;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=9ULL;o_op(16, 5, _a,_b);});uint64_t _b=15ULL;o_op(12, 5, _a,_b);});uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
{uint64_t _i=v4;uint64_t _v=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=512ULL;o_op(2, 5, _a,_b);});uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f37(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 0ULL, 5);
{
uint64_t v3=0;
uint64_t c1=0;
for(;o_op(8,5, v3, v2);v3=o_op(1,5, v3,1)){
if(c1++>=1000000)o_trap(4);
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=v2;o_op(8, 5, _a,_b);})){
({uint64_t p1=v3;f36(v0, v0_n, p1);});
}
}
}
return o_leave(0);
}
static uint64_t f38(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=v4;uint64_t p3=v3;f18(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=v5;uint64_t p3=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=32ULL;uint64_t _b=v3;o_op(2, 5, _a,_b);});o_op(15, 5, _a,_b);});uint64_t p4=v6;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=29ULL;uint64_t p3=v4;uint64_t p4=v7;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v9=({uint64_t p2=v5;uint64_t p3=v3;f18(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v10=({uint64_t p2=29ULL;uint64_t p3=v5;uint64_t p4=v9;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(({uint64_t p0=v8;uint64_t p1=v10;f7(p0, p1);}));
}
static uint64_t f39(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v3;uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=v4;uint64_t p3=v6;uint64_t p4=65535ULL;f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v9=({uint64_t p2=v5;uint64_t p3=v6;uint64_t p4=({uint64_t p0=v8;uint64_t p1=1ULL;f6(p0, p1);});f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v10=v9;
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v10=({uint64_t p2=v4;uint64_t p3=v7;uint64_t p4=v9;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
return o_leave(({uint64_t p0=({uint64_t p0=v8;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p1=v10;f7(p0, p1);}));
}
static uint64_t f40(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=v5;uint64_t p3=({uint64_t _a=32ULL;uint64_t _b=v3;o_op(2, 5, _a,_b);});f18(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=v4;uint64_t p3=({uint64_t _a=32ULL;uint64_t _b=v3;o_op(2, 5, _a,_b);});f18(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=v4;uint64_t p3=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});uint64_t p4=v6;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v9=({uint64_t p2=v5;uint64_t p3=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});uint64_t p4=v7;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(({uint64_t p0=v8;uint64_t p1=v9;f7(p0, p1);}));
}
static uint64_t f41(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p2=v2;uint64_t p3=30ULL;f38(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v4=({uint64_t p2=v3;uint64_t p3=13787848793156543929ULL;f39(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v5=({uint64_t p2=v4;uint64_t p3=27ULL;f38(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=v5;uint64_t p3=10723151780598845931ULL;f39(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(({uint64_t p2=v6;uint64_t p3=31ULL;f38(v0, v0_n, v1, v1_n, p2, p3);}));
}
static uint64_t f42(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
({uint64_t p2=29ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=v4;uint64_t p2=({uint64_t _a=v3;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=29ULL;uint64_t p3=({uint64_t p0=v5;uint64_t p1=v5;uint64_t p2=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f43(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=v2;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t p2=v3;uint64_t p3=19ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=v4;uint64_t p3=19ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=v4;uint64_t p3=8192ULL;uint64_t p4=v5;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=v6;f4(p0, p1, p2, p3);});uint64_t p4=8192ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v3;uint64_t p1=v7;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f44(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 0ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 1ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 2ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 3ULL, 5);
uint64_t __attribute__((unused)) v7=({uint64_t _a=v4;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v4;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t p2=v7;uint64_t p3=131072ULL;uint64_t p4=65535ULL;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v10=({uint64_t p2=v8;uint64_t p3=131072ULL;uint64_t p4=({uint64_t p2=v7;uint64_t p3=15ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v5;uint64_t p3=v3;f42(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v6;uint64_t p3=v4;f42(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v4;uint64_t p3=v5;f42(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v3;uint64_t p3=v6;f42(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v5;uint64_t p3=({uint64_t p0=v9;uint64_t p1=v10;f7(p0, p1);});f42(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p2=v6;f43(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f45(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p2=({uint64_t p0=854680968ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});
v4=({uint64_t p2=v4;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=884493110ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v4=({uint64_t p2=v4;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=3060560727ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v4=({uint64_t p2=v4;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=3063110337ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v4=({uint64_t p2=v4;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=962933467ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v4=({uint64_t p2=v4;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=3131331592ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v4=({uint64_t p2=v4;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=3146310895ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v4=({uint64_t p2=v4;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=1048350563ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v5=({uint64_t p0=1069559343ULL;f0(p0);});
if(({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t p0=1069547520ULL;f0(p0);});
}
return o_leave(({uint64_t p2=v4;uint64_t p3=v2;uint64_t p4=({uint64_t p2=v5;f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);}));
}
static uint64_t f46(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p2=({uint64_t p0=3109154971ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});
v3=({uint64_t p2=v3;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=953398635ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v3=({uint64_t p2=v3;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=984669298ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v3=({uint64_t p2=v3;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=3144728039ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v3=({uint64_t p2=v3;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=1002181243ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v3=({uint64_t p2=v3;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=3153708503ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v3=({uint64_t p2=v3;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=1008379262ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
v3=({uint64_t p2=v3;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=1065367259ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(({uint64_t p2=v3;uint64_t p3=v2;uint64_t p4=({uint64_t p2=({uint64_t p0=1077235582ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f20(v0, v0_n, v1, v1_n, p2, p3, p4);}));
}
static uint64_t f47(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p2=1ULL;f19(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v4=({uint64_t p2=v2;uint64_t p3=8ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
v4=({uint64_t p2=v4;uint64_t p3=v3;uint64_t p4=65535ULL;uint64_t p5=252ULL;f21(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v5=({uint64_t p2=16777216ULL;f19(v0, v0_n, v1, v1_n, p2);});
v5=({uint64_t p2=v4;uint64_t p3=4294967295ULL;uint64_t p4=v5;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=4278190080ULL;f19(v0, v0_n, v1, v1_n, p2);});
v6=({uint64_t p2=v4;uint64_t p3=2ULL;uint64_t p4=v6;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
v4=({uint64_t p2=38ULL;uint64_t p3=v4;uint64_t p4=65535ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
v5=({uint64_t p2=38ULL;uint64_t p3=v5;uint64_t p4=65535ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=32ULL;uint64_t p3=v4;uint64_t p4=v5;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
v7=({uint64_t p2=32ULL;uint64_t p3=v7;uint64_t p4=({uint64_t p2=({uint64_t p0=679477248ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
v7=({uint64_t p2=59ULL;uint64_t p3=1ULL;uint64_t p4=v7;uint64_t p5=0ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
v7=({uint64_t p2=32ULL;uint64_t p3=v7;uint64_t p4=({uint64_t p2=({uint64_t p0=3207688728ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(({uint64_t p0=v7;uint64_t p1=v6;f7(p0, p1);}));
}
static uint64_t f48(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p2=v2;f47(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v4;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=31ULL;uint64_t p3=v5;uint64_t p4=({uint64_t p2=({uint64_t p0=1075838976ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=v6;uint64_t p3=v3;f45(v0, v0_n, v1, v1_n, p2, p3);});
v6=({uint64_t p2=41ULL;uint64_t p3=1ULL;uint64_t p4=v5;uint64_t p5=0ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
v6=({uint64_t p2=32ULL;uint64_t p3=v5;uint64_t p4=v6;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
v6=({uint64_t p2=31ULL;uint64_t p3=v6;uint64_t p4=({uint64_t p2=({uint64_t p0=1077936128ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=v6;f46(v0, v0_n, v1, v1_n, p2);});
v6=({uint64_t p2=31ULL;uint64_t p3=v5;uint64_t p4=({uint64_t p2=({uint64_t p0=1084227584ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
v6=({uint64_t p2=v6;uint64_t p3=31ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
v6=({uint64_t p2=v6;uint64_t p3=4294967295ULL;uint64_t p4=65535ULL;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
v7=({uint64_t p2=v6;uint64_t p3=v7;uint64_t p4=v8;uint64_t p5=202ULL;f21(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
v6=({uint64_t p2=38ULL;uint64_t p3=({uint64_t _a=v4;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t p4=65535ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
v6=({uint64_t p2=32ULL;uint64_t p3=v6;uint64_t p4=({uint64_t p2=({uint64_t p0=864026624ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
v6=({uint64_t p2=32ULL;uint64_t p3=v7;uint64_t p4=v6;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(({uint64_t p2=32ULL;uint64_t p3=v6;uint64_t p4=({uint64_t p2=({uint64_t p0=1068827891ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);}));
}
static uint64_t f49(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=2ULL;f13(v1, v1_n, p1);});
{uint64_t _i=0ULL;uint64_t _v=v3;o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=({uint64_t p0=7ULL;uint64_t p1=3ULL;uint64_t p2=1ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=v3;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=856ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=1ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=896ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{
uint64_t v4=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
{uint64_t _i=({uint64_t _a=9ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=({uint64_t _a=904ULL;uint64_t _b=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
}
}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t p2=4ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=936ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t p2=4ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=940ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=({uint64_t p2=4ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=944ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v4=({uint64_t p2=3ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=33ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v5=({uint64_t p2=3ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=37ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=v5;uint64_t p3=128ULL;uint64_t p4=v4;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=5ULL;uint64_t _v=v6;o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, 4ULL, 5);f27(v0, v0_n, v1, v1_n, p2, p3);});
({f28(v0, v0_n, v1, v1_n);});
{uint64_t _i=6ULL;uint64_t _v=({uint64_t p2=1ULL;f19(v0, v0_n, v1, v1_n, p2);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=1ULL;uint64_t p3=({uint64_t p0=v7;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=7ULL;uint64_t _v=v7;o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f50(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n){
o_enter();
{
uint64_t v4=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=({uint64_t p2=o_get(v2, v2_n, 5ULL, 5);uint64_t p3=1ULL;uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=9ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);}), 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=v4;uint64_t _v=({uint64_t p2=({uint64_t p0=({uint64_t p0=v5;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p1=({uint64_t p0=v5;uint64_t p1=1ULL;f6(p0, p1);});f7(p0, p1);});f41(v0, v0_n, v1, v1_n, p2);});o_put(v3, v3_n, _i,_v,5);}
}
}
return o_leave(0);
}
static uint64_t f51(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 2ULL, 5);
({uint64_t p2=o_get(v2, v2_n, 7ULL, 5);uint64_t p3=v4;f27(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v5=({f32(v0, v0_n, v1, v1_n);});
({f30(v1, v1_n);});
({f44(v0, v0_n, v1, v1_n, v3, v3_n);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=o_get(v2, v2_n, 6ULL, 5);uint64_t p2=65535ULL;uint64_t p3=v4;f4(p0, p1, p2, p3);});uint64_t p4=4294967295ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 7ULL, 5);uint64_t p3=v4;f27(v0, v0_n, v1, v1_n, p2, p3);});
({f34(v0, v0_n, v1, v1_n);});
({f31(v1, v1_n);});
({uint64_t p2=v5;f33(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f52(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t p2=o_get(v2, v2_n, 8ULL, 5);uint64_t p3=8ULL;uint64_t p4=o_get(v2, v2_n, 1ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v7;uint64_t p3=({uint64_t _a=v3;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});uint64_t p4=0ULL;uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v7;uint64_t p3=v6;uint64_t p4=4ULL;uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
uint64_t __attribute__((unused)) v7=0ULL;
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v7=({uint64_t p2=v6;uint64_t p3=8ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
v7=({uint64_t p2=38ULL;uint64_t p3=v7;uint64_t p4=65535ULL;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
v7=({uint64_t p2=32ULL;uint64_t p3=v7;uint64_t p4=({uint64_t p2=({uint64_t p0=864026624ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
}else{
v7=({uint64_t p2=v6;uint64_t p3=v5;f48(v0, v0_n, v1, v1_n, p2, p3);});
}
uint64_t __attribute__((unused)) v8=({uint64_t p2=o_get(v2, v2_n, 8ULL, 5);uint64_t p3=4ULL;uint64_t p4=o_get(v2, v2_n, 1ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v8;uint64_t p3=v7;uint64_t p4=0ULL;uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f53(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t p2=o_get(v2, v2_n, 5ULL, 5);uint64_t p3=1ULL;uint64_t p4=65535ULL;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=8ULL;uint64_t _v=v6;o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=7ULL;
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v7=8ULL;
}
({f30(v1, v1_n);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=o_get(v3, v3_n, 1ULL, 5);uint64_t p3=5ULL;f39(v0, v0_n, v1, v1_n, p2, p3);});
v8=({uint64_t p2=v8;uint64_t p3=v7;f40(v0, v0_n, v1, v1_n, p2, p3);});
v8=({uint64_t p2=v8;uint64_t p3=9ULL;f39(v0, v0_n, v1, v1_n, p2, p3);});
({uint64_t p3=v8;uint64_t p4=v4;uint64_t p5=v5;f52(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({f44(v0, v0_n, v1, v1_n, v3, v3_n);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v6;uint64_t p1=o_get(v2, v2_n, 4ULL, 5);uint64_t p2=65535ULL;uint64_t p3=v6;f4(p0, p1, p2, p3);});uint64_t p4=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v9=o_get(v2, v2_n, 3ULL, 5);
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v9;uint64_t p1=o_get(v2, v2_n, 6ULL, 5);uint64_t p2=65535ULL;uint64_t p3=v9;f4(p0, p1, p2, p3);});uint64_t p4=4294967295ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 7ULL, 5);uint64_t p3=v9;f27(v0, v0_n, v1, v1_n, p2, p3);});
({f34(v0, v0_n, v1, v1_n);});
({f31(v1, v1_n);});
return o_leave(0);
}
static uint64_t f54(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t v4[16]={0};
uint64_t v4_n=16;
o_fill(v4, v4_n, 0ULL, 5);
uint64_t v5[4]={0};
uint64_t v5_n=4;
o_fill(v5, v5_n, 0ULL, 5);
({f49(v0, v0_n, v1, v1_n, v4, v4_n);});
({f50(v0, v0_n, v1, v1_n, v4, v4_n, v5, v5_n);});
({f51(v0, v0_n, v1, v1_n, v4, v4_n, v5, v5_n);});
({uint64_t p4=v2;uint64_t p5=v3;f53(v0, v0_n, v1, v1_n, v4, v4_n, v5, v5_n, p4, p5);});
({f29(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f55(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
return o_leave(({uint64_t _a=v0;uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f56(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, ({uint64_t _a=v4;uint64_t _b=255ULL;o_op(12, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, ({uint64_t _a=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);}), 5);
({uint64_t p2=v3;uint64_t p3=({uint64_t p0=v5;uint64_t p1=v6;uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=v4;uint64_t _b=16ULL;o_op(16, 5, _a,_b);}), 5);uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f57(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});
{uint64_t _i=48ULL;uint64_t _v=v6;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=51ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=50ULL;uint64_t _v=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});o_op(1, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=1ULL;
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v7=0ULL;
}
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v7=({uint64_t _a=1ULL;uint64_t _b=v7;o_op(2, 5, _a,_b);});
}
{uint64_t _i=52ULL;uint64_t _v=v7;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=53ULL;uint64_t _v=1ULL;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=55ULL;uint64_t _v=v5;o_put(v2, v2_n, _i,_v,5);}
{
uint64_t v8=0;
uint64_t k1=16ULL;
for(;o_op(8,5, v8, k1);v8=o_op(1,5, v8,1)){
{uint64_t _i=v8;uint64_t _v=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
}
}
{
uint64_t v8=0;
uint64_t k1=3ULL;
for(;o_op(8,5, v8, k1);v8=o_op(1,5, v8,1)){
{uint64_t _i=({uint64_t _a=32ULL;uint64_t _b=v8;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=2ULL;f12(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
}
}
uint64_t __attribute__((unused)) v8=({uint64_t p1=2ULL;f13(v1, v1_n, p1);});
{uint64_t _i=40ULL;uint64_t _v=v8;o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=({uint64_t p0=7ULL;uint64_t p1=3ULL;uint64_t p2=1ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=v8;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=856ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=41ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=896ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=42ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=904ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=43ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=912ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v9=({uint64_t p2=3ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=33ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=44ULL;uint64_t _v=v9;o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v10=({uint64_t p2=3ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=37ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=45ULL;uint64_t _v=({uint64_t p2=1ULL;f19(v0, v0_n, v1, v1_n, p2);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=46ULL;uint64_t _v=({uint64_t p2=v10;uint64_t p3=v6;uint64_t p4=v9;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=47ULL;uint64_t _v=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=o_get(v2, v2_n, 50ULL, 5);uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
({f58(v0, v0_n, v1, v1_n, v2, v2_n);});
}
return o_leave(0);
}
static uint64_t f58(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 48ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 45ULL, 5);
uint64_t __attribute__((unused)) v5=({uint64_t p2=o_get(v2, v2_n, 44ULL, 5);uint64_t p3=4ULL;uint64_t p4=65535ULL;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
{
uint64_t v6=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
uint64_t __attribute__((unused)) v7=({uint64_t _a=v6;uint64_t _b=v3;o_op(3, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=16ULL;uint64_t _b=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p2=v4;uint64_t p3=v7;uint64_t p4=v5;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=17ULL;uint64_t _b=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p2=v4;uint64_t p3=({uint64_t _a=v7;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t p4=v5;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
}
}
return o_leave(0);
}
static uint64_t f59(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 48ULL, 5);
uint64_t __attribute__((unused)) v5=({uint64_t p2=o_get(v2, v2_n, 46ULL, 5);uint64_t p3=8ULL;uint64_t p4=o_get(v2, v2_n, 41ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
{
uint64_t v6=0;
uint64_t k2=4ULL;
for(;o_op(8,5, v6, k2);v6=o_op(1,5, v6,1)){
uint64_t __attribute__((unused)) v7=({uint64_t p2=v5;uint64_t p3=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(3, 5, _a,_b);});uint64_t p4=o_get(v2, v2_n, 40ULL, 5);f23(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=({uint64_t _a=24ULL;uint64_t _b=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=v7;uint64_t p1=0ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=25ULL;uint64_t _b=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=v7;uint64_t p1=1ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
}
}
return o_leave(0);
}
({uint64_t p2=({uint64_t p0=49ULL;uint64_t p1=2ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 0ULL, 5);uint64_t p1=o_get(v2, v2_n, 16ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
{
uint64_t v4=0;
uint64_t k1=7ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
({uint64_t p2=({uint64_t p0=49ULL;uint64_t p1=8ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=o_get(v2, v2_n, v5, 5);uint64_t p1=o_get(v2, v2_n, ({uint64_t _a=16ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=927744ULL;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
{
uint64_t v4=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
{uint64_t _i=({uint64_t _a=24ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});uint64_t _v=o_get(v2, v2_n, v4, 5);o_put(v2, v2_n, _i,_v,5);}
}
}
return o_leave(0);
}
static uint64_t f60(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, ({uint64_t _a=24ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, ({uint64_t _a=25ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v7=({uint64_t p2=32ULL;uint64_t p3=v5;uint64_t p4=({uint64_t p0=v4;uint64_t p1=0ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=32ULL;uint64_t p3=v6;uint64_t p4=({uint64_t p0=v4;uint64_t p1=1ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v9=({uint64_t p2=32ULL;uint64_t p3=v6;uint64_t p4=({uint64_t p0=v4;uint64_t p1=0ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=33ULL;uint64_t p3=({uint64_t p0=v6;uint64_t p1=v5;uint64_t p2=({uint64_t p0=v4;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p3=v9;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=31ULL;uint64_t p3=({uint64_t p0=v5;uint64_t p1=v7;uint64_t p2=v8;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(0);
}
static uint64_t f61(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 53ULL, 5);
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v4=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
{uint64_t _i=56ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=18ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=o_get(v2, v2_n, 44ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 55ULL, 5);uint64_t _b=2ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 54ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});
{uint64_t _i=54ULL;uint64_t _v=1ULL;o_put(v2, v2_n, _i,_v,5);}
if(v5){
uint64_t __attribute__((unused)) v6=({uint64_t _a=o_get(v2, v2_n, 48ULL, 5);uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(4, 5, _a,_b);});
{
uint64_t v7=0;
uint64_t k2=3ULL;
for(;o_op(8,5, v7, k2);v7=o_op(1,5, v7,1)){
uint64_t __attribute__((unused)) v8=({uint64_t p2=v4;uint64_t p3=({uint64_t _a=({uint64_t _a=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=v6;o_op(3, 5, _a,_b);});uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t p4=o_get(v2, v2_n, 43ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v9=o_get(v2, v2_n, ({uint64_t _a=32ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);}), 5);
({uint64_t p2=({uint64_t p0=56ULL;uint64_t p1=2ULL;uint64_t p2=0ULL;uint64_t p3=o_get(v2, v2_n, 40ULL, 5);f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=v9;uint64_t p1=v8;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p3=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p4=v9;f60(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
}
}
}
return o_leave(0);
}
static uint64_t f62(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n){
o_enter();
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=8ULL;uint64_t p1=24ULL;uint64_t p2=28ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=9ULL;uint64_t p1=25ULL;uint64_t p2=29ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=10ULL;uint64_t p1=24ULL;uint64_t p2=28ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=11ULL;uint64_t p1=25ULL;uint64_t p2=29ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=12ULL;uint64_t p1=26ULL;uint64_t p2=30ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=13ULL;uint64_t p1=27ULL;uint64_t p2=31ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=14ULL;uint64_t p1=26ULL;uint64_t p2=30ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=15ULL;uint64_t p1=27ULL;uint64_t p2=31ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=0ULL;uint64_t p1=8ULL;uint64_t p2=12ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=1ULL;uint64_t p1=9ULL;uint64_t p2=13ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=4ULL;uint64_t p1=8ULL;uint64_t p2=12ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=5ULL;uint64_t p1=9ULL;uint64_t p2=13ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
uint64_t __attribute__((unused)) v3=2ULL;
uint64_t __attribute__((unused)) v4=6ULL;
if(({uint64_t _a=o_get(v2, v2_n, 52ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v3=6ULL;
v4=2ULL;
}
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=v3;uint64_t p1=10ULL;uint64_t p2=15ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p1=11ULL;uint64_t p2=14ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=v4;uint64_t p1=10ULL;uint64_t p2=15ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p1=11ULL;uint64_t p2=14ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
return o_leave(0);
}
static uint64_t f63(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=0ULL;uint64_t p1=24ULL;uint64_t p2=28ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=1ULL;uint64_t p1=25ULL;uint64_t p2=29ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=2ULL;uint64_t p1=24ULL;uint64_t p2=28ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=3ULL;uint64_t p1=25ULL;uint64_t p2=29ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=4ULL;uint64_t p1=26ULL;uint64_t p2=30ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=30ULL;uint64_t p4=({uint64_t p0=5ULL;uint64_t p1=27ULL;uint64_t p2=31ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=6ULL;uint64_t p1=26ULL;uint64_t p2=30ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=({uint64_t p0=7ULL;uint64_t p1=27ULL;uint64_t p2=31ULL;f55(p0, p1, p2);});f56(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 48ULL, 5);
uint64_t __attribute__((unused)) v4=({uint64_t p2=o_get(v2, v2_n, 44ULL, 5);uint64_t p3=8ULL;uint64_t p4=65535ULL;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 47ULL, 5);
{
uint64_t v6=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
uint64_t __attribute__((unused)) v7=({uint64_t _a=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{
uint64_t v8=0;
uint64_t k2=2ULL;
for(;o_op(8,5, v8, k2);v8=o_op(1,5, v8,1)){
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v5;uint64_t p1=o_get(v2, v2_n, 45ULL, 5);uint64_t p2=65535ULL;uint64_t p3=v4;f4(p0, p1, p2, p3);});uint64_t p4=({uint64_t _a=v7;uint64_t _b=({uint64_t _a=v8;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v5;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=v8;o_op(1, 5, _a,_b);}), 5);f25(v0, v0_n, v1, v1_n, p2, p3);});
}
}
}
}
({f26(v0, v0_n, v1, v1_n);});
{uint64_t _i=53ULL;uint64_t _v=2ULL;o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f64(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 48ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 53ULL, 5);
uint64_t __attribute__((unused)) v5=({uint64_t p2=o_get(v2, v2_n, 44ULL, 5);uint64_t p3=16ULL;uint64_t p4=65535ULL;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
v5=({uint64_t p2=o_get(v2, v2_n, 56ULL, 5);uint64_t p3=4294967284ULL;uint64_t p4=v5;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 47ULL, 5);
{
uint64_t v7=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v7, k1);v7=o_op(1,5, v7,1)){
{
uint64_t v8=0;
uint64_t k2=2ULL;
for(;o_op(8,5, v8, k2);v8=o_op(1,5, v8,1)){
uint64_t __attribute__((unused)) v9=({uint64_t _a=({uint64_t _a=({uint64_t _a=v7;uint64_t _b=v4;o_op(3, 5, _a,_b);});uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=v8;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v6;uint64_t p1=o_get(v2, v2_n, 45ULL, 5);uint64_t p2=65535ULL;uint64_t p3=v5;f4(p0, p1, p2, p3);});uint64_t p4=v9;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v6;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=({uint64_t _a=v7;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=v8;o_op(1, 5, _a,_b);}), 5);f25(v0, v0_n, v1, v1_n, p2, p3);});
}
}
}
}
({f26(v0, v0_n, v1, v1_n);});
{uint64_t _i=53ULL;uint64_t _v=({uint64_t _a=v4;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f65(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 48ULL, 5);
uint64_t __attribute__((unused)) v4=({uint64_t p2=o_get(v2, v2_n, 46ULL, 5);uint64_t p3=8ULL;uint64_t p4=o_get(v2, v2_n, 42ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
{
uint64_t v5=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
uint64_t __attribute__((unused)) v6=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(3, 5, _a,_b);});
({uint64_t p2=v4;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=v5;uint64_t _b=2ULL;o_op(3, 5, _a,_b);}), 5);uint64_t p4=v6;uint64_t p5=o_get(v2, v2_n, 40ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v4;uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);uint64_t p4=({uint64_t _a=v6;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 40ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
}
}
return o_leave(0);
}
static uint64_t f66(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
({uint64_t p3=v3;f59(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 51ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
({f63(v0, v0_n, v1, v1_n, v2, v2_n);});
return o_leave(0);
}
({f61(v0, v0_n, v1, v1_n, v2, v2_n);});
({f62(v0, v0_n, v1, v1_n, v2, v2_n);});
if(({uint64_t _a=v3;uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 50ULL, 5);uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(6, 5, _a,_b);})){
({f65(v0, v0_n, v1, v1_n, v2, v2_n);});
return o_leave(0);
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
({f26(v0, v0_n, v1, v1_n);});
}
({f64(v0, v0_n, v1, v1_n, v2, v2_n);});
return o_leave(0);
}
static uint64_t f67(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t v5[64]={0};
uint64_t v5_n=64;
o_fill(v5, v5_n, 0ULL, 5);
({uint64_t p3=v2;uint64_t p4=v3;uint64_t p5=v4;f57(v0, v0_n, v1, v1_n, v5, v5_n, p3, p4, p5);});
uint64_t __attribute__((unused)) v6=o_get(v5, v5_n, 50ULL, 5);
{
uint64_t v7=0;
uint64_t c1=0;
for(;o_op(8,5, v7, v6);v7=o_op(1,5, v7,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p3=v7;f66(v0, v0_n, v1, v1_n, v5, v5_n, p3);});
}
}
({f29(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f68(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=v0;uint64_t _b=v0;o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v2=v0;
uint64_t __attribute__((unused)) v3=v0;
{
uint64_t v4=0;
uint64_t k1=12ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=o_fcv(12, 5, ({uint64_t _a=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=2ULL;o_op(1, 5, _a,_b);}));
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v1;uint64_t _b=0;o_fp(17, 12, _a,_b);});o_fp(3, 12, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=v5;uint64_t _b=4607182418800017408ULL;o_fp(1, 12, _a,_b);});o_fp(3, 12, _a,_b);});o_fp(4, 12, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=v2;o_fp(1, 12, _a,_b);});
}
}
return o_leave(v3);
}
static uint64_t f69(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=v0;uint64_t _b=v0;o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v2=4607182418800017408ULL;
uint64_t __attribute__((unused)) v3=4607182418800017408ULL;
{
uint64_t v4=0;
uint64_t k1=12ULL;
for(;o_op(8,5, v4, k1);v4=o_op(1,5, v4,1)){
uint64_t __attribute__((unused)) v5=o_fcv(12, 5, ({uint64_t _a=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));
v2=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v1;uint64_t _b=0;o_fp(17, 12, _a,_b);});o_fp(3, 12, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=v5;uint64_t _b=4607182418800017408ULL;o_fp(1, 12, _a,_b);});o_fp(3, 12, _a,_b);});o_fp(4, 12, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=v2;o_fp(1, 12, _a,_b);});
}
}
return o_leave(v3);
}
static uint64_t f70(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=v1;
uint64_t __attribute__((unused)) v5=v2;
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=v2;uint64_t _b=0;o_fp(17, 12, _a,_b);});
v5=v1;
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v4=({uint64_t _a=v1;uint64_t _b=0;o_fp(17, 12, _a,_b);});
v5=({uint64_t _a=v2;uint64_t _b=0;o_fp(17, 12, _a,_b);});
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v4=v2;
v5=({uint64_t _a=v1;uint64_t _b=0;o_fp(17, 12, _a,_b);});
}
uint64_t __attribute__((unused)) v6=o_fcv(11, 12, v4);
uint64_t __attribute__((unused)) v7=o_fcv(11, 12, v5);
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v7=({uint64_t _a=v7;uint64_t _b=0;o_fp(17, 11, _a,_b);});
}
return o_leave(({uint64_t _a=o_cast(5, 4, v6);uint64_t _b=({uint64_t _a=o_cast(5, 4, v7);uint64_t _b=32ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f71(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=1ULL;uint64_t _b=v1;o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t _b=v1;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=v3;o_op(3, 5, _a,_b);});o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});o_op(10, 5, _a,_b);});
if(v6){
v5=({uint64_t _a=v3;uint64_t _b=v5;o_op(2, 5, _a,_b);});
}
uint64_t __attribute__((unused)) v7=({uint64_t _a=4609753056924675352ULL;uint64_t _b=({uint64_t _a=o_fcv(12, 5, v5);uint64_t _b=o_fcv(12, 5, v3);o_fp(4, 12, _a,_b);});o_fp(3, 12, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t p0=v7;f69(p0);});
uint64_t __attribute__((unused)) v9=({uint64_t p0=v7;f68(p0);});
if(v6){
uint64_t __attribute__((unused)) v10=v8;
v8=v9;
v9=v10;
}
return o_leave(({uint64_t p0=v4;uint64_t p1=v8;uint64_t p2=v9;uint64_t p3=v2;f70(p0, p1, p2, p3);}));
}
static uint64_t f72(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=1ULL;uint64_t _b=v1;o_op(15, 5, _a,_b);});
{
uint64_t v4=0;
uint64_t c1=0;
for(;o_op(8,5, v4, v3);v4=o_op(1,5, v4,1)){
if(c1++>=1000000)o_trap(4);
{uint64_t _i=v4;uint64_t _v=({uint64_t p0=v4;uint64_t p1=v1;uint64_t p2=v2;f71(p0, p1, p2);});o_put(v0, v0_n, _i,_v,5);}
}
}
return o_leave(0);
}
static uint64_t f73(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=({uint64_t p1=2ULL;f13(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=({uint64_t p0=7ULL;uint64_t p1=3ULL;uint64_t p2=1ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 0ULL, 5);uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=856ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=1ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=896ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=904ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v4=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=912ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=4ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=920ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=({uint64_t p2=4ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=928ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t p2=4ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=932ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=30ULL;uint64_t _v=({uint64_t p2=4ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=936ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v5=({uint64_t p2=3ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=33ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=3ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=37ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=v6;uint64_t p3=128ULL;uint64_t p4=v5;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v7;uint64_t p3=o_get(v2, v2_n, 6ULL, 5);f27(v0, v0_n, v1, v1_n, p2, p3);});
({f28(v0, v0_n, v1, v1_n);});
{uint64_t _i=3ULL;uint64_t _v=({uint64_t p2=v7;uint64_t p3=1ULL;uint64_t p4=v4;f17(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=8ULL;uint64_t _v=({uint64_t p2=1ULL;f19(v0, v0_n, v1, v1_n, p2);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v8=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=1ULL;uint64_t p3=({uint64_t p0=v8;uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=9ULL;uint64_t _v=v8;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=({uint64_t p2=v7;uint64_t p3=v3;uint64_t p4=v8;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=32ULL;uint64_t _v=({uint64_t p2=o_get(v2, v2_n, 6ULL, 5);uint64_t p3=v3;uint64_t p4=v8;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v9=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v9;uint64_t p1=v7;uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=v8;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=31ULL;uint64_t _v=v9;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=33ULL;uint64_t _v=({uint64_t p2=v3;f19(v0, v0_n, v1, v1_n, p2);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=38ULL;uint64_t _v=({uint64_t p2=4294967294ULL;f19(v0, v0_n, v1, v1_n, p2);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=49ULL;uint64_t _v=o_get(v2, v2_n, 8ULL, 5);o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=50ULL;uint64_t _v=({uint64_t p2=2ULL;f19(v0, v0_n, v1, v1_n, p2);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=51ULL;uint64_t _v=({uint64_t p2=3ULL;f19(v0, v0_n, v1, v1_n, p2);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f74(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 2ULL, 5);
uint64_t __attribute__((unused)) v4=3144134277ULL;
if(({uint64_t _a=o_get(v2, v2_n, 54ULL, 5);uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v4=2654435769ULL;
}
{uint64_t _i=10ULL;uint64_t _v=({uint64_t p0=v3;uint64_t p1=0ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=20ULL;uint64_t _v=({uint64_t p0=v3;uint64_t p1=1ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
{
uint64_t v5=0;
uint64_t k1=9ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
uint64_t __attribute__((unused)) v6=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, 8ULL, 5);
uint64_t __attribute__((unused)) v8=({uint64_t p2=v7;uint64_t p3=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=2654435769ULL;o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t p4=({uint64_t p0=v3;uint64_t p1=0ULL;f6(p0, p1);});f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=({uint64_t _a=10ULL;uint64_t _b=v6;o_op(1, 5, _a,_b);});uint64_t _v=v8;o_put(v2, v2_n, _i,_v,5);}
v8=({uint64_t p2=v7;uint64_t p3=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=v4;o_op(3, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});uint64_t p4=({uint64_t p0=v3;uint64_t p1=1ULL;f6(p0, p1);});f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=({uint64_t _a=20ULL;uint64_t _b=v6;o_op(1, 5, _a,_b);});uint64_t _v=v8;o_put(v2, v2_n, _i,_v,5);}
}
}
return o_leave(0);
}
static uint64_t f75(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
return o_leave(o_get(v0, v0_n, ({uint64_t _a=64ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);}), 5));
}
static uint64_t f76(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
{uint64_t _i=({uint64_t _a=64ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p2=v4;f19(v0, v0_n, v1, v1_n, p2);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f77(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v2, v2_n, 55ULL, 5);
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
({uint64_t p3=0ULL;uint64_t p4=1199570944ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=1ULL;uint64_t p4=796917760ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=2ULL;uint64_t p4=788529152ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
}
if(({uint64_t _a=v3;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
({uint64_t p3=3ULL;uint64_t p4=818483163ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=4ULL;uint64_t p4=810094555ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=5ULL;uint64_t p4=3212836864ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=6ULL;uint64_t p4=3271426048ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=7ULL;uint64_t p4=1051372202ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=8ULL;uint64_t p4=3196059644ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=9ULL;uint64_t p4=1045221036ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=10ULL;uint64_t p4=3190468176ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=11ULL;uint64_t p4=1041361343ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=12ULL;uint64_t p4=3187563855ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=13ULL;uint64_t p4=1039082778ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=14ULL;uint64_t p4=3186348470ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=15ULL;uint64_t p4=1032855994ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=16ULL;uint64_t p4=3204448256ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=17ULL;uint64_t p4=1060204928ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=18ULL;uint64_t p4=924317650ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=19ULL;uint64_t p4=3221225472ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=20ULL;uint64_t p4=8388608ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=21ULL;uint64_t p4=1056964608ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=22ULL;uint64_t p4=1059256707ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=23ULL;uint64_t p4=1262485504ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=24ULL;uint64_t p4=3409969152ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=25ULL;uint64_t p4=3217625051ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=26ULL;uint64_t p4=859553070ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=27ULL;uint64_t p4=653733101ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=28ULL;uint64_t p4=3190467243ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=29ULL;uint64_t p4=1007192201ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=30ULL;uint64_t p4=3109031169ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=31ULL;uint64_t p4=909700893ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=32ULL;uint64_t p4=1026206379ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=33ULL;uint64_t p4=3132492641ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=34ULL;uint64_t p4=936381697ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=35ULL;uint64_t p4=3029594750ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=36ULL;uint64_t p4=1065353216ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
({uint64_t p3=37ULL;uint64_t p4=158ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
uint64_t __attribute__((unused)) v4=863ULL;
if(({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v4=841ULL;
}
({uint64_t p3=38ULL;uint64_t p4=({uint64_t _a=v4;uint64_t _b=20ULL;o_op(15, 5, _a,_b);});f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
{uint64_t _i=103ULL;uint64_t _v=({uint64_t p2=({uint64_t p2=4294967295ULL;f19(v0, v0_n, v1, v1_n, p2);});uint64_t p3=1ULL;uint64_t p4=65535ULL;f17(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=104ULL;uint64_t _v=({uint64_t p2=({uint64_t p2=1ULL;f19(v0, v0_n, v1, v1_n, p2);});uint64_t p3=1ULL;uint64_t p4=65535ULL;f17(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
({uint64_t p3=41ULL;uint64_t p4=2ULL;f76(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
}
return o_leave(0);
}
static uint64_t f78(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v1, v1_n, 223ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=95ULL;o_op(11, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=3ULL;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
{uint64_t _i=223ULL;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, ({uint64_t _a=128ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);}), 5);
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v3=({uint64_t _a=({uint64_t p1=1ULL;f12(v0, v0_n, p1);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
{uint64_t _i=({uint64_t _a=128ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
}
return o_leave(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);}));
}
static uint64_t f79(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({f78(v1, v1_n, v2, v2_n);});
({uint64_t p2=v3;uint64_t p3=({uint64_t p0=v6;uint64_t p1=v4;uint64_t p2=v5;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v6);
}
static uint64_t f80(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({f78(v1, v1_n, v2, v2_n);});
({uint64_t p2=33ULL;uint64_t p3=({uint64_t p0=v6;uint64_t p1=v3;uint64_t p2=v4;uint64_t p3=v5;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v6);
}
static uint64_t f81(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({f78(v1, v1_n, v2, v2_n);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v6;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=v5;f4(p0, p1, p2, p3);});uint64_t p4=v4;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v6);
}
static uint64_t f82(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({f78(v1, v1_n, v2, v2_n);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v6;uint64_t p1=v3;uint64_t p2=v4;uint64_t p3=v5;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v6);
}
static uint64_t f83(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({f78(v1, v1_n, v2, v2_n);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=v6;uint64_t p1=v3;uint64_t p2=v4;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=v5;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v6);
}
static uint64_t f84(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({f78(v1, v1_n, v2, v2_n);});
({uint64_t p2=17ULL;uint64_t p3=({uint64_t p0=v5;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=v4;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v5);
}
static uint64_t f85(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({f78(v1, v1_n, v2, v2_n);});
({uint64_t p2=18ULL;uint64_t p3=({uint64_t p0=v5;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=v4;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v5);
}
static uint64_t f86(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t _a=40ULL;uint64_t _b=({uint64_t _a=v5;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=o_get(v2, v2_n, v6, 5);
uint64_t __attribute__((unused)) v8=o_get(v2, v2_n, ({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v9=o_get(v2, v2_n, ({uint64_t _a=v6;uint64_t _b=2ULL;o_op(1, 5, _a,_b);}), 5);
uint64_t __attribute__((unused)) v10=o_get(v2, v2_n, ({uint64_t _a=v6;uint64_t _b=3ULL;o_op(1, 5, _a,_b);}), 5);
({uint64_t p2=9ULL;uint64_t p3=({uint64_t p0=v7;uint64_t p1=o_get(v2, v2_n, 58ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=3528531795ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=9ULL;uint64_t p3=({uint64_t p0=v8;uint64_t p1=o_get(v2, v2_n, 60ULL, 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=3449720151ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=v9;uint64_t p1=({uint64_t p0=v8;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p2=o_get(v2, v2_n, 59ULL, 5);uint64_t p3=v3;f4(p0, p1, p2, p3);});uint64_t p4=150ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=51ULL;uint64_t p3=({uint64_t p0=v10;uint64_t p1=({uint64_t p0=v7;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p2=o_get(v2, v2_n, 61ULL, 5);uint64_t p3=v4;f4(p0, p1, p2, p3);});uint64_t p4=150ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=58ULL;uint64_t _v=v9;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=59ULL;uint64_t _v=({uint64_t p0=v8;uint64_t p1=0ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=60ULL;uint64_t _v=v10;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=61ULL;uint64_t _v=({uint64_t p0=v7;uint64_t p1=0ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f87(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
{uint64_t _i=58ULL;uint64_t _v=({uint64_t p0=v3;uint64_t p1=0ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=59ULL;uint64_t _v=({uint64_t p0=v3;uint64_t p1=1ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=60ULL;uint64_t _v=({uint64_t p0=o_get(v2, v2_n, 3ULL, 5);uint64_t p1=0ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=61ULL;uint64_t _v=({uint64_t p0=o_get(v2, v2_n, 3ULL, 5);uint64_t p1=1ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v4);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
({uint64_t p3=o_get(v2, v2_n, ({uint64_t _a=10ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t p4=o_get(v2, v2_n, ({uint64_t _a=20ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);}), 5);uint64_t p5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});f86(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
}
}
return o_leave(0);
}
static uint64_t f88(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 56ULL, 5);
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v0, v0_n, 54ULL, 5);uint64_t _b=4ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(o_get(v0, v0_n, ({uint64_t _a=58ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);}), 5));
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=v2;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=4ULL;o_op(8, 5, _a,_b);})){
return o_leave(o_get(v0, v0_n, ({uint64_t _a=34ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5));
}
return o_leave(o_get(v0, v0_n, ({uint64_t _a=58ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(2, 5, _a,_b);});o_op(1, 5, _a,_b);}), 5));
}
static uint64_t f89(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(18446744073709551615ULL);
}
({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=48ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p3=o_get(v2, v2_n, 52ULL, 5);f27(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(({f32(v0, v0_n, v1, v1_n);}));
}
static uint64_t f90(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=18446744073709551615ULL;o_op(7, 5, _a,_b);})){
({uint64_t p2=v2;f33(v0, v0_n, v1, v1_n, p2);});
}
return o_leave(0);
}
static uint64_t f91(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p3=38ULL;uint64_t p4=({uint64_t p3=v3;uint64_t p4=16ULL;f84(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});uint64_t p5=65535ULL;f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
uint64_t __attribute__((unused)) v5=({uint64_t p3=38ULL;uint64_t p4=({uint64_t p3=v3;uint64_t p4=65535ULL;f85(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});uint64_t p5=65535ULL;f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
return o_leave(({uint64_t p3=v4;uint64_t p4=({uint64_t p1=0ULL;f75(v2, v2_n, p1);});uint64_t p5=v5;f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);}));
}
static uint64_t f92(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p3=v3;f91(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
if(({uint64_t _a=o_get(v2, v2_n, 54ULL, 5);uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p3=32ULL;uint64_t p4=v4;uint64_t p5=({uint64_t p1=1ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);}));
}
return o_leave(({uint64_t p3=v4;uint64_t p4=({uint64_t p1=1ULL;f75(v2, v2_n, p1);});uint64_t p5=({uint64_t p1=2ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);}));
}
static uint64_t f93(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t p2=v3;uint64_t p3=1ULL;uint64_t p4=o_get(v2, v2_n, 103ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=224ULL;uint64_t _v=({uint64_t p0=v6;uint64_t p1=1ULL;f6(p0, p1);});o_put(v2, v2_n, _i,_v,5);}
v6=({uint64_t p3=o_get(v2, v2_n, 224ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 8ULL, 5);f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=225ULL;uint64_t _v=({uint64_t p3=v4;uint64_t p4=v6;uint64_t p5=v3;f82(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=226ULL;uint64_t _v=({uint64_t p3=v4;uint64_t p4=o_get(v2, v2_n, 224ULL, 5);uint64_t p5=65535ULL;f82(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
v6=o_get(v2, v2_n, 225ULL, 5);
{
uint64_t v7=0;
uint64_t k1=5ULL;
for(;o_op(8,5, v7, k1);v7=o_op(1,5, v7,1)){
v6=({uint64_t p3=v6;uint64_t p4=({uint64_t p3=v6;uint64_t p4=({uint64_t _a=1ULL;uint64_t _b=v7;o_op(15, 5, _a,_b);});f84(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});uint64_t p5=252ULL;f83(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
}
}
v6=({uint64_t p3=29ULL;uint64_t p4=v6;uint64_t p5=({uint64_t p3=v6;uint64_t p4=1ULL;f84(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v6=({uint64_t p3=38ULL;uint64_t p4=v6;uint64_t p5=65535ULL;f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=227ULL;uint64_t _v=({uint64_t p3=({uint64_t p3=v6;uint64_t p4=23ULL;f84(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});uint64_t p4=255ULL;f85(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
v6=({uint64_t p3=o_get(v2, v2_n, 227ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=({uint64_t p1=37ULL;f75(v2, v2_n, p1);});f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v6=({uint64_t p3=52ULL;uint64_t p4=o_get(v2, v2_n, 8ULL, 5);uint64_t p5=v6;f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
uint64_t __attribute__((unused)) v7=({uint64_t p3=53ULL;uint64_t p4=o_get(v2, v2_n, 226ULL, 5);uint64_t p5=v6;f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=228ULL;uint64_t _v=({uint64_t p3=o_get(v2, v2_n, 225ULL, 5);uint64_t p4=v6;uint64_t p5=v7;f82(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=229ULL;uint64_t _v=({uint64_t p3=o_get(v2, v2_n, 226ULL, 5);uint64_t p4=v6;uint64_t p5=65535ULL;f82(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
v6=({uint64_t p3=o_get(v2, v2_n, 224ULL, 5);uint64_t p4=32ULL;uint64_t p5=o_get(v2, v2_n, 227ULL, 5);f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
if(({uint64_t _a=v5;uint64_t _b=65535ULL;o_op(7, 5, _a,_b);})){
v6=({uint64_t p3=14ULL;uint64_t p4=v6;uint64_t p5=v5;f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
}
v6=({uint64_t p3=v6;uint64_t p4=1048576ULL;uint64_t p5=({uint64_t p1=38ULL;f75(v2, v2_n, p1);});f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v7=({uint64_t p3=({uint64_t p3=o_get(v2, v2_n, 228ULL, 5);uint64_t p4=11ULL;f84(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});uint64_t p4=1ULL;uint64_t p5=v6;f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v6=({uint64_t p3=o_get(v2, v2_n, 228ULL, 5);uint64_t p4=2097152ULL;uint64_t p5=({uint64_t p3=o_get(v2, v2_n, 229ULL, 5);uint64_t p4=11ULL;f84(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
return o_leave(({uint64_t p0=v6;uint64_t p1=v7;f7(p0, p1);}));
}
static uint64_t f94(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
{uint64_t _i=224ULL;uint64_t _v=({uint64_t p3=v4;uint64_t p4=11ULL;f84(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=225ULL;uint64_t _v=({uint64_t p3=29ULL;uint64_t p4=v3;uint64_t p5=({uint64_t p3=v4;uint64_t p4=2097152ULL;uint64_t p5=o_get(v2, v2_n, 9ULL, 5);f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=226ULL;uint64_t _v=({uint64_t p3=o_get(v2, v2_n, 224ULL, 5);uint64_t p4=20ULL;f84(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=227ULL;uint64_t _v=({uint64_t p3=o_get(v2, v2_n, 226ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=o_get(v2, v2_n, 8ULL, 5);f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
v3=({uint64_t p3=o_get(v2, v2_n, 225ULL, 5);uint64_t p4=o_get(v2, v2_n, 227ULL, 5);uint64_t p5=65535ULL;f82(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p2=o_get(v2, v2_n, 225ULL, 5);uint64_t p3=1ULL;uint64_t p4=65535ULL;f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
v4=({uint64_t p2=v3;uint64_t p3=1ULL;uint64_t p4=v4;f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
v3=o_get(v2, v2_n, 227ULL, 5);
if(({uint64_t _a=o_get(v2, v2_n, 54ULL, 5);uint64_t _b=6ULL;o_op(7, 5, _a,_b);})){
v3=({uint64_t p3=o_get(v2, v2_n, 226ULL, 5);uint64_t p4=({uint64_t p3=o_get(v2, v2_n, 225ULL, 5);uint64_t p4=1ULL;f85(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});uint64_t p5=o_get(v2, v2_n, 227ULL, 5);f82(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
}
v4=({uint64_t p2=v3;uint64_t p3=1ULL;uint64_t p4=v4;f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
v3=({uint64_t p3=o_get(v2, v2_n, 224ULL, 5);uint64_t p4=({uint64_t p3=o_get(v2, v2_n, 226ULL, 5);uint64_t p4=4294967295ULL;uint64_t p5=({uint64_t p1=41ULL;f75(v2, v2_n, p1);});f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});uint64_t p5=({uint64_t p0=v4;uint64_t p1=1ULL;f6(p0, p1);});f82(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
return o_leave(({uint64_t p3=v3;uint64_t p4=({uint64_t p0=v4;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p5=o_get(v2, v2_n, 226ULL, 5);f93(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);}));
}
static uint64_t f95(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
{uint64_t _i=230ULL;uint64_t _v=({uint64_t p3=v3;f92(v0, v0_n, v1, v1_n, v2, v2_n, p3);});o_put(v2, v2_n, _i,_v,5);}
v3=({uint64_t p3=v4;f91(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
{uint64_t _i=231ULL;uint64_t _v=({uint64_t p3=30ULL;uint64_t p4=({uint64_t p3=32ULL;uint64_t p4=v3;uint64_t p5=({uint64_t p1=3ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});uint64_t p5=({uint64_t p1=4ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
v3=({uint64_t p3=o_get(v2, v2_n, 8ULL, 5);uint64_t p4=4913933ULL;uint64_t p5=o_get(v2, v2_n, 230ULL, 5);f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=v3;uint64_t p4=23ULL;f84(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
v3=({uint64_t p3=o_get(v2, v2_n, 8ULL, 5);uint64_t p4=1060439283ULL;uint64_t p5=({uint64_t p3=v3;uint64_t p4=8388607ULL;f85(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});f81(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=232ULL;uint64_t _v=({uint64_t p3=30ULL;uint64_t p4=v3;uint64_t p5=({uint64_t p1=5ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=233ULL;uint64_t _v=({uint64_t p3=30ULL;uint64_t p4=({uint64_t p3=38ULL;uint64_t p4=v4;uint64_t p5=65535ULL;f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});uint64_t p5=({uint64_t p1=6ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
v3=({uint64_t p1=15ULL;f75(v2, v2_n, p1);});
{
uint64_t v5=0;
uint64_t k1=8ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
v3=({uint64_t p3=v3;uint64_t p4=o_get(v2, v2_n, 232ULL, 5);uint64_t p5=({uint64_t p1=({uint64_t _a=14ULL;uint64_t _b=v5;o_op(2, 5, _a,_b);});f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
}
}
v3=({uint64_t p3=o_get(v2, v2_n, 232ULL, 5);uint64_t p4=v3;uint64_t p5=({uint64_t p1=16ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=32ULL;uint64_t p4=o_get(v2, v2_n, 232ULL, 5);uint64_t p5=o_get(v2, v2_n, 232ULL, 5);f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v3=({uint64_t p3=v4;uint64_t p4=v3;uint64_t p5=o_get(v2, v2_n, 232ULL, 5);f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v3=({uint64_t p3=o_get(v2, v2_n, 233ULL, 5);uint64_t p4=({uint64_t p1=17ULL;f75(v2, v2_n, p1);});uint64_t p5=({uint64_t p3=o_get(v2, v2_n, 233ULL, 5);uint64_t p4=({uint64_t p1=18ULL;f75(v2, v2_n, p1);});uint64_t p5=v3;f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=234ULL;uint64_t _v=({uint64_t p3=32ULL;uint64_t p4=v3;uint64_t p5=({uint64_t p1=19ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
v3=({uint64_t p3=37ULL;uint64_t p4=o_get(v2, v2_n, 234ULL, 5);uint64_t p5=({uint64_t p1=20ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({f78(v1, v1_n, v2, v2_n);});
({uint64_t p2=({uint64_t p0=41ULL;uint64_t p1=2ULL;uint64_t p2=0ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=v4;uint64_t p1=v3;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=235ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
v3=({uint64_t p3=32ULL;uint64_t p4=o_get(v2, v2_n, 234ULL, 5);uint64_t p5=v4;f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=({uint64_t p3=32ULL;uint64_t p4=v3;uint64_t p5=({uint64_t p1=5ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});uint64_t p4=v3;uint64_t p5=o_get(v2, v2_n, 234ULL, 5);f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=236ULL;uint64_t _v=({uint64_t p3=v4;uint64_t p4=({uint64_t p3=32ULL;uint64_t p4=o_get(v2, v2_n, 235ULL, 5);uint64_t p5=({uint64_t p1=21ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});uint64_t p5=v3;f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
v3=({uint64_t p3=30ULL;uint64_t p4=({uint64_t p3=o_get(v2, v2_n, 231ULL, 5);uint64_t p4=({uint64_t p1=22ULL;f75(v2, v2_n, p1);});uint64_t p5=({uint64_t p1=23ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});uint64_t p5=({uint64_t p1=24ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=237ULL;uint64_t _v=v3;o_put(v2, v2_n, _i,_v,5);}
v4=({uint64_t p3=v3;uint64_t p4=({uint64_t p1=25ULL;f75(v2, v2_n, p1);});uint64_t p5=o_get(v2, v2_n, 231ULL, 5);f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=v3;uint64_t p4=({uint64_t p1=26ULL;f75(v2, v2_n, p1);});uint64_t p5=v4;f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=v3;uint64_t p4=({uint64_t p1=27ULL;f75(v2, v2_n, p1);});uint64_t p5=v4;f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=238ULL;uint64_t _v=v4;o_put(v2, v2_n, _i,_v,5);}
v3=({uint64_t p3=32ULL;uint64_t p4=v4;uint64_t p5=v4;f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=v3;uint64_t p4=({uint64_t p1=31ULL;f75(v2, v2_n, p1);});uint64_t p5=({uint64_t p1=30ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=({uint64_t p1=29ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=({uint64_t p1=28ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=240ULL;uint64_t _v=({uint64_t p3=({uint64_t p3=32ULL;uint64_t p4=o_get(v2, v2_n, 238ULL, 5);uint64_t p5=v3;f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});uint64_t p4=v4;uint64_t p5=o_get(v2, v2_n, 238ULL, 5);f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
v4=({uint64_t p3=v3;uint64_t p4=({uint64_t p1=35ULL;f75(v2, v2_n, p1);});uint64_t p5=({uint64_t p1=34ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=({uint64_t p1=33ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=({uint64_t p1=32ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=({uint64_t p1=16ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=241ULL;uint64_t _v=({uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=({uint64_t p1=36ULL;f75(v2, v2_n, p1);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
v3=({uint64_t p3=30ULL;uint64_t p4=o_get(v2, v2_n, 237ULL, 5);uint64_t p5=({uint64_t p1=19ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=37ULL;uint64_t p4=v3;uint64_t p5=({uint64_t p3=32ULL;uint64_t p4=v3;uint64_t p5=({uint64_t p1=5ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
{uint64_t _i=242ULL;uint64_t _v=({uint64_t p3=30ULL;uint64_t p4=v4;uint64_t p5=({uint64_t p1=5ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=243ULL;uint64_t _v=({uint64_t p3=32ULL;uint64_t p4=v3;uint64_t p5=({uint64_t p3=30ULL;uint64_t p4=v4;uint64_t p5=({uint64_t p1=19ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
v3=({uint64_t p3=o_get(v2, v2_n, 240ULL, 5);uint64_t p4=o_get(v2, v2_n, 242ULL, 5);uint64_t p5=({uint64_t p3=32ULL;uint64_t p4=o_get(v2, v2_n, 241ULL, 5);uint64_t p5=o_get(v2, v2_n, 243ULL, 5);f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=o_get(v2, v2_n, 241ULL, 5);uint64_t p4=o_get(v2, v2_n, 242ULL, 5);uint64_t p5=({uint64_t p3=32ULL;uint64_t p4=o_get(v2, v2_n, 240ULL, 5);uint64_t p5=({uint64_t p3=32ULL;uint64_t p4=o_get(v2, v2_n, 243ULL, 5);uint64_t p5=({uint64_t p1=5ULL;f75(v2, v2_n, p1);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});f80(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v3=({uint64_t p3=32ULL;uint64_t p4=v3;uint64_t p5=o_get(v2, v2_n, 236ULL, 5);f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
v4=({uint64_t p3=32ULL;uint64_t p4=v4;uint64_t p5=o_get(v2, v2_n, 236ULL, 5);f79(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
if(({uint64_t _a=o_get(v2, v2_n, 54ULL, 5);uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v4;uint64_t p1=v3;f7(p0, p1);}));
}
return o_leave(({uint64_t p0=v3;uint64_t p1=v4;f7(p0, p1);}));
}
static uint64_t f96(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t p3=v4;f89(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=v3;uint64_t p3=({uint64_t _a=v5;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});uint64_t p4=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v3;uint64_t p3=({uint64_t _a=v5;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t p4=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;f90(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f97(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t p3=v4;f89(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
({uint64_t p2=v3;uint64_t p3=v5;uint64_t p4=({uint64_t _a=v4;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});uint64_t p5=o_get(v2, v2_n, 0ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v6;f90(v0, v0_n, v1, v1_n, p2);});
return o_leave(0);
}
static uint64_t f98(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 55ULL, 5);
if(({uint64_t _a=v4;uint64_t _b=2ULL;o_op(8, 5, _a,_b);})){
{
uint64_t v5=0;
uint64_t k2=4ULL;
for(;o_op(8,5, v5, k2);v5=o_op(1,5, v5,1)){
{uint64_t _i=223ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f88(v2, v2_n, p1);});
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v6=({uint64_t p3=v6;f92(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
}
({uint64_t p3=v3;uint64_t p4=v5;uint64_t p5=v6;f97(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
}
}
return o_leave(0);
}
if(({uint64_t _a=v4;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
{
uint64_t v5=0;
uint64_t k2=4ULL;
for(;o_op(8,5, v5, k2);v5=o_op(1,5, v5,1)){
{uint64_t _i=223ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=({uint64_t p2=({uint64_t p1=v5;f88(v2, v2_n, p1);});uint64_t p3=1ULL;uint64_t p4=o_get(v2, v2_n, 104ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p3=v3;uint64_t p4=v5;uint64_t p5=({uint64_t p3=({uint64_t p0=v6;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p4=({uint64_t p0=v6;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p5=65535ULL;f93(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});f96(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
}
}
return o_leave(0);
}
if(({uint64_t _a=v4;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
{
uint64_t v5=0;
uint64_t k2=2ULL;
for(;o_op(8,5, v5, k2);v5=o_op(1,5, v5,1)){
{uint64_t _i=223ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
({uint64_t p3=v3;uint64_t p4=v5;uint64_t p5=({uint64_t p3=({uint64_t p1=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});f88(v2, v2_n, p1);});uint64_t p4=({uint64_t p1=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});uint64_t _b=1ULL;o_op(1, 5, _a,_b);});f88(v2, v2_n, p1);});f94(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});f96(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
}
}
return o_leave(0);
}
{
uint64_t v5=0;
uint64_t k1=2ULL;
for(;o_op(8,5, v5, k1);v5=o_op(1,5, v5,1)){
{uint64_t _i=223ULL;uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{
uint64_t v6=0;
uint64_t k3=95ULL;
for(;o_op(8,5, v6, k3);v6=o_op(1,5, v6,1)){
{uint64_t _i=({uint64_t _a=128ULL;uint64_t _b=v6;o_op(1, 5, _a,_b);});uint64_t _v=0ULL;o_put(v2, v2_n, _i,_v,5);}
}
}
}
uint64_t __attribute__((unused)) v6=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t p3=({uint64_t p1=v6;f88(v2, v2_n, p1);});uint64_t p4=({uint64_t p1=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});f88(v2, v2_n, p1);});f95(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({uint64_t p3=v3;uint64_t p4=v6;uint64_t p5=({uint64_t _a=v7;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f97(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
({uint64_t p3=v3;uint64_t p4=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t p5=({uint64_t _a=v7;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});f97(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4, p5);});
}
}
return o_leave(0);
}
static uint64_t f99(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{
uint64_t v3=0;
uint64_t k1=4ULL;
for(;o_op(8,5, v3, k1);v3=o_op(1,5, v3,1)){
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=o_get(v2, v2_n, ({uint64_t _a=34ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p1=o_get(v2, v2_n, ({uint64_t _a=58ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);}), 5);uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
return o_leave(0);
}
static uint64_t f100(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
uint64_t __attribute__((unused)) v3=10ULL;
if(({uint64_t _a=o_get(v2, v2_n, 54ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v3=9ULL;
}
uint64_t __attribute__((unused)) v4=4ULL;
if(({uint64_t _a=({uint64_t _a=o_get(v2, v2_n, 55ULL, 5);uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=o_get(v2, v2_n, 55ULL, 5);uint64_t _b=3ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
v4=8ULL;
}
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 4ULL, 5);
if(({uint64_t _a=o_get(v2, v2_n, 56ULL, 5);uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
({uint64_t p3=v5;uint64_t p4=v3;f87(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
({f99(v0, v0_n, v1, v1_n, v2, v2_n);});
({uint64_t p2=9ULL;uint64_t p3=({uint64_t p0=v5;uint64_t p1=o_get(v2, v2_n, 8ULL, 5);uint64_t p2=65535ULL;uint64_t p3=v5;f4(p0, p1, p2, p3);});uint64_t p4=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
({f30(v1, v1_n);});
({uint64_t p3=v5;uint64_t p4=v3;f87(v0, v0_n, v1, v1_n, v2, v2_n, p3, p4);});
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 5ULL, 5);
uint64_t __attribute__((unused)) v7=({uint64_t p2=({uint64_t p2=v6;uint64_t p3=1ULL;uint64_t p4=o_get(v2, v2_n, 38ULL, 5);f16(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p3=31ULL;f18(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v8=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v8;uint64_t p1=v7;uint64_t p2=o_get(v2, v2_n, 30ULL, 5);uint64_t p3=o_get(v2, v2_n, 33ULL, 5);f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=52ULL;uint64_t _v=v8;o_put(v2, v2_n, _i,_v,5);}
v8=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v8;uint64_t p1=v7;uint64_t p2=o_get(v2, v2_n, 31ULL, 5);uint64_t p3=o_get(v2, v2_n, 7ULL, 5);f4(p0, p1, p2, p3);});uint64_t p4=0ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p3=({uint64_t p2=v8;uint64_t p3=v4;uint64_t p4=o_get(v2, v2_n, 1ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});f98(v0, v0_n, v1, v1_n, v2, v2_n, p3);});
if(({uint64_t _a=o_get(v2, v2_n, 56ULL, 5);uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
({f99(v0, v0_n, v1, v1_n, v2, v2_n);});
}
({uint64_t p2=9ULL;uint64_t p3=({uint64_t p0=v5;uint64_t p1=o_get(v2, v2_n, 8ULL, 5);uint64_t p2=65535ULL;uint64_t p3=v5;f4(p0, p1, p2, p3);});uint64_t p4=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
v7=o_get(v2, v2_n, 7ULL, 5);
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v7;uint64_t p1=o_get(v2, v2_n, 32ULL, 5);uint64_t p2=65535ULL;uint64_t p3=v7;f4(p0, p1, p2, p3);});uint64_t p4=1ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=8ULL;uint64_t p3=({uint64_t p0=v6;uint64_t p1=o_get(v2, v2_n, 8ULL, 5);uint64_t p2=65535ULL;uint64_t p3=v6;f4(p0, p1, p2, p3);});uint64_t p4=4294967295ULL;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=o_get(v2, v2_n, 9ULL, 5);uint64_t p3=v6;f27(v0, v0_n, v1, v1_n, p2, p3);});
({f34(v0, v0_n, v1, v1_n);});
({f31(v1, v1_n);});
return o_leave(0);
}
static uint64_t f101(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t v5[256]={0};
uint64_t v5_n=256;
o_fill(v5, v5_n, 0ULL, 5);
{uint64_t _i=54ULL;uint64_t _v=v4;o_put(v5, v5_n, _i,_v,5);}
{uint64_t _i=55ULL;uint64_t _v=v2;o_put(v5, v5_n, _i,_v,5);}
{uint64_t _i=56ULL;uint64_t _v=v3;o_put(v5, v5_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=4ULL;
if(({uint64_t _a=v2;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v6=2ULL;
}
({uint64_t p3=v6;f73(v0, v0_n, v1, v1_n, v5, v5_n, p3);});
({f74(v0, v0_n, v1, v1_n, v5, v5_n);});
({f77(v0, v0_n, v1, v1_n, v5, v5_n);});
{
uint64_t v7=0;
uint64_t k1=2ULL;
for(;o_op(8,5, v7, k1);v7=o_op(1,5, v7,1)){
uint64_t __attribute__((unused)) v8=({uint64_t _a=40ULL;uint64_t _b=({uint64_t _a=v7;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});
{uint64_t _i=v8;uint64_t _v=({uint64_t p1=2ULL;f12(v1, v1_n, p1);});o_put(v5, v5_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v8;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=2ULL;f12(v1, v1_n, p1);});o_put(v5, v5_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v8;uint64_t _b=2ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});o_put(v5, v5_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=v8;uint64_t _b=3ULL;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});o_put(v5, v5_n, _i,_v,5);}
}
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
{
uint64_t v7=0;
uint64_t k2=4ULL;
for(;o_op(8,5, v7, k2);v7=o_op(1,5, v7,1)){
{uint64_t _i=({uint64_t _a=34ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});o_put(v5, v5_n, _i,_v,5);}
}
}
}
({f100(v0, v0_n, v1, v1_n, v5, v5_n);});
({f29(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f102(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p1=1ULL;f12(v1, v1_n, p1);});
({uint64_t p2=18ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=v2;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=v3;f14(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(v4);
}
static uint64_t f103(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p2=32ULL;uint64_t p3=v2;uint64_t p4=({uint64_t p0=v4;uint64_t p1=0ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=32ULL;uint64_t p3=v3;uint64_t p4=({uint64_t p0=v4;uint64_t p1=1ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=32ULL;uint64_t p3=v3;uint64_t p4=({uint64_t p0=v4;uint64_t p1=0ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=v2;uint64_t p3=({uint64_t p0=v4;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p4=v7;f20(v0, v0_n, v1, v1_n, p2, p3, p4);});
return o_leave(({uint64_t p0=({uint64_t p2=31ULL;uint64_t p3=v5;uint64_t p4=v6;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p1=v8;f7(p0, p1);}));
}
static uint64_t f104(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(15, 5, _a,_b);});
{uint64_t _i=0ULL;uint64_t _v=({uint64_t p1=2ULL;f13(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=({uint64_t p0=7ULL;uint64_t p1=3ULL;uint64_t p2=1ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 0ULL, 5);uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=856ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=1ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=896ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=904ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=912ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=({uint64_t p2=3ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=37ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=3ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=33ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
{uint64_t _i=4ULL;uint64_t _v=v7;o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v8=({uint64_t _a=v5;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});uint64_t _b=v4;o_op(2, 5, _a,_b);});
{uint64_t _i=1ULL;uint64_t _v=({uint64_t p2=v6;uint64_t p3=({uint64_t _a=v8;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t p4=o_get(v2, v2_n, 1ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t p2=v6;uint64_t p3=({uint64_t _a=v9;uint64_t _b=8ULL;o_op(3, 5, _a,_b);});uint64_t p4=o_get(v2, v2_n, 2ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v10=({uint64_t p2=v7;uint64_t p3=4294967295ULL;uint64_t p4=({uint64_t p2=v5;f19(v0, v0_n, v1, v1_n, p2);});f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=5ULL;uint64_t _v=v10;o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=({uint64_t p2=v10;uint64_t p3=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f102(v0, v0_n, v1, v1_n, p2, p3);});o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f105(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, void *v3, uint64_t v3_n, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 0ULL, 5);
{uint64_t _i=0ULL;uint64_t _v=({uint64_t p2=({uint64_t p2=o_get(v2, v2_n, 4ULL, 5);uint64_t p3=8ULL;uint64_t p4=o_get(v2, v2_n, 1ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p3=0ULL;uint64_t p4=v5;f23(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t p2=({uint64_t p2=o_get(v2, v2_n, ({uint64_t _a=6ULL;uint64_t _b=v4;o_op(2, 5, _a,_b);}), 5);uint64_t p3=8ULL;uint64_t p4=o_get(v2, v2_n, 1ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p3=0ULL;uint64_t p4=v5;f23(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v3, v3_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t p2=({uint64_t p2=o_get(v2, v2_n, 4ULL, 5);uint64_t p3=8ULL;uint64_t p4=o_get(v2, v2_n, 3ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p3=0ULL;uint64_t p4=v5;f23(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v3, v3_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f106(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 0ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 1ULL, 5);
uint64_t __attribute__((unused)) v6=({uint64_t p2=({uint64_t p0=1056964608ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});
{uint64_t _i=3ULL;uint64_t _v=({uint64_t p2=32ULL;uint64_t p3=({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p4=({uint64_t p0=v5;uint64_t p1=0ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=v6;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=({uint64_t p2=32ULL;uint64_t p3=({uint64_t p2=31ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p4=({uint64_t p0=v5;uint64_t p1=1ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=v6;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=({uint64_t p2=32ULL;uint64_t p3=({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p4=({uint64_t p0=v5;uint64_t p1=1ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=v6;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=32ULL;uint64_t p3=({uint64_t p2=31ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p4=({uint64_t p0=v5;uint64_t p1=0ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=({uint64_t p2=({uint64_t p0=3204448256ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v9=({uint64_t p0=v7;uint64_t p1=v8;f7(p0, p1);});
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
v9=({uint64_t p2=v7;uint64_t p3=v8;uint64_t p4=o_get(v2, v2_n, 2ULL, 5);f103(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
{uint64_t _i=5ULL;uint64_t _v=v9;o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f107(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v3, v3_n, 3ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v3, v3_n, 4ULL, 5);
uint64_t __attribute__((unused)) v6=({uint64_t _a=o_get(v3, v3_n, 5ULL, 5);uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=o_get(v3, v3_n, 5ULL, 5);uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=o_get(v2, v2_n, 0ULL, 5);
uint64_t __attribute__((unused)) v9=({uint64_t p2=o_get(v2, v2_n, 4ULL, 5);uint64_t p3=8ULL;uint64_t p4=o_get(v2, v2_n, 2ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v9;uint64_t p3=({uint64_t p2=30ULL;uint64_t p3=v4;uint64_t p4=v6;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=0ULL;uint64_t p5=v8;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v9;uint64_t p3=({uint64_t p2=30ULL;uint64_t p3=v5;uint64_t p4=v7;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=4ULL;uint64_t p5=v8;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
v9=({uint64_t p2=o_get(v2, v2_n, 5ULL, 5);uint64_t p3=8ULL;uint64_t p4=o_get(v2, v2_n, 2ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v9;uint64_t p3=({uint64_t p2=31ULL;uint64_t p3=v4;uint64_t p4=v6;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=0ULL;uint64_t p5=v8;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v9;uint64_t p3=({uint64_t p2=31ULL;uint64_t p3=v7;uint64_t p4=v5;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=4ULL;uint64_t p5=v8;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f108(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v2, v2_n, 0ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v2, v2_n, 1ULL, 5);
{uint64_t _i=3ULL;uint64_t _v=({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p4=({uint64_t p0=v5;uint64_t p1=0ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=4ULL;uint64_t _v=({uint64_t p2=31ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p4=({uint64_t p0=v5;uint64_t p1=1ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v6=({uint64_t p2=31ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p4=({uint64_t p0=v5;uint64_t p1=0ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=30ULL;uint64_t p3=({uint64_t p0=v4;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p4=({uint64_t p0=v5;uint64_t p1=1ULL;f6(p0, p1);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v8=({uint64_t p0=v6;uint64_t p1=v7;f7(p0, p1);});
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
v8=({uint64_t p2=v6;uint64_t p3=v7;uint64_t p4=o_get(v2, v2_n, 2ULL, 5);f103(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
{uint64_t _i=5ULL;uint64_t _v=v8;o_put(v2, v2_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f109(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, const void *v3, uint64_t v3_n){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v3, v3_n, 3ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v3, v3_n, 4ULL, 5);
uint64_t __attribute__((unused)) v6=({uint64_t _a=o_get(v3, v3_n, 5ULL, 5);uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=o_get(v3, v3_n, 5ULL, 5);uint64_t _b=16ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=o_get(v2, v2_n, 0ULL, 5);
uint64_t __attribute__((unused)) v9=({uint64_t p2=o_get(v2, v2_n, 6ULL, 5);uint64_t p3=8ULL;uint64_t p4=o_get(v2, v2_n, 2ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v9;uint64_t p3=({uint64_t p2=30ULL;uint64_t p3=v4;uint64_t p4=v7;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=0ULL;uint64_t p5=v8;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v9;uint64_t p3=({uint64_t p2=31ULL;uint64_t p3=v6;uint64_t p4=v5;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=4ULL;uint64_t p5=v8;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
v9=({uint64_t p2=o_get(v2, v2_n, 4ULL, 5);uint64_t p3=8ULL;uint64_t p4=o_get(v2, v2_n, 2ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v9;uint64_t p3=({uint64_t p2=31ULL;uint64_t p3=v4;uint64_t p4=v7;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=0ULL;uint64_t p5=v8;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v9;uint64_t p3=({uint64_t p2=30ULL;uint64_t p3=v5;uint64_t p4=v6;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=4ULL;uint64_t p5=v8;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f110(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, const void *v2, uint64_t v2_n, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=o_get(v2, v2_n, 0ULL, 5);
uint64_t __attribute__((unused)) v7=({uint64_t p2=o_get(v2, v2_n, 1ULL, 5);uint64_t p3=v5;uint64_t p4=v6;f23(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=o_get(v2, v2_n, 2ULL, 5);uint64_t p3=({uint64_t p0=v7;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p4=v5;uint64_t p5=v6;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 2ULL, 5);uint64_t p3=({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=v7;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p4=({uint64_t p2=({uint64_t p0=3212836864ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=({uint64_t _a=v5;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t p5=v6;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
({uint64_t p2=o_get(v2, v2_n, 2ULL, 5);uint64_t p3=({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=v7;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p4=({uint64_t p2=({uint64_t p0=1073741824ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=v5;uint64_t p5=v6;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=o_get(v2, v2_n, 2ULL, 5);uint64_t p3=({uint64_t p2=32ULL;uint64_t p3=({uint64_t p0=v7;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p4=({uint64_t p2=({uint64_t p0=3221225472ULL;f0(p0);});f19(v0, v0_n, v1, v1_n, p2);});f15(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=({uint64_t _a=v5;uint64_t _b=4ULL;o_op(1, 5, _a,_b);});uint64_t p5=v6;f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
return o_leave(0);
}
static uint64_t f111(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t v5[16]={0};
uint64_t v5_n=16;
o_fill(v5, v5_n, 0ULL, 5);
uint64_t v6[8]={0};
uint64_t v6_n=8;
o_fill(v6, v6_n, 0ULL, 5);
({uint64_t p3=v2;uint64_t p4=v3;f104(v0, v0_n, v1, v1_n, v5, v5_n, p3, p4);});
({uint64_t p4=v3;f105(v0, v0_n, v1, v1_n, v5, v5_n, v6, v6_n, p4);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p3=v4;f106(v0, v0_n, v1, v1_n, v6, v6_n, p3);});
({f107(v0, v0_n, v1, v1_n, v5, v5_n, v6, v6_n);});
}else{
({uint64_t p3=v4;f108(v0, v0_n, v1, v1_n, v6, v6_n, p3);});
({f109(v0, v0_n, v1, v1_n, v5, v5_n, v6, v6_n);});
}
if(({uint64_t _a=v4;uint64_t _b=2ULL;o_op(7, 5, _a,_b);})){
({uint64_t p3=v2;uint64_t p4=v3;f110(v0, v0_n, v1, v1_n, v5, v5_n, p3, p4);});
}
({f29(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f112(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, void *v2, uint64_t v2_n){
o_enter();
{uint64_t _i=0ULL;uint64_t _v=({uint64_t p1=2ULL;f13(v1, v1_n, p1);});o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=({uint64_t p0=7ULL;uint64_t p1=3ULL;uint64_t p2=1ULL;uint64_t p3=65535ULL;f5(p0, p1, p2, p3);});uint64_t p3=({uint64_t p0=o_get(v2, v2_n, 0ULL, 5);uint64_t p1=65535ULL;uint64_t p2=65535ULL;uint64_t p3=65535ULL;f4(p0, p1, p2, p3);});uint64_t p4=856ULL;f11(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=1ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=896ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t p2=5ULL;uint64_t p3=2ULL;uint64_t p4=65535ULL;uint64_t p5=904ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});o_put(v2, v2_n, _i,_v,5);}
uint64_t __attribute__((unused)) v3=({uint64_t p2=4ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=912ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v4=({uint64_t p2=4ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=916ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v5=({uint64_t p2=3ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=37ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=3ULL;uint64_t p3=1ULL;uint64_t p4=65535ULL;uint64_t p5=33ULL;f22(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=14ULL;uint64_t p3=({uint64_t p2=v5;uint64_t p3=256ULL;uint64_t p4=v4;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p4=v6;f15(v0, v0_n, v1, v1_n, p2, p3, p4);});
{uint64_t _i=3ULL;uint64_t _v=v7;o_put(v2, v2_n, _i,_v,5);}
({uint64_t p2=v7;uint64_t p3=v3;f27(v0, v0_n, v1, v1_n, p2, p3);});
({f28(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f113(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(v2);
}
uint64_t __attribute__((unused)) v6=({uint64_t p2=v2;uint64_t p3=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v4;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f102(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v7=({uint64_t p2=({uint64_t p2=v2;uint64_t p3=v4;f18(v0, v0_n, v1, v1_n, p2, p3);});uint64_t p3=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});f102(v0, v0_n, v1, v1_n, p2, p3);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=v6;uint64_t p3=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});uint64_t p4=v7;f16(v0, v0_n, v1, v1_n, p2, p3, p4);});
if(({uint64_t _a=v5;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(v8);
}
uint64_t __attribute__((unused)) v9=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=v4;o_op(1, 5, _a,_b);});o_op(15, 5, _a,_b);});
v9=({uint64_t _a=({uint64_t _a=0ULL;uint64_t _b=v9;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v10=({uint64_t p2=v2;uint64_t p3=v9;f102(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(({uint64_t p2=14ULL;uint64_t p3=v8;uint64_t p4=v10;f15(v0, v0_n, v1, v1_n, p2, p3, p4);}));
}
static uint64_t f114(void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t v5[8]={0};
uint64_t v5_n=8;
o_fill(v5, v5_n, 0ULL, 5);
({f112(v0, v0_n, v1, v1_n, v5, v5_n);});
uint64_t __attribute__((unused)) v6=o_get(v5, v5_n, 3ULL, 5);
uint64_t __attribute__((unused)) v7=({uint64_t p2=({uint64_t p2=v6;uint64_t p3=8ULL;uint64_t p4=o_get(v5, v5_n, 1ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});uint64_t p3=0ULL;uint64_t p4=o_get(v5, v5_n, 0ULL, 5);f23(v0, v0_n, v1, v1_n, p2, p3, p4);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=({uint64_t p2=v6;uint64_t p3=v2;uint64_t p4=v3;uint64_t p5=v4;f113(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});uint64_t p3=8ULL;uint64_t p4=o_get(v5, v5_n, 2ULL, 5);f17(v0, v0_n, v1, v1_n, p2, p3, p4);});
({uint64_t p2=v8;uint64_t p3=({uint64_t p0=v7;uint64_t p1=0ULL;f6(p0, p1);});uint64_t p4=0ULL;uint64_t p5=o_get(v5, v5_n, 0ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({uint64_t p2=v8;uint64_t p3=({uint64_t p0=v7;uint64_t p1=1ULL;f6(p0, p1);});uint64_t p4=4ULL;uint64_t p5=o_get(v5, v5_n, 0ULL, 5);f24(v0, v0_n, v1, v1_n, p2, p3, p4, p5);});
({f29(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f115(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v1;uint64_t _b=8ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=24ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=40ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f116(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=12ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
{uint64_t _i=0ULL;uint64_t _v=2ULL;o_put(v0, v0_n, _i,_v,5);}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=1ULL;uint64_t _v=({uint64_t p0=1ULL;uint64_t p1=v3;uint64_t p2=0ULL;uint64_t p3=0ULL;uint64_t p4=2ULL;uint64_t p5=0ULL;f115(p0, p1, p2, p3, p4, p5);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t p0=4ULL;uint64_t p1=v2;uint64_t p2=0ULL;uint64_t p3=2ULL;uint64_t p4=1ULL;uint64_t p5=0ULL;f115(p0, p1, p2, p3, p4, p5);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t p0=4ULL;uint64_t p1=v2;uint64_t p2=1ULL;uint64_t p3=0ULL;uint64_t p4=2ULL;uint64_t p5=0ULL;f115(p0, p1, p2, p3, p4, p5);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=2ULL;uint64_t _v=({uint64_t p0=1ULL;uint64_t p1=v3;uint64_t p2=1ULL;uint64_t p3=2ULL;uint64_t p4=1ULL;uint64_t p5=0ULL;f115(p0, p1, p2, p3, p4, p5);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
static uint64_t f117(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=4ULL;uint64_t _b=v1;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v1);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=4ULL;uint64_t _b=v5;o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=15ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v6;uint64_t _b=2ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=11ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
v4=({uint64_t _a=v4;uint64_t _b=v6;o_op(1, 5, _a,_b);});
}
}
if(({uint64_t _a=v4;uint64_t _b=27ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=0ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=({uint64_t p0=1ULL;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=0ULL;uint64_t p4=1ULL;uint64_t p5=0ULL;f115(p0, p1, p2, p3, p4, p5);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0ULL);
}
{uint64_t _i=0ULL;uint64_t _v=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
{
uint64_t v5=0;
uint64_t c1=0;
for(;o_op(8,5, v5, v1);v5=o_op(1,5, v5,1)){
if(c1++>=1000000)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=4ULL;uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});uint64_t _b=v5;o_op(2, 5, _a,_b);});o_op(3, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=15ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=1ULL;
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v7=0ULL;
}
{uint64_t _i=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=1ULL;uint64_t p1=v6;uint64_t p2=v3;uint64_t p3=v7;uint64_t p4=2ULL;uint64_t p5=({uint64_t _a=v4;uint64_t _b=v6;o_op(2, 5, _a,_b);});f115(p0, p1, p2, p3, p4, p5);});o_put(v0, v0_n, _i,_v,5);}
{uint64_t _i=({uint64_t _a=2ULL;uint64_t _b=({uint64_t _a=v5;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});o_op(1, 5, _a,_b);});uint64_t _v=({uint64_t p0=5ULL;uint64_t p1=({uint64_t _a=v4;uint64_t _b=v6;o_op(2, 5, _a,_b);});uint64_t p2=v6;uint64_t p3=2ULL;uint64_t p4=1ULL;uint64_t p5=0ULL;f115(p0, p1, p2, p3, p4, p5);});o_put(v0, v0_n, _i,_v,5);}
}
}
return o_leave(0ULL);
}
static uint64_t f118(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=15ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(16, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=2ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(({uint64_t p1=v4;uint64_t p2=v2;f116(v0, v0_n, p1, p2);}));
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=3ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(({uint64_t p1=v5;uint64_t p2=v2;uint64_t p3=v3;f117(v0, v0_n, p1, p2, p3);}));
}
static uint64_t f119(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, void *v4, uint64_t v4_n, void *v5, uint64_t v5_n){
o_enter();
{
uint64_t v6=0;
uint64_t k1=512ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
{uint64_t _i=v6;uint64_t _v=0ULL;o_put(v5, v5_n, _i,_v,5);}
}
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=12ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
({uint64_t p1=v1;uint64_t p2=v2;f72(v4, v4_n, p1, p2);});
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;f118(v4, v4_n, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
({uint64_t p2=v1;uint64_t p3=v3;f54(v4, v4_n, v5, v5_n, p2, p3);});
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=11ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
({uint64_t p2=v1;uint64_t p3=v2;uint64_t p4=v3;f67(v4, v4_n, v5, v5_n, p2, p3, p4);});
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=3ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=6ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
({uint64_t p2=v1;uint64_t p3=v2;uint64_t p4=v3;f101(v4, v4_n, v5, v5_n, p2, p3, p4);});
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=3ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=12ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
({uint64_t p2=v1;uint64_t p3=v2;uint64_t p4=v3;f111(v4, v4_n, v5, v5_n, p2, p3, p4);});
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=25ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=11ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=v2;o_op(1, 5, _a,_b);});uint64_t _b=27ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
({uint64_t p2=v1;uint64_t p3=v2;uint64_t p4=v3;f114(v4, v4_n, v5, v5_n, p2, p3, p4);});
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(10, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=o_get(v5, v5_n, 4ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=o_get(v5, v5_n, 6ULL, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
({f37(v4, v4_n, v5, v5_n);});
return o_leave(0ULL);
}
uint32_t omega_mathlibs_ir(uint64_t *result, volatile uint64_t p0, volatile uint64_t p1, volatile uint64_t p2, volatile uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n){
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
if(!code)*result=f119(p0, p1, p2, p3, p4, p4_n, p5, p5_n);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
