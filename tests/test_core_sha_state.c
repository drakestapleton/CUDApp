/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "sha256.h"
#include <stdio.h>
#include <string.h>
void ref_sha256_init(sha256_ctx *);
void ref_sha256_update(sha256_ctx *,const uint8_t *,size_t);
void ref_sha256_final(sha256_ctx *,uint8_t *);
int main(void) {
    const size_t sizes[]={0,1,3,55,56,63,64,65,119,120,127,128,4095,4096,4097,32768};
    const size_t chunks[]={1,63,64,257,4096};
    uint8_t input[32768],a[32],b[32];memset(input,0x5a,sizeof input);
    for(size_t t=0;t<sizeof sizes/sizeof sizes[0];t++)for(size_t k=0;k<sizeof chunks/sizeof chunks[0];k++) {
        sha256_ctx x,y;memset(&x,0xa5,sizeof x);y=x;
        sha256_init(&x);ref_sha256_init(&y);
        for(size_t i=0;i<sizes[t];){size_t n=sizes[t]-i;if(n>chunks[k])n=chunks[k];sha256_update(&x,input+i,n);ref_sha256_update(&y,input+i,n);i+=n;}
        for(unsigned final=0;final<2;final++) {
            sha256_final(&x,a);ref_sha256_final(&y,b);
            if(memcmp(a,b,sizeof a)||memcmp(&x,&y,sizeof x)) {
                fprintf(stderr,"SHA context mismatch: size=%zu chunk=%zu final=%u count=%llu reference=%llu\n",sizes[t],chunks[k],final,(unsigned long long)x.count,(unsigned long long)y.count);
                return 1;
            }
        }
    }
    puts("CORE-UTIL SHA context PASS: padding boundaries, chunk splits and repeated finalization");return 0;
}
