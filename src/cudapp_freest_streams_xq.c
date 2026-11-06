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
static uint64_t f0(uint64_t v0, uint64_t v1);
static uint64_t f0(uint64_t v0, uint64_t v1){
if(ombr_fs_op(13, 1, ombr_fs_op(6, 5, v0, 0ULL), ombr_fs_op(10, 5, v0, 3ULL))){
return 0ULL;
}
uint64_t v2=ombr_fs_op(12, 5, ombr_fs_op(16, 5, v1, 1ULL), 1ULL);
uint64_t v3=ombr_fs_op(12, 5, ombr_fs_op(16, 5, v1, 2ULL), 1ULL);
uint64_t v4=ombr_fs_op(12, 5, ombr_fs_op(16, 5, v1, 3ULL), 1ULL);
if(ombr_fs_op(6, 5, v0, 1ULL)){
if(ombr_fs_op(7, 5, v2, 0ULL)){
return 1ULL;
}
return 3ULL;
}
if(ombr_fs_op(6, 5, v0, 2ULL)){
if(ombr_fs_op(7, 5, v3, 0ULL)){
return 1ULL;
}
if(ombr_fs_op(6, 5, v2, 0ULL)){
return 2ULL;
}
return 3ULL;
}
if(ombr_fs_op(7, 5, v4, 0ULL)){
return 1ULL;
}
if(ombr_fs_op(13, 1, ombr_fs_op(6, 5, v2, 0ULL), ombr_fs_op(6, 5, v3, 0ULL))){
return 2ULL;
}
return 3ULL;
}
uint32_t omega_streams_xq(uint64_t *result, uint64_t p0, uint64_t p1){
if(!result || (uintptr_t)result%_Alignof(uint64_t) || (uintptr_t)result>UINTPTR_MAX-sizeof(uint64_t))return 32;
if(!ombr_fs_valid(5, p0))return 32;
if(!ombr_fs_valid(5, p1))return 32;
*result=f0(p0, p1);
return 0;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
