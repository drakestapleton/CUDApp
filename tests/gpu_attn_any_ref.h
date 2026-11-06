/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef GPU_ATTN_ANY_REF_H
#define GPU_ATTN_ANY_REF_H
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/* Independent materialized-score softmax. No online recurrence, IR or production helpers. */
static double aa_input(const void *p, size_t i, unsigned dtype) {
    if (!dtype) return ((const float *)p)[i];
    if (dtype==2) {
        unsigned h=((const uint16_t *)p)[i], e=(h>>10)&31u, f=h&1023u;
        double value=e==31?(f?NAN:INFINITY):ldexp(e?1024.0+f:f,e?(int)e-25:-24);
        return copysign(value,(h&32768u)?-1.0:1.0);
    }
    uint32_t u = (uint32_t)((const uint16_t *)p)[i] << 16;
    float f; memcpy(&f, &u, sizeof f); return f;
}
static void aa_reference(const OmegaGpuAttnAnyShape *s, const void *q, const void *k,
                         const void *v, double *o, int wrong_mask) {
    double *score = malloc((size_t)s->seq_kv * sizeof *score);
    if (!score) abort();
    for (uint32_t h = 0; h < s->q_heads; ++h) {
        uint32_t kh = h / (s->q_heads / s->kv_heads);
        for (uint32_t i = 0; i < s->seq_q; ++i) {
            uint32_t n = s->seq_kv;
            if (s->causal && !wrong_mask && n > i + 1) n = i + 1;
            double top = -INFINITY, sum = 0;
            for (uint32_t j = 0; j < n; ++j) {
                double dot = 0;
                for (uint32_t d = 0; d < s->head_dim; ++d)
                    dot += aa_input(q, ((size_t)h*s->seq_q+i)*s->head_dim+d, s->dtype) *
                           aa_input(k, ((size_t)kh*s->seq_kv+j)*s->head_dim+d, s->dtype);
                score[j] = dot * (double)s->scale;
                if (score[j] > top) top = score[j];
            }
            for (uint32_t j = 0; j < n; ++j) { score[j] = exp(score[j]-top); sum += score[j]; }
            for (uint32_t d = 0; d < s->head_dim; ++d) {
                double a = 0;
                for (uint32_t j = 0; j < n; ++j)
                    a += score[j] * aa_input(v, ((size_t)kh*s->seq_kv+j)*s->head_dim+d, s->dtype);
                o[((size_t)h*s->seq_q+i)*s->head_dim+d] = a / sum;
            }
        }
    }
    free(score);
}
/* Wolfram derivation and finite-input envelope in om/attn-any/README.md. Absolute error, |V|<=1. */
#define AA_TOL 0.0009765625
static int aa_equal(const float *a, const double *b, size_t n) {
    for (size_t i = 0; i < n; ++i)
        if (!(fabs((double)a[i]-b[i]) <= AA_TOL)) return 0;
    return 1;
}
#endif
