/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* gpu_mathlibs_test.c -- CUDA audit rows 29 (cuFFT-class) and 30 (cuRAND-class): the GB10 xoshiro256** and
 * Philox4x32-10 generators and the f32 FFTs (complex 1-D, R2C / C2R, 2-D / 3-D) of src/omega_gpu_mathlibs_api.h,
 * kernels and pass plans from om/mathlibs/mathlibs_ir.om.
 *
 *   --host-only  no device: the xoshiro reference against the aien-math fixture (792 rows); the Philox reference
 *                against the Random123 known-answer vectors (a 9-round control must fail all three) and against
 *                the cuRAND fixture (1024 rows); refusals; every kernel through the unit, the allocator and the
 *                .om encoder, digest and listing checks; the twiddle tables up to n = 4096; the unit's pass plans
 *                and the launches they become; the normal transform's f32 model swept over every one of its 2^23
 *                inputs against an f64 erfinv (Newton on libm erf/erfc) within the Wolfram bound 2.87e-6
 *   --sim        --host-only plus the real kernels through the warp simulator (tests/bw_warp_sim.h): xoshiro raw
 *                and uniform outputs bit-exact against the reference, normal outputs bit-exact against the f32
 *                model and within the bound; Philox bit-exact against the KAT, the reference and every cuRAND
 *                fixture row; FFT N = 4..2048, R2C and C2R N = 8..4096 and 2-D / 3-D shapes within the Wolfram
 *                normwise tolerances of an f64 DFT under three warp orders; every mutant must be caught
 *   (no flag)    the GB10: 4096 streams x 17 at offset 0 and x 16 at offset 1000 for bases 0, 1729, 2^64-4
 *                against the fixture and the reference, uniform and normal runs, a 131072-stream run, the FFT at
 *                every N both ways, a batch of 2050 (three launches); Philox against the reference and the cuRAND
 *                fixture and a 131072-stream run; R2C / C2R at every N, the 2-D / 3-D shapes; every mutant must be
 *                caught. --out FILE writes the receipt lines there as well.
 *   --fixture F         the aien-math fixture (default tests/fixtures/mathlibs_rng_aienmath.txt)
 *   --philox-fixture F  the cuRAND fixture (default tests/fixtures/philox_curand_oracle.txt)
 * Last line: "mathlibs <tier>: PASS (<checks> checks)" or "mathlibs <tier>: FAIL (<failed> of <checks>)". */
#include "omega_gpu_mathlibs_api.h"
#include "omega_gpu_om_ir.h"
#include "omega_sm121_disasm.h"
#define WS_MAX_THREADS 1024 /* the real split / merge's N / 4 threads at N = 4096 */
#include "bw_warp_sim.h"
#include "test_out_file.h"
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_checks, g_failed;
static FILE *g_rcpt;

