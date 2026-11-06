/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Device execution and byte-moving effects for the seeded Omega mathlibs units.
 * Host planning/admission is in om/mathlibs/host.om. The default-OFF C branch
 * is retained solely as the migration differential oracle. */
#include "omega_gpu_mathlibs_api.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "omega_blackwell_codegen.h"
#include "omega_gpu_om_ir.h"
#include "omega_gpu_session.h"
#include "omega_gpu_mathlibs_host.h"

#ifndef CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
#define CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST 0
#endif

#define ML_POISON32 0xffbadbadu
#define ML_POISON32_ALT 0x00badbadu
#define ML_POISON64 0xffbadbadffbadbadull
#define ML_WAIT_MS 600000ull
#define ML_GOLDEN 11400714819323198485ull /* SplitMix64 increment, 0x9E3779B97F4A7C15 */

#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
#define ml_sub(a, b) ((a) - (b))
#define ml_mul(a, b) ((a) * (b))
#define ml_overlap(a, na, b, nb) ((b) < (a) + (na) && (a) < (b) + (nb))
#define ml_scan(n, v) ((n) + ((v) == ML_POISON32))
#else
#define ml_sub omega_mh_sub
#define ml_mul omega_mh_mul
#define ml_overlap(a, na, b, nb) omega_mh_overlap((uintptr_t)(a), (na), (uintptr_t)(b), (nb))
#define ml_scan(n, v) omega_mh_add((n), omega_mh_poison((v), 0, 0, 0))
#endif

/* One kernel per (kind, a, b, mutant), device copy uploaded once and resident for the process. */
typedef struct {
    int used;
    uint64_t kind, a, b, mutant;
    uint32_t threads, declared;
    uint8_t *buf;
    OmegaBlackwellKernel kernel;
    OmegaGpuMem code;
} Slot;
static Slot g_cache[64];
static int g_mutant_which, g_mutant_level;
static OmegaGpuScratch g_in, g_out, g_tw, g_scr, g_s1, g_w, g_w2, g_b; /* g_s1..g_b: omega_gpu_fft_many */
static bool g_hooked;

static uint64_t now_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
    return (uint64_t)t.tv_sec * 1000000000ULL + (uint64_t)t.tv_nsec;
#else
    return omega_mh_time((uint64_t)t.tv_sec, (uint64_t)t.tv_nsec);
#endif
}

static void slot_clear(Slot *s) {
    free(s->buf);
    omega_gpu_session_free(&s->code);
    memset(s, 0, sizeof *s);
}

static void cache_clear_locked(void) {
    for (size_t i = 0; i < sizeof g_cache / sizeof g_cache[0]; i++)
        if (g_cache[i].used) slot_clear(&g_cache[i]);
}

void omega_gpu_mathlibs_cache_clear(void) {
    omega_gpu_session_lock();
    cache_clear_locked();
    omega_gpu_session_unlock();
}

static void on_session_close(void) {
    cache_clear_locked();
    omega_gpu_session_scratch_free(&g_in);
    omega_gpu_session_scratch_free(&g_out);
    omega_gpu_session_scratch_free(&g_tw);
    omega_gpu_session_scratch_free(&g_scr);
    omega_gpu_session_scratch_free(&g_s1);
    omega_gpu_session_scratch_free(&g_w);
    omega_gpu_session_scratch_free(&g_w2);
    omega_gpu_session_scratch_free(&g_b);
}

void omega_gpu_mathlibs_test_set_mutant(int which, int level) {
    omega_gpu_session_lock();
    g_mutant_which = which;
    g_mutant_level = level < 0 ? 0 : level > 6 ? 6 : level;
    cache_clear_locked();
    omega_gpu_session_unlock();
}

const char *omega_gpu_mathlibs_last_error(void) { return omega_gpu_session_last_error(); }

/* Caller holds the lock. Builds the kernel on a miss; uploads it when upload is set. */
/* Mutant targets: which 1 RNG (kind 0), 2 FFT (kind 1), 3 Philox (kind 3), 4 real split / merge (kind 4),
 * 5 transpose (kind 5), 7 Stockham pass (kind 7), 8 Bluestein step (kind 8), 9 strided copy (kind 9). */
static int kernel_for(uint64_t kind, uint64_t a, uint64_t b, uint32_t threads, bool upload, Slot **out, bool *hit) {
    bool target = (g_mutant_which == 1 && kind == 0) || (g_mutant_which == 2 && kind == 1) ||
                  (g_mutant_which >= 3 && (uint64_t)g_mutant_which == kind);
    uint64_t mutant = target ? (uint64_t)g_mutant_level : 0;
    Slot *s = NULL;
    *hit = false;
    for (size_t i = 0; i < sizeof g_cache / sizeof g_cache[0]; i++)
        if (g_cache[i].used && g_cache[i].kind == kind && g_cache[i].a == a && g_cache[i].b == b &&
            g_cache[i].mutant == mutant) {
            s = &g_cache[i];
            *hit = true;
            break;
        }
    if (!s) {
        BlackwellIRProgram *prog = (BlackwellIRProgram *)malloc(sizeof *prog);
        uint8_t *buf = (uint8_t *)malloc(OMEGA_BW_CODE_CAP_MAX);
        OmegaBlackwellKernel k;
        uint32_t declared = 0;
        int rc = (prog && buf) ? omega_gpu_om_ir_build(kind, a, b, mutant, threads, prog, buf, OMEGA_BW_CODE_CAP_MAX,
                                                       &k, &declared, NULL)
                               : OMEGA_OM_IR_REPLAY;
        free(prog);
        if (rc != OMEGA_OM_IR_OK) {
            free(buf);
            omega_gpu_session_set_error(omega_gpu_om_ir_rc_name(rc));
            return OMEGA_GPU_ML_CODEGEN_FAIL;
        }
        size_t slot = 0;
        for (size_t i = 0; i < sizeof g_cache / sizeof g_cache[0]; i++)
            if (!g_cache[i].used) { slot = i; break; }
        s = &g_cache[slot];
        if (s->used) slot_clear(s); /* full: evict slot 0 */
        s->used = 1;
        s->kind = kind; s->a = a; s->b = b; s->mutant = mutant;
        s->threads = threads; s->declared = declared;
        s->buf = buf;
        s->kernel = k;
    }
    if (upload && !s->code.cpu) {
        if (omega_gpu_session_alloc_code(s->kernel.code_size, &s->code) != 0) return OMEGA_GPU_ML_CHIP_FAIL;
        memcpy(s->code.cpu, s->kernel.code, s->kernel.code_size);
        if (omega_gpu_session_sync_to_gpu(&s->code) != 0) { omega_gpu_session_free(&s->code); return OMEGA_GPU_ML_CHIP_FAIL; }
    }
    *out = s;
    return OMEGA_GPU_ML_OK;
}

static void info_kernel(OmegaGpuMathInfo *info, const Slot *s, bool hit) {
    if (!info) return;
    info->kernel_cache_hit = hit;
    info->insn_count = (uint32_t)s->kernel.insn_count;
    info->gpr_peak = s->kernel.gpr_count;
    info->gpr_declared = s->declared;
    memcpy(info->code_digest, s->kernel.code_digest, 32);
    info->threads_per_cta = s->threads;
}

static void info_begin(OmegaGpuMathInfo *info, const Slot *s, bool hit) {
    if (!info) return;
    memset(info, 0, sizeof *info);
    info_kernel(info, s, hit);
}

static int codegen_only(uint64_t kind, uint64_t a, uint64_t b, uint32_t threads, OmegaGpuMathInfo *info) {
    omega_gpu_session_lock();
    Slot *s;
    bool hit;
    int rc = kernel_for(kind, a, b, threads, false, &s, &hit);
    if (rc == OMEGA_GPU_ML_OK) info_begin(info, s, hit);
    omega_gpu_session_unlock();
    return rc;
}

/* Caller holds the lock: open the shared device, build and upload the kernel. */
static int begin(uint64_t kind, uint64_t a, uint64_t b, uint32_t threads, Slot **s, OmegaGpuMathInfo *info) {
    if (!g_hooked) { (void)omega_gpu_session_on_close(on_session_close); g_hooked = true; }
    if (!omega_gpu_session_open()) return OMEGA_GPU_ML_CHIP_FAIL;
    bool hit;
    int rc = kernel_for(kind, a, b, threads, true, s, &hit);
    if (rc == OMEGA_GPU_ML_OK) info_begin(info, *s, hit);
    return rc;
}

static int launch(const Slot *s, const uint32_t *args, uint32_t n_args, uint32_t grid, size_t elements,
                  uint32_t shared_bytes, OmegaGpuMathInfo *info) {
    OmegaGpuLaunch L = {
        .code_va = s->code.va, .gpr_count = s->declared, .threads_x = s->threads, .threads_y = 1,
        .grid_x = grid, .grid_y = 1, .num_elements = elements, .args = args, .n_args = n_args,
        .timeout_ms = ML_WAIT_MS, .shared_bytes = shared_bytes,
    };
    uint64_t elapsed = 0;
    uint32_t marker = 0;
    if (omega_gpu_session_launch(&L, &elapsed, &marker) != 0) return OMEGA_GPU_ML_CHIP_FAIL;
    if (info) {
        info->chip_calls++;
        info->elapsed_ns += elapsed;
        info->completion_marker = marker;
        info->ctas_last_launch = grid;
    }
    return OMEGA_GPU_ML_OK;
}

