/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "model_graphs_nested_objects.h"
/* Compile once per family. Production entry points and kernel caches are linked
 * directly from these TUs, not reimplemented. Only backing resource observations
 * are constructed here: record mode neither reads weights nor executes state. */
#if defined(MG_NEST_FP8)
#include "../src/omega_gpu_fp8_api.c"
void *mg_nested_fp8(unsigned slot,uint32_t experts,uint32_t n,uint32_t k) {
    static OmegaGpuFp8Tensor tensors[16]; static uint8_t backing[256];
    if(slot>=16 || !dev_open_locked()) return NULL;
    OmegaGpuFp8Tensor *t=&tensors[slot];
    t->experts=experts;t->n=n;t->k=k;t->allocation_epoch=omega_gpu_session_epoch();t->owner_generation=slot+1;t->s_mul=0x1p-56;
    t->w.cpu=backing;t->w.va=UINT64_C(0xb00000000)+(uint64_t)slot*UINT64_C(0x100000000);
    t->w.size=(size_t)experts*n*k;t->s.cpu=backing;t->s.va=t->w.va+UINT64_C(0x80000000);t->s.size=(size_t)experts*(n/128)*(k/128)*4;
    return t;
}
#elif defined(MG_NEST_BF16)
#include "../src/omega_gpu_bf16_gemv_api.c"
void *mg_nested_bf16(unsigned slot,uint32_t n,uint32_t k) {
    static OmegaGpuBf16Tensor tensors[16];static uint8_t backing[256];
    if(slot>=16 || !dev_open_locked())return NULL;
    OmegaGpuBf16Tensor *t=&tensors[slot];t->n=n;t->k=k;t->w.cpu=backing;
    t->w.va=UINT64_C(0x1b00000000)+(uint64_t)slot*UINT64_C(0x100000000);t->w.size=(size_t)n*k*2;
    return t;
}
#elif defined(MG_NEST_GDN)
#include "../src/omega_gpu_gdn_api.c"
void *mg_nested_gdn(unsigned slot,uint32_t ratio,uint32_t layers) {
    static OmegaGpuGdn objects[8];static uint8_t lb[8][LB_BYTES];
    if(slot>=8 || !dev_open_locked())return NULL;
    OmegaGpuGdn *o=&objects[slot];o->layers=layers;o->ratio=ratio;
    if(shape_of(ratio,&o->sh))return NULL;
    uint64_t base=UINT64_C(0x3000000000)+(uint64_t)slot*UINT64_C(0x100000000);
    o->par.va=base;o->tail.va=base+UINT64_C(0x10000000);o->state.va=base+UINT64_C(0x20000000);
    o->lb.va=base+UINT64_C(0x80000000);o->lb.cpu=lb[slot];o->lb.size=LB_BYTES;
    if(omega_gpu_gdn_multi_prepare(o))return NULL;
    return o;
}
#elif defined(MG_NEST_ATTN)
#include "../src/omega_gpu_decode_attn_api.c"
void *mg_nested_attn(unsigned slot,uint32_t layers) {
    static OmegaGpuDecodeAttnCache objects[8];static uint32_t backing[8192];
    if(slot>=8 || !dev_open_locked())return NULL;
    OmegaGpuDecodeAttnCache *c=&objects[slot];c->layers=layers;c->max_pos=OMEGA_GPU_DATTN_MAX_POS;
    c->max_splits=c->max_pos/OMEGA_DATTN_CHUNK;c->plane_bytes=(uint64_t)OMEGA_GPU_DATTN_KV_HEADS*c->max_pos*OMEGA_DATTN_ROW_BYTES+OMEGA_DATTN_PLANE_SLACK;
    uint64_t base=UINT64_C(0x4000000000)+(uint64_t)slot*UINT64_C(0x1000000000);
    for(unsigned i=0;i<layers;i++)c->kv[i].va=base+(uint64_t)i*2*c->plane_bytes;
    c->rope.va=base+UINT64_C(0x800000000);c->norms.va=c->rope.va+UINT64_C(0x10000000);c->part.va=c->norms.va+UINT64_C(0x10000000);
    c->cnt.va=c->part.va+UINT64_C(0x10000000);c->out.cpu=backing;c->cnt.cpu=backing;
    return c;
}
#elif defined(MG_NEST_ATTN256)
#include "../src/omega_gpu_decode_attn256_api.c"
void *mg_nested_attn256(unsigned slot,uint32_t layers,uint32_t qh,uint32_t kvh,uint32_t qpc) {
    static OmegaGpuDecodeAttn256Cache objects[8];static uint32_t backing[8192];
    if(slot>=8 || !dev_open_locked())return NULL;
    OmegaGpuDecodeAttn256Cache *c=&objects[slot];c->layers=layers;c->max_pos=OMEGA_GPU_DATTN256_MAX_POS;c->rotary_dim=64;
    c->shape.q_heads=qh;c->shape.kv_heads=kvh;c->shape.q_per_cta=qpc;
    if(omega_dattn256_shape(&c->shape,&c->sh))return NULL;
    c->max_splits=c->max_pos/OMEGA_DATTN256_CHUNK;c->plane_bytes=(uint64_t)c->sh.kvh*c->max_pos*RB+OMEGA_DATTN256_PLANE_SLACK;
    uint64_t base=UINT64_C(0xc000000000)+(uint64_t)slot*UINT64_C(0x1000000000);
    for(unsigned i=0;i<layers;i++)c->kv[i].va=base+(uint64_t)i*2*c->plane_bytes;
    c->rope.va=base+UINT64_C(0x800000000);c->norms.va=c->rope.va+UINT64_C(0x10000000);c->part.va=c->norms.va+UINT64_C(0x10000000);
    c->cnt.va=c->part.va+UINT64_C(0x10000000);c->out.cpu=backing;c->cnt.cpu=backing;
    if(!(c->sh.vy&(c->sh.vy-1u)) && omega_gpu_decode_attn256_multi_prepare(c))return NULL;
    return c;
}
#elif defined(MG_NEST_ROUTER)
#include "../src/omega_gpu_router_api.c"
void *mg_nested_router(uint32_t layers,uint32_t experts,uint32_t shared) {
    static OmegaGpuRouter r;static uint8_t backing[LB_BYTES9];
    if(!dev_open_locked())return NULL;
    r.layers=layers;r.experts=experts;r.shared=shared;r.rows=experts+shared;r.slots=8+shared;
    r.lb_tab_dn=shared?LB_TAB_DN9:LB_TAB_DN;r.lb_bytes=shared?LB_BYTES9:LB_BYTES;
    r.gate.va=UINT64_C(0x15000000000);r.norm.va=r.gate.va+UINT64_C(0x10000000);r.dir.va=r.norm.va+UINT64_C(0x10000000);
    r.lb.va=r.dir.va+UINT64_C(0x10000000);r.lb.cpu=backing;r.owner_generation=1;
    if(omega_gpu_router_prepare_rows(&r,2048*4,32768))return NULL;
    return &r;
}
#else
#error Select one MG_NEST recorder family
#endif
