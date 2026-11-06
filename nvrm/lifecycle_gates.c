/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/*
 * lifecycle_gates.c -- M19R NVRM lifecycle qualification gates.
 *
 * Validates nvrm_free() (VA reuse, live/idempotent/stale detection, CPU
 * unmap) and m16_native_wait_marker_ge() (monotonic serial-number wait)
 * against the real GB10 hardware and kernel driver. New file: does not
 * modify m16_requalify.c / m16_concurrent.c or any existing test.
 */
#include "nvrm.h"
#include "cpu_barrier.h"
#include "m16_native.h"
/* Every pass/fail predicate below comes from om/engine/eng.om (op LIFE, plus
 * REQF for the release word count; src/omega_eng_gates.h). Allocation, free,
 * msync, channel and wait calls, and their order, stay here. The C oracle is
 * behind CUDAPP_MIGRATION_PENDING_C_LIFECYCLE. */
#if defined(CUDAPP_MIGRATION_PENDING_C_LIFECYCLE)
#define ENG_C_PART 1
#endif
#include "omega_eng_gates.h"
#include "omega_fixture_rest.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

_Static_assert(ENOMEM == 12, "eng.om life k3 expects ENOMEM 12");

static int g_total = 0, g_passed = 0, g_failed = 0;

static void gate_result(const char *name, int ok) {
    g_total = (int)omega_fixture_calc(FR_ADD, (uint64_t)g_total, 1, 0);
    if (ok) { g_passed = (int)omega_fixture_calc(FR_ADD, (uint64_t)g_passed, 1, 0); printf("[PASS] %s\n", name); }
    else    { g_failed = (int)omega_fixture_calc(FR_ADD, (uint64_t)g_failed, 1, 0); printf("[FAIL] %s\n", name); }
}

static uint64_t now_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return omega_fixture_calc(FR_MS, (uint64_t)t.tv_sec, (uint64_t)t.tv_nsec, 0);
}

/* ---- NVRM_FREE_CHURN ---------------------------------------------------
 * 20,000 alloc/touch/free cycles, sizes cycling 4 KiB..64 MiB, total bytes
 * churned must exceed 256 GiB (2x the machine's 128 GiB unified memory --
 * impossible without real nvrm_free() release), and va_next (the VA
 * high-water mark) must stay bounded, proving reuse rather than a bump leak.
 */
#define CHURN_CYCLES 20000

static int gate_free_churn(Nvrm *rm, uint64_t *out_bytes, uint64_t *out_high_water_delta) {
    uint64_t va_start = rm->va_next;
    uint64_t va_high = rm->va_next;
    uint64_t total_bytes = 0;

    for (int i = 0; i < CHURN_CYCLES; i++) {
        uint64_t size = omega_fixture_calc(FR_SIZE, (uint64_t)i, 0, 0);
        NvrmMem m;
        if (nvrm_alloc(rm, size, &m) != 0) {
            fprintf(stderr, "  churn: nvrm_alloc failed at cycle %d (size 0x%lx): %s\n",
                    i, (unsigned long)size, rm->err);
            return -1;
        }
        va_high = omega_fixture_calc(FR_MAX, rm->va_next, va_high, 0);

        volatile uint8_t *p = (volatile uint8_t *)m.cpu;
        uint8_t first = (uint8_t)omega_fixture_calc(FR_FIRST, (uint64_t)i, 0, 0);
        uint8_t last = (uint8_t)omega_fixture_calc(FR_LAST, (uint64_t)i, 0, 0);
        uint64_t tail = omega_fixture_calc(FR_PREV, m.size, 0, 0);
        p[0] = first;
        p[tail] = last;
        uint8_t seen_first = p[0], seen_last = p[tail];
        if (!omega_fixture_calc(FR_BOTH, omega_fixture_calc(FR_EQ, seen_first, first, 0),
                                omega_fixture_calc(FR_EQ, seen_last, last, 0), 0)) {
            fprintf(stderr, "  churn: readback mismatch at cycle %d\n", i);
            nvrm_free(rm, &m);
            return -1;
        }

        total_bytes = omega_fixture_calc(FR_ADD, total_bytes, m.size, 0);
        if (nvrm_free(rm, &m) != 0) {
            fprintf(stderr, "  churn: nvrm_free failed at cycle %d: %s\n", i, rm->err);
            return -1;
        }
        if (omega_fixture_calc(FR_LOG, (uint64_t)i, 0, 0)) {
            fprintf(stderr, "  churn: cycle %d/%d, %" PRIu64 " bytes so far\n", i, CHURN_CYCLES, total_bytes);
        }
    }

    *out_bytes = total_bytes;
    *out_high_water_delta = omega_fixture_calc(FR_SUB, va_high, va_start, 0);
    return 0;
}

