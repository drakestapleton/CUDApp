/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Exercise the real prefill cache admission without opening a GPU. Stop exactly
 * at code allocation, after the generator and resource checks have run. */
#include "omega_gpu_prefill.h"
#include "omega_blackwell_codegen_qg.h"
#include <stdio.h>
#include <string.h>
static unsigned allocations;
int __wrap_omega_gpu_session_alloc_code(size_t bytes, OmegaGpuMem *out) {
    (void)bytes; (void)out; allocations++; return -1;
}
static int gen(const OmegaPfKey *key, void *vp) {
    BlackwellIRProgram *p = vp;
    OmegaQgSpec s; memset(&s, 0, sizeof s);
    s.k=5120; s.n=16384; s.rt=key->words[0]; s.warps=8; s.wait=45; s.direct=1;
    int rc=omega_blackwell_codegen_qgemm_prog(&s,p);
    if (!rc && key->words[1]==1) p->regalloc.gpr_limit=0;
    if (!rc && key->words[1]==2) p->regalloc.gpr_limit=BW_PHYS_GPR_HW_LIMIT+1;
    if (!rc && key->words[1]==3) p->regalloc.peak_gpr_usage=BW_PHYS_GPR_HW_LIMIT+1;
    return rc;
}
int main(void) {
    unsigned failed=0;
    for(unsigned i=0;i<5;i++) {
        OmegaPfKey key; memset(&key,0,sizeof key); key.kind=999;
        key.words[0]=i==0?4:8; key.words[1]=i>1?i-1:0;
        OmegaPfKernel out; allocations=0;
        int rc=omega_gpu_pf_kernel_locked(&key,gen,&out);
        unsigned want=i<2;
        if(allocations!=want || rc!=(want?OMEGA_GPU_PF_CHIP_FAIL:OMEGA_GPU_PF_TOO_LARGE)) {
            fprintf(stderr,"cache case %u rc=%d allocations=%u wanted=%u\n",i,rc,allocations,want); failed++;
        }
    }
    printf("prefill cache admission: %s\n",failed?"FAIL":"PASS");
    return failed?1:0;
}
