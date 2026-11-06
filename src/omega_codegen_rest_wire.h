/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_CODEGEN_REST_WIRE_H
#define OMEGA_CODEGEN_REST_WIRE_H
/* Memory-only adapters: widen fields without packing, then invoke the seeded unit.
 * The root copies the input prefix before replacing it with IR records. */
#include "cudapp_native.h"
#include "omega_gpu_om_ir.h"
#include "omega_blackwell_codegen_attn256.h"
#include "omega_blackwell_codegen_pattn256.h"
#include "omega_blackwell_codegen_router.h"
#include "omega_blackwell_codegen_gemm.h"
#include "omega_blackwell_codegen_bf16.h"
#include "omega_blackwell_codegen_qg.h"
static inline uint32_t omega_rest_attn256_wire(uint64_t *result, uint64_t kind, uint64_t a, uint64_t b, uint64_t c,
                                           void *words, uint64_t nw, void *metadata, uint64_t nm) {
    uint64_t *w = words;
    (void)c;
    w[15] = a != 0;
    w[16] = b != 0;
    if (a) {
        const OmegaDecodeAttn256Spec *s = (const OmegaDecodeAttn256Spec *)(uintptr_t)a;
        w[0] = (uint64_t)s->depth;
        w[1] = (uint64_t)(int64_t)s->mutant;
        w[2] = (uint64_t)s->prep;
        w[3] = (uint64_t)s->q_heads;
        w[4] = (uint64_t)s->kv_heads;
        w[5] = (uint64_t)s->q_per_cta;
        w[6] = (uint64_t)s->multi;
    }
    return omega_cg_attn256(result, kind, 0, 0, 0, words, nw, metadata, nm);
}
static inline uint32_t omega_rest_pattn256_wire(uint64_t *result, uint64_t kind, uint64_t a, uint64_t b, uint64_t c,
                                           void *words, uint64_t nw, void *metadata, uint64_t nm) {
    uint64_t *w = words;
    (void)c;
    w[15] = a != 0;
    w[16] = b != 0;
    if (a) {
        const OmegaPrefillAttn256Spec *s = (const OmegaPrefillAttn256Spec *)(uintptr_t)a;
        w[0] = (uint64_t)(int64_t)s->mutant;
        w[1] = (uint64_t)s->q_heads;
        w[2] = (uint64_t)s->kv_heads;
    }
    return omega_cg_pattn256(result, kind, 0, 0, 0, words, nw, metadata, nm);
}
static inline uint32_t omega_rest_router_wire(uint64_t *result, uint64_t kind, uint64_t a, uint64_t b, uint64_t c,
                                           void *words, uint64_t nw, void *metadata, uint64_t nm) {
    uint64_t *w = words;
    (void)c;
    w[15] = a != 0;
    w[16] = b != 0;
    if (a) {
        const OmegaRouterSpec *s = (const OmegaRouterSpec *)(uintptr_t)a;
        w[0] = (uint64_t)s->warps;
        w[1] = (uint64_t)s->depth;
        w[2] = (uint64_t)(int64_t)s->mutant;
        w[3] = (uint64_t)s->experts;
        w[4] = (uint64_t)s->shared;
        w[5] = (uint64_t)s->xstride;
        w[6] = (uint64_t)s->lbstride;
    }
    return omega_cg_router(result, kind, 0, 0, 0, words, nw, metadata, nm);
}
static inline uint32_t omega_rest_gemm_wire(uint64_t *result, uint64_t kind, uint64_t a, uint64_t b, uint64_t c,
                                           void *words, uint64_t nw, void *metadata, uint64_t nm) {
    uint64_t *w = words;
    (void)c;
    w[15] = a != 0;
    w[16] = b != 0;
    if (a) {
        const OmegaFp8GemmSpec *s = (const OmegaFp8GemmSpec *)(uintptr_t)a;
        w[0] = (uint64_t)s->k;
        w[1] = (uint64_t)s->lane_bytes;
        w[2] = (uint64_t)s->mt;
        w[3] = (uint64_t)s->warps;
        w[4] = (uint64_t)(int64_t)s->mutant;
        w[5] = (uint64_t)s->rowmap;
    }
    return omega_cg_gemm(result, kind, 0, 0, 0, words, nw, metadata, nm);
}
static inline uint32_t omega_rest_bf16_wire(uint64_t *result, uint64_t kind, uint64_t a, uint64_t b, uint64_t c,
                                           void *words, uint64_t nw, void *metadata, uint64_t nm) {
    uint64_t *w = words;
    (void)c;
    w[15] = a != 0;
    w[16] = b != 0;
    if (a) {
        const OmegaBf16GemvSpec *s = (const OmegaBf16GemvSpec *)(uintptr_t)a;
        w[0] = (uint64_t)s->k;
        w[1] = (uint64_t)s->rows;
        w[2] = (uint64_t)s->warps;
        w[3] = (uint64_t)s->depth;
        w[4] = (uint64_t)(int64_t)s->mutant;
        w[5] = (uint64_t)s->ystride;
        w[6] = (uint64_t)s->xbatch;
        w[7] = (uint64_t)s->mask;
    }
    return omega_cg_bf16(result, kind, 0, 0, 0, words, nw, metadata, nm);
}
static inline uint32_t omega_rest_qg_wire(uint64_t *result, uint64_t kind, uint64_t a, uint64_t b, uint64_t c,
                                           void *words, uint64_t nw, void *metadata, uint64_t nm) {
    uint64_t *w = words;
    (void)c;
    w[15] = a != 0;
    w[16] = b != 0;
    if (a && kind <= 1) {
        const OmegaPqSpec *s = (const OmegaPqSpec *)(uintptr_t)a;
        w[0] = (uint64_t)s->k;
        w[1] = (uint64_t)s->in_type;
        w[2] = (uint64_t)(int64_t)s->mutant;
        w[3] = (uint64_t)s->groups;
    } else if (a) {
        const OmegaQgSpec *s = (const OmegaQgSpec *)(uintptr_t)a;
        w[0] = (uint64_t)s->k;
        w[1] = (uint64_t)s->rt;
        w[2] = (uint64_t)s->warps;
        w[3] = (uint64_t)s->epi;
        w[4] = (uint64_t)s->direct;
        w[5] = (uint64_t)s->n;
        w[6] = (uint64_t)s->wait;
        w[7] = (uint64_t)(int64_t)s->mutant;
        w[8] = (uint64_t)s->group;
    }
    return omega_cg_qg(result, kind, 0, 0, 0, words, nw, metadata, nm);
}
static inline int omega_rest_query(OmegaOmIrUnit unit, uint64_t kind, const void *spec, void *out,
                                   uint64_t meta[OMEGA_OM_IR_META]) {
    uint64_t wire[OMEGA_OM_IR_WORDS] = {0}, result = 0;
    return unit(&result, kind, (uintptr_t)spec, (uintptr_t)out, 0, wire, OMEGA_OM_IR_WORDS,
                meta, OMEGA_OM_IR_META) == OMEGA_OK && result == 0 ? 0 : -1;
}
#endif
