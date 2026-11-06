/* omega_gpu_gemm_ex.c -- the general matrix multiply over the C3 kernels (contract: omega_gpu_gemm_ex.h).
 * The plan, the routes, the k parts, the operand conversion, the k-part sums, the host recompute, the epilogue and the fp8
 * dequantization are seeded Omega (the om/gemmex units, generated src/cudapp_om_gemm_ex_<unit>.c, CUDApp#144); this file is the
 * memory shell: gathers into the units' byte windows, copies, allocation and the C3 calls.
 * Migration-pending C, kept as the differential oracle (mk/gemm-ex.mk): CUDAPP_MIGRATION_PENDING_GEMM_EX=1 builds the
 * old C path instead; CUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF=1 runs both on every call and aborts on any difference. */
#include "omega_gpu_gemm_ex.h"
#include "omega_gpu_session.h"

#include <string.h>

#ifndef CUDAPP_MIGRATION_PENDING_GEMM_EX
#define CUDAPP_MIGRATION_PENDING_GEMM_EX 0
#endif
#ifndef CUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF
#define CUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF 0
#endif
#define GX_C (CUDAPP_MIGRATION_PENDING_GEMM_EX || CUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF)
#define GX_OM (!CUDAPP_MIGRATION_PENDING_GEMM_EX || CUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF)

static int chip_core(void *ctx, OmegaGpuGemmType t, unsigned layout, uint32_t M, uint32_t N, uint32_t K, const void *A, uint64_t lda,
                     const void *B, uint64_t ldb, float *C) {
    (void)ctx;
    return omega_gpu_gemm_ld(t, layout, M, N, K, A, lda, B, ldb, C, NULL);
}
static OmegaGemmExCoreFn g_core = chip_core;
static void *g_core_ctx;
static int g_mutant;

void omega_gpu_gemm_ex_set_backend(OmegaGemmExCoreFn fn, void *ctx) { g_core = fn ? fn : chip_core; g_core_ctx = fn ? ctx : NULL; }
void omega_gpu_gemm_ex_test_set_mutant(int mutant) { g_mutant = mutant; }

#if GX_C /* ------------------------------------------------------------------ the migration-pending C (the oracle) */

#define TILE 4096u        /* rows / columns of C per host tile */
#define KMAX 16384u       /* k per C3 call (OMEGA_GPU_GEMM_MAX_DIM) */
#define KMAX3 1808u       /* k per bf16x3 call: 9 planes of roundup(k, 16) <= 16384 */

/* ------------------------------------------------------------------ number formats */

static float u2f(uint32_t u) { float f; memcpy(&f, &u, 4); return f; }
static uint32_t f2u(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }
static int fin(float f) { return (f2u(f) & 0x7f800000u) != 0x7f800000u; }
static double dabs(double v) { return v < 0 ? -v : v; }

static float h2f(uint16_t h) {
    uint32_t s = (uint32_t)(h >> 15) << 31, e = (h >> 10) & 31u, m = h & 0x3ffu;
    if (e == 31u) return u2f(s | 0x7f800000u | m << 13);
    if (e == 0u) { float v = (float)m * (1.0f / 16777216.0f); return s ? -v : v; } /* m 2^-24, exact */
    return u2f(s | (e + 112u) << 23 | m << 13);
}
static float e4m3f(uint8_t b) {
    uint32_t e = (b >> 3) & 15u, m = b & 7u;
    if (e == 15u && m == 7u) return u2f(0x7fc00000u);
    float v = e == 0u ? (float)m * (1.0f / 512.0f) : u2f((e + 120u) << 23 | m << 20);
    return b & 0x80u ? -v : v;
}
static uint32_t esize(unsigned t) { return t == OMEGA_GEMM_DT_F32 ? 4u : t <= OMEGA_GEMM_DT_F16 ? 2u : 1u; }
static float ldf(unsigned t, const void *p, uint64_t i) {
    switch (t) {
    case OMEGA_GEMM_DT_F32: { float f; memcpy(&f, (const uint8_t *)p + i * 4u, 4); return f; }
    case OMEGA_GEMM_DT_BF16: return u2f((uint32_t)((const uint16_t *)p)[i] << 16);
    case OMEGA_GEMM_DT_F16: return h2f(((const uint16_t *)p)[i]);
    case OMEGA_GEMM_DT_E4M3: return e4m3f(((const uint8_t *)p)[i]);
    default: return h2f((uint16_t)(((const uint8_t *)p)[i] << 8)); /* an e5m2 byte is the top byte of an f16 */
    }
}
static uint16_t f2bf_rne(float f) {
    uint32_t u = f2u(f);
    if (!fin(f)) return (uint16_t)(u >> 16 | ((u & 0x7fffffu) ? 0x40u : 0u));
    return (uint16_t)((u + 0x7fffu + ((u >> 16) & 1u)) >> 16);
}
static uint16_t f2h_rne(float f) {
    uint32_t u = f2u(f), s = (u >> 16) & 0x8000u, a = u & 0x7fffffffu;
    if (a >= 0x7f800000u) return (uint16_t)(s | 0x7c00u | (a > 0x7f800000u ? 0x200u : 0u));
    if (a >= 0x477ff000u) return (uint16_t)(s | 0x7c00u);      /* >= 65520 rounds past the f16 maximum */
    if (a < 0x38800000u) {                                     /* below 2^-14: an f16 subnormal (unit 2^-24) or zero */
        if (a < 0x33000000u) return (uint16_t)s;               /* below 2^-25 */
        uint32_t mant = (a & 0x7fffffu) | 0x800000u, sh = 126u - (a >> 23);
        uint32_t q = mant >> sh, rem = mant & ((1u << sh) - 1u), half = 1u << (sh - 1u);
        if (rem > half || (rem == half && (q & 1u))) q++;
        return (uint16_t)(s | q);
    }
    uint32_t r = a + 0xfffu + ((a >> 13) & 1u);
    return (uint16_t)(s | (((r >> 23) - 112u) << 10 | ((r >> 13) & 0x3ffu)));
}
/* double to float rounding to odd: one more rounding to bf16 or f16 is then the correct rounding of v */
static float d2f_odd(double v) {
    float f = (float)v;
    if (v != v || !fin(f) || (double)f == v) return f;
    uint32_t u = f2u(f);
    if (u & 1u) return f;
    return u2f(dabs(v) > dabs((double)f) ? u + 1u : u - 1u);
}
/* exact split of a finite f32 into three bf16 planes by truncation: f = hi + mid + lo, except the parts below 2^-133 */
static void split3(float f, uint16_t p[3]) {
    if (!fin(f)) { p[0] = p[1] = p[2] = 0; return; }
    uint32_t u = f2u(f);
    p[0] = (uint16_t)(u >> 16);
    float r1 = f - u2f(u & 0xffff0000u);
    uint32_t u1 = f2u(r1);
    p[1] = (uint16_t)(u1 >> 16);
    p[2] = (uint16_t)(f2u(r1 - u2f(u1 & 0xffff0000u)) >> 16);
}
/* plane products in k order, smallest first: (lo,lo) (lo,mid) (mid,lo) (lo,hi) (mid,mid) (hi,lo) (mid,hi) (hi,mid) (hi,hi) */
static const uint8_t TA[9] = { 2, 2, 1, 2, 1, 0, 1, 0, 0 }, TB[9] = { 2, 1, 2, 0, 1, 2, 0, 1, 0 };

/* ------------------------------------------------------------------ the call */

typedef struct {
    OmegaGemmExArgs x;
    uint64_t sa_r, sa_c, sb_c;           /* block-scale grid: rows, k blocks, column blocks */
    const float *sa, *sb;                /* this batch's scales */
    const uint8_t *A, *B;                /* this batch's operands */
    unsigned route;
    OmegaGpuGemmType core;
    uint64_t kmax;
} Call;

static uint64_t a_at(const Call *c, uint64_t i, uint64_t kk) { return c->x.op_a ? kk * c->x.lda + i : i * c->x.lda + kk; }
static uint64_t b_at(const Call *c, uint64_t kk, uint64_t j) { return c->x.op_b ? j * c->x.ldb + kk : kk * c->x.ldb + j; }
static float aval(const Call *c, uint64_t i, uint64_t kk) { return ldf(c->x.a_type, c->A, a_at(c, i, kk)); }
static float bval(const Call *c, uint64_t kk, uint64_t j) { return ldf(c->x.b_type, c->B, b_at(c, kk, j)); }
/* a stored element is non-finite (inf, NaN; e4m3: NaN only) when its exponent field has every bit set */
static int elem_bad(unsigned t, const uint8_t *p, uint64_t i) {
    switch (t) {
    case OMEGA_GEMM_DT_F32: { uint32_t u; memcpy(&u, p + i * 4u, 4); return (u & 0x7f800000u) == 0x7f800000u; }
    case OMEGA_GEMM_DT_BF16: return (((const uint16_t *)p)[i] & 0x7f80u) == 0x7f80u;
    case OMEGA_GEMM_DT_F16: return (((const uint16_t *)p)[i] & 0x7c00u) == 0x7c00u;
    case OMEGA_GEMM_DT_E4M3: return (p[i] & 0x7fu) == 0x7fu;
    default: return (p[i] & 0x7cu) == 0x7cu; /* e5m2 */
    }
}
/* 1 when any of n contiguous elements is non-finite. Per 64-bit word: every lane of y = (w & mask) ^ mask has its top bit
 * clear and is 0 exactly for a non-finite element, so y - 1 per lane sets a lane's top bit only when that lane or a lower
 * lane is 0 (a borrow); the OR of all words has a top bit set exactly when some element is non-finite. 64 bytes per step in
 * four 128-bit accumulators (GCC vector extension: NEON on arm64, SSE2 on x86-64). */
