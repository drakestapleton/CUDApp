/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/*
 * Omega GPU Engine core (A3a). See src/omega_gpu_engine.h and
 * docs/numeric/OMEGA_GPU_ENGINE.md.
 *
 * omega_gpu_execute drives the backend function table in one fixed order:
 *   open, channel, alloc x5, copy-in x3, poison fill, build      -> PREPARED
 *   submit                                                       -> SUBMITTED
 *   marker wait, marker2 wait (unless NO_C3), release wait       -> GPU_COMPLETE
 *   barrier, copy-out                                            -> OUTPUT_VISIBLE
 *   compare every output word with its poison word               -> OUTPUT_PRODUCED
 *   free every buffer (reverse order), close                     -> SUCCESS
 * Every exit runs cleanup EXCEPT an uncertain completion (any non-zero result
 * from one of the three completion waits), which keeps the context and every
 * allocation, sets the process-wide block and reports retained = 1.
 * The engine never judges the math: it only checks that output words changed.
 *
 * Marker comments of the form MUT:<name> label the lines that
 * tools/gpu_engine_mutations.sh breaks one at a time.
 */
#include "omega_gpu_engine.h"
#include "omega_eng_dec.h"

#include <stdatomic.h>
#include <string.h>
#ifdef OMEGA_GPU_NO_POSIX
/* Freestanding link target (native v1 P07 cut A, aienos#286 cut A): no errno
 * and no POSIX clock. err_no is always 0, and the millisecond clock must be
 * installed with omega_gpu_engine_set_clock before any job runs; without one
 * omega_gpu_execute refuses at STEP_CLOCK_CHECK instead of guessing. */
#define ERRNO_CLEAR() ((void)0)
#define ERRNO_READ() 0
#else
#include <errno.h>
#include <time.h>
#define ERRNO_CLEAR() (errno = 0)
#define ERRNO_READ() (errno)
#endif

static const char *const FAILURE_NAMES[OMEGA_GPU_ENGINE_FAILURE_COUNT] = {
    "OK",
    "DEVICE_OPEN",
    "CHANNEL_CREATE",
    "ALLOC",
    "PREPARE",
    "SUBMIT",
    "COMPLETION_WAIT",
    "RELEASE_WAIT",
    "VISIBILITY_WAIT",
    "OUTPUT_UNCHANGED",
    "INVALID_ARGS",
    "INTERNAL_INVARIANT",
    "CLEANUP",
    "UNCERTAIN_COMPLETION_BLOCKED",
};

static const char *const STATE_NAMES[OMEGA_GPU_ENGINE_STATE_COUNT] = {
    "INITIAL",
    "PREPARED",
    "SUBMITTED",
    "GPU_COMPLETE",
    "OUTPUT_VISIBLE",
    "OUTPUT_PRODUCED",
    "SUCCESS",
};

static const char *const WAIT_NAMES[OMEGA_GPU_ENGINE_WAIT_COUNT] = {
    "NONE",
    "MARKER",
    "RELEASE_SEMAPHORE",
    "MARKER2",
};

static const char *const STEP_NAMES[OMEGA_GPU_ENGINE_STEP_COUNT] = {
    "NONE",
    "VALIDATE",
    "BACKEND_CHECK",
    "BLOCKED",
    "NOT_IMPLEMENTED",
    "OPEN_DEVICE",
    "CREATE_CHANNEL",
    "ALLOC",
    "COPY_IN",
    "FILL_POISON",
    "BUILD",
    "SUBMIT",
    "WAIT_MARKER",
    "WAIT_MARKER2",
    "WAIT_RELEASE_SEMAPHORE",
    "BARRIER",
    "COPY_OUT",
    "SCAN",
    "FREE",
    "CLOSE",
    "CLOCK_CHECK",
};

static _Atomic(const OmegaGpuBackend *) g_backend;
static atomic_int g_blocked;
static uint64_t (*g_clock)(void); /* installed clock; NULL means CLOCK_MONOTONIC on the host, none when freestanding */

const char *omega_gpu_engine_failure_name(int failure)
{
    if (failure < 0 || failure >= (int)OMEGA_GPU_ENGINE_FAILURE_COUNT) return "UNKNOWN";
    return FAILURE_NAMES[failure];
}

const char *omega_gpu_engine_state_name(int state)
{
    if (state < 0 || state >= (int)OMEGA_GPU_ENGINE_STATE_COUNT) return "UNKNOWN";
    return STATE_NAMES[state];
}

