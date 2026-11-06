/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_GRAPH_RESIDUE_H
#define OMEGA_GPU_GRAPH_RESIDUE_H
#include "cudapp_native.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/* The C oracle is present only in migration builds, OFF in the library. */
#ifndef CUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE
#define CUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE 0
#endif
static inline uint64_t omega_graph_meta(unsigned mode, const uint64_t c[16], uint64_t d[16]) {
    uint64_t result = 0;
    if (omega_grmeta(&result, mode, c, 16, d, 16) != OMEGA_OK) abort();
    return result;
}
static inline int omega_graph_budget(uint32_t peak, uint32_t limit, uint32_t max, uint32_t hw, int declared_ok) {
#if CUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE
    if (!limit) limit = max - 2u;
    return limit <= hw && peak <= limit && declared_ok;
#else
    const uint64_t c[16] = {peak, limit, max, hw, (uint64_t)declared_ok}; uint64_t d[16];
    return (int)omega_graph_meta(7, c, d);
#endif
}
static inline int omega_graph_timeline(uint32_t used, uint32_t count, uint32_t cap, int *from) {
#if CUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE
    const uint32_t skip = used ? 1 : 0;
    if (used > cap || (count > skip && count - skip > cap - used)) return 0;
    *from = (int)skip; return 1;
#else
    const uint64_t c[16] = {used, count, cap}; uint64_t d[16];
    if (!omega_graph_meta(6, c, d)) return 0;
    *from = (int)d[0]; return 1;
#endif
}
static inline int omega_graph_q38(const uint64_t c[16], uint64_t d[16]) {
#if CUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE
    uint64_t hd=c[0]?c[0]:5120, in=c[1]?c[1]:17408, qh=c[2]?c[2]:24, kv=c[3]?c[3]:4, gh=c[4]?c[4]:48;
    if (hd%1024 || hd>5120 || !in || in%256 || in>17408 || qh>24 || qh%kv || qh/kv>8 ||
        (gh!=32 && gh!=48) || qh*256!=gh*128) return 0;
    d[0]=hd; d[1]=in; d[2]=qh; d[3]=kv; d[4]=gh;
    d[5]=(32+gh)*128; d[6]=d[5]+gh*128; d[7]=2*gh;
    d[8]=(qh+kv)*512; d[9]=qh*256+kv*512; d[10]=qh*256;
    return 1;
#else
    return (int)omega_graph_meta(0, c, d);
#endif
}
static inline uint32_t omega_graph_config(uint32_t layers, uint32_t maxlayers,
    uint32_t vocab, uint32_t span, uint32_t heads, uint32_t maxheads, uint64_t mask, int usemask) {
#if CUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE
    if (!layers || layers > maxlayers || !vocab || vocab > span * span || !heads || heads > maxheads) return 0;
    if (usemask && layers < 64 && mask >> layers) return 0;
    return (vocab + span - 1) / span;
#else
    const uint64_t c[16] = {layers, maxlayers, vocab, span, heads, maxheads, mask, (uint64_t)usemask};
    uint64_t d[16];
    return omega_graph_meta(1, c, d) ? (uint32_t)d[0] : 0;
#endif
}
static inline void omega_graph_layer(uint64_t mask, uint32_t layer, uint64_t counts[2], int *attn, uint32_t *index) {
#if CUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE
    *attn = (int)((mask >> layer) & 1u);
    *index = (uint32_t)counts[*attn ? 0 : 1]++;
#else
    const uint64_t c[16] = {mask, layer, counts[0], counts[1]}; uint64_t d[16];
    if (!omega_graph_meta(2, c, d)) abort();
    *attn = (int)d[0]; *index = (uint32_t)d[1]; counts[0] = d[2]; counts[1] = d[3];
#endif
}
/* Only convert the checked scalar representation to the public uint32_t buffer.
 * Fixed blocks keep the bridge view's advertised extent within real storage. */
static inline void omega_graph_table(unsigned mode, unsigned qh, unsigned kv,
                                     uint32_t *dst, uint32_t n) {
    for (uint32_t base = 0; base < n;) {
        uint64_t words[512], result = 0;
        uint32_t count = n - base < 512 ? n - base : 512;
        if (omega_grtable(&result, mode, qh, kv, base, count, words, 512) != OMEGA_OK || result != 1) abort();
        for (uint32_t i = 0; i < count; ++i) dst[base + i] = (uint32_t)words[i];
        base += count;
    }
}
static inline int omega_graph_shape(uint32_t ae, uint32_t an, uint32_t ak,
                                    uint32_t e, uint32_t n, uint32_t k) {
#if CUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE
    return ae == e && an == n && ak == k;
#else
    const uint64_t c[16] = {ae, an, ak, e, n, k}; uint64_t d[16];
    return (int)omega_graph_meta(5, c, d);
#endif
}
#endif