typedef uint64_t ScanV __attribute__((vector_size(16)));
static int run_bad(unsigned t, const uint8_t *p, uint64_t n) {
    const uint64_t es = esize(t), bytes = n * es;
    const uint64_t mask = t == OMEGA_GEMM_DT_F32 ? 0x7f8000007f800000ull : t == OMEGA_GEMM_DT_BF16 ? 0x7f807f807f807f80ull
                        : t == OMEGA_GEMM_DT_F16 ? 0x7c007c007c007c00ull : t == OMEGA_GEMM_DT_E4M3 ? 0x7f7f7f7f7f7f7f7full
                                                                                   : 0x7c7c7c7c7c7c7c7cull;
    const uint64_t lo = es == 4u ? 0x0000000100000001ull : es == 2u ? 0x0001000100010001ull : 0x0101010101010101ull;
    const uint64_t hi = lo << (8u * es - 1u);
    const ScanV vm = { mask, mask }, vl = { lo, lo };
    ScanV a0 = { 0, 0 }, a1 = a0, a2 = a0, a3 = a0;
    uint64_t i = 0, acc = 0;
    for (; i + 64u <= bytes; i += 64u) {
        ScanV w0, w1, w2, w3;
        memcpy(&w0, p + i, 16); memcpy(&w1, p + i + 16u, 16); memcpy(&w2, p + i + 32u, 16); memcpy(&w3, p + i + 48u, 16);
        a0 |= ((w0 & vm) ^ vm) - vl; a1 |= ((w1 & vm) ^ vm) - vl; a2 |= ((w2 & vm) ^ vm) - vl; a3 |= ((w3 & vm) ^ vm) - vl;
    }
    a0 |= a1 | a2 | a3;
    acc = a0[0] | a0[1];
    for (; i + 8u <= bytes; i += 8u) { uint64_t w; memcpy(&w, p + i, 8); acc |= ((w & mask) ^ mask) - lo; }
    if (acc & hi) return 1;
    for (i /= es; i < n; i++) if (elem_bad(t, p, i)) return 1;
    return 0;
}
/* one stored operand (rows stored rows of cols elements, leading dimension ld): flag[r] for every stored row r holding a
 * non-finite element (per_row), else flag[j] for every stored column j holding one; 1 when there is any */
static int scan_bad(unsigned t, const uint8_t *p, uint64_t rows, uint64_t cols, uint64_t ld, int per_row, uint8_t *flag) {
    const uint64_t es = esize(t);
    int any = 0;
    for (uint64_t r = 0; r < rows; r++) {
        const uint8_t *q = p + r * ld * es;
        if (!run_bad(t, q, cols)) continue;
        any = 1;
        if (per_row) flag[r] = 1;
        else for (uint64_t j = 0; j < cols; j++) if (elem_bad(t, q, j)) flag[j] = 1;
    }
    return any;
}
static double block_factor(const Call *c, uint64_t i, uint64_t kb, uint64_t j) {
    if (c->x.scale != OMEGA_GEMM_SCALE_BLOCK) return 1.0;
    return (double)c->sa[(i / c->x.block_m) * c->sa_c + kb] * (double)c->sb[kb * c->sb_c + j / c->x.block_n];
}

/* stored rows x cols with leading dimension ld; 0 when ld is short or the span passes 2^56 */
static int span_of(uint64_t rows, uint64_t cols, uint64_t ld, uint64_t *span) {
    if (ld < cols || (rows - 1u) > (1ull << 56) / ld) return 0;
    *span = (rows - 1u) * ld + cols;
    return 1;
}
static int extent(uint64_t span, uint64_t stride, uint64_t batch, uint64_t *ext) {
    uint64_t t;
    if (__builtin_mul_overflow(batch - 1u, stride, &t) || __builtin_add_overflow(t, span, ext) || *ext > (1ull << 58)) return 0;
    return 1;
}
static int overlap(const void *p, uint64_t pbytes, const void *q, uint64_t qbytes) {
    uintptr_t a = (uintptr_t)p, b = (uintptr_t)q;
    return a < b + qbytes && b < a + pbytes;
}
static int scales_finite(const float *s, uint64_t count, uint64_t stride, uint64_t batch) {
    for (uint64_t b = 0; b < (stride ? batch : 1u); b++)
        for (uint64_t i = 0; i < count; i++) if (!fin(s[b * stride + i])) return 0;
    return 1;
}

static int validate(Call *c, const OmegaGemmExArgs *in) {
    c->x = *in;
    OmegaGemmExArgs *x = &c->x;
    if (!x->batch) x->batch = 1;
    if (x->op_a > 1u || x->op_b > 1u || x->a_type > OMEGA_GEMM_DT_E5M2 || x->b_type > OMEGA_GEMM_DT_E5M2 ||
        x->c_type > OMEGA_GEMM_DT_E5M2 || x->compute > OMEGA_GEMM_EX_F32_FFMA_REF || x->scale > OMEGA_GEMM_SCALE_BLOCK || !x->c)
        return OMEGA_GPU_GEMM_BAD_ARGS;
    if (!x->m || !x->n || !x->k) return OMEGA_GPU_GEMM_BAD_ARGS;
    if (x->m > OMEGA_GPU_GEMM_EX_MAX_DIM || x->n > OMEGA_GPU_GEMM_EX_MAX_DIM || x->k > OMEGA_GPU_GEMM_EX_MAX_DIM ||
        x->batch > OMEGA_GPU_GEMM_EX_MAX_BATCH) return OMEGA_GPU_GEMM_TOO_LARGE;
    if (x->c_type > OMEGA_GEMM_DT_F16) return OMEGA_GPU_GEMM_UNSUPPORTED;
    const uint64_t ar = x->op_a ? x->k : x->m, ac = x->op_a ? x->m : x->k;
    const uint64_t br = x->op_b ? x->n : x->k, bc = x->op_b ? x->k : x->n;
    if (!x->lda) x->lda = ac;
    if (!x->ldb) x->ldb = bc;
    if (!x->ldc) x->ldc = x->n;
    uint64_t sa, sb, sc, ea, eb, ec;
    if (!span_of(ar, ac, x->lda, &sa) || !span_of(br, bc, x->ldb, &sb) || !span_of(x->m, x->n, x->ldc, &sc)) return OMEGA_GPU_GEMM_BAD_ARGS;
    if (x->batch > 1u && x->stride_c < sc) return OMEGA_GPU_GEMM_BAD_ARGS; /* C batches would overlap */
    if (!extent(sa, x->stride_a, x->batch, &ea) || !extent(sb, x->stride_b, x->batch, &eb) || !extent(sc, x->stride_c, x->batch, &ec))
        return OMEGA_GPU_GEMM_BAD_ARGS;
    const uint64_t cbytes = ec * esize(x->c_type);
    if (x->alpha != 0.0f) {
        if (!x->a || !x->b) return OMEGA_GPU_GEMM_BAD_ARGS;
        if (overlap(x->c, cbytes, x->a, ea * esize(x->a_type)) || overlap(x->c, cbytes, x->b, eb * esize(x->b_type))) return OMEGA_GPU_GEMM_BAD_ARGS;
    }
    if (x->scale == OMEGA_GEMM_SCALE_TENSOR) {
        if (!x->scale_a || !x->scale_b || !scales_finite(x->scale_a, 1, x->scale_stride_a, x->batch) ||
            !scales_finite(x->scale_b, 1, x->scale_stride_b, x->batch)) return OMEGA_GPU_GEMM_BAD_ARGS;
    } else if (x->scale == OMEGA_GEMM_SCALE_BLOCK) {
        if (!x->scale_a || !x->scale_b || !x->block_m || !x->block_n || !x->block_k) return OMEGA_GPU_GEMM_BAD_ARGS;
        c->sa_r = (x->m + x->block_m - 1u) / x->block_m; c->sa_c = (x->k + x->block_k - 1u) / x->block_k;
        c->sb_c = (x->n + x->block_n - 1u) / x->block_n;
        if (!scales_finite(x->scale_a, c->sa_r * c->sa_c, x->scale_stride_a, x->batch) ||
            !scales_finite(x->scale_b, c->sa_c * c->sb_c, x->scale_stride_b, x->batch)) return OMEGA_GPU_GEMM_BAD_ARGS;
    }
    /* the route */
    const unsigned ta = x->a_type, tb = x->b_type;
    const int bf_ok = ta != OMEGA_GEMM_DT_F32 && ta != OMEGA_GEMM_DT_F16 && tb != OMEGA_GEMM_DT_F32 && tb != OMEGA_GEMM_DT_F16;
    const int h_ok = ta != OMEGA_GEMM_DT_F32 && ta != OMEGA_GEMM_DT_BF16 && tb != OMEGA_GEMM_DT_F32 && tb != OMEGA_GEMM_DT_BF16;
    c->kmax = KMAX;
    if (x->compute == OMEGA_GEMM_EX_F32_FFMA_REF) { c->route = OMEGA_GEMM_ROUTE_FFMA; c->core = OMEGA_GPU_GEMM_F32_FFMA; }
    else if (ta == tb && ta == OMEGA_GEMM_DT_BF16) { c->route = OMEGA_GEMM_ROUTE_NATIVE; c->core = OMEGA_GPU_GEMM_BF16; }
    else if (ta == tb && ta == OMEGA_GEMM_DT_F16) { c->route = OMEGA_GEMM_ROUTE_NATIVE; c->core = OMEGA_GPU_GEMM_F16; }
    else if (ta == tb && ta == OMEGA_GEMM_DT_E4M3) { c->route = OMEGA_GEMM_ROUTE_NATIVE; c->core = OMEGA_GPU_GEMM_E4M3; }
    else if (bf_ok) { c->route = OMEGA_GEMM_ROUTE_WIDEN; c->core = OMEGA_GPU_GEMM_BF16; }
    else if (h_ok) { c->route = OMEGA_GEMM_ROUTE_WIDEN; c->core = OMEGA_GPU_GEMM_F16; }
    else if (x->compute == OMEGA_GEMM_EX_F32_FAST_BF16) { c->route = OMEGA_GEMM_ROUTE_FAST_BF16; c->core = OMEGA_GPU_GEMM_BF16; }
    else { c->route = OMEGA_GEMM_ROUTE_BF16X3; c->core = OMEGA_GPU_GEMM_BF16; c->kmax = KMAX3; }
    if (g_mutant == 2) x->op_b = OMEGA_GEMM_OP_N;
    return OMEGA_GPU_GEMM_OK;
}

