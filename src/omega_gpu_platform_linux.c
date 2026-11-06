/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_platform_linux.c -- Linux REFERENCE implementation of OmegaGpuPlatform 1.1. REFERENCE ONLY: see the
 * header. Every driver call here is the one src/omega_gpu_session.c made directly before P07 cut C, in the same
 * order, so the launch path on GB10 under Linux is unchanged:
 *   device_open  = nvrm_open + m16_native_create_channel   (session open, before the four session buffers)
 *   dma_alloc    = nvrm_alloc_gpu_uncached                  (every session / API buffer)
 *   dma_free     = nvrm_free
 *   queue_open   = ctx.pb_mem = the ring buffer             (m16_native_enqueue_methods copies methods there)
 *   queue_submit = m16_native_submit_methods               (refused BUSY while the last one is unretired)
 *   queue_retire = nvrm_retire(&rm, rm.put)                 (after both markers landed; no-op when nothing is in flight)
 *   poll_word    = m16_native_wait_marker (>= serial compare, 50 us sleep-poll)
 *   sync_*       = dsb sy                                   (the barrier the session issued itself)
 *   device_close = m16_native_close (nvrm_close)
 * Every submission copies its methods to offset 0 of the one ring buffer, and Blackwell USERD has no GPGet the
 * client could read (nvrm/nvrm.c, nvrm_submit), so nothing tells the reference which entry the GPU has fetched.
 * caps.max_inflight is therefore 1 and enforced: a submit while the previous one is unretired is refused
 * REFUSED_BUSY before the ring is written or the doorbell rung (CudaAuditFable REPORT.md Q6), and one retire after
 * the caller observed completion covers everything submitted.
 */
#include "omega_gpu_platform_linux.h"
#include "cpu_barrier.h"

#include "m16_native.h"
#include "nvrm.h"
#include "omega_ampere_qmd.h"
#include "omega_gpu_mem_reserve.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#if defined(CUDAPP_MIGRATION_PENDING_C_PLATFORM_LINUX)
#include "omega_gpu_platform_linux_cpolicy.h"
#else
uint32_t omega_plat_policy(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
#endif
#if !defined(CUDAPP_MIGRATION_PENDING_C_PLATFORM_LINUX)
#include "cudapp_plat_effects.h"
#endif

/* CUDAPP_MIGRATION_PENDING_C_PLATFORM_LINUX defaults OFF. The .om unit is the path; C is the oracle. */
static uint64_t plat_decide(const uint64_t m_in[16], uint64_t q_out[16]) {
    uint64_t q[16];
    memset(q, 0, sizeof q);
#if defined(CUDAPP_MIGRATION_PENDING_C_PLATFORM_LINUX)
    uint64_t result = plat_c(m_in, q);
#else
    uint64_t m[16], result = 0;
    memcpy(m, m_in, sizeof m);
    if (omega_plat_policy(&result, m, 16, q, 16) != 0) return 0;
#endif
    if (q_out) memcpy(q_out, q, sizeof q);
    return result;
}

static void plat_l2_words(uint32_t out[5]) {
#if defined(CUDAPP_MIGRATION_PENDING_C_PLATFORM_LINUX)
    const uint32_t words[5] = { (2u << 28) | (4u << 16) | (0x0028u >> 2), 0, 0, 0, 0x11u << 27 };
    memcpy(out, words, sizeof words);
#else
    uint64_t m[16], q[16];
    memset(m, 0, sizeof m);
    m[0] = 1;
    (void)plat_decide(m, q);
    for (int i = 0; i < 5; i++) out[i] = (uint32_t)q[i];
#endif
}

#if !defined(CUDAPP_MIGRATION_PENDING_C_PLATFORM_LINUX) && !defined(CUDAPP_MIGRATION_PENDING_C_WAIT_MARKER)
/* Same contract as nvrm/native_client.h nvrm_live_token (Streams). One live
 * unquarantined owner, or -1 with EINVAL/EFAULT. Delete this copy when that
 * header is on this branch; do not add a second mapping table. */
static int plat_live_token(const Nvrm *rm, const void *p, size_t n, uint64_t *token, uint64_t *offset) {
    if (!rm || !p || !n || !token || !offset || !rm->omega_context) { errno = EINVAL; return -1; }
    uintptr_t a = (uintptr_t)p;
    if (a > UINTPTR_MAX - n) { errno = EINVAL; return -1; }
    uint32_t hits = 0;
    for (uint32_t i = 0; i < rm->live_count; i++) {
        const NvrmLiveAlloc *l = &rm->live[i];
        uintptr_t b = (uintptr_t)l->cpu;
        if (!l->cpu_live || l->quarantined || !l->omega_mapping || !b) continue;
        if (a < b || a + n > b + l->size) continue;
        *token = l->omega_mapping;
        *offset = a - b;
        hits++;
    }
    if (hits != 1) { errno = EFAULT; return -1; }
    return 0;
}
#endif

#define LINUX_REF_BUFFERS (OMEGA_GPU_HANDLE_SLOTS - 2u) /* one slot each for the device and the queue */

typedef struct {
    pthread_mutex_t mu; /* table, device and diagnostics */
    pthread_mutex_t locks[OMEGA_GPU_PLATFORM_LINUX_LOCKS];
    int ready;
    OmegaGpuHandleTable t;
    NvrmMem mem[OMEGA_GPU_HANDLE_SLOTS];
    uint32_t rights[OMEGA_GPU_HANDLE_SLOTS];
    uint8_t gpu_cached[OMEGA_GPU_HANDLE_SLOTS]; /* allocated GPU-cached (nvrm_alloc), omega #457 */
    int l2_inval_pending; /* a host sync into a GPU-cached buffer since the last submission, omega #457 */
    uint32_t inflight;    /* submissions not yet retired: 0 or 1 (caps.max_inflight) */
    M16NativeContext m16;
    OmegaGpuDeviceHandle dev; /* generation 0 = closed */
    OmegaGpuQueueHandle queue;
    /* L136: extra queues on extra channels of the same channel group (nvrm_channel_n), opened only when
     * AIEN_OMEGA_GPU_CHANNELS > 1 at device_open; queue 0 above is unchanged. */
    uint32_t nchan;
    OmegaGpuQueueHandle xq[NVRM_MAX_EXTRA_CHANNELS];
    NvrmMem xring[NVRM_MAX_EXTRA_CHANNELS];
    uint8_t x_l2_pending[NVRM_MAX_EXTRA_CHANNELS];
    uint32_t xinflight[NVRM_MAX_EXTRA_CHANNELS]; /* per extra queue, as inflight above (caps.max_inflight is per queue) */
    char diag[320];
} LinuxRef;

static LinuxRef g_ref = {
    .mu = PTHREAD_MUTEX_INITIALIZER,
    .locks = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER},
};

