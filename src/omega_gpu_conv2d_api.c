/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Memory/session shell. Geometry, grouping, addressing, arithmetic and launch
 * decisions are checked seeded Omega. No host convolution or im2col. */
#include "omega_gpu_conv2d_api.h"
#include "omega_gpu_om_ir.h"
#include "omega_gpu_session.h"
#include "cudapp_native.h"
#include <stdlib.h>
#include <string.h>
static void words(const OmegaGpuConv2dShape *s, uint64_t w[18]) {
    const uint32_t v[18]={s->batch,s->in_channels,s->height,s->width,s->out_channels,
        s->kernel_h,s->kernel_w,s->stride_h,s->stride_w,s->pad_top,s->pad_bottom,
        s->pad_left,s->pad_right,s->dilation_h,s->dilation_w,s->groups,s->layout,s->dtype};
    for(unsigned j=0;j<18;j++) w[j]=v[j];
}
int omega_gpu_conv2d_plan(const OmegaGpuConv2dShape *s,uint64_t ix,uint64_t p[32]) {
    uint64_t w[18],r;
    if(!s||!p) return OMEGA_GPU_CONV2D_BAD_ARGS;
    words(s,w);
    if(omega_conv_plan(&r,w,18,ix,p,32)) return OMEGA_GPU_CONV2D_CODEGEN;
    return r?OMEGA_GPU_CONV2D_REFUSED:0;
}
/* Replay's synchronous callback only needs a view of the shape words. TLS keeps
 * simultaneous callers independent; restoring the previous view permits nesting. */
static _Thread_local const uint64_t *active_shape;
static uint32_t unit(uint64_t *result,uint64_t k,uint64_t a,uint64_t b,uint64_t c,
                     void *ins,uint64_t ni,void *meta,uint64_t nm) {
    (void)k;(void)a;(void)b;(void)c;
    return omega_conv_ir(result,active_shape,18,ins,ni,meta,nm);
}
int omega_gpu_conv2d_codegen(const OmegaGpuConv2dShape *s,BlackwellIRProgram *program,
                            uint8_t *code,size_t capacity,OmegaBlackwellKernel *kernel) {
    uint64_t p[32],w[18];
    if(!s||!program||!code||!kernel) return OMEGA_GPU_CONV2D_BAD_ARGS;
    memset(kernel,0,sizeof *kernel);
    int rc=omega_gpu_conv2d_plan(s,0,p); if(rc) return rc;
    words(s,w);
    const uint64_t *prior=active_shape; active_shape=w;
    rc=omega_gpu_om_ir_replay(unit,0,0,0,0,program,NULL);
    active_shape=prior;
    if(rc||omega_blackwell_kernel_encode_into(program,code,capacity,kernel)) return OMEGA_GPU_CONV2D_CODEGEN;
    return 0;
}
static void ptr(uint32_t *a,uint64_t p) {a[0]=(uint32_t)p;a[1]=(uint32_t)(p>>32);}
int omega_gpu_conv2d(const OmegaGpuConv2dShape *s,const void *image,const void *filter,float *out) {
    uint64_t p[32];
    if(!s||!image||!filter||!out) return OMEGA_GPU_CONV2D_BAD_ARGS;
    int rc=omega_gpu_conv2d_plan(s,0,p); if(rc) return rc;
    BlackwellIRProgram *program=malloc(sizeof *program); uint8_t *code=malloc(65536);
    OmegaBlackwellKernel kernel={0}; OmegaGpuMem mem[4]; memset(mem,0,sizeof mem);
    if(!program||!code) {free(program);free(code);return OMEGA_GPU_CONV2D_CODEGEN;}
    rc=omega_gpu_conv2d_codegen(s,program,code,65536,&kernel);
    if(rc) {free(program);free(code);return rc;}
    omega_gpu_session_lock();
    if(!omega_gpu_session_open()||omega_gpu_session_alloc(p[2],mem)||omega_gpu_session_alloc(p[3],mem+1)||
       omega_gpu_session_alloc(p[4],mem+2)||omega_gpu_session_alloc_code(kernel.code_size,mem+3)) {
        rc=OMEGA_GPU_CONV2D_CHIP;goto done;
    }
    memcpy(mem[0].cpu,image,p[2]);memcpy(mem[1].cpu,filter,p[3]);memset(mem[2].cpu,0,p[4]);
    memcpy(mem[3].cpu,code,kernel.code_size);
    for(unsigned j=0;j<4;j++) if(omega_gpu_session_sync_to_gpu(mem+j)) {rc=OMEGA_GPU_CONV2D_CHIP;goto done;}
    for(uint64_t ix=0;ix<p[5];ix++) {
        rc=omega_gpu_conv2d_plan(s,ix,p);if(rc) goto done;
        uint32_t args[7];ptr(args,mem[0].va+p[8]);ptr(args+2,mem[1].va+p[9]);ptr(args+4,mem[2].va+p[10]);args[6]=(uint32_t)p[11];
        OmegaGpuLaunch launch={.code_va=mem[3].va,.gpr_count=omega_bw_gpr_declared(program->regalloc.peak_gpr_usage),
            .threads_x=(uint32_t)p[7],.threads_y=1,.grid_x=(uint32_t)p[6],.grid_y=1,.num_elements=p[1],
            .args=args,.n_args=7,.timeout_ms=30000};
        if(omega_gpu_session_launch(&launch,NULL,NULL)) {rc=OMEGA_GPU_CONV2D_CHIP;goto done;}
    }
    if(omega_gpu_session_sync_to_cpu(mem+2)) {rc=OMEGA_GPU_CONV2D_CHIP;goto done;}
    memcpy(out,mem[2].cpu,p[4]);
done:
    for(unsigned j=0;j<4;j++) omega_gpu_session_free(mem+j);
    omega_gpu_session_unlock();free(program);free(code);return rc;
}