/* the operand buffers of one k part: A' rows of op(A), B' columns of op(B) (B' row-major k x n for FFMA), allocated on the
 * first part that packs (na, nb elements of the core type); P the dense core output of a tile, acc the k-part sum */
typedef struct { uint8_t *a, *b; uint64_t na, nb; float *p; double *acc; uint8_t *rowbad, *colbad; } Bufs;

static uint32_t core_esize(OmegaGpuGemmType t) { return t == OMEGA_GPU_GEMM_E4M3 ? 1u : t == OMEGA_GPU_GEMM_F32_FFMA ? 4u : 2u; }
static uint32_t round16(uint32_t v) { return (v + 15u) & ~15u; }

/* encode an exactly representable finite value in the core type */
static void put_core(OmegaGpuGemmType t, uint8_t *dst, uint64_t i, float f) {
    if (!fin(f)) f = 0.0f;
    if (t == OMEGA_GPU_GEMM_F32_FFMA) memcpy(dst + i * 4u, &f, 4);
    else if (t == OMEGA_GPU_GEMM_F16) ((uint16_t *)dst)[i] = f2h_rne(f);
    else ((uint16_t *)dst)[i] = (uint16_t)(f2u(f) >> 16);
}

/* one k part [k0, k0 + kc) of the tile [m0, m0 + mc) x [n0, n0 + nc) into P (dense mc x nc) */
static int run_part(Call *c, Bufs *bf, int any_bad, uint64_t m0, uint32_t mc, uint64_t n0, uint32_t nc, uint64_t k0, uint32_t kc,
                    float *P, uint64_t *calls) {
    const OmegaGemmExArgs *x = &c->x;
    const OmegaGpuGemmType t = c->core;
    const unsigned layout = (x->op_a ? OMEGA_GPU_GEMM_A_COL : 0u) | (x->op_b ? OMEGA_GPU_GEMM_B_COL : 0u);
    (*calls)++;
    if (c->route == OMEGA_GEMM_ROUTE_NATIVE && !any_bad && omega_gpu_gemm_span(layout, mc, nc, kc, x->lda, x->ldb, NULL, NULL) == 0) {
        const uint64_t es = esize(x->a_type);
        const uint8_t *pa = c->A + (x->op_a ? k0 * x->lda + m0 : m0 * x->lda + k0) * es;
        const uint8_t *pb = c->B + (x->op_b ? n0 * x->ldb + k0 : k0 * x->ldb + n0) * es;
        return g_core(g_core_ctx, t, layout, mc, nc, kc, pa, x->lda, pb, x->ldb, P);
    }
    if (!bf->a) {
        bf->a = omega_gpu_host_calloc(bf->na, core_esize(t));
        bf->b = omega_gpu_host_calloc(bf->nb, core_esize(t));
        if (!bf->a || !bf->b) return OMEGA_GPU_GEMM_CHIP_FAIL;
    }
    if (c->route == OMEGA_GEMM_ROUTE_BF16X3) {
        const uint32_t kp = round16(kc), K = 9u * kp;
        uint16_t *A2 = (uint16_t *)bf->a, *B2 = (uint16_t *)bf->b;
        for (uint32_t i = 0; i < mc; i++) {
            uint16_t *row = A2 + (uint64_t)i * K;
            for (uint32_t kk = 0; kk < kp; kk++) {
                uint16_t p[3] = { 0, 0, 0 };
                if (kk < kc) split3(aval(c, m0 + i, k0 + kk), p);
                for (unsigned q = 0; q < 9u; q++) row[q * kp + kk] = p[TA[q]];
            }
        }
        for (uint32_t j = 0; j < nc; j++) {
            uint16_t *col = B2 + (uint64_t)j * K;
            for (uint32_t kk = 0; kk < kp; kk++) {
                uint16_t p[3] = { 0, 0, 0 };
                if (kk < kc) split3(bval(c, k0 + kk, n0 + j), p);
                for (unsigned q = 0; q < 9u; q++) col[q * kp + kk] = p[TB[q]];
            }
        }
        return g_core(g_core_ctx, OMEGA_GPU_GEMM_BF16, OMEGA_GPU_GEMM_B_COL, mc, nc, K, A2, 0, B2, 0, P);
    }
    if (c->route == OMEGA_GEMM_ROUTE_FAST_BF16) {
        uint16_t *A2 = (uint16_t *)bf->a, *B2 = (uint16_t *)bf->b;
        for (uint32_t i = 0; i < mc; i++)
            for (uint32_t kk = 0; kk < kc; kk++) { float f = aval(c, m0 + i, k0 + kk); A2[(uint64_t)i * kc + kk] = fin(f) ? f2bf_rne(f) : 0; }
        for (uint32_t j = 0; j < nc; j++)
            for (uint32_t kk = 0; kk < kc; kk++) { float f = bval(c, k0 + kk, n0 + j); B2[(uint64_t)j * kc + kk] = fin(f) ? f2bf_rne(f) : 0; }
        return g_core(g_core_ctx, OMEGA_GPU_GEMM_BF16, OMEGA_GPU_GEMM_B_COL, mc, nc, kc, A2, 0, B2, 0, P);
    }
    if (c->route == OMEGA_GEMM_ROUTE_FFMA) {
        for (uint32_t i = 0; i < mc; i++)
            for (uint32_t kk = 0; kk < kc; kk++) put_core(t, bf->a, (uint64_t)i * kc + kk, aval(c, m0 + i, k0 + kk));
        for (uint32_t kk = 0; kk < kc; kk++)
            for (uint32_t j = 0; j < nc; j++) put_core(t, bf->b, (uint64_t)kk * nc + j, bval(c, k0 + kk, n0 + j));
        return g_core(g_core_ctx, t, 0u, mc, nc, kc, bf->a, 0, bf->b, 0, P);
    }
    /* WIDEN, or NATIVE with non-finite inputs or a span past 2^32: dense copies in the core type, non-finite as 0 */
    if (t == OMEGA_GPU_GEMM_E4M3) {
        for (uint32_t i = 0; i < mc; i++)
            for (uint32_t kk = 0; kk < kc; kk++) { uint8_t v = c->A[a_at(c, m0 + i, k0 + kk)]; bf->a[(uint64_t)i * kc + kk] = (v & 0x7fu) == 0x7fu ? 0 : v; }
        for (uint32_t j = 0; j < nc; j++)
            for (uint32_t kk = 0; kk < kc; kk++) { uint8_t v = c->B[b_at(c, k0 + kk, n0 + j)]; bf->b[(uint64_t)j * kc + kk] = (v & 0x7fu) == 0x7fu ? 0 : v; }
    } else {
        for (uint32_t i = 0; i < mc; i++)
            for (uint32_t kk = 0; kk < kc; kk++) put_core(t, bf->a, (uint64_t)i * kc + kk, aval(c, m0 + i, k0 + kk));
        for (uint32_t j = 0; j < nc; j++)
            for (uint32_t kk = 0; kk < kc; kk++) put_core(t, bf->b, (uint64_t)j * kc + kk, bval(c, k0 + kk, n0 + j));
    }
    return g_core(g_core_ctx, t, OMEGA_GPU_GEMM_B_COL, mc, nc, kc, bf->a, 0, bf->b, 0, P);
}

