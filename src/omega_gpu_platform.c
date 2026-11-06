/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_platform.c -- contract checks and the handle table of
 * src/omega_gpu_platform.h. Freestanding: no libc beyond memset.
 * The decisions (negotiate, service check, buffer check, handle open and check)
 * come from om/engine/eng.om ops NEGO, PCHK, BUFCK, HOPEN and HCHK; the C bodies
 * are the oracle, kept behind CUDAPP_MIGRATION_PENDING_C_PLATFORM and used by the
 * freestanding build. Table stores stay here. */
#include "omega_gpu_platform.h"

#include <string.h>
#if defined(CUDAPP_MIGRATION_PENDING_C_PLATFORM)
#define ENG_C_PART 1
#endif
#include "omega_eng_dec.h"

static const char *const REFUSAL_NAMES[OMEGA_GPU_PLAT_REFUSAL_COUNT] = {
    "OK",
    "REFUSED_ARGS",
    "REFUSED_ABI",
    "REFUSED_MISSING_SERVICE",
    "REFUSED_NULL_SERVICE",
    "REFUSED_BAD_HANDLE",
    "REFUSED_STALE_HANDLE",
    "REFUSED_BAD_LENGTH",
    "REFUSED_BAD_RIGHTS",
    "REFUSED_BAD_KIND",
    "REFUSED_CACHE_POLICY",
    "REFUSED_LIMIT",
    "REFUSED_BUSY",
    "REFUSED_DEVICE",
};

const char *omega_gpu_platform_refusal_name(int refusal)
{
    if (refusal < 0 || refusal >= (int)OMEGA_GPU_PLAT_REFUSAL_COUNT) return "UNKNOWN";
    return REFUSAL_NAMES[refusal];
}

int omega_gpu_platform_negotiate(const OmegaGpuPlatformCaps *provided, const OmegaGpuPlatformCaps *required)
{
    if (eng_on()) {
        uint64_t in[19], w[16];
        memset(in, 0, sizeof in);
        in[0] = (!provided || !required) ? 1u : 0u;
        if (provided) {
            in[1] = provided->abi_major;
            in[2] = provided->abi_minor;
            in[3] = provided->services;
            in[4] = provided->max_devices;
            in[5] = provided->max_buffers;
            in[6] = provided->max_queues;
            in[7] = provided->max_inflight;
            in[8] = provided->max_buffer_bytes;
            in[9] = provided->max_total_bytes;
        }
        if (required) {
            in[10] = required->abi_major;
            in[11] = required->abi_minor;
            in[12] = required->services;
            in[13] = required->max_devices;
            in[14] = required->max_buffers;
            in[15] = required->max_queues;
            in[16] = required->max_inflight;
            in[17] = required->max_buffer_bytes;
            in[18] = required->max_total_bytes;
        }
        if (eng_ask(ENG_OP_NEGO, in, 19, w) != 0) return OMEGA_GPU_PLAT_REFUSED_ARGS;
        return (int)w[0];
    }
    if (!provided || !required) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    if (provided->abi_major != required->abi_major) return OMEGA_GPU_PLAT_REFUSED_ABI;
    if (provided->abi_minor < required->abi_minor) return OMEGA_GPU_PLAT_REFUSED_ABI;
    if ((provided->services | required->services) & ~OMEGA_GPU_SVC_ALL) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    if (required->services & ~provided->services) return OMEGA_GPU_PLAT_REFUSED_MISSING_SERVICE;
    if (required->max_devices > provided->max_devices) return OMEGA_GPU_PLAT_REFUSED_LIMIT;
    if (required->max_buffers > provided->max_buffers) return OMEGA_GPU_PLAT_REFUSED_LIMIT;
    if (required->max_queues > provided->max_queues) return OMEGA_GPU_PLAT_REFUSED_LIMIT;
    if (required->max_inflight > provided->max_inflight) return OMEGA_GPU_PLAT_REFUSED_LIMIT;
    if (required->max_buffer_bytes > provided->max_buffer_bytes) return OMEGA_GPU_PLAT_REFUSED_LIMIT;
    if (required->max_total_bytes > provided->max_total_bytes) return OMEGA_GPU_PLAT_REFUSED_LIMIT;
    return OMEGA_GPU_PLAT_OK;
}

/* A claimed service must have its functions; an unclaimed one must have none
 * (a function the table never negotiated would be a silent fallback path). */
