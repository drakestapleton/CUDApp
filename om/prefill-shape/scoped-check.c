/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Real call-path isolation test. The device-open and kernel-cache boundaries are
 * intercepted; neither a GPU nor fake matrix arithmetic is used. */
#include "omega_gpu_prefill.h"
#include "omega_blackwell_codegen_qg.h"
#include <stdio.h>
#include <string.h>
#include "../standalone-oracles/oracle.h"
static unsigned opened, keyed, failed;
static OmegaQgSpec want;
int __wrap_omega_gpu_pf_open_locked(void) { opened=(unsigned)so_call(5,opened,1,0,0); return OMEGA_GPU_PF_OK; }
int __wrap_omega_gpu_pf_kernel_locked(const OmegaPfKey *key, OmegaPfGen gen, OmegaPfKernel *out) {
    OmegaQgSpec got; (void)gen; (void)out;
    memcpy(&got,key->words,sizeof got); keyed=(unsigned)so_call(5,keyed,1,0,0);
    if (!so_eq((uint64_t)(int64_t)memcmp(&got,&want,sizeof got),0)) { fprintf(stderr,"wrong per-call kernel key\n"); failed=so_bad(failed,0); }
    return OMEGA_GPU_PF_TOO_LARGE; /* stop before allocation/launch */
}
static void expect(unsigned rt,unsigned warps,unsigned group,unsigned mutant) {
    memset(&want,0,sizeof want);want.rt=rt;want.warps=warps;want.group=group;want.mutant=mutant;
    want.k=128;want.n=256;want.wait=45;want.direct=1;
    opened=keyed=0;
}
static void check(int ok,const char *why) { failed=so_bad(failed,ok); if(!ok) fprintf(stderr,"%s\n",why); }
static int scoped(unsigned rt,unsigned group) {
    return omega_gpu_pf_qgemm_scoped(rt,8,45,group,256,128,OMEGA_GPU_PF_PLAIN,16,1,0,32,48,64,NULL);
}
int main(void) {
    omega_gpu_pf_qgemm_set_shape(2,4,45);omega_gpu_pf_qgemm_set_group(1);omega_gpu_pf_qgemm_test_set_mutant(1);
    check(so_eq(omega_gpu_pf_qgemm_scoped_bm(8,8,45,4),128),"checked128 tile height");
    check(so_eq(omega_gpu_pf_qgemm_scoped_bm(4,8,45,0),64),"checked64 tile height");
    check(so_eq(omega_gpu_pf_qgemm_scoped_bm(0,8,45,4),0),"zero shape refused");
    check(so_eq(omega_gpu_pf_qgemm_scoped_bm(8,8,121,4),0),"unsafe wait refused");
    check(so_eq(omega_gpu_pf_qgemm_scoped_bm(8,8,45,6),0),"group outside bound refused");
    for(unsigned i=0;i<3;i++) {
        unsigned rt=(unsigned)so_call(41,so_eq(i,1),8,4,0),group=(unsigned)so_call(41,so_eq(i,1),4,0,0);
        expect(rt,8,group,0);
        check(so_all(so_eq((uint64_t)(int64_t)scoped(rt,group),(uint64_t)(int64_t)OMEGA_GPU_PF_TOO_LARGE),so_eq(opened,1),so_eq(keyed,1)),"scoped call reached its own kernel key");
        expect(2,4,1,1);
        int rc=omega_gpu_pf_qgemm(256,128,OMEGA_GPU_PF_PLAIN,16,1,0,32,48,64,NULL);
        check(so_all(so_eq((uint64_t)(int64_t)rc,(uint64_t)(int64_t)OMEGA_GPU_PF_TOO_LARGE),so_eq(opened,1),so_eq(keyed,1)),"legacy settings unchanged after scoped call");
        check(so_eq(omega_gpu_pf_qgemm_bm(),32),"legacy tile height unchanged");
    }
    const uint32_t rows[] = {1,30,64,65,128,4096,UINT32_MAX};
    for(unsigned i=0;i<sizeof(rows)/sizeof(rows[0]);i++) {
        uint32_t p[4];for(unsigned j=0;j<4;j++)p[j]=omega_gpu_pf_qgemm_reading_option(rows[i],j);
        expect((unsigned)so_call(15,rows[i],0,0,0),8,(unsigned)so_call(16,rows[i],0,0,0),0);
        int rc=omega_gpu_pf_qgemm_scoped(p[0],p[1],p[2],p[3],256,128,OMEGA_GPU_PF_PLAIN,16,1,0,32,48,64,NULL);
        check(so_all(so_eq((uint64_t)(int64_t)rc,(uint64_t)(int64_t)OMEGA_GPU_PF_TOO_LARGE),so_eq(opened,1),so_eq(keyed,1)),"adaptive profile reaches matching cache key");
        check(so_eq(omega_gpu_pf_qgemm_bm(),32),"adaptive query leaves legacy height unchanged");
    }
    check(so_eq(omega_gpu_pf_qgemm_reading_option(0,0),UINT32_MAX),"zero reading rows refused");
    check(so_eq(omega_gpu_pf_qgemm_reading_option(64,4),UINT32_MAX),"unknown reading option refused");
    expect(8,8,6,0);
    check(so_all(so_eq((uint64_t)(int64_t)scoped(8,6),(uint64_t)(int64_t)OMEGA_GPU_PF_BAD_ARGS),so_eq(opened,0),so_eq(keyed,0)),"invalid options refused before device access");
    omega_gpu_pf_qgemm_set_shape(0,0,0);omega_gpu_pf_qgemm_set_group(0);omega_gpu_pf_qgemm_test_set_mutant(0);
    printf("scoped qgemm isolation: %s\n",failed?"FAIL":"PASS");return failed?1:0;
}
