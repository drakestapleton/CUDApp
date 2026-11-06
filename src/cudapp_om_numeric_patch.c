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
static uint64_t f1(uint64_t v0);
static uint64_t f2(uint64_t v0);
static uint64_t f3(uint64_t v0);
static uint64_t f4(uint64_t v0);
static uint64_t f5(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f6(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f7(uint64_t v0, uint64_t v1);
static uint64_t f8(uint64_t v0);
static uint64_t f9(uint64_t v0);
static uint64_t f10(uint64_t v0);
static uint64_t f11(uint64_t v0);
static uint64_t f12(uint64_t v0);
static uint64_t f13(uint64_t v0, uint64_t v1);
static uint64_t f14(uint64_t v0, uint64_t v1);
static uint64_t f15(uint64_t v0, uint64_t v1);
static uint64_t f16(uint64_t v0, uint64_t v1);
static uint64_t f17(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f18(uint64_t v0, uint64_t v1);
static uint64_t f19(uint64_t v0, uint64_t v1);
static uint64_t f20(uint64_t v0, uint64_t v1);
static uint64_t f21(uint64_t v0);
static uint64_t f22(uint64_t v0);
static uint64_t f23(uint64_t v0);
static uint64_t f24(uint64_t v0, uint64_t v1);
static uint64_t f25(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f26(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f27(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f28(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f29(uint64_t v0);
static uint64_t f30(uint64_t v0);
static uint64_t f31(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f32(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f33(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f34(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f35(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f36(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f37(uint64_t v0, uint64_t v1);
static uint64_t f38(uint64_t v0, uint64_t v1);
static uint64_t f39(uint64_t v0);
static uint64_t f40(uint64_t v0);
static uint64_t f41(uint64_t v0, uint64_t v1);
static uint64_t f42(uint64_t v0);
static uint64_t f43(uint64_t v0);
static uint64_t f44(uint64_t v0);
static uint64_t f45(uint64_t v0, uint64_t v1);
static uint64_t f46(uint64_t v0, uint64_t v1);
static uint64_t f47(uint64_t v0);
static uint64_t f48(uint64_t v0, uint64_t v1);
static uint64_t f49(uint64_t v0);
static uint64_t f50(uint64_t v0, uint64_t v1);
static uint64_t f51(uint64_t v0);
static uint64_t f52(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f53(uint64_t v0);
static uint64_t f54(uint64_t v0);
static uint64_t f55(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f56(uint64_t v0);
static uint64_t f57(uint64_t v0, uint64_t v1);
static uint64_t f58(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f59(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f60(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f61(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f62(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f63(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f64(uint64_t v0);
static uint64_t f65(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f66(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f0(uint64_t v0){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});uint64_t _b=2139095040ULL;o_op(10, 5, _a,_b);}));
}
static uint64_t f1(uint64_t v0){
o_enter();
if(({uint64_t p0=v0;f0(p0);})){
return o_leave(2147483647ULL);
}
return o_leave(v0);
}
static uint64_t f2(uint64_t v0){
o_enter();
return o_leave(({uint64_t _a=({uint64_t p0=v0;f0(p0);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=4194304ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);}));
}
static uint64_t f3(uint64_t v0){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);}));
}
static uint64_t f4(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=63ULL;
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(c1++>=64)o_trap(4);
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f5(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(9, 9, _a,_b);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=o_cast(5, 9, ({uint64_t _a=v1;uint64_t _b=0;o_op(17, 9, _a,_b);}));o_op(15, 5, _a,_b);}));
}
if(({uint64_t _a=v1;uint64_t _b=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 9, _a,_b);});o_op(10, 9, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v3=o_cast(5, 9, v1);
uint64_t __attribute__((unused)) v4=0ULL;
if(({uint64_t _a=v3;uint64_t _b=64ULL;o_op(8, 5, _a,_b);})){
v4=({uint64_t _a=v0;uint64_t _b=v3;o_op(16, 5, _a,_b);});
}
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=1ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(12, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v6;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(12, 1, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
return o_leave(v4);
}
static uint64_t f6(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=o_cast(9, 5, ({uint64_t p0=v0;f4(p0);}));
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=v1;o_op(1, 9, _a,_b);});uint64_t _b=v3;o_op(1, 9, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v4;uint64_t _b=o_cast(5, 9, v2);o_op(15, 5, _a,_b);});
if(({uint64_t _a=v6;uint64_t _b=o_cast(9, 5, v4);o_op(11, 9, _a,_b);})){
return o_leave(v7);
}
uint64_t __attribute__((unused)) v8=({uint64_t _a=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(2, 9, _a,_b);});uint64_t _b=v2;o_op(2, 9, _a,_b);});uint64_t _b=v1;o_op(2, 9, _a,_b);});
if(({uint64_t _a=v6;uint64_t _b=1ULL;o_op(11, 9, _a,_b);})){
v8=({uint64_t _a=v5;uint64_t _b=v2;o_op(2, 9, _a,_b);});
}
uint64_t __attribute__((unused)) v9=({uint64_t p0=v0;uint64_t p1=v8;uint64_t p2=v5;f5(p0, p1, p2);});
uint64_t __attribute__((unused)) v10=v9;
if(({uint64_t _a=v6;uint64_t _b=1ULL;o_op(11, 9, _a,_b);})){
v10=({uint64_t _a=({uint64_t _a=o_cast(5, 9, ({uint64_t _a=v6;uint64_t _b=1ULL;o_op(2, 9, _a,_b);}));uint64_t _b=o_cast(5, 9, v2);o_op(15, 5, _a,_b);});uint64_t _b=v9;o_op(1, 5, _a,_b);});
}
if(({uint64_t _a=v10;uint64_t _b=v7;o_op(11, 5, _a,_b);})){
return o_leave(v7);
}
return o_leave(v10);
}
static uint64_t f7(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(({uint64_t p0=v0;uint64_t p1=v1;uint64_t p2=23ULL;uint64_t p3=127ULL;uint64_t p4=255ULL;f6(p0, p1, p2, p3, p4);}));
}
static uint64_t f8(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=v0;uint64_t _b=8388607ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=23ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=8388608ULL;o_op(13, 5, _a,_b);}));
}
return o_leave(v1);
}
static uint64_t f9(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=23ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t _a=o_cast(9, 5, v1);uint64_t _b=150ULL;o_op(2, 9, _a,_b);}));
}
return o_leave(18446744073709551467ULL);
}
static uint64_t f10(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t p0=v0;f8(p0);});
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8388608ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(c1++>=24)o_trap(4);
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f11(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t p0=v0;f8(p0);});
uint64_t __attribute__((unused)) v2=({uint64_t p0=v0;f9(p0);});
{
uint64_t c1=0;
while(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8388608ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(c1++>=24)o_trap(4);
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 9, _a,_b);});
}
}
return o_leave(v2);
}
static uint64_t f12(uint64_t v0){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t _a=2147483648ULL;uint64_t _b=({uint64_t _a=v0;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});o_op(2, 5, _a,_b);}));
}
return o_leave(({uint64_t _a=2147483648ULL;uint64_t _b=v0;o_op(1, 5, _a,_b);}));
}
static uint64_t f13(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t p0=v0;f0(p0);})){
return o_leave(({uint64_t p0=v1;f1(p0);}));
}
if(({uint64_t p0=v1;f0(p0);})){
return o_leave(v0);
}
if(({uint64_t _a=({uint64_t p0=v0;f3(p0);});uint64_t _b=({uint64_t p0=v1;f3(p0);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(13, 5, _a,_b);});uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t p0=v0;f12(p0);});uint64_t _b=({uint64_t p0=v1;f12(p0);});o_op(8, 5, _a,_b);})){
return o_leave(v0);
}
return o_leave(v1);
}
static uint64_t f14(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t p0=v0;f0(p0);})){
return o_leave(({uint64_t p0=v1;f1(p0);}));
}
if(({uint64_t p0=v1;f0(p0);})){
return o_leave(v0);
}
if(({uint64_t _a=({uint64_t p0=v0;f3(p0);});uint64_t _b=({uint64_t p0=v1;f3(p0);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(12, 5, _a,_b);});uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t p0=v0;f12(p0);});uint64_t _b=({uint64_t p0=v1;f12(p0);});o_op(10, 5, _a,_b);})){
return o_leave(v0);
}
return o_leave(v1);
}
static uint64_t f15(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=({uint64_t p0=v0;f0(p0);});uint64_t _b=({uint64_t p0=v1;f0(p0);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t p0=v0;f12(p0);});uint64_t _b=({uint64_t p0=v1;f12(p0);});o_op(11, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f16(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=({uint64_t p0=v0;f0(p0);});uint64_t _b=({uint64_t p0=v1;f0(p0);});o_op(13, 1, _a,_b);})){
return o_leave(8ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t p0=v0;f12(p0);});
uint64_t __attribute__((unused)) v3=({uint64_t p0=v1;f12(p0);});
if(({uint64_t _a=v2;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v2;uint64_t _b=v3;o_op(10, 5, _a,_b);})){
return o_leave(4ULL);
}
return o_leave(2ULL);
}
static uint64_t f17(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t p0=v1;uint64_t p1=v2;f16(p0, p1);});o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f18(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(({uint64_t p0=o_cast(5, 4, ({uint64_t _a=o_cast(4, 5, v0);uint64_t _b=o_cast(4, 5, v1);o_fp(1, 11, _a,_b);}));f1(p0);}));
}
static uint64_t f19(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(({uint64_t p0=o_cast(5, 4, ({uint64_t _a=o_cast(4, 5, v0);uint64_t _b=o_cast(4, 5, v1);o_fp(2, 11, _a,_b);}));f1(p0);}));
}
static uint64_t f20(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(({uint64_t p0=o_cast(5, 4, ({uint64_t _a=o_cast(4, 5, v0);uint64_t _b=o_cast(4, 5, v1);o_fp(3, 11, _a,_b);}));f1(p0);}));
}
static uint64_t f21(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(({uint64_t _a=2147483648ULL;uint64_t _b=({uint64_t p0=({uint64_t _a=4294967296ULL;uint64_t _b=v0;o_op(2, 5, _a,_b);});uint64_t p1=0ULL;f7(p0, p1);});o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t p0=v0;uint64_t p1=0ULL;f7(p0, p1);}));
}
static uint64_t f22(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t p0=v0;uint64_t p1=0ULL;f7(p0, p1);}));
}
static uint64_t f23(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t p0=v0;f8(p0);});
uint64_t __attribute__((unused)) v2=({uint64_t p0=v0;f9(p0);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(11, 9, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=16ULL;o_op(10, 9, _a,_b);})){
return o_leave(9223372036854775808ULL);
}
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=o_cast(5, 9, v2);o_op(15, 5, _a,_b);});uint64_t _b=2ULL;o_op(15, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0;o_op(17, 9, _a,_b);});uint64_t _b=40ULL;o_op(11, 9, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v3=o_cast(5, 9, ({uint64_t _a=v2;uint64_t _b=0;o_op(17, 9, _a,_b);}));
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=v3;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v1;uint64_t _b=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=v3;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=3ULL;
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v7=0ULL;
}else{
if(({uint64_t _a=v5;uint64_t _b=v6;o_op(8, 5, _a,_b);})){
v7=1ULL;
}else{
if(({uint64_t _a=v5;uint64_t _b=v6;o_op(6, 5, _a,_b);})){
v7=2ULL;
}
}
}
return o_leave(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(15, 5, _a,_b);});uint64_t _b=v7;o_op(13, 5, _a,_b);}));
}
static uint64_t f24(uint64_t v0, uint64_t v1){
o_enter();
if(v1){
if(v0){
return o_leave(0ULL);
}
return o_leave(4294967295ULL);
}
if(v0){
return o_leave(2147483648ULL);
}
return o_leave(2147483647ULL);
}
static uint64_t f25(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(v2){
if(v0){
return o_leave(0ULL);
}
if(({uint64_t _a=v1;uint64_t _b=4294967295ULL;o_op(10, 5, _a,_b);})){
return o_leave(4294967295ULL);
}
return o_leave(v1);
}
if(v0){
if(({uint64_t _a=v1;uint64_t _b=2147483648ULL;o_op(10, 5, _a,_b);})){
return o_leave(2147483648ULL);
}
return o_leave(({uint64_t _a=({uint64_t _a=0ULL;uint64_t _b=v1;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=v1;uint64_t _b=2147483647ULL;o_op(10, 5, _a,_b);})){
return o_leave(2147483647ULL);
}
return o_leave(v1);
}
static uint64_t f26(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t p0=v0;f0(p0);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v0;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v3;uint64_t p1=v2;f24(p0, p1);}));
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v5=({uint64_t p0=v4;f23(p0);});
if(({uint64_t _a=v5;uint64_t _b=9223372036854775808ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v3;uint64_t p1=v2;f24(p0, p1);}));
}
return o_leave(({uint64_t p0=v3;uint64_t p1=({uint64_t p0=v5;uint64_t p1=v1;uint64_t p2=v3;f27(p0, p1, p2);});uint64_t p2=v2;f25(p0, p1, p2);}));
}
static uint64_t f27(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=v0;uint64_t _b=2ULL;o_op(16, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v0;uint64_t _b=3ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=0ULL;
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=v2;uint64_t _b=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);});
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0;o_op(18, 1, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);});
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v5=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);});
}
if(v5){
return o_leave(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));
}
return o_leave(v3);
}
static uint64_t f28(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=32768ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=23ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v0;uint64_t _b=8388607ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v3;uint64_t _b=v1;o_op(15, 5, _a,_b);});
if(({uint64_t _a=v5;uint64_t _b=255ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=v4;uint64_t _b=v7;o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t _a=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=v7;o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=1ULL;uint64_t _b=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=({uint64_t _a=23ULL;uint64_t _b=v1;o_op(2, 5, _a,_b);});o_op(16, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(v4);
}
return o_leave(({uint64_t _a=v4;uint64_t _b=({uint64_t p0=({uint64_t p0=v0;f8(p0);});uint64_t p1=({uint64_t p0=v0;f9(p0);});uint64_t p2=o_cast(9, 5, v1);uint64_t p3=v2;uint64_t p4=v3;f6(p0, p1, p2, p3, p4);});o_op(13, 5, _a,_b);}));
}
static uint64_t f29(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=32768ULL;o_op(12, 5, _a,_b);});uint64_t _b=16ULL;o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=10ULL;o_op(16, 5, _a,_b);});uint64_t _b=31ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=v0;uint64_t _b=1023ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=31ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=2139095040ULL;o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2139095040ULL;o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=4194304ULL;uint64_t _b=({uint64_t _a=v3;uint64_t _b=13ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(v1);
}
uint64_t __attribute__((unused)) v4=113ULL;
{
uint64_t c2=0;
while(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1024ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(c2++>=10)o_trap(4);
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}
}
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=({uint64_t _a=v4;uint64_t _b=23ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1023ULL;o_op(12, 5, _a,_b);});uint64_t _b=13ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=112ULL;o_op(1, 5, _a,_b);});uint64_t _b=23ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=13ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
static uint64_t f30(uint64_t v0){
o_enter();
return o_leave(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});uint64_t _b=16ULL;o_op(15, 5, _a,_b);}));
}
static uint64_t f31(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
uint64_t __attribute__((unused)) v6=({uint64_t _a=62ULL;uint64_t _b=o_cast(9, 5, ({uint64_t p0=v0;f4(p0);}));o_op(2, 9, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v0;uint64_t _b=o_cast(5, 9, v6);o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v1;uint64_t _b=v6;o_op(2, 9, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=v8;uint64_t _b=v4;o_op(2, 9, _a,_b);});
uint64_t __attribute__((unused)) v10=v3;
if(({uint64_t _a=v9;uint64_t _b=0ULL;o_op(8, 9, _a,_b);})){
v10=({uint64_t _a=v3;uint64_t _b=o_cast(5, 9, ({uint64_t _a=v9;uint64_t _b=0;o_op(17, 9, _a,_b);}));o_op(15, 5, _a,_b);});
}else{
if(({uint64_t _a=v9;uint64_t _b=64ULL;o_op(11, 9, _a,_b);})){
v10=1ULL;
}else{
if(({uint64_t _a=v9;uint64_t _b=0ULL;o_op(10, 9, _a,_b);})){
v10=({uint64_t _a=v3;uint64_t _b=o_cast(5, 9, v9);o_op(16, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=({uint64_t _a=1ULL;uint64_t _b=o_cast(5, 9, v9);o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v10=({uint64_t _a=v10;uint64_t _b=1ULL;o_op(13, 5, _a,_b);});
}
}
}
}
uint64_t __attribute__((unused)) v11=({uint64_t _a=v7;uint64_t _b=v10;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v12=v2;
if(({uint64_t _a=v2;uint64_t _b=v5;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v7;uint64_t _b=v10;o_op(11, 5, _a,_b);})){
v11=({uint64_t _a=v7;uint64_t _b=v10;o_op(2, 5, _a,_b);});
}else{
v11=({uint64_t _a=v10;uint64_t _b=v7;o_op(2, 5, _a,_b);});
v12=v5;
}
}
if(({uint64_t _a=v11;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(({uint64_t _a=v12;uint64_t _b=({uint64_t p0=v11;uint64_t p1=v8;f7(p0, p1);});o_op(13, 5, _a,_b);}));
}
static uint64_t f32(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
if(({uint64_t _a=({uint64_t _a=o_cast(9, 5, ({uint64_t p0=v3;f4(p0);}));uint64_t _b=v4;o_op(1, 9, _a,_b);});uint64_t _b=({uint64_t _a=o_cast(9, 5, ({uint64_t p0=v0;f4(p0);}));uint64_t _b=v1;o_op(1, 9, _a,_b);});o_op(10, 9, _a,_b);})){
return o_leave(({uint64_t p0=v3;uint64_t p1=v4;uint64_t p2=v5;uint64_t p3=v0;uint64_t p4=v1;uint64_t p5=v2;f31(p0, p1, p2, p3, p4, p5);}));
}
return o_leave(({uint64_t p0=v0;uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=v4;uint64_t p5=v5;f31(p0, p1, p2, p3, p4, p5);}));
}
static uint64_t f33(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t p0=v0;f8(p0);});uint64_t _b=({uint64_t p0=v1;f8(p0);});o_op(3, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=({uint64_t p0=v0;f9(p0);});uint64_t _b=({uint64_t p0=v1;f9(p0);});o_op(1, 9, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=v3;uint64_t _b=({uint64_t p0=v5;uint64_t p1=v6;f7(p0, p1);});o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t p0=v5;uint64_t p1=v6;uint64_t p2=v3;uint64_t p3=({uint64_t p0=v2;f8(p0);});uint64_t p4=({uint64_t p0=v2;f9(p0);});uint64_t p5=v4;f32(p0, p1, p2, p3, p4, p5);}));
}
static uint64_t f34(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(14, 5, _a,_b);});uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v2;uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=v0;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v1;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v2;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=2139095040ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=2139095040ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v7;uint64_t _b=2139095040ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2143289344ULL);
}
uint64_t __attribute__((unused)) v8=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});
if(({uint64_t _a=v8;uint64_t _b=v9;o_op(12, 1, _a,_b);})){
return o_leave(2143289344ULL);
}
if(v8){
if(({uint64_t _a=({uint64_t _a=v7;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=v3;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(2143289344ULL);
}
return o_leave(({uint64_t _a=v3;uint64_t _b=2139095040ULL;o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v7;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);})){
return o_leave(v2);
}
if(v9){
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v2);
}
return o_leave(({uint64_t _a=v3;uint64_t _b=v4;o_op(12, 5, _a,_b);}));
}
return o_leave(({uint64_t p0=v5;uint64_t p1=v6;uint64_t p2=v7;uint64_t p3=v3;uint64_t p4=v4;f33(p0, p1, p2, p3, p4);}));
}
static uint64_t f35(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t p0=v2;f2(p0);})){
return o_leave(({uint64_t _a=v2;uint64_t _b=4194304ULL;o_op(13, 5, _a,_b);}));
}
if(({uint64_t p0=v0;f2(p0);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=4194304ULL;o_op(13, 5, _a,_b);}));
}
if(({uint64_t p0=v1;f2(p0);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=4194304ULL;o_op(13, 5, _a,_b);}));
}
if(({uint64_t p0=v2;f0(p0);})){
uint64_t __attribute__((unused)) v3=({uint64_t _a=v0;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2143289344ULL);
}
return o_leave(v2);
}
if(({uint64_t p0=v0;f0(p0);})){
return o_leave(v0);
}
if(({uint64_t p0=v1;f0(p0);})){
return o_leave(v1);
}
return o_leave(({uint64_t p0=v0;uint64_t p1=v1;uint64_t p2=v2;f34(p0, p1, p2);}));
}
static uint64_t f36(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p0=v1;f10(p0);});
uint64_t __attribute__((unused)) v4=({uint64_t p0=v0;f10(p0);});
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t v6=0;
uint64_t k1=27ULL;
for(;o_op(8,5, v6, k1);v6=o_op(1,5, v6,1)){
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=v3;o_op(11, 5, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=v3;o_op(2, 5, _a,_b);});
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(13, 5, _a,_b);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v6=0ULL;
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v6=1ULL;
}
return o_leave(({uint64_t _a=v2;uint64_t _b=({uint64_t p0=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});uint64_t _b=v6;o_op(13, 5, _a,_b);});uint64_t p1=({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t p0=v0;f11(p0);});uint64_t _b=({uint64_t p0=v1;f11(p0);});o_op(2, 9, _a,_b);});uint64_t _b=26ULL;o_op(2, 9, _a,_b);});uint64_t _b=1ULL;o_op(2, 9, _a,_b);});f7(p0, p1);});o_op(13, 5, _a,_b);}));
}
static uint64_t f37(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=({uint64_t p0=v0;f0(p0);});uint64_t _b=({uint64_t p0=v1;f0(p0);});o_op(13, 1, _a,_b);})){
return o_leave(2143289344ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=v1;o_op(14, 5, _a,_b);});uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=v0;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(2143289344ULL);
}
return o_leave(({uint64_t _a=v2;uint64_t _b=2139095040ULL;o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(v2);
}
if(({uint64_t _a=v3;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v4;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);})){
return o_leave(2143289344ULL);
}
return o_leave(({uint64_t _a=v2;uint64_t _b=2139095040ULL;o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v4;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);})){
return o_leave(v2);
}
return o_leave(({uint64_t p0=v3;uint64_t p1=v4;uint64_t p2=v2;f36(p0, p1, p2);}));
}
static uint64_t f38(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t _a=v0;uint64_t _b=28ULL;o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=({uint64_t _a=1ULL;uint64_t _b=62ULL;o_op(15, 5, _a,_b);});
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(10, 5, _a,_b);})){
if(c1++>=32)o_trap(4);
v4=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(16, 5, _a,_b);});
}
}
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(c1++>=32)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=v3;uint64_t _b=v4;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=v5;o_op(11, 5, _a,_b);})){
v2=({uint64_t _a=v2;uint64_t _b=v5;o_op(2, 5, _a,_b);});
v3=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});uint64_t _b=v4;o_op(1, 5, _a,_b);});
}else{
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(16, 5, _a,_b);});
}
v4=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(16, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v5=0ULL;
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v5=1ULL;
}
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});uint64_t _b=v5;o_op(13, 5, _a,_b);});uint64_t p1=({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(4, 9, _a,_b);});uint64_t _b=14ULL;o_op(2, 9, _a,_b);});uint64_t _b=1ULL;o_op(2, 9, _a,_b);});f7(p0, p1);}));
}
static uint64_t f39(uint64_t v0){
o_enter();
if(({uint64_t p0=v0;f0(p0);})){
return o_leave(2143289344ULL);
}
if(({uint64_t p0=v0;f3(p0);})){
return o_leave(v0);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2143289344ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);})){
return o_leave(2139095040ULL);
}
uint64_t __attribute__((unused)) v1=({uint64_t p0=v0;f10(p0);});
uint64_t __attribute__((unused)) v2=({uint64_t p0=v0;f11(p0);});
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(5, 9, _a,_b);});uint64_t _b=0ULL;o_op(7, 9, _a,_b);})){
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 9, _a,_b);});
}
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f38(p0, p1);}));
}
static uint64_t f40(uint64_t v0){
o_enter();
if(({uint64_t p0=v0;f0(p0);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=4194304ULL;o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t p0=v0;f39(p0);}));
}
static uint64_t f41(uint64_t v0, uint64_t v1){
o_enter();
return o_leave(o_cast(5, 4, ({uint64_t _a=o_cast(4, 5, v0);uint64_t _b=o_cast(4, 5, v1);o_fp(4, 11, _a,_b);})));
}
static uint64_t f42(uint64_t v0){
o_enter();
return o_leave(o_cast(4, 5, v0));
}
static uint64_t f43(uint64_t v0){
o_enter();
return o_leave(o_cast(5, 4, v0));
}
static uint64_t f44(uint64_t v0){
o_enter();
return o_leave(o_cast(4, 9, ({uint64_t _a=({uint64_t _a=v0;uint64_t _b=127ULL;o_op(1, 9, _a,_b);});uint64_t _b=23ULL;o_op(15, 9, _a,_b);})));
}
static uint64_t f45(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v1;uint64_t _b=127ULL;o_op(10, 9, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t p0=2130706432ULL;f42(p0);});o_fp(3, 11, _a,_b);});uint64_t _b=({uint64_t p0=({uint64_t _a=v1;uint64_t _b=127ULL;o_op(2, 9, _a,_b);});f44(p0);});o_fp(3, 11, _a,_b);});f43(p0);}));
}
if(({uint64_t _a=v1;uint64_t _b=18446744073709551490ULL;o_op(8, 9, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t p0=({uint64_t _a=v1;uint64_t _b=100ULL;o_op(1, 9, _a,_b);});f44(p0);});o_fp(3, 11, _a,_b);});uint64_t _b=({uint64_t p0=18446744073709551516ULL;f44(p0);});o_fp(3, 11, _a,_b);});f43(p0);}));
}
return o_leave(({uint64_t p0=({uint64_t _a=v0;uint64_t _b=({uint64_t p0=v1;f44(p0);});o_fp(3, 11, _a,_b);});f43(p0);}));
}
static uint64_t f46(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_fcv(11, 9, v1);
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=({uint64_t _a=v2;uint64_t _b=({uint64_t p0=1060205056ULL;f42(p0);});o_fp(3, 11, _a,_b);});o_fp(2, 11, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=({uint64_t p0=901758606ULL;f42(p0);});o_fp(3, 11, _a,_b);});o_fp(2, 11, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=({uint64_t p0=1026206379ULL;f42(p0);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=({uint64_t p0=1007192201ULL;f42(p0);});o_fp(3, 11, _a,_b);});o_fp(1, 11, _a,_b);});o_fp(3, 11, _a,_b);});
v4=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=({uint64_t p0=1042983595ULL;f42(p0);});uint64_t _b=v4;o_fp(1, 11, _a,_b);});o_fp(3, 11, _a,_b);});
v4=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=({uint64_t p0=1056964608ULL;f42(p0);});uint64_t _b=v4;o_fp(1, 11, _a,_b);});o_fp(3, 11, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t p0=1065353216ULL;f42(p0);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=({uint64_t p0=1065353216ULL;f42(p0);});uint64_t _b=v4;o_fp(1, 11, _a,_b);});o_fp(3, 11, _a,_b);});o_fp(1, 11, _a,_b);});
return o_leave(({uint64_t p0=v5;uint64_t p1=v1;f45(p0, p1);}));
}
static uint64_t f47(uint64_t v0){
o_enter();
if(({uint64_t p0=v0;f0(p0);})){
return o_leave(2143289344ULL);
}
uint64_t __attribute__((unused)) v1=({uint64_t p0=v0;f42(p0);});
if(({uint64_t _a=v1;uint64_t _b=({uint64_t p0=3268411392ULL;f42(p0);});o_fp(8, 11, _a,_b);})){
return o_leave(0ULL);
}
if(({uint64_t _a=v1;uint64_t _b=({uint64_t p0=1118925336ULL;f42(p0);});o_fp(10, 11, _a,_b);})){
return o_leave(2139095040ULL);
}
uint64_t __attribute__((unused)) v2=({uint64_t _a=v1;uint64_t _b=({uint64_t p0=1069066811ULL;f42(p0);});o_fp(3, 11, _a,_b);});
uint64_t __attribute__((unused)) v3=0ULL;
if(({uint64_t _a=v2;uint64_t _b=({uint64_t p0=0ULL;f42(p0);});o_fp(11, 11, _a,_b);})){
v3=o_fcv(8, 11, ({uint64_t _a=v2;uint64_t _b=({uint64_t p0=1056964608ULL;f42(p0);});o_fp(1, 11, _a,_b);}));
}else{
v3=o_fcv(8, 11, ({uint64_t _a=v2;uint64_t _b=({uint64_t p0=1056964608ULL;f42(p0);});o_fp(2, 11, _a,_b);}));
}
return o_leave(({uint64_t p0=v1;uint64_t p1=o_cast(9, 8, v3);f46(p0, p1);}));
}
static uint64_t f48(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t p0=({uint64_t p0=({uint64_t p0=({uint64_t _a=v0;uint64_t _b=({uint64_t p0=1065353216ULL;f42(p0);});o_fp(2, 11, _a,_b);});f43(p0);});uint64_t p1=({uint64_t p0=({uint64_t _a=v0;uint64_t _b=({uint64_t p0=1065353216ULL;f42(p0);});o_fp(1, 11, _a,_b);});f43(p0);});f37(p0, p1);});f42(p0);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=v2;uint64_t _b=v2;o_fp(3, 11, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t p0=1065353216ULL;f42(p0);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=({uint64_t p0=1051372203ULL;f42(p0);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=({uint64_t _a=({uint64_t p0=1045220557ULL;f42(p0);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=({uint64_t p0=1041385765ULL;f42(p0);});o_fp(3, 11, _a,_b);});o_fp(1, 11, _a,_b);});o_fp(3, 11, _a,_b);});o_fp(1, 11, _a,_b);});o_fp(3, 11, _a,_b);});o_fp(1, 11, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=({uint64_t p0=1073741824ULL;f42(p0);});uint64_t _b=v2;o_fp(3, 11, _a,_b);});uint64_t _b=v4;o_fp(3, 11, _a,_b);});
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=o_fcv(11, 9, v1);uint64_t _b=({uint64_t p0=1060205080ULL;f42(p0);});o_fp(3, 11, _a,_b);});uint64_t _b=v5;o_fp(1, 11, _a,_b);});f43(p0);}));
}
static uint64_t f49(uint64_t v0){
o_enter();
if(({uint64_t p0=v0;f0(p0);})){
return o_leave(2143289344ULL);
}
if(({uint64_t _a=({uint64_t p0=v0;f42(p0);});uint64_t _b=({uint64_t p0=0ULL;f42(p0);});o_fp(8, 11, _a,_b);})){
return o_leave(2143289344ULL);
}
if(({uint64_t p0=v0;f3(p0);})){
return o_leave(4286578688ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);})){
return o_leave(2139095040ULL);
}
uint64_t __attribute__((unused)) v1=v0;
uint64_t __attribute__((unused)) v2=0ULL;
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=23ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p0=({uint64_t _a=({uint64_t p0=v0;f42(p0);});uint64_t _b=({uint64_t p0=1258291200ULL;f42(p0);});o_fp(3, 11, _a,_b);});f43(p0);});
v2=18446744073709551593ULL;
}
uint64_t __attribute__((unused)) v3=({uint64_t _a=({uint64_t _a=o_cast(9, 5, ({uint64_t _a=({uint64_t _a=v1;uint64_t _b=23ULL;o_op(16, 5, _a,_b);});uint64_t _b=255ULL;o_op(12, 5, _a,_b);}));uint64_t _b=127ULL;o_op(2, 9, _a,_b);});uint64_t _b=v2;o_op(1, 9, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t p0=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8388607ULL;o_op(12, 5, _a,_b);});uint64_t _b=1065353216ULL;o_op(13, 5, _a,_b);});f42(p0);});
if(({uint64_t _a=v4;uint64_t _b=({uint64_t p0=1068827891ULL;f42(p0);});o_fp(10, 11, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=({uint64_t p0=1056964608ULL;f42(p0);});o_fp(3, 11, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 9, _a,_b);});
}
return o_leave(({uint64_t p0=v4;uint64_t p1=v3;f48(p0, p1);}));
}
static uint64_t f50(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=({uint64_t p0=v0;f0(p0);});uint64_t _b=({uint64_t p0=v1;f0(p0);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=v1;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f51(uint64_t v0){
o_enter();
if(v0){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f52(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f50(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=v1;uint64_t _b=v2;o_op(6, 5, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(16, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v3=31744ULL;
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v3=32640ULL;
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=32767ULL;o_op(12, 5, _a,_b);});uint64_t _b=v3;o_op(10, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32767ULL;o_op(12, 5, _a,_b);});uint64_t _b=v3;o_op(10, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=v5;o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=v4;uint64_t _b=v5;o_op(12, 1, _a,_b);});f51(p0);}));
}
return o_leave(({uint64_t p0=({uint64_t _a=v1;uint64_t _b=v2;o_op(6, 5, _a,_b);});f51(p0);}));
}
return o_leave(0ULL);
}
static uint64_t f53(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v1=1ULL;
uint64_t __attribute__((unused)) v2=32ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=v0;o_op(8, 5, _a,_b);})){
if(c1++>=13)o_trap(4);
if(({uint64_t _a=v2;uint64_t _b=576460752303423487ULL;o_op(10, 5, _a,_b);})){
v2=({uint64_t _a=0ULL;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
}else{
v2=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(3, 5, _a,_b);});
}
v1=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v1);
}
static uint64_t f54(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=63ULL;o_op(1, 5, _a,_b);});uint64_t _b=64ULL;o_op(4, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(v1);
}
static uint64_t f55(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f18(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f19(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f20(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;uint64_t p1=v2;uint64_t p2=v3;f35(p0, p1, p2);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;uint64_t p1=v2;uint64_t p2=v3;f34(p0, p1, p2);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f13(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f14(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f15(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v3;uint64_t p1=v1;uint64_t p2=v2;f17(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f21(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f22(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=0ULL;uint64_t p2=0ULL;f26(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=1ULL;uint64_t p2=0ULL;f26(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=2ULL;uint64_t p2=0ULL;f26(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=3ULL;uint64_t p2=0ULL;f26(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=0ULL;uint64_t p2=1ULL;f26(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=10ULL;uint64_t p2=15ULL;uint64_t p3=31ULL;f28(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=7ULL;uint64_t p2=127ULL;uint64_t p3=255ULL;f28(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f29(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f30(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;uint64_t p1=v2;f37(p0, p1);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;f39(p0);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;f47(p0);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;f49(p0);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=24ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;uint64_t p1=v2;f41(p0, p1);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=25ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;f40(p0);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=26ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=2129859008ULL;uint64_t _b=v1;o_op(2, 5, _a,_b);});uint64_t _b=4294967295ULL;o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=27ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f50(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=28ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;uint64_t p2=v3;f52(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=29ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f53(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=30ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f54(p0);}));
}
return o_leave(0ULL);
}
static uint64_t f56(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(6ULL);
}
if(({uint64_t _a=v0;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v0;uint64_t _b=20ULL;o_op(6, 5, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=v0;uint64_t _b=21ULL;o_op(6, 5, _a,_b);})){
return o_leave(4ULL);
}
if(({uint64_t _a=v0;uint64_t _b=22ULL;o_op(6, 5, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=v0;uint64_t _b=23ULL;o_op(6, 5, _a,_b);})){
return o_leave(5ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=24ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=26ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(2, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=27ULL;o_op(6, 5, _a,_b);})){
return o_leave(11ULL);
}
if(({uint64_t _a=v0;uint64_t _b=28ULL;o_op(6, 5, _a,_b);})){
return o_leave(12ULL);
}
if(({uint64_t _a=v0;uint64_t _b=29ULL;o_op(6, 5, _a,_b);})){
return o_leave(14ULL);
}
if(({uint64_t _a=v0;uint64_t _b=30ULL;o_op(6, 5, _a,_b);})){
return o_leave(10ULL);
}
if(({uint64_t _a=v0;uint64_t _b=31ULL;o_op(6, 5, _a,_b);})){
return o_leave(13ULL);
}
return o_leave(0ULL);
}
static uint64_t f57(uint64_t v0, uint64_t v1){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;f0(p0);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2147483647ULL;o_op(12, 5, _a,_b);});uint64_t _b=2139095040ULL;o_op(6, 5, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2139095040ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8388607ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;f3(p0);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2139095040ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2139095040ULL;o_op(12, 5, _a,_b);});uint64_t _b=2139095040ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2139095040ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=8388607ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);}));
}
return o_leave(v1);
}
static uint64_t f58(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(8, 5, _a,_b);})){
return o_leave(({uint64_t p0=v0;uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;f55(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t p0=v1;uint64_t p1=v2;f15(p0, p1);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(v1);
}
return o_leave(v2);
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t p0=v1;uint64_t p1=0ULL;f15(p0, p1);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(v2);
}
return o_leave(v1);
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f13(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;f14(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f21(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=0ULL;uint64_t p2=0ULL;f26(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=1065353216ULL;uint64_t p1=v1;f41(p0, p1);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=1065353216ULL;uint64_t p1=({uint64_t p0=v1;f40(p0);});f41(p0, p1);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;uint64_t p1=v2;f37(p0, p1);});f1(p0);}));
}
return o_leave(({uint64_t p0=({uint64_t p0=v1;uint64_t p1=v2;f41(p0, p1);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v4;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;f39(p0);});f1(p0);}));
}
return o_leave(({uint64_t p0=({uint64_t p0=v1;f40(p0);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;f47(p0);});f1(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;f49(p0);});f1(p0);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=19ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=31ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(({uint64_t _a=({uint64_t p0=({uint64_t p0=v0;f56(p0);});uint64_t p1=v1;uint64_t p2=v2;f17(p0, p1, p2);});uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(v1);
}
return o_leave(v2);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=32ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=34ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=({uint64_t _a=v0;uint64_t _b=31ULL;o_op(2, 5, _a,_b);});uint64_t p2=0ULL;f26(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=35ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=0ULL;uint64_t p2=1ULL;f26(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=36ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f22(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=37ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=10ULL;uint64_t p2=15ULL;uint64_t p3=31ULL;f28(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=38ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=7ULL;uint64_t p2=127ULL;uint64_t p3=255ULL;f28(p0, p1, p2, p3);}));
}
if(({uint64_t _a=v0;uint64_t _b=39ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=v1;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f29(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=40ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=v1;uint64_t _b=65535ULL;o_op(12, 5, _a,_b);});f30(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=41ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v1;uint64_t p1=v2;uint64_t p2=v3;f34(p0, p1, p2);});f1(p0);}));
}
return o_leave(v1);
}
static uint64_t f59(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=42ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=9ULL;o_op(12, 5, _a,_b);});uint64_t _b=9ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=16ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(5ULL);
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=3ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=4ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=32ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=64ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(3ULL);
}
}
return o_leave(0ULL);
}
static uint64_t f60(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=({uint64_t _a=({uint64_t p0=6ULL;uint64_t p1=v1;f57(p0, p1);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2147483648ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v3=({uint64_t p0=v0;uint64_t p1=v1;uint64_t p2=0ULL;uint64_t p3=0ULL;uint64_t p4=0ULL;f58(p0, p1, p2, p3, p4);});
if(({uint64_t _a=({uint64_t p0=6ULL;uint64_t p1=v3;f57(p0, p1);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=o_fcv(12, 11, ({uint64_t p0=v2;f42(p0);}));uint64_t _b=o_fcv(12, 11, ({uint64_t p0=v3;f42(p0);}));o_fp(2, 12, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);})){
v4=({uint64_t _a=v4;uint64_t _b=0;o_fp(17, 12, _a,_b);});
}
uint64_t __attribute__((unused)) v5=o_fcv(12, 11, ({uint64_t p0=v3;f42(p0);}));
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);})){
v5=({uint64_t _a=v5;uint64_t _b=0;o_fp(17, 12, _a,_b);});
}
if(({uint64_t _a=({uint64_t p0=v2;f0(p0);});uint64_t _b=({uint64_t _a=({uint64_t _a=v4;uint64_t _b=({uint64_t _a=v5;uint64_t _b=({uint64_t _a=4607182418800017408ULL;uint64_t _b=4697254411347427328ULL;o_fp(4, 12, _a,_b);});o_fp(3, 12, _a,_b);});o_fp(9, 12, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t _b=2ULL;o_op(13, 5, _a,_b);}));
}
return o_leave(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(15, 5, _a,_b);});uint64_t _b=1ULL;o_op(13, 5, _a,_b);}));
}
static uint64_t f61(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2143289344ULL);
}
return o_leave(2147483648ULL);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(2143289344ULL);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=8ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=16777216ULL;o_op(10, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=v2;uint64_t _b=4611686018427387839ULL;o_op(10, 5, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(5ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=31ULL;o_op(1, 5, _a,_b);});uint64_t _b=32ULL;o_op(4, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=31ULL;o_op(1, 5, _a,_b);});uint64_t _b=({uint64_t _a=31ULL;uint64_t _b=0;o_op(18, 5, _a,_b);});o_op(12, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v3;uint64_t p1=({uint64_t p0=v2;f22(p0);});f37(p0, p1);});f1(p0);}));
}
return o_leave(v3);
}
return o_leave(0ULL);
}
static uint64_t f62(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(9, 5, _a,_b);})){
return o_leave(({uint64_t p0=v0;uint64_t p1=v1;f57(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f56(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t p0=v1;f56(p0);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(4294967295ULL);
}
return o_leave(({uint64_t p0=v6;uint64_t p1=v2;uint64_t p2=v3;f17(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=v4;uint64_t p4=v5;f58(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=v2;uint64_t _b=63ULL;o_op(14, 5, _a,_b);}));
}
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=13ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(5, 5, _a,_b);});uint64_t _b=31ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}));
}
return o_leave(v2);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;uint64_t p2=v3;uint64_t p3=v4;uint64_t p4=v5;f59(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=18ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=14ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;uint64_t p2=v3;f60(p0, p1, p2);}));
}
if(({uint64_t _a=v0;uint64_t _b=15ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t p0=v1;uint64_t p1=v2;uint64_t p2=v3;f52(p0, p1, p2);});uint64_t _b=({uint64_t _a=({uint64_t p0=4ULL;uint64_t p1=v2;f57(p0, p1);});uint64_t _b=1ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);}));
}
if(({uint64_t _a=v0;uint64_t _b=16ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=41ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=6ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=7ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
return o_leave(4294967296ULL);
}
return o_leave(({uint64_t p0=7ULL;uint64_t p1=({uint64_t p0=v1;uint64_t p1=({uint64_t p0=7ULL;uint64_t p1=v2;f57(p0, p1);});uint64_t p2=({uint64_t p0=7ULL;uint64_t p1=v3;f57(p0, p1);});uint64_t p3=({uint64_t p0=7ULL;uint64_t p1=v4;f57(p0, p1);});uint64_t p4=0ULL;f58(p0, p1, p2, p3, p4);});f57(p0, p1);}));
}
if(({uint64_t _a=v0;uint64_t _b=17ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(2ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=32ULL;o_op(8, 5, _a,_b);})){
return o_leave(v2);
}
return o_leave(32ULL);
}
if(({uint64_t _a=v0;uint64_t _b=19ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(11, 5, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=25ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t p0=v3;uint64_t p1=({uint64_t p0=v2;f22(p0);});f41(p0, p1);});f1(p0);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=20ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=25ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=v0;uint64_t _b=20ULL;o_op(2, 5, _a,_b);});uint64_t p1=v1;uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=v4;f61(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=26ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=8ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(10, 5, _a,_b);});o_op(12, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(10, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=4611686018427387903ULL;o_op(10, 5, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=v3;uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
if(({uint64_t _a=v4;uint64_t _b=({uint64_t _a=({uint64_t _a=4611686018427387903ULL;uint64_t _b=v2;o_op(2, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(4, 5, _a,_b);});o_op(10, 5, _a,_b);})){
return o_leave(3ULL);
}
}
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=27ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(2ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=65536ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=9ULL;o_op(12, 5, _a,_b);});uint64_t _b=9ULL;o_op(7, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=2ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v4;uint64_t _b=3ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v3;uint64_t _b=4ULL;o_op(12, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=13ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=18ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=32ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(3ULL);
}
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=12ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v2;uint64_t _b=64ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(7, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(3ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=28ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=3ULL;o_op(7, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=v3;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=29ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=42ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=0ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(8, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=15ULL;o_op(10, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=30ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(10, 5, _a,_b);})){
return o_leave(({uint64_t _a=({uint64_t _a=v2;uint64_t _b=31ULL;o_op(1, 5, _a,_b);});uint64_t _b=32ULL;o_op(4, 5, _a,_b);}));
}
return o_leave(0ULL);
}
if(({uint64_t _a=v0;uint64_t _b=31ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
return o_leave(2147483648ULL);
}
if(({uint64_t _a=v0;uint64_t _b=32ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=33ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v1;uint64_t _b=41ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=34ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(11, 5, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=35ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=({uint64_t _a=({uint64_t _a=({uint64_t _a=v1;uint64_t _b=38ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v2;uint64_t _b=0ULL;o_op(10, 5, _a,_b);});o_op(12, 1, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(6, 5, _a,_b);});o_op(12, 1, _a,_b);});f51(p0);}));
}
if(({uint64_t _a=v0;uint64_t _b=36ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(2ULL);
}
return o_leave(0ULL);
}
return o_leave(0ULL);
}
static uint64_t f63(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(v1);
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(v2);
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(v3);
}
return o_leave(v4);
}
static uint64_t f64(uint64_t v0){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=42ULL;o_op(11, 5, _a,_b);})){
return o_leave(4294967295ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=14ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=17ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(4294967294ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=3ULL;o_op(9, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=({uint64_t _a=v0;uint64_t _b=36ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=39ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=40ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(1ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
return o_leave(4ULL);
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=19ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=31ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
return o_leave(4ULL);
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
return o_leave(7ULL);
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
return o_leave(12ULL);
}
if(({uint64_t _a=v0;uint64_t _b=41ULL;o_op(6, 5, _a,_b);})){
return o_leave(6ULL);
}
return o_leave(3ULL);
}
static uint64_t f65(uint64_t v0, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p0=v0;f64(p0);});
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(10, 5, _a,_b);});uint64_t _b=({uint64_t _a=v3;uint64_t _b=16ULL;o_op(8, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=31053ULL;uint64_t p2=0ULL;uint64_t p3=58720256ULL;uint64_t p4=1042944ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v1;uint64_t _b=({uint64_t _a=v3;uint64_t _b=2ULL;o_op(2, 5, _a,_b);});o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v4=1040896ULL;
if(({uint64_t _a=v3;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v4=2089472ULL;
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
v4=2089472ULL;
}
return o_leave(({uint64_t p0=v2;uint64_t p1=100694406ULL;uint64_t p2=9ULL;uint64_t p3=202381572ULL;uint64_t p4=v4;f63(p0, p1, p2, p3, p4);}));
}
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=4ULL;o_op(6, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);});o_op(13, 1, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v4=5ULL;
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
v4=255ULL;
}
return o_leave(({uint64_t p0=v2;uint64_t p1=33583627ULL;uint64_t p2=v4;uint64_t p3=66084864ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=5ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=84505096ULL;uint64_t p2=2ULL;uint64_t p3=0ULL;uint64_t p4=1034752ULL;f63(p0, p1, p2, p3, p4);}));
}
return o_leave(({uint64_t p0=v2;uint64_t p1=34173448ULL;uint64_t p2=5ULL;uint64_t p3=0ULL;uint64_t p4=1034752ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=({uint64_t _a=v0;uint64_t _b=19ULL;o_op(11, 5, _a,_b);});uint64_t _b=({uint64_t _a=v0;uint64_t _b=31ULL;o_op(9, 5, _a,_b);});o_op(12, 1, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=33583627ULL;uint64_t p2=5ULL;uint64_t p3=({uint64_t _a=66060288ULL;uint64_t _b=({uint64_t _a=({uint64_t p0=v0;f56(p0);});uint64_t _b=12ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
return o_leave(({uint64_t p0=v2;uint64_t p1=34173448ULL;uint64_t p2=5ULL;uint64_t p3=0ULL;uint64_t p4=1034752ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=34173473ULL;uint64_t p2=5ULL;uint64_t p3=0ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=34173473ULL;uint64_t p2=2147483653ULL;uint64_t p3=0ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=34173472ULL;uint64_t p2=5ULL;uint64_t p3=4194304ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=34173475ULL;uint64_t p2=5ULL;uint64_t p3=1ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=6ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=34173449ULL;uint64_t p2=5ULL;uint64_t p3=58720256ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=7ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=34173449ULL;uint64_t p2=5ULL;uint64_t p3=125829120ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=8ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619077ULL;uint64_t p2=2ULL;uint64_t p3=2102272ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619269ULL;uint64_t p2=2ULL;uint64_t p3=2158848ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=10ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619272ULL;uint64_t p2=2ULL;uint64_t p3=4096ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=11ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619272ULL;uint64_t p2=2ULL;uint64_t p3=5120ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=13ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=34176905ULL;uint64_t p2=136322816ULL;uint64_t p3=917504ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=12ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=555033ULL;uint64_t p2=2ULL;uint64_t p3=1791ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=134903826ULL;uint64_t p2=252ULL;uint64_t p3=126762239ULL;uint64_t p4=1034752ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=134248840ULL;uint64_t p2=2ULL;uint64_t p3=134220031ULL;uint64_t p4=17818112ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v1;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=31517ULL;uint64_t p2=0ULL;uint64_t p3=65536ULL;uint64_t p4=1043456ULL;f63(p0, p1, p2, p3, p4);}));
}
return o_leave(({uint64_t p0=v2;uint64_t p1=168393092ULL;uint64_t p2=255ULL;uint64_t p3=134219776ULL;uint64_t p4=927744ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=18ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(5, 5, _a,_b);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v4=926720ULL;
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
v4=17704960ULL;
}
return o_leave(({uint64_t p0=v2;uint64_t p1=34176905ULL;uint64_t p2=({uint64_t _a=134225664ULL;uint64_t _b=({uint64_t _a=({uint64_t _a=16ULL;uint64_t _b=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(4, 5, _a,_b);});o_op(16, 5, _a,_b);});uint64_t _b=21ULL;o_op(15, 5, _a,_b);});o_op(13, 5, _a,_b);});uint64_t p3=917504ULL;uint64_t p4=v4;f63(p0, p1, p2, p3, p4);}));
}
uint64_t __attribute__((unused)) v4=33714721ULL;
if(({uint64_t _a=v1;uint64_t _b=9ULL;o_op(6, 5, _a,_b);})){
v4=34173473ULL;
}
return o_leave(({uint64_t p0=v2;uint64_t p1=v4;uint64_t p2=9ULL;uint64_t p3=0ULL;uint64_t p4=2083328ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=32ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619269ULL;uint64_t p2=2ULL;uint64_t p3=2126080ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=33ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619269ULL;uint64_t p2=2ULL;uint64_t p3=2142464ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=34ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619269ULL;uint64_t p2=2ULL;uint64_t p3=2109696ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=35ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619269ULL;uint64_t p2=2ULL;uint64_t p3=2158592ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=36ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619077ULL;uint64_t p2=2ULL;uint64_t p3=2101248ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=37ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619268ULL;uint64_t p2=2ULL;uint64_t p3=2099200ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=38ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=619268ULL;uint64_t p2=2ULL;uint64_t p3=2105344ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=39ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=4278809136ULL;uint64_t p2=536870914ULL;uint64_t p3=16640ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=40ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=34175001ULL;uint64_t p2=16ULL;uint64_t p3=1791ULL;uint64_t p4=17811968ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v0;uint64_t _b=41ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v1;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=4278877058ULL;uint64_t p2=59392ULL;uint64_t p3=2560ULL;uint64_t p4=17704960ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=151681061ULL;uint64_t p2=4ULL;uint64_t p3=126746634ULL;uint64_t p4=2083840ULL;f63(p0, p1, p2, p3, p4);}));
}
if(({uint64_t _a=v1;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v2;uint64_t p1=168524161ULL;uint64_t p2=4ULL;uint64_t p3=203299072ULL;uint64_t p4=927744ULL;f63(p0, p1, p2, p3, p4);}));
}
return o_leave(({uint64_t p0=v2;uint64_t p1=34173475ULL;uint64_t p2=5ULL;uint64_t p3=11ULL;uint64_t p4=2083328ULL;f63(p0, p1, p2, p3, p4);}));
}
return o_leave(0ULL);
}
static uint64_t f66(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p0=v1;f64(p0);}));
}
return o_leave(({uint64_t p0=v1;uint64_t p1=v2;uint64_t p2=v3;f65(p0, p1, p2);}));
}
uint32_t omega_numeric_patch(uint64_t *result, volatile uint64_t p0, volatile uint64_t p1, volatile uint64_t p2, volatile uint64_t p3){
if(!o_buf(result,1,1,5))return 32;
if(!o_valid(5, p0))return 32;
if(!o_valid(5, p1))return 32;
if(!o_valid(5, p2))return 32;
if(!o_valid(5, p3))return 32;
if(o_busy)return 33;
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f66(p0, p1, p2, p3);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
