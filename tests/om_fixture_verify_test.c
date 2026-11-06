/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_fixture_verify.h"
#include <inttypes.h>
#include <stdio.h>
int main(void) {
    static const uint64_t edge[] = {0,1,2,3,7,8,15,16,17,31,32,33,255,256,262143,262144,262145,67108863,67108864,67108865,2147483647,2147483648,4294967294,4294967295,4294967296,9223372036854775807ull,9223372036854775808ull,UINT64_MAX};
    static const unsigned shift[] = {0,1,15,16,17,23,24,25,31,32,63};
    for (unsigned i=0;i<sizeof edge/sizeof edge[0];++i) {
        uint32_t w[4]={(uint32_t)edge[i],(uint32_t)edge[i],(uint32_t)edge[i],(uint32_t)edge[i]};
        for (unsigned mask=0;mask<16;++mask) for (unsigned word=0;word<4;++word) for(unsigned mode=0;mode<3;++mode) {
            uint32_t x[4]; memcpy(x,w,sizeof x); x[word] += mode;
            printf("words %u %u %u %u %d %d\n",i,mask,word,mode,omega_fv_words(w,x[0],x[1],x[2],x[3],mask),omega_fv_optional(w,x[0],x[1],x[2],x[3]));
            printf("pair %d %d %d\n",omega_fv_pair(w,x,0),omega_fv_pair(w,x,1),omega_fv_bfly(w,x));
        }
        uint32_t pair[4]; memcpy(pair,w,sizeof pair); pair[2]^=0x40000;
        printf("bf16 %d\n",omega_fv_pair(w,pair,0));
        memcpy(pair,w,sizeof pair); pair[1] |= 0x04000000;
        printf("bfly %d\n",omega_fv_bfly(pair,w));
        for(unsigned j=0;j<sizeof edge/sizeof edge[0];++j) {
            printf("error %u %u %d\n",i,j,omega_fv_error(edge[i],edge[j]));
            printf("choose %u %u %u\n",i,j,omega_fv_choose((uint32_t)edge[i],(uint32_t)edge[j],17,33));
            for(unsigned k=0;k<sizeof shift/sizeof shift[0];++k)
                printf("bits %u %u %u %" PRIu64 "\n",i,j,k,omega_fv_ask(5,edge[i],edge[j],shift[k],edge[i],shift[k]));
        }
        printf("status %d %d\n",omega_fv_status((int)(uint32_t)edge[i],0),omega_fv_status((int)(uint32_t)edge[i],1));
        printf("bits-wrapper %u\n",omega_fv_bits((uint32_t)edge[i],(uint32_t)edge[i],16,(uint32_t)edge[i],24));
    }
    puts("PASS codegen fixture source partitions");
    return 0;
}
