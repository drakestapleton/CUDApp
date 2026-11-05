/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* gpu_l2_stale_test: omega #457 regression test (chip). After the host rewrites a GPU-cached (CPU_WRITE | GPU_READ) buffer that the GPU has
 * already read, does the next launch read the old bytes from GPU L2?
 *
 * Real path only: omega_gpu_fp8_tensor_upload (memcpy into the CPU mapping + omega_gpu_session_sync_to_gpu, which is
 * a dsb sy and nothing else) and omega_gpu_fp8_gemv_f32 (one launch, C3 tail, output read back from uncached memory).
 * One variable per process: --cached 0|1 (omega_gpu_fp8_set_weight_cache). --cached 0 is the negative control
 * (uncached mapping: nvos.h:1117-1118 says only the cached case is not coherent with CPU mappings).
 *
 * Tensor: 1 expert x N=256 x K=2048 e4m3 (512 KB), scales bf16 1.0, x = 1.0.
 *   A = every weight +1.0 (0x38) -> every y = +2048.   B = every weight -1.0 (0xB8) -> every y = -2048.
 * Camera: a row's 2048 bytes are 16 lines of 128 B; a stale line adds +128 instead of -128, so
 *   stale_lines(row) = (y + 2048) / 256, exact in f32. Any other y is printed as UNDECODED.
 * Modes:
 *   rewrite  upload A, GEMV (GPU reads A), host memcpy B into the SAME buffer + sync_to_gpu, then GEMV x reps.
 *   realloc  upload A, GEMV, free A, upload B (fresh buffer: VA printed, same-VA noted), then GEMV x reps.
 * The first GEMV on A must read +2048 on every row: that shows the decode can see A's bytes at all.
 * Exit 0 = FRESH, 1 = STALE_READ or API failure, 2 = usage. Red receipt (unfixed platform, omega 710d438c): rewrite
 * --cached 1 read 4096 of 4096 lines stale on every launch, 5/5; --cached 0 FRESH 5/5. */
#include "omega_gpu_fp8_api.h"
#include "omega_gpu_fp8_internal.h"
#include "omega_gpu_session.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* No copy of the tensor layout: the test reaches the weight buffer through the library-internal accessors
 * (omega_gpu_fp8_internal.h), so a change to struct OmegaGpuFp8Tensor cannot desynchronise it. A hand copy did:
 * CUDApp added allocation_epoch/owner_generation ahead of w and the copy read allocation_epoch as w.va. */

#define N 256u
#define K 2048u

static int snap(const char *site, const float *y, float expect) {
    uint32_t stale_rows = 0, stale_lines = 0, undecoded = 0; float lo = y[0], hi = y[0];
    for (uint32_t r = 0; r < N; r++) {
        if (y[r] < lo) lo = y[r];
        if (y[r] > hi) hi = y[r];
        if (expect > 0) { if (y[r] != 2048.0f) undecoded++; continue; }
        float s = (y[r] + 2048.0f) / 256.0f;
        if (s != (float)(int)s || s < 0 || s > 16) { undecoded++; continue; }
        if (s > 0) { stale_rows++; stale_lines += (uint32_t)s; }
    }
    printf("SNAP site=%s expected=%.0f rows=%u stale_rows=%u stale_lines=%u of %u undecoded=%u y_min=%.1f y_max=%.1f\n",
           site, expect, N, stale_rows, stale_lines, N * 16u, undecoded, lo, hi);
    if (undecoded) printf("SNAP site=%s y_row0=%.3f interpretation=UNDECODED\n", site, y[0]);
    return expect > 0 ? (undecoded ? -1 : 0) : (int)(stale_lines + undecoded);
}

