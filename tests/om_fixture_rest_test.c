/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_fixture_rest.h"
#include <stdio.h>
#include <inttypes.h>
static const uint64_t edges[] = {
    0,1,2,3,4,5,6,7,8,9,10,11,15,16,17,31,32,33,127,128,129,254,255,256,
    1999,2000,2001,999999,1000000,1000001,999999999,1000000000,1000000001,
    18446744072ull,18446744073ull,18446744074ull,709551614,709551615,709551616,
    18446744073709550ull,18446744073709551ull,18446744073709552ull,
    UINT32_MAX-1,UINT32_MAX,UINT32_MAX+1ull,INT64_MAX,1ull<<63,UINT64_MAX-1,UINT64_MAX
};
int main(void) {
    for (uint32_t op = FR_NS; op <= FR_RESULT; ++op)
        for (size_t i=0;i<sizeof edges/sizeof edges[0];++i)
            for (size_t j=0;j<sizeof edges/sizeof edges[0];++j)
                printf("c %u %zu %zu %" PRIu64 "\n",op,i,j,omega_fixture_calc(op,edges[i],edges[j],edges[j]));
    for (size_t i=0;i<sizeof edges/sizeof edges[0];++i)
        for (uint64_t j=0;j<12;++j)
            printf("f %zu %" PRIu64 " %u\n",i,j,omega_fixture_flush(j,edges[i]));
    for (size_t i=0;i<sizeof edges/sizeof edges[0];++i)
        for(uint32_t k=0;k<2;++k) printf("s %zu %u %a\n",i,k,omega_fixture_scale(edges[i],k));
    static const unsigned char pats[][32] = {"libcuda.so","libcudart.so","NV_ERR_NO_MEMORY","attempts","2 MiB blocks"};
    static const uint64_t lens[] = {10,12,16,8,12};
    for (size_t p=0;p<5;++p) {
        for(size_t pos=0;pos<512;++pos) {
            unsigned char buf[512]; memset(buf,'x',sizeof buf);
            size_t n=(size_t)lens[p];
            if(n<=512-pos) memcpy(buf+pos,pats[p],n);
            printf("t %zu %zu %d\n",p,pos,omega_fixture_text(buf,pats[p],lens[p]));
            buf[0]=0;
            printf("z %zu %zu %d\n",p,pos,omega_fixture_text(buf,pats[p],lens[p]));
        }
    }
    unsigned char empty[512]={0}, pat[32]={0};
    printf("bad %d %d\n",omega_fixture_text(empty,pat,0),omega_fixture_text(empty,pat,33));
    return 0;
}