const char *omega_gpu_engine_wait_name(int wait)
{
    if (wait < 0 || wait >= (int)OMEGA_GPU_ENGINE_WAIT_COUNT) return "UNKNOWN";
    return WAIT_NAMES[wait];
}

const char *omega_gpu_engine_step_name(int step)
{
    if (step < 0 || step >= (int)OMEGA_GPU_ENGINE_STEP_COUNT) return "UNKNOWN";
    return STEP_NAMES[step];
}

void omega_gpu_engine_set_backend(const OmegaGpuBackend *backend)
{
    atomic_store(&g_backend, backend);
}

int omega_gpu_engine_is_blocked(void)
{
    return atomic_load(&g_blocked) ? 1 : 0;
}

void omega_gpu_engine_test_reset_block(void)
{
    atomic_store(&g_blocked, 0);
}

void omega_gpu_engine_test_force_block(void)
{
    atomic_store(&g_blocked, 1);
}

void omega_gpu_engine_set_clock(uint64_t (*now_ms)(void))
{
    g_clock = now_ms;
}

void omega_gpu_engine_test_set_clock(uint64_t (*now_ms)(void))
{
    g_clock = now_ms;
}

#ifdef OMEGA_GPU_NO_POSIX
static uint64_t now_ms(void)
{
    return g_clock ? g_clock() : 0u; /* unreachable without a clock: omega_gpu_execute refuses first */
}
#else
static uint64_t mono_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0u;
    return (uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u;
}

static uint64_t now_ms(void)
{
    return g_clock ? g_clock() : mono_ms();
}
#endif

static int backend_complete(const OmegaGpuBackend *b)
{
    return b->open_device && b->create_channel && b->alloc && b->copy_in && b->fill_poison &&
           b->build && b->submit && b->wait_marker && b->wait_marker2 &&
           b->wait_release_semaphore && b->barrier && b->copy_out && b->diagnostics &&
           b->free_buf && b->close;
}

/* Returns 1 when the job is well formed for the VECTOR_1D layout. */
static int job_valid(const OmegaGpuJob *job)
{
    uint64_t bytes;
    if (!job->program || job->program_len == 0) return 0; /* MUT:VALID_PROGRAM */
    if (job->layout != OMEGA_GPU_LAYOUT_VECTOR_1D) return 0; /* MUT:VALID_LAYOUT */
    if (job->element_count == 0) return 0; /* MUT:VALID_COUNT_ZERO */
    bytes = (uint64_t)job->element_count * 4u;
    if (!job->input_a || (uint64_t)job->input_a_len != bytes) return 0;
    if (!job->input_b || (uint64_t)job->input_b_len != bytes) return 0;
    if (!job->output || (uint64_t)job->output_len != bytes) return 0;
    if (((uintptr_t)job->output & 3u) != 0) return 0; /* MUT:VALID_ALIGN */
    if (!job->poison || (uint64_t)job->poison_count != (uint64_t)job->element_count) return 0; /* MUT:VALID_POISON_COUNT */
    if ((job->flags & ~(uint32_t)OMEGA_GPU_ENGINE_FLAG_NO_C3) != 0) return 0; /* MUT:VALID_FLAGS */
    if (job->timeouts.marker_ms == 0) return 0; /* MUT:VALID_TIMEOUT_MARKER1 */
    if (job->timeouts.release_semaphore_ms == 0) return 0;
    if (job->timeouts.visibility_ms == 0) return 0;
    if ((job->flags & OMEGA_GPU_ENGINE_FLAG_NO_C3) == 0 && job->timeouts.marker2_ms == 0) return 0; /* MUT:VALID_TIMEOUT_MARKER2 */
    return 1;
}

/* What the engine still owes the device: close if opened, free each allocated buffer. */
typedef struct {
    int opened;
    int allocated[OMEGA_GPU_BUF_ROLE_COUNT];
} Run;

/* Failure before any backend call: nothing to capture, nothing to clean. */
static int stop(OmegaGpuResult *result, int failure, int step)
{
    result->failure = failure;
    result->failed_step = step;
    return failure;
}

/* Copies the backend diagnostics (driver code, text, observed words) into the result. */
static void capture(const OmegaGpuBackend *b, OmegaGpuResult *result)
{
    OmegaGpuBackendDiag d;
    memset(&d, 0, sizeof d);
    b->diagnostics(b->ctx, &d);
    result->drv_rc = d.drv_rc;
    memcpy(result->drv_text, d.drv_text, sizeof result->drv_text);
    result->drv_text[sizeof result->drv_text - 1u] = '\0';
    result->sync_valid = d.sync_valid;
    result->marker = d.marker;
    result->marker2 = d.marker2;
    memcpy(result->semaphore, d.semaphore, sizeof result->semaphore);
}

