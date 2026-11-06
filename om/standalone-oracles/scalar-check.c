/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "oracle.h"
#include <stdio.h>
uint64_t standalone_c_oracle(unsigned,uint64_t,uint64_t,uint64_t,uint64_t);
static int same(unsigned op,uint64_t a,uint64_t b) {
    if(a==b)return 1;
    if(op==20 || op==38) return (a&0x7fffffffffffffffULL)>0x7ff0000000000000ULL && (b&0x7fffffffffffffffULL)>0x7ff0000000000000ULL;
    if(op==18||op==19||op==23||op==25||op==26||op==27||op==30||op==31||op==32||op==43)
        return (a&0x7fffffffULL)>0x7f800000ULL && (b&0x7fffffffULL)>0x7f800000ULL;
    return 0;
}
int main(void) {
    /* Source-derived branch/modulo/tiling limits, each at v-1/v/v+1, plus
     * integer endpoints and f32/f64 zero/subnormal/finite/infinity/NaN classes. */
    static const uint64_t cases[]={0,1,2,3,4,5,6,7,8,15,16,17,30,31,32,33,63,64,65,66,67,68,127,128,129,142,143,144,172,173,174,255,256,257,1023,1024,1025,4095,4096,4097,4098,4099,4100,5118,5119,5120,5121,8191,8192,8193,UINT32_MAX-1,UINT32_MAX,UINT64_MAX,
        0x80000000,0x80000001,0x007fffff,0x00800000,0x3f800000,0xbf800000,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00000,0x7f800001,0xffc00001,
        0x38d1b716,0x38d1b717,0x38d1b718,0x3f1a36e2eb1c432cULL,0x3f1a36e2eb1c432dULL,0x3f1a36e2eb1c432eULL,
        0x8000000000000000ULL,0x8000000000000001ULL,0x000fffffffffffffULL,0x0010000000000000ULL,0x3ff0000000000000ULL,0xbff0000000000000ULL,0x7fefffffffffffffULL,0xffefffffffffffffULL,0x7ff0000000000000ULL,0xfff0000000000000ULL,0x7ff8000000000000ULL,0x7ff0000000000001ULL};
    unsigned long count=0;
    for(unsigned op=0;op<=44;op++)for(size_t i=0;i<sizeof cases/sizeof *cases;i++)for(size_t j=0;j<sizeof cases/sizeof *cases;j++)for(unsigned k=0;k<4;k++) {
        uint64_t c=k&1,d=k>>1;
        uint64_t got=so_call(op,cases[i],cases[j],c,d),want=standalone_c_oracle(op,cases[i],cases[j],c,d);
        if(!same(op,got,want)) {fprintf(stderr,"scalar mismatch op=%u a=%llx b=%llx c=%llu d=%llu got=%llx want=%llx\n",op,(unsigned long long)cases[i],(unsigned long long)cases[j],(unsigned long long)c,(unsigned long long)d,(unsigned long long)got,(unsigned long long)want);return 1;}
        count++;
    }
    printf("PASS standalone scalar C/Omega differential: %lu cases; source boundaries and float classes\n",count);
    return 0;
}
