/* Omega Bridge portable nontrapping scalar profile. No host runtime. */
#include <stdint.h>
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
static unsigned ombr_fs_width(unsigned t){static const unsigned w[]={0,1,8,16,32,64,8,16,32,64};return w[t];}
static int ombr_fs_valid(unsigned t,uint64_t v){unsigned w=ombr_fs_width(t);if(t<6)return w==64||v<((uint64_t)1<<w);if(w==64)return 1;uint64_t m=(uint64_t)1<<(w-1);return v<m||v>=UINT64_MAX-m+1;}
static uint64_t ombr_fs_op(unsigned op,unsigned t,uint64_t a,uint64_t b){unsigned w=ombr_fs_width(t);uint64_t mask=UINT64_MAX>>(64-w);if(op>=6&&op<=11){uint64_t x=a,y=b;if(t>=6){x^=(uint64_t)1<<63;y^=(uint64_t)1<<63;}switch(op){case 6:return x==y;case 7:return x!=y;case 8:return x<y;case 9:return x<=y;case 10:return x>y;default:return x>=y;}}switch(op){case 1:return (a+b)&mask;case 2:return (a-b)&mask;case 3:return (a*b)&mask;case 12:return a&b;case 13:return a|b;case 14:return a^b;case 15:return (a<<b)&mask;case 16:return a>>b;case 17:return (0-a)&mask;default:return t==1?!a:t>=6?~a:(~a)&mask;}}
static uint64_t f0(uint64_t v0);
static uint64_t f1(uint64_t v0);
static uint64_t f2(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f0(uint64_t v0){
if(ombr_fs_op(8, 5, v0, 2ULL)){
return ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(2, 5, 113ULL, 10ULL), 23ULL), ombr_fs_op(15, 5, ombr_fs_op(12, 5, ombr_fs_op(15, 5, v0, 10ULL), 1023ULL), 13ULL));
}
if(ombr_fs_op(8, 5, v0, 4ULL)){
return ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(2, 5, 113ULL, 9ULL), 23ULL), ombr_fs_op(15, 5, ombr_fs_op(12, 5, ombr_fs_op(15, 5, v0, 9ULL), 1023ULL), 13ULL));
}
if(ombr_fs_op(8, 5, v0, 8ULL)){
return ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(2, 5, 113ULL, 8ULL), 23ULL), ombr_fs_op(15, 5, ombr_fs_op(12, 5, ombr_fs_op(15, 5, v0, 8ULL), 1023ULL), 13ULL));
}
if(ombr_fs_op(8, 5, v0, 16ULL)){
return ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(2, 5, 113ULL, 7ULL), 23ULL), ombr_fs_op(15, 5, ombr_fs_op(12, 5, ombr_fs_op(15, 5, v0, 7ULL), 1023ULL), 13ULL));
}
if(ombr_fs_op(8, 5, v0, 32ULL)){
return ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(2, 5, 113ULL, 6ULL), 23ULL), ombr_fs_op(15, 5, ombr_fs_op(12, 5, ombr_fs_op(15, 5, v0, 6ULL), 1023ULL), 13ULL));
}
if(ombr_fs_op(8, 5, v0, 64ULL)){
return ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(2, 5, 113ULL, 5ULL), 23ULL), ombr_fs_op(15, 5, ombr_fs_op(12, 5, ombr_fs_op(15, 5, v0, 5ULL), 1023ULL), 13ULL));
}
if(ombr_fs_op(8, 5, v0, 128ULL)){
return ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(2, 5, 113ULL, 4ULL), 23ULL), ombr_fs_op(15, 5, ombr_fs_op(12, 5, ombr_fs_op(15, 5, v0, 4ULL), 1023ULL), 13ULL));
}
if(ombr_fs_op(8, 5, v0, 256ULL)){
return ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(2, 5, 113ULL, 3ULL), 23ULL), ombr_fs_op(15, 5, ombr_fs_op(12, 5, ombr_fs_op(15, 5, v0, 3ULL), 1023ULL), 13ULL));
}
if(ombr_fs_op(8, 5, v0, 512ULL)){
return ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(2, 5, 113ULL, 2ULL), 23ULL), ombr_fs_op(15, 5, ombr_fs_op(12, 5, ombr_fs_op(15, 5, v0, 2ULL), 1023ULL), 13ULL));
}
return ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(2, 5, 113ULL, 1ULL), 23ULL), ombr_fs_op(15, 5, ombr_fs_op(12, 5, ombr_fs_op(15, 5, v0, 1ULL), 1023ULL), 13ULL));
}
static uint64_t f1(uint64_t v0){
uint64_t v1=ombr_fs_op(15, 5, ombr_fs_op(12, 5, v0, 32768ULL), 16ULL);
uint64_t v2=ombr_fs_op(12, 5, ombr_fs_op(16, 5, v0, 10ULL), 31ULL);
uint64_t v3=ombr_fs_op(12, 5, v0, 1023ULL);
if(ombr_fs_op(6, 5, v2, 0ULL)){
if(ombr_fs_op(6, 5, v3, 0ULL)){
return v1;
}
return ombr_fs_op(13, 5, v1, f0(v3));
}else{
if(ombr_fs_op(6, 5, v2, 31ULL)){
return ombr_fs_op(13, 5, v1, ombr_fs_op(13, 5, 2139095040ULL, ombr_fs_op(15, 5, v3, 13ULL)));
}
}
return ombr_fs_op(12, 5, ombr_fs_op(13, 5, v1, ombr_fs_op(13, 5, ombr_fs_op(15, 5, ombr_fs_op(1, 5, v2, 112ULL), 23ULL), ombr_fs_op(15, 5, v3, 13ULL))), 4294967295ULL);
}
static uint64_t f2(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
if(ombr_fs_op(6, 5, v0, 1ULL)){
return f1(v1);
}
if(ombr_fs_op(6, 5, v0, 2ULL)){
return ombr_fs_op(15, 5, ombr_fs_op(12, 5, v1, 65535ULL), 16ULL);
}
if(ombr_fs_op(6, 5, v0, 3ULL)){
if(ombr_fs_op(12, 1, ombr_fs_op(6, 5, ombr_fs_op(12, 5, v1, 2139095040ULL), 2139095040ULL), ombr_fs_op(7, 5, ombr_fs_op(12, 5, v1, 8388607ULL), 0ULL))){
return ombr_fs_op(13, 5, ombr_fs_op(16, 5, v1, 16ULL), 64ULL);
}
return ombr_fs_op(12, 5, ombr_fs_op(16, 5, ombr_fs_op(12, 5, ombr_fs_op(1, 5, ombr_fs_op(1, 5, v1, 32767ULL), ombr_fs_op(12, 5, ombr_fs_op(16, 5, v1, 16ULL), 1ULL)), 4294967295ULL), 16ULL), 65535ULL);
}
if(ombr_fs_op(6, 5, v0, 4ULL)){
if(ombr_fs_op(10, 5, v1, ombr_fs_op(2, 5, ombr_fs_op(2, 5, 18446744073709551615ULL, v2), 4095ULL))){
return 0ULL;
}
uint64_t v6=ombr_fs_op(12, 5, ombr_fs_op(1, 5, ombr_fs_op(1, 5, v1, v2), 4095ULL), 18446744073709547520ULL);
if(ombr_fs_op(8, 5, v6, 4096ULL)){
return 4096ULL;
}
return v6;
}
if(ombr_fs_op(6, 5, v0, 7ULL)){
if(ombr_fs_op(6, 5, v2, 0ULL)){
return v1;
}
return 18446744073709551615ULL;
}
if(ombr_fs_op(6, 5, v0, 8ULL)){
if(ombr_fs_op(12, 1, ombr_fs_op(6, 5, v1, 0ULL), ombr_fs_op(6, 5, v2, 0ULL))){
return 0ULL;
}
return 18446744073709551615ULL;
}
if(ombr_fs_op(6, 5, v0, 9ULL)){
if(ombr_fs_op(12, 1, ombr_fs_op(10, 5, v1, 0ULL), ombr_fs_op(8, 5, v1, 9223372036854775808ULL))){
return ombr_fs_op(2, 5, v1, 1ULL);
}
return 18446744073709551615ULL;
}
if(ombr_fs_op(6, 5, v0, 10ULL)){
if(ombr_fs_op(7, 5, v1, 0ULL)){
return 2ULL;
}
return 3ULL;
}
if(ombr_fs_op(6, 5, v0, 11ULL)){
if(ombr_fs_op(7, 5, v1, 0ULL)){
return 14ULL;
}
return 13ULL;
}
if(ombr_fs_op(6, 5, v0, 12ULL)){
if(ombr_fs_op(6, 5, v2, 0ULL)){
return v1;
}
return 0ULL;
}
if(ombr_fs_op(6, 5, v0, 13ULL)){
if(ombr_fs_op(7, 5, v1, 0ULL)){
return 1ULL;
}
return 0ULL;
}
if(ombr_fs_op(6, 5, v0, 14ULL)){
if(ombr_fs_op(13, 1, ombr_fs_op(10, 5, v1, 32768ULL), ombr_fs_op(10, 5, v2, 16384ULL))){
return 0ULL;
}
if(ombr_fs_op(13, 1, ombr_fs_op(7, 5, v3, 384ULL), ombr_fs_op(13, 1, ombr_fs_op(7, 5, v4, 384ULL), ombr_fs_op(10, 5, v5, 128ULL)))){
return 0ULL;
}
return 1ULL;
}
if(ombr_fs_op(6, 5, v0, 15ULL)){
if(ombr_fs_op(9, 5, v1, 4096ULL)){
return 1ULL;
}
return 0ULL;
}
return 18446744073709551615ULL;
}
uint32_t omega_bw_scalar(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5){
if(!result || (uintptr_t)result%_Alignof(uint64_t) || (uintptr_t)result>UINTPTR_MAX-sizeof(uint64_t))return 32;
if(!ombr_fs_valid(5, p0))return 32;
if(!ombr_fs_valid(5, p1))return 32;
if(!ombr_fs_valid(5, p2))return 32;
if(!ombr_fs_valid(5, p3))return 32;
if(!ombr_fs_valid(5, p4))return 32;
if(!ombr_fs_valid(5, p5))return 32;
*result=f2(p0, p1, p2, p3, p4, p5);
return 0;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