/* Records the failure of a backend call. err is errno captured right after that call. */
static int fail(const OmegaGpuBackend *b, OmegaGpuResult *result, int failure, int step, int err)
{
    capture(b, result);
    result->failure = failure;
    result->failed_step = step;
    result->err_no = err;
    return failure;
}

/* A cleanup call failed. The first failure of the run is never overwritten. */
static void cleanup_fault(const OmegaGpuBackend *b, OmegaGpuResult *result, int step, int err)
{
    if (result->failure == OMEGA_GPU_ENGINE_OK) /* MUT:CLEANUP_OVERWRITES_FIRST */
        (void)fail(b, result, OMEGA_GPU_ENGINE_CLEANUP, step, err);
    result->cleanup_failed = 1; /* MUT:CLEANUP_FLAG_NOT_SET */
}

/* Frees every allocated buffer (reverse of allocation order), then closes the
 * device. A failing call does not stop the rest. Returns result->failure. */
static int leave(const OmegaGpuBackend *b, const Run *run, OmegaGpuResult *result)
{
    int role;
    int rc;
    int e;
    for (role = (int)OMEGA_GPU_BUF_ROLE_COUNT - 1; role >= 0; role--) {
        if (!run->allocated[role]) continue; /* MUT:FREE_UNALLOCATED */
        ERRNO_CLEAR();
        rc = b->free_buf(b->ctx, role); /* MUT:FREE_SKIPPED */
        e = ERRNO_READ();
        if (rc != 0) cleanup_fault(b, result, OMEGA_GPU_ENGINE_STEP_FREE, e);
    }
    if (run->opened) {
        ERRNO_CLEAR();
        rc = b->close(b->ctx); /* MUT:CLOSE_SKIPPED */
        e = ERRNO_READ();
        if (rc != 0) cleanup_fault(b, result, OMEGA_GPU_ENGINE_STEP_CLOSE, e);
    }
    return result->failure;
}

/* A completion wait gave a non-zero result: completion is uncertain. Keep the
 * context and every allocation (no free, no close), say so, and block the process. */
static int uncertain(const OmegaGpuBackend *b, OmegaGpuResult *result, int failure, int step,
                     int wait, uint64_t limit_ms, uint64_t waited_ms, int err)
{
    (void)fail(b, result, failure, step, err);
    result->wait = wait;
    result->wait_timeout_ms = limit_ms;
    result->waited_ms = waited_ms;
    result->retained = 1; /* MUT:RETAINED_FLAG_NOT_SET */
    atomic_store(&g_blocked, 1); /* MUT:NO_BLOCK */
    return failure;
}

/* Calls a timed backend callback; captures errno right after it and measures the wait. */
static int timed_call(int (*fn)(void *, uint64_t), void *ctx, uint64_t ms, uint64_t *waited_ms,
                      int *err)
{
    uint64_t t0 = now_ms();
    uint64_t t1;
    int rc;
    ERRNO_CLEAR();
    rc = fn(ctx, ms);
    *err = ERRNO_READ();
    t1 = now_ms();
    *waited_ms = t1 >= t0 ? t1 - t0 : 0u; /* MUT:WAITED_NOT_A_DIFFERENCE */
    return rc;
}

static int copy_in_role(const OmegaGpuBackend *b, int role, const void *src, size_t bytes, int *err)
{
    int rc;
    ERRNO_CLEAR();
    rc = b->copy_in(b->ctx, role, src, bytes);
    *err = ERRNO_READ();
    return rc;
}

/* Device bytes for a buffer role. SCRATCH (pushbuffer, marker page, QMD) is the
 * backend's own business, so the engine passes 0 and the backend sizes it. */
static size_t buf_bytes(const OmegaGpuJob *job, int role)
{
    switch (role) {
    case OMEGA_GPU_BUF_PROGRAM: return job->program_len;
    case OMEGA_GPU_BUF_INPUT_A: return job->input_a_len;
    case OMEGA_GPU_BUF_INPUT_B: return job->input_b_len;
    case OMEGA_GPU_BUF_OUTPUT: return job->output_len;
    default: return 0u;
    }
}