static int svc(uint32_t services, uint32_t bit, int present_all, int present_any)
{
    if (services & bit) return present_all ? OMEGA_GPU_PLAT_OK : OMEGA_GPU_PLAT_REFUSED_NULL_SERVICE;
    return present_any ? OMEGA_GPU_PLAT_REFUSED_ARGS : OMEGA_GPU_PLAT_OK;
}

int omega_gpu_platform_check(const OmegaGpuPlatform *p, const OmegaGpuPlatformCaps *caps)
{
    return omega_gpu_platform_check_for(p, caps, OMEGA_GPU_SVC_REQUIRED_V1);
}

int omega_gpu_platform_check_for(const OmegaGpuPlatform *p, const OmegaGpuPlatformCaps *caps, uint32_t required)
{
    int rc;
    uint32_t s;
    if (eng_on()) {
        uint64_t in[4], w[16];
        uint64_t pr = 0;
        in[0] = (!p || !caps) ? 1u : 0u;
        in[1] = caps ? caps->services : 0u;
        in[2] = required;
        if (p) {
            /* Presence bits in service order; see pchk/svgm in om/engine/eng.om. */
            pr |= (uint64_t)(p->time_now_ms != 0) << 0;
            pr |= (uint64_t)(p->wait_ms != 0) << 1;
            pr |= (uint64_t)(p->lock != 0) << 2;
            pr |= (uint64_t)(p->unlock != 0) << 3;
            pr |= (uint64_t)(p->mmio_read32 != 0) << 4;
            pr |= (uint64_t)(p->mmio_write32 != 0) << 5;
            pr |= (uint64_t)(p->dma_alloc != 0) << 6;
            pr |= (uint64_t)(p->dma_free != 0) << 7;
            pr |= (uint64_t)(p->sync_cpu_to_gpu != 0) << 8;
            pr |= (uint64_t)(p->sync_gpu_to_cpu != 0) << 9;
            pr |= (uint64_t)(p->poll_word != 0) << 10;
            pr |= (uint64_t)(p->irq_wait != 0) << 11;
            pr |= (uint64_t)(p->device_open != 0) << 12;
            pr |= (uint64_t)(p->device_close != 0) << 13;
            pr |= (uint64_t)(p->diag_text != 0) << 14;
            pr |= (uint64_t)(p->queue_open != 0) << 15;
            pr |= (uint64_t)(p->queue_close != 0) << 16;
            pr |= (uint64_t)(p->queue_submit != 0) << 17;
            pr |= (uint64_t)(p->queue_retire != 0) << 18;
            pr |= (uint64_t)(p->time_now_ns != 0) << 19;
            pr |= (uint64_t)(p->host_alloc != 0) << 20;
            pr |= (uint64_t)(p->host_free != 0) << 21;
        }
        in[3] = pr;
        if (eng_ask(ENG_OP_PCHK, in, 4, w) != 0) return OMEGA_GPU_PLAT_REFUSED_ARGS;
        return (int)w[0];
    }
    if (!p || !caps) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    s = caps->services;
    if (s & ~OMEGA_GPU_SVC_ALL) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    if (required & ~OMEGA_GPU_SVC_ALL) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    if ((s & required) != required) return OMEGA_GPU_PLAT_REFUSED_MISSING_SERVICE;
    if ((rc = svc(s, OMEGA_GPU_SVC_TIME, p->time_now_ms != 0, p->time_now_ms != 0)) != 0) return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_WAIT, p->wait_ms != 0, p->wait_ms != 0)) != 0) return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_LOCK, p->lock != 0 && p->unlock != 0, p->lock != 0 || p->unlock != 0)) != 0)
        return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_MMIO, p->mmio_read32 != 0 && p->mmio_write32 != 0,
                  p->mmio_read32 != 0 || p->mmio_write32 != 0)) != 0)
        return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_DMA, p->dma_alloc != 0 && p->dma_free != 0,
                  p->dma_alloc != 0 || p->dma_free != 0)) != 0)
        return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_SYNC, p->sync_cpu_to_gpu != 0 && p->sync_gpu_to_cpu != 0,
                  p->sync_cpu_to_gpu != 0 || p->sync_gpu_to_cpu != 0)) != 0)
        return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_COMPLETION, p->poll_word != 0, p->poll_word != 0)) != 0) return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_IRQ, p->irq_wait != 0, p->irq_wait != 0)) != 0) return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_DEVICE, p->device_open != 0 && p->device_close != 0 && p->diag_text != 0,
                  p->device_open != 0 || p->device_close != 0 || p->diag_text != 0)) != 0)
        return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_QUEUE,
                  p->queue_open != 0 && p->queue_close != 0 && p->queue_submit != 0 && p->queue_retire != 0,
                  p->queue_open != 0 || p->queue_close != 0 || p->queue_submit != 0 || p->queue_retire != 0)) != 0)
        return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_CLOCK_NS, p->time_now_ns != 0, p->time_now_ns != 0)) != 0) return rc;
    if ((rc = svc(s, OMEGA_GPU_SVC_HOST_MEM, p->host_alloc != 0 && p->host_free != 0,
                  p->host_alloc != 0 || p->host_free != 0)) != 0)
        return rc;
    return OMEGA_GPU_PLAT_OK;
}