/* ---- NVRM_FREE_UNMAPS ---------------------------------------------------
 * After nvrm_free(), the CPU range must no longer be mapped: msync() on it
 * must fail with ENOMEM.
 */
static int gate_free_unmaps(Nvrm *rm) {
    NvrmMem m;
    if (nvrm_alloc(rm, 0x4000, &m) != 0) {
        fprintf(stderr, "  unmaps: alloc failed: %s\n", rm->err);
        return -1;
    }
    void *va = m.cpu;
    size_t size = (size_t)m.size;

    if (nvrm_free(rm, &m) != 0) {
        fprintf(stderr, "  unmaps: free failed: %s\n", rm->err);
        return -1;
    }

    errno = 0;
    int rc = msync(va, size, MS_ASYNC);
    int en = errno;
    if (!omega_gate_life(OMEGA_LIFE_UNMAPPED, (uint64_t)(int64_t)rc, (uint64_t)(int64_t)en, 0, 0, 0, 0)) {
        if (omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)rc, 0, 0))
            fprintf(stderr, "  unmaps: msync unexpectedly succeeded on freed range %p\n", va);
        else
            fprintf(stderr, "  unmaps: msync failed with errno %d (%s), expected ENOMEM\n", en, strerror(en));
        return -1;
    }
    return 0;
}

/* ---- NVRM_FREE_IDEMPOTENT_AND_STALE ------------------------------------
 * Freeing an already-zeroed NvrmMem (double free) is a silent no-op (0, no
 * syscalls). Freeing a saved stale copy (pre-free snapshot of a struct that
 * has since been freed through another handle) must fail with -1 and must
 * not touch the driver.
 */
static int gate_idempotent_and_stale(Nvrm *rm) {
    NvrmMem m;
    if (nvrm_alloc(rm, 0x2000, &m) != 0) {
        fprintf(stderr, "  idempotent: alloc failed: %s\n", rm->err);
        return -1;
    }
    NvrmMem stale = m; /* stale copy, snapshotted before the real free */

    if (nvrm_free(rm, &m) != 0) {
        fprintf(stderr, "  idempotent: first free failed: %s\n", rm->err);
        return -1;
    }
    if (!omega_gate_life(OMEGA_LIFE_ZEROED, m.handle, m.va, m.size, (uint64_t)(uintptr_t)m.cpu, 0, 0)) {
        fprintf(stderr, "  idempotent: nvrm_free did not zero the NvrmMem on success\n");
        return -1;
    }

    /* Double free of the now-zeroed struct: must return 0. */
    if (!omega_gate_life(OMEGA_LIFE_ZERO, (uint64_t)(int64_t)nvrm_free(rm, &m), 0, 0, 0, 0, 0)) {
        fprintf(stderr, "  idempotent: double free of zeroed struct returned nonzero\n");
        return -1;
    }

    /* Free of the stale (already-freed-elsewhere) copy: must return -1. */
    if (!omega_gate_life(OMEGA_LIFE_NONZERO, (uint64_t)(int64_t)nvrm_free(rm, &stale), 0, 0, 0, 0, 0)) {
        fprintf(stderr, "  idempotent: freeing stale copy unexpectedly succeeded\n");
        return -1;
    }
    return 0;
}

/* ---- NVRM_ALLOC_AFTER_FREE_REUSES_VA ------------------------------------
 * Allocate, free, allocate the same size again: must get the same VA back.
 */