/* omega #457: MEM_OP_A..D = 0, 0, 0, OPERATION L2_SYSMEM_NCOH_INVALIDATE 0x11 in D bits 31:27 (clc96f.h:30-73), the
 * same method form as the C3 tail's L2_FLUSH_DIRTY. ref_queue_submit puts it ahead of the next submission after a host
 * sync into a GPU-cached buffer; g_caps.submit_prefix_words is its size, so callers budget exactly what is written. */
static const uint32_t L2_INVAL_PREFIX[] = { (2u << 28) | (4u << 16) | (0x0028u >> 2), 0, 0, 0, 0x11u << 27 };
#define L2_INVAL_PREFIX_WORDS ((uint32_t)(sizeof L2_INVAL_PREFIX / sizeof L2_INVAL_PREFIX[0]))

static const OmegaGpuPlatformCaps g_caps = {
    .submit_prefix_words = L2_INVAL_PREFIX_WORDS,
    .abi_major = OMEGA_GPU_PLATFORM_ABI_MAJOR,
    .abi_minor = OMEGA_GPU_PLATFORM_ABI_MINOR,
    .services = OMEGA_GPU_SVC_REQUIRED_SESSION, /* no MMIO (RM owns the registers), no IRQ (polling only) */
    .max_devices = 1,
    .max_buffers = LINUX_REF_BUFFERS,
    .max_queues = 1u + NVRM_MAX_EXTRA_CHANNELS, /* queues past the first need AIEN_OMEGA_GPU_CHANNELS (L136) */
    .max_inflight = 1,
    .max_buffer_bytes = 1ull << 37,  /* 128 GiB: the whole unified pool */
    .max_total_bytes = 1ull << 37,
};

static void ref_lock(LinuxRef *r) {
    pthread_mutex_lock(&r->mu);
    if (!r->ready) { omega_gpu_handles_init(&r->t); r->ready = 1; }
}
static void ref_unlock(LinuxRef *r) { pthread_mutex_unlock(&r->mu); }

/* Records why the call failed; REFUSED_DEVICE carries the driver text (Nvrm.err). Called with r->mu held. */
static int refuse(LinuxRef *r, int rc, const char *stage) {
    if (rc == OMEGA_GPU_PLAT_REFUSED_DEVICE)
        snprintf(r->diag, sizeof r->diag, "%s: %s", stage, r->m16.rm.err[0] ? r->m16.rm.err : "(no driver text)");
    else
        snprintf(r->diag, sizeof r->diag, "%s: %s", stage, omega_gpu_platform_refusal_name(rc));
    return rc;
}

static int dev_ok(LinuxRef *r, OmegaGpuDeviceHandle d) {
    return omega_gpu_handles_check(&r->t, d.index, d.generation, OMEGA_GPU_HANDLE_DEVICE);
}

/* A live buffer handle of any buffer kind; *kind gets it. */
static int buf_ok(LinuxRef *r, OmegaGpuBufferHandle b, uint32_t *kind) {
    if (b.index >= OMEGA_GPU_HANDLE_SLOTS || b.generation == 0u) return OMEGA_GPU_PLAT_REFUSED_BAD_HANDLE;
    if (!r->t.live[b.index] || r->t.generation[b.index] != b.generation) return OMEGA_GPU_PLAT_REFUSED_STALE_HANDLE;
    const uint32_t k = r->t.kind[b.index];
    if (k != OMEGA_GPU_HANDLE_BUF_DATA && k != OMEGA_GPU_HANDLE_BUF_CODE && k != OMEGA_GPU_HANDLE_BUF_QUEUE)
        return OMEGA_GPU_PLAT_REFUSED_BAD_HANDLE;
    if (kind) *kind = k;
    return OMEGA_GPU_PLAT_OK;
}

