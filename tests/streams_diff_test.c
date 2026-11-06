// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
/* streams_diff_test.c -- CUDApp#115 host differential: built only with CUDAPP_MIGRATION_PENDING_STREAMS_DIFF (mk/om-streams.mk).
 * Runs omega_gpu_cmdlist_streams_diff_sweep (every chunk configuration through C and seeded Omega, abort on any
 * difference), every AIEN_OMEGA_GPU_CHANNELS value class, and every extra queue index (both diffed in their callers). */
#include "omega_gpu_platform_linux.h"
#include "omega_gpu_session.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint64_t omega_gpu_cmdlist_streams_diff_sweep(void);
void cudapp_streams_diff_note(int same, const char *line);

/* Diff hook of src/omega_gpu_cmdlist.c / omega_gpu_session.c: SAME lines go to $CUDAPP_STREAMS_DIFF_LOG; a
 * difference is printed and aborts. */
void cudapp_streams_diff_note(int same, const char *line) {
    if (!same) { fprintf(stderr, "%s\n", line); abort(); }
    static FILE *f;
    const char *log = getenv("CUDAPP_STREAMS_DIFF_LOG");
    if (!f && log) f = fopen(log, "a");
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
    printf("streams diff sweep: %llu chunks, %u channel values, 6 extra queue indexes: C and Omega identical\n",
           (unsigned long long)chunks, nenv);
    return chunks ? 0 : 1;
}