static int gate_alloc_after_free_reuses_va(Nvrm *rm) {
    NvrmMem a, b;
    if (nvrm_alloc(rm, 0x8000, &a) != 0) {
        fprintf(stderr, "  reuse: first alloc failed: %s\n", rm->err);
        return -1;
    }
    uint64_t va = a.va;
    if (nvrm_free(rm, &a) != 0) {
        fprintf(stderr, "  reuse: free failed: %s\n", rm->err);
        return -1;
    }
    if (nvrm_alloc(rm, 0x8000, &b) != 0) {
        fprintf(stderr, "  reuse: second alloc failed: %s\n", rm->err);
        return -1;
    }
    int ok = omega_gate_life(OMEGA_LIFE_EQ, b.va, va, 0, 0, 0, 0);
    if (!ok) {
        fprintf(stderr, "  reuse: expected va 0x%lx, got 0x%lx\n",
                (unsigned long)va, (unsigned long)b.va);
    }
    nvrm_free(rm, &b);
    return (int32_t)(uint32_t)omega_fixture_calc(FR_RESULT, (uint64_t)ok, 0, 0);
}

static int gate_injected_free_failure(int uvm_failure) {
    Nvrm rm;
    if (nvrm_open(&rm) != 0) return -1;
    NvrmMem m;
    if (nvrm_alloc(&rm, 0x4000, &m) != 0) { nvrm_close(&rm); return -1; }
    uint64_t va = m.va;
    uint32_t free_before = rm.free_count;
    if (uvm_failure) rm.inject_uvm_free_failure = 1;
    else rm.inject_rm_free_failure = 1;
    int frc = nvrm_free(&rm, &m);
    int refused = omega_gate_life(OMEGA_LIFE_QUARANTINE, (uint64_t)(int64_t)frc, rm.faulted, m.handle,
                                  rm.live_count, rm.free_count, free_before);
    NvrmMem other;
    if (refused)
        refused = omega_gate_life(OMEGA_LIFE_NONZERO, (uint64_t)(int64_t)nvrm_alloc(&rm, 0x4000, &other), 0, 0, 0, 0, 0);
    int rrc = nvrm_free(&rm, &m);
    uint64_t last_va = rm.free_count ? rm.free_list[omega_fixture_calc(FR_PREV, rm.free_count, 0, 0)].va : 0u;
    int recovered_release = omega_gate_life(OMEGA_LIFE_RECOVERED, (uint64_t)(int64_t)rrc, rm.free_count,
                                            free_before, last_va, va, 0);
    nvrm_close(&rm);
    return (int32_t)(uint32_t)omega_fixture_calc(FR_RESULT, omega_fixture_calc(FR_BOTH, refused, recovered_release, 0), 0, 0);
}

static int gate_corrupt_table_refusal(void) {
    Nvrm rm;
    if (nvrm_open(&rm) != 0) return -1;
    NvrmMem m;
    if (nvrm_alloc(&rm, 0x4000, &m) != 0) { nvrm_close(&rm); return -1; }
    void *cpu = rm.live[0].cpu;
    rm.live[0].cpu = NULL;
    int frc = nvrm_free(&rm, &m);
    int refused = omega_gate_life(OMEGA_LIFE_CORRUPT, (uint64_t)(int64_t)frc, rm.faulted, rm.live_count,
                                  m.handle, 0, 0);
    rm.live[0].cpu = cpu;
    int released = omega_gate_life(OMEGA_LIFE_ZERO, (uint64_t)(int64_t)nvrm_free(&rm, &m), 0, 0, 0, 0, 0);
    nvrm_close(&rm);
    return (int32_t)(uint32_t)omega_fixture_calc(FR_RESULT, omega_fixture_calc(FR_BOTH, refused, released, 0), 0, 0);
}

static int gate_allocation_overflow(Nvrm *rm) {
    NvrmMem m = {0};
    uint32_t before = rm->live_count;
    uint64_t va_before = rm->va_next;
    int r1 = nvrm_alloc(rm, UINT64_MAX, &m);
    int r2 = 0;
    if (omega_fixture_calc(FR_FAILED, (uint64_t)(int64_t)r1, 0, 0)) r2 = nvrm_alloc(rm, 0, &m);
    int ok = omega_gate_life(OMEGA_LIFE_OVERFLOW, (uint64_t)(int64_t)r1, (uint64_t)(int64_t)r2, rm->live_count,
                             before, rm->va_next, va_before);
    return (int32_t)(uint32_t)omega_fixture_calc(FR_RESULT, (uint64_t)ok, 0, 0);
}

/* ---- M16_WAIT_GE_SERIAL --------------------------------------------------
 * Pure CPU unit test, no hardware dependency: synthetic marker + serial
 * (RFC 1982 style) comparison against a target that has wrapped past 0.
 */