static int range_ok(LinuxRef *r, OmegaGpuBufferHandle b, uint64_t off, uint64_t len) {
    const uint64_t m[16] = {3, r->mem[b.index].size, off, len};
    return (int)plat_decide(m, NULL);
}

static uint64_t ref_now_ns(void *ctx) {
    (void)ctx;
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec;
}
static uint64_t ref_now_ms(void *ctx) { return ref_now_ns(ctx) / 1000000ull; }

static int ref_wait_ms(void *ctx, uint64_t ms) {
    (void)ctx;
    struct timespec t = {.tv_sec = (time_t)(ms / 1000u), .tv_nsec = (long)(ms % 1000u) * 1000000L};
    while (nanosleep(&t, &t) != 0) {}
    return OMEGA_GPU_PLAT_OK;
}

static int ref_lock_id(void *ctx, uint32_t id) {
    LinuxRef *r = ctx;
    if (id >= OMEGA_GPU_PLATFORM_LINUX_LOCKS) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    return pthread_mutex_lock(&r->locks[id]) == 0 ? OMEGA_GPU_PLAT_OK : OMEGA_GPU_PLAT_REFUSED_BUSY;
}
static int ref_unlock_id(void *ctx, uint32_t id) {
    LinuxRef *r = ctx;
    if (id >= OMEGA_GPU_PLATFORM_LINUX_LOCKS) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    return pthread_mutex_unlock(&r->locks[id]) == 0 ? OMEGA_GPU_PLAT_OK : OMEGA_GPU_PLAT_REFUSED_ARGS;
}

uint32_t omega_gpu_platform_linux_channels_env(void) {
    const char *e = getenv("AIEN_OMEGA_GPU_CHANNELS");
    uint64_t m[16] = {2, 0, 0, 0};
    if (e) {
        size_t n = strlen(e);
        m[1] = n > 32 ? 33 : n;
        if (n > 0) m[2] = (unsigned char)e[0];
        if (n > 1) m[3] = (unsigned char)e[1];
    }
    return (uint32_t)plat_decide(m, NULL);
}

/* Extra queue slot of q (0..nchan-2), or -1 when q is not an open extra queue. Caller holds r->mu. */
static int xq_slot(const LinuxRef *r, OmegaGpuQueueHandle q) {
    uint64_t m[16] = {12, r->nchan, q.index, q.generation};
    for (uint32_t i = 0; i < NVRM_MAX_EXTRA_CHANNELS; i++) {
        m[4 + i * 2] = r->xq[i].index;
        m[5 + i * 2] = r->xq[i].generation;
    }
    uint64_t slot = plat_decide(m, NULL);
    return slot ? (int)slot - 1 : -1;
}

static int ref_device_open(void *ctx, uint32_t ordinal, OmegaGpuDeviceHandle *out) {
    LinuxRef *r = ctx;
    int rc;
    if (!out) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    memset(out, 0, sizeof *out);
    ref_lock(r);
    if (ordinal != 0u) { rc = refuse(r, OMEGA_GPU_PLAT_REFUSED_ARGS, "device_open"); goto out; }
    if (r->dev.generation != 0u) { rc = refuse(r, OMEGA_GPU_PLAT_REFUSED_LIMIT, "device_open (already open)"); goto out; }
    memset(&r->m16, 0, sizeof r->m16);
    /* nvrm_open directly (m16_native_open is memset + nvrm_open + nvrm_close on failure) so the driver's own
     * failure text survives into the stage name, as the session did before cut C. */
    if (nvrm_open(&r->m16.rm) != 0) {
        rc = refuse(r, OMEGA_GPU_PLAT_REFUSED_DEVICE, "nvrm_open");
        (void)nvrm_close(&r->m16.rm);
        goto out;
    }
    r->nchan = omega_gpu_platform_linux_channels_env();
    if (r->nchan == 1u ? m16_native_create_channel(&r->m16) != 0 : nvrm_channel_n(&r->m16.rm, r->nchan - 1u) != 0) {
        rc = refuse(r, OMEGA_GPU_PLAT_REFUSED_DEVICE, r->nchan == 1u ? "m16_native_create_channel" : "nvrm_channel_n");
        (void)m16_native_close(&r->m16);
        goto out;
    }
    rc = omega_gpu_handles_open(&r->t, OMEGA_GPU_HANDLE_DEVICE, 0, 1, 0, &out->index, &out->generation);
    if (rc != OMEGA_GPU_PLAT_OK) { (void)m16_native_close(&r->m16); rc = refuse(r, rc, "device_open"); goto out; }
    r->dev = *out;
    r->diag[0] = '\0';
out:
    ref_unlock(r);
    return rc;
}