static double host_dot(const Call *c, uint64_t i, uint64_t j) {
    double s = 0.0;
    for (uint64_t kk = 0; kk < c->x.k; kk++)
        s += (double)aval(c, i, kk) * (double)bval(c, kk, j) * block_factor(c, i, kk / (c->x.block_k ? c->x.block_k : 1u), j);
    return s;
}

static double load_c(unsigned t, const void *p, uint64_t i) { return (double)ldf(t, p, i); }
static void store_c(unsigned t, void *p, uint64_t i, double v) {
    if (t == OMEGA_GEMM_DT_F32) { float f = (float)v; memcpy((uint8_t *)p + i * 4u, &f, 4); }
    else if (t == OMEGA_GEMM_DT_BF16) ((uint16_t *)p)[i] = f2bf_rne(d2f_odd(v));
    else ((uint16_t *)p)[i] = f2h_rne(d2f_odd(v));
}

static int c_gemm_ex(const OmegaGemmExArgs *args) {
    if (!args) return OMEGA_GPU_GEMM_BAD_ARGS;
    Call c;
    memset(&c, 0, sizeof c);
    int rc = validate(&c, args);
    if (rc != OMEGA_GPU_GEMM_OK) return rc;
    const OmegaGemmExArgs *x = &c.x;
    OmegaGemmExInfo info;
    memset(&info, 0, sizeof info);
    const int compute = x->alpha != 0.0f;
    const double beta = g_mutant == 4 ? 0.0 : (double)x->beta;
    const uint64_t tm = x->m < TILE ? x->m : TILE, tn = x->n < TILE ? x->n : TILE;
    /* one k part and no block scales: the k-part sum is 0 + P, so the epilogue reads P and no sum is kept */
    const int one_part = x->k <= c.kmax && x->scale != OMEGA_GEMM_SCALE_BLOCK;
    /* direct: native, C f32 in dense rows no wider than one C3 call, beta 0, operands C3 addresses in place. A batch with
     * no non-finite input and alpha s == 1 then has the C3 output as its result (the bit-for-bit case of the header), so
     * C3 writes C itself, in parts of at most OMEGA_GPU_GEMM_MAX_DIM rows of n, with no host tile, sum or epilogue */
    const uint64_t dm = x->m < OMEGA_GPU_GEMM_MAX_DIM ? x->m : OMEGA_GPU_GEMM_MAX_DIM;
    const unsigned layout = (x->op_a ? OMEGA_GPU_GEMM_A_COL : 0u) | (x->op_b ? OMEGA_GPU_GEMM_B_COL : 0u);
    const int direct_shape = compute && one_part && c.route == OMEGA_GEMM_ROUTE_NATIVE && beta == 0.0 &&
                             x->c_type == OMEGA_GEMM_DT_F32 && x->ldc == x->n && x->n <= OMEGA_GPU_GEMM_MAX_DIM &&
                             omega_gpu_gemm_span(layout, (uint32_t)dm, (uint32_t)x->n, (uint32_t)x->k, x->lda, x->ldb, NULL, NULL) == 0;
    Bufs bf;
    memset(&bf, 0, sizeof bf);
    if (compute) {
        info.route = c.route; info.core_type = c.core;
        const uint64_t kc = x->k < c.kmax ? x->k : c.kmax;
        const uint64_t kcore = c.route == OMEGA_GEMM_ROUTE_BF16X3 ? 9u * round16((uint32_t)kc) : kc;
        bf.na = tm * kcore + 16u; bf.nb = tn * kcore + 16u;
        bf.rowbad = omega_gpu_host_calloc(x->m, 1);
        bf.colbad = omega_gpu_host_calloc(x->n, 1);
        if (!bf.rowbad || !bf.colbad) { rc = OMEGA_GPU_GEMM_CHIP_FAIL; goto out; }
    }
    for (uint64_t b = 0; b < x->batch; b++) {
        uint8_t *C = (uint8_t *)x->c + b * x->stride_c * esize(x->c_type);
        double alpha = (double)x->alpha;
        int any_bad = 0;
        if (compute) {
            c.A = (const uint8_t *)x->a + b * x->stride_a * esize(x->a_type);
            c.B = (const uint8_t *)x->b + b * x->stride_b * esize(x->b_type);
            if (x->scale != OMEGA_GEMM_SCALE_NONE) { c.sa = x->scale_a + b * x->scale_stride_a; c.sb = x->scale_b + b * x->scale_stride_b; }
            if (x->scale == OMEGA_GEMM_SCALE_TENSOR) alpha *= (double)c.sa[0] * (double)c.sb[0];
            memset(bf.rowbad, 0, x->m); memset(bf.colbad, 0, x->n);
            const int bad_a = scan_bad(x->a_type, c.A, x->op_a ? x->k : x->m, x->op_a ? x->m : x->k, x->lda, !x->op_a, bf.rowbad);
            const int bad_b = scan_bad(x->b_type, c.B, x->op_b ? x->n : x->k, x->op_b ? x->k : x->n, x->ldb, x->op_b, bf.colbad);
            any_bad = bad_a | bad_b;
            const uint64_t ku = g_mutant == 1 ? x->k - 1u : x->k; /* the one part's k (mutant 1 drops the last k) */
            if (direct_shape && !any_bad && alpha == 1.0) {
                for (uint64_t m0 = 0; m0 < x->m; m0 += dm) {
                    const uint32_t mc = (uint32_t)(x->m - m0 < dm ? x->m - m0 : dm);
                    float *out = (float *)C + m0 * x->ldc;
                    if (!ku) { memset(out, 0, (uint64_t)mc * x->n * sizeof(float)); continue; }
                    rc = run_part(&c, &bf, 0, m0, mc, 0, (uint32_t)x->n, 0, (uint32_t)ku, out, &info.core_calls);
                    if (rc != OMEGA_GPU_GEMM_OK) goto out;
                }
                continue;
            }
            if (!bf.p && !(bf.p = omega_gpu_host_calloc(tm * tn, sizeof(float)))) { rc = OMEGA_GPU_GEMM_CHIP_FAIL; goto out; }
            if (!one_part && !bf.acc && !(bf.acc = omega_gpu_host_calloc(tm * tn, sizeof(double)))) { rc = OMEGA_GPU_GEMM_CHIP_FAIL; goto out; }
        }
        for (uint64_t m0 = 0; m0 < x->m; m0 += TILE)
            for (uint64_t n0 = 0; n0 < x->n; n0 += TILE) {
                const uint32_t mc = (uint32_t)(x->m - m0 < TILE ? x->m - m0 : TILE), nc = (uint32_t)(x->n - n0 < TILE ? x->n - n0 : TILE);
                if (compute && one_part) {
                    const uint64_t ku = g_mutant == 1 ? x->k - 1u : x->k;
                    if (!ku) memset(bf.p, 0, (uint64_t)mc * nc * sizeof(float));
                    else if ((rc = run_part(&c, &bf, any_bad, m0, mc, n0, nc, 0, (uint32_t)ku, bf.p, &info.core_calls)) != OMEGA_GPU_GEMM_OK) goto out;
                } else if (compute) {
                    memset(bf.acc, 0, (uint64_t)mc * nc * sizeof(double));
                    for (uint64_t k0 = 0; k0 < x->k;) {
                        uint64_t kc = x->k - k0 < c.kmax ? x->k - k0 : c.kmax;
                        if (x->scale == OMEGA_GEMM_SCALE_BLOCK && kc > x->block_k - k0 % x->block_k) kc = x->block_k - k0 % x->block_k;
                        uint64_t ku = kc;
                        if (g_mutant == 1 && k0 + kc == x->k) ku = kc - 1u;
                        if (ku) {
                            rc = run_part(&c, &bf, any_bad, m0, mc, n0, nc, k0, (uint32_t)ku, bf.p, &info.core_calls);
                            if (rc != OMEGA_GPU_GEMM_OK) goto out;
                            uint64_t kb = x->scale == OMEGA_GEMM_SCALE_BLOCK ? k0 / x->block_k : 0;
                            if (g_mutant == 3 && x->scale == OMEGA_GEMM_SCALE_BLOCK) kb = (kb + 1u) % c.sa_c;
                            for (uint32_t i = 0; i < mc; i++)
                                for (uint32_t j = 0; j < nc; j++)
                                    bf.acc[(uint64_t)i * nc + j] += (double)bf.p[(uint64_t)i * nc + j] * block_factor(&c, m0 + i, kb, n0 + j);
                        }
                        k0 += kc;
                    }
                }
                for (uint32_t i = 0; i < mc; i++)
                    for (uint32_t j = 0; j < nc; j++) {
                        const uint64_t ci = (m0 + i) * x->ldc + n0 + j;
                        double v = 0.0;
                        if (compute) {
                            double a = one_part ? 0.0 + (double)bf.p[(uint64_t)i * nc + j] : bf.acc[(uint64_t)i * nc + j];
                            if (any_bad && (bf.rowbad[m0 + i] || bf.colbad[n0 + j])) { a = host_dot(&c, m0 + i, n0 + j); info.host_fixed++; }
                            v = alpha * a;
                        }
                        if (beta != 0.0) v += beta * load_c(x->c_type, C, ci);
                        store_c(x->c_type, C, ci, v);
                    }
            }
    }
out:
    omega_gpu_host_free(bf.a); omega_gpu_host_free(bf.b); omega_gpu_host_free(bf.p); omega_gpu_host_free(bf.acc);
    omega_gpu_host_free(bf.rowbad); omega_gpu_host_free(bf.colbad);
    if (x->info) *x->info = info;
    return rc;
}