static void put64(uint32_t *w, uint64_t v) { w[0] = (uint32_t)v; w[1] = (uint32_t)(v >> 32); }

/* ------------------------------------------------------------------ RNG */

#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
static int rng_args_ok(OmegaGpuRngKind kind, uint32_t nstreams, uint32_t per) {
    if ((unsigned)kind > OMEGA_GPU_RNG_NORMAL || nstreams == 0 || per == 0) return OMEGA_GPU_ML_BAD_ARGS;
    if (nstreams > OMEGA_GPU_RNG_MAX_STREAMS || (uint64_t)nstreams * per > OMEGA_GPU_RNG_MAX_OUTPUTS)
        return OMEGA_GPU_ML_TOO_LARGE;
    return OMEGA_GPU_ML_OK;
}

int omega_gpu_rng_codegen(OmegaGpuRngKind kind, OmegaGpuMathInfo *info) {
    if ((unsigned)kind > OMEGA_GPU_RNG_NORMAL) return OMEGA_GPU_ML_BAD_ARGS;
    return codegen_only(0, (uint64_t)kind, 0, OMEGA_GPU_RNG_THREADS, info);
}

int omega_gpu_rng_generate(OmegaGpuRngKind kind, uint64_t seed, uint64_t stream0, uint32_t nstreams,
                           uint32_t offset, uint32_t per_stream, void *out, OmegaGpuMathInfo *info) {
    if (!out) return OMEGA_GPU_ML_BAD_ARGS;
    int rc = rng_args_ok(kind, nstreams, per_stream);
    if (rc != OMEGA_GPU_ML_OK) return rc;
    uint64_t t0 = now_ns();
    size_t count = (size_t)nstreams * per_stream;
    size_t esize = kind == OMEGA_GPU_RNG_U64 ? 8u : 4u;
    size_t bytes = count * esize;
    omega_gpu_session_lock();
    Slot *s;
    rc = begin(0, (uint64_t)kind, 0, OMEGA_GPU_RNG_THREADS, &s, info);
    if (rc == OMEGA_GPU_ML_OK && omega_gpu_session_scratch(&g_out, bytes) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (rc == OMEGA_GPU_ML_OK) {
        uint32_t *dev = (uint32_t *)g_out.mem.cpu;
        for (size_t i = 0; i < bytes / 4; i++) dev[i] = ML_POISON32;
        if (omega_gpu_session_sync_to_gpu(&g_out.mem) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
        /* 0x380 out, 0x388 + 8 i base_i, 0x3a8 skip, 0x3ac per-stream count, 0x3b0 streams */
        uint32_t args[13];
        put64(&args[0], g_out.mem.va);
        uint64_t x = seed + stream0;
        for (int i = 0; i < 4; i++) { x += ML_GOLDEN; put64(&args[2 + 2 * i], x); }
        args[10] = offset;
        args[11] = per_stream;
        args[12] = nstreams;
        uint32_t grid = (nstreams + OMEGA_GPU_RNG_THREADS - 1) / OMEGA_GPU_RNG_THREADS;
        if (rc == OMEGA_GPU_ML_OK) rc = launch(s, args, 13, grid, count, 0, info);
        if (rc == OMEGA_GPU_ML_OK && omega_gpu_session_sync_to_cpu(&g_out.mem) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
        if (rc == OMEGA_GPU_ML_OK) {
            memcpy(out, g_out.mem.cpu, bytes);
            uint64_t unwritten = 0;
            if (esize == 8) {
                for (size_t i = 0; i < count; i++) unwritten += ((const uint64_t *)out)[i] == ML_POISON64;
            } else {
                for (size_t i = 0; i < count; i++) unwritten += ((const uint32_t *)out)[i] == ML_POISON32;
            }
            if (info) info->unwritten_words = unwritten;
            if (unwritten) rc = OMEGA_GPU_ML_UNWRITTEN;
        }
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = now_ns() - t0;
    return rc;
}
#endif

/* ------------------------------------------------------------------ FFT */

static int lg_in(uint32_t n, uint32_t lo, uint32_t hi) {
#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
    if (n < lo || n > hi || (n & (n - 1)) != 0) return -1;
    int k = 0;
    while ((1u << k) < n) k++;
    return k;
#else
    uint64_t k = omega_mh_lg(n, lo, hi);
    return k == UINT32_MAX ? -1 : (int)k;
#endif
}

static int fft_log2(uint32_t n) { return lg_in(n, OMEGA_GPU_FFT_MIN_N, OMEGA_GPU_FFT_MAX_N); }

static float g_twf[2u * OMEGA_GPU_FFT_REAL_MAX_N];
static uint64_t g_words[OMEGA_OM_IR_WORDS];

/* Caller holds the lock: g_twf = the unit's f32 table of 2^lg points (kind 2). */
static int twiddles_locked(uint64_t lg, uint64_t inverse) {
    if (omega_gpu_om_ir_data(2, lg, inverse, 0, g_words) != OMEGA_OM_IR_OK) return OMEGA_GPU_ML_CODEGEN_FAIL;
    for (uint32_t m = 0; m < (1u << lg); m++) {
        uint32_t re = (uint32_t)g_words[m], im = (uint32_t)(g_words[m] >> 32);
        memcpy(&g_twf[2 * m], &re, 4);
        memcpy(&g_twf[2 * m + 1], &im, 4);
    }
    return OMEGA_GPU_ML_OK;
}

int omega_gpu_fft_codegen(uint32_t n, int inverse, OmegaGpuMathInfo *info) {
    int lg = fft_log2(n);
    if (lg < 0 || (inverse != 0 && inverse != 1)) return OMEGA_GPU_ML_BAD_ARGS;
    return codegen_only(1, (uint64_t)lg, (uint64_t)inverse, n / 4, info);
}

int omega_gpu_fft_twiddles(uint32_t n, int inverse, float *tw) {
    int lg = fft_log2(n);
    if (lg < 0 || !tw || (inverse != 0 && inverse != 1)) return OMEGA_GPU_ML_BAD_ARGS;
    omega_gpu_session_lock();
    int rc = twiddles_locked((uint64_t)lg, (uint64_t)inverse);
    if (rc == OMEGA_GPU_ML_OK) memcpy(tw, g_twf, (size_t)8 * n);
    omega_gpu_session_unlock();
    return rc;
}

static int many_basic(OmegaGpuFftType type, uint32_t rank, const uint32_t *n, uint32_t batch, int inverse,
                      const float *in, float *out, OmegaGpuMathInfo *info);

#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
int omega_gpu_fft_c2c(const float *in, float *out, uint32_t n, uint32_t batch, int inverse, OmegaGpuMathInfo *info) {
    int lg = fft_log2(n);
    if (lg < 0 && n >= 2 && n <= OMEGA_GPU_FFT_MANY_MAX_N)
        return many_basic(OMEGA_GPU_FFT_TYPE_C2C, 1, &n, batch, inverse, in, out, info);
    if (lg < 0 || !in || !out || batch == 0 || (inverse != 0 && inverse != 1)) return OMEGA_GPU_ML_BAD_ARGS;
    size_t floats = (size_t)2 * n * batch;
    if (floats > ((size_t)1 << 28)) return OMEGA_GPU_ML_TOO_LARGE;
    if ((out < in + floats) && (in < out + floats)) return OMEGA_GPU_ML_BAD_ARGS;
    float *tw = (float *)malloc((size_t)2 * n * sizeof *tw);
    if (!tw) return OMEGA_GPU_ML_CHIP_FAIL;
    int rc = omega_gpu_fft_twiddles(n, inverse, tw);
    if (rc != OMEGA_GPU_ML_OK) { free(tw); return rc; }
    uint64_t t0 = now_ns();
    size_t bytes = floats * 4;
    omega_gpu_session_lock();
    Slot *s;
    rc = begin(1, (uint64_t)lg, (uint64_t)inverse, n / 4, &s, info);
    if (rc == OMEGA_GPU_ML_OK && (omega_gpu_session_scratch(&g_in, bytes) != 0 ||
                                  omega_gpu_session_scratch(&g_out, bytes) != 0 ||
                                  omega_gpu_session_scratch(&g_tw, (size_t)8 * n) != 0))
        rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (rc == OMEGA_GPU_ML_OK) {
        memcpy(g_in.mem.cpu, in, bytes);
        memcpy(g_tw.mem.cpu, tw, (size_t)8 * n);
        uint32_t *dev = (uint32_t *)g_out.mem.cpu;
        for (size_t i = 0; i < floats; i++) dev[i] = ML_POISON32;
        if (omega_gpu_session_sync_to_gpu(&g_in.mem) != 0 || omega_gpu_session_sync_to_gpu(&g_tw.mem) != 0 ||
            omega_gpu_session_sync_to_gpu(&g_out.mem) != 0)
            rc = OMEGA_GPU_ML_CHIP_FAIL;
        for (uint32_t b0 = 0; b0 < batch && rc == OMEGA_GPU_ML_OK; b0 += OMEGA_GPU_FFT_MAX_CTAS) {
            uint32_t grid = batch - b0 < OMEGA_GPU_FFT_MAX_CTAS ? batch - b0 : OMEGA_GPU_FFT_MAX_CTAS;
            uint64_t off = (uint64_t)b0 * n * 8u;
            uint32_t args[6]; /* 0x380 in, 0x388 out, 0x390 twiddles */
            put64(&args[0], g_in.mem.va + off);
            put64(&args[2], g_out.mem.va + off);
            put64(&args[4], g_tw.mem.va);
            rc = launch(s, args, 6, grid, (size_t)grid * n * 2u, 8u * n, info);
        }
        if (rc == OMEGA_GPU_ML_OK && omega_gpu_session_sync_to_cpu(&g_out.mem) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
        if (rc == OMEGA_GPU_ML_OK) {
            memcpy(out, g_out.mem.cpu, bytes);
            uint64_t unwritten = 0;
            for (size_t i = 0; i < floats; i++) {
                uint32_t u;
                memcpy(&u, &out[i], 4);
                unwritten += u == ML_POISON32;
            }
            if (info) info->unwritten_words = unwritten;
            if (unwritten) rc = OMEGA_GPU_ML_UNWRITTEN;
        }
    }
    omega_gpu_session_unlock();
    free(tw);
    if (info) info->call_ns = now_ns() - t0;
    return rc;
}
#endif

/* ------------------------------------------------------------------ Philox */

int omega_gpu_philox_codegen(OmegaGpuMathInfo *info) { return codegen_only(3, 0, 0, OMEGA_GPU_PHILOX_THREADS, info); }

/* Caller holds the lock: fill g_out with poison, run, read back into out. */
static int philox_pass(const Slot *s, const uint32_t *args, uint32_t grid, size_t count, uint32_t poison, uint32_t *out,
                       OmegaGpuMathInfo *info) {
    uint32_t *dev = (uint32_t *)g_out.mem.cpu;
    for (size_t i = 0; i < count; i++) dev[i] = poison;
    if (omega_gpu_session_sync_to_gpu(&g_out.mem) != 0) return OMEGA_GPU_ML_CHIP_FAIL;
    int rc = launch(s, args, 11, grid, count, 0, info);
    if (rc == OMEGA_GPU_ML_OK && omega_gpu_session_sync_to_cpu(&g_out.mem) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (rc == OMEGA_GPU_ML_OK) memcpy(out, dev, count * 4u);
    return rc;
}

#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
int omega_gpu_philox_generate_dist(OmegaGpuPhiloxDist dist, uint64_t seed, uint64_t subseq0, uint32_t nstreams,
                                   uint64_t offset, uint32_t per_stream, void *out, OmegaGpuMathInfo *info) {
    if (!out || (unsigned)dist > OMEGA_GPU_PHILOX_NORMAL || nstreams == 0 || per_stream == 0)
        return OMEGA_GPU_ML_BAD_ARGS;
    if (nstreams > OMEGA_GPU_PHILOX_MAX_STREAMS || (uint64_t)nstreams * per_stream > OMEGA_GPU_RNG_MAX_OUTPUTS)
        return OMEGA_GPU_ML_TOO_LARGE;
    uint64_t t0 = now_ns();
    /* G outputs per group of four words; 64-bit outputs are two words each */
    uint32_t g = dist == OMEGA_GPU_PHILOX_UNIFORM2_DOUBLE ? 2u : 4u;
    uint32_t esz = dist == OMEGA_GPU_PHILOX_UNIFORM_DOUBLE || dist == OMEGA_GPU_PHILOX_UNIFORM2_DOUBLE ? 2u : 1u;
    size_t count = (size_t)nstreams * per_stream * esz;
    uint32_t *words = (uint32_t *)out;
    omega_gpu_session_lock();
    Slot *s;
    int rc = begin(3, (uint64_t)dist, offset % 4u, OMEGA_GPU_PHILOX_THREADS, &s, info);
    if (rc == OMEGA_GPU_ML_OK && omega_gpu_session_scratch(&g_out, count * 4u) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (rc == OMEGA_GPU_ML_OK) {
        /* 0x380 out, 0x388 key, 0x390 subsequence of stream 0, 0x398 first counter block (offset / 4), 0x3a0 groups
         * T = ceil(per / G), 0x3a4 streams, 0x3a8 tail delta (per % G - G mod 2^32, 0 without a partial group) */
        uint32_t args[11];
        put64(&args[0], g_out.mem.va);
        put64(&args[2], seed);
        put64(&args[4], subseq0);
        put64(&args[6], offset / 4u);
        uint32_t r = per_stream % g;
        args[8] = per_stream / g + (r != 0);
        args[9] = nstreams;
        args[10] = r ? r - g : 0u;
        uint32_t grid = (nstreams + OMEGA_GPU_PHILOX_THREADS - 1) / OMEGA_GPU_PHILOX_THREADS;
        rc = philox_pass(s, args, grid, count, ML_POISON32, words, info);
        uint64_t unwritten = 0;
        if (dist == OMEGA_GPU_PHILOX_RAW) {
            /* Every 32-bit value is a possible output: a word equal to the poison is unwritten only if a second
             * run with another poison leaves that poison there too. */
            uint64_t hits = 0;
            for (size_t i = 0; rc == OMEGA_GPU_ML_OK && i < count; i++) hits += words[i] == ML_POISON32;
            if (rc == OMEGA_GPU_ML_OK && hits) {
                uint32_t *first = (uint32_t *)malloc(count * 4u);
                if (!first) rc = OMEGA_GPU_ML_CHIP_FAIL;
                if (first) {
                    memcpy(first, words, count * 4u);
                    rc = philox_pass(s, args, grid, count, ML_POISON32_ALT, words, info);
                    for (size_t i = 0; rc == OMEGA_GPU_ML_OK && i < count; i++)
                        unwritten += first[i] == ML_POISON32 && words[i] == ML_POISON32_ALT;
                    free(first);
                }
            }
        } else if (esz == 1) {
            /* the poison is a NaN: no f32 output of these distributions */
            for (size_t i = 0; rc == OMEGA_GPU_ML_OK && i < count; i++) unwritten += words[i] == ML_POISON32;
        } else {
            /* the poison pair is a NaN: no f64 output */
            const uint64_t *d = (const uint64_t *)out;
            for (size_t i = 0; rc == OMEGA_GPU_ML_OK && i < count / 2; i++) unwritten += d[i] == ML_POISON64;
        }
        if (info) info->unwritten_words = unwritten;
        if (rc == OMEGA_GPU_ML_OK && unwritten) rc = OMEGA_GPU_ML_UNWRITTEN;
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = now_ns() - t0;
    return rc;
}
#endif

int omega_gpu_philox_generate(uint64_t seed, uint64_t subseq0, uint32_t nstreams, uint64_t offset, uint32_t per_stream,
                              uint32_t *out, OmegaGpuMathInfo *info) {
    return omega_gpu_philox_generate_dist(OMEGA_GPU_PHILOX_RAW, seed, subseq0, nstreams, offset, per_stream, out, info);
}

/* ------------------------------------------------------------------ FFT plans (R2C, C2R, 2-D, 3-D) */

/* Caller holds the lock. Validates, sizes the buffers, asks the unit for the pass plan (kind 6) and, when fn is set,
 * turns each step into launches of at most OMEGA_GPU_FFT_MAX_CTAS CTAs. */
static int walk_locked(OmegaGpuFftPlanMode mode, uint32_t ndim, const uint32_t *dims, uint32_t batch, int inverse,
                       size_t bytes[3], OmegaGpuFftLaunchFn fn, void *ctx);
#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
static int walk_locked(OmegaGpuFftPlanMode mode, uint32_t ndim, const uint32_t *dims, uint32_t batch, int inverse,
                       size_t bytes[3], OmegaGpuFftLaunchFn fn, void *ctx) {
    if (!dims || !bytes || batch == 0 || (inverse != 0 && inverse != 1)) return OMEGA_GPU_ML_BAD_ARGS;
    uint64_t lgs = 0;
    uint32_t sum = 0;
    if (mode == OMEGA_GPU_FFT_C2C_ND) {
        if (ndim < 1 || ndim > 3) return OMEGA_GPU_ML_BAD_ARGS;
        for (uint32_t i = 0; i < ndim; i++) {
            int k = fft_log2(dims[i]);
            if (k < 0) return OMEGA_GPU_ML_BAD_ARGS;
            lgs |= (uint64_t)k << (4 * i);
            sum += (uint32_t)k;
        }
    } else if (mode == OMEGA_GPU_FFT_R2C || mode == OMEGA_GPU_FFT_C2R) {
        if (ndim != 1 || inverse != 0) return OMEGA_GPU_ML_BAD_ARGS;
        int k = lg_in(dims[0], OMEGA_GPU_FFT_REAL_MIN_N, OMEGA_GPU_FFT_REAL_MAX_N);
        if (k < 0) return OMEGA_GPU_ML_BAD_ARGS;
        lgs = (uint64_t)k;
        sum = (uint32_t)k;
    } else {
        return OMEGA_GPU_ML_BAD_ARGS;
    }
    if (sum > 27 || ((uint64_t)batch << sum) > OMEGA_GPU_FFT_MAX_ELEMENTS) return OMEGA_GPU_ML_TOO_LARGE;
    size_t elems = (size_t)batch << sum, half = ((size_t)1 << sum) / 2u + 1u;
    bytes[0] = bytes[1] = bytes[2] = elems * 8u;
    if (mode == OMEGA_GPU_FFT_R2C) { bytes[0] = elems * 4u; bytes[1] = half * 8u * batch; bytes[2] = elems * 4u; }
    if (mode == OMEGA_GPU_FFT_C2R) { bytes[0] = half * 8u * batch; bytes[1] = elems * 4u; bytes[2] = elems * 4u; }
    int rc = omega_gpu_om_ir_data(6, (uint64_t)mode | (uint64_t)ndim << 4, lgs, (uint64_t)inverse, g_words);
    if (rc == OMEGA_OM_IR_REFUSED) return OMEGA_GPU_ML_BAD_ARGS;
    if (rc != OMEGA_OM_IR_OK || g_words[0] == 0 || g_words[0] > 6) return OMEGA_GPU_ML_CODEGEN_FAIL;
    uint64_t steps[6];
    uint32_t nsteps = (uint32_t)g_words[0];
    memcpy(steps, &g_words[1], nsteps * sizeof steps[0]);
    for (uint32_t i = 0; fn && i < nsteps; i++) {
        OmegaGpuFftLaunch L;
        memset(&L, 0, sizeof L);
        L.kind = (uint32_t)(steps[i] & 255u);
        L.a = (uint32_t)(steps[i] >> 8 & 255u);
        L.b = (uint32_t)(steps[i] >> 16 & 255u);
        L.src = (uint32_t)(steps[i] >> 24 & 255u);
        L.dst = (uint32_t)(steps[i] >> 32 & 255u);
        uint64_t rows = (uint64_t)batch << (steps[i] >> 40 & 255u);
        if (L.src > 2 || L.dst > 2) return OMEGA_GPU_ML_CODEGEN_FAIL;
        if (L.kind == 1 || L.kind == 4) {
            rc = twiddles_locked(L.a, L.b);
            if (rc != OMEGA_GPU_ML_OK) return rc;
            uint64_t n = 1ull << L.a, si = n, so = n;
            L.tw = g_twf;
            L.tw_n = (uint32_t)n;
            L.tw_inverse = L.b;
            L.threads = (uint32_t)n / 4u;
            L.shared_bytes = L.kind == 1 ? (uint32_t)(8u * n) : 0u;
            if (L.kind == 4) { si = n / 2u + L.b; so = n / 2u + 1u - L.b; }
            for (uint64_t r0 = 0; r0 < rows; r0 += OMEGA_GPU_FFT_MAX_CTAS) {
                L.grid = (uint32_t)(rows - r0 < OMEGA_GPU_FFT_MAX_CTAS ? rows - r0 : OMEGA_GPU_FFT_MAX_CTAS);
                L.src_off = r0 * si * 8u;
                L.dst_off = r0 * so * 8u;
                L.elements = (size_t)2u * so * L.grid;
                rc = fn(ctx, &L);
                if (rc != OMEGA_GPU_ML_OK) return rc;
            }
        } else if (L.kind == 5) {
            uint64_t total = rows << (L.a + L.b);
            uint64_t ctas = (total + 255u) / 256u;
            L.threads = 256;
            L.total = (uint32_t)total;
            for (uint64_t c0 = 0; c0 < ctas; c0 += OMEGA_GPU_FFT_MAX_CTAS) {
                L.grid = (uint32_t)(ctas - c0 < OMEGA_GPU_FFT_MAX_CTAS ? ctas - c0 : OMEGA_GPU_FFT_MAX_CTAS);
                L.first = (uint32_t)(c0 * 256u);
                uint64_t left = total - L.first;
                L.elements = (size_t)2u * ((uint64_t)L.grid * 256u < left ? (uint64_t)L.grid * 256u : left);
                rc = fn(ctx, &L);
                if (rc != OMEGA_GPU_ML_OK) return rc;
            }
        } else {
            return OMEGA_GPU_ML_CODEGEN_FAIL;
        }
    }
    return OMEGA_GPU_ML_OK;
}
#endif

/* The launches of a walk with private copies of their twiddle tables, so the caller's fn runs unlocked. */
typedef struct {
    OmegaGpuFftLaunch *l;
    size_t n, cap;
    float **tw;
    size_t ntw, twcap;
} Collect;

static int collect(void *ctx, const OmegaGpuFftLaunch *L) {
    Collect *c = (Collect *)ctx;
    if (c->n == c->cap) {
        size_t cap = c->cap ? 2u * c->cap : 32u;
        OmegaGpuFftLaunch *l = (OmegaGpuFftLaunch *)realloc(c->l, cap * sizeof *l);
        if (!l) return OMEGA_GPU_ML_CHIP_FAIL;
        c->l = l;
        c->cap = cap;
    }
    OmegaGpuFftLaunch x = *L;
    if (L->tw_n) {
        const OmegaGpuFftLaunch *p = NULL;
        for (size_t i = c->n; i-- > 0;)
            if (c->l[i].tw_n) { p = &c->l[i]; break; }
        if (p && p->tw_n == L->tw_n && p->tw_inverse == L->tw_inverse) {
            x.tw = p->tw;
        } else {
            if (c->ntw == c->twcap) {
                size_t cap = c->twcap ? 2u * c->twcap : 8u;
                float **t = (float **)realloc(c->tw, cap * sizeof *t);
                if (!t) return OMEGA_GPU_ML_CHIP_FAIL;
                c->tw = t;
                c->twcap = cap;
            }
            float *copy = (float *)malloc((size_t)8 * L->tw_n);
            if (!copy) return OMEGA_GPU_ML_CHIP_FAIL;
            memcpy(copy, L->tw, (size_t)8 * L->tw_n);
            c->tw[c->ntw++] = copy;
            x.tw = copy;
        }
    }
    c->l[c->n++] = x;
    return OMEGA_GPU_ML_OK;
}

int omega_gpu_fft_plan_walk(OmegaGpuFftPlanMode mode, uint32_t ndim, const uint32_t *dims, uint32_t batch, int inverse,
                            size_t bytes[3], OmegaGpuFftLaunchFn fn, void *ctx) {
    Collect c;
    memset(&c, 0, sizeof c);
    omega_gpu_session_lock();
    int rc = walk_locked(mode, ndim, dims, batch, inverse, bytes, fn ? collect : NULL, &c);
    omega_gpu_session_unlock();
    for (size_t i = 0; rc == OMEGA_GPU_ML_OK && i < c.n; i++) rc = fn(ctx, &c.l[i]);
    for (size_t i = 0; i < c.ntw; i++) free(c.tw[i]);
    free(c.tw);
    free(c.l);
    return rc;
}

typedef struct {
    OmegaGpuMathInfo *info;
    OmegaGpuScratch *buf[3];
    uint32_t tw_n, tw_inverse;
} ChipRun;

/* Caller holds the lock: one launch of a plan step on the chip. */
static int chip_launch(void *ctx, const OmegaGpuFftLaunch *l) {
    ChipRun *r = (ChipRun *)ctx;
    Slot *s;
    bool hit;
    int rc = kernel_for(l->kind, l->a, l->b, l->threads, true, &s, &hit);
    if (rc != OMEGA_GPU_ML_OK) return rc;
    info_kernel(r->info, s, hit);
    if (l->tw_n && (l->tw_n != r->tw_n || l->tw_inverse != r->tw_inverse)) {
        memcpy(g_tw.mem.cpu, l->tw, (size_t)8 * l->tw_n);
        if (omega_gpu_session_sync_to_gpu(&g_tw.mem) != 0) return OMEGA_GPU_ML_CHIP_FAIL;
        r->tw_n = l->tw_n;
        r->tw_inverse = l->tw_inverse;
    }
    uint32_t args[6]; /* 0x380 in, 0x388 out, 0x390 twiddles (kind 5: element count, first element) */
    put64(&args[0], r->buf[l->src]->mem.va + l->src_off);
    put64(&args[2], r->buf[l->dst]->mem.va + l->dst_off);
    if (l->kind == 5) {
        args[4] = l->total;
        args[5] = l->first;
    } else {
        put64(&args[4], g_tw.mem.va);
    }
    return launch(s, args, 6, l->grid, l->elements, l->shared_bytes, r->info);
}

static int run_plan(OmegaGpuFftPlanMode mode, uint32_t ndim, const uint32_t *dims, uint32_t batch, int inverse,
                    const float *in, float *out, OmegaGpuMathInfo *info) {
    if (!in || !out) return OMEGA_GPU_ML_BAD_ARGS;
    if (info) memset(info, 0, sizeof *info);
    uint64_t t0 = now_ns();
    size_t bytes[3];
    omega_gpu_session_lock();
    int rc = walk_locked(mode, ndim, dims, batch, inverse, bytes, NULL, NULL);
    const char *pi = (const char *)in, *po = (const char *)out;
    if (rc == OMEGA_GPU_ML_OK && ml_overlap(pi, bytes[0], po, bytes[1])) rc = OMEGA_GPU_ML_BAD_ARGS;
    if (rc == OMEGA_GPU_ML_OK) {
        if (!g_hooked) { (void)omega_gpu_session_on_close(on_session_close); g_hooked = true; }
        if (!omega_gpu_session_open()) rc = OMEGA_GPU_ML_CHIP_FAIL;
    }
    if (rc == OMEGA_GPU_ML_OK && (omega_gpu_session_scratch(&g_in, bytes[0]) != 0 ||
                                  omega_gpu_session_scratch(&g_out, bytes[1]) != 0 ||
                                  omega_gpu_session_scratch(&g_scr, bytes[2]) != 0 ||
                                  omega_gpu_session_scratch(&g_tw, (size_t)8 * OMEGA_GPU_FFT_REAL_MAX_N) != 0))
        rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (rc == OMEGA_GPU_ML_OK) {
        memcpy(g_in.mem.cpu, in, bytes[0]);
        uint32_t *dev = (uint32_t *)g_out.mem.cpu, *scr = (uint32_t *)g_scr.mem.cpu;
        for (size_t i = 0; i < bytes[1] / 4u; i++) dev[i] = ML_POISON32;
        for (size_t i = 0; i < bytes[2] / 4u; i++) scr[i] = ML_POISON32;
        if (omega_gpu_session_sync_to_gpu(&g_in.mem) != 0 || omega_gpu_session_sync_to_gpu(&g_out.mem) != 0 ||
            omega_gpu_session_sync_to_gpu(&g_scr.mem) != 0)
            rc = OMEGA_GPU_ML_CHIP_FAIL;
        ChipRun r = {info, {&g_in, &g_out, &g_scr}, 0, 0};
        if (rc == OMEGA_GPU_ML_OK) rc = walk_locked(mode, ndim, dims, batch, inverse, bytes, chip_launch, &r);
        if (rc == OMEGA_GPU_ML_OK && omega_gpu_session_sync_to_cpu(&g_out.mem) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
        if (rc == OMEGA_GPU_ML_OK) {
            memcpy(out, dev, bytes[1]);
            uint64_t unwritten = 0;
            for (size_t i = 0; i < bytes[1] / 4u; i++) unwritten = ml_scan(unwritten, dev[i]);
            if (info) info->unwritten_words = unwritten;
            if (unwritten) rc = OMEGA_GPU_ML_UNWRITTEN;
        }
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = ml_sub(now_ns(), t0);
    return rc;
}

int omega_gpu_fft_r2c(const float *in, float *out, uint32_t n, uint32_t batch, OmegaGpuMathInfo *info) {
    if (lg_in(n, OMEGA_GPU_FFT_REAL_MIN_N, OMEGA_GPU_FFT_REAL_MAX_N) < 0 && n >= 2 && n <= OMEGA_GPU_FFT_MANY_MAX_N)
        return many_basic(OMEGA_GPU_FFT_TYPE_R2C, 1, &n, batch, 0, in, out, info);
    return run_plan(OMEGA_GPU_FFT_R2C, 1, &n, batch, 0, in, out, info);
}

int omega_gpu_fft_c2r(const float *in, float *out, uint32_t n, uint32_t batch, OmegaGpuMathInfo *info) {
    if (lg_in(n, OMEGA_GPU_FFT_REAL_MIN_N, OMEGA_GPU_FFT_REAL_MAX_N) < 0 && n >= 2 && n <= OMEGA_GPU_FFT_MANY_MAX_N)
        return many_basic(OMEGA_GPU_FFT_TYPE_C2R, 1, &n, batch, 0, in, out, info);
    return run_plan(OMEGA_GPU_FFT_C2R, 1, &n, batch, 0, in, out, info);
}

int omega_gpu_fft_c2c_nd(const float *in, float *out, uint32_t ndim, const uint32_t *dims, uint32_t batch, int inverse,
                         OmegaGpuMathInfo *info) {
    if (dims && ndim >= 1 && ndim <= 3) {
        int legacy = 1, any = 1;
        for (uint32_t i = 0; i < ndim; i++) {
            legacy &= fft_log2(dims[i]) >= 0;
            any &= dims[i] >= 2 && dims[i] <= OMEGA_GPU_FFT_MANY_MAX_N;
        }
        if (!legacy && any) return many_basic(OMEGA_GPU_FFT_TYPE_C2C, ndim, dims, batch, inverse, in, out, info);
    }
    return run_plan(OMEGA_GPU_FFT_C2C_ND, ndim, dims, batch, inverse, in, out, info);
}

/* ------------------------------------------------------------------ FFT many (cufftPlanMany layouts) */

/* A layout: element (b, x) at base + b * bs + sum x[i] * st[i] (mod 2^32), i the logical axis. */
typedef struct { uint32_t base, bs, st[3]; } Lay;

typedef struct {
    OmegaGpuFftManyLaunch *l;
    size_t n, cap;
    uint64_t *tab; /* the constant table: the unit's u64 (re, im) words */
#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
    size_t ntab, tabcap;
    struct { uint32_t kind, a, b; size_t at; } tk[16];
    uint32_t ntk;
    int alias;
    uint64_t w_bytes, b_bytes;
#endif
} Many;
typedef struct {
    uint32_t rank, batch, oext[3], owords; /* output: extents, f32 words per element */
    Lay olay;
    uint64_t oextent;
    int obasic;
} ManyOut;

static void many_free(Many *m) {
    free(m->l);
    free(m->tab);
    memset(m, 0, sizeof *m);
}

static int many_build(const OmegaGpuFftMany *p, int in_place, int alias, Many *m, size_t bytes[OMEGA_GPU_FFT_MANY_BUFS],
                      ManyOut *o);
static int many_each(uint32_t rank, const Lay *l, const uint32_t *ext, uint32_t batch,
                     int (*f)(void *, uint32_t), void *ctx);
#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
/* Caller holds the lock: byte offset of the unit's table (kind 10 twiddles, 11 chirp) of a points, appended once. */
static int many_table(Many *m, uint32_t kind, uint32_t a, uint32_t b, uint64_t *off) {
    for (uint32_t i = 0; i < m->ntk; i++)
        if (m->tk[i].kind == kind && m->tk[i].a == a && m->tk[i].b == b) { *off = 8u * m->tk[i].at; return OMEGA_GPU_ML_OK; }
    if (m->ntk == sizeof m->tk / sizeof m->tk[0]) return OMEGA_GPU_ML_CODEGEN_FAIL;
    if (omega_gpu_om_ir_data(kind, a, b, 0, g_words) != OMEGA_OM_IR_OK) return OMEGA_GPU_ML_CODEGEN_FAIL;
    if (m->ntab + a > m->tabcap) {
        size_t cap = (m->ntab + a) * 2u;
        uint64_t *t = (uint64_t *)realloc(m->tab, cap * sizeof *t);
        if (!t) return OMEGA_GPU_ML_CHIP_FAIL;
        m->tab = t;
        m->tabcap = cap;
    }
    memcpy(&m->tab[m->ntab], g_words, (size_t)a * 8u);
    m->tk[m->ntk].kind = kind; m->tk[m->ntk].a = a; m->tk[m->ntk].b = b; m->tk[m->ntk].at = m->ntab;
    m->ntk++;
    *off = 8u * m->ntab;
    m->ntab += a;
    return OMEGA_GPU_ML_OK;
}

/* Caller holds the lock: the unit's magic divisor of d (kind 13). */
static int many_magic(uint32_t d, uint32_t *mg) {
    if (omega_gpu_om_ir_data(13, d, 0, 0, g_words) != OMEGA_OM_IR_OK) return OMEGA_GPU_ML_CODEGEN_FAIL;
    *mg = (uint32_t)g_words[0];
    return OMEGA_GPU_ML_OK;
}

/* One step of `total` threads as launches of at most OMEGA_GPU_FFT_MAX_CTAS CTAs (word 1: first thread). */
static int many_step(Many *m, uint32_t kind, uint32_t a, uint32_t b, uint32_t nptr, const uint32_t *buf,
                     const uint64_t *off, uint32_t nw, const uint32_t *w, uint64_t total, uint32_t out_words) {
    OmegaGpuFftManyLaunch L;
    memset(&L, 0, sizeof L);
    L.kind = kind; L.a = a; L.b = b; L.nptr = nptr; L.nwords = nw;
    for (uint32_t i = 0; i < nptr; i++) {
        L.buf[i] = m->alias && buf[i] == OMEGA_GPU_FFT_BUF_OUT ? (uint32_t)OMEGA_GPU_FFT_BUF_IN : buf[i];
        L.off[i] = off[i];
    }
    memcpy(L.words, w, nw * sizeof w[0]);
    L.words[0] = (uint32_t)total;
    L.threads = OMEGA_GPU_FFT_MANY_THREADS;
    uint64_t ctas = (total + OMEGA_GPU_FFT_MANY_THREADS - 1u) / OMEGA_GPU_FFT_MANY_THREADS;
    for (uint64_t c0 = 0; c0 < ctas; c0 += OMEGA_GPU_FFT_MAX_CTAS) {
        L.grid = (uint32_t)(ctas - c0 < OMEGA_GPU_FFT_MAX_CTAS ? ctas - c0 : OMEGA_GPU_FFT_MAX_CTAS);
        L.words[1] = (uint32_t)(c0 * OMEGA_GPU_FFT_MANY_THREADS);
        uint64_t left = total - L.words[1], here = (uint64_t)L.grid * OMEGA_GPU_FFT_MANY_THREADS;
        L.elements = (size_t)out_words * (here < left ? here : left);
        if (m->n == m->cap) {
            size_t cap = m->cap ? 2u * m->cap : 64u;
            OmegaGpuFftManyLaunch *l = (OmegaGpuFftManyLaunch *)realloc(m->l, cap * sizeof *l);
            if (!l) return OMEGA_GPU_ML_CHIP_FAIL;
            m->l = l;
            m->cap = cap;
        }
        m->l[m->n++] = L;
    }
    return OMEGA_GPU_ML_OK;
}

/* Stockham passes (kind 7) over rows of n points, radices rad[0..np-1], from buffer src ping-ponging with alt; the
 * last pass writes fin when fin >= 0. *res: the buffer holding the result. */
static int many_stockham(Many *m, uint32_t n, uint64_t rows, uint32_t dir, uint32_t np, const uint32_t *rad,
                         uint32_t src, uint32_t alt, int fin, uint64_t tw_off, uint32_t *res) {
    uint32_t cur = src, ns = 1;
    for (uint32_t i = 0; i < np; i++) {
        uint32_t R = rad[i], nr = n / R, dst = (i + 1 == np && fin >= 0) ? (uint32_t)fin : cur == src ? alt : src;
        uint32_t w[8] = { 0, 0, nr, 0, ns, 0, n / (ns * R), n };
        int rc = many_magic(nr, &w[3]);
        if (rc == OMEGA_GPU_ML_OK) rc = many_magic(ns, &w[5]);
        uint32_t buf[3] = { cur, dst, OMEGA_GPU_FFT_BUF_TABLE };
        uint64_t off[3] = { 0, 0, tw_off };
        if (rc == OMEGA_GPU_ML_OK) rc = many_step(m, 7, R, dir, 3, buf, off, 8, w, rows * nr, 2u * R);
        if (rc != OMEGA_GPU_ML_OK) return rc;
        cur = dst;
        ns *= R;
    }
    *res = cur;
    return OMEGA_GPU_ML_OK;
}

/* One 1-D transform along contiguous rows of n points in buffer *cur (other: the second scratch array). The result
 * lands in *cur (Bluestein, or an even pass count) or in other; *cur is updated. */
static int many_axis(Many *m, uint32_t n, uint64_t rows, uint32_t dir, uint32_t *cur, uint32_t other) {
    if (omega_gpu_om_ir_data(12, n, 0, 0, g_words) != OMEGA_OM_IR_OK) return OMEGA_GPU_ML_CODEGEN_FAIL;
    uint32_t M = (uint32_t)g_words[0], blue = (uint32_t)g_words[1], np = (uint32_t)g_words[2], rad[16];
    if (np == 0 || np > 16) return OMEGA_GPU_ML_CODEGEN_FAIL;
    for (uint32_t i = 0; i < np; i++) rad[i] = (uint32_t)g_words[3 + i];
    uint64_t tw, twi, ch;
    int rc;
    if (!blue) {
        rc = many_table(m, 10, n, dir, &tw);
        if (rc == OMEGA_GPU_ML_OK) rc = many_stockham(m, n, rows, dir, np, rad, *cur, other, -1, tw, cur);
        return rc;
    }
    if (rows * M > ((uint64_t)1 << 28)) return OMEGA_GPU_ML_TOO_LARGE;
    if (rows * M * 8u > m->w_bytes) m->w_bytes = rows * M * 8u;
    if ((uint64_t)M * 8u > m->b_bytes) m->b_bytes = (uint64_t)M * 8u;
    uint32_t lgm = 0;
    while ((1u << lgm) < M) lgm++;
    rc = many_table(m, 11, n, dir, &ch);
    if (rc == OMEGA_GPU_ML_OK) rc = many_table(m, 10, M, 0, &tw);
    if (rc == OMEGA_GPU_ML_OK) rc = many_table(m, 10, M, 1, &twi);
    if (rc != OMEGA_GPU_ML_OK) return rc;
    const uint32_t W = OMEGA_GPU_FFT_BUF_W, W2 = OMEGA_GPU_FFT_BUF_W2, B = OMEGA_GPU_FFT_BUF_B, T = OMEGA_GPU_FFT_BUF_TABLE;
    uint32_t w[3] = { 0, 0, n }, x, y;
    /* the filter: b[m] = conj chirp[min(m, M - m)] (0 past n), forward FFT into B */
    uint32_t bb[3] = { T, W, T };
    uint64_t ob[3] = { ch, 0, ch };
    rc = many_step(m, 8, 3, lgm, 3, bb, ob, 3, w, M, 2);
    if (rc == OMEGA_GPU_ML_OK) rc = many_stockham(m, M, 1, 0, np, rad, W, W2, (int)B, tw, &x);
    /* a = x chirp zero-padded to M, A = FFT(a), A B, inverse FFT, chirp / M on the first n points */
    uint32_t pb[3] = { *cur, W, T };
    uint64_t po[3] = { 0, 0, ch };
    if (rc == OMEGA_GPU_ML_OK) rc = many_step(m, 8, 0, lgm, 3, pb, po, 3, w, rows * M, 2);
    if (rc == OMEGA_GPU_ML_OK) rc = many_stockham(m, M, rows, 0, np, rad, W, W2, -1, tw, &x);
    uint32_t mb[3] = { x, x, B };
    uint64_t mo[3] = { 0, 0, 0 };
    if (rc == OMEGA_GPU_ML_OK) rc = many_step(m, 8, 1, lgm, 3, mb, mo, 3, w, rows * M, 2);
    if (rc == OMEGA_GPU_ML_OK) rc = many_stockham(m, M, rows, 1, np, rad, x, x == W ? W2 : W, -1, twi, &y);
    uint32_t qb[3] = { y, *cur, T };
    if (rc == OMEGA_GPU_ML_OK) rc = many_step(m, 8, 2, lgm, 3, qb, po, 3, w, rows * M, 2);
    return rc;
}

/* A strided copy (kind 9): slot k (i0 fastest) walks logical axis ord[k] (-1: none) over ext[k] points. */
static int many_copy(Many *m, uint32_t mode, uint32_t sbuf, const Lay *s, uint32_t dbuf, const Lay *d, const int ord[3],
                     const uint32_t ext[3], uint32_t batch) {
    uint32_t w[18];
    memset(w, 0, sizeof w);
    uint64_t total = batch;
    for (int k = 0; k < 3; k++) {
        uint32_t e = ord[k] < 0 ? 1u : ext[k];
        if (e == 0) return OMEGA_GPU_ML_OK;
        total *= e;
        w[2 + 2 * k] = e;
        int rc = many_magic(e, &w[3 + 2 * k]);
        if (rc != OMEGA_GPU_ML_OK) return rc;
        w[10 + k] = ord[k] < 0 ? 0u : s->st[ord[k]];
        w[15 + k] = ord[k] < 0 ? 0u : d->st[ord[k]];
    }
    w[8] = s->base; w[9] = s->bs; w[13] = d->base; w[14] = d->bs;
    uint32_t buf[2] = { sbuf, dbuf };
    uint64_t off[2] = { 0, 0 };
    return many_step(m, 9, mode, 0, 2, buf, off, 18, w, total, mode == 2 ? 1u : 2u);
}

/* The contiguous layout of extents e with axis perm[0] slowest. */
static Lay many_dense(uint32_t rank, const uint32_t *perm, const uint32_t *e) {
    Lay l;
    memset(&l, 0, sizeof l);
    uint32_t p = 1;
    for (int k = (int)rank - 1; k >= 0; k--) { l.st[perm[k]] = p; p *= e[perm[k]]; }
    l.bs = p;
    return l;
}

/* Rows of a transform along axis ax: batch times the other extents. */
static uint64_t many_rows(uint32_t rank, const uint32_t *e, uint32_t ax, uint32_t batch) {
    uint64_t rows = batch;
    for (uint32_t i = 0; i < rank; i++)
        if (i != ax) rows *= e[i];
    return rows;
}

/* Slots of a copy that walks the destination order perm (fastest first). */
static void many_slots(uint32_t rank, const uint32_t *perm, const uint32_t *e, int ord[3], uint32_t ext[3]) {
    for (uint32_t k = 0; k < 3; k++) {
        ord[k] = k < rank ? (int)perm[rank - 1 - k] : -1;
        ext[k] = k < rank ? e[perm[rank - 1 - k]] : 1u;
    }
}

/* A user side: extents ext, embed (all zero: basic), stride, dist; real pads the basic last axis to pad points. */
static int many_side(uint32_t rank, const uint32_t *ext, const uint32_t *emb, uint32_t stride, uint32_t dist,
                     uint32_t batch, int basic, uint32_t pad, Lay *l, uint64_t *extent) {
    uint32_t e[3];
    uint64_t st[3], prod = 1;
    for (uint32_t i = 0; i < rank; i++) e[i] = basic ? ext[i] : emb[i];
    if (basic) {
        if (pad) e[rank - 1] = pad;
        stride = 1;
        for (uint32_t i = 0; i < rank; i++) prod *= e[i];
        if (prod >= ((uint64_t)1 << 32)) return OMEGA_GPU_ML_TOO_LARGE;
        dist = (uint32_t)prod;
    } else {
        if (stride == 0) return OMEGA_GPU_ML_BAD_ARGS;
        for (uint32_t i = 1; i < rank; i++)
            if (e[i] < ext[i]) return OMEGA_GPU_ML_BAD_ARGS;
    }
    uint64_t p = stride, hi = (uint64_t)dist * (batch - 1u);
    for (int i = (int)rank - 1; i >= 0; i--) {
        if (p >= ((uint64_t)1 << 32)) return OMEGA_GPU_ML_TOO_LARGE;
        st[i] = p;
        hi += (ext[i] - 1u) * p;
        p *= e[i];
    }
    if (hi >= ((uint64_t)1 << 32) || hi + 1u > ((uint64_t)1 << 28)) return OMEGA_GPU_ML_TOO_LARGE;
    memset(l, 0, sizeof *l);
    l->bs = dist;
    for (uint32_t i = 0; i < rank; i++) l->st[i] = (uint32_t)st[i];
    *extent = hi + 1u;
    return OMEGA_GPU_ML_OK;
}

/* Visits every element index of a side (batch major, last axis fastest); stops on a nonzero return. */
static int many_each(uint32_t rank, const Lay *l, const uint32_t *ext, uint32_t batch,
                     int (*f)(void *ctx, uint32_t idx), void *ctx) {
    uint32_t e[3] = { 1, 1, 1 }, s[3] = { 0, 0, 0 };
    for (uint32_t i = 0; i < rank; i++) { e[3 - rank + i] = ext[i]; s[3 - rank + i] = l->st[i]; }
    for (uint32_t b = 0; b < batch; b++)
        for (uint32_t x0 = 0; x0 < e[0]; x0++)
            for (uint32_t x1 = 0; x1 < e[1]; x1++)
                for (uint32_t x2 = 0; x2 < e[2]; x2++) {
                    int rc = f(ctx, l->base + b * l->bs + x0 * s[0] + x1 * s[1] + x2 * s[2]);
                    if (rc) return rc;
                }
    return 0;
}

static int many_mark(void *ctx, uint32_t idx) {
    uint8_t *bits = (uint8_t *)ctx;
    uint8_t bit = (uint8_t)(1u << (idx & 7u));
    if (bits[idx >> 3] & bit) return 1;
    bits[idx >> 3] |= bit;
    return 0;
}


/* Caller holds the lock. Validates p, fills bytes[] and m's launches and table; *o describes the output side. */
static int many_build(const OmegaGpuFftMany *p, int in_place, int alias, Many *m, size_t bytes[OMEGA_GPU_FFT_MANY_BUFS],
                      ManyOut *o) {
    memset(m, 0, sizeof *m);
    m->alias = alias;
    if (!p || !bytes || p->rank < 1 || p->rank > 3 || p->batch == 0 || (unsigned)p->type > OMEGA_GPU_FFT_TYPE_C2R)
        return OMEGA_GPU_ML_BAD_ARGS;
    if (p->type == OMEGA_GPU_FFT_TYPE_C2C ? (p->inverse != 0 && p->inverse != 1) : p->inverse != 0)
        return OMEGA_GPU_ML_BAD_ARGS;
    const uint32_t r = p->rank, last = r - 1u, L = p->n[last], h = L / 2u + 1u;
    uint64_t P = 1;
    int ibasic = 1, obasic = 1;
    for (uint32_t i = 0; i < r; i++) {
        if (p->n[i] < 2 || p->n[i] > OMEGA_GPU_FFT_MANY_MAX_N) return OMEGA_GPU_ML_BAD_ARGS;
        P *= p->n[i];
        ibasic &= p->inembed[i] == 0;
        obasic &= p->onembed[i] == 0;
    }
    if (ibasic != obasic) return OMEGA_GPU_ML_BAD_ARGS;
    if (P * p->batch > OMEGA_GPU_FFT_MAX_ELEMENTS) return OMEGA_GPU_ML_TOO_LARGE;
    uint32_t iext[3], oext[3];
    memcpy(iext, p->n, sizeof iext);
    memcpy(oext, p->n, sizeof oext);
    int r2c = p->type == OMEGA_GPU_FFT_TYPE_R2C, c2r = p->type == OMEGA_GPU_FFT_TYPE_C2R;
    if (c2r) iext[last] = h;
    if (r2c) oext[last] = h;
    Lay il, ol;
    uint64_t iextent, oextent;
    int rc = many_side(r, iext, p->inembed, p->istride, p->idist, p->batch, ibasic, r2c && in_place ? 2u * h : 0u, &il,
                       &iextent);
    if (rc == OMEGA_GPU_ML_OK)
        rc = many_side(r, oext, p->onembed, p->ostride, p->odist, p->batch, obasic, c2r && in_place ? 2u * h : 0u, &ol,
                       &oextent);
    if (rc != OMEGA_GPU_ML_OK) return rc;
    if (!obasic) {
        uint8_t *bits = (uint8_t *)calloc((size_t)(oextent + 7u) / 8u, 1);
        if (!bits) return OMEGA_GPU_ML_CHIP_FAIL;
        int dup = many_each(r, &ol, oext, p->batch, many_mark, bits);
        free(bits);
        if (dup) return OMEGA_GPU_ML_BAD_ARGS;
    }
    const uint32_t S0 = OMEGA_GPU_FFT_BUF_S0, S1 = OMEGA_GPU_FFT_BUF_S1;
    const uint32_t dir = c2r ? 1u : (uint32_t)p->inverse;
    uint32_t perm[3] = { 0, 1, 2 }, e[3], cur = S0, ext[3];
    int ord[3];
    memcpy(e, iext, sizeof e);
    if (r2c) e[last] = L;
    if (c2r) /* the last axis slowest: the outer inverses come first */
        for (uint32_t k = 0; k < r; k++) perm[k] = k == 0 ? last : k - 1u;
    Lay cl = many_dense(r, perm, e);
    many_slots(r, perm, e, ord, ext);
    rc = many_copy(m, r2c ? 1u : 0u, OMEGA_GPU_FFT_BUF_IN, &il, S0, &cl, ord, ext, p->batch);
    uint32_t nax = c2r ? r - 1u : r;
    for (uint32_t t = 0; t < nax && rc == OMEGA_GPU_ML_OK; t++) {
        uint32_t ax = perm[r - 1u], other = cur == S0 ? S1 : S0;
        rc = many_axis(m, p->n[ax], many_rows(r, e, ax, p->batch), dir, &cur, other);
        if (r2c && ax == last) e[last] = h;
        if (rc != OMEGA_GPU_ML_OK || (t + 1u == nax && !c2r)) break;
        /* rotate: the fastest axis becomes the slowest */
        uint32_t np[3];
        for (uint32_t k = 0; k < r; k++) np[k] = k == 0 ? perm[r - 1u] : perm[k - 1u];
        Lay nl = many_dense(r, np, e);
        many_slots(r, np, e, ord, ext);
        other = cur == S0 ? S1 : S0;
        rc = many_copy(m, 0, cur, &cl, other, &nl, ord, ext, p->batch);
        memcpy(perm, np, sizeof perm);
        cl = nl;
        cur = other;
    }
    if (rc == OMEGA_GPU_ML_OK && c2r) {
        /* Hermitian extension of the last axis: k < h as it is, k in [h, L) the conjugate of point L - k */
        uint32_t other = cur == S0 ? S1 : S0, fe[3];
        memcpy(fe, e, sizeof fe);
        fe[last] = L;
        Lay fl = many_dense(r, perm, fe);
        many_slots(r, perm, e, ord, ext);
        rc = many_copy(m, 0, cur, &cl, other, &fl, ord, ext, p->batch);
        if (rc == OMEGA_GPU_ML_OK && L > h) {
            Lay sl = cl, dl = fl;
            sl.base = L - h;
            sl.st[last] = 0xFFFFFFFFu;
            dl.base = h;
            ext[0] = L - h;
            rc = many_copy(m, 3, cur, &sl, other, &dl, ord, ext, p->batch);
        }
        cl = fl;
        cur = other;
        e[last] = L;
        if (rc == OMEGA_GPU_ML_OK) rc = many_axis(m, L, many_rows(r, e, last, p->batch), dir, &cur, cur == S0 ? S1 : S0);
    }
    if (rc == OMEGA_GPU_ML_OK) {
        uint32_t id[3] = { 0, 1, 2 };
        many_slots(r, id, e, ord, ext);
        rc = many_copy(m, c2r ? 2u : 0u, cur, &cl, OMEGA_GPU_FFT_BUF_OUT, &ol, ord, ext, p->batch);
    }
    if (rc != OMEGA_GPU_ML_OK) return rc;
    uint64_t ib = iextent * (r2c ? 4u : 8u), ob = oextent * (c2r ? 4u : 8u);
    bytes[OMEGA_GPU_FFT_BUF_IN] = alias ? (ib > ob ? ib : ob) : ib;
    bytes[OMEGA_GPU_FFT_BUF_OUT] = alias ? 0u : ob;
    bytes[OMEGA_GPU_FFT_BUF_S0] = bytes[OMEGA_GPU_FFT_BUF_S1] = (size_t)(P * p->batch * 8u);
    bytes[OMEGA_GPU_FFT_BUF_W] = bytes[OMEGA_GPU_FFT_BUF_W2] = (size_t)m->w_bytes;
    bytes[OMEGA_GPU_FFT_BUF_B] = (size_t)m->b_bytes;
    bytes[OMEGA_GPU_FFT_BUF_TABLE] = m->ntab * 8u;
    if (o) {
        o->rank = r;
        o->batch = p->batch;
        memcpy(o->oext, oext, sizeof oext);
        o->owords = c2r ? 1u : 2u;
        o->olay = ol;
        o->oextent = oextent;
        o->obasic = obasic;
    }
    return OMEGA_GPU_ML_OK;
}
#endif

int omega_gpu_fft_many_walk(const OmegaGpuFftMany *plan, int in_place, size_t bytes[OMEGA_GPU_FFT_MANY_BUFS],
                            OmegaGpuFftManyLaunchFn fn, void *ctx) {
    if (in_place != 0 && in_place != 1) return OMEGA_GPU_ML_BAD_ARGS;
    Many m;
    omega_gpu_session_lock();
    int rc = many_build(plan, in_place, in_place, &m, bytes, NULL);
    omega_gpu_session_unlock();
    for (size_t i = 0; fn && rc == OMEGA_GPU_ML_OK && i < m.n; i++) {
        m.l[i].table = m.tab;
        rc = fn(ctx, &m.l[i]);
    }
    many_free(&m);
    return rc;
}

typedef struct { const uint32_t *dev; uint32_t *out; uint32_t words; uint64_t unwritten; } ManyScan;
#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
static int many_scan(void *ctx, uint32_t idx) {
    ManyScan *c = (ManyScan *)ctx;
    for (uint32_t k = 0; k < c->words; k++) c->unwritten += c->dev[(size_t)idx * c->words + k] == ML_POISON32;
    return 0;
}
#else
static int many_scan(void *ctx, uint32_t idx) {
    ManyScan *c = (ManyScan *)ctx;
    size_t first = omega_mh_mul(idx, c->words);
    for (uint32_t k = 0; k < c->words; k++)
        c->unwritten = omega_mh_add(c->unwritten, omega_mh_poison(c->dev[first + k], 0, 0, 0));
    return 0;
}
#endif
static int many_copy_back(void *ctx, uint32_t idx) {
    ManyScan *c = (ManyScan *)ctx;
    size_t at = ml_mul(idx, c->words);
    memcpy(&c->out[at], &c->dev[at], ml_mul(4, c->words));
    return 0;
}

int omega_gpu_fft_many(const OmegaGpuFftMany *plan, const float *in, float *out, OmegaGpuMathInfo *info) {
    if (!plan || !in || !out) return OMEGA_GPU_ML_BAD_ARGS;
    if (info) memset(info, 0, sizeof *info);
    uint64_t t0 = now_ns();
    int in_place = (const void *)in == (const void *)out;
    size_t bytes[OMEGA_GPU_FFT_MANY_BUFS];
    Many m;
    ManyOut o;
    omega_gpu_session_lock();
    int rc = many_build(plan, in_place, 0, &m, bytes, &o);
    const char *pi = (const char *)in, *po = (const char *)out;
    if (rc == OMEGA_GPU_ML_OK && !in_place &&
        ml_overlap(pi, bytes[OMEGA_GPU_FFT_BUF_IN], po, bytes[OMEGA_GPU_FFT_BUF_OUT]))
        rc = OMEGA_GPU_ML_BAD_ARGS;
    if (rc == OMEGA_GPU_ML_OK) {
        if (!g_hooked) { (void)omega_gpu_session_on_close(on_session_close); g_hooked = true; }
        if (!omega_gpu_session_open()) rc = OMEGA_GPU_ML_CHIP_FAIL;
    }
    OmegaGpuScratch *buf[OMEGA_GPU_FFT_MANY_BUFS] = { &g_in, &g_out, &g_scr, &g_s1, &g_w, &g_w2, &g_b, &g_tw };
    for (int i = 0; rc == OMEGA_GPU_ML_OK && i < OMEGA_GPU_FFT_MANY_BUFS; i++)
        if (omega_gpu_session_scratch(buf[i], bytes[i] ? bytes[i] : 8u) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (rc == OMEGA_GPU_ML_OK) {
        memcpy(g_in.mem.cpu, in, bytes[OMEGA_GPU_FFT_BUF_IN]);
        memcpy(g_tw.mem.cpu, m.tab, bytes[OMEGA_GPU_FFT_BUF_TABLE]);
        uint32_t *dev = (uint32_t *)g_out.mem.cpu;
        for (size_t i = 0; i < bytes[OMEGA_GPU_FFT_BUF_OUT] / 4u; i++) dev[i] = ML_POISON32;
        if (omega_gpu_session_sync_to_gpu(&g_in.mem) != 0 || omega_gpu_session_sync_to_gpu(&g_tw.mem) != 0 ||
            omega_gpu_session_sync_to_gpu(&g_out.mem) != 0)
            rc = OMEGA_GPU_ML_CHIP_FAIL;
        for (size_t i = 0; rc == OMEGA_GPU_ML_OK && i < m.n; i++) {
            const OmegaGpuFftManyLaunch *l = &m.l[i];
            Slot *s;
            bool hit;
            rc = kernel_for(l->kind, l->a, l->b, l->threads, true, &s, &hit);
            if (rc != OMEGA_GPU_ML_OK) break;
            info_kernel(info, s, hit);
            uint32_t args[OMEGA_GPU_SESSION_MAX_ARGS];
            for (uint32_t k = 0; k < l->nptr; k++) put64(&args[2 * k], buf[l->buf[k]]->mem.va + l->off[k]);
            memcpy(&args[2 * l->nptr], l->words, l->nwords * sizeof l->words[0]);
            rc = launch(s, args, 2 * l->nptr + l->nwords, l->grid, l->elements, 0, info);
        }
        if (rc == OMEGA_GPU_ML_OK && omega_gpu_session_sync_to_cpu(&g_out.mem) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
        if (rc == OMEGA_GPU_ML_OK) {
            /* only the addressed output elements are checked and copied back */
            ManyScan c = { dev, (uint32_t *)(void *)out, o.owords, 0 };
            (void)many_each(o.rank, &o.olay, o.oext, o.batch, many_scan, &c);
            if (info) info->unwritten_words = c.unwritten;
            if (c.unwritten) rc = OMEGA_GPU_ML_UNWRITTEN;
            else (void)many_each(o.rank, &o.olay, o.oext, o.batch, many_copy_back, &c);
        }
    }
    omega_gpu_session_unlock();
    many_free(&m);
    if (info) info->call_ns = ml_sub(now_ns(), t0);
    return rc;
}

/* The legacy entry points' other lengths: basic layout, in and out may not overlap. */
static int many_basic(OmegaGpuFftType type, uint32_t rank, const uint32_t *n, uint32_t batch, int inverse,
                      const float *in, float *out, OmegaGpuMathInfo *info) {
#if CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
    if (!in || !out || in == out || !n || rank < 1 || rank > 3) return OMEGA_GPU_ML_BAD_ARGS;
#else
    if (omega_mh_basic(rank, (uintptr_t)in, (uintptr_t)out, (uintptr_t)n)) return OMEGA_GPU_ML_BAD_ARGS;
#endif
    OmegaGpuFftMany p;
    memset(&p, 0, sizeof p);
    p.rank = rank;
    memcpy(p.n, n, rank * sizeof n[0]);
    p.batch = batch;
    p.type = type;
    p.inverse = inverse;
    return omega_gpu_fft_many(&p, in, out, info);
}

#if !CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST
#include "omega_gpu_mathlibs_host.inc"
#endif