static void check(int ok, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void check(int ok, const char *fmt, ...) {
    g_checks++;
    if (ok) return;
    g_failed++;
    va_list ap;
    va_start(ap, fmt);
    printf("FAIL: ");
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

static void receipt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void receipt(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    if (g_rcpt) {
        va_start(ap, fmt);
        vfprintf(g_rcpt, fmt, ap);
        va_end(ap);
        fprintf(g_rcpt, "\n");
    }
}

static uint32_t f2u(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }
static float u2f(uint32_t u) { float f; memcpy(&f, &u, 4); return f; }

/* ------------------------------------------------------------------ reference generator */

#define GOLDEN 0x9E3779B97F4A7C15ull
typedef struct { uint64_t s[4]; } Xo;
static uint64_t rotl64(uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }
static void xo_seed(Xo *x, uint64_t seed) {
    for (int i = 0; i < 4; i++) {
        seed += GOLDEN;
        uint64_t z = seed;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        x->s[i] = z ^ (z >> 31);
    }
}
static uint64_t xo_next(Xo *x) {
    uint64_t *s = x->s, r = rotl64(s[1] * 5, 7) * 9, t = s[1] << 17;
    s[2] ^= s[0]; s[3] ^= s[1]; s[1] ^= s[2]; s[0] ^= s[3]; s[2] ^= t; s[3] = rotl64(s[3], 45);
    return r;
}
/* out[j * ns + g] = output offset + j of the stream seeded with seed + stream0 + g */
static void ref_raw(uint64_t seed, uint64_t stream0, uint32_t ns, uint32_t offset, uint32_t per, uint64_t *out) {
    for (uint32_t g = 0; g < ns; g++) {
        Xo x;
        xo_seed(&x, seed + stream0 + g);
        for (uint32_t i = 0; i < offset; i++) (void)xo_next(&x);
        for (uint32_t j = 0; j < per; j++) out[(size_t)j * ns + g] = xo_next(&x);
    }
}

/* ------------------------------------------------------------------ normal transform: f32 model and f64 truth */

/* f32 constants, bit patterns from evidence/MATHLIBS-RNG-FFT-2026-10-27/wolfram-constants.txt section 2 */
static const uint32_t GC_BITS[9] = { 0x3fc02e2f, 0x3e7c8f63, 0xbb88e4ef, 0xbaa45408, 0x396532db,
                                     0xb6935ac1, 0xb66c7357, 0x34b84b36, 0x32f16588 };
static const uint32_t GT_BITS[9] = { 0x40354f7e, 0x3f8036db, 0x3c1aa57e, 0xbbf9c5d7, 0x3bbc127b,
                                     0xbb70bde7, 0x3ab0dc72, 0x38d3b56b, 0xb951f09b };
#define NLN2_BITS 0xbf317218u
#define RT2_BITS 0x3fb504f3u
#define NORMAL_TOL 2.87e-6

static float uniform_of(uint64_t r) { return (float)(uint32_t)(r >> 40) * 0x1p-24f; }

/* The kernel's operation order with exact LG2 and RSQ (what the simulator executes): A = (r >> 41) * 2 + 1. */
static float normal_model(uint32_t A) {
    uint32_t b = 16777216u - A;
    int32_t xi = (int32_t)(2u * A - 16777216u);
    float q = (float)(int32_t)A * (float)(int32_t)b;
    q = q * 0x1p-46f;
    q = log2f(q);
    float w = q * u2f(NLN2_BITS);
    float t = w - 2.5f, pc = u2f(GC_BITS[8]);
    for (int k = 7; k >= 0; k--) pc = fmaf(pc, t, u2f(GC_BITS[k]));
    t = w * (1.0f / sqrtf(w));
    t = t - 3.0f;
    float pt = u2f(GT_BITS[8]);
    for (int k = 7; k >= 0; k--) pt = fmaf(pt, t, u2f(GT_BITS[k]));
    uint32_t mask = (uint32_t)0 - (f2u(w - 5.0f) >> 31);
    float p = u2f((mask & f2u(pc)) | (~mask & f2u(pt)));
    float x = (float)xi * 0x1p-24f;
    return p * x * u2f(RT2_BITS);
}

/* sqrt(2) erfinv(y), y = (2A - 2^24) / 2^24: Winitzki's start, Newton on erf (|y| <= 1/2) or erfc (tail) */
static double normal_ref(uint32_t A) {
    double y = ((double)(2.0 * A) - 16777216.0) * 0x1p-24;
    double omy2 = (double)A * (double)(16777216u - A) * 0x1p-46; /* 1 - y^2, exact */
    double c = 1.0 - fabs(y);                                    /* exact */
    const double a = 0.147, l = log(omy2), u = 2.0 / (M_PI * a) + l / 2.0;
    double x = sqrt(sqrt(u * u - l / a) - u);
    for (int it = 0; it < 12; it++) {
        double d = M_2_SQRTPI * exp(-x * x), step = fabs(y) > 0.5 ? -(erfc(x) - c) / d : (erf(x) - fabs(y)) / d;
        x -= step;
        if (fabs(step) <= 1e-17 * x) break;
    }
    return copysign(M_SQRT2 * x, y);
}
static uint32_t normal_A(uint64_t r) { return (uint32_t)(r >> 41) * 2u + 1u; }
static double rel_err(float z, double ref) { return fabs((double)z - ref) / fabs(ref); }

/* ------------------------------------------------------------------ FFT reference */

static const double FFT_TOL[12] = { 0, 0, 1.19209296e-7, 4.66610727e-7, 5.26215405e-7, 8.73616990e-7, 9.33221726e-7,
                                    1.28062354e-6, 1.34022856e-6, 1.68763126e-6, 1.74723836e-6, 2.09464702e-6 };
static int lg2u(uint32_t n) { int k = 0; while ((1u << k) < n) k++; return k; }

/* f64 DFT of one transform, twiddles exp(-+2 pi i (l m mod n) / n) from libm */
static void dft64(const float *x, double *y, uint32_t n, int inverse, const double *cs, const double *sn) {
    for (uint32_t m = 0; m < n; m++) {
        double re = 0, im = 0;
        for (uint32_t l = 0; l < n; l++) {
            uint32_t k = (uint32_t)(((uint64_t)l * m) % n);
            double c = cs[k], s = inverse ? sn[k] : -sn[k], xr = x[2 * l], xm = x[2 * l + 1];
            re += xr * c - xm * s;
            im += xr * s + xm * c;
        }
        y[2 * m] = re; y[2 * m + 1] = im;
    }
}
/* worst normwise ||y - ref|| / ||ref|| over the batch */
static double fft_err(const float *in, const float *out, uint32_t n, uint32_t batch, int inverse) {
    double *cs = malloc(n * sizeof *cs), *sn = malloc(n * sizeof *sn), *ref = malloc(2 * n * sizeof *ref), worst = 0;
    for (uint32_t k = 0; k < n; k++) { cs[k] = cos(2.0 * M_PI * k / n); sn[k] = sin(2.0 * M_PI * k / n); }
    for (uint32_t b = 0; b < batch; b++) {
        dft64(in + (size_t)2 * n * b, ref, n, inverse, cs, sn);
        double num = 0, den = 0;
        for (uint32_t i = 0; i < 2 * n; i++) {
            double d = (double)out[(size_t)2 * n * b + i] - ref[i];
            num += d * d; den += ref[i] * ref[i];
        }
        double e = sqrt(num / den);
        if (!(e <= worst)) worst = e; /* a NaN stays */
    }
    free(cs); free(sn); free(ref);
    return worst;
}
static uint64_t g_lcg = 0x243F6A8885A308D3ull;
static float rnd_pm1(void) {
    g_lcg = g_lcg * 6364136223846793005ull + 1442695040888963407ull;
    return (float)(int32_t)(g_lcg >> 40 & 0xffffff) * 0x1p-23f - 1.0f;
}
static void fill_rand(float *x, size_t n) { for (size_t i = 0; i < n; i++) x[i] = rnd_pm1(); }

/* ------------------------------------------------------------------ R2C, C2R and 2-D / 3-D references */

/* R2C tolerances by log2 N and the 2-D / 3-D ones by shape: WolframLanguageEvaluator, 2026-11-06, the query text in
 * evidence/MATHLIBS-PHILOX-RFFT-ND-2026-11-06/wolfram-tolerances.txt (normwise, against an f64 DFT) */
static const double R2C_TOL[13] = { 0, 0, 0, 8.561065560e-7, 1.550909633e-6, 1.670119030e-6, 2.364922423e-6,
                                    2.484131961e-6, 3.178935896e-6, 3.298146214e-6, 3.992952496e-6, 4.112168690e-6,
                                    4.806991727e-6 };
static const double C2R_TOL[13] = { 0, 0, 0, 7.368972629e-7, 1.084298912e-6, 1.143903636e-6, 1.491305459e-6,
                                    1.550910301e-6, 1.898312525e-6, 1.957918122e-6, 2.305322551e-6, 2.364934000e-6,
                                    2.712355043e-6 };
static const struct { uint32_t nd, d[3]; double tol; } ND_SHAPES[] = {
    { 1, { 256, 0, 0 }, 1.34022856e-6 }, /* FFT_TOL[8]: the 1-D plan */
    { 2, { 4, 4, 0 }, 2.384186066e-7 },    { 2, { 8, 16, 0 }, 9.928263778e-7 },   { 2, { 32, 64, 0 }, 1.806839531e-6 },
    { 2, { 16, 2048, 0 }, 2.620863531e-6 }, { 2, { 2048, 4, 0 }, 2.213856570e-6 }, { 3, { 4, 4, 4 }, 3.576279313e-7 },
    { 3, { 4, 8, 16 }, 1.112035792e-6 },   { 3, { 16, 16, 16 }, 1.578647047e-6 }, { 3, { 8, 4, 32 }, 1.459437580e-6 },
};
#define N_ND_SHAPES (sizeof ND_SHAPES / sizeof ND_SHAPES[0])

static void trig(uint32_t n, double **cs, double **sn) {
    *cs = malloc(n * sizeof **cs);
    *sn = malloc(n * sizeof **sn);
    for (uint32_t k = 0; k < n; k++) { (*cs)[k] = cos(2.0 * M_PI * k / n); (*sn)[k] = sin(2.0 * M_PI * k / n); }
}
static double normwise(const float *got, const double *ref, size_t len) {
    double num = 0, den = 0;
    for (size_t i = 0; i < len; i++) {
        double d = (double)got[i] - ref[i];
        num += d * d; den += ref[i] * ref[i];
    }
    return sqrt(num / den);
}
/* R2C: X_m, m <= n / 2, of every row of n reals, against out rows of n / 2 + 1 complex; worst row */
static double r2c_err(const float *in, const float *out, uint32_t n, uint32_t batch) {
    double *cs, *sn, *ref = malloc((n + 2) * sizeof *ref), worst = 0;
    trig(n, &cs, &sn);
    for (uint32_t b = 0; b < batch; b++) {
        const float *x = in + (size_t)n * b;
        for (uint32_t m = 0; m <= n / 2; m++) {
            double re = 0, im = 0;
            for (uint32_t l = 0; l < n; l++) {
                uint32_t k = (uint32_t)(((uint64_t)l * m) % n);
                re += x[l] * cs[k];
                im -= x[l] * sn[k];
            }
            ref[2 * m] = re; ref[2 * m + 1] = im;
        }
        double e = normwise(out + (size_t)(n + 2) * b, ref, n + 2);
        if (!(e <= worst)) worst = e; /* a NaN stays */
    }
    free(cs); free(sn); free(ref);
    return worst;
}
/* C2R of Hermitian rows (n / 2 + 1 complex, Im X_0 = Im X_(n/2) = 0): y_l = sum over all n bins, unnormalised */
static double c2r_err(const float *in, const float *out, uint32_t n, uint32_t batch) {
    double *cs, *sn, *ref = malloc(n * sizeof *ref), worst = 0;
    trig(n, &cs, &sn);
    for (uint32_t b = 0; b < batch; b++) {
        const float *X = in + (size_t)(n + 2) * b;
        for (uint32_t l = 0; l < n; l++) {
            double y = (double)X[0] + ((l & 1) ? -1.0 : 1.0) * X[n];
            for (uint32_t m = 1; m < n / 2; m++) {
                uint32_t k = (uint32_t)(((uint64_t)l * m) % n);
                y += 2.0 * (X[2 * m] * cs[k] - X[2 * m + 1] * sn[k]);
            }
            ref[l] = y;
        }
        double e = normwise(out + (size_t)n * b, ref, n);
        if (!(e <= worst)) worst = e;
    }
    free(cs); free(sn); free(ref);
    return worst;
}
/* f64 DFT in place along one axis of complex x: outer lines of n points at a stride of inner */
static void dft_axis(double *x, size_t outer, uint32_t n, size_t inner, int inverse) {
    double *cs, *sn, *ln = malloc(2 * n * sizeof *ln), *res = malloc(2 * n * sizeof *res);
    trig(n, &cs, &sn);
    for (size_t o = 0; o < outer; o++)
        for (size_t i = 0; i < inner; i++) {
            double *p = x + 2 * (o * n * inner + i);
            for (uint32_t l = 0; l < n; l++) { ln[2 * l] = p[2 * l * inner]; ln[2 * l + 1] = p[2 * l * inner + 1]; }
            for (uint32_t m = 0; m < n; m++) {
                double re = 0, im = 0;
                for (uint32_t l = 0; l < n; l++) {
                    uint32_t k = (uint32_t)(((uint64_t)l * m) % n);
                    double c = cs[k], s = inverse ? sn[k] : -sn[k];
                    re += ln[2 * l] * c - ln[2 * l + 1] * s;
                    im += ln[2 * l] * s + ln[2 * l + 1] * c;
                }
                res[2 * m] = re; res[2 * m + 1] = im;
            }
            for (uint32_t m = 0; m < n; m++) { p[2 * m * inner] = res[2 * m]; p[2 * m * inner + 1] = res[2 * m + 1]; }
        }
    free(cs); free(sn); free(ln); free(res);
}
/* the separable f64 DFT of each row-major [d0][d1][d2] array of the batch; worst normwise error */
static double nd_err(const float *in, const float *out, uint32_t nd, const uint32_t *dims, uint32_t batch, int inverse) {
    size_t tot = 1;
    for (uint32_t d = 0; d < nd; d++) tot *= dims[d];
    double *x = malloc(2 * tot * batch * sizeof *x), worst = 0;
    for (size_t i = 0; i < 2 * tot * batch; i++) x[i] = in[i];
    for (uint32_t ax = 0; ax < nd; ax++) {
        size_t outer = batch, inner = 1;
        for (uint32_t d = 0; d < ax; d++) outer *= dims[d];
        for (uint32_t d = ax + 1; d < nd; d++) inner *= dims[d];
        dft_axis(x, outer, dims[ax], inner, inverse);
    }
    for (uint32_t b = 0; b < batch; b++) {
        double e = normwise(out + 2 * tot * b, x + 2 * tot * b, 2 * tot);
        if (!(e <= worst)) worst = e;
    }
    free(x);
    return worst;
}

/* ------------------------------------------------------------------ Philox4x32-10 reference (Random123, cuRAND) */

#define PH_M0 0xD2511F53u
#define PH_M1 0xCD9E8D57u
#define PH_W0 0x9E3779B9u
#define PH_W1 0xBB67AE85u
/* rounds of Philox4x32 on ctr with key (key lo, key hi): the round, then the Weyl step of the key */
static void philox_block(const uint32_t ctr[4], uint64_t key, int rounds, uint32_t out[4]) {
    uint32_t c0 = ctr[0], c1 = ctr[1], c2 = ctr[2], c3 = ctr[3], k0 = (uint32_t)key, k1 = (uint32_t)(key >> 32);
    for (int r = 0; r < rounds; r++) {
        uint64_t p0 = (uint64_t)PH_M0 * c0, p1 = (uint64_t)PH_M1 * c2;
        uint32_t n0 = (uint32_t)(p1 >> 32) ^ c1 ^ k0, n2 = (uint32_t)(p0 >> 32) ^ c3 ^ k1;
        c0 = n0; c1 = (uint32_t)p1; c2 = n2; c3 = (uint32_t)p0;
        k0 += PH_W0; k1 += PH_W1;
    }
    out[0] = c0; out[1] = c1; out[2] = c2; out[3] = c3;
}
/* Random123 tests/kat_vectors, philox4x32 10: zero, all ones, pi digits (ctr, key, expected output) */
static const struct { uint32_t ctr[4], key[2], want[4]; } PH_KAT[3] = {
    { { 0, 0, 0, 0 }, { 0, 0 }, { 0x6627e8d5, 0xe169c58d, 0xbc57ac4c, 0x9b00dbd8 } },
    { { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff }, { 0xffffffff, 0xffffffff },
      { 0x408f276d, 0x41c83b0e, 0xa20bc7c6, 0x6d5451fd } },
    { { 0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344 }, { 0xa4093822, 0x299f31d0 },
      { 0xd16cfe09, 0x94fdcceb, 0x5001e420, 0x24126ea1 } },
};
/* cuRAND's k-th curand() output after curand_init(seed, subseq, offset), offset % 4 == 0: counter
 * (block lo, block hi, subseq lo, subseq hi), block = offset / 4 + k / 4, key (seed lo, seed hi) */
static void philox_ctr(uint64_t blk, uint64_t sub, uint32_t ctr[4]) {
    ctr[0] = (uint32_t)blk; ctr[1] = (uint32_t)(blk >> 32); ctr[2] = (uint32_t)sub; ctr[3] = (uint32_t)(sub >> 32);
}
static uint32_t philox_ref(uint64_t seed, uint64_t sub, uint64_t offset, uint32_t k) {
    uint32_t ctr[4], o[4];
    philox_ctr(offset / 4 + k / 4, sub, ctr);
    philox_block(ctr, seed, 10, o);
    return o[k % 4];
}
/* the API's layout: out[(j * ns + g) * 4 + c] = word c of block j of the stream with subsequence subseq0 + g */
static void ref_philox(uint64_t seed, uint64_t sub0, uint32_t ns, uint64_t offset, uint32_t per, uint32_t *out) {
    for (uint32_t j = 0; j < per / 4; j++)
        for (uint32_t g = 0; g < ns; g++) {
            uint32_t ctr[4];
            philox_ctr(offset / 4 + j, sub0 + g, ctr);
            philox_block(ctr, seed, 10, &out[((size_t)j * ns + g) * 4]);
        }
}

/* ------------------------------------------------------------------ fixture */

typedef struct { uint64_t seed; uint32_t pos; uint64_t val; } FixRow;
static FixRow g_fix[1024];
static size_t g_nfix;
static int load_fixture(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    char line[256];
    while (fgets(line, sizeof line, f) && g_nfix < sizeof g_fix / sizeof g_fix[0]) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char *e1, *e2, *e3;
        uint64_t s = strtoull(line, &e1, 10);
        unsigned long p = strtoul(e1, &e2, 10);
        uint64_t v = strtoull(e2, &e3, 10);
        if (e1 == line || e2 == e1 || e3 == e2) { fclose(f); return -2; }
        g_fix[g_nfix++] = (FixRow){ s, (uint32_t)p, v };
    }
    fclose(f);
    return 0;
}
static const uint64_t BASES[3] = { 0, 1729, 0xFFFFFFFFFFFFFFFCull };

/* fixture rows with seed = base + g (g < ns) and offset <= pos < offset + per, checked against out; g_fix_hit marks
 * every row compared (runs overlap, so the distinct count is what says the whole fixture was covered) */
static unsigned char g_fix_hit[1024];
static size_t fixture_compare(const uint64_t *out, uint64_t base, uint32_t ns, uint32_t offset, uint32_t per,
                              size_t *bad) {
    size_t used = 0;
    for (size_t i = 0; i < g_nfix; i++) {
        uint64_t g = g_fix[i].seed - base;
        if (g >= ns || g_fix[i].pos < offset || g_fix[i].pos >= offset + per) continue;
        used++;
        g_fix_hit[i] = 1;
        if (out[(size_t)(g_fix[i].pos - offset) * ns + g] != g_fix[i].val) (*bad)++;
    }
    return used;
}
static size_t fixture_distinct(void) {
    size_t n = 0;
    for (size_t i = 0; i < g_nfix; i++) n += g_fix_hit[i];
    return n;
}

/* the cuRAND fixture: "seed subseq offset k value_hex8", decimal but the value (tests/fixtures/philox_curand_oracle.txt
 * carries the generator source and command; the oracle is not a build dependency) */
typedef struct { uint64_t seed, sub, off; uint32_t k, val; } PhRow;
static PhRow g_ph[1024];
static size_t g_nph;
static unsigned char g_ph_hit[1024];
static int load_philox_fixture(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    char line[256];
    int rc = 0;
    while (rc == 0 && fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char *e0, *e1, *e2, *e3, *e4;
        uint64_t s = strtoull(line, &e0, 10);
        uint64_t sub = strtoull(e0, &e1, 10);
        uint64_t off = strtoull(e1, &e2, 10);
        unsigned long k = strtoul(e2, &e3, 10);
        unsigned long v = strtoul(e3, &e4, 16);
        if (e0 == line || e1 == e0 || e2 == e1 || e3 == e2 || e4 == e3 || k > 7 || v > 0xffffffffUL) rc = -2;
        else if (g_nph == sizeof g_ph / sizeof g_ph[0]) rc = -3;
        else g_ph[g_nph++] = (PhRow){ s, sub, off, (uint32_t)k, (uint32_t)v };
    }
    fclose(f);
    return rc;
}
/* rows of this seed and offset with subseq0 <= sub < subseq0 + ns and k < per, against the API layout */
static size_t ph_compare(const uint32_t *out, uint64_t seed, uint64_t sub0, uint32_t ns, uint64_t offset, uint32_t per,
                         size_t *bad) {
    size_t used = 0;
    for (size_t i = 0; i < g_nph; i++) {
        uint64_t g = g_ph[i].sub - sub0;
        if (g_ph[i].seed != seed || g_ph[i].off != offset || g >= ns || g_ph[i].k >= per) continue;
        used++;
        g_ph_hit[i] = 1;
        if (out[((size_t)(g_ph[i].k / 4) * ns + g) * 4 + g_ph[i].k % 4] != g_ph[i].val) (*bad)++;
    }
    return used;
}
static size_t ph_distinct(void) {
    size_t n = 0;
    for (size_t i = 0; i < g_nph; i++) n += g_ph_hit[i];
    return n;
}
static const uint64_t PH_SEEDS[4] = { 0, 1729, 0x123456789abcdefull, 0xffffffffffffffffull };
static const uint64_t PH_OFFS[4] = { 0, 4, 1000, 0x400000000ull };

/* ------------------------------------------------------------------ host tier */

static int build(uint64_t kind, uint64_t a, uint64_t b, uint64_t c, uint32_t threads, BlackwellIRProgram *prog,
                 uint8_t *code, OmegaBlackwellKernel *k) {
    uint32_t declared = 0;
    uint64_t emit_err = 0;
    return omega_gpu_om_ir_build(kind, a, b, c, threads, prog, code, OMEGA_BW_CODE_CAP_MAX, k, &declared, &emit_err);
}

static char *listing_of(const OmegaBlackwellKernel *k) {
    char *buf = NULL;
    size_t len = 0;
    FILE *f = open_memstream(&buf, &len);
    if (!f) return NULL;
    int rc = omega_sm121_listing(k->code, k->code_size, f);
    fclose(f);
    if (rc != 0) {
        printf("listing: %d words match no known form\n", rc);
        for (char *l = buf; l && (l = strstr(l, "??")); l++) {
            char *e = strchr(l, '\n');
            printf("  %.*s\n", e ? (int)(e - l) : 60, l);
        }
        free(buf);
        return NULL;
    }
    return buf;
}

static void host_fixture(void) {
    check(g_nfix == 792, "fixture rows %zu, expected 792", g_nfix);
    size_t bad = 0;
    for (size_t i = 0; i < g_nfix; i++) {
        Xo x;
        xo_seed(&x, g_fix[i].seed);
        for (uint32_t p = 0; p < g_fix[i].pos; p++) (void)xo_next(&x);
        if (xo_next(&x) != g_fix[i].val) bad++;
    }
    check(bad == 0, "C reference differs from aien-math in %zu of %zu fixture rows", bad, g_nfix);
    receipt("host rng_reference_vs_aienmath rows=%zu mismatches=%zu", g_nfix, bad);
}

static void host_refusals(void) {
    uint64_t buf[4];
    float fb[64], fo[64];
    check(omega_gpu_rng_generate(OMEGA_GPU_RNG_U64, 0, 0, 0, 0, 1, buf, NULL) == OMEGA_GPU_ML_BAD_ARGS, "rng ns=0");
    check(omega_gpu_rng_generate(OMEGA_GPU_RNG_U64, 0, 0, 1, 0, 0, buf, NULL) == OMEGA_GPU_ML_BAD_ARGS, "rng per=0");
    check(omega_gpu_rng_generate(OMEGA_GPU_RNG_U64, 0, 0, 1, 0, 1, NULL, NULL) == OMEGA_GPU_ML_BAD_ARGS, "rng out NULL");
    check(omega_gpu_rng_generate((OmegaGpuRngKind)3, 0, 0, 1, 0, 1, buf, NULL) == OMEGA_GPU_ML_BAD_ARGS, "rng kind 3");
    check(omega_gpu_rng_generate(OMEGA_GPU_RNG_U64, 0, 0, OMEGA_GPU_RNG_MAX_STREAMS + 1, 0, 1, buf, NULL) ==
              OMEGA_GPU_ML_TOO_LARGE, "rng streams past 131072");
    check(omega_gpu_rng_generate(OMEGA_GPU_RNG_U64, 0, 0, OMEGA_GPU_RNG_MAX_STREAMS, 0, 2049, buf, NULL) ==
              OMEGA_GPU_ML_TOO_LARGE, "rng outputs past 2^28");
    check(omega_gpu_rng_codegen((OmegaGpuRngKind)3, NULL) == OMEGA_GPU_ML_BAD_ARGS, "rng codegen kind 3");
    const uint32_t badn[] = { 0, 1, 2, 3, 6, 1000, 4096, 8192 };
    for (size_t i = 0; i < sizeof badn / sizeof badn[0]; i++) {
        check(omega_gpu_fft_c2c(fb, fo, badn[i], 1, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "fft n=%u", badn[i]);
        check(omega_gpu_fft_codegen(badn[i], 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "fft codegen n=%u", badn[i]);
    }
    check(omega_gpu_fft_c2c(fb, fo, 4, 0, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "fft batch=0");
    check(omega_gpu_fft_c2c(fb, fo, 4, 1, 2, NULL) == OMEGA_GPU_ML_BAD_ARGS, "fft inverse=2");
    check(omega_gpu_fft_c2c(NULL, fo, 4, 1, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "fft in NULL");
    check(omega_gpu_fft_c2c(fb, NULL, 4, 1, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "fft out NULL");
    check(omega_gpu_fft_c2c(fb, fb + 4, 4, 1, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "fft overlapping in/out");
    check(omega_gpu_fft_c2c(fb, fo, 2048, 65537, 0, NULL) == OMEGA_GPU_ML_TOO_LARGE, "fft past 2^28 floats");
    check(omega_gpu_fft_twiddles(8, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "twiddles NULL");
    uint32_t pw[8];
    check(omega_gpu_philox_generate(0, 0, 0, 0, 4, pw, NULL) == OMEGA_GPU_ML_BAD_ARGS, "philox ns=0");
    check(omega_gpu_philox_generate(0, 0, 1, 0, 0, pw, NULL) == OMEGA_GPU_ML_BAD_ARGS, "philox per=0");
    check(omega_gpu_philox_generate(0, 0, 1, 0, 6, pw, NULL) == OMEGA_GPU_ML_BAD_ARGS, "philox per=6");
    check(omega_gpu_philox_generate(0, 0, 1, 2, 4, pw, NULL) == OMEGA_GPU_ML_BAD_ARGS, "philox offset=2");
    check(omega_gpu_philox_generate(0, 0, 1, 0, 4, NULL, NULL) == OMEGA_GPU_ML_BAD_ARGS, "philox out NULL");
    check(omega_gpu_philox_generate(0, 0, OMEGA_GPU_PHILOX_MAX_STREAMS + 1, 0, 4, pw, NULL) == OMEGA_GPU_ML_TOO_LARGE,
          "philox streams past 131072");
    check(omega_gpu_philox_generate(0, 0, OMEGA_GPU_PHILOX_MAX_STREAMS, 0, 2052, pw, NULL) == OMEGA_GPU_ML_TOO_LARGE,
          "philox outputs past 2^28");
    const uint32_t badr[] = { 0, 4, 6, 12, 8192 };
    for (size_t i = 0; i < sizeof badr / sizeof badr[0]; i++) {
        check(omega_gpu_fft_r2c(fb, fo, badr[i], 1, NULL) == OMEGA_GPU_ML_BAD_ARGS, "r2c n=%u", badr[i]);
        check(omega_gpu_fft_c2r(fb, fo, badr[i], 1, NULL) == OMEGA_GPU_ML_BAD_ARGS, "c2r n=%u", badr[i]);
    }
    check(omega_gpu_fft_r2c(fb, fo, 8, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "r2c batch=0");
    check(omega_gpu_fft_r2c(NULL, fo, 8, 1, NULL) == OMEGA_GPU_ML_BAD_ARGS, "r2c in NULL");
    check(omega_gpu_fft_c2r(fb, NULL, 8, 1, NULL) == OMEGA_GPU_ML_BAD_ARGS, "c2r out NULL");
    check(omega_gpu_fft_r2c(fb, fb + 4, 8, 1, NULL) == OMEGA_GPU_ML_BAD_ARGS, "r2c overlapping in/out");
    check(omega_gpu_fft_c2r(fb + 4, fb, 8, 1, NULL) == OMEGA_GPU_ML_BAD_ARGS, "c2r overlapping in/out");
    check(omega_gpu_fft_r2c(fb, fo, 4096, 32769, NULL) == OMEGA_GPU_ML_TOO_LARGE, "r2c past 2^27 elements");
    const uint32_t d2[] = { 8, 16 }, d4[] = { 4, 4, 4, 4 }, dbig[] = { 2048, 2048, 64 };
    const uint32_t dbad[][3] = { { 2, 4, 4 }, { 4096, 4, 4 }, { 4, 6, 4 }, { 4, 4, 0 } };
    check(omega_gpu_fft_c2c_nd(fb, fo, 0, d2, 1, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "nd ndim=0");
    check(omega_gpu_fft_c2c_nd(fb, fo, 4, d4, 1, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "nd ndim=4");
    check(omega_gpu_fft_c2c_nd(fb, fo, 2, NULL, 1, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "nd dims NULL");
    for (size_t i = 0; i < sizeof dbad / sizeof dbad[0]; i++)
        check(omega_gpu_fft_c2c_nd(fb, fo, 3, dbad[i], 1, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "nd dims %u %u %u",
              dbad[i][0], dbad[i][1], dbad[i][2]);
    check(omega_gpu_fft_c2c_nd(fb, fo, 2, d2, 1, 2, NULL) == OMEGA_GPU_ML_BAD_ARGS, "nd inverse=2");
    check(omega_gpu_fft_c2c_nd(fb, fo, 2, d2, 0, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "nd batch=0");
    check(omega_gpu_fft_c2c_nd(fb, fo, 3, dbig, 1, 0, NULL) == OMEGA_GPU_ML_TOO_LARGE, "nd 2^28 elements");
    check(omega_gpu_fft_c2c_nd(fb, fo, 2, d2, (1u << 20) + 1, 0, NULL) == OMEGA_GPU_ML_TOO_LARGE,
          "nd batch past 2^27 elements");
    check(omega_gpu_fft_c2c_nd(fb, fb + 2, 2, d2, 1, 0, NULL) == OMEGA_GPU_ML_BAD_ARGS, "nd overlapping in/out");
    size_t by[3];
    uint32_t n8 = 8;
    check(omega_gpu_fft_plan_walk((OmegaGpuFftPlanMode)3, 1, &n8, 1, 0, by, NULL, NULL) == OMEGA_GPU_ML_BAD_ARGS,
          "plan mode 3");
    check(omega_gpu_fft_plan_walk(OMEGA_GPU_FFT_R2C, 1, &n8, 1, 1, by, NULL, NULL) == OMEGA_GPU_ML_BAD_ARGS,
          "plan r2c inverse 1");
    check(omega_gpu_fft_plan_walk(OMEGA_GPU_FFT_C2R, 2, d2, 1, 0, by, NULL, NULL) == OMEGA_GPU_ML_BAD_ARGS,
          "plan c2r ndim 2");
    check(omega_gpu_fft_plan_walk(OMEGA_GPU_FFT_R2C, 1, &n8, 1, 0, NULL, NULL, NULL) == OMEGA_GPU_ML_BAD_ARGS,
          "plan bytes NULL");
    /* the unit's own refusals */
    BlackwellIRProgram *prog = calloc(1, sizeof *prog);
    uint8_t *code = malloc(OMEGA_BW_CODE_CAP_MAX);
    OmegaBlackwellKernel k;
    const uint64_t refuse[][4] = {
        { 0, 3, 0, 0 },  { 0, 0, 1, 0 },  { 0, 0, 0, 3 },  { 1, 1, 0, 0 },  { 1, 12, 0, 0 }, { 1, 4, 2, 0 },
        { 1, 4, 0, 3 },  { 2, 1, 0, 0 },  { 2, 4, 0, 1 },  { 2, 13, 0, 0 }, { 2, 12, 2, 0 }, { 3, 1, 0, 0 },
        { 3, 0, 1, 0 },  { 3, 0, 0, 3 },  { 4, 2, 0, 0 },  { 4, 13, 0, 0 }, { 4, 4, 2, 0 },  { 4, 4, 0, 3 },
        { 5, 1, 4, 0 },  { 5, 4, 1, 0 },  { 5, 4, 12, 0 }, { 5, 20, 8, 0 }, { 5, 26, 2, 0 }, { 5, 4, 4, 3 },
        { 7, 0, 0, 0 },
    };
    for (size_t i = 0; i < sizeof refuse / sizeof refuse[0]; i++)
        check(build(refuse[i][0], refuse[i][1], refuse[i][2], refuse[i][3], 128, prog, code, &k) == OMEGA_OM_IR_REFUSED,
              "unit accepted kind %" PRIu64 " a %" PRIu64 " b %" PRIu64 " c %" PRIu64, refuse[i][0], refuse[i][1],
              refuse[i][2], refuse[i][3]);
    /* kind 6 (the pass plan) through the data entry: a = mode | ndim << 4, b = log2 dims (4 bits each), c = dir */
    static uint64_t words[OMEGA_OM_IR_WORDS];
    const uint64_t refuse6[][3] = {
        { 3 | 1 << 4, 4, 0 },  { 1 | 1 << 4, 13, 0 }, { 1 | 1 << 4, 2, 0 },        { 1 | 2 << 4, 4 | 4 << 4, 0 },
        { 1 | 1 << 4, 4, 1 },  { 2 | 1 << 4, 12, 1 }, { 0, 0, 0 },                  { 4 << 4, 0x2222, 0 },
        { 1 << 4, 1, 0 },      { 1 << 4, 12, 0 },     { 1 << 4, 4 | 4 << 4, 0 },    { 3 << 4, 11 | 11 << 4 | 6 << 8, 0 },
        { 2 << 4, 4 | 4 << 4, 2 }, { 1 << 4 | 1 << 8, 4, 0 },
    };
    for (size_t i = 0; i < sizeof refuse6 / sizeof refuse6[0]; i++)
        check(omega_gpu_om_ir_data(6, refuse6[i][0], refuse6[i][1], refuse6[i][2], words) == OMEGA_OM_IR_REFUSED,
              "unit planned a %#" PRIx64 " b %#" PRIx64 " c %" PRIu64, refuse6[i][0], refuse6[i][1], refuse6[i][2]);
    free(prog);
    free(code);
}

static void host_codegen(void) {
    OmegaGpuMathInfo a, b;
    uint8_t base_rng[3][32];
    omega_gpu_mathlibs_test_set_mutant(0, 0);
    for (int kind = 0; kind < 3; kind++) {
        int rc = omega_gpu_rng_codegen((OmegaGpuRngKind)kind, &a);
        check(rc == OMEGA_GPU_ML_OK, "rng codegen kind %d rc %d (%s)", kind, rc, omega_gpu_mathlibs_last_error());
        rc = omega_gpu_rng_codegen((OmegaGpuRngKind)kind, &b);
        check(rc == OMEGA_GPU_ML_OK && b.kernel_cache_hit && !memcmp(a.code_digest, b.code_digest, 32),
              "rng kind %d: second codegen not a cache hit with the same digest", kind);
        memcpy(base_rng[kind], a.code_digest, 32);
        receipt("host rng_codegen kind=%d insns=%u gpr_peak=%u gpr_declared=%u", kind, a.insn_count, a.gpr_peak,
                a.gpr_declared);
    }
    uint8_t base_fft[12][2][32];
    for (int lg = 2; lg <= 11; lg++)
        for (int inv = 0; inv < 2; inv++) {
            int rc = omega_gpu_fft_codegen(1u << lg, inv, &a);
            check(rc == OMEGA_GPU_ML_OK, "fft codegen n=%u inv=%d rc %d (%s)", 1u << lg, inv, rc,
                  omega_gpu_mathlibs_last_error());
            check(a.threads_per_cta == (1u << lg) / 4, "fft n=%u threads %u", 1u << lg, a.threads_per_cta);
            memcpy(base_fft[lg][inv], a.code_digest, 32);
            receipt("host fft_codegen n=%u inverse=%d insns=%u gpr_peak=%u gpr_declared=%u", 1u << lg, inv,
                    a.insn_count, a.gpr_peak, a.gpr_declared);
        }
    for (int lg = 2; lg <= 11; lg++)
        check(memcmp(base_fft[lg][0], base_fft[lg][1], 32) != 0, "fft n=%u forward and inverse share code", 1u << lg);
    /* mutants change the code they claim to change, and only that */
    for (int lvl = 1; lvl <= 2; lvl++) {
        omega_gpu_mathlibs_test_set_mutant(1, lvl);
        for (int kind = 0; kind < 3; kind++) {
            check(omega_gpu_rng_codegen((OmegaGpuRngKind)kind, &a) == OMEGA_GPU_ML_OK, "rng mutant %d kind %d", lvl, kind);
            int differs = memcmp(a.code_digest, base_rng[kind], 32) != 0;
            int want = lvl == 1 || kind == OMEGA_GPU_RNG_NORMAL;
            check(differs == want, "rng mutant %d kind %d: digest %s", lvl, kind, differs ? "differs" : "unchanged");
        }
        omega_gpu_mathlibs_test_set_mutant(2, lvl);
        for (int lg = 2; lg <= 11; lg++) {
            check(omega_gpu_fft_codegen(1u << lg, 0, &a) == OMEGA_GPU_ML_OK, "fft mutant %d n=%u", lvl, 1u << lg);
            int differs = memcmp(a.code_digest, base_fft[lg][0], 32) != 0;
            int want = lvl == 1 || lg >= 3;
            check(differs == want, "fft mutant %d n=%u: digest %s", lvl, 1u << lg, differs ? "differs" : "unchanged");
        }
    }
    /* Philox: one kernel; its two mutants change its digest */
    uint8_t base_ph[32];
    int rc = omega_gpu_philox_codegen(&a);
    check(rc == OMEGA_GPU_ML_OK, "philox codegen rc %d (%s)", rc, omega_gpu_mathlibs_last_error());
    rc = omega_gpu_philox_codegen(&b);
    check(rc == OMEGA_GPU_ML_OK && b.kernel_cache_hit && !memcmp(a.code_digest, b.code_digest, 32),
          "philox: second codegen not a cache hit with the same digest");
    check(a.threads_per_cta == OMEGA_GPU_PHILOX_THREADS, "philox threads %u", a.threads_per_cta);
    memcpy(base_ph, a.code_digest, 32);
    receipt("host philox_codegen insns=%u gpr_peak=%u gpr_declared=%u", a.insn_count, a.gpr_peak, a.gpr_declared);
    for (int lvl = 1; lvl <= 2; lvl++) {
        omega_gpu_mathlibs_test_set_mutant(3, lvl);
        check(omega_gpu_philox_codegen(&a) == OMEGA_GPU_ML_OK, "philox mutant %d codegen", lvl);
        check(memcmp(a.code_digest, base_ph, 32) != 0, "philox mutant %d: digest unchanged", lvl);
    }
    omega_gpu_mathlibs_test_set_mutant(0, 0);
    /* split / merge and transpose: built through the unit directly; both mutants change the code at every shape */
    BlackwellIRProgram *prog = calloc(1, sizeof *prog);
    uint8_t *code = malloc(OMEGA_BW_CODE_CAP_MAX), *code0 = malloc(OMEGA_BW_CODE_CAP_MAX);
    OmegaBlackwellKernel k, k0;
    const struct { uint64_t kind, a, b; uint32_t threads; } mk[] = {
        { 4, 3, 0, 2 },     { 4, 3, 1, 2 },     { 4, 6, 0, 16 },  { 4, 6, 1, 16 }, { 4, 12, 0, 1024 },
        { 4, 12, 1, 1024 }, { 5, 2, 2, 256 },  { 5, 4, 3, 256 }, { 5, 25, 2, 256 }, { 5, 16, 11, 256 },
    };
    for (size_t i = 0; i < sizeof mk / sizeof mk[0]; i++) {
        rc = build(mk[i].kind, mk[i].a, mk[i].b, 0, mk[i].threads, prog, code0, &k0);
        check(rc == OMEGA_OM_IR_OK, "build kind %" PRIu64 " a %" PRIu64 " b %" PRIu64 ": %s", mk[i].kind, mk[i].a, mk[i].b,
              omega_gpu_om_ir_rc_name(rc));
        if (rc != OMEGA_OM_IR_OK) continue;
        receipt("host unit_codegen kind=%" PRIu64 " a=%" PRIu64 " b=%" PRIu64 " threads=%u code_bytes=%zu", mk[i].kind,
                mk[i].a, mk[i].b, mk[i].threads, k0.code_size);
        for (uint64_t lvl = 1; lvl <= 2; lvl++) {
            rc = build(mk[i].kind, mk[i].a, mk[i].b, lvl, mk[i].threads, prog, code, &k);
            int differs = rc == OMEGA_OM_IR_OK && (k.code_size != k0.code_size || memcmp(code, code0, k0.code_size) != 0);
            check(differs, "kind %" PRIu64 " a %" PRIu64 " b %" PRIu64 " mutant %" PRIu64 ": rc %s, code %s", mk[i].kind,
                  mk[i].a, mk[i].b, lvl, omega_gpu_om_ir_rc_name(rc), differs ? "differs" : "unchanged");
        }
    }
    free(code0);
    /* our own disassembler finds the instructions the encoder was asked for */
    const struct { uint64_t kind, a, b; uint32_t threads; const char *need[4]; } want[] = {
        { 0, 0, 0, 128, { "STG.E", "+0x4]", "IMAD.WIDE", NULL } },
        { 0, 2, 0, 128, { "MUFU.LG2", "MUFU.RSQ", "I2FP.F32.S32", "LOP3.LUT" } },
        { 1, 11, 1, 512, { "BAR.SYNC", "LDS", "STS", "LDG.E.64" } },
        { 3, 0, 0, 128, { "IMAD.WIDE", "LOP3.LUT", "STG.E", NULL } },
        { 4, 12, 0, 1024, { "LDG.E.64", "STG.E", "FMUL", NULL } },
        { 5, 4, 3, 256, { "LDG.E.64", "STG.E", NULL, NULL } },
    };
    for (size_t i = 0; i < sizeof want / sizeof want[0]; i++) {
        int rc = build(want[i].kind, want[i].a, want[i].b, 0, want[i].threads, prog, code, &k);
        check(rc == OMEGA_OM_IR_OK, "build kind %" PRIu64 " a %" PRIu64 ": %s", want[i].kind, want[i].a,
              omega_gpu_om_ir_rc_name(rc));
        if (rc != OMEGA_OM_IR_OK) continue;
        char *l = listing_of(&k);
        check(l != NULL, "listing of kind %" PRIu64 " a %" PRIu64 " failed", want[i].kind, want[i].a);
        for (int j = 0; l && j < 4 && want[i].need[j]; j++)
            check(strstr(l, want[i].need[j]) != NULL, "kind %" PRIu64 " a %" PRIu64 ": listing lacks %s", want[i].kind,
                  want[i].a, want[i].need[j]);
        free(l);
    }
    free(prog);
    free(code);
}

static void host_twiddles(void) {
    static float tw[2 * 2048];
    size_t bad = 0, total = 0;
    for (int lg = 2; lg <= 11; lg++)
        for (int inv = 0; inv < 2; inv++) {
            uint32_t n = 1u << lg;
            check(omega_gpu_fft_twiddles(n, inv, tw) == OMEGA_GPU_ML_OK, "twiddles n=%u", n);
            for (uint32_t m = 0; m < n; m++) {
                double c = cos(2.0 * M_PI * m / n), s = (inv ? 1.0 : -1.0) * sin(2.0 * M_PI * m / n);
                /* within half an f32 ulp of the libm value (plus the libm error) */
                double tc = tw[2 * m], ts = tw[2 * m + 1];
                double uc = fabs(nextafterf(tw[2 * m], INFINITY) - tw[2 * m]);
                double us = fabs(nextafterf(tw[2 * m + 1], INFINITY) - tw[2 * m + 1]);
                if (fabs(tc - c) > 0.5 * uc + 1e-15 || fabs(ts - s) > 0.5 * us + 1e-15) bad++;
                total++;
            }
        }
    /* n = 4096 (the real transforms' merge and split tables) through the unit's data entry */
    static uint64_t words[OMEGA_OM_IR_WORDS];
    for (int inv = 0; inv < 2; inv++) {
        check(omega_gpu_om_ir_data(2, 12, (uint64_t)inv, 0, words) == OMEGA_OM_IR_OK, "unit twiddles n=4096");
        for (uint32_t m = 0; m < 4096; m++) {
            uint32_t rb = (uint32_t)words[m], ib = (uint32_t)(words[m] >> 32);
            float tc, ts;
            memcpy(&tc, &rb, 4);
            memcpy(&ts, &ib, 4);
            double c = cos(2.0 * M_PI * m / 4096), s = (inv ? 1.0 : -1.0) * sin(2.0 * M_PI * m / 4096);
            double uc = fabs(nextafterf(tc, INFINITY) - tc), us = fabs(nextafterf(ts, INFINITY) - ts);
            if (fabs(tc - c) > 0.5 * uc + 1e-15 || fabs(ts - s) > 0.5 * us + 1e-15) bad++;
            total++;
        }
    }
    check(bad == 0, "%zu of %zu twiddles not the correctly rounded f32 of w^m", bad, total);
    receipt("host fft_twiddles entries=%zu off_by_more_than_half_ulp=%zu", total, bad);
}

static void host_normal_sweep(void) {
    double worst = 0;
    uint32_t worst_a = 0;
    size_t over = 0, central = 0;
    for (uint32_t A = 1; A < 16777216u; A += 2) {
        float z = normal_model(A);
        double ref = normal_ref(A), e = rel_err(z, ref);
        if (!(e <= NORMAL_TOL)) over++;
        if (!(e <= worst)) { worst = e; worst_a = A; }
        float w = -log2f((float)A * (float)(16777216u - A) * 0x1p-46f) * 0.69314718f;
        central += w < 5.0f;
    }
    check(over == 0, "normal f32 model: %zu of 2^23 inputs past %.3g (worst %.4g at A=%u)", over, NORMAL_TOL, worst,
          worst_a);
    receipt("host normal_model_sweep inputs=8388608 over_tol=%zu worst_rel=%.6e at_A=%u tol=%.3g central~%zu", over,
            worst, worst_a, NORMAL_TOL, central);
    /* the f64 reference itself against WolframLanguageEvaluator, 2026-10-27:
     * Table[{A, N[Sqrt[2] InverseErf[(2 A - 2^24)/2^24], 20]}, {A, {1, 8388607, 8388609, 16777215, 12345679, 5000001}}] */
    const struct { uint32_t A; double z; } wl[] = {
        { 1u, -5.2947040848545980574 },
        { 8388607u, -1.4940668789333163667e-7 },
        { 8388609u, 1.4940668789333163667e-7 },
        { 16777215u, 5.2947040848545980574 },
        { 12345679u, 0.63063321267521096414 },
        { 5000001u, -0.53009427681848114495 },
    };
    for (size_t i = 0; i < sizeof wl / sizeof wl[0]; i++) {
        double r = normal_ref(wl[i].A);
        check(fabs(r - wl[i].z) <= 1e-13 * fabs(wl[i].z), "normal_ref(A=%u) = %.17g, Wolfram %.17g", wl[i].A, r, wl[i].z);
    }
}

static void host_philox(void) {
    size_t kat_bad = 0, ctl_bad = 0;
    for (int i = 0; i < 3; i++) {
        uint32_t o[4];
        uint64_t key = PH_KAT[i].key[0] | (uint64_t)PH_KAT[i].key[1] << 32;
        philox_block(PH_KAT[i].ctr, key, 10, o);
        kat_bad += memcmp(o, PH_KAT[i].want, 16) != 0;
        philox_block(PH_KAT[i].ctr, key, 9, o);
        ctl_bad += memcmp(o, PH_KAT[i].want, 16) != 0;
    }
    check(kat_bad == 0, "philox reference: %zu of 3 Random123 KAT vectors differ", kat_bad);
    check(ctl_bad == 3, "negative control: the 9-round reference matches %zu KAT vectors", 3 - ctl_bad);
    receipt("host philox_reference_vs_random123_kat vectors=3 mismatches=%zu nine_round_control_mismatches=%zu", kat_bad,
            ctl_bad);
    size_t bad = 0;
    for (size_t i = 0; i < g_nph; i++)
        bad += philox_ref(g_ph[i].seed, g_ph[i].sub, g_ph[i].off, g_ph[i].k) != g_ph[i].val;
    check(g_nph == 1024, "cuRAND fixture: %zu rows, want 1024", g_nph);
    check(bad == 0, "philox reference: %zu of %zu cuRAND fixture rows differ", bad, g_nph);
    receipt("host philox_reference_vs_curand rows=%zu mismatches=%zu", g_nph, bad);
}

static uint64_t pstep(uint64_t k, uint64_t a, uint64_t b, uint64_t s, uint64_t d, uint64_t r) {
    return k | a << 8 | b << 16 | s << 24 | d << 32 | r << 40;
}
typedef struct { OmegaGpuFftLaunch l[64]; uint32_t n; } PlanLog;
static int plan_record(void *ctx, const OmegaGpuFftLaunch *l) {
    PlanLog *p = ctx;
    if (p->n < 64) p->l[p->n] = *l;
    p->n++;
    return 0;
}

static void host_plans(void) {
    static uint64_t words[OMEGA_OM_IR_WORDS];
    /* the unit's step words: R2C / C2R, 1-D, 2-D {8, 16} forward, 3-D {4, 8, 16} inverse */
    const struct { uint64_t a, b, c; uint32_t n; uint64_t s[6]; } P[] = {
        { 1 | 1 << 4, 12, 0, 2, { pstep(1, 11, 0, 0, 2, 0), pstep(4, 12, 0, 2, 1, 0) } },
        { 2 | 1 << 4, 3, 0, 2, { pstep(4, 3, 1, 0, 2, 0), pstep(1, 2, 1, 2, 1, 0) } },
        { 1 << 4, 8, 1, 1, { pstep(1, 8, 1, 0, 1, 0) } },
        { 2 << 4, 3 | 4 << 4, 0, 4,
          { pstep(1, 4, 0, 0, 2, 3), pstep(5, 3, 4, 2, 1, 0), pstep(1, 3, 0, 1, 2, 4), pstep(5, 4, 3, 2, 1, 0) } },
        { 3 << 4, 2 | 3 << 4 | 4 << 8, 1, 6,
          { pstep(1, 4, 1, 0, 2, 5), pstep(5, 5, 4, 2, 1, 0), pstep(1, 3, 1, 1, 2, 6), pstep(5, 6, 3, 2, 1, 0),
            pstep(1, 2, 1, 1, 2, 7), pstep(5, 7, 2, 2, 1, 0) } },
    };
    size_t bad = 0;
    for (size_t i = 0; i < sizeof P / sizeof P[0]; i++) {
        int rc = omega_gpu_om_ir_data(6, P[i].a, P[i].b, P[i].c, words);
        int ok = rc == OMEGA_OM_IR_OK && words[0] == P[i].n;
        for (uint32_t s = 0; ok && s < P[i].n; s++) ok = words[1 + s] == P[i].s[s];
        bad += !ok;
        check(ok, "unit plan a %#" PRIx64 " b %#" PRIx64 " c %" PRIu64 ": rc %s, %" PRIu64 " steps, step 1 %#" PRIx64,
              P[i].a, P[i].b, P[i].c, omega_gpu_om_ir_rc_name(rc), words[0], words[1]);
    }
    receipt("host fft_plan_words plans=%zu mismatches=%zu", sizeof P / sizeof P[0], bad);
    /* the launches: R2C n 4096 batch 2500 (three chunks per pass) */
    static PlanLog L;
    size_t by[3];
    uint32_t n = 4096;
    memset(&L, 0, sizeof L);
    int rc = omega_gpu_fft_plan_walk(OMEGA_GPU_FFT_R2C, 1, &n, 2500, 0, by, plan_record, &L);
    check(rc == OMEGA_GPU_ML_OK && L.n == 6, "plan r2c 4096 x 2500: rc %d, %u launches", rc, L.n);
    check(by[0] == (size_t)4096 * 4 * 2500 && by[1] == (size_t)2049 * 8 * 2500 && by[2] == by[0],
          "plan r2c 4096 x 2500 bytes %zu %zu %zu", by[0], by[1], by[2]);
    if (L.n == 6) {
        const struct { uint32_t kind, grid, threads, shared, tw_n, src, dst; uint64_t src_off, dst_off; size_t el; } W[6] = {
            { 1, 1024, 512, 16384, 2048, 0, 2, 0, 0, (size_t)2 * 2048 * 1024 },
            { 1, 1024, 512, 16384, 2048, 0, 2, (uint64_t)1024 * 2048 * 8, (uint64_t)1024 * 2048 * 8, (size_t)2 * 2048 * 1024 },
            { 1, 452, 512, 16384, 2048, 0, 2, (uint64_t)2048 * 2048 * 8, (uint64_t)2048 * 2048 * 8, (size_t)2 * 2048 * 452 },
            { 4, 1024, 1024, 0, 4096, 2, 1, 0, 0, (size_t)2 * 2049 * 1024 },
            { 4, 1024, 1024, 0, 4096, 2, 1, (uint64_t)1024 * 2048 * 8, (uint64_t)1024 * 2049 * 8, (size_t)2 * 2049 * 1024 },
            { 4, 452, 1024, 0, 4096, 2, 1, (uint64_t)2048 * 2048 * 8, (uint64_t)2048 * 2049 * 8, (size_t)2 * 2049 * 452 },
        };
        for (int i = 0; i < 6; i++) {
            const OmegaGpuFftLaunch *l = &L.l[i];
            check(l->kind == W[i].kind && l->grid == W[i].grid && l->threads == W[i].threads &&
                      l->shared_bytes == W[i].shared && l->tw_n == W[i].tw_n && l->tw != NULL && l->src == W[i].src &&
                      l->dst == W[i].dst && l->src_off == W[i].src_off && l->dst_off == W[i].dst_off &&
                      l->elements == W[i].el && l->tw_inverse == 0,
                  "plan r2c launch %d: kind %u grid %u threads %u shared %u tw %u src %u+%" PRIu64 " dst %u+%" PRIu64
                  " elements %zu",
                  i, l->kind, l->grid, l->threads, l->shared_bytes, l->tw_n, l->src, l->src_off, l->dst, l->dst_off,
                  l->elements);
        }
    }
    /* 2-D {4, 2048} batch 4: FFT 2048 over 16 rows, transpose, FFT 4 over 8192 rows (8 chunks), transpose */
    const uint32_t d[2] = { 4, 2048 };
    memset(&L, 0, sizeof L);
    rc = omega_gpu_fft_plan_walk(OMEGA_GPU_FFT_C2C_ND, 2, d, 4, 0, by, plan_record, &L);
    check(rc == OMEGA_GPU_ML_OK && L.n == 11, "plan 2-D {4, 2048} x 4: rc %d, %u launches", rc, L.n);
    check(by[0] == (size_t)8 * 8192 * 4 && by[1] == by[0] && by[2] == by[0], "plan 2-D bytes %zu %zu %zu", by[0], by[1],
          by[2]);
    if (L.n == 11) {
        int ok = L.l[0].kind == 1 && L.l[0].a == 11 && L.l[0].grid == 16 && L.l[0].src == 0 && L.l[0].dst == 2;
        ok &= L.l[1].kind == 5 && L.l[1].a == 2 && L.l[1].b == 11 && L.l[1].total == 32768 && L.l[1].first == 0 &&
              L.l[1].grid == 128 && L.l[1].threads == 256 && L.l[1].src == 2 && L.l[1].dst == 1;
        for (int i = 0; i < 8; i++)
            ok &= L.l[2 + i].kind == 1 && L.l[2 + i].a == 2 && L.l[2 + i].grid == 1024 && L.l[2 + i].threads == 1 &&
                  L.l[2 + i].src == 1 && L.l[2 + i].dst == 2 && L.l[2 + i].src_off == (uint64_t)i * 1024 * 4 * 8;
        ok &= L.l[10].kind == 5 && L.l[10].a == 11 && L.l[10].b == 2 && L.l[10].src == 2 && L.l[10].dst == 1;
        check(ok, "plan 2-D {4, 2048} x 4: launch sequence differs");
    }
    /* a transpose past one launch: {16, 2048} batch 16 = 2^19 elements, 2048 CTAs, two launches per transpose */
    const uint32_t d3[2] = { 16, 2048 };
    memset(&L, 0, sizeof L);
    rc = omega_gpu_fft_plan_walk(OMEGA_GPU_FFT_C2C_ND, 2, d3, 16, 1, by, plan_record, &L);
    uint32_t tr = 0, tr_ok = 0;
    for (uint32_t i = 0; i < L.n && i < 64; i++)
        if (L.l[i].kind == 5) {
            tr_ok += L.l[i].grid == 1024 && L.l[i].total == (1u << 19) && L.l[i].first == (tr & 1) * 262144u;
            tr++;
        }
    check(rc == OMEGA_GPU_ML_OK && tr == 4 && tr_ok == 4, "plan 2-D {16, 2048} x 16: rc %d, %u transposes, %u as expected",
          rc, tr, tr_ok);
    receipt("host fft_plan_launches r2c_4096x2500=%u nd_4x2048x4=%u nd_16x2048x16_transposes=%u", 6u, 11u, tr);
}

static void host_tier(void) {
    host_fixture();
    host_philox();
    host_refusals();
    host_codegen();
    host_twiddles();
    host_plans();
    host_normal_sweep();
}

/* ------------------------------------------------------------------ simulator tier */

static BlackwellIRProgram *g_prog;
static uint8_t *g_code;
static WsSim g_sim;

static int sim_prog(uint64_t kind, uint64_t a, uint64_t b, uint64_t mut, uint32_t threads) {
    OmegaBlackwellKernel k;
    int rc = build(kind, a, b, mut, threads, g_prog, g_code, &k);
    check(rc == OMEGA_OM_IR_OK, "sim build kind %" PRIu64 " a %" PRIu64 " b %" PRIu64 " mut %" PRIu64 ": %s", kind, a, b,
          mut, omega_gpu_om_ir_rc_name(rc));
    return rc == OMEGA_OM_IR_OK ? 0 : -1;
}

static void put64(uint32_t *w, uint64_t v) { w[0] = (uint32_t)v; w[1] = (uint32_t)(v >> 32); }

/* the API's argument block (src/omega_gpu_mathlibs_api.c) against host memory */
static int sim_rng(int kind, int mut, uint64_t seed, uint64_t stream0, uint32_t ns, uint32_t offset, uint32_t per,
                   void *out) {
    if (sim_prog(0, (uint64_t)kind, 0, (uint64_t)mut, OMEGA_GPU_RNG_THREADS) != 0) return -1;
    size_t bytes = (size_t)ns * per * (kind == 0 ? 8u : 4u);
    memset(out, 0xa5, bytes);
    memset(&g_sim, 0, sizeof g_sim);
    g_sim.p = g_prog;
    g_sim.threads = OMEGA_GPU_RNG_THREADS;
    uint32_t *cb = &g_sim.cbank[0x380 / 4];
    put64(&cb[0], (uint64_t)(uintptr_t)out);
    uint64_t x = seed + stream0;
    for (int i = 0; i < 4; i++) { x += GOLDEN; put64(&cb[2 + 2 * i], x); }
    cb[10] = offset; cb[11] = per; cb[12] = ns;
    g_sim.ranges[0] = (WsRange){ (uintptr_t)out, (uintptr_t)out + bytes };
    g_sim.nranges = 1;
    int rc = ws_run(&g_sim, (ns + OMEGA_GPU_RNG_THREADS - 1) / OMEGA_GPU_RNG_THREADS, 1);
    check(rc == 0, "sim rng kind %d: %s", kind, g_sim.msg);
    return rc;
}

static int sim_fft(uint32_t n, int inverse, int mut, uint32_t sched, const float *in, float *out, uint32_t batch) {
    int lg = lg2u(n);
    if (sim_prog(1, (uint64_t)lg, (uint64_t)inverse, (uint64_t)mut, n / 4) != 0) return -1;
    static float tw[2 * 2048];
    static uint8_t smem[8 * 2048];
    if (omega_gpu_fft_twiddles(n, inverse, tw) != OMEGA_GPU_ML_OK) return -1;
    memset(out, 0xa5, (size_t)8 * n * batch);
    memset(&g_sim, 0, sizeof g_sim);
    g_sim.p = g_prog;
    g_sim.threads = n / 4;
    g_sim.smem = smem;
    g_sim.smem_bytes = 8 * n;
    g_sim.warp_sched = sched;
    uint32_t *cb = &g_sim.cbank[0x380 / 4];
    put64(&cb[0], (uint64_t)(uintptr_t)in);
    put64(&cb[2], (uint64_t)(uintptr_t)out);
    put64(&cb[4], (uint64_t)(uintptr_t)tw);
    g_sim.ranges[0] = (WsRange){ (uintptr_t)in, (uintptr_t)in + (size_t)8 * n * batch };
    g_sim.ranges[1] = (WsRange){ (uintptr_t)out, (uintptr_t)out + (size_t)8 * n * batch };
    g_sim.ranges[2] = (WsRange){ (uintptr_t)tw, (uintptr_t)tw + (size_t)8 * n };
    g_sim.nranges = 3;
    int rc = ws_run(&g_sim, batch, 1);
    check(rc == 0, "sim fft n=%u inv=%d mut=%d: %s", n, inverse, mut, g_sim.msg);
    return rc;
}

static void sim_rng_tier(void) {
    const uint32_t ns = 130; /* two CTAs, the second with 2 live threads and 126 that exit */
    static uint64_t got[130 * 17], ref[130 * 17];
    static float fgot[130 * 17];
    const struct { uint64_t seed, stream0; uint32_t offset, per; } runs[] = {
        { 0, 0, 0, 17 }, { 0, 0, 1000, 16 }, { 0xFFFFFFFFFFFFFFFCull, 0, 0, 17 }, { 0xFFFFFFFFFFFFFFFCull, 0, 1000, 16 },
        { 1729, 0, 0, 17 }, { 1729, 0xFFFFFFFFFFFFFF80ull, 3, 5 },
    };
    size_t fix_used = 0, fix_bad = 0;
    memset(g_fix_hit, 0, sizeof g_fix_hit);
    for (size_t r = 0; r < sizeof runs / sizeof runs[0]; r++) {
        uint32_t cnt = ns * runs[r].per;
        ref_raw(runs[r].seed, runs[r].stream0, ns, runs[r].offset, runs[r].per, ref);
        if (sim_rng(0, 0, runs[r].seed, runs[r].stream0, ns, runs[r].offset, runs[r].per, got) == 0) {
            size_t bad = 0;
            for (uint32_t i = 0; i < cnt; i++) bad += got[i] != ref[i];
            check(bad == 0, "sim raw seed %" PRIu64 " offset %u: %zu of %u differ", runs[r].seed, runs[r].offset, bad, cnt);
            if (runs[r].stream0 == 0) fix_used += fixture_compare(got, runs[r].seed, ns, runs[r].offset, runs[r].per, &fix_bad);
            receipt("sim rng_raw seed=%" PRIu64 " stream0=%" PRIu64 " ns=%u offset=%u per=%u mismatches=%zu", runs[r].seed,
                    runs[r].stream0, ns, runs[r].offset, runs[r].per, bad);
        }
        if (runs[r].offset == 1000) continue; /* uniform and normal share the skip loop */
        if (sim_rng(1, 0, runs[r].seed, runs[r].stream0, ns, runs[r].offset, runs[r].per, fgot) == 0) {
            size_t bad = 0;
            for (uint32_t i = 0; i < cnt; i++) bad += f2u(fgot[i]) != f2u(uniform_of(ref[i]));
            check(bad == 0, "sim uniform seed %" PRIu64 ": %zu of %u differ", runs[r].seed, bad, cnt);
            receipt("sim rng_uniform seed=%" PRIu64 " n=%u mismatches=%zu", runs[r].seed, cnt, bad);
        }
        if (sim_rng(2, 0, runs[r].seed, runs[r].stream0, ns, runs[r].offset, runs[r].per, fgot) == 0) {
            size_t bad = 0, over = 0;
            double worst = 0;
            for (uint32_t i = 0; i < cnt; i++) {
                uint32_t A = normal_A(ref[i]);
                bad += f2u(fgot[i]) != f2u(normal_model(A));
                double e = rel_err(fgot[i], normal_ref(A));
                over += !(e <= NORMAL_TOL);
                if (!(e <= worst)) worst = e;
            }
            check(bad == 0, "sim normal seed %" PRIu64 ": %zu of %u differ from the f32 model", runs[r].seed, bad, cnt);
            check(over == 0, "sim normal seed %" PRIu64 ": %zu of %u past the bound", runs[r].seed, over, cnt);
            receipt("sim rng_normal seed=%" PRIu64 " n=%u model_mismatches=%zu over_tol=%zu worst_rel=%.4e", runs[r].seed,
                    cnt, bad, over, worst);
        }
    }
    check(fix_used > 0 && fix_bad == 0, "sim: %zu of %zu fixture comparisons differ", fix_bad, fix_used);
    receipt("sim rng_fixture rows_covered=%zu of %zu comparisons=%zu mismatches=%zu", fixture_distinct(), g_nfix,
            fix_used, fix_bad);
    /* mutants: rotation 8 breaks the raw stream; GC0 = 1.5 breaks the normal bound */
    ref_raw(0, 0, ns, 0, 17, ref);
    if (sim_rng(0, 1, 0, 0, ns, 0, 17, got) == 0) {
        size_t bad = 0;
        for (uint32_t i = 0; i < ns * 17; i++) bad += got[i] != ref[i];
        check(bad > 0, "sim rng mutant 1 not caught");
        receipt("sim rng_mutant1 mismatches=%zu of %u (must be > 0)", bad, ns * 17);
    }
    if (sim_rng(2, 2, 0, 0, ns, 0, 17, fgot) == 0) {
        size_t over = 0;
        for (uint32_t i = 0; i < ns * 17; i++) over += !(rel_err(fgot[i], normal_ref(normal_A(ref[i]))) <= NORMAL_TOL);
        check(over > 0, "sim rng mutant 2 not caught");
        receipt("sim rng_mutant2 over_tol=%zu of %u (must be > 0)", over, ns * 17);
    }
}

static void sim_fft_tier(void) {
    const uint32_t batch = 3;
    float *in = malloc((size_t)8 * 2048 * batch), *out = malloc((size_t)8 * 2048 * batch),
          *out2 = malloc((size_t)8 * 2048 * batch);
    const uint32_t sched[3] = { 0, 1, 0x9E3779B9u };
    for (int lg = 2; lg <= 11; lg++)
        for (int inv = 0; inv < 2; inv++) {
            uint32_t n = 1u << lg;
            fill_rand(in, (size_t)2 * n * batch);
            double e = -1;
            int same = 1;
            for (int s = 0; s < 3; s++) {
                if (sim_fft(n, inv, 0, sched[s], in, s ? out2 : out, batch) != 0) { same = 0; break; }
                if (s == 0) e = fft_err(in, out, n, batch, inv);
                else same &= !memcmp(out, out2, (size_t)8 * n * batch);
            }
            check(e >= 0 && e <= FFT_TOL[lg], "sim fft n=%u inv=%d: error %.4e, tolerance %.4e", n, inv, e, FFT_TOL[lg]);
            check(same, "sim fft n=%u inv=%d: output depends on the warp order", n, inv);
            receipt("sim fft n=%u inverse=%d batch=%u normwise_err=%.4e tol=%.4e warp_orders_identical=%d", n, inv, batch, e,
                    FFT_TOL[lg], same);
        }
    /* mutants: direction flip at every N, dropped twiddles at N >= 8 (N = 4 has no twiddled stage) */
    const struct { uint32_t n; int mut; int caught; } mm[] = { { 4, 1, 1 }, { 64, 1, 1 }, { 2048, 1, 1 },
                                                              { 8, 2, 1 }, { 16, 2, 1 }, { 2048, 2, 1 }, { 4, 2, 0 } };
    for (size_t i = 0; i < sizeof mm / sizeof mm[0]; i++) {
        uint32_t n = mm[i].n;
        int lg = lg2u(n);
        fill_rand(in, (size_t)2 * n * batch);
        if (sim_fft(n, 0, mm[i].mut, 0, in, out, batch) != 0) continue;
        double e = fft_err(in, out, n, batch, 0);
        int caught = !(e <= FFT_TOL[lg]);
        check(caught == mm[i].caught, "sim fft mutant %d n=%u: error %.4e, %s", mm[i].mut, n, e,
              caught ? "caught" : "not caught");
        receipt("sim fft_mutant%d n=%u normwise_err=%.4e caught=%d expected=%d", mm[i].mut, n, e, caught, mm[i].caught);
    }
    free(in); free(out); free(out2);
}

/* Philox through the API's argument block: out, key, subseq0, first block, blocks per stream, streams */
static int sim_philox(int mut, uint64_t key, uint64_t sub0, uint64_t blk0, uint32_t nblk, uint32_t ns, uint32_t *out) {
    if (sim_prog(3, 0, 0, (uint64_t)mut, OMEGA_GPU_PHILOX_THREADS) != 0) return -1;
    size_t bytes = (size_t)ns * nblk * 16;
    memset(out, 0xa5, bytes);
    memset(&g_sim, 0, sizeof g_sim);
    g_sim.p = g_prog;
    g_sim.threads = OMEGA_GPU_PHILOX_THREADS;
    uint32_t *cb = &g_sim.cbank[0x380 / 4];
    put64(&cb[0], (uint64_t)(uintptr_t)out);
    put64(&cb[2], key);
    put64(&cb[4], sub0);
    put64(&cb[6], blk0);
    cb[8] = nblk;
    cb[9] = ns;
    g_sim.ranges[0] = (WsRange){ (uintptr_t)out, (uintptr_t)out + bytes };
    g_sim.nranges = 1;
    int rc = ws_run(&g_sim, (ns + OMEGA_GPU_PHILOX_THREADS - 1) / OMEGA_GPU_PHILOX_THREADS, 1);
    check(rc == 0, "sim philox mut=%d: %s", mut, g_sim.msg);
    return rc;
}

static size_t diff_words(const uint32_t *a, const uint32_t *b, size_t n) {
    size_t bad = 0;
    for (size_t i = 0; i < n; i++) bad += a[i] != b[i];
    return bad;
}

static void sim_philox_tier(void) {
    static uint32_t got[130 * 68], ref[130 * 68];
    /* the three KAT vectors through the argument block: ctr = (block lo, block hi, subseq lo, subseq hi) */
    size_t kat_bad = 0;
    for (int i = 0; i < 3; i++) {
        const uint32_t *c = PH_KAT[i].ctr;
        uint64_t key = PH_KAT[i].key[0] | (uint64_t)PH_KAT[i].key[1] << 32;
        if (sim_philox(0, key, c[2] | (uint64_t)c[3] << 32, c[0] | (uint64_t)c[1] << 32, 1, 1, got) != 0) { kat_bad++; continue; }
        kat_bad += memcmp(got, PH_KAT[i].want, 16) != 0;
    }
    check(kat_bad == 0, "sim philox: %zu of 3 KAT vectors differ", kat_bad);
    receipt("sim philox_kat vectors=3 mismatches=%zu", kat_bad);
    /* 4 seeds x 4 offsets x (130 streams from subseq 0, 2 streams across subseq 2^32), 8 words each */
    size_t bad = 0, words = 0, used = 0, fbad = 0;
    memset(g_ph_hit, 0, sizeof g_ph_hit);
    for (int s = 0; s < 4; s++)
        for (int o = 0; o < 4; o++)
            for (int r = 0; r < 2; r++) {
                uint64_t sub0 = r ? 0xFFFFFFFFull : 0;
                uint32_t ns = r ? 2 : 130;
                ref_philox(PH_SEEDS[s], sub0, ns, PH_OFFS[o], 8, ref);
                if (sim_philox(0, PH_SEEDS[s], sub0, PH_OFFS[o] / 4, 2, ns, got) != 0) { bad++; continue; }
                bad += diff_words(got, ref, (size_t)ns * 8);
                words += (size_t)ns * 8;
                used += ph_compare(got, PH_SEEDS[s], sub0, ns, PH_OFFS[o], 8, &fbad);
            }
    check(bad == 0, "sim philox streams: %zu of %zu words differ from the reference", bad, words);
    check(ph_distinct() == g_nph && g_nph == 1024 && fbad == 0,
          "sim philox cuRAND fixture: %zu of %zu rows covered, %zu of %zu comparisons differ", ph_distinct(), g_nph, fbad,
          used);
    receipt("sim philox_streams runs=32 words=%zu mismatches=%zu", words, bad);
    receipt("sim philox_curand_fixture rows_covered=%zu of %zu comparisons=%zu mismatches=%zu", ph_distinct(), g_nph,
            used, fbad);
    /* 17 blocks per stream; and the block counter carrying from its low word into its high word */
    ref_philox(1729, 5, 130, 1000, 68, ref);
    bad = sim_philox(0, 1729, 5, 250, 17, 130, got) == 0 ? diff_words(got, ref, (size_t)130 * 68) : 1;
    check(bad == 0, "sim philox 17 blocks: %zu differ", bad);
    receipt("sim philox seed=1729 subseq0=5 ns=130 offset=1000 per=68 mismatches=%zu", bad);
    ref_philox(7, 0, 3, 0x3FFFFFFF4ull, 32, ref);
    bad = sim_philox(0, 7, 0, 0xFFFFFFFDull, 8, 3, got) == 0 ? diff_words(got, ref, (size_t)3 * 32) : 1;
    check(bad == 0, "sim philox block carry: %zu differ", bad);
    receipt("sim philox seed=7 ns=3 offset=4*(2^32-3) per=32 mismatches=%zu", bad);
    /* mutants: key Weyl step W0 for the second word, nine rounds */
    ref_philox(0x123456789abcdefull, 0, 130, 0, 8, ref);
    for (int mut = 1; mut <= 2; mut++) {
        size_t mb = sim_philox(mut, 0x123456789abcdefull, 0, 0, 2, 130, got) == 0 ? diff_words(got, ref, (size_t)130 * 8) : 0;
        check(mb > 0, "sim philox mutant %d not caught", mut);
        receipt("sim philox_mutant%d mismatches=%zu of %u (must be > 0)", mut, mb, 130u * 8);
    }
}

/* the API's plan walked on the simulator: every launch of omega_gpu_fft_plan_walk built by the unit and run */
typedef struct { void *buf[3]; size_t bytes[3]; uint32_t mkind; int mlvl; uint32_t sched, launches; } SimPlan;
static int sim_launch(void *ctx, const OmegaGpuFftLaunch *l) {
    SimPlan *r = ctx;
    static uint8_t smem[8 * 2048];
    uint64_t mut = l->kind == r->mkind ? (uint64_t)r->mlvl : 0;
    if (l->shared_bytes > sizeof smem) { check(0, "sim plan: %u shared bytes", l->shared_bytes); return -1; }
    if (sim_prog(l->kind, l->a, l->b, mut, l->threads) != 0) return -1;
    memset(&g_sim, 0, sizeof g_sim);
    g_sim.p = g_prog;
    g_sim.threads = l->threads;
    g_sim.warp_sched = r->sched;
    if (l->shared_bytes) { g_sim.smem = smem; g_sim.smem_bytes = l->shared_bytes; }
    uint32_t *cb = &g_sim.cbank[0x380 / 4];
    put64(&cb[0], (uint64_t)(uintptr_t)r->buf[l->src] + l->src_off);
    put64(&cb[2], (uint64_t)(uintptr_t)r->buf[l->dst] + l->dst_off);
    if (l->kind == 5) { cb[4] = l->total; cb[5] = l->first; }
    else put64(&cb[4], (uint64_t)(uintptr_t)l->tw);
    for (int i = 0; i < 3; i++)
        g_sim.ranges[i] = (WsRange){ (uintptr_t)r->buf[i], (uintptr_t)r->buf[i] + r->bytes[i] };
    g_sim.nranges = 3;
    if (l->tw_n)
        g_sim.ranges[g_sim.nranges++] = (WsRange){ (uintptr_t)l->tw, (uintptr_t)l->tw + (size_t)8 * l->tw_n };
    int rc = ws_run(&g_sim, l->grid, 1);
    r->launches++;
    check(rc == 0, "sim plan launch kind %u a %u b %u grid %u: %s", l->kind, l->a, l->b, l->grid, g_sim.msg);
    return rc == 0 ? 0 : -1;
}
/* out and scratch start as 0xff bytes (a NaN); *unwritten counts the out words still holding it */
static int sim_plan(OmegaGpuFftPlanMode mode, uint32_t nd, const uint32_t *dims, uint32_t batch, int inverse,
                    const float *in, float *out, uint32_t mkind, int mlvl, uint32_t sched, size_t *unwritten) {
    SimPlan r;
    memset(&r, 0, sizeof r);
    int rc = omega_gpu_fft_plan_walk(mode, nd, dims, batch, inverse, r.bytes, NULL, NULL);
    check(rc == OMEGA_GPU_ML_OK, "sim plan mode %d ndim %u: rc %d (%s)", (int)mode, nd, rc, omega_gpu_mathlibs_last_error());
    if (rc != OMEGA_GPU_ML_OK) return -1;
    float *scr = malloc(r.bytes[2]);
    memset(out, 0xff, r.bytes[1]);
    memset(scr, 0xff, r.bytes[2]);
    r.buf[0] = (void *)(uintptr_t)in;
    r.buf[1] = out;
    r.buf[2] = scr;
    r.mkind = mkind;
    r.mlvl = mlvl;
    r.sched = sched;
    rc = omega_gpu_fft_plan_walk(mode, nd, dims, batch, inverse, r.bytes, sim_launch, &r);
    check(rc == 0, "sim plan mode %d ndim %u: walk rc %d", (int)mode, nd, rc);
    size_t u = 0;
    const uint32_t *w = (const uint32_t *)out;
    for (size_t i = 0; i < r.bytes[1] / 4; i++) u += w[i] == 0xffffffffu;
    *unwritten = u;
    free(scr);
    return rc == 0 ? 0 : -1;
}

static void hermitian(float *x, uint32_t n, uint32_t batch) {
    for (uint32_t b = 0; b < batch; b++) { x[(size_t)(n + 2) * b + 1] = 0; x[(size_t)(n + 2) * b + n + 1] = 0; }
}

static void sim_real_tier(void) {
    const uint32_t batch = 3;
    size_t cap = (size_t)(4096 + 2) * batch;
    float *in = malloc(cap * 4), *out = malloc(cap * 4), *out2 = malloc(cap * 4);
    const uint32_t sched[3] = { 0, 1, 0x9E3779B9u };
    for (int lg = 3; lg <= 12; lg++)
        for (int c2r = 0; c2r < 2; c2r++) {
            uint32_t n = 1u << lg;
            size_t nin = c2r ? (size_t)(n + 2) * batch : (size_t)n * batch;
            size_t nout = c2r ? (size_t)n * batch : (size_t)(n + 2) * batch;
            OmegaGpuFftPlanMode mode = c2r ? OMEGA_GPU_FFT_C2R : OMEGA_GPU_FFT_R2C;
            fill_rand(in, nin);
            if (c2r) hermitian(in, n, batch);
            double e = -1;
            int same = 1;
            size_t unw = 0, u;
            for (int s = 0; s < 3; s++) {
                if (sim_plan(mode, 1, &n, batch, 0, in, s ? out2 : out, 0, 0, sched[s], &u) != 0) { same = 0; break; }
                unw += u;
                if (s == 0) e = c2r ? c2r_err(in, out, n, batch) : r2c_err(in, out, n, batch);
                else same &= !memcmp(out, out2, nout * 4);
            }
            double tol = c2r ? C2R_TOL[lg] : R2C_TOL[lg];
            const char *nm = c2r ? "c2r" : "r2c";
            check(e >= 0 && e <= tol, "sim %s n=%u: error %.4e, tolerance %.4e", nm, n, e, tol);
            check(same, "sim %s n=%u: output depends on the warp order", nm, n);
            check(unw == 0, "sim %s n=%u: %zu words unwritten", nm, n, unw);
            receipt("sim %s n=%u batch=%u normwise_err=%.4e tol=%.4e warp_orders_identical=%d unwritten=%zu", nm, n, batch,
                    e, tol, same, unw);
        }
    /* mutants of the split (R2C) and merge (C2R) pass: the dropped twiddle, the skipped bin n / 4 */
    const struct { int c2r; uint32_t n; int mut; } mm[] = { { 0, 64, 1 }, { 0, 8, 2 },  { 0, 4096, 2 },
                                                           { 1, 64, 1 }, { 1, 8, 2 },  { 1, 4096, 2 } };
    for (size_t i = 0; i < sizeof mm / sizeof mm[0]; i++) {
        uint32_t n = mm[i].n;
        int lg = lg2u(n), c2r = mm[i].c2r;
        size_t u;
        fill_rand(in, c2r ? (size_t)(n + 2) * batch : (size_t)n * batch);
        if (c2r) hermitian(in, n, batch);
        if (sim_plan(c2r ? OMEGA_GPU_FFT_C2R : OMEGA_GPU_FFT_R2C, 1, &n, batch, 0, in, out, 4, mm[i].mut, 0, &u) != 0)
            continue;
        double e = c2r ? c2r_err(in, out, n, batch) : r2c_err(in, out, n, batch);
        double tol = c2r ? C2R_TOL[lg] : R2C_TOL[lg];
        int caught = !(e <= tol);
        check(caught, "sim %s mutant %d n=%u: error %.4e not caught", c2r ? "c2r" : "r2c", mm[i].mut, n, e);
        receipt("sim %s_mutant%d n=%u normwise_err=%.4e unwritten=%zu caught=%d", c2r ? "c2r" : "r2c", mm[i].mut, n, e, u,
                caught);
    }
    free(in); free(out); free(out2);
}

static void sim_nd_tier(void) {
    const uint32_t batch = 2;
    size_t cap = (size_t)2 * 32768 * batch;
    float *in = malloc(cap * 4), *out = malloc(cap * 4), *out2 = malloc(cap * 4);
    const uint32_t sched[3] = { 0, 1, 0x9E3779B9u };
    for (size_t i = 0; i < N_ND_SHAPES; i++)
        for (int inv = 0; inv < 2; inv++) {
            const uint32_t *d = ND_SHAPES[i].d;
            uint32_t nd = ND_SHAPES[i].nd;
            size_t tot = (size_t)d[0] * (nd > 1 ? d[1] : 1) * (nd > 2 ? d[2] : 1), nf = 2 * tot * batch;
            fill_rand(in, nf);
            double e = -1;
            int same = 1;
            size_t unw = 0, u;
            for (int s = 0; s < 3; s++) {
                if (sim_plan(OMEGA_GPU_FFT_C2C_ND, nd, d, batch, inv, in, s ? out2 : out, 0, 0, sched[s], &u) != 0) {
                    same = 0;
                    break;
                }
                unw += u;
                if (s == 0) e = nd_err(in, out, nd, d, batch, inv);
                else same &= !memcmp(out, out2, nf * 4);
            }
            double tol = ND_SHAPES[i].tol;
            check(e >= 0 && e <= tol, "sim nd {%u %u %u} inv=%d: error %.4e, tolerance %.4e", d[0], d[1], d[2], inv, e, tol);
            check(same, "sim nd {%u %u %u} inv=%d: output depends on the warp order", d[0], d[1], d[2], inv);
            check(unw == 0, "sim nd {%u %u %u} inv=%d: %zu words unwritten", d[0], d[1], d[2], inv, unw);
            receipt("sim nd ndim=%u dims=%u,%u,%u inverse=%d batch=%u normwise_err=%.4e tol=%.4e warp_orders_identical=%d",
                    nd, d[0], d[1], d[2], inv, batch, e, tol, same);
        }
    /* mutants: the transpose's identity copy and its dropped batch index (invisible with one transform: the
     * expected miss), the complex pass's direction flip through a 3-D plan */
    const struct { uint32_t nd, d[3], batch, kind; int mut, caught; double tol; } mm[] = {
        { 2, { 8, 16, 0 }, 2, 5, 1, 1, 9.928263778e-7 },
        { 2, { 8, 16, 0 }, 2, 5, 2, 1, 9.928263778e-7 },
        { 2, { 8, 16, 0 }, 1, 5, 2, 0, 9.928263778e-7 },
        { 3, { 4, 8, 16 }, 2, 1, 1, 1, 1.112035792e-6 },
    };
    for (size_t i = 0; i < sizeof mm / sizeof mm[0]; i++) {
        size_t tot = (size_t)mm[i].d[0] * mm[i].d[1] * (mm[i].nd > 2 ? mm[i].d[2] : 1), u;
        fill_rand(in, 2 * tot * mm[i].batch);
        if (sim_plan(OMEGA_GPU_FFT_C2C_ND, mm[i].nd, mm[i].d, mm[i].batch, 0, in, out, mm[i].kind, mm[i].mut, 0, &u) != 0)
            continue;
        double e = nd_err(in, out, mm[i].nd, mm[i].d, mm[i].batch, 0);
        int caught = !(e <= mm[i].tol);
        check(caught == mm[i].caught, "sim nd mutant kind %u level %d batch %u: error %.4e, %s", mm[i].kind, mm[i].mut,
              mm[i].batch, e, caught ? "caught" : "not caught");
        receipt("sim nd_mutant kind=%u level=%d ndim=%u batch=%u normwise_err=%.4e caught=%d expected=%d", mm[i].kind,
                mm[i].mut, mm[i].nd, mm[i].batch, e, caught, mm[i].caught);
    }
    free(in); free(out); free(out2);
}

static void sim_tier(void) {
    g_prog = calloc(1, sizeof *g_prog);
    g_code = malloc(OMEGA_BW_CODE_CAP_MAX);
    sim_rng_tier();
    sim_fft_tier();
    sim_philox_tier();
    sim_real_tier();
    sim_nd_tier();
    free(g_prog);
    free(g_code);
}

/* ------------------------------------------------------------------ chip tier */

static void chip_rng(void) {
    const uint32_t ns = 4096;
    uint64_t *got = malloc((size_t)ns * 17 * 8), *ref = malloc((size_t)ns * 17 * 8);
    float *fgot = malloc((size_t)ns * 17 * 4);
    OmegaGpuMathInfo info;
    size_t fix_used = 0, fix_bad = 0;
    memset(g_fix_hit, 0, sizeof g_fix_hit);
    for (int b = 0; b < 3; b++)
        for (int o = 0; o < 2; o++) {
            uint32_t offset = o ? 1000 : 0, per = o ? 16 : 17, cnt = ns * per;
            ref_raw(BASES[b], 0, ns, offset, per, ref);
            int rc = omega_gpu_rng_generate(OMEGA_GPU_RNG_U64, BASES[b], 0, ns, offset, per, got, &info);
            check(rc == OMEGA_GPU_ML_OK, "chip raw base %" PRIu64 " offset %u: rc %d (%s)", BASES[b], offset, rc,
                  omega_gpu_mathlibs_last_error());
            if (rc != OMEGA_GPU_ML_OK) continue;
            size_t bad = 0;
            for (uint32_t i = 0; i < cnt; i++) bad += got[i] != ref[i];
            fix_used += fixture_compare(got, BASES[b], ns, offset, per, &fix_bad);
            check(bad == 0, "chip raw base %" PRIu64 " offset %u: %zu of %u differ", BASES[b], offset, bad, cnt);
            receipt("chip rng_raw base=%" PRIu64 " ns=%u offset=%u per=%u mismatches=%zu unwritten=%" PRIu64
                    " elapsed_ns=%" PRIu64 " marker=%u",
                    BASES[b], ns, offset, per, bad, info.unwritten_words, info.elapsed_ns, info.completion_marker);
        }
    size_t fix_distinct = fixture_distinct();
    check(fix_distinct == g_nfix && fix_bad == 0, "chip fixture: %zu of %zu rows covered, %zu of %zu comparisons differ",
          fix_distinct, g_nfix, fix_bad, fix_used);
    receipt("chip rng_fixture rows_covered=%zu of %zu comparisons=%zu mismatches=%zu", fix_distinct, g_nfix, fix_used,
            fix_bad);
    /* a wrapping stream0 */
    ref_raw(0x123, 0xFFFFFFFFFFFFFF9Cull, 300, 7, 5, ref);
    if (omega_gpu_rng_generate(OMEGA_GPU_RNG_U64, 0x123, 0xFFFFFFFFFFFFFF9Cull, 300, 7, 5, got, &info) == OMEGA_GPU_ML_OK) {
        size_t bad = 0;
        for (uint32_t i = 0; i < 1500; i++) bad += got[i] != ref[i];
        check(bad == 0, "chip raw wrapping stream0: %zu differ", bad);
        receipt("chip rng_raw seed=291 stream0=2^64-100 ns=300 offset=7 per=5 mismatches=%zu", bad);
    } else check(0, "chip raw wrapping stream0 failed (%s)", omega_gpu_mathlibs_last_error());
    /* uniform and normal at base 1729 */
    ref_raw(1729, 0, ns, 0, 17, ref);
    if (omega_gpu_rng_generate(OMEGA_GPU_RNG_UNIFORM, 1729, 0, ns, 0, 17, fgot, &info) == OMEGA_GPU_ML_OK) {
        size_t bad = 0;
        for (uint32_t i = 0; i < ns * 17; i++) bad += f2u(fgot[i]) != f2u(uniform_of(ref[i]));
        check(bad == 0, "chip uniform: %zu differ", bad);
        receipt("chip rng_uniform base=1729 n=%u mismatches=%zu", ns * 17, bad);
    } else check(0, "chip uniform failed (%s)", omega_gpu_mathlibs_last_error());
    if (omega_gpu_rng_generate(OMEGA_GPU_RNG_NORMAL, 1729, 0, ns, 0, 17, fgot, &info) == OMEGA_GPU_ML_OK) {
        size_t over = 0, differ = 0;
        double worst = 0;
        for (uint32_t i = 0; i < ns * 17; i++) {
            uint32_t A = normal_A(ref[i]);
            double e = rel_err(fgot[i], normal_ref(A));
            over += !(e <= NORMAL_TOL);
            if (!(e <= worst)) worst = e;
            differ += f2u(fgot[i]) != f2u(normal_model(A));
        }
        check(over == 0, "chip normal: %zu past the bound (worst %.4e)", over, worst);
        receipt("chip rng_normal base=1729 n=%u over_tol=%zu worst_rel=%.6e tol=%.3g differ_from_exact_mufu_model=%zu",
                ns * 17, over, worst, NORMAL_TOL, differ);
    } else check(0, "chip normal failed (%s)", omega_gpu_mathlibs_last_error());
    free(got); free(ref); free(fgot);
    /* the full launch: 131072 streams x 64 normals, and the same as uniforms */
    const uint32_t big = OMEGA_GPU_RNG_MAX_STREAMS, per = 64;
    size_t cnt = (size_t)big * per;
    uint64_t *r2 = malloc(cnt * 8);
    float *f2 = malloc(cnt * 4);
    ref_raw(42, 0, big, 0, per, r2);
    for (int kind = 1; kind <= 2; kind++) {
        int rc = omega_gpu_rng_generate((OmegaGpuRngKind)kind, 42, 0, big, 0, per, f2, &info);
        check(rc == OMEGA_GPU_ML_OK, "chip big kind %d rc %d (%s)", kind, rc, omega_gpu_mathlibs_last_error());
        if (rc != OMEGA_GPU_ML_OK) continue;
        size_t bad = 0;
        double worst = 0;
        for (size_t i = 0; i < cnt; i++) {
            if (kind == 1) { bad += f2u(f2[i]) != f2u(uniform_of(r2[i])); continue; }
            double e = rel_err(f2[i], normal_ref(normal_A(r2[i])));
            bad += !(e <= NORMAL_TOL);
            if (!(e <= worst)) worst = e;
        }
        check(bad == 0, "chip big kind %d: %zu bad", kind, bad);
        receipt("chip rng_big kind=%d ns=%u per=%u n=%zu bad=%zu worst_rel=%.6e elapsed_ns=%" PRIu64 " ctas=%u marker=%u",
                kind, big, per, cnt, bad, worst, info.elapsed_ns, info.ctas_last_launch, info.completion_marker);
    }
    free(r2); free(f2);
    /* mutants */
    uint64_t *mg = malloc(256 * 17 * 8), *mr = malloc(256 * 17 * 8);
    float *mf = malloc(256 * 17 * 4);
    ref_raw(0, 0, 256, 0, 17, mr);
    omega_gpu_mathlibs_test_set_mutant(1, 1);
    if (omega_gpu_rng_generate(OMEGA_GPU_RNG_U64, 0, 0, 256, 0, 17, mg, NULL) == OMEGA_GPU_ML_OK) {
        size_t bad = 0;
        for (uint32_t i = 0; i < 256 * 17; i++) bad += mg[i] != mr[i];
        check(bad > 0, "chip rng mutant 1 not caught");
        receipt("chip rng_mutant1 mismatches=%zu of %u (must be > 0)", bad, 256 * 17);
    } else check(0, "chip rng mutant 1 run failed (%s)", omega_gpu_mathlibs_last_error());
    omega_gpu_mathlibs_test_set_mutant(1, 2);
    if (omega_gpu_rng_generate(OMEGA_GPU_RNG_NORMAL, 0, 0, 256, 0, 17, mf, NULL) == OMEGA_GPU_ML_OK) {
        size_t over = 0;
        for (uint32_t i = 0; i < 256 * 17; i++) over += !(rel_err(mf[i], normal_ref(normal_A(mr[i]))) <= NORMAL_TOL);
        check(over > 0, "chip rng mutant 2 not caught");
        receipt("chip rng_mutant2 over_tol=%zu of %u (must be > 0)", over, 256 * 17);
    } else check(0, "chip rng mutant 2 run failed (%s)", omega_gpu_mathlibs_last_error());
    omega_gpu_mathlibs_test_set_mutant(0, 0);
    free(mg); free(mr); free(mf);
}

static void chip_fft(void) {
    const uint32_t batch = 8;
    size_t cap = (size_t)2 * 256 * 2050;
    float *in = malloc(cap * 4), *out = malloc(cap * 4);
    OmegaGpuMathInfo info;
    for (int lg = 2; lg <= 11; lg++)
        for (int inv = 0; inv < 2; inv++) {
            uint32_t n = 1u << lg;
            fill_rand(in, (size_t)2 * n * batch);
            int rc = omega_gpu_fft_c2c(in, out, n, batch, inv, &info);
            check(rc == OMEGA_GPU_ML_OK, "chip fft n=%u inv=%d rc %d (%s)", n, inv, rc, omega_gpu_mathlibs_last_error());
            if (rc != OMEGA_GPU_ML_OK) continue;
            double e = fft_err(in, out, n, batch, inv);
            check(e <= FFT_TOL[lg], "chip fft n=%u inv=%d: error %.4e, tolerance %.4e", n, inv, e, FFT_TOL[lg]);
            receipt("chip fft n=%u inverse=%d batch=%u normwise_err=%.4e tol=%.4e unwritten=%" PRIu64 " elapsed_ns=%" PRIu64,
                    n, inv, batch, e, FFT_TOL[lg], info.unwritten_words, info.elapsed_ns);
        }
    /* a batch past one launch: 2050 transforms = 1024 + 1024 + 2 CTAs */
    fill_rand(in, cap);
    int rc = omega_gpu_fft_c2c(in, out, 256, 2050, 0, &info);
    check(rc == OMEGA_GPU_ML_OK, "chip fft batch 2050 rc %d (%s)", rc, omega_gpu_mathlibs_last_error());
    if (rc == OMEGA_GPU_ML_OK) {
        double e = fft_err(in, out, 256, 2050, 0);
        check(e <= FFT_TOL[8], "chip fft batch 2050: error %.4e", e);
        check(info.chip_calls == 3 && info.ctas_last_launch == 2, "chip fft batch 2050: %" PRIu64 " launches, last %u CTAs",
              info.chip_calls, info.ctas_last_launch);
        receipt("chip fft n=256 batch=2050 launches=%" PRIu64 " last_ctas=%u normwise_err=%.4e tol=%.4e elapsed_ns=%" PRIu64,
                info.chip_calls, info.ctas_last_launch, e, FFT_TOL[8], info.elapsed_ns);
    }
    const struct { uint32_t n; int mut; int caught; } mm[] = { { 64, 1, 1 }, { 2048, 1, 1 }, { 16, 2, 1 }, { 4, 2, 0 } };
    for (size_t i = 0; i < sizeof mm / sizeof mm[0]; i++) {
        uint32_t n = mm[i].n;
        omega_gpu_mathlibs_test_set_mutant(2, mm[i].mut);
        fill_rand(in, (size_t)2 * n * batch);
        rc = omega_gpu_fft_c2c(in, out, n, batch, 0, NULL);
        check(rc == OMEGA_GPU_ML_OK, "chip fft mutant %d n=%u rc %d", mm[i].mut, n, rc);
        if (rc != OMEGA_GPU_ML_OK) continue;
        double e = fft_err(in, out, n, batch, 0);
        int caught = !(e <= FFT_TOL[lg2u(n)]);
        check(caught == mm[i].caught, "chip fft mutant %d n=%u: error %.4e", mm[i].mut, n, e);
        receipt("chip fft_mutant%d n=%u normwise_err=%.4e caught=%d expected=%d", mm[i].mut, n, e, caught, mm[i].caught);
    }
    omega_gpu_mathlibs_test_set_mutant(0, 0);
    free(in); free(out);
}

static void chip_philox(void) {
    const uint32_t ns = 4096;
    uint32_t *got = malloc((size_t)ns * 68 * 4), *ref = malloc((size_t)ns * 68 * 4);
    OmegaGpuMathInfo info;
    size_t used = 0, fbad = 0;
    memset(g_ph_hit, 0, sizeof g_ph_hit);
    for (int s = 0; s < 4; s++)
        for (int o = 0; o < 4; o++)
            for (int r = 0; r < 2; r++) {
                uint64_t sub0 = r ? 0xFFFFFFFFull : 0;
                uint32_t n = r ? 2 : ns, per = r ? 8 : 68;
                ref_philox(PH_SEEDS[s], sub0, n, PH_OFFS[o], per, ref);
                int rc = omega_gpu_philox_generate(PH_SEEDS[s], sub0, n, PH_OFFS[o], per, got, &info);
                check(rc == OMEGA_GPU_ML_OK, "chip philox seed %" PRIu64 " offset %" PRIu64 ": rc %d (%s)", PH_SEEDS[s],
                      PH_OFFS[o], rc, omega_gpu_mathlibs_last_error());
                if (rc != OMEGA_GPU_ML_OK) continue;
                size_t bad = diff_words(got, ref, (size_t)n * per);
                used += ph_compare(got, PH_SEEDS[s], sub0, n, PH_OFFS[o], per, &fbad);
                check(bad == 0, "chip philox seed %" PRIu64 " offset %" PRIu64 ": %zu differ", PH_SEEDS[s], PH_OFFS[o], bad);
                receipt("chip philox seed=%" PRIu64 " subseq0=%" PRIu64 " ns=%u offset=%" PRIu64 " per=%u mismatches=%zu"
                        " unwritten=%" PRIu64 " elapsed_ns=%" PRIu64,
                        PH_SEEDS[s], sub0, n, PH_OFFS[o], per, bad, info.unwritten_words, info.elapsed_ns);
            }
    check(ph_distinct() == g_nph && fbad == 0, "chip philox cuRAND fixture: %zu of %zu rows covered, %zu of %zu differ",
          ph_distinct(), g_nph, fbad, used);
    receipt("chip philox_curand_fixture rows_covered=%zu of %zu comparisons=%zu mismatches=%zu", ph_distinct(), g_nph,
            used, fbad);
    free(got); free(ref);
    /* the full launch: 131072 streams x 64 words */
    const uint32_t big = OMEGA_GPU_PHILOX_MAX_STREAMS, per = 64;
    uint32_t *g2 = malloc((size_t)big * per * 4), *r2 = malloc((size_t)big * per * 4);
    ref_philox(42, 0, big, 0, per, r2);
    int rc = omega_gpu_philox_generate(42, 0, big, 0, per, g2, &info);
    check(rc == OMEGA_GPU_ML_OK, "chip philox big rc %d (%s)", rc, omega_gpu_mathlibs_last_error());
    if (rc == OMEGA_GPU_ML_OK) {
        size_t bad = diff_words(g2, r2, (size_t)big * per);
        check(bad == 0, "chip philox big: %zu differ", bad);
        receipt("chip philox_big ns=%u per=%u mismatches=%zu elapsed_ns=%" PRIu64 " ctas=%u marker=%u", big, per, bad,
                info.elapsed_ns, info.ctas_last_launch, info.completion_marker);
    }
    /* mutants */
    for (int mut = 1; mut <= 2; mut++) {
        omega_gpu_mathlibs_test_set_mutant(3, mut);
        rc = omega_gpu_philox_generate(42, 0, 256, 0, 8, g2, NULL);
        ref_philox(42, 0, 256, 0, 8, r2);
        size_t bad = rc == OMEGA_GPU_ML_OK ? diff_words(g2, r2, 256 * 8) : 0;
        check(bad > 0, "chip philox mutant %d not caught (rc %d)", mut, rc);
        receipt("chip philox_mutant%d mismatches=%zu of %u (must be > 0)", mut, bad, 256 * 8);
    }
    omega_gpu_mathlibs_test_set_mutant(0, 0);
    free(g2); free(r2);
}

static void chip_real_nd(void) {
    const uint32_t batch = 8;
    size_t cap = (size_t)(4096 + 2) * 2050;
    float *in = malloc(cap * 4), *out = malloc(cap * 4);
    OmegaGpuMathInfo info;
    for (int lg = 3; lg <= 12; lg++)
        for (int c2r = 0; c2r < 2; c2r++) {
            uint32_t n = 1u << lg;
            fill_rand(in, (size_t)(n + 2) * batch);
            if (c2r) hermitian(in, n, batch);
            int rc = c2r ? omega_gpu_fft_c2r(in, out, n, batch, &info) : omega_gpu_fft_r2c(in, out, n, batch, &info);
            const char *nm = c2r ? "c2r" : "r2c";
            check(rc == OMEGA_GPU_ML_OK, "chip %s n=%u rc %d (%s)", nm, n, rc, omega_gpu_mathlibs_last_error());
            if (rc != OMEGA_GPU_ML_OK) continue;
            double e = c2r ? c2r_err(in, out, n, batch) : r2c_err(in, out, n, batch), tol = c2r ? C2R_TOL[lg] : R2C_TOL[lg];
            check(e <= tol, "chip %s n=%u: error %.4e, tolerance %.4e", nm, n, e, tol);
            receipt("chip %s n=%u batch=%u normwise_err=%.4e tol=%.4e elapsed_ns=%" PRIu64, nm, n, batch, e, tol,
                    info.elapsed_ns);
        }
    /* past one launch: 2050 rows of 4096 = 1024 + 1024 + 2 CTAs per pass */
    fill_rand(in, (size_t)4096 * 2050);
    int rc = omega_gpu_fft_r2c(in, out, 4096, 2050, &info);
    check(rc == OMEGA_GPU_ML_OK, "chip r2c batch 2050 rc %d (%s)", rc, omega_gpu_mathlibs_last_error());
    if (rc == OMEGA_GPU_ML_OK) {
        double e = r2c_err(in, out, 4096, 2050);
        check(e <= R2C_TOL[12] && info.chip_calls == 6 && info.ctas_last_launch == 2,
              "chip r2c batch 2050: error %.4e, %" PRIu64 " launches, last %u CTAs", e, info.chip_calls,
              info.ctas_last_launch);
        receipt("chip r2c n=4096 batch=2050 launches=%" PRIu64 " normwise_err=%.4e elapsed_ns=%" PRIu64, info.chip_calls, e,
                info.elapsed_ns);
    }
    for (size_t i = 0; i < N_ND_SHAPES; i++)
        for (int inv = 0; inv < 2; inv++) {
            const uint32_t *d = ND_SHAPES[i].d;
            uint32_t nd = ND_SHAPES[i].nd;
            size_t tot = (size_t)d[0] * (nd > 1 ? d[1] : 1) * (nd > 2 ? d[2] : 1);
            fill_rand(in, 2 * tot * 2);
            rc = omega_gpu_fft_c2c_nd(in, out, nd, d, 2, inv, &info);
            check(rc == OMEGA_GPU_ML_OK, "chip nd {%u %u %u} rc %d (%s)", d[0], d[1], d[2], rc, omega_gpu_mathlibs_last_error());
            if (rc != OMEGA_GPU_ML_OK) continue;
            double e = nd_err(in, out, nd, d, 2, inv);
            check(e <= ND_SHAPES[i].tol, "chip nd {%u %u %u} inv=%d: error %.4e", d[0], d[1], d[2], inv, e);
            receipt("chip nd ndim=%u dims=%u,%u,%u inverse=%d normwise_err=%.4e tol=%.4e launches=%" PRIu64
                    " elapsed_ns=%" PRIu64,
                    nd, d[0], d[1], d[2], inv, e, ND_SHAPES[i].tol, info.chip_calls, info.elapsed_ns);
        }
    /* mutants: split twiddle, skipped bin (refused as unwritten), merge skipped bin, transposes */
    const struct { int which, lvl, c2r; uint32_t n; } rm[] = { { 4, 1, 0, 64 }, { 4, 2, 0, 4096 }, { 4, 2, 1, 4096 } };
    for (size_t i = 0; i < sizeof rm / sizeof rm[0]; i++) {
        uint32_t n = rm[i].n;
        omega_gpu_mathlibs_test_set_mutant(rm[i].which, rm[i].lvl);
        fill_rand(in, (size_t)(n + 2) * batch);
        if (rm[i].c2r) hermitian(in, n, batch);
        rc = rm[i].c2r ? omega_gpu_fft_c2r(in, out, n, batch, NULL) : omega_gpu_fft_r2c(in, out, n, batch, NULL);
        double e = rc != OMEGA_GPU_ML_OK ? -1
                   : rm[i].c2r           ? c2r_err(in, out, n, batch)
                                         : r2c_err(in, out, n, batch);
        double tol = rm[i].c2r ? C2R_TOL[lg2u(n)] : R2C_TOL[lg2u(n)];
        int caught = rc == OMEGA_GPU_ML_UNWRITTEN || (rc == OMEGA_GPU_ML_OK && !(e <= tol));
        check(caught, "chip real mutant %d/%d n=%u: rc %d error %.4e not caught", rm[i].which, rm[i].lvl, n, rc, e);
        receipt("chip real_mutant%d c2r=%d n=%u rc=%d normwise_err=%.4e caught=%d", rm[i].lvl, rm[i].c2r, n, rc, e, caught);
    }
    const uint32_t d2[2] = { 8, 16 };
    for (int lvl = 1; lvl <= 2; lvl++) {
        omega_gpu_mathlibs_test_set_mutant(5, lvl);
        fill_rand(in, 2 * 128 * 2);
        rc = omega_gpu_fft_c2c_nd(in, out, 2, d2, 2, 0, NULL);
        double e = rc == OMEGA_GPU_ML_OK ? nd_err(in, out, 2, d2, 2, 0) : -1;
        int caught = rc == OMEGA_GPU_ML_UNWRITTEN || (rc == OMEGA_GPU_ML_OK && !(e <= 9.928263778e-7));
        check(caught, "chip transpose mutant %d: rc %d error %.4e not caught", lvl, rc, e);
        receipt("chip transpose_mutant%d rc=%d normwise_err=%.4e caught=%d", lvl, rc, e, caught);
    }
    omega_gpu_mathlibs_test_set_mutant(0, 0);
    free(in); free(out);
}

int main(int argc, char **argv) {
    const char *tier = "chip", *fixture = "tests/fixtures/mathlibs_rng_aienmath.txt", *out = NULL,
               *phfix = "tests/fixtures/philox_curand_oracle.txt";
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--host-only")) tier = "host";
        else if (!strcmp(argv[i], "--sim")) tier = "sim";
        else if (!strcmp(argv[i], "--fixture") && i + 1 < argc) fixture = argv[++i];
        else if (!strcmp(argv[i], "--philox-fixture") && i + 1 < argc) phfix = argv[++i];
        else if (!strcmp(argv[i], "--out") && i + 1 < argc) out = argv[++i];
        else {
            fprintf(stderr, "usage: %s [--host-only | --sim] [--fixture F] [--philox-fixture F] [--out FILE]\n", argv[0]);
            return 2;
        }
    }
    if (out && !(g_rcpt = test_out_open(out))) { fprintf(stderr, "cannot write %s\n", out); return 2; }
    if (load_fixture(fixture) != 0) { fprintf(stderr, "cannot read fixture %s\n", fixture); return 2; }
    if (load_philox_fixture(phfix) != 0) { fprintf(stderr, "cannot read Philox fixture %s\n", phfix); return 2; }
    receipt("mathlibs tier=%s fixture=%s philox_fixture=%s", tier, fixture, phfix);
    host_tier();
    if (!strcmp(tier, "sim")) sim_tier();
    else if (!strcmp(tier, "chip")) { chip_rng(); chip_fft(); chip_philox(); chip_real_nd(); }
    if (g_failed) receipt("mathlibs %s: FAIL (%d of %d)", tier, g_failed, g_checks);
    else receipt("mathlibs %s: PASS (%d checks)", tier, g_checks);
    if (g_rcpt) fclose(g_rcpt);
    return g_failed ? 1 : 0;
}
