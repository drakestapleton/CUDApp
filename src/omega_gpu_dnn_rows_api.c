/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Memory/session shell only; admission, byte layout and computation are Omega. */
#include "omega_gpu_dnn_rows_api.h"
#include "omega_gpu_om_ir.h"
#include "omega_gpu_session.h"
#include "cudapp_native.h"
#include <stdlib.h>
#include <string.h>
static int plan(const OmegaGpuDnnRowsShape *s,uint64_t words[8],uint64_t p[16]) {
    uint32_t bits;uint64_t result=0;memcpy(&bits,&s->epsilon,4);
    memset(words,0,8*sizeof *words);
    words[0]=s->rows;words[1]=s->length;words[2]=s->dtype;words[3]=s->op;words[4]=bits;
    if(omega_dr_plan(&result,words,8,p,16)) return OMEGA_GPU_DNN_ROWS_CODEGEN;
    return result?OMEGA_GPU_DNN_ROWS_REFUSED:OMEGA_GPU_DNN_ROWS_OK;
}
int omega_gpu_dnn_rows_codegen(const OmegaGpuDnnRowsShape *s,BlackwellIRProgram *program,uint8_t *code,size_t capacity,OmegaBlackwellKernel *kernel) {
    uint64_t words[8],p[16];
    if(!s||!program||!code||!kernel)return OMEGA_GPU_DNN_ROWS_BAD_ARGS;
    memset(kernel,0,sizeof *kernel);int rc=plan(s,words,p);if(rc)return rc;
    rc=omega_gpu_om_ir_replay(omega_dnn_rows,words[3],words[1],words[2],0,program,NULL);
    if(rc||omega_blackwell_kernel_encode_into(program,code,capacity,kernel))return OMEGA_GPU_DNN_ROWS_CODEGEN;
    return OMEGA_GPU_DNN_ROWS_OK;
}
static void putptr(uint32_t *a,uint64_t p){a[0]=(uint32_t)p;a[1]=(uint32_t)(p>>32);}
int omega_gpu_dnn_rows(const OmegaGpuDnnRowsShape *s,const void *input,float *output) {
    uint64_t words[8],p[16];if(!s||!input||!output)return OMEGA_GPU_DNN_ROWS_BAD_ARGS;
    int rc=plan(s,words,p);if(rc)return rc; /* Before all device/session access. */
    BlackwellIRProgram *program=malloc(sizeof *program);uint8_t *code=malloc(65536);OmegaBlackwellKernel kernel={0};
    if(!program||!code){free(program);free(code);return OMEGA_GPU_DNN_ROWS_CODEGEN;}
    rc=omega_gpu_dnn_rows_codegen(s,program,code,65536,&kernel);if(rc){free(program);free(code);return rc;}
    OmegaGpuMem b[3]={0};omega_gpu_session_lock();
    if(omega_gpu_session_open()||omega_gpu_session_alloc(p[0],&b[0])||omega_gpu_session_alloc(p[1],&b[1])||omega_gpu_session_alloc_code(kernel.code_size,&b[2])){rc=OMEGA_GPU_DNN_ROWS_CHIP;goto cleanup;}
    memcpy(b[0].cpu,input,p[0]);memset(b[1].cpu,0,p[1]);memcpy(b[2].cpu,code,kernel.code_size);
    for(unsigned i=0;i<3;++i)if(omega_gpu_session_sync_to_gpu(&b[i])){rc=OMEGA_GPU_DNN_ROWS_CHIP;goto cleanup;}
    uint32_t args[5];putptr(args,b[0].va);putptr(args+2,b[1].va);args[4]=(uint32_t)words[4];
    OmegaGpuLaunch launch={.code_va=b[2].va,.gpr_count=omega_bw_gpr_declared(program->regalloc.peak_gpr_usage),
        .threads_x=(uint32_t)p[3],.threads_y=1,.grid_x=(uint32_t)p[2],.grid_y=1,.num_elements=p[2],.args=args,.n_args=5,.timeout_ms=30000};
    if(omega_gpu_session_launch(&launch,NULL,NULL)||omega_gpu_session_sync_to_cpu(&b[1])){rc=OMEGA_GPU_DNN_ROWS_CHIP;goto cleanup;}
    memcpy(output,b[1].cpu,p[1]);
cleanup:
    for(unsigned i=0;i<3;++i)omega_gpu_session_free(&b[i]);
    omega_gpu_session_unlock();free(program);free(code);return rc;
}