static int gate_wait_ge_serial(void) {
    uint32_t marker;

    /* target 0xFFFFFFF0 satisfied by marker 0x00000005 (wrapped past). */
    marker = 0x00000005u;
    if (!omega_gate_life(OMEGA_LIFE_ZERO, (uint64_t)(int64_t)m16_native_wait_marker_ge(&marker, 0xFFFFFFF0u, 50), 0,
                         0, 0, 0, 0)) {
        fprintf(stderr, "  wait_ge: wrapped-past case unexpectedly failed/timed out\n");
        return -1;
    }

    /* Not satisfied by 0xFFFFFFE0. Must time out, within timeout_ms + 50ms. */
    marker = 0xFFFFFFE0u;
    uint64_t t0 = now_ms();
    int rc = m16_native_wait_marker_ge(&marker, 0xFFFFFFF0u, 100);
    uint64_t elapsed = omega_fixture_calc(FR_SUB, now_ms(), t0, 0);
    if (!omega_gate_life(OMEGA_LIFE_NONZERO, (uint64_t)(int64_t)rc, 0, 0, 0, 0, 0)) {
        fprintf(stderr, "  wait_ge: not-yet-satisfied case unexpectedly returned success\n");
        return -1;
    }
    if (!omega_gate_life(OMEGA_LIFE_LE, elapsed, 150, 0, 0, 0, 0)) {
        fprintf(stderr, "  wait_ge: timeout took %" PRIu64 "ms, expected <= 150ms\n", elapsed);
        return -1;
    }
    return 0;
}

/* ---- GPU_SUBMIT_AFTER_CHURN ----------------------------------------------
 * After churn, open a channel and do one real submit + wait (same pattern
 * as m16/m16_native.c + m16/m16_requalify.c) to prove the device context
 * is still healthy after 20,000 alloc/free cycles.
 */
static int gate_gpu_submit_after_churn(Nvrm *rm) {
    if (nvrm_channel(rm) != 0) {
        fprintf(stderr, "  submit: nvrm_channel failed: %s\n", rm->err);
        return -1;
    }
    NvrmMem pb, marker_mem;
    if (nvrm_alloc(rm, 0x1000, &pb) != 0) {
        fprintf(stderr, "  submit: pushbuffer alloc failed: %s\n", rm->err);
        return -1;
    }
    if (nvrm_alloc(rm, 0x1000, &marker_mem) != 0) {
        fprintf(stderr, "  submit: marker alloc failed: %s\n", rm->err);
        return -1;
    }
    volatile uint32_t *marker = (volatile uint32_t *)marker_mem.cpu;
    *marker = 0;

    uint32_t methods[32];
    int nwords = m16_native_build_release(methods, marker_mem.va, 0x19c0ffee, rm->compute_class);
    if (!omega_gate_reqf(OMEGA_REQF_WORDS, nwords)) {
        fprintf(stderr, "  submit: build_release failed\n");
        return -1;
    }
    memcpy(pb.cpu, methods, (size_t)nwords * sizeof(uint32_t));
    nvrm_cpu_barrier();

    if (nvrm_submit(rm, &pb, 0, (uint32_t)nwords) != 0) {
        fprintf(stderr, "  submit: nvrm_submit failed: %s\n", rm->err);
        return -1;
    }
    if (!omega_gate_life(OMEGA_LIFE_ZERO, (uint64_t)(int64_t)m16_native_wait_marker_ge(marker, 0x19c0ffee, 2000), 0,
                         0, 0, 0, 0)) {
        fprintf(stderr, "  submit: timeout waiting for marker; actual=0x%08x\n", *marker);
        return -1;
    }
    if (!omega_gate_life(OMEGA_LIFE_EQ, *marker, 0x19c0ffeeu, 0, 0, 0, 0)) {
        fprintf(stderr, "  submit: unexpected final marker value 0x%08x\n", *marker);
        return -1;
    }
    nvrm_retire(rm, rm->put);
    return 0;
}


/* omega#523: the compute-object RM_ALLOC is refused with NV_ERR_NO_MEMORY when
 * the host has no free 2 MiB block at that instant. nvrm_channel must retry that
 * one allocation, bounded, instead of failing the channel on the first refusal. */