int main(int argc, char **argv) {
    const char *mode = NULL; int cached = -1; uint32_t reps = 3;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) mode = argv[++i];
        else if (strcmp(argv[i], "--cached") == 0 && i + 1 < argc) cached = atoi(argv[++i]);
        else if (strcmp(argv[i], "--reps") == 0 && i + 1 < argc) reps = (uint32_t)atoi(argv[++i]);
        else { printf("usage: %s --mode rewrite|realloc --cached 0|1 [--reps R]\n", argv[0]); return 2; }
    }
    if (!mode || (strcmp(mode, "rewrite") && strcmp(mode, "realloc")) || (cached != 0 && cached != 1) || reps == 0 || reps > 64) {
        printf("usage: %s --mode rewrite|realloc --cached 0|1 [--reps R]\n", argv[0]); return 2;
    }
    (void)omega_gpu_session_set_spin_us(2000);
    omega_gpu_fp8_set_weight_cache(cached);
    printf("RUN mode=%s cached=%d weight_cache_now=%d n=%u k=%u reps=%u\n", mode, cached, omega_gpu_fp8_weight_cache(), N, K, reps);

    uint8_t *wa = malloc((size_t)N * K), *wb = malloc((size_t)N * K);
    uint16_t sc[(N / 128) * (K / 128)]; float x[K], y[N];
    memset(wa, 0x38, (size_t)N * K); memset(wb, 0xB8, (size_t)N * K);
    for (size_t i = 0; i < sizeof sc / sizeof sc[0]; i++) sc[i] = 0x3F80;
    for (uint32_t i = 0; i < K; i++) x[i] = 1.0f;

    OmegaGpuFp8Tensor *t = NULL;
    int rc = omega_gpu_fp8_tensor_upload(1, N, K, wa, sc, &t);
    if (rc != OMEGA_GPU_FP8_OK) { printf("FAIL upload A: %s\n", omega_gpu_fp8_last_error()); return 1; }
    uint64_t va_a = 0, sva = 0; uint8_t *w_cpu = NULL; float *s_cpu = NULL;
    if (omega_gpu_fp8_tensor_entry(t, 0, &va_a, &sva) != OMEGA_GPU_FP8_OK || omega_gpu_fp8_tensor_cpu_view(t, 0, &w_cpu, &s_cpu) != OMEGA_GPU_FP8_OK) {
        printf("FAIL tensor view: %s\n", omega_gpu_fp8_last_error()); return 1;
    }
    memset(y, 0, sizeof y);
    if ((rc = omega_gpu_fp8_gemv_f32(t, 0, x, y, NULL)) != OMEGA_GPU_FP8_OK) { printf("FAIL gemv A: %s\n", omega_gpu_fp8_last_error()); return 1; }
    if (snap("gemv-A", y, 2048.0f) != 0) { printf("STALE verdict=INVALID reason=first read of A is not +2048 (decode cannot see A)\n"); return 1; }

    if (strcmp(mode, "rewrite") == 0) {
        memcpy(w_cpu, wb, (size_t)N * K);                    /* what omega_gpu_fp8_tensor_upload does, on the same buffer */
        if (omega_gpu_fp8_tensor_sync_to_gpu(t) != 0) { printf("FAIL sync: %s\n", omega_gpu_session_last_error()); return 1; }
        printf("HOST rewrote w (va 0x%" PRIx64 ") with B and synced\n", va_a);
    } else {
        omega_gpu_fp8_tensor_free(t); t = NULL;
        if ((rc = omega_gpu_fp8_tensor_upload(1, N, K, wb, sc, &t)) != OMEGA_GPU_FP8_OK) { printf("FAIL upload B: %s\n", omega_gpu_fp8_last_error()); return 1; }
        uint64_t va_b = 0;
        if (omega_gpu_fp8_tensor_entry(t, 0, &va_b, &sva) != OMEGA_GPU_FP8_OK) { printf("FAIL tensor view B: %s\n", omega_gpu_fp8_last_error()); return 1; }
        printf("HOST freed A (va 0x%" PRIx64 "), uploaded B at va 0x%" PRIx64 " same_va=%s phys=UNKNOWN reason=no physical address in the API\n",
               va_a, va_b, va_b == va_a ? "yes" : "no");
    }
    uint32_t total = 0;
    for (uint32_t r = 0; r < reps; r++) {
        char site[32]; snprintf(site, sizeof site, "gemv-B-%u", r + 1);
        for (uint32_t i = 0; i < N; i++) y[i] = 12345.0f;
        if ((rc = omega_gpu_fp8_gemv_f32(t, 0, x, y, NULL)) != OMEGA_GPU_FP8_OK) { printf("FAIL gemv B: %s\n", omega_gpu_fp8_last_error()); return 1; }
        total += (uint32_t)snap(site, y, -2048.0f);
    }
    printf("STALE mode=%s cached=%d stale_or_undecoded_lines_total=%u verdict=%s\n", mode, cached, total, total ? "STALE_READ" : "FRESH");
    omega_gpu_fp8_tensor_free(t); free(wa); free(wb);
    return total ? 1 : 0;
}
