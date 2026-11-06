/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Memory shell only. Shape, grouping, row ownership, tails and all arithmetic
 * decisions are seeded Omega in om/attn-any; generic replay is shared. */
#include "omega_gpu_attn_any_api.h"
#include "omega_gpu_om_ir.h"
#include "omega_gpu_session.h"
#include "cudapp_native.h"
#include <stdlib.h>
#include <string.h>

static int plan(const OmegaGpuAttnAnyShape *s, uint64_t head, uint64_t words[8], uint64_t p[16]) {
    uint32_t bits;
    uint64_t result = 0;
    memcpy(&bits, &s->scale, sizeof bits);
    words[0] = s->seq_q; words[1] = s->seq_kv; words[2] = s->head_dim;
    words[3] = s->q_heads; words[4] = s->kv_heads; words[5] = s->causal;
    words[6] = s->dtype; words[7] = bits;
    if (omega_aa_plan(&result, words, 8, head, p, 16) != 0) return OMEGA_GPU_ATTN_ANY_CODEGEN;
    return result ? OMEGA_GPU_ATTN_ANY_REFUSED : OMEGA_GPU_ATTN_ANY_OK;
}
int omega_gpu_attn_any_codegen(const OmegaGpuAttnAnyShape *s, BlackwellIRProgram *program, uint8_t *code, size_t capacity, OmegaBlackwellKernel *kernel) {
    uint64_t words[8], p[16];
    if (!s || !program || !code || !kernel) return OMEGA_GPU_ATTN_ANY_BAD_ARGS;
    memset(kernel, 0, sizeof *kernel);
    int rc = plan(s, 0, words, p);
    if (rc) return rc;
    rc = omega_gpu_om_ir_replay(omega_attn_any, 0, words[2], p[9], 0, program, NULL);
    if (rc || omega_blackwell_kernel_encode_into(program, code, capacity, kernel)) return OMEGA_GPU_ATTN_ANY_CODEGEN;
    return OMEGA_GPU_ATTN_ANY_OK;
}
static void putptr(uint32_t *args, uint64_t ptr) { args[0] = (uint32_t)ptr; args[1] = (uint32_t)(ptr >> 32); }
int omega_gpu_attn_any(const OmegaGpuAttnAnyShape *s, const void *q, const void *k, const void *v, float *out) {
    uint64_t words[8], p[16];
    if (!s || !q || !k || !v || !out) return OMEGA_GPU_ATTN_ANY_BAD_ARGS;
    int rc = plan(s, 0, words, p);
    if (rc) return rc; /* No device access for a refused shape. */
    BlackwellIRProgram *program = malloc(sizeof *program);
    uint8_t *code = malloc(65536);
    OmegaBlackwellKernel kernel = {0};
    if (!program || !code) { free(code); free(program); return OMEGA_GPU_ATTN_ANY_CODEGEN; }
    rc = omega_gpu_attn_any_codegen(s, program, code, 65536, &kernel);
    if (rc) { free(code); free(program); return rc; }
    OmegaGpuMem buffers[5] = {0};
    omega_gpu_session_lock();
    if (omega_gpu_session_open() == NULL || omega_gpu_session_alloc(p[0], &buffers[0]) != 0 ||
        omega_gpu_session_alloc(p[1], &buffers[1]) != 0 || omega_gpu_session_alloc(p[1], &buffers[2]) != 0 ||
        omega_gpu_session_alloc(p[2], &buffers[3]) != 0 || omega_gpu_session_alloc_code(kernel.code_size, &buffers[4]) != 0) {
        rc = OMEGA_GPU_ATTN_ANY_CHIP;
        goto cleanup;
    }
    memcpy(buffers[0].cpu, q, p[0]); memcpy(buffers[1].cpu, k, p[1]); memcpy(buffers[2].cpu, v, p[1]);
    memset(buffers[3].cpu, 0, p[2]); memcpy(buffers[4].cpu, code, kernel.code_size);
    for (unsigned i = 0; i < 5; i++) if (omega_gpu_session_sync_to_gpu(&buffers[i]) != 0) { rc = OMEGA_GPU_ATTN_ANY_CHIP; goto cleanup; }
    for (uint64_t head = 0; head < words[3]; head++) {
        rc = plan(s, head, words, p);
        if (rc) goto cleanup;
        uint32_t args[10];
        putptr(args, buffers[0].va + p[4]); putptr(args + 2, buffers[1].va + p[5]);
        putptr(args + 4, buffers[2].va + p[5]); putptr(args + 6, buffers[3].va + p[6]);
        args[8] = (uint32_t)words[1]; args[9] = (uint32_t)words[7];
        OmegaGpuLaunch launch = { .code_va = buffers[4].va, .gpr_count = omega_bw_gpr_declared(program->regalloc.peak_gpr_usage),
            .threads_x = (uint32_t)p[8], .threads_y = 1, .grid_x = (uint32_t)p[7], .grid_y = 1,
            .num_elements = p[7], .args = args, .n_args = 10, .timeout_ms = 30000 };
        if (omega_gpu_session_launch(&launch, NULL, NULL) != 0) { rc = OMEGA_GPU_ATTN_ANY_CHIP; goto cleanup; }
    }
    if (omega_gpu_session_sync_to_cpu(&buffers[3]) != 0) { rc = OMEGA_GPU_ATTN_ANY_CHIP; goto cleanup; }
    memcpy(out, buffers[3].cpu, p[2]);
cleanup:
    /* Session parks rather than releases buffers after uncertain completion. */
    for (unsigned i = 0; i < 5; i++) omega_gpu_session_free(&buffers[i]);
    omega_gpu_session_unlock();
    free(code); free(program);
    return rc;
}
