/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_resident.h"
#include "omega_gpu_cmdlist.h"
#include "gpu_resident_fixture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* No device open without exact --chip argument and an operator-supplied window label. */
int main(int argc,char **argv) {
    if(argc!=2||strcmp(argv[1],"--chip")||!getenv("SPARK_WINDOW")||!*getenv("SPARK_WINDOW")) {
        fprintf(stderr,"NOT_RUN: --chip and an authorized SPARK_WINDOW are required\n");return 2;
    }
    static BlackwellIRProgram p;
    OmegaGpuMem code[2]; OmegaGpuMem dm={0},xm={0},sm={0};
    memset(code,0,sizeof code);
    uint32_t gpr[2]={0};
    OmegaGpuCmdList *cl=NULL;
    int rc=1;
    omega_gpu_session_lock();
    if(!omega_gpu_session_open()) goto unlock;
    if(omega_gpu_session_alloc(sizeof resident_model,&dm)||omega_gpu_session_alloc(64*sizeof(float),&xm)||omega_gpu_session_alloc(10*sizeof(uint32_t),&sm)) goto unlock;
    memcpy(dm.cpu,resident_model,sizeof resident_model);
    if(omega_gpu_session_sync_to_gpu(&dm)) goto unlock;
    for(unsigned i=0;i<2;i++) {
        uint64_t err=0; OmegaBlackwellKernel k;
        if(omega_gpu_resident_build(resident_model,i?1:5,64,2,&p,&err)) goto unlock;
        size_t bytes=omega_bw_program_code_bytes(&p);
        if(omega_gpu_session_alloc_code(bytes,&code[i])||omega_blackwell_kernel_encode_into(&p,code[i].cpu,bytes,&k)||omega_gpu_session_sync_to_gpu(&code[i])) goto unlock;
        gpr[i]=omega_bw_gpr_declared(p.regalloc.peak_gpr_usage);
        if(gpr[i]<64) gpr[i]=64;
    }
    omega_gpu_session_unlock();
    if(omega_gpu_cmdlist_create(5,&cl)) goto free_buffers;
    omega_gpu_session_lock();
    for(unsigned witness=0;witness<2;witness++) for(unsigned mode=0;mode<2;mode++) {
        float expected[64];resident_input(expected,witness);memcpy(xm.cpu,expected,sizeof expected);
        resident_reference(resident_model,5,expected);memset(sm.cpu,0,10*sizeof(uint32_t));
        if(omega_gpu_session_sync_to_gpu(&xm)||omega_gpu_session_sync_to_gpu(&sm)) goto unlock;
        omega_gpu_cmdlist_reset(cl);
        unsigned count=mode?5:1;
        for(unsigned l=0;l<count;l++) {
            uint64_t ptrs[]={dm.va+l*8*sizeof(uint32_t),xm.va,sm.va+l*2*sizeof(uint32_t)};
            uint32_t args[6];for(unsigned j=0;j<3;j++) {args[2*j]=(uint32_t)ptrs[j];args[2*j+1]=(uint32_t)(ptrs[j]>>32);}
            OmegaGpuLaunch L={.code_va=code[mode].va,.gpr_count=gpr[mode],.threads_x=32,.threads_y=1,.grid_x=2,.grid_y=1,.num_elements=64,.args=args,.n_args=6,.timeout_ms=20000};
            if(omega_gpu_cmdlist_add(cl,&L)<0) goto unlock;
        }
        OmegaGpuCmdListOpts opts={.mode=OMEGA_GPU_CMDLIST_PCAS,.barrier=OMEGA_GPU_CMDLIST_BARRIER_WFI,.timeout_ms=20000};
        OmegaGpuCmdListStats st={0};
        if(omega_gpu_cmdlist_submit(cl,&opts,&st)||omega_gpu_session_sync_to_cpu(&xm)||omega_gpu_session_sync_to_cpu(&sm)) goto unlock;
        if(st.launches!=count||st.sems_done!=count||memcmp(xm.cpu,expected,sizeof expected)) goto unlock;
        uint32_t *sync=sm.cpu;
        for(unsigned l=0;l<count;l++) if(sync[2*l]!=0||sync[2*l+1]!=(mode?1u:5u)) goto unlock;
        printf("resident chip witness=%u mode=%s launches=%u submits=%u elapsed_ns=%llu exact_f32=PASS\n",witness,mode?"cmdlist-layers":"resident",st.launches,st.submits,(unsigned long long)st.elapsed_ns);
    }
    rc=0;
unlock:
    if(rc) fprintf(stderr,"resident chip FAIL: %s\n",omega_gpu_session_last_error());
    omega_gpu_session_unlock();
    if(cl) omega_gpu_cmdlist_free(cl);
free_buffers:
    omega_gpu_session_lock();
    for(unsigned i=0;i<2;i++) omega_gpu_session_free(&code[i]);
    omega_gpu_session_free(&dm);omega_gpu_session_free(&xm);omega_gpu_session_free(&sm);
    omega_gpu_session_unlock();
    printf("GPU RESIDENT CHIP %s\n",rc?"FAIL":"PASS");return rc;
}
