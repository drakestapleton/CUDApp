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
static uint64_t f0(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f1(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f2(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f3(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f4(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f5(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f6(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f7(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f8(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f9(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f10(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f11(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f12(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f13(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f14(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f15(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f16(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f17(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f18(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f19(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f20(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f21(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f22(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f23(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f24(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f25(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f26(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f27(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f28(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f29(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f30(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f31(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f32(uint64_t v0, uint64_t v1);
static uint64_t f33(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f34(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f35(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f36(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f37(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f38(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f39(uint64_t v0);
static uint64_t f40(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f41(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f42(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f43(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f44(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f45(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f46(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f47(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f48(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f49(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f50(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f51(const void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f52(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f53(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f54(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f55(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f56(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f57(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f58(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f59(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f60(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f61(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f62(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f63(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f64(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f65(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f66(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f67(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f68(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f69(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f70(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3);
static uint64_t f71(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2);
static uint64_t f72(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f73(void *v0, uint64_t v0_n, uint64_t v1);
static uint64_t f74(void *v0, uint64_t v0_n);
static uint64_t f75(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f76(void *v0, uint64_t v0_n);
static uint64_t f77(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2);
static uint64_t f78(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f79(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n);
static uint64_t f0(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=2720ULL;o_op(1, 5, _a,_b);});
return o_leave(o_get(v0, v0_n, v2, 5));
}
static uint64_t f1(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t _a=v1;uint64_t _b=2ULL;o_op(3, 5, _a,_b);});
v2=({uint64_t _a=v2;uint64_t _b=2720ULL;o_op(1, 5, _a,_b);});
return o_leave(o_get(v0, v0_n, ({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);}), 5));
}
static uint64_t f2(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=10ULL;o_op(3, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=32ULL;o_op(1, 5, _a,_b);});
return o_leave(o_get(v0, v0_n, ({uint64_t _a=v3;uint64_t _b=v2;o_op(1, 5, _a,_b);}), 5));
}
static uint64_t f3(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=v1;uint64_t p2=v2;f2(v0, v0_n, p1, p2);});
return o_leave(v3);
}
static uint64_t f4(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=2592ULL;o_op(1, 5, _a,_b);});
return o_leave(o_get(v0, v0_n, ({uint64_t _a=v3;uint64_t _b=v2;o_op(1, 5, _a,_b);}), 5));
}
static uint64_t f5(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=v1;uint64_t p2=v2;f4(v0, v0_n, p1, p2);});
return o_leave(v3);
}
static uint64_t f6(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=4ULL;o_op(3, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=4768ULL;o_op(1, 5, _a,_b);});
return o_leave(o_get(v0, v0_n, ({uint64_t _a=v3;uint64_t _b=v2;o_op(1, 5, _a,_b);}), 5));
}
static uint64_t f7(const void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=v1;uint64_t p2=v2;f6(v0, v0_n, p1, p2);});
return o_leave(v3);
}
static uint64_t f8(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, v1, 5);
return o_leave(v2);
}
static uint64_t f9(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, v1, 5);
{uint64_t _i=v1;uint64_t _v=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f10(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, v1, 5);
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
{uint64_t _i=v1;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
return o_leave(v2);
}
static uint64_t f11(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
{uint64_t _i=v1;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f12(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, v1, 5);
{uint64_t _i=v1;uint64_t _v=({uint64_t _a=v3;uint64_t _b=v2;o_fp(1, 12, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f13(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=2688ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
{uint64_t _i=v5;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=1024)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t p1=v4;f1(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=2688ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);});
({uint64_t p1=v6;f9(v1, v1_n, p1);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f14(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=2688ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
if(({uint64_t _a=o_get(v1, v1_n, v5, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t _a=2432ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v6;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v3);
}
static uint64_t f15(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v4;o_op(8, 5, _a,_b);})){
if(c1++>=1024)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f0(v0, v0_n, p1);});
if(({uint64_t _a=v6;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t p1=v5;f1(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=2688ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t p1=v8;f10(v1, v1_n, p1);});
if(({uint64_t _a=v9;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v10=({uint64_t _a=2432ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v10;uint64_t _v=v7;o_put(v1, v1_n, _i,_v,5);}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v3);
}
static uint64_t f16(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
({f13(v0, v0_n, v1, v1_n);});
uint64_t __attribute__((unused)) v3=({f14(v0, v0_n, v1, v1_n);});
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=2432ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, v5, 5);
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v3=({uint64_t p2=v6;uint64_t p3=v3;f15(v0, v0_n, v1, v1_n, p2, p3);});
}
}
if(({uint64_t _a=v3;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
return o_leave(0ULL);
}
static uint64_t f17(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t p1=v3;uint64_t p2=0ULL;f7(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t p1=v3;uint64_t p2=1ULL;f6(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;uint64_t p2=2ULL;f5(v0, v0_n, p1, p2);});
if(({uint64_t _a=v4;uint64_t _b=v6;o_fp(10, 12, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=3ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=7ULL;uint64_t _v=v5;o_put(v1, v1_n, _i,_v,5);}
return o_leave(4ULL);
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0ULL);
}
static uint64_t f18(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t p1=v4;uint64_t p2=8ULL;f2(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=1ULL;uint64_t _b=v5;o_op(15, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=v6;o_op(13, 5, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
{uint64_t _i=11ULL;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f19(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t p1=v4;uint64_t p2=4ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v5;uint64_t _b=v1;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v6=({uint64_t p1=v4;uint64_t p2=5ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v6;uint64_t _b=v1;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
uint64_t __attribute__((unused)) v7=({uint64_t p1=v4;uint64_t p2=6ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v7;uint64_t _b=v1;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t p1=v4;uint64_t p2=1ULL;f6(v0, v0_n, p1, p2);});
if(({uint64_t _a=v5;uint64_t _b=v1;o_op(6, 5, _a,_b);})){
return o_leave(1ULL);
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0ULL);
}
static uint64_t f20(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 11ULL, 5);
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=32)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t p1=v4;f19(v0, v0_n, p1);});
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t p1=v4;uint64_t p2=3ULL;f4(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=1ULL;uint64_t _b=v6;o_op(15, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=v7;o_op(13, 5, _a,_b);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
{uint64_t _i=11ULL;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f21(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
{uint64_t _i=11ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
({f18(v0, v0_n, v1, v1_n);});
({f20(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f22(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=0ULL;
uint64_t __attribute__((unused)) v5=0ULL;
uint64_t __attribute__((unused)) v6=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v7=({uint64_t _a=64ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v7;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
v7=({uint64_t _a=320ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v7;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
v7=({uint64_t _a=576ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v7;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
v7=({uint64_t _a=832ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v7;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v8=({uint64_t p1=v3;uint64_t p2=0ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v9=({uint64_t p1=v3;uint64_t p2=1ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v10=({uint64_t p1=v3;uint64_t p2=2ULL;f3(v0, v0_n, p1, p2);});
v7=({uint64_t _a=1088ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
({uint64_t p1=v7;uint64_t p2=v8;f11(v1, v1_n, p1, p2);});
v7=({uint64_t _a=1344ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
({uint64_t p1=v7;uint64_t p2=v9;f11(v1, v1_n, p1, p2);});
v7=({uint64_t _a=1600ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
({uint64_t p1=v7;uint64_t p2=v10;f11(v1, v1_n, p1, p2);});
v7=({uint64_t _a=1856ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v7;uint64_t _v=13830554455654793216ULL;o_put(v1, v1_n, _i,_v,5);}
v7=({uint64_t _a=2112ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v7;uint64_t _v=13830554455654793216ULL;o_put(v1, v1_n, _i,_v,5);}
v4=({uint64_t _a=v4;uint64_t _b=v9;o_fp(1, 12, _a,_b);});
v5=({uint64_t _a=v5;uint64_t _b=v8;o_fp(1, 12, _a,_b);});
v6=({uint64_t _a=v6;uint64_t _b=v10;o_fp(1, 12, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
({uint64_t p1=15ULL;uint64_t p2=v4;f11(v1, v1_n, p1, p2);});
({uint64_t p1=16ULL;uint64_t p2=v5;f11(v1, v1_n, p1, p2);});
({uint64_t p1=17ULL;uint64_t p2=v6;f11(v1, v1_n, p1, p2);});
return o_leave(0);
}
static uint64_t f23(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=32)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t p1=v3;uint64_t p2=1ULL;f4(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=2368ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v5;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
v5=({uint64_t _a=2400ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
{uint64_t _i=v5;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f24(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=1024)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t p1=v3;f1(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=576ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
({uint64_t p1=v5;f9(v1, v1_n, p1);});
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f25(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
({f22(v0, v0_n, v1, v1_n);});
{uint64_t _i=0ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=1ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=3ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
({f23(v0, v0_n, v1, v1_n);});
({f24(v0, v0_n, v1, v1_n);});
return o_leave(0);
}
static uint64_t f26(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=({uint64_t p1=0ULL;f8(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=64ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, v5, 5);
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t _a=832ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=576ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
if(({uint64_t _a=o_get(v1, v1_n, v7, 5);uint64_t _b=o_get(v1, v1_n, v8, 5);o_op(6, 5, _a,_b);})){
{uint64_t _i=v5;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v9=({uint64_t _a=5024ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
({uint64_t p1=v9;uint64_t p2=v3;f11(v1, v1_n, p1, p2);});
}
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f27(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=v2;uint64_t p2=4ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v3;uint64_t _b=18446744073709551615ULL;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v4=({uint64_t _a=2368ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
if(({uint64_t _a=o_get(v1, v1_n, v4, 5);uint64_t _b=0ULL;o_op(9, 5, _a,_b);})){
return o_leave(1ULL);
}
}
uint64_t __attribute__((unused)) v4=({uint64_t p1=v2;uint64_t p2=6ULL;f2(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=1600ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f8(v1, v1_n, p1);});
if(({uint64_t _a=v4;uint64_t _b=18446744073709551615ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t _a=2400ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
if(({uint64_t _a=o_get(v1, v1_n, v7, 5);uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(2ULL);
}
}
}
if(({uint64_t _a=v4;uint64_t _b=18446744073709551615ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
return o_leave(3ULL);
}
}
return o_leave(0ULL);
}
static uint64_t f28(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t p1=v2;uint64_t p2=4ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v5;uint64_t _b=18446744073709551615ULL;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t _a=2368ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t p1=v6;f10(v1, v1_n, p1);});
}
uint64_t __attribute__((unused)) v6=({uint64_t _a=4512ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
({uint64_t p1=v6;uint64_t p2=v3;f11(v1, v1_n, p1, p2);});
if(({uint64_t _a=v4;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t p1=v2;uint64_t p2=6ULL;f2(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=2400ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
{uint64_t _i=v8;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v9=({uint64_t _a=64ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
{uint64_t _i=v9;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
}
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t _a=64ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
{uint64_t _i=v7;uint64_t _v=3ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v8=({uint64_t _a=1856ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
({uint64_t p1=v8;uint64_t p2=v3;f11(v1, v1_n, p1, p2);});
}
return o_leave(0);
}
static uint64_t f29(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=({uint64_t p1=0ULL;f8(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=64ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, v5, 5);
if(({uint64_t _a=v6;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t p2=v4;f27(v0, v0_n, v1, v1_n, p2);});
if(({uint64_t _a=v7;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v8=({uint64_t _a=320ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
{uint64_t _i=v8;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
}
if(({uint64_t _a=v7;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v8=({uint64_t _a=320ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
{uint64_t _i=v8;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
}
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=v4;uint64_t p3=v3;uint64_t p4=0ULL;f28(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
if(({uint64_t _a=v7;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=v4;uint64_t p3=v3;uint64_t p4=3ULL;f28(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f30(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p1=v2;uint64_t p2=5ULL;f2(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t p1=v2;uint64_t p2=3ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=2944ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=o_get(v1, v1_n, v6, 5);
uint64_t __attribute__((unused)) v8=({uint64_t _a=v5;uint64_t _b=o_fcv(12, 5, v7);o_fp(4, 12, _a,_b);});
return o_leave(({uint64_t _a=v3;uint64_t _b=v8;o_fp(4, 12, _a,_b);}));
}
static uint64_t f31(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=4607182418800017408ULL;uint64_t _b=0;o_fp(17, 12, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=1088ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t p1=v4;f8(v1, v1_n, p1);});
if(({uint64_t _a=v5;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
v3=v5;
}
uint64_t __attribute__((unused)) v6=({uint64_t _a=1344ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t p1=v6;f8(v1, v1_n, p1);});
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
uint64_t __attribute__((unused)) v8=({uint64_t p2=v2;uint64_t p3=v7;f30(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);})){
v3=v8;
}
if(({uint64_t _a=({uint64_t _a=v3;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
if(({uint64_t _a=v8;uint64_t _b=v3;o_fp(8, 12, _a,_b);})){
v3=v8;
}
}
}
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);})){
v3=0ULL;
}
return o_leave(v3);
}
static uint64_t f32(uint64_t v0, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
if(({uint64_t _a=v0;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);})){
v2=1ULL;
}
if(({uint64_t _a=v1;uint64_t _b=v0;o_fp(8, 12, _a,_b);})){
v2=1ULL;
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
return o_leave(v1);
}
return o_leave(v0);
}
static uint64_t f33(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=32)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=2944ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
{uint64_t _i=v5;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=64ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, v5, 5);
if(({uint64_t _a=v6;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t _a=1344ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t p1=v7;f8(v1, v1_n, p1);});
if(({uint64_t _a=v8;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
uint64_t __attribute__((unused)) v9=({uint64_t p1=v4;uint64_t p2=5ULL;f2(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v10=({uint64_t _a=2944ULL;uint64_t _b=v9;o_op(1, 5, _a,_b);});
({uint64_t p1=v10;f9(v1, v1_n, p1);});
}
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f34(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=({uint64_t _a=4607182418800017408ULL;uint64_t _b=0;o_fp(17, 12, _a,_b);});
uint64_t __attribute__((unused)) v4=0ULL;
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t _a=64ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=o_get(v1, v1_n, v6, 5);
if(({uint64_t _a=v7;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v4=1ULL;
uint64_t __attribute__((unused)) v8=({uint64_t _a=1600ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t p1=v8;f8(v1, v1_n, p1);});
v3=({uint64_t p0=v3;uint64_t p1=v9;f32(p0, p1);});
}
if(({uint64_t _a=v7;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
v4=1ULL;
uint64_t __attribute__((unused)) v8=({uint64_t p2=v5;f31(v0, v0_n, v1, v1_n, p2);});
v3=({uint64_t p0=v3;uint64_t p1=v8;f32(p0, p1);});
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
({uint64_t p1=13ULL;uint64_t p2=v3;f11(v1, v1_n, p1, p2);});
return o_leave(v4);
}
static uint64_t f35(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
({f33(v0, v0_n, v1, v1_n);});
return o_leave(({f34(v0, v0_n, v1, v1_n);}));
}
static uint64_t f36(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p1=v2;uint64_t p2=1ULL;f3(v0, v0_n, p1, p2);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_fp(9, 12, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v5=({uint64_t p1=v2;uint64_t p2=3ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v4;uint64_t _b=v5;o_fp(4, 12, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=2112ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t p1=v7;f8(v1, v1_n, p1);});
if(({uint64_t _a=v8;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);})){
v8=v3;
}
uint64_t __attribute__((unused)) v9=({uint64_t _a=1856ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v10=({uint64_t p1=v9;f8(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v11=({uint64_t _a=v8;uint64_t _b=v10;o_fp(2, 12, _a,_b);});
uint64_t __attribute__((unused)) v12=0ULL;
uint64_t __attribute__((unused)) v13=({uint64_t _a=v11;uint64_t _b=v6;o_fp(2, 12, _a,_b);});
if(({uint64_t _a=v13;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
v12=v13;
}
uint64_t __attribute__((unused)) v14=({uint64_t _a=5536ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
({uint64_t p1=v14;uint64_t p2=v12;f11(v1, v1_n, p1, p2);});
return o_leave(0);
}
static uint64_t f37(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=1024)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t p1=v4;f0(v0, v0_n, p1);});
if(({uint64_t _a=v5;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t p1=v4;f1(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=832ULL;uint64_t _b=v6;o_op(1, 5, _a,_b);});
({uint64_t p1=v7;f9(v1, v1_n, p1);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f38(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=64ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
{uint64_t _i=v4;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v5=({uint64_t _a=4768ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
({uint64_t p1=v5;uint64_t p2=v3;f11(v1, v1_n, p1, p2);});
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, 1ULL, 5);
{uint64_t _i=1ULL;uint64_t _v=({uint64_t _a=v6;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v7=({uint64_t p1=v2;uint64_t p2=4ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v7;uint64_t _b=18446744073709551615ULL;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v8=({uint64_t _a=2368ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
({uint64_t p1=v8;f9(v1, v1_n, p1);});
}
({uint64_t p2=v2;f37(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v2;uint64_t p3=v3;f36(v0, v0_n, v1, v1_n, p2, p3);});
return o_leave(0);
}
static uint64_t f39(uint64_t v0){
o_enter();
uint64_t __attribute__((unused)) v1=4472406533629990549ULL;
if(({uint64_t _a=v0;uint64_t _b=4607182418800017408ULL;o_fp(10, 12, _a,_b);})){
return o_leave(({uint64_t _a=v1;uint64_t _b=v0;o_fp(3, 12, _a,_b);}));
}
return o_leave(v1);
}
static uint64_t f40(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=v2;uint64_t p2=5ULL;f2(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v4=({uint64_t p1=v2;uint64_t p2=3ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=2944ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, v5, 5);
return o_leave(({uint64_t _a=v4;uint64_t _b=o_fcv(12, 5, v6);o_fp(4, 12, _a,_b);}));
}
static uint64_t f41(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=1344ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f8(v1, v1_n, p1);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_fp(9, 12, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v7=({uint64_t p2=v2;f40(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v7;uint64_t _b=v3;o_fp(3, 12, _a,_b);});
v6=({uint64_t _a=v6;uint64_t _b=v8;o_fp(2, 12, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t p1=v2;uint64_t p2=1ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v10=({uint64_t p0=v9;f39(p0);});
if(({uint64_t _a=v6;uint64_t _b=v10;o_fp(9, 12, _a,_b);})){
v6=0ULL;
uint64_t __attribute__((unused)) v11=({uint64_t _a=2112ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
({uint64_t p1=v11;uint64_t p2=v4;f11(v1, v1_n, p1, p2);});
}
({uint64_t p1=v5;uint64_t p2=v6;f11(v1, v1_n, p1, p2);});
return o_leave(0);
}
static uint64_t f42(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=1600ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f8(v1, v1_n, p1);});
v6=({uint64_t _a=v6;uint64_t _b=v3;o_fp(2, 12, _a,_b);});
uint64_t __attribute__((unused)) v7=4472406533629990549ULL;
if(({uint64_t _a=v6;uint64_t _b=v7;o_fp(9, 12, _a,_b);})){
v6=0ULL;
uint64_t __attribute__((unused)) v8=({uint64_t p1=v2;uint64_t p2=6ULL;f2(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=2400ULL;uint64_t _b=v8;o_op(1, 5, _a,_b);});
{uint64_t _i=v9;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v10=({uint64_t _a=64ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
{uint64_t _i=v10;uint64_t _v=3ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v11=({uint64_t _a=1856ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
({uint64_t p1=v11;uint64_t p2=v4;f11(v1, v1_n, p1, p2);});
}
({uint64_t p1=v5;uint64_t p2=v6;f11(v1, v1_n, p1, p2);});
return o_leave(0);
}
static uint64_t f43(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=4472406533629990549ULL;
uint64_t __attribute__((unused)) v6=({uint64_t _a=1088ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t p1=v6;f8(v1, v1_n, p1);});
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
v7=({uint64_t _a=v7;uint64_t _b=v3;o_fp(2, 12, _a,_b);});
if(({uint64_t _a=v7;uint64_t _b=v5;o_fp(9, 12, _a,_b);})){
v7=0ULL;
}
({uint64_t p1=v6;uint64_t p2=v7;f11(v1, v1_n, p1, p2);});
}
({uint64_t p2=v2;uint64_t p3=v3;uint64_t p4=v4;f41(v0, v0_n, v1, v1_n, p2, p3, p4);});
v7=({uint64_t p1=v6;f8(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=1344ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t p1=v8;f8(v1, v1_n, p1);});
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_fp(9, 12, _a,_b);})){
if(({uint64_t _a=v9;uint64_t _b=0ULL;o_fp(9, 12, _a,_b);})){
({uint64_t p2=v2;uint64_t p3=v4;f38(v0, v0_n, v1, v1_n, p2, p3);});
}
}
return o_leave(0);
}
static uint64_t f44(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=({uint64_t p1=13ULL;f8(v1, v1_n, p1);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);})){
v3=0ULL;
}
uint64_t __attribute__((unused)) v4=({uint64_t p1=0ULL;f8(v1, v1_n, p1);});
v4=({uint64_t _a=v4;uint64_t _b=v3;o_fp(1, 12, _a,_b);});
({uint64_t p1=0ULL;uint64_t p2=v4;f11(v1, v1_n, p1, p2);});
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t _a=64ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v7=o_get(v1, v1_n, v6, 5);
if(({uint64_t _a=v7;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=v5;uint64_t p3=v3;uint64_t p4=v4;f42(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
if(({uint64_t _a=v7;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
({uint64_t p2=v5;uint64_t p3=v3;uint64_t p4=v4;f43(v0, v0_n, v1, v1_n, p2, p3, p4);});
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
uint64_t __attribute__((unused)) v6=({uint64_t p1=3ULL;f8(v1, v1_n, p1);});
if(({uint64_t _a=v4;uint64_t _b=v6;o_fp(10, 12, _a,_b);})){
({uint64_t p1=3ULL;uint64_t p2=v4;f11(v1, v1_n, p1, p2);});
}
return o_leave(0);
}
static uint64_t f45(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
uint64_t __attribute__((unused)) v3=o_get(v1, v1_n, 1ULL, 5);
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c1++>=2065)o_trap(4);
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
if(({uint64_t _a=v2;uint64_t _b=2064ULL;o_op(10, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=5ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(6ULL);
}
({f26(v0, v0_n, v1, v1_n);});
({f29(v0, v0_n, v1, v1_n);});
uint64_t __attribute__((unused)) v4=({f35(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(6ULL);
}
({f44(v0, v0_n, v1, v1_n);});
v3=o_get(v1, v1_n, 1ULL, 5);
}
}
return o_leave(0ULL);
}
static uint64_t f46(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=v2;uint64_t p2=0ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v4=({uint64_t p1=v2;uint64_t p2=1ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t p1=v2;uint64_t p2=2ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v6=v3;
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t p1=v2;uint64_t p2=3ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v4;uint64_t _b=v7;o_fp(4, 12, _a,_b);});
if(({uint64_t _a=v8;uint64_t _b=v6;o_fp(10, 12, _a,_b);})){
v6=v8;
}
}
v6=({uint64_t _a=v6;uint64_t _b=v5;o_fp(1, 12, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=5280ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
({uint64_t p1=v7;uint64_t p2=v6;f11(v1, v1_n, p1, p2);});
return o_leave(v6);
}
static uint64_t f47(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v4=0ULL;
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=1024)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f1(v0, v0_n, p1);});
if(({uint64_t _a=v6;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t p1=v5;f0(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=2976ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t p1=v8;f8(v1, v1_n, p1);});
if(({uint64_t _a=v9;uint64_t _b=v4;o_fp(10, 12, _a,_b);})){
v4=v9;
}
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v4);
}
static uint64_t f48(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t p1=v4;uint64_t p2=5ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v5;uint64_t _b=v1;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t p1=v4;uint64_t p2=1ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v7=({uint64_t p1=v4;uint64_t p2=3ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v6;uint64_t _b=v7;o_fp(4, 12, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=v8;o_fp(1, 12, _a,_b);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v3);
}
static uint64_t f49(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=({uint64_t p1=v1;uint64_t p2=1ULL;f4(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v4=0ULL;
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;uint64_t p2=4ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v6;uint64_t _b=v1;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t p1=v5;uint64_t p2=0ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v8=({uint64_t p1=v5;uint64_t p2=2ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=v7;uint64_t _b=v8;o_fp(1, 12, _a,_b);});
uint64_t __attribute__((unused)) v10=({uint64_t _a=v9;uint64_t _b=o_fcv(12, 5, v3);o_fp(4, 12, _a,_b);});
v4=({uint64_t _a=v4;uint64_t _b=v10;o_fp(1, 12, _a,_b);});
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v4);
}
static uint64_t f50(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t p1=v4;uint64_t p2=6ULL;f2(v0, v0_n, p1, p2);});
if(({uint64_t _a=v5;uint64_t _b=v1;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v6=({uint64_t p1=v4;uint64_t p2=2ULL;f3(v0, v0_n, p1, p2);});
v3=({uint64_t _a=v3;uint64_t _b=v6;o_fp(1, 12, _a,_b);});
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v3);
}
static uint64_t f51(const void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=({uint64_t p1=v1;uint64_t p2=0ULL;f4(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v3=0ULL;
if(({uint64_t _a=v2;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
v3=({uint64_t p1=v1;f48(v0, v0_n, p1);});
}
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v3=({uint64_t p1=v1;f49(v0, v0_n, p1);});
}
if(({uint64_t _a=v2;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
v3=({uint64_t p1=v1;f50(v0, v0_n, p1);});
}
return o_leave(v3);
}
static uint64_t f52(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=2432ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=o_get(v1, v1_n, v5, 5);
uint64_t __attribute__((unused)) v7=({uint64_t p2=v6;f46(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v8=({uint64_t p2=v6;f47(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=v8;uint64_t _b=v7;o_fp(1, 12, _a,_b);});
uint64_t __attribute__((unused)) v10=({uint64_t _a=2976ULL;uint64_t _b=v6;o_op(1, 5, _a,_b);});
({uint64_t p1=v10;uint64_t p2=v9;f11(v1, v1_n, p1, p2);});
if(({uint64_t _a=v9;uint64_t _b=v4;o_fp(10, 12, _a,_b);})){
v4=v9;
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v4);
}
static uint64_t f53(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v4=v2;
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=32)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f51(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=5792ULL;uint64_t _b=v5;o_op(1, 5, _a,_b);});
({uint64_t p1=v7;uint64_t p2=v6;f11(v1, v1_n, p1, p2);});
if(({uint64_t _a=v6;uint64_t _b=v4;o_fp(10, 12, _a,_b);})){
v4=v6;
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v4);
}
static uint64_t f54(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=({f52(v0, v0_n, v1, v1_n);});
({uint64_t p1=20ULL;uint64_t p2=v2;f11(v1, v1_n, p1, p2);});
uint64_t __attribute__((unused)) v3=({uint64_t p2=v2;f53(v0, v0_n, v1, v1_n, p2);});
({uint64_t p1=19ULL;uint64_t p2=v3;f11(v1, v1_n, p1, p2);});
return o_leave(0);
}
static uint64_t f55(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=v2;uint64_t p2=2ULL;f6(v0, v0_n, p1, p2);});
if(({uint64_t _a=v3;uint64_t _b=18446744073709551615ULL;o_op(6, 5, _a,_b);})){
return o_leave(0ULL);
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=4512ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
return o_leave(({uint64_t p1=v4;f8(v1, v1_n, p1);}));
}
static uint64_t f56(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t p1=v2;uint64_t p2=3ULL;f6(v0, v0_n, p1, p2);});
if(({uint64_t _a=v3;uint64_t _b=18446744073709551615ULL;o_op(6, 5, _a,_b);})){
return o_leave(({uint64_t p1=3ULL;f8(v1, v1_n, p1);}));
}
uint64_t __attribute__((unused)) v4=({uint64_t _a=4768ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
return o_leave(({uint64_t p1=v4;f8(v1, v1_n, p1);}));
}
static uint64_t f57(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=4472406533629990549ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t p2=v3;f55(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=v3;f56(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=v6;uint64_t _b=v4;o_fp(1, 12, _a,_b);});
if(({uint64_t _a=v7;uint64_t _b=v5;o_fp(8, 12, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=8ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p1=8ULL;uint64_t p2=v5;f11(v1, v1_n, p1, p2);});
({uint64_t p1=9ULL;uint64_t p2=v6;f11(v1, v1_n, p1, p2);});
return o_leave(8ULL);
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0ULL);
}
static uint64_t f58(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v4=0ULL;
uint64_t __attribute__((unused)) v5=0ULL;
uint64_t __attribute__((unused)) v6=4472406533629990549ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v7=({uint64_t p1=v5;uint64_t p2=1ULL;f6(v0, v0_n, p1, p2);});
if(({uint64_t _a=v7;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v8=({uint64_t p2=v5;f55(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v9=({uint64_t p2=v5;f56(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v10=({uint64_t _a=v8;uint64_t _b=v6;o_fp(1, 12, _a,_b);});
if(({uint64_t _a=v10;uint64_t _b=v9;o_fp(8, 12, _a,_b);})){
uint64_t __attribute__((unused)) v11=({uint64_t p1=v5;uint64_t p2=0ULL;f7(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v12=({uint64_t _a=3488ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
({uint64_t p1=v12;uint64_t p2=v8;f11(v1, v1_n, p1, p2);});
uint64_t __attribute__((unused)) v13=({uint64_t _a=4000ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
({uint64_t p1=v13;uint64_t p2=v11;f11(v1, v1_n, p1, p2);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
v12=({uint64_t _a=3488ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
({uint64_t p1=v12;uint64_t p2=v9;f11(v1, v1_n, p1, p2);});
v13=({uint64_t _a=4000ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v14=({uint64_t _a=v11;uint64_t _b=0;o_fp(17, 12, _a,_b);});
({uint64_t p1=v13;uint64_t p2=v14;f11(v1, v1_n, p1, p2);});
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v4);
}
static uint64_t f59(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=0ULL;
if(({uint64_t _a=v0;uint64_t _b=v1;o_fp(10, 12, _a,_b);})){
v4=1ULL;
}
if(({uint64_t _a=v0;uint64_t _b=v1;o_fp(6, 12, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_fp(8, 12, _a,_b);})){
v4=1ULL;
}
}
}
return o_leave(v4);
}
static uint64_t f60(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=3488ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);});
({uint64_t p1=v4;uint64_t p2=v2;f11(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=4000ULL;uint64_t _b=v1;o_op(1, 5, _a,_b);});
({uint64_t p1=v5;uint64_t p2=v3;f11(v0, v0_n, p1, p2);});
return o_leave(({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);}));
}
static uint64_t f61(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=3488ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f8(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=4000ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t p1=v7;f8(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v9=({uint64_t p0=v6;uint64_t p1=v2;uint64_t p2=v8;uint64_t p3=v3;f59(p0, p1, p2, p3);});
if(({uint64_t _a=v9;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
v1=({uint64_t p1=v1;uint64_t p2=v6;uint64_t p3=v8;f60(v0, v0_n, p1, p2, p3);});
}
return o_leave(v1);
}
static uint64_t f62(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=1ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
if(c1++>=512)o_trap(4);
uint64_t __attribute__((unused)) v3=({uint64_t _a=3488ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t p1=v3;f8(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=4000ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f8(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v7=v2;
uint64_t __attribute__((unused)) v8=1ULL;
{
uint64_t c2=0;
while(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(10, 5, _a,_b);})){
if(c2++>=512)o_trap(4);
if(({uint64_t _a=v8;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
break;
}
uint64_t __attribute__((unused)) v9=({uint64_t p1=v7;uint64_t p2=v4;uint64_t p3=v6;f61(v0, v0_n, p1, p2, p3);});
if(({uint64_t _a=v9;uint64_t _b=v7;o_op(6, 5, _a,_b);})){
v8=0ULL;
}
v7=v9;
}
}
v3=({uint64_t _a=3488ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
({uint64_t p1=v3;uint64_t p2=v4;f11(v0, v0_n, p1, p2);});
v5=({uint64_t _a=4000ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
({uint64_t p1=v5;uint64_t p2=v6;f11(v0, v0_n, p1, p2);});
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f63(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p2=v2;f55(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t p2=v2;f56(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v6=0ULL;
if(({uint64_t _a=v4;uint64_t _b=v3;o_fp(9, 12, _a,_b);})){
if(({uint64_t _a=v5;uint64_t _b=v3;o_fp(10, 12, _a,_b);})){
v6=1ULL;
}
}
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v7=({uint64_t _a=v2;uint64_t _b=64ULL;o_op(4, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t _a=v2;uint64_t _b=64ULL;o_op(5, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=6336ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v10=o_get(v1, v1_n, v9, 5);
uint64_t __attribute__((unused)) v11=({uint64_t _a=1ULL;uint64_t _b=v8;o_op(15, 5, _a,_b);});
{uint64_t _i=v9;uint64_t _v=({uint64_t _a=v10;uint64_t _b=v11;o_op(13, 5, _a,_b);});o_put(v1, v1_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f64(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
{uint64_t _i=6336ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6337ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6338ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6339ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 3ULL, 5);
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v4;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;uint64_t p2=1ULL;f6(v0, v0_n, p1, p2);});
if(({uint64_t _a=v6;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
({uint64_t p2=v5;uint64_t p3=v3;f63(v0, v0_n, v1, v1_n, p2, p3);});
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f65(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3, uint64_t v4){
o_enter();
uint64_t __attribute__((unused)) v5=({uint64_t _a=6344ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
({uint64_t p1=v5;uint64_t p2=v3;f11(v1, v1_n, p1, p2);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=6376ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
({uint64_t p1=v6;uint64_t p2=v4;f11(v1, v1_n, p1, p2);});
uint64_t __attribute__((unused)) v7=({uint64_t p1=v2;uint64_t p2=2ULL;f5(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v8=4472406533629990549ULL;
uint64_t __attribute__((unused)) v9=({uint64_t _a=v7;uint64_t _b=v8;o_fp(1, 12, _a,_b);});
if(({uint64_t _a=v3;uint64_t _b=v9;o_fp(10, 12, _a,_b);})){
({uint64_t p2=v2;uint64_t p3=v4;f64(v0, v0_n, v1, v1_n, p2, p3);});
{uint64_t _i=4ULL;uint64_t _v=4ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=8ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p1=8ULL;uint64_t p2=v3;f11(v1, v1_n, p1, p2);});
({uint64_t p1=9ULL;uint64_t p2=v4;f11(v1, v1_n, p1, p2);});
return o_leave(4ULL);
}
return o_leave(0ULL);
}
static uint64_t f66(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=0ULL;
uint64_t __attribute__((unused)) v5=0ULL;
uint64_t __attribute__((unused)) v6=0ULL;
uint64_t __attribute__((unused)) v7=0ULL;
uint64_t __attribute__((unused)) v8=4517329193108106637ULL;
uint64_t __attribute__((unused)) v9=({uint64_t _a=v8;uint64_t _b=0;o_fp(17, 12, _a,_b);});
{
uint64_t c1=0;
while(({uint64_t _a=v7;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=512)o_trap(4);
uint64_t __attribute__((unused)) v10=({uint64_t _a=4000ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v11=({uint64_t p1=v10;f8(v1, v1_n, p1);});
v4=({uint64_t _a=v4;uint64_t _b=v11;o_fp(1, 12, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=v9;o_fp(8, 12, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=8ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=7ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=6ULL;uint64_t _v=v2;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p1=8ULL;uint64_t p2=v4;f11(v1, v1_n, p1, p2);});
uint64_t __attribute__((unused)) v12=({uint64_t _a=3488ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v13=({uint64_t p1=v12;f8(v1, v1_n, p1);});
({uint64_t p1=9ULL;uint64_t p2=v13;f11(v1, v1_n, p1, p2);});
return o_leave(8ULL);
}
if(({uint64_t _a=v4;uint64_t _b=v5;o_fp(10, 12, _a,_b);})){
v5=v4;
uint64_t __attribute__((unused)) v12=({uint64_t _a=3488ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
v6=({uint64_t p1=v12;f8(v1, v1_n, p1);});
}
v7=({uint64_t _a=v7;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(({uint64_t p2=v2;uint64_t p3=v5;uint64_t p4=v6;f65(v0, v0_n, v1, v1_n, p2, p3, p4);}));
}
static uint64_t f67(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 1ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v3;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=32)o_trap(4);
uint64_t __attribute__((unused)) v4=({uint64_t p1=v3;uint64_t p2=0ULL;f4(v0, v0_n, p1, p2);});
if(({uint64_t _a=v4;uint64_t _b=3ULL;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v5=({uint64_t p2=v3;f58(v0, v0_n, v1, v1_n, p2);});
({uint64_t p1=v5;f62(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=v3;uint64_t p3=v5;f66(v0, v0_n, v1, v1_n, p2, p3);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v6);
}
}
v3=({uint64_t _a=v3;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0ULL);
}
static uint64_t f68(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 2ULL, 5);
uint64_t __attribute__((unused)) v4=18446744073709551615ULL;
uint64_t __attribute__((unused)) v5=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v5;uint64_t _b=v3;o_op(8, 5, _a,_b);})){
if(c1++>=1024)o_trap(4);
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f1(v0, v0_n, p1);});
if(({uint64_t _a=v6;uint64_t _b=v2;o_op(6, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t p1=v5;f0(v0, v0_n, p1);});
if(({uint64_t _a=v4;uint64_t _b=18446744073709551615ULL;o_op(6, 5, _a,_b);})){
v4=v7;
}
if(({uint64_t _a=v4;uint64_t _b=v7;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v8=({uint64_t _a=4768ULL;uint64_t _b=v7;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=({uint64_t p1=v8;f8(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v10=({uint64_t _a=4768ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v11=({uint64_t p1=v10;f8(v1, v1_n, p1);});
if(({uint64_t _a=v9;uint64_t _b=v11;o_fp(10, 12, _a,_b);})){
v4=v7;
}
}
}
v5=({uint64_t _a=v5;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v4);
}
static uint64_t f69(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t _a=4512ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=({uint64_t p1=v4;f8(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v6=0ULL;
if(({uint64_t _a=v3;uint64_t _b=18446744073709551615ULL;o_op(7, 5, _a,_b);})){
uint64_t __attribute__((unused)) v7=({uint64_t _a=4768ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
v6=({uint64_t p1=v7;f8(v1, v1_n, p1);});
}
uint64_t __attribute__((unused)) v7=({uint64_t _a=v5;uint64_t _b=v6;o_fp(2, 12, _a,_b);});
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_fp(9, 12, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v8=({uint64_t _a=320ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v9=o_get(v1, v1_n, v8, 5);
if(({uint64_t _a=v9;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=5828ULL;uint64_t p2=v7;f12(v1, v1_n, p1, p2);});
}
if(({uint64_t _a=v9;uint64_t _b=2ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=5827ULL;uint64_t p2=v7;f12(v1, v1_n, p1, p2);});
}
if(({uint64_t _a=v9;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=5830ULL;uint64_t p2=v7;f12(v1, v1_n, p1, p2);});
}
({uint64_t p1=12ULL;uint64_t p2=v7;f12(v1, v1_n, p1, p2);});
return o_leave(0);
}
static uint64_t f70(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2, uint64_t v3){
o_enter();
uint64_t __attribute__((unused)) v4=({uint64_t p1=v2;uint64_t p2=1ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v5=({uint64_t p1=v2;uint64_t p2=3ULL;f3(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v6=({uint64_t _a=v4;uint64_t _b=v5;o_fp(4, 12, _a,_b);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=5536ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t p1=v7;f8(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v9=({uint64_t _a=v6;uint64_t _b=v8;o_fp(1, 12, _a,_b);});
uint64_t __attribute__((unused)) v10=({uint64_t p1=v2;uint64_t p2=0ULL;f3(v0, v0_n, p1, p2);});
if(({uint64_t _a=v10;uint64_t _b=v9;o_fp(11, 12, _a,_b);})){
({uint64_t p1=5824ULL;uint64_t p2=v3;f12(v1, v1_n, p1, p2);});
}
if(({uint64_t _a=({uint64_t _a=v10;uint64_t _b=v9;o_fp(11, 12, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
({uint64_t p1=5825ULL;uint64_t p2=v6;f12(v1, v1_n, p1, p2);});
({uint64_t p1=5826ULL;uint64_t p2=v8;f12(v1, v1_n, p1, p2);});
if(({uint64_t _a=v3;uint64_t _b=v9;o_fp(10, 12, _a,_b);})){
uint64_t __attribute__((unused)) v11=({uint64_t _a=v3;uint64_t _b=v9;o_fp(2, 12, _a,_b);});
({uint64_t p1=5830ULL;uint64_t p2=v11;f12(v1, v1_n, p1, p2);});
}
}
return o_leave(0);
}
static uint64_t f71(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n, uint64_t v2){
o_enter();
uint64_t __attribute__((unused)) v3=({uint64_t _a=4768ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t p1=v3;f8(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v5=({uint64_t _a=1856ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f8(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=4512ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t p1=v7;f8(v1, v1_n, p1);});
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_fp(11, 12, _a,_b);})){
v8=v6;
}
uint64_t __attribute__((unused)) v9=({uint64_t _a=v4;uint64_t _b=v8;o_fp(2, 12, _a,_b);});
uint64_t __attribute__((unused)) v10=({uint64_t p1=v2;uint64_t p2=2ULL;f3(v0, v0_n, p1, p2);});
({uint64_t p1=5827ULL;uint64_t p2=v10;f12(v1, v1_n, p1, p2);});
uint64_t __attribute__((unused)) v11=({uint64_t p1=12ULL;f8(v1, v1_n, p1);});
v11=({uint64_t _a=v11;uint64_t _b=v10;o_fp(1, 12, _a,_b);});
uint64_t __attribute__((unused)) v12=({uint64_t p1=v2;uint64_t p2=7ULL;f2(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v13=({uint64_t p1=v2;uint64_t p2=1ULL;f3(v0, v0_n, p1, p2);});
if(({uint64_t _a=v12;uint64_t _b=4ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=5829ULL;uint64_t p2=v9;f12(v1, v1_n, p1, p2);});
}
if(({uint64_t _a=v12;uint64_t _b=4ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v13;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
({uint64_t p2=v2;uint64_t p3=v9;f70(v0, v0_n, v1, v1_n, p2, p3);});
}
if(({uint64_t _a=({uint64_t _a=v13;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);});uint64_t _b=0;o_op(18, 1, _a,_b);})){
({uint64_t p1=5824ULL;uint64_t p2=v9;f12(v1, v1_n, p1, p2);});
}
}
v11=({uint64_t _a=v11;uint64_t _b=v9;o_fp(1, 12, _a,_b);});
({uint64_t p1=12ULL;uint64_t p2=v11;f11(v1, v1_n, p1, p2);});
return o_leave(0);
}
static uint64_t f72(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
uint64_t __attribute__((unused)) v3=0ULL;
uint64_t __attribute__((unused)) v4=1ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=v2;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v5=({uint64_t _a=4768ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v6=({uint64_t p1=v5;f8(v1, v1_n, p1);});
uint64_t __attribute__((unused)) v7=({uint64_t _a=4768ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v8=({uint64_t p1=v7;f8(v1, v1_n, p1);});
if(({uint64_t _a=v6;uint64_t _b=v8;o_fp(10, 12, _a,_b);})){
v3=v4;
}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(v3);
}
static uint64_t f73(void *v0, uint64_t v0_n, uint64_t v1){
o_enter();
uint64_t __attribute__((unused)) v2=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v2;uint64_t _b=v1;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=1ULL;o_op(2, 5, _a,_b);});
v3=({uint64_t _a=v3;uint64_t _b=v2;o_op(2, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=3232ULL;uint64_t _b=v3;o_op(1, 5, _a,_b);});
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, v4, 5);
uint64_t __attribute__((unused)) v6=({uint64_t _a=6080ULL;uint64_t _b=v2;o_op(1, 5, _a,_b);});
{uint64_t _i=v6;uint64_t _v=v5;o_put(v0, v0_n, _i,_v,5);}
v2=({uint64_t _a=v2;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
}
}
return o_leave(0);
}
static uint64_t f74(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=({uint64_t p1=3ULL;f8(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v2=({uint64_t p1=12ULL;f8(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v3=({uint64_t _a=v1;uint64_t _b=v2;o_fp(2, 12, _a,_b);});
uint64_t __attribute__((unused)) v4=4517329193108106637ULL;
uint64_t __attribute__((unused)) v5=0ULL;
if(({uint64_t _a=v3;uint64_t _b=v4;o_fp(10, 12, _a,_b);})){
v5=1ULL;
}
if(({uint64_t _a=v3;uint64_t _b=({uint64_t _a=v4;uint64_t _b=0;o_fp(17, 12, _a,_b);});o_fp(8, 12, _a,_b);})){
v5=1ULL;
}
if(({uint64_t _a=v5;uint64_t _b=1ULL;o_op(6, 5, _a,_b);})){
({uint64_t p1=5830ULL;uint64_t p2=v3;f12(v0, v0_n, p1, p2);});
}
return o_leave(0);
}
static uint64_t f75(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 0ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=({f72(v0, v0_n, v1, v1_n);});
{uint64_t _i=12ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
uint64_t __attribute__((unused)) v4=0ULL;
{
uint64_t c1=0;
while(({uint64_t _a=v4;uint64_t _b=256ULL;o_op(8, 5, _a,_b);})){
if(c1++>=256)o_trap(4);
if(({uint64_t _a=v3;uint64_t _b=18446744073709551615ULL;o_op(6, 5, _a,_b);})){
break;
}
uint64_t __attribute__((unused)) v5=({uint64_t _a=3232ULL;uint64_t _b=v4;o_op(1, 5, _a,_b);});
{uint64_t _i=v5;uint64_t _v=v3;o_put(v1, v1_n, _i,_v,5);}
v4=({uint64_t _a=v4;uint64_t _b=1ULL;o_op(1, 5, _a,_b);});
({uint64_t p2=v3;f71(v0, v0_n, v1, v1_n, p2);});
uint64_t __attribute__((unused)) v6=({uint64_t p2=v3;f68(v0, v0_n, v1, v1_n, p2);});
({uint64_t p2=v3;uint64_t p3=v6;f69(v0, v0_n, v1, v1_n, p2, p3);});
v3=v6;
}
}
{uint64_t _i=21ULL;uint64_t _v=v4;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p1=v4;f73(v1, v1_n, p1);});
({f74(v1, v1_n);});
return o_leave(0);
}
static uint64_t f76(void *v0, uint64_t v0_n){
o_enter();
uint64_t __attribute__((unused)) v1=o_get(v0, v0_n, 11ULL, 5);
uint64_t __attribute__((unused)) v2=0ULL;
uint64_t __attribute__((unused)) v3=({uint64_t _a=1ULL;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
uint64_t __attribute__((unused)) v4=({uint64_t _a=v1;uint64_t _b=v3;o_op(12, 5, _a,_b);});
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
v2=1ULL;
}
{uint64_t _i=26ULL;uint64_t _v=v2;o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f77(void *v0, uint64_t v0_n, uint64_t v1, uint64_t v2){
o_enter();
if(({uint64_t _a=v2;uint64_t _b=1ULL;o_op(7, 5, _a,_b);})){
return o_leave(0);
}
uint64_t __attribute__((unused)) v3=({uint64_t p1=3ULL;f8(v0, v0_n, p1);});
uint64_t __attribute__((unused)) v4=4472406533629990549ULL;
uint64_t __attribute__((unused)) v5=({uint64_t _a=v1;uint64_t _b=v3;o_fp(3, 12, _a,_b);});
v5=({uint64_t _a=v5;uint64_t _b=v4;o_fp(3, 12, _a,_b);});
{uint64_t _i=23ULL;uint64_t _v=1ULL;o_put(v0, v0_n, _i,_v,5);}
({uint64_t p1=24ULL;uint64_t p2=v5;f11(v0, v0_n, p1, p2);});
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, 11ULL, 5);
uint64_t __attribute__((unused)) v7=({uint64_t _a=1ULL;uint64_t _b=1ULL;o_op(15, 5, _a,_b);});
{uint64_t _i=11ULL;uint64_t _v=({uint64_t _a=v6;uint64_t _b=v7;o_op(13, 5, _a,_b);});o_put(v0, v0_n, _i,_v,5);}
return o_leave(0);
}
static uint64_t f78(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 4ULL, 5);
uint64_t __attribute__((unused)) v3=o_get(v0, v0_n, 5ULL, 5);
uint64_t __attribute__((unused)) v4=o_get(v0, v0_n, 6ULL, 5);
uint64_t __attribute__((unused)) v5=o_get(v0, v0_n, 7ULL, 5);
uint64_t __attribute__((unused)) v6=o_get(v0, v0_n, 8ULL, 5);
uint64_t __attribute__((unused)) v7=0ULL;
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
if(({uint64_t _a=v6;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=22ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
({uint64_t p1=24ULL;uint64_t p2=v3;f11(v1, v1_n, p1, p2);});
v7=1ULL;
}
}
}
if(({uint64_t _a=v7;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
if(({uint64_t _a=v4;uint64_t _b=0ULL;o_fp(10, 12, _a,_b);})){
({uint64_t p1=v4;uint64_t p2=v5;f77(v1, v1_n, p1, p2);});
}
}
}
({f76(v1, v1_n);});
return o_leave(0);
}
static uint64_t f79(const void *v0, uint64_t v0_n, void *v1, uint64_t v1_n){
o_enter();
uint64_t __attribute__((unused)) v2=o_get(v0, v0_n, 9ULL, 5);
if(({uint64_t _a=v2;uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
if(({uint64_t _a=o_get(v0, v0_n, 0ULL, 5);uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=6ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=1ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(6ULL);
}
{uint64_t _i=4ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
if(({uint64_t _a=({f16(v0, v0_n, v1, v1_n);});uint64_t _b=0ULL;o_op(6, 5, _a,_b);})){
{uint64_t _i=4ULL;uint64_t _v=5ULL;o_put(v1, v1_n, _i,_v,5);}
{uint64_t _i=5ULL;uint64_t _v=2ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(5ULL);
}
uint64_t __attribute__((unused)) v3=({f17(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v3);
}
({f21(v0, v0_n, v1, v1_n);});
({f25(v0, v0_n, v1, v1_n);});
v3=({f45(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v3);
}
({f54(v0, v0_n, v1, v1_n);});
v3=({f57(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v3);
}
v3=({f67(v0, v0_n, v1, v1_n);});
if(({uint64_t _a=v3;uint64_t _b=0ULL;o_op(7, 5, _a,_b);})){
return o_leave(v3);
}
({f75(v0, v0_n, v1, v1_n);});
({f78(v0, v0_n, v1, v1_n);});
{uint64_t _i=4ULL;uint64_t _v=0ULL;o_put(v1, v1_n, _i,_v,5);}
return o_leave(0ULL);
}
uint32_t omega_dag_eval(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n){
if(!o_buf(result,1,1,5))return 32;
if(!o_buf(p0, p0_n,8192, 5))return 32;
if(o_overlap(result,8,p0, p0_n*8))return 32;
if(!o_buf(p1, p1_n,8192, 5))return 32;
if(o_overlap(result,8,p1, p1_n*8))return 32;
if(o_overlap(p1, p1_n*8, p0, p0_n*8))return 32;
if(o_busy)return 33;
o_busy=1;o_depth=0;int code=setjmp(o_jmp);
if(!code)*result=f79(p0, p0_n, p1, p1_n);
o_busy=0;o_depth=0;return (uint32_t)code;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