/* ------------------------------------------------------------------ the fp8 grouped/MoE fallback */

#define FP8_SLAB_BYTES (32ull << 20) /* dequantized weight rows per gemm_ex call */

static int c_fp8_block_rows(const uint8_t *w, const float *scale, double scale_mul, uint32_t n, uint32_t k, uint32_t block,
                            const float *x32, const uint16_t *x16, uint32_t m, float *y, uint64_t ldy, OmegaGemmExInfo *info) {
    if (info) memset(info, 0, sizeof *info);
    if (!w || !scale || !y || (!x32) == (!x16) || n == 0 || k == 0 || m == 0 || block == 0 || (ldy && ldy < n)) return OMEGA_GPU_GEMM_BAD_ARGS;
    if (!ldy) ldy = n;
    const uint64_t kb = (k + (uint64_t)block - 1u) / block;
    uint64_t rows = FP8_SLAB_BYTES / ((uint64_t)k * 4u);
    if (rows == 0) rows = 1;
    if (rows > n) rows = n;
    float *slab = omega_gpu_host_calloc((size_t)(rows * k), sizeof *slab);
    if (!slab) return OMEGA_GPU_GEMM_CHIP_FAIL;
    int rc = OMEGA_GPU_GEMM_OK;
    for (uint64_t j0 = 0; j0 < n && rc == OMEGA_GPU_GEMM_OK; j0 += rows) {
        const uint64_t nr = n - j0 < rows ? n - j0 : rows;
        for (uint64_t j = 0; j < nr; j++) {
            const uint8_t *wr = w + (j0 + j) * k;
            const float *sr = scale + ((j0 + j) / block) * kb;
            float *dr = slab + j * k;
            for (uint64_t c = 0; c < k; c++) {
                uint64_t cb = c / block;
                if (g_mutant == 5) cb = (cb + 1u) % kb;
                dr[c] = (float)((double)e4m3f(wr[c]) * (double)sr[cb] * scale_mul); /* one rounding */
            }
        }
        OmegaGemmExInfo pi;
        OmegaGemmExArgs a = {
            .op_a = OMEGA_GEMM_OP_N, .op_b = OMEGA_GEMM_OP_T, .m = m, .n = nr, .k = k, .batch = 1,
            .a_type = x32 ? OMEGA_GEMM_DT_F32 : OMEGA_GEMM_DT_BF16, .b_type = OMEGA_GEMM_DT_F32, .c_type = OMEGA_GEMM_DT_F32,
            .a = x32 ? (const void *)x32 : (const void *)x16, .lda = k, .b = slab, .ldb = k, .c = y + j0, .ldc = ldy,
            .alpha = 1.0f, .beta = 0.0f, .info = &pi,
        };
        rc = c_gemm_ex(&a);
        if (info) { info->route = pi.route; info->core_type = pi.core_type; info->core_calls += pi.core_calls; info->host_fixed += pi.host_fixed; }
    }
    omega_gpu_host_free(slab);
    return rc;
}

#endif /* GX_C */

#if GX_OM /* ------------------------------------------------------------------ the Omega path: the memory shell */
#include "cudapp_native.h"

#define GX_WIN 65536u /* bytes of a unit's byte window (view uchar 65536 in om/gemmex) */
#define GX_ROW 4096u  /* words of a unit's tile-row window (acc, fix: one row of a 4096-wide C tile) */

/* The units' windows: x the call, p the plan, q one unit call's arguments, res a unit's result; ga gb gathered runs,
 * gc the copied C row or the acc scales, fl the recompute flags of a row, zp a zero P row and acc0 a zero sum row. */
typedef struct {
    uint64_t x[48], p[64], q[8], res, fix[GX_ROW], acc0[GX_ROW];
    uint8_t ga[GX_WIN], gb[GX_WIN], gc[GX_WIN], fl[GX_WIN], zp[GX_WIN];
} Gx;

/* the heap buffers of one call: the converted operands, the bad-row and bad-column flags, P and the k-part sums */
typedef struct {
    uint8_t *a, *b, *rowbad, *colbad;
    float *P;
    uint64_t *acc;
} GxBufs;

/* A unit that refuses its views (32), re-enters (33) or traps (1..14) is a defect of this shell, not of the call. */
#define GX_FAULT OMEGA_GPU_GEMM_CODEGEN_FAIL

static uint64_t gx_fbits(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }
static uint64_t gx_dbits(double d) { uint64_t u; memcpy(&u, &d, 8); return u; }
static uint64_t gx_min(uint64_t a, uint64_t b) { return a < b ? a : b; }

static void gx_load(Gx *g, const OmegaGemmExArgs *a) {
    uint64_t *x = g->x;
    x[0] = (uint64_t)a->op_a; x[1] = (uint64_t)a->op_b; x[2] = a->m; x[3] = a->n; x[4] = a->k; x[5] = a->batch;
    x[6] = (uint64_t)a->a_type; x[7] = (uint64_t)a->b_type; x[8] = (uint64_t)a->c_type;
    x[9] = a->lda; x[10] = a->stride_a; x[11] = a->ldb; x[12] = a->stride_b; x[13] = a->ldc; x[14] = a->stride_c;
    x[15] = gx_fbits(a->alpha); x[16] = gx_fbits(a->beta); x[17] = (uint64_t)a->compute; x[18] = (uint64_t)a->scale;
    x[19] = (uint64_t)(uintptr_t)a->scale_a; x[20] = (uint64_t)(uintptr_t)a->scale_b;
    x[21] = a->scale_stride_a; x[22] = a->scale_stride_b; x[23] = a->block_m; x[24] = a->block_n; x[25] = a->block_k;
    x[26] = (uint64_t)(uintptr_t)a->a; x[27] = (uint64_t)(uintptr_t)a->b; x[28] = (uint64_t)(uintptr_t)a->c;
    x[29] = (uint64_t)(int64_t)g_mutant;
}

static int gx_plan(Gx *g, uint64_t op) {
    static const int code[4] = {OMEGA_GPU_GEMM_OK, OMEGA_GPU_GEMM_BAD_ARGS, OMEGA_GPU_GEMM_TOO_LARGE, OMEGA_GPU_GEMM_UNSUPPORTED};
    if (omega_gemm_ex_plan(&g->res, op, g->x, 48, g->p, 64) != OMEGA_OK || g->res > 3u) return GX_FAULT;
    return code[g->res];
}

static int gx_row(Gx *g, uint64_t op, uint64_t t, uint64_t n, const uint8_t *src, uint8_t *dst) {
    return omega_gemm_ex_row(&g->res, op, t, n, src, GX_WIN, dst, GX_WIN) == OMEGA_OK ? OMEGA_GPU_GEMM_OK : GX_FAULT;
}

/* n elements of es bytes from element i0 of src at element stride st, packed into dst */
static void gx_gather(uint8_t *dst, const uint8_t *src, uint64_t i0, uint64_t st, uint64_t n, uint64_t es) {
    if (st == 1u) { memcpy(dst, src + i0 * es, n * es); return; }
    for (uint64_t i = 0; i < n; i++) memcpy(dst + i * es, src + (i0 + i * st) * es, es);
}

