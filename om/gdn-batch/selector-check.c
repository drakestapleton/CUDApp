/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* External host regression: optional instrumentation must preserve requested timing. */
#define GDN_BATCH_SELECTOR_ONLY
#include "chip-check.c"
static int expect_info;
int __wrap_omega_gpu_gdn_prefill_scoped(OmegaGpuGdn *g,uint32_t l,uint32_t n,
    const float *q,const float *z,const float *ab,float el,float en,
    float is,float os,float *out,OmegaGpuGdnPfInfo *info,uint32_t mode) {
    (void)g;(void)l;(void)n;(void)q;(void)z;(void)ab;(void)el;(void)en;(void)is;(void)os;(void)out;
    assert(mode==1 && !!info==expect_info);
    if(info){memset(info,0,sizeof *info);info->elapsed_ns[0]=10;info->elapsed_ns[1]=20;info->elapsed_ns[2]=30;info->call_ns=70;}
    return 0;
}
int main(void) {
    float x=0;OmegaGpuGdnPfInfo info;
    for(int setting=0;setting<=1;setting++)for(int requested=0;requested<=1;requested++){
        batch_timing=setting;expect_info=setting||requested;
        memset(&info,0x7b,sizeof info);
        assert(batch_prefill(NULL,0,3,&x,&x,&x,1,1,1,1,&x,requested?&info:NULL)==0);
        if(requested)assert(info.elapsed_ns[0]==10&&info.call_ns==70);
    }
    puts("PASS chip selector preserves requested timing in all four combinations");
    return 0;
}
