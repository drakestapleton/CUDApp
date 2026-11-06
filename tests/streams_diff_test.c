// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/* streams_diff_test.c -- CUDApp#115 host differential: built only with CUDAPP_MIGRATION_PENDING_STREAMS_DIFF (mk/om-streams.mk).
 * Runs omega_gpu_cmdlist_streams_diff_sweep (every chunk configuration through C and seeded Omega, abort on any
 * difference), every AIEN_OMEGA_GPU_CHANNELS value class, and every extra queue index (both diffed in their callers). */
#include "cudapp_native.h"
#include "omega_gpu_platform_linux.h"
#include "omega_gpu_session.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

uint64_t omega_gpu_cmdlist_streams_diff_sweep(void);
void cudapp_streams_diff_note(int same, const char *line);

/* Diff hook of src/omega_gpu_cmdlist.c / omega_gpu_session.c: SAME lines go to $CUDAPP_STREAMS_DIFF_LOG; a
 * difference is printed and aborts. */
void cudapp_streams_diff_note(int same, const char *line) {
    if (!same) { fprintf(stderr, "%s\n", line); abort(); }
    static FILE *f;
    const char *log = getenv("CUDAPP_STREAMS_DIFF_LOG");
    if (!f && log) { int fd = open(log, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0600); if (fd >= 0) { f = fdopen(fd, "a"); if (!f) close(fd); } }
    if (f) fprintf(f, "%s\n", line);
}

int main(void) {
    const uint64_t chunks = omega_gpu_cmdlist_streams_diff_sweep();
    static const char *const env[] = { "", "0", "1", "2", "3", "4", "5", "9", "22", "2x", "/", ":", "a" };
    unsigned nenv = 0;
    unsetenv("AIEN_OMEGA_GPU_CHANNELS"); (void)omega_gpu_platform_linux_channels_env(); nenv++;
    for (size_t i = 0; i < sizeof env / sizeof env[0]; i++) {
        setenv("AIEN_OMEGA_GPU_CHANNELS", env[i], 1);
        (void)omega_gpu_platform_linux_channels_env(); nenv++;
    }
    OmegaGpuQueueHandle q;
    for (uint32_t i = 0; i < 6u; i++) (void)omega_gpu_session_extra_queue(i, &q); /* no device: admission still diffed */
    /* The session holds no open queues here, so the extra-queue rule is also checked on every open set directly:
     * om/streams/xq.om against the rule of xq_admit_c (src/omega_gpu_session.c). */
    unsigned nxq = 0;
    for (uint64_t i = 0; i < 6u; i++) for (uint64_t open = 0; open < 16u; open++) {
        uint64_t want = 3, r = 99;
        if (i == 0 || i > 3) want = 0;
        else if ((open >> i) & 1u) want = 1;
        else for (uint64_t j = 1; j < i; j++) if (!((open >> j) & 1u)) { want = 2; break; }
        if (omega_streams_xq(&r, i, open) != 0 || r != want) {
            fprintf(stderr, "streams diff: extra queue i=%llu open=%llx Omega %llu C %llu\n", (unsigned long long)i,
                    (unsigned long long)open, (unsigned long long)r, (unsigned long long)want);
            abort();
        }
        nxq++;
    }
    printf("streams diff sweep: %llu chunks, %u channel values, 6 extra queue indexes, %u extra queue open sets: C and Omega identical\n",
           (unsigned long long)chunks, nenv, nxq);
    return chunks ? 0 : 1;
}
