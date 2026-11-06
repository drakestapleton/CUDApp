/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* pre-fix reproduction: a plan_walk callback that calls omega_gpu_fft_twiddles (same as host_reentry which 0) */
#include "omega_gpu_mathlibs_api.h"
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
static int cb(void *ctx, const OmegaGpuFftLaunch *l) {
    (void)l;
    float tw[16];
    int *calls = ctx;
    if (omega_gpu_fft_twiddles(8, 0, tw) != OMEGA_GPU_ML_OK) return 99;
    (*calls)++;
    return 0;
}
int main(void) {
    pid_t pid = fork();
    if (pid == 0) {
        alarm(20);
        int calls = 0;
        size_t bytes[3];
        uint32_t n = 8;
        int rc = omega_gpu_fft_plan_walk(OMEGA_GPU_FFT_R2C, 1, &n, 1, 0, bytes, cb, &calls);
        _exit(rc == 0 && calls > 0 ? 0 : 3);
    }
    int st = 0;
    waitpid(pid, &st, 0);
    printf("reentry: exited=%d status=%d signaled=%d sig=%d (SIGALRM=%d)\n", WIFEXITED(st), WIFEXITED(st) ? WEXITSTATUS(st) : -1,
           WIFSIGNALED(st), WIFSIGNALED(st) ? WTERMSIG(st) : 0, SIGALRM);
    return WIFEXITED(st) && WEXITSTATUS(st) == 0 ? 0 : 1;
}
