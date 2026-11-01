/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Reuse the independent GDN reference, tail/state and next-decode checks with
 * only the prefill entry redirected to the scoped production batch. */
#include "omega_gpu_gdn_batch.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
static int batch_timing = 1;
static unsigned batch_calls;
static int batch_prefill(OmegaGpuGdn *g,uint32_t layer,uint32_t n,
    const float *q,const float *z,const float *ab,float el,float en,
    float is,float os,float *out,OmegaGpuGdnPfInfo *info) {
    OmegaGpuGdnPfInfo local;
    OmegaGpuGdnPfInfo *timing = info ? info : (batch_timing ? &local : NULL);
    int rc=omega_gpu_gdn_prefill_scoped(g,layer,n,q,z,ab,el,en,is,os,out,timing,1);
    if(!rc) { batch_calls++; if(timing)for(unsigned i=0;i<3;i++)assert(timing->elapsed_ns[i]>0); }
    return rc;
}
#define omega_gpu_gdn_prefill batch_prefill
#define main original_gdn_test_main
#include "../../tests/gpu_gdn_test.c"
#undef main
#undef omega_gpu_gdn_prefill
#ifndef GDN_BATCH_SELECTOR_ONLY
int main(int argc,char **argv) {
    const char *setting=getenv("GDN_BATCH_TEST_TIMING");
    if(setting && strcmp(setting,"0") && strcmp(setting,"1"))return 2;
    batch_timing=!setting || !strcmp(setting,"1");
    int host=0;for(int i=1;i<argc;i++)if(!strcmp(argv[i],"--host-only") || !strcmp(argv[i],"--sim"))host=1;
    int rc=original_gdn_test_main(argc,argv);
    if(!host && !rc)assert(batch_calls>0);
    printf("scoped GDN batches: %u, timing: %d, status: %s\n",batch_calls,batch_timing,rc?"FAIL":"PASS");
    return rc;
}
#endif
