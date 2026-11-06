/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Memory/session shell. Admission, launch geometry and all arithmetic are .om. */
#include "omega_gpu_dnn_activations_api.h"
#include "omega_gpu_om_ir.h"
#include "omega_gpu_session.h"
#include "cudapp_native.h"
#include <stdlib.h>
#include <string.h>
static int plan(const OmegaGpuDnnActivationsShape *s,uint64_t p[8]) {
    uint64_t result=0;
    if(omega_da_plan(&result,s->elements,s->operation,s->dtype,p,8)) return OMEGA_GPU_DNN_ACTIVATIONS_CODEGEN;
    return result ? OMEGA_GPU_DNN_ACTIVATIONS_REFUSED : OMEGA_GPU_DNN_ACTIVATIONS_OK;
}
int omega_gpu_dnn_activations_codegen(const OmegaGpuDnnActivationsShape *s,BlackwellIRProgram *program,uint8_t *code,size_t capacity,OmegaBlackwellKernel *kernel) {
    uint64_t p[8];
    if(!s||!program||!code||!kernel) return OMEGA_GPU_DNN_ACTIVATIONS_BAD_ARGS;
    memset(kernel,0,sizeof *kernel);
    int rc=plan(s,p);if(rc) return rc;
    if(omega_gpu_om_ir_replay(omega_dnn_activations,s->operation,s->dtype,0,0,program,NULL) ||
       omega_blackwell_kernel_encode_into(program,code,capacity,kernel)) return OMEGA_GPU_DNN_ACTIVATIONS_CODEGEN;
    return OMEGA_GPU_DNN_ACTIVATIONS_OK;
}
static void putptr(uint32_t *p,uint64_t v) {p[0]=(uint32_t)v;p[1]=(uint32_t)(v>>32);}
int omega_gpu_dnn_activations(const OmegaGpuDnnActivationsShape *s,const void *input,float *output) {
    uint64_t p[8];
    if(!s||!input||!output) return OMEGA_GPU_DNN_ACTIVATIONS_BAD_ARGS;
    int rc=plan(s,p);if(rc) return rc; /* Refusal must precede session/device access. */
    BlackwellIRProgram *program=malloc(sizeof *program);uint8_t *code=malloc(65536);
    OmegaBlackwellKernel kernel={0};
    if(!program||!code) {free(program);free(code);return OMEGA_GPU_DNN_ACTIVATIONS_CODEGEN;}
    rc=omega_gpu_dnn_activations_codegen(s,program,code,65536,&kernel);
    if(rc) {free(program);free(code);return rc;}
    OmegaGpuMem buffers[3]={0};
    omega_gpu_session_lock();
    if(!omega_gpu_session_open() || omega_gpu_session_alloc(p[0],&buffers[0]) ||
       omega_gpu_session_alloc(p[1],&buffers[1]) || omega_gpu_session_alloc_code(kernel.code_size,&buffers[2])) {
        rc=OMEGA_GPU_DNN_ACTIVATIONS_CHIP;goto cleanup;
    }
    memcpy(buffers[0].cpu,input,p[0]);memset(buffers[1].cpu,0,p[1]);memcpy(buffers[2].cpu,code,kernel.code_size);
    for(unsigned i=0;i<3;++i) if(omega_gpu_session_sync_to_gpu(&buffers[i])) {rc=OMEGA_GPU_DNN_ACTIVATIONS_CHIP;goto cleanup;}
    uint32_t args[5];putptr(args,buffers[0].va);putptr(args+2,buffers[1].va);args[4]=(uint32_t)s->elements;
    OmegaGpuLaunch launch={.code_va=buffers[2].va,.gpr_count=omega_bw_gpr_declared(program->regalloc.peak_gpr_usage),
        .threads_x=(uint32_t)p[3],.threads_y=1,.grid_x=(uint32_t)p[2],.grid_y=1,.num_elements=s->elements,
        .args=args,.n_args=5,.timeout_ms=30000};
    if(omega_gpu_session_launch(&launch,NULL,NULL) || omega_gpu_session_sync_to_cpu(&buffers[1])) {
        rc=OMEGA_GPU_DNN_ACTIVATIONS_CHIP;goto cleanup;
    }
    memcpy(output,buffers[1].cpu,p[1]);
cleanup:
    for(unsigned i=0;i<3;++i) omega_gpu_session_free(&buffers[i]);
    omega_gpu_session_unlock();free(program);free(code);return rc;
}
