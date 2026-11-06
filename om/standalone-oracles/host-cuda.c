/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Verification-only CPU transport, never a replacement libcuda or runtime.
 * Its independent C arithmetic is intentional: it supplies corruptible results
 * to the actual fixture checkers without loading any driver or opening devices. */
#include <cuda.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "omega_blackwell_matmul.h"
static int vadd;
CUresult CUDAAPI cuInit(unsigned x) {(void)x;return CUDA_SUCCESS;}
CUresult CUDAAPI cuDeviceGet(CUdevice *d,int x) {*d=x;return CUDA_SUCCESS;}
CUresult CUDAAPI cuCtxCreate(CUcontext *p,CUctxCreateParams *params,unsigned flags,CUdevice d) {(void)params;(void)flags;(void)d;*p=(CUcontext)(uintptr_t)1;return CUDA_SUCCESS;}
CUresult CUDAAPI cuCtxDestroy(CUcontext p) {(void)p;return CUDA_SUCCESS;}
CUresult CUDAAPI cuCtxSynchronize(void) {return CUDA_SUCCESS;}
CUresult CUDAAPI cuModuleLoadData(CUmodule *p,const void *b) {(void)b;*p=(CUmodule)(uintptr_t)1;return CUDA_SUCCESS;}
CUresult CUDAAPI cuModuleLoadDataEx(CUmodule *p,const void *b,unsigned n,CUjit_option *o,void **v) {(void)n;(void)o;(void)v;return cuModuleLoadData(p,b);}
CUresult CUDAAPI cuModuleGetFunction(CUfunction *p,CUmodule m,const char *s) {(void)m;vadd=!strcmp(s,"vadd_u32");*p=(CUfunction)(uintptr_t)1;return CUDA_SUCCESS;}
CUresult CUDAAPI cuModuleUnload(CUmodule p) {(void)p;return CUDA_SUCCESS;}
CUresult CUDAAPI cuMemAlloc(CUdeviceptr *p,size_t n) {*p=(CUdeviceptr)(uintptr_t)malloc(n);return *p?CUDA_SUCCESS:CUDA_ERROR_OUT_OF_MEMORY;}
CUresult CUDAAPI cuMemFree(CUdeviceptr p) {free((void *)(uintptr_t)p);return CUDA_SUCCESS;}
CUresult CUDAAPI cuMemcpyHtoD(CUdeviceptr p,const void *b,size_t n) {memcpy((void *)(uintptr_t)p,b,n);return CUDA_SUCCESS;}
CUresult CUDAAPI cuMemcpyDtoH(void *b,CUdeviceptr p,size_t n) {memcpy(b,(void *)(uintptr_t)p,n);return CUDA_SUCCESS;}
CUresult CUDAAPI cuMemsetD32(CUdeviceptr p,unsigned value,size_t n) {uint32_t *b=(void *)(uintptr_t)p;for(size_t i=0;i<n;i++)b[i]=value;return CUDA_SUCCESS;}
CUresult CUDAAPI cuGetErrorString(CUresult r,const char **s) {(void)r;*s="host CPU transport";return CUDA_SUCCESS;}
CUresult CUDAAPI cuLaunchKernel(CUfunction f,unsigned gx,unsigned gy,unsigned gz,unsigned tx,unsigned ty,unsigned tz,unsigned shared,CUstream stream,void **a,void **extra) {
    (void)f;(void)gx;(void)gy;(void)gz;(void)tx;(void)ty;(void)tz;(void)shared;(void)stream;(void)extra;
    const void *left=(void *)(uintptr_t)*(CUdeviceptr *)a[0],*right=(void *)(uintptr_t)*(CUdeviceptr *)a[1];
    void *out=(void *)(uintptr_t)*(CUdeviceptr *)a[2];
    if(vadd) {
        const uint32_t *x=left,*y=right;uint32_t *z=out,n=*(uint32_t *)a[3];
        for(uint32_t i=0;i<n;i++)z[i]=x[i]+y[i];
        if(getenv("SO_HOST_RED"))z[0]^=1;
    } else {
        unsigned m=(unsigned)strtoul(getenv("SO_HOST_M"),NULL,0),k=(unsigned)strtoul(getenv("SO_HOST_K"),NULL,0),n=(unsigned)strtoul(getenv("SO_HOST_N"),NULL,0);
        omega_matmul_cpu_oracle_bf16(left,right,out,m,k,n);
        if(getenv("SO_HOST_RED"))((float *)out)[0]+=1.0f;
    }
    return CUDA_SUCCESS;
}
