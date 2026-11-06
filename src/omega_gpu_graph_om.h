/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_GRAPH_OM_H
#define OMEGA_GPU_GRAPH_OM_H
#include "cudapp_native.h"
#include <stdint.h>
#include <string.h>
/* Projection only: floats arrive as raw bits, and Omega emits the full node.
 * Tensor/session objects never cross the seeded numeric ABI. */
static inline uint64_t omega_graph_fbits(float f) { uint32_t u; memcpy(&u,&f,4); return u; }
static inline int omega_graph_next(const uint64_t cfg[16], const uint64_t va[64], uint64_t node[64], uint64_t phase) {
    uint64_t result=0;
    return omega_mg_node(&result,cfg,16,va,64,node,64,phase)==OMEGA_OK && result==1;
}
static inline void omega_graph_args(const uint64_t node[64], uint32_t words[32]) {
    for (unsigned i=0;i<node[5];i++) words[i]=(uint32_t)node[32+i];
}
static inline uint64_t omega_graph_addr(uint64_t op,uint64_t base,uint64_t dev,uint64_t split,uint64_t off,uint64_t index,uint64_t stride) {
#ifdef CUDAPP_MIGRATION_PENDING_C_GRAPHS
    if(op==0)return(base+255u)&~UINT64_C(255);
    if(op==1)return off<split?base+off:dev+(off-split);
    return base+index*stride;
#else
    const uint64_t q[8]={op,base,dev,split,off,index,stride,0};uint64_t result=0;
    if(omega_mg_addr(&result,q,8)!=OMEGA_OK)__builtin_trap();
    return result;
#endif
}
#endif
