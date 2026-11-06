/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_mem_reserve.c -- developer bar D1: refuse an allocation that would leave less than the reserve free
 * (src/omega_gpu_mem_reserve.h). Hosted only: reads /proc/meminfo, or a regular file a test names by code with
 * omega_gpu_platform_linux_test_set_meminfo. */
#if defined(CUDAPP_MIGRATION_PENDING_C_MEMRESV)
#define ENG_C_PART 1
#endif
#include "omega_eng_dec.h"
#include "omega_gpu_mem_reserve.h"

#include "omega_gpu_platform.h"
#include "omega_gpu_platform_linux.h"

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* The meminfo file is chosen by code, never by the environment: the Linux platform is the default session platform
 * (omega_gpu_session.c install_default) of every hosted program linked with libomega_gpu.a, serving processes included,
 * so a stale variable left in an environment must not be able to switch the D1 guard off (CodeQL cpp/path-injection
 * #40 was the former CUDAPP_MEMINFO variable). Every open is O_NOFOLLOW|O_NONBLOCK and must reach a regular file, since
 * the check runs under the platform lock: open() on a FIFO blocks, and a device file may act on open. A test file that
 * cannot be used that way falls back to the real /proc/meminfo. */
static pthread_mutex_t g_meminfo_mu = PTHREAD_MUTEX_INITIALIZER;
static char g_meminfo_test_path[4096]; /* guarded by g_meminfo_mu; "" = /proc/meminfo */

int omega_gpu_platform_linux_test_set_meminfo(const char *path) {
    size_t n = path ? strlen(path) : 0;
    if (n >= sizeof g_meminfo_test_path) return -1;
    pthread_mutex_lock(&g_meminfo_mu);
    memcpy(g_meminfo_test_path, n ? path : "", n);
    g_meminfo_test_path[n] = '\0';
    pthread_mutex_unlock(&g_meminfo_mu);
    return 0;
}

static FILE *meminfo_open_regular(const char *path) {
    int fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) return NULL;
    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) { close(fd); return NULL; }
    FILE *f = fdopen(fd, "r");
    if (!f) close(fd);
    return f;
}

static FILE *meminfo_open(void) {
    pthread_mutex_lock(&g_meminfo_mu);
    FILE *f = g_meminfo_test_path[0] ? meminfo_open_regular(g_meminfo_test_path) : NULL;
    pthread_mutex_unlock(&g_meminfo_mu);
    return f ? f : meminfo_open_regular("/proc/meminfo");
}

static void human_bytes(char *out, size_t n, uint64_t b) {
    if (b < (1ull << 30)) snprintf(out, n, "%.1f MiB", (double)b / (double)(1ull << 20));
    else snprintf(out, n, "%.1f GiB", (double)b / (double)(1ull << 30));
}

static unsigned long long reserve_mib_env(void) {
    unsigned long long reserve_mib = CUDAPP_MEM_RESERVE_DEFAULT_MIB;
    const char *e = getenv("CUDAPP_MEM_RESERVE_MIB");
    if (e && *e) { char *end = NULL; unsigned long long v = strtoull(e, &end, 10); if (end && end != e && *end == 0) reserve_mib = v; }
    return reserve_mib;
}

/* Saturate rather than wrap: 2^44 MiB shifted by 20 is 2^64 (RtMem F9). */
static uint64_t mib_bytes(unsigned long long mib) { return mib > (UINT64_MAX >> 20) ? UINT64_MAX : (uint64_t)mib << 20; }

uint64_t omega_gpu_mem_reserve_bytes(void) { return mib_bytes(reserve_mib_env()); }

int omega_gpu_mem_reserve_check(uint64_t bytes, char *diag, size_t diag_len) {
    const unsigned long long reserve_mib = reserve_mib_env();
    if (eng_on()) {
        unsigned long long avail_kb = 0;
        int have = 0;
        uint64_t in[4], w[16];
        if (reserve_mib != 0) {
            FILE *f = meminfo_open();
            if (f) {
                char line[256];
                while (!have && fgets(line, sizeof line, f))
                    have = sscanf(line, "MemAvailable: %llu kB", &avail_kb) == 1;
                fclose(f);
            }
        }
        in[0] = bytes;
        in[1] = avail_kb;
        in[2] = reserve_mib;
        in[3] = (reserve_mib != 0 && have) ? 1u : 0u;
        if (eng_ask(ENG_OP_RESV, in, 4, w) != 0) return OMEGA_GPU_PLAT_REFUSED_LIMIT;
        if (w[0] == 0) return OMEGA_GPU_PLAT_OK;
        char want[32], free_[32], keep[32];
        human_bytes(want, sizeof want, bytes);
        human_bytes(free_, sizeof free_, w[1]);
        human_bytes(keep, sizeof keep, w[2]);
        snprintf(diag, diag_len,
                 "dma_alloc: refused %s: %s of memory is free and CUDA++ keeps %s free (CUDAPP_MEM_RESERVE_MIB=%llu), because the CPU and "
                 "GPU share it and running it out freezes the machine. Free buffers or use a smaller batch.",
                 want, free_, keep, reserve_mib);
        return (int)w[0];
    }
    if (reserve_mib == 0) return OMEGA_GPU_PLAT_OK;
    FILE *f = meminfo_open();
    if (!f) return OMEGA_GPU_PLAT_OK;
    unsigned long long avail_kb = 0; int have = 0; char line[256];
    while (!have && fgets(line, sizeof line, f)) have = sscanf(line, "MemAvailable: %llu kB", &avail_kb) == 1;
    fclose(f);
    if (!have) return OMEGA_GPU_PLAT_OK;
    /* Saturate rather than wrap: 2^54 kB times 1024 is 2^64 (RtMem F9). */
    const uint64_t avail = avail_kb > UINT64_MAX / 1024ull ? UINT64_MAX : (uint64_t)avail_kb * 1024ull;
    const uint64_t reserve = mib_bytes(reserve_mib);
    if (bytes <= avail && avail - bytes >= reserve) return OMEGA_GPU_PLAT_OK;
    char want[32], free_[32], keep[32];
    human_bytes(want, sizeof want, bytes); human_bytes(free_, sizeof free_, avail); human_bytes(keep, sizeof keep, reserve);
    snprintf(diag, diag_len,
             "dma_alloc: refused %s: %s of memory is free and CUDA++ keeps %s free (CUDAPP_MEM_RESERVE_MIB=%llu), because the CPU and "
             "GPU share it and running it out freezes the machine. Free buffers or use a smaller batch.",
             want, free_, keep, reserve_mib);
    return OMEGA_GPU_PLAT_REFUSED_LIMIT;
}
