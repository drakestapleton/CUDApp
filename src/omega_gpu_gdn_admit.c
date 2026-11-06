/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_gdn_admit.c: see the header. */
#include "omega_gpu_gdn_admit.h"
#include "omega_gpu_gdn_api.h"
#include <math.h>
#include <string.h>

#if defined(CUDAPP_MIGRATION_PENDING_C_GDN_ADMIT)
int omega_gdn_admit_shape(uint32_t ratio, uint32_t state_bf16, OmegaGdnShape *s) {
    OmegaGdnSpec sp; memset(&sp, 0, sizeof sp); sp.ratio = ratio; sp.state_bf16 = state_bf16;
    return omega_gdn_shape(&sp, s);
}
int omega_gdn_admit_alloc(uint32_t layers, uint32_t ratio, uint32_t state_bf16, OmegaGdnShape *s) {
    if (layers == 0 || layers > OMEGA_GPU_GDN_MAX_LAYERS) return -1;
    return omega_gdn_admit_shape(ratio, state_bf16, s);
}
int omega_gdn_admit_layer(uint32_t layer, uint32_t layers) { return layer >= layers ? -1 : 0; }
int omega_gdn_admit_finite(float eps_l2, float eps_norm, float in_scale, float out_scale) {
    return !isfinite(eps_l2) || !isfinite(eps_norm) || !isfinite(in_scale) || !isfinite(out_scale) ? -1 : 0;
}
int omega_gdn_admit_multi(uint32_t layer, uint32_t layers, uint32_t tokens, uint64_t qkv_va, uint64_t z_va, uint32_t xstride,
                          uint64_t ab_va, uint32_t abstride, uint64_t out_va, uint32_t ostride) {
    if (layer >= layers || tokens == 0 || !qkv_va || !z_va || !ab_va || !out_va) return -1;
    if (((qkv_va | z_va | out_va | xstride | ostride) & 15u) || ((ab_va | abstride) & 7u)) return -1;
    return 0;
}
int omega_gdn_admit_tokens(uint32_t tokens) {
    return tokens < OMEGA_GDN_PF_MIN_T || tokens > OMEGA_GPU_GDN_PF_MAX_T ? -1 : 0;
}
int omega_gdn_admit_layout(uint32_t ratio, uint32_t T, size_t off[OMEGA_GDN_PF_SECTIONS], size_t *bytes) {
    OmegaGdnShape S; const OmegaGdnShape *shp = &S;
    if (T > OMEGA_GPU_GDN_PF_MAX_T || omega_gdn_admit_shape(ratio, 0, &S)) return -1;
    const uint64_t T8 = ((uint64_t)T + OMEGA_GDN_SCAN_CHUNK - 1u) / OMEGA_GDN_SCAN_CHUNK * OMEGA_GDN_SCAN_CHUNK;
    const uint64_t sz[OMEGA_GDN_PF_SECTIONS] = { (uint64_t)T * shp->conv * 4u, (uint64_t)T * shp->vdim * 4u,
                                                 (uint64_t)T * shp->vh * 8u, (uint64_t)T * shp->vdim * 4u,
                                                 (uint64_t)T * shp->vdim * 4u, T8 * shp->vh * OMEGA_GDN_REC_BYTES };
    size_t at = 0;
    for (unsigned i = 0; i < OMEGA_GDN_PF_SECTIONS; i++) { off[i] = at; at += (size_t)((sz[i] + 255u) & ~(uint64_t)255u); }
    *bytes = at;
    return 0;
}
int omega_gdn_admit_move(unsigned kind, uint64_t dst, size_t bytes, uint32_t *grid) {
    const size_t per = (size_t)OMEGA_GDN_COPY_WORDS_PER_CTA * 4u;
    if ((kind != OMEGA_GDN_ADMIT_KIND_COPY && kind != OMEGA_GDN_ADMIT_KIND_ZERO) || !dst || bytes == 0 || bytes % per ||
        bytes / per > 0x7fffffffu) return -1;
    *grid = (uint32_t)(bytes / per);
    return 0;
}
#else
#include "cudapp_native.h"
/* One call of the unit: 0 admitted, -1 refused or trapped. o carries inputs and outputs (see om/gdn/admit.om). */
static int admit(uint64_t op, uint64_t a, uint64_t b, uint64_t c, uint64_t o[16]) {
    uint64_t r = 1;
    if (omega_gdn_admit(&r, op, a, b, c, o, 16) != 0) return -1;
    return r == 0 ? 0 : -1;
}
static void shape_out(const uint64_t o[16], OmegaGdnShape *s) {
    s->r = (uint32_t)o[0]; s->vh = (uint32_t)o[1]; s->conv = (uint32_t)o[2]; s->vdim = (uint32_t)o[3];
    s->par_scal = (uint32_t)o[4]; s->par_norm = (uint32_t)o[5]; s->par_bytes = (uint32_t)o[6];
    s->tail_bytes = (uint32_t)o[7]; s->state_bytes = (uint32_t)o[8]; s->post_grid_y = (uint32_t)o[9];
}
static uint32_t f32_bits(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }
int omega_gdn_admit_shape(uint32_t ratio, uint32_t state_bf16, OmegaGdnShape *s) {
    uint64_t o[16] = {0};
    if (admit(1, ratio, state_bf16, 0, o)) return -1;
    shape_out(o, s);
    return 0;
}
int omega_gdn_admit_alloc(uint32_t layers, uint32_t ratio, uint32_t state_bf16, OmegaGdnShape *s) {
    uint64_t o[16] = {0};
    if (admit(2, layers, ratio, state_bf16, o)) return -1;
    shape_out(o, s);
    return 0;
}
int omega_gdn_admit_layer(uint32_t layer, uint32_t layers) { uint64_t o[16] = {0}; return admit(3, layer, layers, 0, o); }
int omega_gdn_admit_finite(float eps_l2, float eps_norm, float in_scale, float out_scale) {
    uint64_t o[16] = {0};
    return admit(4, f32_bits(eps_l2) | (uint64_t)f32_bits(eps_norm) << 32, f32_bits(in_scale) | (uint64_t)f32_bits(out_scale) << 32,
                 0, o);
}
int omega_gdn_admit_multi(uint32_t layer, uint32_t layers, uint32_t tokens, uint64_t qkv_va, uint64_t z_va, uint32_t xstride,
                          uint64_t ab_va, uint32_t abstride, uint64_t out_va, uint32_t ostride) {
    uint64_t o[16] = {qkv_va, z_va, ab_va, out_va, xstride, abstride, ostride};
    return admit(5, layer, layers, tokens, o);
}
int omega_gdn_admit_tokens(uint32_t tokens) { uint64_t o[16] = {0}; return admit(6, tokens, 0, 0, o); }
int omega_gdn_admit_layout(uint32_t ratio, uint32_t tokens, size_t off[OMEGA_GDN_PF_SECTIONS], size_t *bytes) {
    uint64_t o[16] = {0};
    if (admit(7, tokens, ratio, 0, o)) return -1;
    for (unsigned i = 0; i < OMEGA_GDN_PF_SECTIONS; i++) off[i] = (size_t)o[i];
    *bytes = (size_t)o[OMEGA_GDN_PF_SECTIONS];
    return 0;
}
int omega_gdn_admit_move(unsigned kind, uint64_t dst, size_t bytes, uint32_t *grid) {
    uint64_t o[16] = {0};
    if (admit(8, kind, bytes, dst, o)) return -1;
    *grid = (uint32_t)o[0];
    return 0;
}
#endif