static int ref_device_close(void *ctx, OmegaGpuDeviceHandle dev) {
    LinuxRef *r = ctx;
    ref_lock(r);
    int rc = dev_ok(r, dev);
    if (rc != OMEGA_GPU_PLAT_OK) { rc = refuse(r, rc, "device_close"); ref_unlock(r); return rc; }
    /* Every handle of the device goes stale. The RM client close below releases the driver objects; buffers the
     * caller did not free are not unmapped here, exactly as before cut C. */
    for (uint32_t i = 0; i < OMEGA_GPU_HANDLE_SLOTS; i++) {
        if (!r->t.live[i] || i == dev.index) continue;
        (void)omega_gpu_handles_close(&r->t, i, r->t.generation[i], r->t.kind[i]);
        memset(&r->mem[i], 0, sizeof r->mem[i]);
        r->rights[i] = 0;
        r->gpu_cached[i] = 0;
    }
    (void)omega_gpu_handles_close(&r->t, dev.index, dev.generation, OMEGA_GPU_HANDLE_DEVICE);
    memset(&r->dev, 0, sizeof r->dev);
    r->l2_inval_pending = 0; /* omega #457: nothing pending survives the device */
    r->inflight = 0;
    memset(&r->queue, 0, sizeof r->queue);
    memset(r->xq, 0, sizeof r->xq); memset(r->xring, 0, sizeof r->xring); memset(r->x_l2_pending, 0, sizeof r->x_l2_pending);
    memset(r->xinflight, 0, sizeof r->xinflight);
    r->nchan = 0;
    /* C5: the driver's close result is the proof that this process's channels (and any grid still on them) are gone.
     * The state above is closed either way; a teardown fault is reported so omega_gpu_session_recover stays latched. */
    rc = m16_native_close(&r->m16) == 0 ? OMEGA_GPU_PLAT_OK : refuse(r, OMEGA_GPU_PLAT_REFUSED_DEVICE, "device_close (driver teardown fault)");
    ref_unlock(r);
    return rc;
}

static const char *ref_diag_text(void *ctx) { return ((LinuxRef *)ctx)->diag; }

/* Developer bar D1 (docs/PAIN_POINTS.md). Shared host memory goes through src/omega_gpu_mem_reserve.c, shared with the
 * aien-gpu platform: it runs under ref_lock, and every meminfo open there is O_NOFOLLOW|O_NONBLOCK and must reach a
 * regular file, so it cannot block on a FIFO or act on a device. Local VRAM on a discrete GPU is checked here against
 * the selected framebuffer's free memory, with CUDAPP_GPU_MIN_FREE_BYTES (bytes) replacing the reserve when set. */
static int mem_reserve_check(LinuxRef *r, uint64_t bytes, int local) {
    if (!local) return omega_gpu_mem_reserve_check(bytes, r->diag, sizeof r->diag);
    uint64_t reserve = omega_gpu_mem_reserve_bytes();
    const char *minimum = getenv("CUDAPP_GPU_MIN_FREE_BYTES");
    if (minimum && *minimum) {
        uint64_t pm[16] = {7};
        size_t n = strlen(minimum);
        pm[1] = n > 32 ? 33 : n;
        if (n <= 32) memcpy(&pm[2], minimum, n);
        uint64_t pq[16];
        if (n > 32 || plat_decide(pm, pq) != 1) {
            snprintf(r->diag, sizeof r->diag, "dma_alloc: invalid CUDAPP_GPU_MIN_FREE_BYTES");
            return OMEGA_GPU_PLAT_REFUSED_LIMIT;
        }
        reserve = pq[0];
    }
    if (!reserve) return OMEGA_GPU_PLAT_OK;
    uint64_t avail = 0, total = 0;
    uint64_t page_m[16] = {5, bytes};
    uint64_t page_q[16];
    if (plat_decide(page_m, page_q) != 1 ||
        nvrm_vram_info(&r->m16.rm, &avail, &total) != 0) {
        snprintf(r->diag, sizeof r->diag, "dma_alloc: refused: selected GPU framebuffer free-memory query failed");
        return OMEGA_GPU_PLAT_REFUSED_LIMIT;
    }
    bytes = page_q[0];
    const uint64_t m[16] = {2, avail, bytes, reserve};
    if (omega_ampere_policy(m, NULL) == 0) return OMEGA_GPU_PLAT_OK;
    snprintf(r->diag, sizeof r->diag,
             "dma_alloc: refused local VRAM allocation: free=%llu bytes request=%llu bytes reserve=%llu bytes; host RAM is not VRAM",
             (unsigned long long)avail, (unsigned long long)bytes, (unsigned long long)reserve);
    return OMEGA_GPU_PLAT_REFUSED_LIMIT;
}