static int eng_phase(int phase, uint64_t rc, uint64_t flags, uint64_t bit, uint64_t done,
                     uint64_t w[16]) {
    uint64_t in[8];
    memset(in, 0, sizeof in);
    in[0] = (uint64_t)phase;
    in[1] = rc;
    in[2] = flags;
    in[3] = bit;
    in[4] = done;
    return eng_ask(ENG_OP_PHASE, in, 5, w);
}

static void cleanup_om(const OmegaGpuBackend *b, OmegaGpuResult *result, int step, int err) {
    uint64_t in[1];
    uint64_t w[16];
    in[0] = (uint64_t)result->failure;
    if (eng_ask(ENG_OP_CLEAN, in, 1, w) != 0) return;
    if (w[0]) (void)fail(b, result, OMEGA_GPU_ENGINE_CLEANUP, step, err);
    if (w[1]) result->cleanup_failed = 1;
}

static int leave_om(const OmegaGpuBackend *b, const Run *run, OmegaGpuResult *result) {
    int role, rc, e;
    for (role = (int)OMEGA_GPU_BUF_ROLE_COUNT - 1; role >= 0; role--) {
        if (!run->allocated[role]) continue;
        ERRNO_CLEAR();
        rc = b->free_buf(b->ctx, role);
        e = ERRNO_READ();
        if (rc != 0) cleanup_om(b, result, OMEGA_GPU_ENGINE_STEP_FREE, e);
    }
    if (run->opened) {
        ERRNO_CLEAR();
        rc = b->close(b->ctx);
        e = ERRNO_READ();
        if (rc != 0) cleanup_om(b, result, OMEGA_GPU_ENGINE_STEP_CLOSE, e);
    }
    return result->failure;
}

static int apply_act(const OmegaGpuBackend *b, Run *run, OmegaGpuResult *result, const uint64_t w[16],
                     int err, uint64_t limit, uint64_t waited) {
    int act = (int)w[0];
    int state = (int)w[3];
    if (state != 255) result->last_state = state;
    if (act == 0 || act == 4) return 0;
    if (act == 1) return stop(result, (int)w[1], (int)w[2]);
    if (act == 3) {
        capture(b, result);
        result->failure = (int)w[1];
        result->failed_step = (int)w[2];
        result->err_no = err;
        result->wait = (int)w[4];
        result->wait_timeout_ms = limit;
        result->waited_ms = waited;
        if (w[5]) result->retained = 1;
        if (w[6]) atomic_store(&g_blocked, 1);
        return (int)w[1];
    }
    (void)fail(b, result, (int)w[1], (int)w[2], err);
    return leave_om(b, run, result);
}

static size_t role_bytes(const OmegaGpuJob *job, int role) {
    uint64_t in[5];
    uint64_t w[16];
    memset(in, 0, sizeof in);
    in[0] = (uint64_t)role;
    in[1] = job->program_len;
    in[2] = job->input_a_len;
    in[3] = job->input_b_len;
    in[4] = job->output_len;
    if (eng_ask(ENG_OP_ROLE, in, 5, w) != 0) return 0u;
    return (size_t)w[0];
}

static uint64_t scan_poison(const OmegaGpuJob *job) {
    uint64_t n = 0;
    size_t i;
    for (i = 0; i < (size_t)job->element_count; i++) {
        uint32_t word;
        uint64_t in[2];
        uint64_t w[16];
        memcpy(&word, (const unsigned char *)job->output + i * 4u, sizeof word);
        in[0] = word;
        in[1] = job->poison[i];
        if (eng_ask(ENG_OP_POIS, in, 2, w) == 0 && w[0]) n++;
    }
    return n;
}

static int timed_om(int (*fn)(void *, uint64_t), void *ctx, uint64_t ms, uint64_t *waited_ms, int *err) {
    uint64_t t0 = now_ms();
    uint64_t in[2];
    uint64_t w[16];
    int rc;
    ERRNO_CLEAR();
    rc = fn(ctx, ms);
    *err = ERRNO_READ();
    in[0] = now_ms();
    in[1] = t0;
    *waited_ms = eng_ask(ENG_OP_WAITED, in, 2, w) == 0 ? w[0] : 0u;
    return rc;
}

