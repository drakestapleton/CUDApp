/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host-only verification: none of these calls may open a GPU session. */
#include "omega_gpu_q38_graph.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    assert(omega_gpu_q38_exact_extra_bytes() == (UINT64_C(2)<<30));
    assert(omega_gpu_q38_graph_set_exact_head(NULL,NULL,NULL,10384) == OMEGA_GPU_DGRAPH_BAD_ARGS);
    assert(!omega_gpu_q38_graph_exact_head_active(NULL));
    assert(omega_gpu_q38_graph_set_exact_head_on(NULL,1) == OMEGA_GPU_DGRAPH_BAD_ARGS);
    assert(omega_gpu_q38_graph_set_sampling(NULL,1) == OMEGA_GPU_DGRAPH_BAD_ARGS);
    assert(omega_gpu_q38_graph_test_sampling_guard() == 0);
    puts("PASS: exact-head extra admission reserve and null-handle refusals");
    return 0;
}