static int ref_dma_alloc(void *ctx, OmegaGpuDeviceHandle dev, const OmegaGpuBufferDesc *desc, OmegaGpuBufferHandle *out,
                         OmegaCpuView *cpu, OmegaGpuVa *gpu, OmegaGpuBusAddr *bus) {
    LinuxRef *r = ctx;
    if (!desc || !out || !cpu || !gpu) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    memset(out, 0, sizeof *out); memset(cpu, 0, sizeof *cpu); memset(gpu, 0, sizeof *gpu);
    if (bus) memset(bus, 0, sizeof *bus);
    ref_lock(r);
    int rc = dev_ok(r, dev);
    if (rc == OMEGA_GPU_PLAT_OK) rc = omega_gpu_platform_check_buffer(desc, &g_caps);
    /* GPU-uncached by default. GPU-cached only for DEVICE-PRIVATE data buffers (OWN-30B T3b, 2026-10-13): no CPU_READ, no
     * HOST_POLLED, so the CPU never reads a line the GPU may still hold in L2 (nvos.h:1116 "For system memory this will not be
     * coherent with direct CPU mappings"). omega_gpu_mem_cached.c (OWN-30B L2) writes such a buffer from the CPU exactly once,
     * before any GPU read, then syncs; omega_gpu_devmem.h (T3b) fills them with GPU writes only. Either way no stale L2 line exists. */
    uint64_t am[16] = {4, (uint64_t)r->m16.rm.discrete, desc->kind, desc->rights,
                       desc->gpu_cache == OMEGA_GPU_CACHE_GPU_CACHED, (uint64_t)(rc < 0 ? 0 : rc)};
    uint64_t aq[16];
    (void)plat_decide(am, aq);
    rc = (int)aq[0];
    const int local = (int)aq[1];
    const int cached = (int)aq[2];
    const int alloc_class = (int)aq[3];
    if (rc != OMEGA_GPU_PLAT_OK) { rc = refuse(r, rc, "dma_alloc"); ref_unlock(r); return rc; }
    rc = mem_reserve_check(r, desc->bytes, local); /* D1 before handle/driver allocation; the sentence is in r->diag */
    if (rc != OMEGA_GPU_PLAT_OK) { ref_unlock(r); return rc; }
    uint32_t idx = 0, gen = 0;
    rc = omega_gpu_handles_open(&r->t, desc->kind, desc->bytes, LINUX_REF_BUFFERS, g_caps.max_total_bytes, &idx, &gen);
    if (rc != OMEGA_GPU_PLAT_OK) { rc = refuse(r, rc, "dma_alloc"); ref_unlock(r); return rc; }
    NvrmMem m;
    if ((alloc_class == 1 ? nvrm_alloc_vram(&r->m16.rm, desc->bytes, &m) :
         alloc_class == 2 ? nvrm_alloc(&r->m16.rm, desc->bytes, &m) : nvrm_alloc_gpu_uncached(&r->m16.rm, desc->bytes, &m)) != 0) {
        (void)omega_gpu_handles_close(&r->t, idx, gen, desc->kind); /* this slot never got memory: its generation moves on */
        rc = refuse(r, OMEGA_GPU_PLAT_REFUSED_DEVICE, cached ? "nvrm_alloc" : "nvrm_alloc_gpu_uncached");
        ref_unlock(r);
        return rc;
    }
    r->mem[idx] = m;
    r->rights[idx] = desc->rights;
    r->gpu_cached[idx] = (uint8_t)cached;
    out->index = idx; out->generation = gen;
    cpu->ptr = m.cpu; cpu->len = (size_t)m.size;
    gpu->va = m.va;
    ref_unlock(r);
    return OMEGA_GPU_PLAT_OK;
}

static int ref_dma_free(void *ctx, OmegaGpuBufferHandle b) {
    LinuxRef *r = ctx;
    uint32_t kind = 0;
    ref_lock(r);
    int rc = buf_ok(r, b, &kind);
    if (rc == OMEGA_GPU_PLAT_OK && r->queue.generation != 0u && r->m16.pb_mem.handle == r->mem[b.index].handle &&
        r->m16.pb_mem.cpu == r->mem[b.index].cpu)
        rc = OMEGA_GPU_PLAT_REFUSED_BUSY; /* the open queue still uses it as its ring */
    for (uint32_t i = 0; rc == OMEGA_GPU_PLAT_OK && i + 1u < r->nchan; i++)
        if (r->xq[i].generation != 0u && r->xring[i].handle == r->mem[b.index].handle) rc = OMEGA_GPU_PLAT_REFUSED_BUSY;
    if (rc != OMEGA_GPU_PLAT_OK) { rc = refuse(r, rc, "dma_free"); ref_unlock(r); return rc; }
    if (nvrm_free(&r->m16.rm, &r->mem[b.index]) != 0) {
        rc = refuse(r, OMEGA_GPU_PLAT_REFUSED_DEVICE, "nvrm_free (allocation retained for cleanup)");
        ref_unlock(r);
        return rc;
    }
    (void)omega_gpu_handles_close(&r->t, b.index, b.generation, kind);
    memset(&r->mem[b.index], 0, sizeof r->mem[b.index]);
    r->rights[b.index] = 0;
    r->gpu_cached[b.index] = 0;
    ref_unlock(r);
    return OMEGA_GPU_PLAT_OK;
}

#if defined(CUDAPP_MIGRATION_PENDING_C_PLATFORM_LINUX)
static int plat_publish(void *dst, const void *src, uint64_t n) {
    memcpy(dst, src, (size_t)n);
    return 0;
}
static int plat_fence(void) {
    nvrm_cpu_barrier();
    return 0;
}
#else
static int plat_publish(void *dst, const void *src, uint64_t n) {
    uint64_t wrote = 0;
    if (n == 0) return 0;
    if (omega2_plat_ring_store(&wrote, dst, n, src, n, n) != 0 || wrote != n) return -1;
    return 0;
}
static int plat_fence(void) {
    uint64_t result = 0;
    return omega2_plat_fence(&result) == 0 && result == 1 ? 0 : -1;
}
#endif

