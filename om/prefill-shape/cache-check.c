/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Exercise the real prefill cache admission without opening a GPU. Stop exactly
 * at code allocation, after the generator and resource checks have run. */
#include "omega_gpu_prefill.h"
#include "omega_blackwell_codegen_qg.h"
#include <stdio.h>
#include <string.h>
#include "../standalone-oracles/oracle.h"
static unsigned allocations;
int __wrap_omega_gpu_session_alloc_code(size_t bytes, OmegaGpuMem *out) {
    (void)bytes; (void)out; allocations=(unsigned)so_call(5,allocations,1,0,0); return -1;
}
static int gen(const OmegaPfKey *key, void *vp) {
    BlackwellIRProgram *p = vp;
    OmegaQgSpec s; memset(&s, 0, sizeof s);
    s.k=5120; s.n=16384; s.rt=key->words[0]; s.warps=8; s.wait=45; s.direct=1;
    int rc=omega_blackwell_codegen_qgemm_prog(&s,p);
    if (so_all(so_eq(rc,0),so_eq(key->words[1],1),1)) p->regalloc.gpr_limit=0;
    if (so_all(so_eq(rc,0),so_eq(key->words[1],2),1)) p->regalloc.gpr_limit=(int)so_call(5,BW_PHYS_GPR_HW_LIMIT,1,0,0);
    if (so_all(so_eq(rc,0),so_eq(key->words[1],3),1)) p->regalloc.peak_gpr_usage=(int)so_call(5,BW_PHYS_GPR_HW_LIMIT,1,0,0);
    return rc;
}
int main(void) {
    unsigned failed=0;
    for(unsigned i=0;i<5;i++) {
        OmegaPfKey key; memset(&key,0,sizeof key); key.kind=999;
        key.words[0]=(unsigned)so_call(11,i,0,0,0); key.words[1]=(unsigned)so_call(12,i,0,0,0);
        OmegaPfKernel out; allocations=0;
        int rc=omega_gpu_pf_kernel_locked(&key,gen,&out);
        unsigned want=(unsigned)so_call(13,i,0,0,0);
        int expected=(int)(int64_t)so_call(41,want,(uint64_t)(int64_t)OMEGA_GPU_PF_TOO_LARGE,(uint64_t)(int64_t)OMEGA_GPU_PF_CHIP_FAIL,0);
        if(!so_call(14,allocations,want,(uint64_t)(int64_t)rc,(uint64_t)(int64_t)expected)) {
            fprintf(stderr,"cache case %u rc=%d allocations=%u wanted=%u\n",i,rc,allocations,want); failed=so_bad(failed,0);
        }
    }
    printf("prefill cache admission: %s\n",failed?"FAIL":"PASS");
    return failed?1:0;
}