/* the start element and stride of: row i of op(A) from k0; column j of op(B) from k0; row kk of op(B) from column n0 */
static uint64_t gx_arow(const Gx *g, uint64_t i, uint64_t k0, uint64_t *st) {
    *st = g->x[0] ? g->p[2] : 1u;
    return g->x[0] ? k0 * g->p[2] + i : i * g->p[2] + k0;
}
static uint64_t gx_bcol(const Gx *g, uint64_t j, uint64_t k0, uint64_t *st) {
    *st = g->p[5] ? 1u : g->p[3];
    return g->p[5] ? j * g->p[3] + k0 : k0 * g->p[3] + j;
}
static uint64_t gx_brow(const Gx *g, uint64_t kk, uint64_t n0, uint64_t *st) {
    *st = g->p[5] ? g->p[3] : 1u;
    return g->p[5] ? n0 * g->p[3] + kk : kk * g->p[3] + n0;
}

/* the scale arrays: count floats in each of batches copies stride apart; a non-finite scale refuses the call */
static int gx_scales(Gx *g, const float *s, uint64_t count, uint64_t batches, uint64_t stride) {
    for (uint64_t b = 0; b < batches; b++)
        for (uint64_t i0 = 0; i0 < count; i0 += GX_WIN / 4u) {
            const uint64_t cn = gx_min(GX_WIN / 4u, count - i0);
            memcpy(g->ga, s + b * stride + i0, cn * 4u);
            if (gx_row(g, 7, OMEGA_GEMM_DT_F32, cn, g->ga, g->gb)) return GX_FAULT;
            if (g->res) return OMEGA_GPU_GEMM_BAD_ARGS;
        }
    return OMEGA_GPU_GEMM_OK;
}

/* one stored operand of rows x cols elements (type t, es bytes, leading dimension ld): per_row sets flag[r] for a row
 * holding a non-finite element, else flag[c] for each non-finite column c; *any is set when one exists */
static int gx_scan(Gx *g, uint64_t t, uint64_t es, const uint8_t *src, uint64_t rows, uint64_t cols, uint64_t ld, int per_row,
                   uint8_t *flag, uint64_t *any) {
    const uint64_t cw = GX_WIN / es;
    for (uint64_t r = 0; r < rows; r++)
        for (uint64_t c0 = 0; c0 < cols; c0 += cw) {
            const uint64_t cn = gx_min(cw, cols - c0);
            memcpy(g->ga, src + (r * ld + c0) * es, cn * es);
            if (gx_row(g, 7, t, cn, g->ga, g->gb)) return GX_FAULT;
            if (!g->res) continue;
            *any = 1;
            if (per_row) { flag[r] = 1; break; }
            if (gx_row(g, 8, t, cn, g->ga, flag + c0)) return GX_FAULT;
        }
    return OMEGA_GPU_GEMM_OK;
}

/* one C3 call for the tile part the plan's op 1 described: P (mc x nc, f32) = A'(mc x K) . B'; A and B are this batch's */
static int gx_part(Gx *g, GxBufs *bf, const uint8_t *A, const uint8_t *B, uint64_t m0, uint64_t mc, uint64_t n0, uint64_t nc,
                   uint64_t k0, float *P, uint64_t *calls) {
    const uint64_t *p = g->p, ku = p[37], K = p[47], ce = p[25], ea = p[26], eb = p[27];
    const OmegaGpuGemmType t = (OmegaGpuGemmType)p[7];
    uint64_t st;
    (*calls)++;
    if (p[39])
        return g_core(g_core_ctx, t, (unsigned)p[24], (uint32_t)mc, (uint32_t)nc, (uint32_t)ku, A + p[40], p[2], B + p[41], p[3], P);
    if (!bf->a) {
        bf->a = omega_gpu_host_calloc(p[34] * ce + GX_WIN, 1);
        bf->b = omega_gpu_host_calloc(p[35] * ce + GX_WIN, 1);
        if (!bf->a || !bf->b) return OMEGA_GPU_GEMM_CHIP_FAIL;
    }
    for (uint64_t i = 0; i < mc; i++) {
        const uint64_t i0 = gx_arow(g, m0 + i, k0, &st);
        gx_gather(g->ga, A, i0, st, ku, ea);
        if (gx_row(g, p[30], g->x[6], ku, g->ga, bf->a + i * K * ce)) return GX_FAULT;
    }
    if (p[32]) {
        for (uint64_t kk = 0; kk < ku; kk++) {
            const uint64_t i0 = gx_brow(g, k0 + kk, n0, &st);
            gx_gather(g->gb, B, i0, st, nc, eb);
            if (gx_row(g, p[31], g->x[7], nc, g->gb, bf->b + kk * nc * ce)) return GX_FAULT;
        }
    } else {
        for (uint64_t j = 0; j < nc; j++) {
            const uint64_t i0 = gx_bcol(g, n0 + j, k0, &st);
            gx_gather(g->gb, B, i0, st, ku, eb);
            if (gx_row(g, p[31], g->x[7], ku, g->gb, bf->b + j * K * ce)) return GX_FAULT;
        }
    }
    return g_core(g_core_ctx, t, (unsigned)p[33], (uint32_t)mc, (uint32_t)nc, (uint32_t)K, bf->a, 0, bf->b, 0, P);
}

/* the host recompute of row i (global) of the tile at column n0: fix[j] for every flagged j, in block_k runs */
static int gx_fixrow(Gx *g, const uint8_t *A, const uint8_t *B, const float *sa, const float *sb, uint64_t i, uint64_t n0, uint64_t nc) {
    const uint64_t *x = g->x, *p = g->p, k = x[4], blk = x[18] == OMEGA_GEMM_SCALE_BLOCK;
    uint64_t st;
    for (uint64_t j = 0; j < nc; j++) {
        if (!g->fl[j]) continue;
        uint64_t s = 0;
        for (uint64_t k0 = 0, cn; k0 < k; k0 += cn) {
            cn = gx_min(GX_WIN / 4u, k - k0);
            if (blk) cn = gx_min(cn, x[25] - k0 % x[25]);
            uint64_t i0 = gx_arow(g, i, k0, &st);
            gx_gather(g->ga, A, i0, st, cn, p[26]);
            i0 = gx_bcol(g, n0 + j, k0, &st);
            gx_gather(g->gb, B, i0, st, cn, p[27]);
            const uint64_t q[8] = {x[6], x[7], cn, s, blk, blk ? gx_fbits(sa[(i / x[23]) * p[13] + k0 / x[25]]) : 0u,
                                   blk ? gx_fbits(sb[(k0 / x[25]) * p[14] + (n0 + j) / x[24]]) : 0u, 0u};
            memcpy(g->q, q, sizeof q);
            if (omega_gemm_ex_dot(&g->res, g->q, 8, g->ga, GX_WIN, g->gb, GX_WIN) != OMEGA_OK) return GX_FAULT;
            s = g->res;
        }
        g->fix[j] = s;
    }
    return OMEGA_GPU_GEMM_OK;
}

/* the k-part sum of the tile rows: acc row i += P row i (times the block scales of k block p[38]) */
static int gx_accrows(Gx *g, const float *sa, const float *sb, uint64_t m0, uint64_t mc, uint64_t n0, uint64_t nc, const float *P,
                      uint64_t *acc) {
    const uint64_t *x = g->x, *p = g->p, blk = x[18] == OMEGA_GEMM_SCALE_BLOCK, kb = p[38];
    const uint64_t q[8] = {nc, blk, n0, blk ? x[24] : 1u, 0u, 0u, 0u, 0u};
    if (blk) memcpy(g->gc + 4, sb + kb * p[14] + n0 / x[24], ((n0 + nc - 1u) / x[24] - n0 / x[24] + 1u) * 4u);
    for (uint64_t i = 0; i < mc; i++) {
        if (blk) memcpy(g->gc, sa + ((m0 + i) / x[23]) * p[13] + kb, 4);
        memcpy(g->q, q, sizeof q);
        if (omega_gemm_ex_acc(&g->res, g->q, 8, P + i * nc, GX_WIN, g->gc, GX_WIN, acc + i * nc, GX_ROW) != OMEGA_OK) return GX_FAULT;
    }
    return OMEGA_GPU_GEMM_OK;
}