static int ref_sync(void *ctx, OmegaGpuBufferHandle b, uint64_t off, uint64_t len, const char *stage, int to_gpu) {
    LinuxRef *r = ctx;
    ref_lock(r);
    int rc = buf_ok(r, b, NULL);
    if (rc == OMEGA_GPU_PLAT_OK) rc = range_ok(r, b, off, len);
    if (rc != OMEGA_GPU_PLAT_OK) rc = refuse(r, rc, stage);
    /* omega #457 (chip, 2026-10-15): after the host rewrites a GPU-cached buffer the GPU already read, every later launch
     * read the old bytes from L2 (nvos.h:1117 "For system memory this will not be coherent with direct CPU mappings").
     * The next submission therefore starts with L2_SYSMEM_NCOH_INVALIDATE (ref_queue_submit). */
    else {
        uint64_t pm[16] = {13, (uint64_t)to_gpu, r->gpu_cached[b.index]};
        if (plat_decide(pm, NULL)) { r->l2_inval_pending = 1; memset(r->x_l2_pending, 1, sizeof r->x_l2_pending); }
    }
    ref_unlock(r);
    if (rc == OMEGA_GPU_PLAT_OK && plat_fence() != 0) rc = OMEGA_GPU_PLAT_REFUSED_DEVICE;
    return rc;
}
static int ref_sync_to_gpu(void *ctx, OmegaGpuBufferHandle b, uint64_t off, uint64_t len) { return ref_sync(ctx, b, off, len, "sync_cpu_to_gpu", 1); }
static int ref_sync_to_cpu(void *ctx, OmegaGpuBufferHandle b, uint64_t off, uint64_t len) { return ref_sync(ctx, b, off, len, "sync_gpu_to_cpu", 0); }

static int ref_poll_word(void *ctx, OmegaGpuBufferHandle b, uint64_t off, uint32_t expected, uint64_t timeout_ms, uint32_t *observed) {
    LinuxRef *r = ctx;
    ref_lock(r);
    int rc = buf_ok(r, b, NULL);
    if (rc == OMEGA_GPU_PLAT_OK) {
        uint64_t pm[16] = {10, r->rights[b.index], off};
        rc = (int)plat_decide(pm, NULL);
    }
    if (rc == OMEGA_GPU_PLAT_OK) rc = range_ok(r, b, off, 4u);
    if (rc != OMEGA_GPU_PLAT_OK) { rc = refuse(r, rc, "poll_word"); ref_unlock(r); return rc; }
    volatile uint32_t *w = (volatile uint32_t *)((uint8_t *)r->mem[b.index].cpu + off);
#if !defined(CUDAPP_MIGRATION_PENDING_C_PLATFORM_LINUX) && !defined(CUDAPP_MIGRATION_PENDING_C_WAIT_MARKER)
    uint64_t token = 0, base = 0, polls = 0, result = 0;
    int have = plat_live_token(&r->m16.rm, (const void *)w, 4, &token, &base) == 0
               && omega2_plat_wait_budget(&polls, timeout_ms) == 0;
    ref_unlock(r);
    if (have) {
        uint32_t st = omega2_ctx_plat_wait_ge(r->m16.rm.omega_context, &result, token, base, expected, polls);
        rc = (st == 0 && (int64_t)result == 0) ? OMEGA_GPU_PLAT_OK : OMEGA_GPU_POLL_TIMED_OUT;
    } else
        rc = m16_native_wait_marker(w, expected, timeout_ms) == 0 ? OMEGA_GPU_PLAT_OK : OMEGA_GPU_POLL_TIMED_OUT;
#else
    ref_unlock(r); /* never hold the table lock while polling */
    rc = m16_native_wait_marker(w, expected, timeout_ms) == 0 ? OMEGA_GPU_PLAT_OK : OMEGA_GPU_POLL_TIMED_OUT;
#endif
    if (observed) *observed = *w;
    return rc;
}

static int queue_ok(LinuxRef *r, OmegaGpuQueueHandle q) {
    return omega_gpu_handles_check(&r->t, q.index, q.generation, OMEGA_GPU_HANDLE_QUEUE);
}

static int ref_queue_open(void *ctx, OmegaGpuDeviceHandle dev, OmegaGpuBufferHandle ring, OmegaGpuQueueHandle *out) {
    LinuxRef *r = ctx;
    uint32_t kind = 0;
    if (!out) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    memset(out, 0, sizeof *out);
    ref_lock(r);
    int rc = dev_ok(r, dev);
    if (rc == OMEGA_GPU_PLAT_OK) rc = buf_ok(r, ring, &kind);
    if (rc == OMEGA_GPU_PLAT_OK && kind != OMEGA_GPU_HANDLE_BUF_QUEUE) rc = OMEGA_GPU_PLAT_REFUSED_BAD_KIND;
    /* queue 0 first (channel 0); then, with AIEN_OMEGA_GPU_CHANNELS > 1, one queue per extra channel (L136) */
    int slot = -1;
    if (rc == OMEGA_GPU_PLAT_OK && r->queue.generation != 0u) {
        for (uint32_t i = 0; i + 1u < r->nchan && slot < 0; i++) if (r->xq[i].generation == 0u) slot = (int)i;
        if (slot < 0) rc = OMEGA_GPU_PLAT_REFUSED_LIMIT;
    }
    if (rc == OMEGA_GPU_PLAT_OK) rc = omega_gpu_handles_open(&r->t, OMEGA_GPU_HANDLE_QUEUE, 0, r->nchan, 0, &out->index, &out->generation); /* one queue per channel (L136: nchan is 1 unless AIEN_OMEGA_GPU_CHANNELS) */
    if (rc != OMEGA_GPU_PLAT_OK) { rc = refuse(r, rc, "queue_open"); ref_unlock(r); return rc; }
    if (slot >= 0) { r->xring[slot] = r->mem[ring.index]; r->xq[slot] = *out; r->x_l2_pending[slot] = (uint8_t)r->l2_inval_pending; r->xinflight[slot] = 0; ref_unlock(r); return OMEGA_GPU_PLAT_OK; }
    r->m16.pb_mem = r->mem[ring.index]; /* m16_native_enqueue_methods copies the method stream here */
    r->queue = *out;
    r->inflight = 0;
    ref_unlock(r);
    return OMEGA_GPU_PLAT_OK;
}

