/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_wait.h"

#include <string.h>
#if defined(CUDAPP_MIGRATION_PENDING_C_WAIT)
#define ENG_C_PART 1
#endif
#include "omega_eng_dec.h"

#ifndef OMEGA_GPU_NO_POSIX
#include <time.h>
static uint64_t real_now_ns(void *ctx) {
    (void)ctx;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}
#define DEFAULT_NOW_NS real_now_ns
#else
/* P07 cut C: the freestanding build has no clock of its own; the caller passes the platform clock
 * (cfg->now_ns, e.g. OmegaGpuPlatform.time_now_ns) and a wait without one is refused (INVALID_STATE). */
#define DEFAULT_NOW_NS NULL
#endif

static void real_barrier(void *ctx) {
    (void)ctx;
#if defined(__aarch64__)
    __asm__ volatile("dsb sy" ::: "memory");
#else
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
#endif
}

static void real_relax(void *ctx) {
    (void)ctx;
#if defined(__aarch64__)
    __asm__ volatile("yield");
#else
    __asm__ volatile("" ::: "memory");
#endif
}

const char *omega_gpu_wait_result_name(omega_gpu_wait_result_t r) {
    switch (r) {
    case OMEGA_GPU_WAIT_PASS: return "PASS";
    case OMEGA_GPU_WAIT_TIMEOUT: return "TIMEOUT";
    case OMEGA_GPU_WAIT_STALLED: return "STALLED";
    case OMEGA_GPU_WAIT_INVALID_STATE: return "INVALID_STATE";
    case OMEGA_GPU_WAIT_ABORTED: return "ABORTED";
    default: return "UNKNOWN";
    }
}

const char *omega_gpu_wait_kind_name(omega_gpu_wait_kind_t k) {
    return k == OMEGA_GPU_WAIT_FIXED ? "fixed" : k == OMEGA_GPU_WAIT_SEQUENCE ? "sequence" : "unknown";
}

/* Decisions come from om/engine/eng.om. This function only calls the clock,
 * the barrier, the hooks and the watched words. */
