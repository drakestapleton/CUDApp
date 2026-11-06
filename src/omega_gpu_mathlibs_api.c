/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* GB10 RNG, Philox and FFT launchers; see omega_gpu_mathlibs_api.h. The kernels and the FFT pass plans come from
 * om/mathlibs/mathlibs_ir.om. MIGRATION-PENDING C: launch, buffers, launch chunking and poison scan only. Successor:
 * om/mathlibs/mathlibs_launch.om over the session launch path, once a seeded unit can call it through omega-bridge;
 * then this file is deleted. */
#include "omega_gpu_mathlibs_api.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "omega_blackwell_codegen.h"
#include "omega_gpu_om_ir.h"
#include "omega_gpu_session.h"

#define ML_POISON32 0xffbadbadu
#define ML_POISON32_ALT 0x00badbadu
#define ML_POISON64 0xffbadbadffbadbadull
#define ML_WAIT_MS 600000ull
#define ML_GOLDEN 11400714819323198485ull /* SplitMix64 increment, 0x9E3779B97F4A7C15 */

/* One kernel per (kind, a, b, mutant), device copy uploaded once and resident for the process. */
typedef struct {
    int used;
    uint64_t kind, a, b, mutant;
    uint32_t threads, declared;
    uint8_t *buf;
    OmegaBlackwellKernel kernel;
    OmegaGpuMem code;
} Slot;
static Slot g_cache[16];
static int g_mutant_which, g_mutant_level;
static OmegaGpuScratch g_in, g_out, g_tw, g_scr;
static bool g_hooked;

static uint64_t now_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000ULL + (uint64_t)t.tv_nsec;
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
}

void omega_gpu_mathlibs_test_set_mutant(int which, int level) {
    omega_gpu_session_lock();
    g_mutant_which = which;
    g_mutant_level = level < 0 ? 0 : level > 2 ? 2 : level;
    cache_clear_locked();
    omega_gpu_session_unlock();
}

const char *omega_gpu_mathlibs_last_error(void) { return omega_gpu_session_last_error(); }

/* Caller holds the lock. Builds the kernel on a miss; uploads it when upload is set. */
/* Mutant targets: which 1 RNG (kind 0), 2 FFT (kind 1), 3 Philox (kind 3), 4 real split / merge (kind 4),
 * 5 transpose (kind 5). */
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

/* ------------------------------------------------------------------ FFT */

