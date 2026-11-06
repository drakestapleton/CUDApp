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
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4);
static uint64_t f2(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);
static uint64_t f4(uint64_t v0, uint64_t v1, uint64_t v2);
static uint64_t f5(uint64_t v0, uint64_t v1);
static uint64_t f6(uint64_t v0);
static uint64_t f7(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5);
static uint64_t f0(uint64_t v0, uint64_t v1){
if(ombr_fs_op(6, 5, v1, 0ULL)){
return 0ULL;
}
if(ombr_fs_op(10, 5, v0, ombr_fs_op(2, 5, 18446744073709551615ULL, v1))){
return 0ULL;
}
return ombr_fs_op(1, 5, v0, v1);
}
static uint64_t f1(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4){
if(ombr_fs_op(13, 1, v4, ombr_fs_op(8, 5, v0, v1))){
return 3ULL;
}
if(ombr_fs_op(13, 1, ombr_fs_op(6, 5, v2, 0ULL), ombr_fs_op(11, 5, v0, v2))){
return 3ULL;
}
if(v3){
return 1ULL;
}
return 2ULL;
}
static uint64_t f2(uint64_t v0, uint64_t v1, uint64_t v2){
return ombr_fs_op(12, 1, ombr_fs_op(6, 4, v0, 1ULL), ombr_fs_op(12, 1, ombr_fs_op(7, 5, v1, 0ULL), ombr_fs_op(6, 5, v1, v2)));
}
static uint64_t f3(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3){
if(ombr_fs_op(13, 1, ombr_fs_op(6, 5, v0, 0ULL), ombr_fs_op(7, 5, v0, v1))){
return 0ULL;
}
return ombr_fs_op(12, 1, ombr_fs_op(7, 5, v2, 0ULL), ombr_fs_op(6, 5, v2, v3));
}
static uint64_t f4(uint64_t v0, uint64_t v1, uint64_t v2){
if(ombr_fs_op(10, 5, v1, v0)){
return 0ULL;
}
return ombr_fs_op(9, 5, v2, ombr_fs_op(2, 5, v0, v1));
}
static uint64_t f5(uint64_t v0, uint64_t v1){
if(ombr_fs_op(18, 1, v1, 0)){
return v0;
}
if(ombr_fs_op(6, 4, v0, 3ULL)){
return 4ULL;
}
if(ombr_fs_op(6, 4, v0, 4ULL)){
return 5ULL;
}
if(ombr_fs_op(6, 4, v0, 5ULL)){
return 6ULL;
}
if(ombr_fs_op(6, 4, v0, 6ULL)){
return 7ULL;
}
if(ombr_fs_op(6, 4, v0, 7ULL)){
return 8ULL;
}
if(ombr_fs_op(6, 4, v0, 8ULL)){
return 9ULL;
}
if(ombr_fs_op(6, 4, v0, 9ULL)){
return 1ULL;
}
return v0;
}
static uint64_t f6(uint64_t v0){
if(ombr_fs_op(13, 1, ombr_fs_op(6, 5, v0, 0ULL), ombr_fs_op(6, 5, v0, 18446744073709551615ULL))){
return 0ULL;
}
return ombr_fs_op(1, 5, v0, 1ULL);
}
static uint64_t f7(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4, uint64_t v5){
if(ombr_fs_op(6, 5, v0, 0ULL)){
if(ombr_fs_op(10, 5, v2, 18446744073709ULL)){
return 0ULL;
}
return f0(v1, ombr_fs_op(3, 5, v2, 1000000ULL));
}
if(ombr_fs_op(6, 5, v0, 1ULL)){
return f1(v1, v2, v3, ombr_fs_op(7, 5, v4, 0ULL), ombr_fs_op(7, 5, v5, 0ULL));
}
if(ombr_fs_op(6, 5, v0, 2ULL)){
if(ombr_fs_op(11, 5, v1, v2)){
return 0ULL;
}
if(ombr_fs_op(12, 1, ombr_fs_op(7, 5, v3, 0ULL), ombr_fs_op(11, 5, ombr_fs_op(2, 5, v2, v1), 1000000ULL))){
return 1ULL;
}
return 0ULL;
}
if(ombr_fs_op(6, 5, v0, 3ULL)){
if(ombr_fs_op(12, 1, ombr_fs_op(7, 5, v1, 0ULL), ombr_fs_op(12, 1, ombr_fs_op(7, 5, v2, 0ULL), ombr_fs_op(7, 5, v3, 0ULL)))){
return 1ULL;
}
return 0ULL;
}
if(ombr_fs_op(6, 5, v0, 4ULL)){
uint64_t v6=f5(3ULL, ombr_fs_op(7, 5, v1, 0ULL));
v6=f5(v6, ombr_fs_op(7, 5, v1, 0ULL));
v6=f5(v6, ombr_fs_op(7, 5, v1, 0ULL));
return f5(v6, ombr_fs_op(7, 5, v1, 0ULL));
}
return 0ULL;
}
uint32_t omega_native_c5(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5){
if(!result || (uintptr_t)result%_Alignof(uint64_t) || (uintptr_t)result>UINTPTR_MAX-sizeof(uint64_t))return 32;
if(!ombr_fs_valid(5, p0))return 32;
if(!ombr_fs_valid(5, p1))return 32;
if(!ombr_fs_valid(5, p2))return 32;
if(!ombr_fs_valid(5, p3))return 32;
if(!ombr_fs_valid(5, p4))return 32;
if(!ombr_fs_valid(5, p5))return 32;
*result=f7(p0, p1, p2, p3, p4, p5);
return 0;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