static bool wait_om(omega_gpu_wait_kind_t kind, volatile uint32_t *word, uint32_t expected,
                    omega_gpu_wait_report_t *rpt, const omega_gpu_wait_cfg_t *cfg) {
    uint64_t (*now_ns)(void *) = cfg->now_ns ? cfg->now_ns : DEFAULT_NOW_NS;
    void (*barrier)(void *) = cfg->barrier ? cfg->barrier : real_barrier;
    void (*relax)(void *) = cfg->relax ? cfg->relax : real_relax;
    uint64_t in[23];
    uint64_t w[16];
    uint64_t start, now, hard, stall, total_ns, stall_ns, flags;
    int first;

    memset(in, 0, sizeof in);
    in[0] = cfg->total_timeout_ms;
    in[1] = cfg->progress_timeout_ms;
    in[3] = word ? 1u : 0u;
    in[4] = now_ns ? 1u : 0u;
    if (eng_ask(ENG_OP_WINIT, in, 5, w) != 0) {
        rpt->result = OMEGA_GPU_WAIT_INVALID_STATE;
        return false;
    }
    if (w[5]) {
        rpt->result = OMEGA_GPU_WAIT_INVALID_STATE;
        return false;
    }
    start = now_ns(cfg->hook_ctx);
    in[2] = start;
    if (eng_ask(ENG_OP_WINIT, in, 5, w) != 0) {
        rpt->result = OMEGA_GPU_WAIT_INVALID_STATE;
        return false;
    }
    /* The C oracle records the start before it refuses a null word. */
    rpt->start_ns = start;
    rpt->last_progress_ns = start;
    if (w[4]) {
        rpt->result = OMEGA_GPU_WAIT_INVALID_STATE;
        return false;
    }
    total_ns = w[0];
    stall_ns = w[1];
    hard = w[2];
    stall = w[3];
    now = start;
    first = 1;
    flags = 0;
#ifdef OMEGA_GPU_WAIT_MUTANT_STALL_NO_RESET
    flags |= 2u;
#endif
#ifdef OMEGA_GPU_WAIT_MUTANT_PROGRESS_EXTENDS_HARD
    flags |= 1u;
#endif
    for (;;) {
        uint32_t m, m2, snap[2];
        int ask;

        memset(in, 0, sizeof in);
        in[0] = now;
        in[1] = hard;
        in[2] = start;
        if (eng_ask(ENG_OP_WENTER, in, 3, w) != 0) {
            rpt->result = OMEGA_GPU_WAIT_INVALID_STATE;
            return false;
        }
        if (w[0] == 0) {
            rpt->elapsed_ns = w[6];
            rpt->result = OMEGA_GPU_WAIT_TIMEOUT;
            return false;
        }
        if (cfg->abort && cfg->abort(cfg->hook_ctx)) {
            memset(in, 0, sizeof in);
            in[8] = 1u;
            in[9] = now;
            in[10] = start;
            if (eng_ask(ENG_OP_WPOLL, in, 11, w) != 0) {
                rpt->result = OMEGA_GPU_WAIT_INVALID_STATE;
                return false;
            }
            rpt->result = OMEGA_GPU_WAIT_ABORTED;
            rpt->elapsed_ns = w[6];
            return false;
        }
#ifndef OMEGA_GPU_WAIT_MUTANT_NO_READ_BARRIER
        barrier(cfg->hook_ctx);
#endif
        m = *word;
        m2 = 0;
        snap[0] = 0;
        snap[1] = 0;
        if (cfg->marker2) m2 = *cfg->marker2;
        if (cfg->snapshot) cfg->snapshot(cfg->snapshot_ctx, snap);
        rpt->polls++;
        now = now_ns(cfg->hook_ctx);
        memset(in, 0, sizeof in);
        in[0] = (uint64_t)kind;
        in[1] = expected;
        in[2] = m;
        in[3] = m2;
        in[4] = cfg->marker2 ? 1u : 0u;
        in[5] = cfg->marker2_want;
        in[9] = now;
        in[10] = start;
        in[11] = stall;
        in[12] = stall_ns;
        in[13] = hard;
        in[14] = total_ns;
        in[15] = first ? 1u : 0u;
        in[16] = rpt->last_observed;
        in[17] = rpt->last_marker2;
        in[18] = rpt->snap[0];
        in[19] = rpt->snap[1];
        in[20] = snap[0];
        in[21] = snap[1];
        in[22] = flags;
        /* First ask without the valid hook. As in the C oracle, the hook runs
         * only when the poll did not pass, and then the unit decides again. */
        ask = eng_ask(ENG_OP_WPOLL, in, 23, w);
        if (ask == 0 && w[1] != 1 && cfg->valid) {
            in[6] = 1u;
            in[7] = cfg->valid(cfg->hook_ctx, m) ? 1u : 0u;
            ask = eng_ask(ENG_OP_WPOLL, in, 23, w);
        }
        if (ask != 0) {
            rpt->result = OMEGA_GPU_WAIT_INVALID_STATE;
            return false;
        }
        rpt->last_observed = m;
        rpt->last_marker2 = m2;
        rpt->snap[0] = snap[0];
        rpt->snap[1] = snap[1];
        rpt->elapsed_ns = now - start;
        if (w[4]) {
            rpt->progress_count++;
            rpt->last_progress_ns = w[5];
        }
        stall = w[2];
        hard = w[3];
        if (w[1] == 1) {
            barrier(cfg->hook_ctx);
            rpt->result = OMEGA_GPU_WAIT_PASS;
            return true;
        }
        if (w[1] == 2) {
            rpt->result = (omega_gpu_wait_result_t)w[0];
            return false;
        }
        first = 0;
        relax(cfg->hook_ctx);
        now = now_ns(cfg->hook_ctx);
    }
}

