/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen_pf.h"
#include <stdio.h>
#include <string.h>

static int same(const BlackwellIRProgram *a, const BlackwellIRProgram *b) {
    if (a->count != b->count) return 0;
    if (a->regalloc.num_vregs != b->regalloc.num_vregs) return 0;
    if (a->regalloc.num_uvregs != b->regalloc.num_uvregs) return 0;
    for (size_t i = 0; i < a->count; i++) {
        if (memcmp(&a->insns[i], &b->insns[i], sizeof a->insns[i]) != 0) return 0;
    }
    return 1;
}

int main(void) {
    BlackwellIRProgram c, o, m;
    if (omega_blackwell_codegen_pf_copy_prog_c(&c) != 0) { printf("FAIL c build\n"); return 1; }
    if (omega_cg_pf_copy_from_omega(&o, 0) != 0) { printf("FAIL omega build\n"); return 1; }
    if (!same(&c, &o)) { printf("FAIL bytes c=%zu o=%zu\n", c.count, o.count); return 1; }
    if (omega_cg_pf_copy_from_omega(&m, 1) != 0) { printf("FAIL mutant build\n"); return 1; }
    if (same(&c, &m)) { printf("FAIL mutant not caught\n"); return 1; }
    printf("PASS pf copy byte-identical insns=%zu mutant caught\n", c.count);
    return 0;
}