static int gate_channel_no_memory_retried(void) {
    Nvrm rm;
    if (nvrm_open(&rm) != 0) return -1;
    rm.inject_rm_alloc_no_memory = 3;
    rm.channel_no_memory_retry_ms = 1;
    int rc = nvrm_channel(&rm);
    int ok = omega_gate_life(OMEGA_LIFE_RETRIED, (uint64_t)(int64_t)rc, rm.compute_obj, rm.channel_no_memory_retries,
                             rm.inject_rm_alloc_no_memory, 0, 0);
    if (!ok) printf("  channel after 3 injected NV_ERR_NO_MEMORY: rc=%d retries=%u err=\"%s\"\n",
                    rc, rm.channel_no_memory_retries, rm.err);
    nvrm_close(&rm);
    return (int32_t)(uint32_t)omega_fixture_calc(FR_RESULT, (uint64_t)ok, 0, 0);
}

/* The retry is bounded: one refusal more than the budget is final, the channel
 * has no compute object, and the error names the attempts and the host's supply
 * of 2 MiB blocks so the next reader does not have to guess. */
static int gate_channel_no_memory_bounded(void) {
    Nvrm rm;
    if (nvrm_open(&rm) != 0) return -1;
    rm.inject_rm_alloc_no_memory = (uint8_t)omega_fixture_calc(FR_ADD, NVRM_CHANNEL_NO_MEMORY_RETRIES, 1, 0);
    rm.channel_no_memory_retry_ms = 1;
    int rc = nvrm_channel(&rm);
    unsigned char error_text[512] = {0};
    _Static_assert(sizeof rm.err <= sizeof error_text, "fixture text transport bound");
    memcpy(error_text, rm.err, sizeof rm.err);
    int hits = (int)omega_fixture_calc(FR_SUM3,
        omega_fixture_text(error_text, (const unsigned char[32]){"NV_ERR_NO_MEMORY"}, 16),
        omega_fixture_text(error_text, (const unsigned char[32]){"attempts"}, 8),
        omega_fixture_text(error_text, (const unsigned char[32]){"2 MiB blocks"}, 12));
    int ok = omega_gate_life(OMEGA_LIFE_BOUNDED, (uint64_t)(int64_t)rc, rm.compute_obj, rm.channel_no_memory_retries,
                             NVRM_CHANNEL_NO_MEMORY_RETRIES, (uint64_t)hits, 0);
    if (!ok) printf("  bounded retry: rc=%d retries=%u err=\"%s\"\n", rc, rm.channel_no_memory_retries, rm.err);
    int closed = omega_gate_life(OMEGA_LIFE_ZERO, (uint64_t)(int64_t)nvrm_close(&rm), 0, 0, 0, 0, 0);
    if (!closed) printf("  bounded retry: close after refused channel failed: %s\n", rm.err);
    return (int32_t)(uint32_t)omega_fixture_calc(FR_RESULT, omega_fixture_calc(FR_BOTH, ok, closed, 0), 0, 0);
}

/* The extra channel owns a subcontext as well as a FIFO. Rebuild each supported
 * configuration on one open client and check teardown returns all tracked buffers. */
