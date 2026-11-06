/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "../src/omega_gpu_matmul_api.c"
#ifndef REST
#define REST(x) om_##x
#endif
void REST(oracle)(uint32_t m,uint32_t k,uint32_t n,const uint16_t *a,const uint16_t *b,const float *c,OmegaGpuMatmulInfo *info) { oracle(m,k,n,a,b,c,info); }
