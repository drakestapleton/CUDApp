/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_serving.c: see omega_gpu_serving.h. Thin wrapper over the per-API reserve calls. */
#include "omega_gpu_serving.h"
#include "omega_gpu_attention_api.h"
#include "omega_gpu_matmul_api.h"
#if defined(CUDAPP_MIGRATION_PENDING_C_SERVING)
#define ENG_C_PART 1
#endif
#include "omega_eng_dec.h"

int omega_gpu_reserve_serving(const OmegaGpuServingBounds *b) {
    uint32_t seqs, one_row;
    int rows;
    if (eng_on()) {
        uint64_t in[5], w[16];
        in[0] = b ? 1u : 0u;
        in[1] = b ? b->max_seqs : 0u;
        in[2] = b ? b->max_rows : 0u;
        in[3] = b ? b->max_n : 0u;
        in[4] = b ? b->max_n_one_row : 0u;
        if (eng_ask(ENG_OP_SERVE, in, 5, w) != 0 || w[0] != 0) return OMEGA_GPU_MATMUL_BAD_ARGS;
        seqs = (uint32_t)w[1];
        rows = (int)w[2];
        one_row = (uint32_t)w[3];
    } else {
        if (!b) return OMEGA_GPU_MATMUL_BAD_ARGS;
        seqs = b->max_seqs ? b->max_seqs : 1;
        rows = b->max_rows > 1;
        one_row = b->max_n_one_row ? b->max_n_one_row : b->max_n;
    }
    int rc = omega_gpu_attention_reserve(b->max_context, seqs, b->num_q_heads, b->num_kv_heads, b->head_dim, b->kv_block_size);
    if (rc != 0) return rc;
    if (rows) {
        rc = omega_gpu_attention_reserve_rows(b->max_rows, b->num_q_heads, b->head_dim);
        if (rc != 0) { omega_gpu_attention_unreserve(); return rc; }
    }
    rc = omega_gpu_matmul_reserve(b->max_rows, b->max_k, b->max_n, one_row, b->kernel_slots);
    if (rc != 0) omega_gpu_attention_unreserve();
    return rc;
}

void omega_gpu_serving_seal(void) { omega_gpu_matmul_seal(1); }

void omega_gpu_serving_release(void) {
    omega_gpu_matmul_seal(0);
    omega_gpu_attention_unreserve();
    omega_gpu_matmul_unreserve();
}
