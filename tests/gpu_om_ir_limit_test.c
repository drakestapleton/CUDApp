/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_om_ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t records(uint64_t *result, uint64_t kind, uint64_t limit, uint64_t b, uint64_t c,
                        void *words, uint64_t nw, void *metadata, uint64_t nm) {
    uint64_t *w = words, *m = metadata;
    (void)b; (void)c;
    if (nw != OMEGA_OM_IR_WORDS || nm != OMEGA_OM_IR_META) return 1;
    *result = 0;
    m[6] = limit;
    if (!kind) {
        m[0] = 1;
        w[0] = BW_IR_EXIT;
        for (int i = 1; i <= 5; i++) w[i] = UINT64_MAX;
        return 0;
    }
    m[1] = BW_PHYS_GPR_MAX;
    m[0] = 2 * m[1];
    for (uint64_t v = 0; v < m[1]; v++) {
        m[64 + v] = 1;
        uint64_t *d = w + v * 8, *u = w + (m[1] + v) * 8;
        d[0] = BW_IR_MOV_IMM;
        d[1] = v;
        for (int i = 2; i <= 5; i++) d[i] = UINT64_MAX;
        d[6] = v;
        u[0] = BW_IR_MOV_RZ;
        for (int i = 1; i <= 5; i++) u[i] = UINT64_MAX;
        u[1] = v;
        u[2] = v;
    }
    return 0;
}

int main(void) {
    BlackwellIRProgram *p = malloc(sizeof(*p)), *old = malloc(sizeof(*old)), *zero = malloc(sizeof(*zero));
    if (!p || !old || !zero) return 2;
    const uint64_t bad[] = {1, BW_PHYS_GPR_MAX, BW_PHYS_GPR_HW_LIMIT + 1, UINT64_MAX,
                            (UINT64_C(1) << 32) | BW_PHYS_GPR_HW_LIMIT};
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
        memset(p, 0xa5, sizeof(*p));
        memcpy(old, p, sizeof(*p));
        if (omega_gpu_om_ir_replay(records, 0, bad[i], 0, 0, p, NULL) != OMEGA_OM_IR_REPLAY ||
            memcmp(p, old, sizeof(*p))) return 3;
    }
    if (omega_gpu_om_ir_replay(records, 0, 0, 0, 0, p, NULL) || p->regalloc.gpr_limit) return 4;
    memcpy(zero, p, sizeof(*p));
    if (omega_gpu_om_ir_replay(records, 0, 0, 0, 0, p, NULL) || memcmp(p, zero, sizeof(*p))) return 5;
    const uint64_t good[] = {BW_PHYS_GPR_MAX + 1, BW_PHYS_GPR_HW_LIMIT};
    for (size_t i = 0; i < sizeof(good) / sizeof(good[0]); i++)
        if (omega_gpu_om_ir_replay(records, 0, good[i], 0, 0, p, NULL) || p->regalloc.gpr_limit != good[i]) return 6;
    if (omega_gpu_om_ir_replay(records, 1, 0, 0, 0, p, NULL) != OMEGA_OM_IR_REGALLOC) return 7;
    if (omega_gpu_om_ir_replay(records, 1, BW_PHYS_GPR_HW_LIMIT, 0, 0, p, NULL) ||
        p->regalloc.gpr_limit != BW_PHYS_GPR_HW_LIMIT) return 8;
    free(p); free(old); free(zero);
    puts("PASS replay-limit zero/default, boundaries, pre-mutation refusals, pressure");
    return 0;
}