static int ref_queue_close(void *ctx, OmegaGpuQueueHandle q) {
    LinuxRef *r = ctx;
    ref_lock(r);
    int rc = queue_ok(r, q);
    if (rc == OMEGA_GPU_PLAT_OK) rc = omega_gpu_handles_close(&r->t, q.index, q.generation, OMEGA_GPU_HANDLE_QUEUE);
    if (rc != OMEGA_GPU_PLAT_OK) { rc = refuse(r, rc, "queue_close"); ref_unlock(r); return rc; }
    const int slot = xq_slot(r, q);
    if (slot >= 0) { memset(&r->xq[slot], 0, sizeof r->xq[slot]); memset(&r->xring[slot], 0, sizeof r->xring[slot]); r->xinflight[slot] = 0; ref_unlock(r); return OMEGA_GPU_PLAT_OK; }
    memset(&r->queue, 0, sizeof r->queue);
    r->inflight = 0;
    memset(&r->m16.pb_mem, 0, sizeof r->m16.pb_mem);
    ref_unlock(r);
    return OMEGA_GPU_PLAT_OK;
}

static int ref_queue_submit(void *ctx, OmegaGpuQueueHandle q, const uint32_t *words, uint32_t nwords) {
    LinuxRef *r = ctx;
    ref_lock(r);
    int rc = queue_ok(r, q);
    if (rc == OMEGA_GPU_PLAT_OK && (!words || nwords == 0u)) rc = OMEGA_GPU_PLAT_REFUSED_ARGS;
    /* CudaAuditFable Q6: every ring (channel 0 and each L136 extra channel) is rewritten from offset 0 by every
     * submission, so a second one while the first is unretired would overwrite methods the GPU may still be fetching.
     * caps.max_inflight is per queue; refused before any copy or doorbell. */
    const char *stage = "submit_methods";
    const int slot = rc == OMEGA_GPU_PLAT_OK ? xq_slot(r, q) : -1;
    if (rc == OMEGA_GPU_PLAT_OK) {
        uint64_t im[16] = {8, slot >= 0 ? r->xinflight[slot] : r->inflight, g_caps.max_inflight};
        if (plat_decide(im, NULL) != 0) {
            rc = OMEGA_GPU_PLAT_REFUSED_BUSY;
            stage = "queue_submit (the previous submission is not retired; caps.max_inflight is 1)";
        }
    }
    if (rc == OMEGA_GPU_PLAT_OK && slot >= 0) {
        /* an extra channel: the same #457 prefix rule (its own pending flag), the same copy-to-ring-offset-0 + one GP
         * entry + doorbell as m16_native_submit_methods, on channel slot + 1 */
        NvrmMem *ring = &r->xring[slot];
        const uint32_t xpre = r->x_l2_pending[slot] ? L2_INVAL_PREFIX_WORDS : 0u;
        uint32_t prefix[5];
        plat_l2_words(prefix);
        uint64_t fm[16] = {9, nwords, xpre, ring->size};
        if (plat_decide(fm, NULL) != 0) rc = OMEGA_GPU_PLAT_REFUSED_BAD_LENGTH;
        else {
            uint32_t *dst = ring->cpu;
            if (xpre && plat_publish(dst, prefix, sizeof prefix) != 0) rc = OMEGA_GPU_PLAT_REFUSED_DEVICE;
            else if (plat_publish(dst + xpre, words, (uint64_t)nwords * 4u) != 0) rc = OMEGA_GPU_PLAT_REFUSED_DEVICE;
            else if (plat_fence() != 0) rc = OMEGA_GPU_PLAT_REFUSED_DEVICE;
            if (rc == OMEGA_GPU_PLAT_OK && nvrm_chan_submit(&r->m16.rm, (uint32_t)slot + 1u, ring, 0, nwords + xpre) != 0) rc = OMEGA_GPU_PLAT_REFUSED_DEVICE;
            if (rc == OMEGA_GPU_PLAT_OK) { r->x_l2_pending[slot] = 0; r->xinflight[slot]++; }
        }
        if (rc != OMEGA_GPU_PLAT_OK) rc = refuse(r, rc, "submit_methods (extra channel)");
        ref_unlock(r);
        return rc;
    }
    /* omega #457: a host sync into a GPU-cached buffer is pending, so this submission starts with L2_INVAL_PREFIX.
     * No dirty line is dropped: every earlier submission ended with L2_FLUSH_DIRTY and the host waited for it.
     * DOCS SILENT on ordering between this MEM_OP and the launch that follows (no WFI is inserted); the chip gate
     * tools/run_gpu_l2_stale_chip.sh (20/20 FRESH) is the only proof. The prefix counts against the ring like any
     * word; callers budget it from g_caps.submit_prefix_words (omega_gpu_cmdlist.c). */
    const uint32_t pre = r->l2_inval_pending ? L2_INVAL_PREFIX_WORDS : 0u;
    uint64_t fm[16] = {9, nwords, pre, r->m16.pb_mem.size};
    if (rc == OMEGA_GPU_PLAT_OK && plat_decide(fm, NULL) != 0) rc = OMEGA_GPU_PLAT_REFUSED_BAD_LENGTH;
    uint32_t *buf = NULL;
    if (rc == OMEGA_GPU_PLAT_OK && pre) {
        buf = malloc(((size_t)nwords + pre) * 4u);
        if (!buf) { rc = OMEGA_GPU_PLAT_REFUSED_LIMIT; stage = "submit_methods (host copy for the #457 L2 invalidate prefix)"; }
        else {
            uint32_t prefix[5];
            plat_l2_words(prefix);
            if (plat_publish(buf, prefix, sizeof prefix) != 0 || plat_publish(buf + pre, words, (uint64_t)nwords * 4u) != 0)
                rc = OMEGA_GPU_PLAT_REFUSED_DEVICE;
        }
    }
    if (rc == OMEGA_GPU_PLAT_OK && m16_native_submit_methods(&r->m16, buf ? buf : words, nwords + pre) != 0) rc = OMEGA_GPU_PLAT_REFUSED_DEVICE;
    if (rc == OMEGA_GPU_PLAT_OK && pre) r->l2_inval_pending = 0;
    if (rc == OMEGA_GPU_PLAT_OK) r->inflight++;
    free(buf);
    if (rc != OMEGA_GPU_PLAT_OK) rc = refuse(r, rc, stage);
    ref_unlock(r);
    return rc;
}