/* the epilogue of the tile rows into C (this batch's), with the recompute of the flagged elements */
static int gx_epirows(Gx *g, GxBufs *bf, const uint8_t *A, const uint8_t *B, const float *sa, const float *sb, uint8_t *C,
                      uint64_t any, uint64_t m0, uint64_t mc, uint64_t n0, uint64_t nc, uint64_t *fixed) {
    const uint64_t *x = g->x, *p = g->p, ec = p[28], compute = p[10], one = p[9];
    const uint64_t q[8] = {nc, x[8], compute, one, p[42], p[11], 0u, 0u};
    for (uint64_t i = 0; i < mc; i++) {
        if (compute) {
            if (gx_row(g, 9, any | (uint64_t)bf->rowbad[m0 + i] << 1, nc, bf->colbad + n0, g->fl)) return GX_FAULT;
            if (any && gx_fixrow(g, A, B, sa, sb, m0 + i, n0, nc)) return GX_FAULT;
        }
        const uint8_t *pv = compute ? (const uint8_t *)(bf->P + i * nc) : g->zp;
        const uint64_t *av = compute && !one ? bf->acc + i * nc : g->acc0;
        uint8_t *row = C + ((m0 + i) * p[4] + n0) * ec;
        memcpy(g->gc, row, nc * ec);
        memcpy(g->q, q, sizeof q);
        if (omega_gemm_ex_epi(&g->res, g->q, 8, pv, GX_WIN, av, GX_ROW, g->fix, GX_ROW, g->fl, GX_ROW, g->gc, GX_WIN) != OMEGA_OK)
            return GX_FAULT;
        memcpy(row, g->gc, nc * ec);
        *fixed += g->res;
    }
    return OMEGA_GPU_GEMM_OK;
}

/* one batch: the bad-element scan, the per-batch plan (op 2), then the direct rows or the tiles */
static int gx_batch(Gx *g, GxBufs *bf, const OmegaGemmExArgs *args, uint64_t b, uint64_t *calls, uint64_t *fixed) {
    uint64_t *x = g->x;
    const uint64_t *p = g->p, m = x[2], n = x[3], k = x[4];
    const uint8_t *A = (const uint8_t *)args->a + b * x[10] * p[26], *B = (const uint8_t *)args->b + b * x[12] * p[27];
    uint8_t *C = (uint8_t *)args->c + b * x[14] * p[28];
    const float *sa = args->scale_a ? args->scale_a + b * args->scale_stride_a : NULL;
    const float *sb = args->scale_b ? args->scale_b + b * args->scale_stride_b : NULL;
    uint64_t any = 0;
    int rc;
    if (p[10]) {
        memset(bf->rowbad, 0, m);
        memset(bf->colbad, 0, n);
        if ((rc = gx_scan(g, x[6], p[26], A, x[0] ? k : m, x[0] ? m : k, p[2], !x[0], bf->rowbad, &any))) return rc;
        if ((rc = gx_scan(g, x[7], p[27], B, p[5] ? n : k, p[5] ? k : n, p[3], (int)p[5], bf->colbad, &any))) return rc;
        x[30] = x[18] != OMEGA_GEMM_SCALE_NONE ? gx_fbits(sa[0]) : 0u;
        x[31] = x[18] != OMEGA_GEMM_SCALE_NONE ? gx_fbits(sb[0]) : 0u;
        x[32] = any;
        if ((rc = gx_plan(g, 2))) return rc;
        if (p[43]) {
            for (uint64_t m0 = 0; m0 < m; m0 += p[20]) {
                const uint64_t mc = gx_min(p[20], m - m0);
                x[30] = 0; x[31] = mc; x[32] = n; x[33] = 0; x[34] = m0; x[35] = 0;
                if ((rc = gx_plan(g, 1))) return rc;
                float *out = (float *)C + m0 * p[4];
                if (!p[37]) memset(out, 0, mc * n * 4u);
                else if ((rc = gx_part(g, bf, A, B, m0, mc, 0, n, 0, out, calls))) return rc;
            }
            return OMEGA_GPU_GEMM_OK;
        }
        if (!bf->P && !(bf->P = omega_gpu_host_calloc(p[21] * p[22] * 4u + GX_WIN, 1))) return OMEGA_GPU_GEMM_CHIP_FAIL;
        if (!p[9] && !bf->acc && !(bf->acc = omega_gpu_host_calloc(p[21] * p[22] + GX_ROW, 8))) return OMEGA_GPU_GEMM_CHIP_FAIL;
    }
    for (uint64_t m0 = 0; m0 < m; m0 += p[21])
        for (uint64_t n0 = 0; n0 < n; n0 += p[22]) {
            const uint64_t mc = gx_min(p[21], m - m0), nc = gx_min(p[22], n - n0);
            if (p[10]) {
                if (p[9]) {
                    x[30] = 0; x[31] = mc; x[32] = nc; x[33] = any; x[34] = m0; x[35] = n0;
                    if ((rc = gx_plan(g, 1))) return rc;
                    if (!p[37]) memset(bf->P, 0, mc * nc * 4u);
                    else if ((rc = gx_part(g, bf, A, B, m0, mc, n0, nc, 0, bf->P, calls))) return rc;
                } else {
                    memset(bf->acc, 0, mc * nc * 8u);
                    for (uint64_t k0 = 0; k0 < k; k0 += p[36]) {
                        x[30] = k0; x[31] = mc; x[32] = nc; x[33] = any; x[34] = m0; x[35] = n0;
                        if ((rc = gx_plan(g, 1))) return rc;
                        if (!p[37]) continue;
                        if ((rc = gx_part(g, bf, A, B, m0, mc, n0, nc, k0, bf->P, calls))) return rc;
                        if ((rc = gx_accrows(g, sa, sb, m0, mc, n0, nc, bf->P, bf->acc))) return rc;
                    }
                }
            }
            if ((rc = gx_epirows(g, bf, A, B, sa, sb, C, any, m0, mc, n0, nc, fixed))) return rc;
        }
    return OMEGA_GPU_GEMM_OK;
}

static int gx_run(Gx *g, const OmegaGemmExArgs *args) {
    const uint64_t *p = g->p;
    GxBufs bf = {0};
    uint64_t calls = 0, fixed = 0;
    int rc = OMEGA_GPU_GEMM_OK;
    if (p[10]) {
        bf.rowbad = omega_gpu_host_calloc(g->x[2] + GX_WIN, 1);
        bf.colbad = omega_gpu_host_calloc(g->x[3] + GX_WIN, 1);
        if (!bf.rowbad || !bf.colbad) rc = OMEGA_GPU_GEMM_CHIP_FAIL;
    }
    for (uint64_t b = 0; b < p[1] && rc == OMEGA_GPU_GEMM_OK; b++) rc = gx_batch(g, &bf, args, b, &calls, &fixed);
    if (args->info) {
        args->info->route = p[10] ? (uint32_t)p[6] : OMEGA_GEMM_ROUTE_NONE;
        args->info->core_type = p[10] ? (uint32_t)p[7] : 0u;
        args->info->core_calls = calls;
        args->info->host_fixed = fixed;
    }
    omega_gpu_host_free(bf.a);
    omega_gpu_host_free(bf.b);
    omega_gpu_host_free(bf.rowbad);
    omega_gpu_host_free(bf.colbad);
    omega_gpu_host_free(bf.P);
    omega_gpu_host_free(bf.acc);
    return rc;
}

static int om_gemm_ex(const OmegaGemmExArgs *args) {
    if (!args) return OMEGA_GPU_GEMM_BAD_ARGS;
    Gx *g = omega_gpu_host_calloc(1, sizeof *g);
    if (!g) return OMEGA_GPU_GEMM_CHIP_FAIL;
    gx_load(g, args);
    int rc = gx_plan(g, 0);
    if (rc == OMEGA_GPU_GEMM_OK) rc = gx_scales(g, args->scale_a, g->p[15], g->p[17], args->scale_stride_a);
    if (rc == OMEGA_GPU_GEMM_OK) rc = gx_scales(g, args->scale_b, g->p[16], g->p[18], args->scale_stride_b);
    if (rc == OMEGA_GPU_GEMM_OK) rc = gx_run(g, args);
    omega_gpu_host_free(g);
    return rc;
}

static void gx_fp8_load(Gx *g, const uint8_t *w, const float *scale, uint32_t n, uint32_t k, uint32_t block, const float *x32,
                        const uint16_t *x16, uint32_t m, const float *y, uint64_t ldy) {
    uint64_t *x = g->x;
    x[29] = (uint64_t)(int64_t)g_mutant;
    x[30] = (uint64_t)(uintptr_t)w; x[31] = (uint64_t)(uintptr_t)scale; x[32] = (uint64_t)(uintptr_t)y;
    x[33] = (uint64_t)(uintptr_t)x32; x[34] = (uint64_t)(uintptr_t)x16;
    x[35] = n; x[36] = k; x[37] = m; x[38] = block; x[39] = ldy;
}