static int execute_om(const OmegaGpuJob *job, OmegaGpuResult *result) {
    const OmegaGpuBackend *b;
    Run run;
    uint64_t in[23];
    uint64_t w[16];
    uint64_t waited = 0;
    int rc, e = 0, role, valid = 0;

    memset(result, 0, sizeof *result);
    result->last_state = OMEGA_GPU_ENGINE_STATE_INITIAL;
    if (eng_phase(0, 0, 0, (uint64_t)omega_gpu_engine_is_blocked(), 0, w) != 0)
        return stop(result, OMEGA_GPU_ENGINE_INTERNAL_INVARIANT, OMEGA_GPU_ENGINE_STEP_NOT_IMPLEMENTED);
    if (w[0] != 0) return apply_act(NULL, NULL, result, w, 0, 0, 0);
    if (job) {
        memset(in, 0, sizeof in);
        in[0] = job->program ? 1u : 0u;
        in[1] = job->program_len;
        in[2] = (uint64_t)job->layout;
        in[3] = job->element_count;
        in[4] = job->input_a ? 1u : 0u;
        in[5] = job->input_a_len;
        in[6] = job->input_b ? 1u : 0u;
        in[7] = job->input_b_len;
        in[8] = job->output ? 1u : 0u;
        in[9] = job->output_len;
        in[10] = (uint64_t)((uintptr_t)job->output & 3u);
        in[11] = job->poison ? 1u : 0u;
        in[12] = job->poison_count;
        in[13] = job->flags;
        in[14] = job->timeouts.marker_ms;
        in[15] = job->timeouts.release_semaphore_ms;
        in[16] = job->timeouts.visibility_ms;
        in[17] = job->timeouts.marker2_ms;
        if (eng_ask(ENG_OP_JOBV, in, 18, w) == 0) valid = (int)w[0];
    }
    if (eng_phase(1, 0, 0, job ? 1u : 0u, (uint64_t)valid, w) != 0)
        return stop(result, OMEGA_GPU_ENGINE_INTERNAL_INVARIANT, OMEGA_GPU_ENGINE_STEP_VALIDATE);
    if (w[0] != 0) return apply_act(NULL, NULL, result, w, 0, 0, 0);
    b = atomic_load(&g_backend);
    if (!b) {
        if (eng_phase(2, 0, 0, 0, 0, w) != 0)
            return stop(result, OMEGA_GPU_ENGINE_INTERNAL_INVARIANT, OMEGA_GPU_ENGINE_STEP_NOT_IMPLEMENTED);
        return apply_act(NULL, NULL, result, w, 0, 0, 0);
    }
    if (eng_phase(3, 0, 0, (uint64_t)backend_complete(b), 0, w) != 0)
        return stop(result, OMEGA_GPU_ENGINE_INTERNAL_INVARIANT, OMEGA_GPU_ENGINE_STEP_BACKEND_CHECK);
    if (w[0] != 0) return apply_act(b, NULL, result, w, 0, 0, 0);
#ifdef OMEGA_GPU_NO_POSIX
    if (eng_phase(4, 0, 0, g_clock ? 1u : 0u, 0, w) != 0)
        return stop(result, OMEGA_GPU_ENGINE_INTERNAL_INVARIANT, OMEGA_GPU_ENGINE_STEP_CLOCK_CHECK);
    if (w[0] != 0) return apply_act(b, NULL, result, w, 0, 0, 0);
#endif
    memset(&run, 0, sizeof run);
    ERRNO_CLEAR();
    rc = b->open_device(b->ctx);
    e = ERRNO_READ();
    if (eng_phase(5, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, e, 0, 0);
    run.opened = 1;
    ERRNO_CLEAR();
    rc = b->create_channel(b->ctx);
    e = ERRNO_READ();
    if (eng_phase(6, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, e, 0, 0);
    for (role = 0; role < (int)OMEGA_GPU_BUF_ROLE_COUNT; role++) {
        ERRNO_CLEAR();
        rc = b->alloc(b->ctx, role, role_bytes(job, role));
        e = ERRNO_READ();
        if (eng_phase(7, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
        if (w[0] != 0) return apply_act(b, &run, result, w, e, 0, 0);
        run.allocated[role] = 1;
    }
    rc = copy_in_role(b, OMEGA_GPU_BUF_PROGRAM, job->program, job->program_len, &e);
    if (rc == 0) rc = copy_in_role(b, OMEGA_GPU_BUF_INPUT_A, job->input_a, job->input_a_len, &e);
    if (rc == 0) rc = copy_in_role(b, OMEGA_GPU_BUF_INPUT_B, job->input_b, job->input_b_len, &e);
    if (eng_phase(8, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, e, 0, 0);
    ERRNO_CLEAR();
    rc = b->fill_poison(b->ctx, job->poison, job->poison_count);
    e = ERRNO_READ();
    if (eng_phase(9, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, e, 0, 0);
    ERRNO_CLEAR();
    rc = b->build(b->ctx, job);
    e = ERRNO_READ();
    if (eng_phase(10, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, e, 0, 0);
    if (w[3] != 255) result->last_state = (int)w[3];
    ERRNO_CLEAR();
    rc = b->submit(b->ctx);
    e = ERRNO_READ();
    if (eng_phase(11, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, e, 0, 0);
    if (w[3] != 255) result->last_state = (int)w[3];
    rc = timed_om(b->wait_marker, b->ctx, job->timeouts.marker_ms, &waited, &e);
    if (eng_phase(12, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, e, job->timeouts.marker_ms, waited);
    if (eng_phase(13, 0, job->flags, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 4) {
        rc = timed_om(b->wait_marker2, b->ctx, job->timeouts.marker2_ms, &waited, &e);
        if (eng_phase(13, (uint64_t)rc, job->flags, 0, 1, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
        if (w[0] != 0) return apply_act(b, &run, result, w, e, job->timeouts.marker2_ms, waited);
    }
    rc = timed_om(b->wait_release_semaphore, b->ctx, job->timeouts.release_semaphore_ms, &waited, &e);
    if (eng_phase(14, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, e, job->timeouts.release_semaphore_ms, waited);
    if (w[3] != 255) result->last_state = (int)w[3];
    rc = timed_om(b->barrier, b->ctx, job->timeouts.visibility_ms, &waited, &e);
    if (eng_phase(15, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, e, 0, 0);
    ERRNO_CLEAR();
    rc = b->copy_out(b->ctx, job->output, job->output_len);
    e = ERRNO_READ();
    if (eng_phase(16, (uint64_t)rc, 0, 0, 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, e, 0, 0);
    if (w[3] != 255) result->last_state = (int)w[3];
    waited = scan_poison(job);
    result->n_outputs = 1u;
    result->output_unchanged_words[0] = waited;
    in[0] = waited;
    if (eng_ask(ENG_OP_SCAN, in, 1, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (eng_phase(17, 0, 0, w[0], 0, w) != 0) return OMEGA_GPU_ENGINE_INTERNAL_INVARIANT;
    if (w[0] != 0) return apply_act(b, &run, result, w, 0, 0, 0);
    if (w[3] != 255) result->last_state = (int)w[3];
    rc = leave_om(b, &run, result);
    if (eng_phase(18, (uint64_t)(result->failure != OMEGA_GPU_ENGINE_OK), 0, 0, 0, w) != 0) return rc;
    if (w[3] != 255) result->last_state = (int)w[3];
    return rc;
}

int omega_gpu_execute(const OmegaGpuJob *job, OmegaGpuResult *result)
{
    const OmegaGpuBackend *b;
    Run run;
    uint64_t waited = 0u;
    uint64_t unchanged = 0u;
    size_t i;
    int role;
    int rc;
    int e = 0;

    if (!result) return OMEGA_GPU_ENGINE_INVALID_ARGS;
    if (eng_on()) return execute_om(job, result);
    memset(result, 0, sizeof *result);
    result->failure = OMEGA_GPU_ENGINE_OK;
    result->last_state = OMEGA_GPU_ENGINE_STATE_INITIAL;
    result->failed_step = OMEGA_GPU_ENGINE_STEP_NONE;
    result->wait = OMEGA_GPU_ENGINE_WAIT_NONE;
    if (omega_gpu_engine_is_blocked()) /* MUT:BLOCK_CHECK_SKIPPED */
        return stop(result, OMEGA_GPU_ENGINE_UNCERTAIN_COMPLETION_BLOCKED,
                    OMEGA_GPU_ENGINE_STEP_BLOCKED);
    if (!job || !job_valid(job)) /* MUT:VALIDATE_SKIPPED */
        return stop(result, OMEGA_GPU_ENGINE_INVALID_ARGS, OMEGA_GPU_ENGINE_STEP_VALIDATE);
    b = atomic_load(&g_backend);
    if (!b)
        return stop(result, OMEGA_GPU_ENGINE_INTERNAL_INVARIANT, OMEGA_GPU_ENGINE_STEP_NOT_IMPLEMENTED);
    if (!backend_complete(b)) /* MUT:BACKEND_CHECK_SKIPPED */
        return stop(result, OMEGA_GPU_ENGINE_INTERNAL_INVARIANT,
                    OMEGA_GPU_ENGINE_STEP_BACKEND_CHECK);
#ifdef OMEGA_GPU_NO_POSIX
    if (!g_clock) /* no platform clock: waited_ms could not be measured; refuse before touching the device */
        return stop(result, OMEGA_GPU_ENGINE_INTERNAL_INVARIANT, OMEGA_GPU_ENGINE_STEP_CLOCK_CHECK);
#endif
    memset(&run, 0, sizeof run);

    /* INITIAL -> PREPARED */
    ERRNO_CLEAR();
    rc = b->open_device(b->ctx);
    e = ERRNO_READ();
    if (rc != 0) {
        (void)fail(b, result, OMEGA_GPU_ENGINE_DEVICE_OPEN, OMEGA_GPU_ENGINE_STEP_OPEN_DEVICE, e);
        return leave(b, &run, result);
    }
    run.opened = 1; /* MUT:OPEN_NOT_TRACKED */

    ERRNO_CLEAR();
    rc = b->create_channel(b->ctx);
    e = ERRNO_READ();
    if (rc != 0) {
        (void)fail(b, result, OMEGA_GPU_ENGINE_CHANNEL_CREATE, OMEGA_GPU_ENGINE_STEP_CREATE_CHANNEL, e);
        return leave(b, &run, result); /* MUT:CLEANUP_SKIPPED_CHANNEL */
    }

    for (role = 0; role < (int)OMEGA_GPU_BUF_ROLE_COUNT; role++) {
        ERRNO_CLEAR();
        rc = b->alloc(b->ctx, role, buf_bytes(job, role));
        e = ERRNO_READ();
        if (rc != 0) {
            (void)fail(b, result, OMEGA_GPU_ENGINE_ALLOC, OMEGA_GPU_ENGINE_STEP_ALLOC, e);
            return leave(b, &run, result); /* MUT:CLEANUP_SKIPPED_ALLOC */
        }
        run.allocated[role] = 1; /* MUT:ALLOC_NOT_TRACKED */
    }

    rc = copy_in_role(b, OMEGA_GPU_BUF_PROGRAM, job->program, job->program_len, &e);
    if (rc == 0) rc = copy_in_role(b, OMEGA_GPU_BUF_INPUT_A, job->input_a, job->input_a_len, &e);
    if (rc == 0) rc = copy_in_role(b, OMEGA_GPU_BUF_INPUT_B, job->input_b, job->input_b_len, &e);
    if (rc != 0) {
        (void)fail(b, result, OMEGA_GPU_ENGINE_PREPARE, OMEGA_GPU_ENGINE_STEP_COPY_IN, e);
        return leave(b, &run, result); /* MUT:CLEANUP_SKIPPED_COPY_IN */
    }

    ERRNO_CLEAR(); rc = b->fill_poison(b->ctx, job->poison, job->poison_count); e = ERRNO_READ(); /* MUT:POISON_FILL_SKIPPED */
    if (rc != 0) {
        (void)fail(b, result, OMEGA_GPU_ENGINE_PREPARE, OMEGA_GPU_ENGINE_STEP_FILL_POISON, e);
        return leave(b, &run, result);
    }

    ERRNO_CLEAR();
    rc = b->build(b->ctx, job);
    e = ERRNO_READ();
    if (rc != 0) {
        (void)fail(b, result, OMEGA_GPU_ENGINE_PREPARE, OMEGA_GPU_ENGINE_STEP_BUILD, e);
        return leave(b, &run, result);
    }
    result->last_state = OMEGA_GPU_ENGINE_STATE_PREPARED; /* MUT:STATE_PREPARED_SKIPPED */

    /* PREPARED -> SUBMITTED. A failed submit is a known failure, not an uncertain completion. */
    ERRNO_CLEAR();
    rc = b->submit(b->ctx);
    e = ERRNO_READ();
    if (rc != 0) {
        (void)fail(b, result, OMEGA_GPU_ENGINE_SUBMIT, OMEGA_GPU_ENGINE_STEP_SUBMIT, e);
        return leave(b, &run, result); /* MUT:CLEANUP_SKIPPED_SUBMIT */
    }
    result->last_state = OMEGA_GPU_ENGINE_STATE_SUBMITTED; /* MUT:STATE_SUBMITTED_SKIPPED */

    /* SUBMITTED -> GPU_COMPLETE: marker, marker2 (unless NO_C3), release semaphore.
     * Any non-zero result is an uncertain completion. */
    rc = timed_call(b->wait_marker, b->ctx, job->timeouts.marker_ms, &waited, &e); /* MUT:MARKER1_WAIT_SKIPPED MUT:MARKER1_TIMEOUT_WRONG */
    if (rc != 0) return uncertain(b, result, OMEGA_GPU_ENGINE_COMPLETION_WAIT, OMEGA_GPU_ENGINE_STEP_WAIT_MARKER, OMEGA_GPU_ENGINE_WAIT_MARKER, job->timeouts.marker_ms, waited, e); /* MUT:MARKER1_RESULT_IGNORED MUT:UNCERTAIN_FREES_MARKER1 */
    if (!(job->flags & OMEGA_GPU_ENGINE_FLAG_NO_C3)) { /* MUT:MARKER2_WAIT_SKIPPED MUT:C3_FLAG_IGNORED */
        rc = timed_call(b->wait_marker2, b->ctx, job->timeouts.marker2_ms, &waited, &e);
        if (rc != 0) return uncertain(b, result, OMEGA_GPU_ENGINE_COMPLETION_WAIT, OMEGA_GPU_ENGINE_STEP_WAIT_MARKER2, OMEGA_GPU_ENGINE_WAIT_MARKER2, job->timeouts.marker2_ms, waited, e); /* MUT:MARKER2_RESULT_IGNORED MUT:UNCERTAIN_FREES_MARKER2 */
    }
    rc = timed_call(b->wait_release_semaphore, b->ctx, job->timeouts.release_semaphore_ms, &waited, &e); /* MUT:RELEASE_WAIT_SKIPPED */
    if (rc != 0) return uncertain(b, result, OMEGA_GPU_ENGINE_RELEASE_WAIT, OMEGA_GPU_ENGINE_STEP_WAIT_RELEASE_SEMAPHORE, OMEGA_GPU_ENGINE_WAIT_RELEASE_SEMAPHORE, job->timeouts.release_semaphore_ms, waited, e); /* MUT:RELEASE_RESULT_IGNORED MUT:UNCERTAIN_FREES_RELEASE */
    result->last_state = OMEGA_GPU_ENGINE_STATE_GPU_COMPLETE; /* MUT:STATE_GPU_COMPLETE_SKIPPED */

    /* GPU_COMPLETE -> OUTPUT_VISIBLE: completion is known here, so a failure cleans up. */
    rc = timed_call(b->barrier, b->ctx, job->timeouts.visibility_ms, &waited, &e); /* MUT:BARRIER_SKIPPED */
    if (rc != 0) { /* MUT:BARRIER_RESULT_IGNORED */
        (void)fail(b, result, OMEGA_GPU_ENGINE_VISIBILITY_WAIT, OMEGA_GPU_ENGINE_STEP_BARRIER, e);
        return leave(b, &run, result);
    }
    ERRNO_CLEAR();
    rc = b->copy_out(b->ctx, job->output, job->output_len);
    e = ERRNO_READ();
    if (rc != 0) {
        (void)fail(b, result, OMEGA_GPU_ENGINE_VISIBILITY_WAIT, OMEGA_GPU_ENGINE_STEP_COPY_OUT, e);
        return leave(b, &run, result);
    }
    result->last_state = OMEGA_GPU_ENGINE_STATE_OUTPUT_VISIBLE; /* MUT:STATE_OUTPUT_VISIBLE_SKIPPED */

    /* OUTPUT_VISIBLE -> OUTPUT_PRODUCED: no output word may still equal its poison word. */
    for (i = 0; i < (size_t)job->element_count; i++) {
        uint32_t w;
        memcpy(&w, (const unsigned char *)job->output + i * 4u, sizeof w);
        if (w == job->poison[i]) unchanged++; /* MUT:POISON_INDEX_FIXED MUT:UNCHANGED_COUNT_SATURATES */
    }
    result->n_outputs = 1u;
    result->output_unchanged_words[0] = unchanged;
    if (unchanged != 0u) { /* MUT:UNCHANGED_CHECK_SKIPPED */
        (void)fail(b, result, OMEGA_GPU_ENGINE_OUTPUT_UNCHANGED, OMEGA_GPU_ENGINE_STEP_SCAN, 0);
        return leave(b, &run, result);
    }
    result->last_state = OMEGA_GPU_ENGINE_STATE_OUTPUT_PRODUCED; /* MUT:STATE_OUTPUT_PRODUCED_SKIPPED */

    /* OUTPUT_PRODUCED -> SUCCESS, only when cleanup also went well. */
    rc = leave(b, &run, result);
    if (rc == OMEGA_GPU_ENGINE_OK) result->last_state = OMEGA_GPU_ENGINE_STATE_SUCCESS; /* MUT:STATE_SUCCESS_SKIPPED MUT:SUCCESS_AFTER_CLEANUP_FAIL */
    return rc;
}
