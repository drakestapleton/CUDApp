/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Differential: retained C GDN admission (oracle, CUDAPP_MIGRATION_PENDING_C_GDN_ADMIT) against the Omega unit
 * gdn_admit. Every decision is compared for the refusal verdict and every output field. Domain: each predicate at
 * v-1/v/v+1, 0 and UINT32_MAX on every u32 field, non-finite f32 classes, every alignment residue class. */
#include "omega_gpu_gdn_admit.h"
#include "omega_gpu_gdn_api.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define O(f) omega_gdn_admit_##f##_oracle
int O(shape)(uint32_t, uint32_t, OmegaGdnShape *);
int O(alloc)(uint32_t, uint32_t, uint32_t, OmegaGdnShape *);
int O(layer)(uint32_t, uint32_t);
int O(finite)(float, float, float, float);
int O(multi)(uint32_t, uint32_t, uint32_t, uint64_t, uint64_t, uint32_t, uint64_t, uint32_t, uint64_t, uint32_t);
int O(tokens)(uint32_t);
int O(layout)(uint32_t, uint32_t, size_t *, size_t *);
int O(move)(unsigned, uint64_t, size_t, uint32_t *);
static unsigned long cases, admitted;
#define N(a) (sizeof a / sizeof a[0])
#define CHECK(c, cond, ...) do { ++cases; if (!(cond)) { printf("FAIL " __VA_ARGS__); puts(""); return 1; } if (!(c)) ++admitted; } while (0)
int main(void) {
    static const uint32_t ratios[] = {0, 1, 2, 3, 4, 5, 0x7fffffffu, 0x80000000u, UINT32_MAX};
    static const uint32_t bf[] = {0, 1, 2, UINT32_MAX};
    static const uint32_t lays[] = {0, 1, 2, 63, 64, 65, UINT32_MAX};
    for (unsigned r = 0; r < N(ratios); ++r) for (unsigned b = 0; b < N(bf); ++b) {
        OmegaGdnShape cs, os; memset(&cs, 0xa5, sizeof cs); memset(&os, 0xa5, sizeof os);
        int c = O(shape)(ratios[r], bf[b], &cs), o = omega_gdn_admit_shape(ratios[r], bf[b], &os);
        CHECK(c, c == o && (c || !memcmp(&cs, &os, sizeof cs)), "shape ratio=%u bf16=%u c=%d om=%d", ratios[r], bf[b], c, o);
        for (unsigned l = 0; l < N(lays); ++l) {
            memset(&cs, 0xa5, sizeof cs); memset(&os, 0xa5, sizeof os);
            c = O(alloc)(lays[l], ratios[r], bf[b], &cs); o = omega_gdn_admit_alloc(lays[l], ratios[r], bf[b], &os);
            CHECK(c, c == o && (c || !memcmp(&cs, &os, sizeof cs)), "alloc layers=%u ratio=%u bf16=%u", lays[l], ratios[r], bf[b]);
        }
    }
    for (unsigned a = 0; a < N(lays); ++a) for (unsigned b = 0; b < N(lays); ++b) {
        int c = O(layer)(lays[a], lays[b]), o = omega_gdn_admit_layer(lays[a], lays[b]);
        CHECK(c, c == o, "layer %u/%u", lays[a], lays[b]);
    }
    const float fv[] = {0.0f, -0.0f, 1e-6f, -1.0f, 3.4028235e38f, 1.4e-45f, INFINITY, -INFINITY, NAN};
    for (unsigned i = 0; i < N(fv); ++i) for (unsigned p = 0; p < 4; ++p) {
        float f[4] = {1.0f, 1.0f, 1.0f, 1.0f}; f[p] = fv[i];
        int c = O(finite)(f[0], f[1], f[2], f[3]), o = omega_gdn_admit_finite(f[0], f[1], f[2], f[3]);
        CHECK(c, c == o, "finite value=%g position=%u", (double)fv[i], p);
    }
    static const uint32_t toks[] = {0, 1, 2, 3, 4, 7, 8, 9, 65535, 65536, 65537, UINT32_MAX};
    for (unsigned t = 0; t < N(toks); ++t) {
        int c = O(tokens)(toks[t]), o = omega_gdn_admit_tokens(toks[t]);
        CHECK(c, c == o, "tokens %u", toks[t]);
        for (unsigned r = 0; r < N(ratios); ++r) {
            size_t co[6], oo[6], cb = 1, ob = 1; memset(co, 0, sizeof co); memset(oo, 0, sizeof oo);
            c = O(layout)(ratios[r], toks[t], co, &cb); o = omega_gdn_admit_layout(ratios[r], toks[t], oo, &ob);
            CHECK(c, c == o && (c || (cb == ob && !memcmp(co, oo, sizeof co))), "layout T=%u ratio=%u bytes=%zu/%zu", toks[t], ratios[r], cb, ob);
        }
    }
    /* multi: each address/stride over its residues mod 16 at a base, plus 0 and the extremes */
    static const uint64_t vas[] = {0, 1, 4, 8, 12, 15, 16, 0x1000, 0x1008, UINT64_MAX};
    static const uint32_t strd[] = {0, 1, 4, 8, 12, 16, 40960, 40968, UINT32_MAX};
    for (unsigned f = 0; f < 7; ++f) for (unsigned v = 0; v < (f < 4 ? N(vas) : N(strd)); ++v)
    for (unsigned t = 0; t < 3; ++t) for (unsigned l = 0; l < 3; ++l) {
        uint64_t va[4] = {0x10000, 0x20000, 0x30000, 0x40000}; uint32_t st[3] = {40960, 384, 24576};
        if (f < 4) va[f] = vas[v]; else st[f - 4] = strd[v];
        const uint32_t tk[3] = {0, 1, UINT32_MAX}, ly[3] = {0, 1, 2};
        int c = O(multi)(ly[l], 2, tk[t], va[0], va[1], st[0], va[2], st[1], va[3], st[2]);
        int o = omega_gdn_admit_multi(ly[l], 2, tk[t], va[0], va[1], st[0], va[2], st[1], va[3], st[2]);
        CHECK(c, c == o, "multi field=%u value=%u tokens=%u layer=%u", f, v, tk[t], ly[l]);
    }
    static const unsigned kinds[] = {0, 3, 4, 5, 6, UINT32_MAX};
    static const uint64_t dsts[] = {0, 1, UINT64_MAX};
    static const size_t bys[] = {0, 1, 4095, 4096, 4097, 8192, 4096ull * 0x7fffffffull, 4096ull * 0x7fffffffull + 4096u,
                                 4096ull * 0x7fffffffull - 1u, SIZE_MAX};
    for (unsigned k = 0; k < N(kinds); ++k) for (unsigned d = 0; d < N(dsts); ++d) for (unsigned y = 0; y < N(bys); ++y) {
        uint32_t cg = 7, og = 7;
        int c = O(move)(kinds[k], dsts[d], bys[y], &cg), o = omega_gdn_admit_move(kinds[k], dsts[d], bys[y], &og);
        CHECK(c, c == o && cg == og, "move kind=%u dst=%llu bytes=%zu", kinds[k], (unsigned long long)dsts[d], bys[y]);
    }
    printf("PASS gdn admission differential cases=%lu admitted=%lu refused=%lu verdict+outputs\n", cases, admitted, cases - admitted);
    return admitted > 0 && admitted < cases ? 0 : 6;
}