static bool wait_core(omega_gpu_wait_kind_t kind, volatile uint32_t *word, uint32_t expected,
                      omega_gpu_wait_report_t *rpt, const omega_gpu_wait_cfg_t *cfg) {
    omega_gpu_wait_report_t local;
    omega_gpu_wait_cfg_t zero;
    if (!rpt) rpt = &local;
    memset(rpt, 0, sizeof(*rpt));
    rpt->wait_kind = kind;
    rpt->expected = expected;
    if (!cfg) {
        memset(&zero, 0, sizeof(zero));
        cfg = &zero;
    }
    if (eng_on()) return wait_om(kind, word, expected, rpt, cfg);


    uint64_t (*now_ns)(void *) = cfg->now_ns ? cfg->now_ns : DEFAULT_NOW_NS;
    if (!now_ns) {
        rpt->result = OMEGA_GPU_WAIT_INVALID_STATE; /* no clock: a bounded wait cannot be bounded */
        return false;
    }
    void (*barrier)(void *) = cfg->barrier ? cfg->barrier : real_barrier;
    void (*relax)(void *) = cfg->relax ? cfg->relax : real_relax;
    uint64_t total_ns = 1000000ull * (cfg->total_timeout_ms ? cfg->total_timeout_ms : OMEGA_GPU_WAIT_DEFAULT_TOTAL_MS);
    uint64_t stall_ns = 1000000ull * (cfg->progress_timeout_ms ? cfg->progress_timeout_ms : OMEGA_GPU_WAIT_DEFAULT_PROGRESS_MS);

    uint64_t start = now_ns(cfg->hook_ctx);
    uint64_t hard_deadline = start + total_ns;
    uint64_t stall_deadline = start + stall_ns;
    rpt->start_ns = start;
    rpt->last_progress_ns = start;
    uint64_t now = start;
    bool first = true;

    /* A null word is a caller bug; report it as an invalid state, not a hang. */
    if (!word) {
        rpt->result = OMEGA_GPU_WAIT_INVALID_STATE;
        return false;
    }

    while (now < hard_deadline) {
        uint32_t m, m2 = 0, snap[2] = {0, 0};
        if (cfg->abort && cfg->abort(cfg->hook_ctx)) {
            rpt->result = OMEGA_GPU_WAIT_ABORTED;
            rpt->elapsed_ns = now - start;
            return false;
        }
#ifndef OMEGA_GPU_WAIT_MUTANT_NO_READ_BARRIER
        barrier(cfg->hook_ctx);
#endif
        m = *word;
        if (cfg->marker2) m2 = *cfg->marker2;
        if (cfg->snapshot) cfg->snapshot(cfg->snapshot_ctx, snap);
        rpt->polls++;
        now = now_ns(cfg->hook_ctx);

        bool changed = m != rpt->last_observed || m2 != rpt->last_marker2 ||
                       snap[0] != rpt->snap[0] || snap[1] != rpt->snap[1];
        rpt->last_observed = m;
        rpt->last_marker2 = m2;
        rpt->snap[0] = snap[0];
        rpt->snap[1] = snap[1];
        rpt->elapsed_ns = now - start;

        if (changed && !first) {
            rpt->progress_count++;
            rpt->last_progress_ns = now;
#ifndef OMEGA_GPU_WAIT_MUTANT_STALL_NO_RESET
            stall_deadline = now + stall_ns;
#endif
#ifdef OMEGA_GPU_WAIT_MUTANT_PROGRESS_EXTENDS_HARD
            hard_deadline = now + total_ns;
#endif
        }
        bool ok = kind == OMEGA_GPU_WAIT_FIXED ? (m == expected) : ((int32_t)(m - expected) >= 0);
        if (ok && (!cfg->marker2 || m2 == cfg->marker2_want)) {
            barrier(cfg->hook_ctx); /* acquire: later CPU loads are ordered after the marker load */
            rpt->result = OMEGA_GPU_WAIT_PASS;
            return true;
        }
        if (cfg->valid && !cfg->valid(cfg->hook_ctx, m)) {
            rpt->result = OMEGA_GPU_WAIT_INVALID_STATE;
            return false;
        }
        first = false;
        if (now >= stall_deadline) {
            rpt->result = OMEGA_GPU_WAIT_STALLED;
            return false;
        }
        relax(cfg->hook_ctx);
        now = now_ns(cfg->hook_ctx);
    }
    rpt->elapsed_ns = now - start;
    rpt->result = OMEGA_GPU_WAIT_TIMEOUT;
    return false;
}

bool omega_gpu_wait_fixed(volatile uint32_t *word, uint32_t expected,
                          omega_gpu_wait_report_t *rpt, const omega_gpu_wait_cfg_t *cfg) {
    return wait_core(OMEGA_GPU_WAIT_FIXED, word, expected, rpt, cfg);
}

bool omega_gpu_wait_sequence(volatile uint32_t *word, uint32_t target,
                             omega_gpu_wait_report_t *rpt, const omega_gpu_wait_cfg_t *cfg) {
    return wait_core(OMEGA_GPU_WAIT_SEQUENCE, word, target, rpt, cfg);
}