static int lg_in(uint32_t n, uint32_t lo, uint32_t hi) {
    if (n < lo || n > hi || (n & (n - 1)) != 0) return -1;
    int k = 0;
    while ((1u << k) < n) k++;
    return k;
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

int omega_gpu_fft_c2c(const float *in, float *out, uint32_t n, uint32_t batch, int inverse, OmegaGpuMathInfo *info) {
    int lg = fft_log2(n);
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

/* ------------------------------------------------------------------ Philox */

int omega_gpu_philox_codegen(OmegaGpuMathInfo *info) { return codegen_only(3, 0, 0, OMEGA_GPU_PHILOX_THREADS, info); }

/* Caller holds the lock: fill g_out with poison, run, read back into out. */
static int philox_pass(const Slot *s, const uint32_t *args, uint32_t grid, size_t count, uint32_t poison, uint32_t *out,
                       OmegaGpuMathInfo *info) {
    uint32_t *dev = (uint32_t *)g_out.mem.cpu;
    for (size_t i = 0; i < count; i++) dev[i] = poison;
    if (omega_gpu_session_sync_to_gpu(&g_out.mem) != 0) return OMEGA_GPU_ML_CHIP_FAIL;
    int rc = launch(s, args, 10, grid, count, 0, info);
    if (rc == OMEGA_GPU_ML_OK && omega_gpu_session_sync_to_cpu(&g_out.mem) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (rc == OMEGA_GPU_ML_OK) memcpy(out, dev, count * 4u);
    return rc;
}

int omega_gpu_philox_generate(uint64_t seed, uint64_t subseq0, uint32_t nstreams, uint64_t offset, uint32_t per_stream,
                              uint32_t *out, OmegaGpuMathInfo *info) {
    if (!out || nstreams == 0 || per_stream == 0 || per_stream % 4u != 0 || offset % 4u != 0)
        return OMEGA_GPU_ML_BAD_ARGS;
    if (nstreams > OMEGA_GPU_PHILOX_MAX_STREAMS || (uint64_t)nstreams * per_stream > OMEGA_GPU_RNG_MAX_OUTPUTS)
        return OMEGA_GPU_ML_TOO_LARGE;
    uint64_t t0 = now_ns();
    size_t count = (size_t)nstreams * per_stream;
    omega_gpu_session_lock();
    Slot *s;
    int rc = begin(3, 0, 0, OMEGA_GPU_PHILOX_THREADS, &s, info);
    if (rc == OMEGA_GPU_ML_OK && omega_gpu_session_scratch(&g_out, count * 4u) != 0) rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (rc == OMEGA_GPU_ML_OK) {
        /* 0x380 out, 0x388 key, 0x390 subsequence of stream 0, 0x398 first block, 0x3a0 blocks, 0x3a4 streams */
        uint32_t args[10];
        put64(&args[0], g_out.mem.va);
        put64(&args[2], seed);
        put64(&args[4], subseq0);
        put64(&args[6], offset / 4u);
        args[8] = per_stream / 4u;
        args[9] = nstreams;
        uint32_t grid = (nstreams + OMEGA_GPU_PHILOX_THREADS - 1) / OMEGA_GPU_PHILOX_THREADS;
        rc = philox_pass(s, args, grid, count, ML_POISON32, out, info);
        /* Every 32-bit value is a possible output: a word equal to the poison is unwritten only if a second
         * run with another poison leaves that poison there too. */
        uint64_t hits = 0;
        for (size_t i = 0; rc == OMEGA_GPU_ML_OK && i < count; i++) hits += out[i] == ML_POISON32;
        uint64_t unwritten = 0;
        if (rc == OMEGA_GPU_ML_OK && hits) {
            uint32_t *first = (uint32_t *)malloc(count * 4u);
            if (!first) rc = OMEGA_GPU_ML_CHIP_FAIL;
            if (first) {
                memcpy(first, out, count * 4u);
                rc = philox_pass(s, args, grid, count, ML_POISON32_ALT, out, info);
                for (size_t i = 0; rc == OMEGA_GPU_ML_OK && i < count; i++)
                    unwritten += first[i] == ML_POISON32 && out[i] == ML_POISON32_ALT;
                free(first);
            }
        }
        if (info) info->unwritten_words = unwritten;
        if (rc == OMEGA_GPU_ML_OK && unwritten) rc = OMEGA_GPU_ML_UNWRITTEN;
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = now_ns() - t0;
    return rc;
}

/* ------------------------------------------------------------------ FFT plans (R2C, C2R, 2-D, 3-D) */

/* Caller holds the lock. Validates, sizes the buffers, asks the unit for the pass plan (kind 6) and, when fn is set,
 * turns each step into launches of at most OMEGA_GPU_FFT_MAX_CTAS CTAs. */
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

int omega_gpu_fft_plan_walk(OmegaGpuFftPlanMode mode, uint32_t ndim, const uint32_t *dims, uint32_t batch, int inverse,
                            size_t bytes[3], OmegaGpuFftLaunchFn fn, void *ctx) {
    omega_gpu_session_lock();
    int rc = walk_locked(mode, ndim, dims, batch, inverse, bytes, fn, ctx);
    omega_gpu_session_unlock();
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
    if (rc == OMEGA_GPU_ML_OK && po < pi + bytes[0] && pi < po + bytes[1]) rc = OMEGA_GPU_ML_BAD_ARGS;
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
            for (size_t i = 0; i < bytes[1] / 4u; i++) unwritten += dev[i] == ML_POISON32;
            if (info) info->unwritten_words = unwritten;
            if (unwritten) rc = OMEGA_GPU_ML_UNWRITTEN;
        }
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = now_ns() - t0;
    return rc;
}

int omega_gpu_fft_r2c(const float *in, float *out, uint32_t n, uint32_t batch, OmegaGpuMathInfo *info) {
    return run_plan(OMEGA_GPU_FFT_R2C, 1, &n, batch, 0, in, out, info);
}

int omega_gpu_fft_c2r(const float *in, float *out, uint32_t n, uint32_t batch, OmegaGpuMathInfo *info) {
    return run_plan(OMEGA_GPU_FFT_C2R, 1, &n, batch, 0, in, out, info);
}

int omega_gpu_fft_c2c_nd(const float *in, float *out, uint32_t ndim, const uint32_t *dims, uint32_t batch, int inverse,
                         OmegaGpuMathInfo *info) {
    return run_plan(OMEGA_GPU_FFT_C2C_ND, ndim, dims, batch, inverse, in, out, info);
}
