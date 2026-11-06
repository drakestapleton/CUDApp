/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* The Linux reference's marker poll through omega_gpu_live_marker (src/omega_gpu_platform_linux.c ref_poll_word),
 * on the simulated driver (tests/fake_m16_native.c). Host program, opens no device.
 *
 * The fake normally hands out memfd pages, which the platform binds itself (plat_bind_fd), so the lookup is never
 * reached. Here memfd_create is replaced by an O_TMPFILE file: the platform's mmap capture skips it, the buffer has
 * no platform token, and the poll must take the lookup. This file's omega_gpu_live_marker stands in for nvrm.c's: it
 * answers only for the Nvrm the platform opened, binds the page in its own context (not the platform's), and returns
 * that context. The poll must then read the word through the returned context at the returned offset, with
 * the platform's Nvrm. Mutants in mk/platform-linux.mk: polling plat_wait_ctx instead, passing NULL for rm and
 * dropping the offset are each caught. */
#include "fake_m16_native.h"
#include "nvrm.h"
#include "omega_gpu_platform_linux.h"
#include "omega_gpu_session.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

uint32_t plat_wt_context_create(uint64_t capacity, struct omega_native_context **out);
uint32_t omega2_ctx_plat_wait_hold(struct omega_native_context *context, uint64_t *result, uint64_t o_addr, uint64_t o_n);
uint32_t omega2_ctx_plat_wait_bind(struct omega_native_context *context, uint64_t *result, uint64_t o_handle, uint64_t o_fd);

static const Nvrm *g_open_rm;
int nvrm_open(Nvrm *rm) { rm->target_sm = 121; fk.device_open = 1; g_open_rm = rm; return 0; }
int nvrm_close(Nvrm *rm) { (void)rm; return 0; }
void nvrm_retire(Nvrm *rm, uint32_t upto) { (void)rm; (void)upto; }

/* Not a memfd: readlink gives "/tmp/#N (deleted)", so the platform's mmap hook does not bind it. */
static int g_last_fd = -1;
int memfd_create(const char *name, unsigned int flags) {
    (void)name;
    int fd = open("/tmp", O_TMPFILE | O_RDWR | ((flags & MFD_CLOEXEC) ? O_CLOEXEC : 0), 0600);
    if (fd >= 0) g_last_fd = fd;
    return fd;
}

static const uint8_t *g_base;
static struct omega_native_context *g_live_ctx;
static uint64_t g_live_token;
static int g_calls, g_bad_rm;
int omega_gpu_live_marker(const Nvrm *rm, const void *cpu, uint64_t *mapping, uint64_t *offset,
                          struct omega_native_context **ctx) {
    g_calls++;
    if (!ctx || !mapping || !offset) { errno = EINVAL; return -1; }
    *ctx = NULL;
    if (!rm || rm != g_open_rm) { g_bad_rm++; errno = EINVAL; return -1; }
    const uint8_t *p = cpu;
    if (!g_base || p < g_base || p + 4 > g_base + 4096) { errno = EFAULT; return -1; }
    if (!g_live_ctx) {
        uint64_t held = 0, r = 0;
        if (plat_wt_context_create(8, &g_live_ctx) != 0 || !g_live_ctx) return -1;
        if (omega2_ctx_plat_wait_hold(g_live_ctx, &held, 536870912ull, 4096) != 0 || (int64_t)held <= 0) return -1;
        if (omega2_ctx_plat_wait_bind(g_live_ctx, &r, held, (uint64_t)dup(g_last_fd)) != 0 || (int64_t)r != 0) return -1;
        g_live_token = held;
    }
    *mapping = g_live_token;
    *offset = (uint64_t)(p - g_base);
    *ctx = g_live_ctx;
    return 0;
}

#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

int main(void) {
    int fails = 0;
    fake_reset();
    OmegaGpuPlatformCaps caps;
    const OmegaGpuPlatform *p = omega_gpu_platform_linux(&caps);
    OmegaGpuDeviceHandle d;
    CHECK(p->device_open(p->ctx, 0, &d) == OMEGA_GPU_PLAT_OK);
    OmegaGpuBufferDesc polled = {.kind = OMEGA_GPU_HANDLE_BUF_DATA,
                                 .rights = OMEGA_GPU_RIGHT_HOST_POLLED | OMEGA_GPU_RIGHT_CPU_READ | OMEGA_GPU_RIGHT_GPU_WRITE,
                                 .gpu_cache = OMEGA_GPU_CACHE_GPU_UNCACHED, .bytes = 4096};
    OmegaGpuBufferHandle h; OmegaCpuView cpu; OmegaGpuVa va; OmegaGpuBusAddr bus;
    CHECK(p->dma_alloc(p->ctx, d, &polled, &h, &cpu, &va, &bus) == OMEGA_GPU_PLAT_OK);
    if (fails) return 1;
    g_base = cpu.ptr;
    memset(cpu.ptr, 0, 4096);
    /* The poll is wrap-aware >= (plat_wait.om: ready when (got - want) mod 2^32 < 2^31). WolframLanguageEvaluator:
     * {16^^5a5a5a5a + 16^^10000000, Mod[0 - 16^^6a5a5a5a, 2^32] >= 2^31, Mod[16^^6a5a5a5a - 16^^6a5a5a5b, 2^32] >= 2^31,
     * 512*1024*1024} -> {1784306266 (0x6a5a5a5a), True, True, 536870912}. Word 0 stays 0, which is behind the marker,
     * so a poll that drops the offset never sees it; one past the marker is not ready. */
    ((volatile uint32_t *)cpu.ptr)[2] = 0x6a5a5a5au;
    uint32_t seen = 0;
    CHECK(p->poll_word(p->ctx, h, 8, 0x6a5a5a5au, 5, &seen) == OMEGA_GPU_PLAT_OK && seen == 0x6a5a5a5au);
    CHECK(g_calls >= 1 && g_bad_rm == 0 && g_live_ctx != NULL);
    /* One past the marker is not ready: the poll times out rather than reporting success. */
    CHECK(p->poll_word(p->ctx, h, 8, 0x6a5a5a5bu, 2, &seen) == OMEGA_GPU_POLL_TIMED_OUT);
    CHECK(g_bad_rm == 0);
    if (fails) return 1;
    printf("PLATFORM LINUX LIVE MARKER PASS (poll through the returned context at the returned offset with the platform's Nvrm; one-past timed out; HOST_ONLY)\n");
    return 0;
}
