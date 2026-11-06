/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host-only verification: none of these calls may open a GPU session. */
#include "omega_gpu_q38_graph.h"
#include <assert.h>
#include <stdio.h>
#include "../standalone-oracles/oracle.h"
int main(void) {
    assert(so_eq(omega_gpu_q38_exact_extra_bytes(),so_call(17,0,0,0,0)));
    assert(so_eq((uint64_t)(int64_t)omega_gpu_q38_graph_set_exact_head(NULL,NULL,NULL,10384),(uint64_t)(int64_t)OMEGA_GPU_DGRAPH_BAD_ARGS));
    assert(so_eq(omega_gpu_q38_graph_exact_head_active(NULL),0));
    omega_gpu_q38_graph_set_exact_head_on(NULL,1);
    puts("PASS: exact-head extra admission reserve and null-handle refusals");
    return 0;
}
