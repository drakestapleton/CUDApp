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
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f2(uint64_t v0, uint64_t v1);
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f0(uint64_t v0){
if(v0){
return 1ULL;
}
return 0ULL;
}
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v4, 1ULL), 0ULL)){
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v0, 4294967295ULL), ombr_fs_op(12, 5, v2, 4294967295ULL))){
return 0ULL;
}
}
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v4, 2ULL), 0ULL)){
if(ombr_fs_op(7, 5, ombr_fs_op(16, 5, v0, 32ULL), ombr_fs_op(16, 5, v2, 32ULL))){
return 0ULL;
}
}
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v4, 4ULL), 0ULL)){
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v1, 4294967295ULL), ombr_fs_op(12, 5, v3, 4294967295ULL))){
return 0ULL;
}
}
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v4, 8ULL), 0ULL)){
if(ombr_fs_op(7, 5, ombr_fs_op(16, 5, v1, 32ULL), ombr_fs_op(16, 5, v3, 32ULL))){
return 0ULL;
}
}
return 1ULL;
}
static uint64_t f2(uint64_t v0, uint64_t v1){
uint64_t v2=v0;
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v1, 1ULL), 0ULL)){
v2=ombr_fs_op(15, 5, v2, 1ULL);
}
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v1, 2ULL), 0ULL)){
v2=ombr_fs_op(15, 5, v2, 2ULL);
}
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v1, 4ULL), 0ULL)){
v2=ombr_fs_op(15, 5, v2, 4ULL);
}
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v1, 8ULL), 0ULL)){
v2=ombr_fs_op(15, 5, v2, 8ULL);
}
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v1, 16ULL), 0ULL)){
v2=ombr_fs_op(15, 5, v2, 16ULL);
}
if(ombr_fs_op(7, 5, ombr_fs_op(12, 5, v1, 32ULL), 0ULL)){
v2=ombr_fs_op(15, 5, v2, 32ULL);
}
return v2;
}
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
if(ombr_fs_op(6, 5, v0, 0ULL)){
return f1(v1, v2, v3, v4, v5);
}
if(ombr_fs_op(6, 5, v0, 7ULL)){
uint64_t v6=7ULL;
if(ombr_fs_op(7, 5, ombr_fs_op(16, 5, v4, 32ULL), 0ULL)){
v6=15ULL;
}
return f1(v1, v2, v3, v4, v6);
}
if(ombr_fs_op(6, 5, v0, 1ULL)){
return ombr_fs_op(12, 5, ombr_fs_op(2, 5, 0ULL, ombr_fs_op(1, 5, ombr_fs_op(3, 5, v1, 2ULL), v2)), 4294967295ULL);
}
if(ombr_fs_op(6, 5, v0, 2ULL)){
if(ombr_fs_op(6, 5, v2, 0ULL)){
return f0(ombr_fs_op(7, 5, v1, 0ULL));
}
return f0(ombr_fs_op(6, 5, v1, 0ULL));
}
if(ombr_fs_op(6, 5, v0, 3ULL)){
if(ombr_fs_op(6, 5, v5, 0ULL)){
return f0(ombr_fs_op(6, 5, ombr_fs_op(12, 5, ombr_fs_op(14, 5, v2, v4), 4294967295ULL), 262144ULL));
}
return f0(ombr_fs_op(12, 1, ombr_fs_op(6, 5, v1, v3), ombr_fs_op(6, 5, ombr_fs_op(16, 5, v2, 32ULL), ombr_fs_op(16, 5, v4, 32ULL))));
}
if(ombr_fs_op(6, 5, v0, 4ULL)){
return f0(ombr_fs_op(12, 1, ombr_fs_op(6, 5, v1, ombr_fs_op(13, 5, v3, 288230376151711744ULL)), ombr_fs_op(6, 5, v2, v4)));
}
if(ombr_fs_op(6, 5, v0, 5ULL)){
uint64_t v6=f2(v2, v3);
uint64_t v7=f2(v4, v5);
return ombr_fs_op(13, 5, ombr_fs_op(13, 5, v1, v6), v7);
}
if(ombr_fs_op(6, 5, v0, 6ULL)){
if(ombr_fs_op(6, 5, v1, v2)){
return v3;
}
return v4;
}
return 0ULL;
}
uint32_t omega_fv_run(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5){
if(!result || (uintptr_t)result%_Alignof(uint64_t) || (uintptr_t)result>UINTPTR_MAX-sizeof(uint64_t))return 32;
if(!ombr_fs_valid(5, p0))return 32;
if(!ombr_fs_valid(5, p1))return 32;
if(!ombr_fs_valid(5, p2))return 32;
if(!ombr_fs_valid(5, p3))return 32;
if(!ombr_fs_valid(5, p4))return 32;
if(!ombr_fs_valid(5, p5))return 32;
*result=f3(p0, p1, p2, p3, p4, p5);
return 0;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