static void gate_extra_channel_contexts(void) {
    for (uint32_t n = 0; n <= NVRM_MAX_EXTRA_CHANNELS; n++) {
        Nvrm rm;
        int opened = (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)nvrm_open(&rm), 0, 0);
        int ok = opened;
        uint32_t baseline = opened ? rm.live_count : 0;
        for (int round = 0; ok && round < 3; round++) {
            int crc = nvrm_channel_n(&rm, n);
            ok = omega_gate_life(OMEGA_LIFE_TORNDOWN, (uint64_t)(int64_t)crc, 0, rm.n_extra, n, 0, 0);
            for (uint32_t i = 0; ok && i < n; i++) {
                ok = omega_gate_life(OMEGA_LIFE_DISTINCT, rm.extra[i].ctxshare, rm.ctxshare, 0, 0, 0, 0);
                for (uint32_t j = 0; ok && j < i; j++)
                    ok = omega_gate_life(OMEGA_LIFE_DISTINCT, rm.extra[i].ctxshare, rm.extra[j].ctxshare, 0, 0, 0, 0);
            }
            if (nvrm_channel_destroy(&rm) != 0) ok = 0;
            if (ok) ok = omega_gate_life(OMEGA_LIFE_TORNDOWN, rm.n_extra, rm.ctxshare, rm.live_count, baseline, 0, 0);
            for (uint32_t i = 0; ok && i < n; i++)
                ok = omega_gate_life(OMEGA_LIFE_BOTHZERO, rm.extra[i].ctxshare, rm.extra[i].gpfifo, 0, 0, 0, 0);
        }
        if (opened && nvrm_close(&rm) != 0) ok = 0;
        char name[80];
        snprintf(name, sizeof name, "EXTRA_CHANNEL_CONTEXTS_%u_REOPEN_AND_RELEASE", n);
        if (!ok) fprintf(stderr, "extra channels %u: %s\n", n, rm.err);
        gate_result(name, ok);
    }
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--channels-only") == 0) {
        gate_extra_channel_contexts();
        printf("TOTAL: %d PASSED: %d FAILED: %d\n", g_total, g_passed, g_failed);
        return (int)omega_fixture_calc(FR_FAILED, (uint64_t)g_failed, 0, 0);
    }
    if (argc != 1) { fprintf(stderr, "usage: %s [--channels-only]\n", argv[0]); return 2; }
    printf("=== M19R NVRM LIFECYCLE GATES ===\n");

    gate_result("M16_WAIT_GE_SERIAL", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_wait_ge_serial(), 0, 0));

    Nvrm rm;
    if (nvrm_open(&rm) != 0) {
        fprintf(stderr, "FATAL: nvrm_open failed: %s\n", rm.err);
        gate_result("NVRM_FREE_CHURN", 0);
        gate_result("NVRM_FREE_UNMAPS", 0);
        gate_result("NVRM_FREE_IDEMPOTENT_AND_STALE", 0);
        gate_result("NVRM_ALLOC_AFTER_FREE_REUSES_VA", 0);
        gate_result("GPU_SUBMIT_AFTER_CHURN", 0);
        printf("TOTAL: %d PASSED: %d FAILED: %d\n", g_total, g_passed, g_failed);
        return 1;
    }

    uint64_t churn_bytes = 0, high_water_delta = 0;
    int churn_rc = gate_free_churn(&rm, &churn_bytes, &high_water_delta);
    /* churn_ok: 256 GiB of traffic, high-water delta under 2 GiB (eng.om life k8). */
    int churn_ok = omega_gate_life(OMEGA_LIFE_CHURN, (uint64_t)(int64_t)churn_rc, churn_bytes, high_water_delta, 0, 0, 0);
    printf("  churn: bytes=%" PRIu64 " (%.2f GiB) high_water_delta=%" PRIu64 " (%.2f MiB)\n",
           churn_bytes, omega_fixture_scale(churn_bytes, 0),
           high_water_delta, omega_fixture_scale(high_water_delta, 1));
    gate_result("NVRM_FREE_CHURN", churn_ok);

    gate_result("NVRM_FREE_UNMAPS", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_free_unmaps(&rm), 0, 0));
    gate_result("NVRM_FREE_IDEMPOTENT_AND_STALE", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_idempotent_and_stale(&rm), 0, 0));
    gate_result("NVRM_ALLOC_AFTER_FREE_REUSES_VA", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_alloc_after_free_reuses_va(&rm), 0, 0));
    gate_result("NVRM_ALLOCATION_OVERFLOW_REFUSED", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_allocation_overflow(&rm), 0, 0));
    gate_result("GPU_SUBMIT_AFTER_CHURN", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_gpu_submit_after_churn(&rm), 0, 0));

    nvrm_close(&rm);
    gate_result("UVM_FREE_FAILURE_QUARANTINES", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_injected_free_failure(1), 0, 0));
    gate_result("RM_FREE_FAILURE_QUARANTINES", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_injected_free_failure(0), 0, 0));
    gate_result("CORRUPT_TABLE_REFUSED", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_corrupt_table_refusal(), 0, 0));
    gate_result("CHANNEL_NO_MEMORY_RETRIED", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_channel_no_memory_retried(), 0, 0));
    gate_result("CHANNEL_NO_MEMORY_BOUNDED", (int)omega_fixture_calc(FR_EQ, (uint64_t)(int64_t)gate_channel_no_memory_bounded(), 0, 0));
    gate_extra_channel_contexts();
    printf("TOTAL: %d PASSED: %d FAILED: %d\n", g_total, g_passed, g_failed);
    return (int)omega_fixture_calc(FR_FAILED, (uint64_t)g_failed, 0, 0);
}