static int ref_queue_retire(void *ctx, OmegaGpuQueueHandle q) {
    LinuxRef *r = ctx;
    ref_lock(r);
    int rc = queue_ok(r, q);
    const int slot = rc == OMEGA_GPU_PLAT_OK ? xq_slot(r, q) : -1;
    if (slot >= 0) {
        if (r->xinflight[slot]) { /* no-op when nothing is in flight on this extra channel */
            nvrm_chan_retire(&r->m16.rm, (uint32_t)slot + 1u, nvrm_chan_put(&r->m16.rm, (uint32_t)slot + 1u));
            r->xinflight[slot] = 0;
        }
    }
    else if (rc == OMEGA_GPU_PLAT_OK && r->inflight) {
        nvrm_retire(&r->m16.rm, r->m16.rm.put); /* both releases landed: the entry is consumed */
        r->inflight = 0;
    }
    else if (rc != OMEGA_GPU_PLAT_OK) rc = refuse(r, rc, "queue_retire");
    ref_unlock(r);
    return rc;
}

static void *ref_host_alloc(void *ctx, size_t bytes) { (void)ctx; return bytes ? malloc(bytes) : NULL; }
static void ref_host_free(void *ctx, void *p) { (void)ctx; free(p); }

static const OmegaGpuPlatform g_platform = {
    .ctx = &g_ref,
    .time_now_ms = ref_now_ms,
    .wait_ms = ref_wait_ms,
    .lock = ref_lock_id,
    .unlock = ref_unlock_id,
    .mmio_read32 = NULL,  /* not provided: the RM driver owns the registers */
    .mmio_write32 = NULL,
    .dma_alloc = ref_dma_alloc,
    .dma_free = ref_dma_free,
    .sync_cpu_to_gpu = ref_sync_to_gpu,
    .sync_gpu_to_cpu = ref_sync_to_cpu,
    .poll_word = ref_poll_word,
    .irq_wait = NULL,     /* polling is not interrupt support */
    .device_open = ref_device_open,
    .device_close = ref_device_close,
    .diag_text = ref_diag_text,
    .queue_open = ref_queue_open,
    .queue_close = ref_queue_close,
    .queue_submit = ref_queue_submit,
    .queue_retire = ref_queue_retire,
    .time_now_ns = ref_now_ns,
    .host_alloc = ref_host_alloc,
    .host_free = ref_host_free,
};

const OmegaGpuPlatform *omega_gpu_platform_linux(OmegaGpuPlatformCaps *caps) {
    if (caps) *caps = g_caps;
    return &g_platform;
}

uint32_t omega_gpu_platform_linux_target_sm(void) {
    ref_lock(&g_ref);
    uint64_t sm[16] = {14, g_ref.dev.generation, g_ref.m16.rm.target_sm};
    const uint32_t target = (uint32_t)plat_decide(sm, NULL);
    ref_unlock(&g_ref);
    return target;
}

int omega_gpu_platform_linux_vram_info(uint64_t *free_bytes, uint64_t *total_bytes) {
    ref_lock(&g_ref);
    int rc = g_ref.dev.generation ? nvrm_vram_info(&g_ref.m16.rm, free_bytes, total_bytes) : -1;
    ref_unlock(&g_ref);
    return rc;
}