static int om_fp8_block_rows(const uint8_t *w, const float *scale, double scale_mul, uint32_t n, uint32_t k, uint32_t block,
                             const float *x32, const uint16_t *x16, uint32_t m, float *y, uint64_t ldy, OmegaGemmExInfo *info) {
    if (info) memset(info, 0, sizeof *info);
    Gx *g = omega_gpu_host_calloc(1, sizeof *g);
    if (!g) return OMEGA_GPU_GEMM_CHIP_FAIL;
    gx_fp8_load(g, w, scale, n, k, block, x32, x16, m, y, ldy);
    int rc = gx_plan(g, 3);
    const uint64_t ly = g->p[1], kb = g->p[2], rows = g->p[3], at = g->p[4];
    uint8_t *slab = rc == OMEGA_GPU_GEMM_OK ? omega_gpu_host_calloc(rows * k * 4u + GX_WIN, 1) : NULL;
    if (rc == OMEGA_GPU_GEMM_OK && !slab) rc = OMEGA_GPU_GEMM_CHIP_FAIL;
    const uint64_t cw = GX_WIN / 4u - 1u; /* 16383 weights per run: a run's scales, shifted one block, stay in one window */
    for (uint64_t j0 = 0; j0 < n && rc == OMEGA_GPU_GEMM_OK; j0 += rows) {
        const uint64_t nr = gx_min(rows, n - j0);
        for (uint64_t j = 0; j < nr && rc == OMEGA_GPU_GEMM_OK; j++) {
            const uint8_t *wr = w + (j0 + j) * k;
            const float *sr = scale + (j0 + j) / block * kb;
            for (uint64_t c0 = 0; c0 < k; c0 += cw) {
                const uint64_t cn = gx_min(cw, k - c0), sbase = c0 / block;
                memcpy(g->ga, wr + c0, cn);
                memcpy(g->gb, sr + sbase, gx_min(GX_WIN / 4u, kb - sbase) * 4u);
                const uint64_t q[8] = {cn, c0, block, kb, gx_dbits(scale_mul), (uint64_t)(int64_t)g_mutant, sbase, gx_fbits(sr[0])};
                memcpy(g->q, q, sizeof q);
                if (omega_gemm_ex_fp8(&g->res, g->q, 8, g->ga, GX_WIN, g->gb, GX_WIN, slab + (j * k + c0) * 4u, GX_WIN) != OMEGA_OK ||
                    g->res) {
                    rc = GX_FAULT;
                    break;
                }
            }
        }
        if (rc != OMEGA_GPU_GEMM_OK) break;
        OmegaGemmExInfo pi = {0};
        OmegaGemmExArgs a = {
            .op_a = OMEGA_GEMM_OP_N, .op_b = OMEGA_GEMM_OP_T, .m = m, .n = nr, .k = k, .batch = 1,
            .a_type = (OmegaGemmDtype)at, .b_type = OMEGA_GEMM_DT_F32, .c_type = OMEGA_GEMM_DT_F32,
            .a = x32 ? (const void *)x32 : (const void *)x16, .lda = k, .b = slab, .ldb = k, .c = y + j0, .ldc = ly,
            .alpha = 1.0f, .beta = 0.0f, .info = &pi,
        };
        rc = om_gemm_ex(&a);
        if (info) { info->route = pi.route; info->core_type = pi.core_type; info->core_calls += pi.core_calls; info->host_fixed += pi.host_fixed; }
    }
    omega_gpu_host_free(slab);
    omega_gpu_host_free(g);
    return rc;
}
#endif /* GX_OM */

/* ------------------------------------------------------------------ the entries */
#if CUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF
#ifndef OMEGA_GPU_NO_POSIX /* the differential is a hosted test build only */
#include <stdio.h>
#include <stdlib.h>
#else
#error "CUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF is a hosted test build"
#endif

/* Both paths on every call: the C runs on a copy of C (or y), the Omega path on the caller's; return code, info and
 * output bytes must agree bit for bit. CUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF_SELFTEST flips one bit of the C copy
 * (the red check). */
static void gx_diff(int crc, int orc, const OmegaGemmExInfo *ci, const OmegaGemmExInfo *oi, uint8_t *ref, const void *out, uint64_t bytes) {
    const char *what = NULL;
#ifdef CUDAPP_MIGRATION_PENDING_GEMM_EX_DIFF_SELFTEST
    if (bytes) ref[bytes / 2u] ^= 1u;
#endif
    if (crc != orc) what = "return codes differ";
    else if (memcmp(ci, oi, sizeof *ci)) what = "info differs";
    else if (bytes && memcmp(ref, out, bytes)) what = "output bytes differ";
    if (!what) return;
    fprintf(stderr, "FAIL gemm_ex C-vs-Omega differential: %s (C rc %d, Omega rc %d)\n", what, crc, orc);
    abort();
}

int omega_gpu_gemm_ex(const OmegaGemmExArgs *args) {
    OmegaGemmExInfo ci, oi;
    memset(&ci, 0xff, sizeof ci);
    memset(&oi, 0xff, sizeof oi);
    if (!args) {
        const int crc = c_gemm_ex(NULL), orc = om_gemm_ex(NULL);
        gx_diff(crc, orc, &ci, &oi, NULL, NULL, 0);
        return orc;
    }
    Gx *g = omega_gpu_host_calloc(1, sizeof *g);
    if (!g) return OMEGA_GPU_GEMM_CHIP_FAIL;
    gx_load(g, args);
    const uint64_t bytes = gx_plan(g, 0) == OMEGA_GPU_GEMM_OK ? g->p[46] * g->p[28] : 0u;
    omega_gpu_host_free(g);
    uint8_t *ref = bytes ? omega_gpu_host_calloc(bytes, 1) : NULL;
    if (bytes && !ref) return OMEGA_GPU_GEMM_CHIP_FAIL;
    OmegaGemmExArgs ca = *args, oa = *args;
    if (ref) { memcpy(ref, args->c, bytes); ca.c = ref; }
    ca.info = &ci;
    oa.info = &oi;
    const int crc = c_gemm_ex(&ca), orc = om_gemm_ex(&oa);
    gx_diff(crc, orc, &ci, &oi, ref, args->c, bytes);
    omega_gpu_host_free(ref);
    if (args->info && oi.route != 0xffffffffu) *args->info = oi;
    return orc;
}

int omega_gpu_gemm_ex_fp8_block_rows(const uint8_t *w, const float *scale, double scale_mul, uint32_t n, uint32_t k, uint32_t block,
                                     const float *x32, const uint16_t *x16, uint32_t m, float *y, uint64_t ldy, OmegaGemmExInfo *info) {
    OmegaGemmExInfo ci, oi;
    Gx *g = omega_gpu_host_calloc(1, sizeof *g);
    if (!g) return OMEGA_GPU_GEMM_CHIP_FAIL;
    gx_fp8_load(g, w, scale, n, k, block, x32, x16, m, y, ldy);
    const uint64_t bytes = gx_plan(g, 3) == OMEGA_GPU_GEMM_OK ? ((uint64_t)(m - 1u) * g->p[1] + n) * 4u : 0u;
    omega_gpu_host_free(g);
    uint8_t *ref = bytes ? omega_gpu_host_calloc(bytes, 1) : NULL;
    if (bytes && !ref) return OMEGA_GPU_GEMM_CHIP_FAIL;
    if (ref) memcpy(ref, y, bytes);
    const int crc = c_fp8_block_rows(w, scale, scale_mul, n, k, block, x32, x16, m, ref ? (float *)(void *)ref : y, ldy, &ci);
    const int orc = om_fp8_block_rows(w, scale, scale_mul, n, k, block, x32, x16, m, y, ldy, &oi);
    gx_diff(crc, orc, &ci, &oi, ref, y, bytes);
    omega_gpu_host_free(ref);
    if (info) *info = oi;
    return orc;
}
#elif CUDAPP_MIGRATION_PENDING_GEMM_EX
int omega_gpu_gemm_ex(const OmegaGemmExArgs *args) { return c_gemm_ex(args); }
int omega_gpu_gemm_ex_fp8_block_rows(const uint8_t *w, const float *scale, double scale_mul, uint32_t n, uint32_t k, uint32_t block,
                                     const float *x32, const uint16_t *x16, uint32_t m, float *y, uint64_t ldy, OmegaGemmExInfo *info) {
    return c_fp8_block_rows(w, scale, scale_mul, n, k, block, x32, x16, m, y, ldy, info);
}
#else
int omega_gpu_gemm_ex(const OmegaGemmExArgs *args) { return om_gemm_ex(args); }
int omega_gpu_gemm_ex_fp8_block_rows(const uint8_t *w, const float *scale, double scale_mul, uint32_t n, uint32_t k, uint32_t block,
                                     const float *x32, const uint16_t *x16, uint32_t m, float *y, uint64_t ldy, OmegaGemmExInfo *info) {
    return om_fp8_block_rows(w, scale, scale_mul, n, k, block, x32, x16, m, y, ldy, info);
}
#endif
