/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
uint64_t attn_bench_omega(unsigned,size_t,unsigned);
uint64_t attn_bench_oracle(unsigned,size_t,unsigned);
int main(void){
    const char *names[]={"launch-9-addresses","copy-65537-bytes","expand-65537-bf16","descriptor-8-rows","rope-4x128","fold-4x2x8"};
    const unsigned rounds[]={1000,32,32,1000,4,256};
    puts("case\tsample\trounds\tomega_ns\tc_ns");
    for(unsigned op=0;op<6;op++)for(unsigned sample=0;sample<5;sample++){
        uint64_t o,c;
        if(sample&1){c=attn_bench_oracle(op,65537,rounds[op]);o=attn_bench_omega(op,65537,rounds[op]);}
        else{o=attn_bench_omega(op,65537,rounds[op]);c=attn_bench_oracle(op,65537,rounds[op]);}
        printf("%s\t%u\t%u\t%llu\t%llu\n",names[op],sample,rounds[op],(unsigned long long)o,(unsigned long long)c);
    }
    return 0;
}