static int is_pow2(uint32_t v)
{
    return v != 0u && (v & (v - 1u)) == 0u;
}

int omega_gpu_platform_check_buffer(const OmegaGpuBufferDesc *d, const OmegaGpuPlatformCaps *caps)
{
    uint32_t r;
    if (eng_on()) {
        uint64_t in[7], w[16];
        memset(in, 0, sizeof in);
        in[0] = (!d || !caps) ? 1u : 0u;
        if (d && caps) {
            in[1] = (uint64_t)d->kind;
            in[2] = d->bytes;
            in[3] = caps->max_buffer_bytes;
            in[4] = d->align;
            in[5] = (uint64_t)d->gpu_cache;
            in[6] = d->rights;
        }
        if (eng_ask(ENG_OP_BUFCK, in, 7, w) != 0) return OMEGA_GPU_PLAT_REFUSED_ARGS;
        return (int)w[0];
    }
    if (!d || !caps) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    if (d->kind != OMEGA_GPU_HANDLE_BUF_DATA && d->kind != OMEGA_GPU_HANDLE_BUF_CODE &&
        d->kind != OMEGA_GPU_HANDLE_BUF_QUEUE)
        return OMEGA_GPU_PLAT_REFUSED_BAD_KIND;
    if (d->bytes == 0u) return OMEGA_GPU_PLAT_REFUSED_BAD_LENGTH;
    if (d->bytes > caps->max_buffer_bytes) return OMEGA_GPU_PLAT_REFUSED_LIMIT;
    if (d->align != 0u && !is_pow2(d->align)) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    if (d->gpu_cache != OMEGA_GPU_CACHE_GPU_UNCACHED && d->gpu_cache != OMEGA_GPU_CACHE_GPU_CACHED)
        return OMEGA_GPU_PLAT_REFUSED_ARGS;
    r = d->rights;
    if (r == 0u || (r & ~OMEGA_GPU_RIGHT_ALL)) return OMEGA_GPU_PLAT_REFUSED_BAD_RIGHTS;
    if ((r & OMEGA_GPU_RIGHT_GPU_EXEC) && d->kind != OMEGA_GPU_HANDLE_BUF_CODE) return OMEGA_GPU_PLAT_REFUSED_BAD_RIGHTS;
    if (d->kind == OMEGA_GPU_HANDLE_BUF_CODE) {
        /* code: CPU writes it, GPU executes it, GPU never writes it, nobody polls it */
        if (r & (OMEGA_GPU_RIGHT_GPU_WRITE | OMEGA_GPU_RIGHT_HOST_POLLED)) return OMEGA_GPU_PLAT_REFUSED_BAD_RIGHTS;
        if (!(r & OMEGA_GPU_RIGHT_GPU_EXEC) || !(r & OMEGA_GPU_RIGHT_CPU_WRITE)) return OMEGA_GPU_PLAT_REFUSED_BAD_RIGHTS;
    }
    if (d->kind == OMEGA_GPU_HANDLE_BUF_QUEUE) {
        /* a submission ring: the CPU writes methods, the device reads them */
        if (!(r & OMEGA_GPU_RIGHT_CPU_WRITE) || !(r & OMEGA_GPU_RIGHT_GPU_READ)) return OMEGA_GPU_PLAT_REFUSED_BAD_RIGHTS;
    }
    if (r & OMEGA_GPU_RIGHT_HOST_POLLED) {
        if (!(r & OMEGA_GPU_RIGHT_CPU_READ) || !(r & OMEGA_GPU_RIGHT_GPU_WRITE)) return OMEGA_GPU_PLAT_REFUSED_BAD_RIGHTS;
        if (d->gpu_cache != OMEGA_GPU_CACHE_GPU_UNCACHED) return OMEGA_GPU_PLAT_REFUSED_CACHE_POLICY;
    }
    return OMEGA_GPU_PLAT_OK;
}

void omega_gpu_handles_init(OmegaGpuHandleTable *t)
{
    uint32_t i;
    if (!t) return;
    memset(t, 0, sizeof *t);
    for (i = 0; i < OMEGA_GPU_HANDLE_SLOTS; i++) t->generation[i] = 1u;
}

