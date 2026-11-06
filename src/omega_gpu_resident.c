/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Internal memory shell: widen descriptor words; all admission and IR decisions are Omega. */
#include "omega_gpu_resident.h"
#include "cudapp_native.h"
int omega_gpu_resident_build(const uint32_t *descs, uint64_t n, uint64_t size, uint64_t ctas,
                             BlackwellIRProgram *p, uint64_t *err) {
    uint64_t d[512]={0}, result=1;
    if(!descs || !p) return OMEGA_OM_IR_REFUSED;
    for(uint64_t i=0;i<512 && i/8<n;i++) d[i]=descs[i];
    if(omega_resident_check(&result,d,512,n,size,ctas)!=OMEGA_OK || result) return OMEGA_OM_IR_REFUSED;
    return omega_gpu_om_ir_replay(omega_resident,0,n,size,ctas,p,err);
}
