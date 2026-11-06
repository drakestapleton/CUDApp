/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_mem_cached.h"
#include <string.h>
#include "omega_gpu_platform.h"
#include "omega_gpu_fmt.h"
#if defined(CUDAPP_MIGRATION_PENDING_C_MEMCACHE)
#define ENG_C_PART 1
#endif
#include "omega_eng_dec.h"

int omega_gpu_session_alloc_cached(size_t bytes, OmegaGpuMem *out) {
    size_t want;
    if (eng_on()) {
        uint64_t in[2], w[16];
        in[0] = (uint64_t)bytes;
        in[1] = out ? 1u : 0u;
        if (eng_ask(ENG_OP_CACHE, in, 2, w) != 0 || w[0] != 0) {
            omega_gpu_session_set_error("cached alloc: arguments");
            return -1;
        }
        want = (size_t)w[1];
    } else {
        if (!out || bytes == 0) { omega_gpu_session_set_error("cached alloc: arguments"); return -1; }
        want = (bytes + 0xfffu) & ~(size_t)0xfffu;
    }
    memset(out, 0, sizeof *out);
    OmegaGpuSession *s = omega_gpu_session_open();
    const OmegaGpuPlatform *p = omega_gpu_session_platform();
    if (!s || !p) return -1;
    OmegaGpuBufferDesc d = { .kind = OMEGA_GPU_HANDLE_BUF_DATA, .rights = OMEGA_GPU_RIGHT_CPU_WRITE | OMEGA_GPU_RIGHT_GPU_READ, /* no CPU_READ: the platform refuses a CPU-readable GPU-cached buffer */
                             .gpu_cache = OMEGA_GPU_CACHE_GPU_CACHED, .align = 0x1000u, .bytes = want };
    OmegaCpuView cpu; OmegaGpuVa va; OmegaGpuBusAddr bus;
    int prc = p->dma_alloc(p->ctx, s->dev, &d, &out->handle, &cpu, &va, &bus);
    if (prc != OMEGA_GPU_PLAT_OK || !cpu.ptr || cpu.len < want) {
        if (prc == OMEGA_GPU_PLAT_OK) (void)p->dma_free(p->ctx, out->handle);
        char e[96]; omega_gpu_fmt(e, sizeof e, "cached alloc of %zu bytes refused (%d)", want, prc);
        omega_gpu_session_set_error(e); memset(out, 0, sizeof *out); return -1;
    }
    out->kind = OMEGA_GPU_HANDLE_BUF_DATA; out->cpu = cpu.ptr; out->va = va.va; out->size = cpu.len;
    return 0;
}