static int kind_known(uint32_t kind)
{
    return kind >= OMEGA_GPU_HANDLE_DEVICE && kind < OMEGA_GPU_HANDLE_KIND_COUNT;
}

int omega_gpu_handles_open(OmegaGpuHandleTable *t, uint32_t kind, uint64_t bytes, uint32_t limit_slots,
                           uint64_t limit_bytes, uint32_t *index, uint32_t *generation)
{
    uint32_t i;
    uint32_t same_kind = 0;
    if (eng_on()) {
        uint64_t in[7], w[16];
        in[0] = (!t || !index || !generation) ? 1u : 0u;
        in[1] = kind;
        if (t)
            for (i = 0; i < OMEGA_GPU_HANDLE_SLOTS; i++)
                if (t->live[i] && t->kind[i] == kind) same_kind++;
        in[2] = same_kind;
        in[3] = limit_slots;
        in[4] = bytes;
        in[5] = limit_bytes;
        in[6] = t ? t->total_bytes : 0u;
        if (eng_ask(ENG_OP_HOPEN, in, 7, w) != 0) return OMEGA_GPU_PLAT_REFUSED_ARGS;
        if (w[0] != OMEGA_GPU_PLAT_OK) return (int)w[0];
    } else {
        if (!t || !index || !generation) return OMEGA_GPU_PLAT_REFUSED_ARGS;
        if (!kind_known(kind)) return OMEGA_GPU_PLAT_REFUSED_BAD_KIND;
        for (i = 0; i < OMEGA_GPU_HANDLE_SLOTS; i++)
            if (t->live[i] && t->kind[i] == kind) same_kind++;
        if (same_kind >= limit_slots) return OMEGA_GPU_PLAT_REFUSED_LIMIT;
        if (bytes != 0u && (bytes > limit_bytes || t->total_bytes > limit_bytes - bytes)) /* byte accounting: buffers only */
            return OMEGA_GPU_PLAT_REFUSED_LIMIT;
    }
    for (i = 0; i < OMEGA_GPU_HANDLE_SLOTS; i++) {
        if (t->live[i]) continue;
        if (t->generation[i] == 0u) continue; /* generation wrapped: the slot is retired, never reissued */
        t->live[i] = 1;
        t->kind[i] = kind;
        t->bytes[i] = bytes;
        t->nlive++;
        t->total_bytes += bytes;
        *index = i;
        *generation = t->generation[i];
        return OMEGA_GPU_PLAT_OK;
    }
    return OMEGA_GPU_PLAT_REFUSED_LIMIT;
}

int omega_gpu_handles_check(const OmegaGpuHandleTable *t, uint32_t index, uint32_t generation, uint32_t kind)
{
    if (eng_on()) {
        uint64_t in[7], w[16];
        int in_range = t && index < OMEGA_GPU_HANDLE_SLOTS;
        in[0] = t ? 0u : 1u;
        in[1] = index;
        in[2] = generation;
        in[3] = in_range ? t->live[index] : 0u;
        in[4] = in_range ? t->generation[index] : 0u;
        in[5] = in_range ? t->kind[index] : 0u;
        in[6] = kind;
        if (eng_ask(ENG_OP_HCHK, in, 7, w) != 0) return OMEGA_GPU_PLAT_REFUSED_ARGS;
        return (int)w[0];
    }
    if (!t) return OMEGA_GPU_PLAT_REFUSED_ARGS;
    if (index >= OMEGA_GPU_HANDLE_SLOTS || generation == 0u) return OMEGA_GPU_PLAT_REFUSED_BAD_HANDLE;
    if (!t->live[index] || t->generation[index] != generation) return OMEGA_GPU_PLAT_REFUSED_STALE_HANDLE;
    if (t->kind[index] != kind) return OMEGA_GPU_PLAT_REFUSED_BAD_HANDLE;
    return OMEGA_GPU_PLAT_OK;
}

int omega_gpu_handles_close(OmegaGpuHandleTable *t, uint32_t index, uint32_t generation, uint32_t kind)
{
    int rc = omega_gpu_handles_check(t, index, generation, kind);
    if (rc != OMEGA_GPU_PLAT_OK) return rc;
    t->live[index] = 0;
    t->total_bytes -= t->bytes[index];
    t->bytes[index] = 0u;
    t->nlive--;
    t->generation[index]++; /* the closed handle is now stale; 0 after wrap retires the slot */
    return OMEGA_GPU_PLAT_OK;
}
