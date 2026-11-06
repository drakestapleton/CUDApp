/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* codegen_gdn_diff_test.c: the seeded Omega GDN builders (om/codegen/cg_gdn*.om, replayed by omega_gpu_om_ir_replay)
 * against the retained C oracle (CUDAPP_MIGRATION_PENDING_C_CODEGEN_GDN) over the whole finite spec domain, refusals
 * included: IR instructions, register allocation and encoded bytes must be identical, and so must the shape policy and
 * the stats accessors. Ends with a corrupted-record negative control. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CUDAPP_MIGRATION_PENDING_C_CODEGEN_GDN 1
#define omega_gdn_shape oracle_shape
#define omega_blackwell_codegen_gdn_prog oracle_decode
#define omega_blackwell_codegen_gdn_multi_prog oracle_multi
#define omega_blackwell_codegen_gdn_copy_prog oracle_copy
#define omega_blackwell_codegen_gdn_prep_prog oracle_prep
#define omega_blackwell_codegen_gdn_scan_prog oracle_scan
#define omega_blackwell_codegen_gdn_post_prog oracle_post
#define omega_gdn_prog_stats oracle_stats
#define omega_gdn_pf_prog_stats oracle_pf_stats
#define omega_gdn_multi_prog_stats oracle_multi_stats
#include "../src/omega_blackwell_codegen_gdn.c"
#undef omega_gdn_shape
#undef omega_blackwell_codegen_gdn_prog
#undef omega_blackwell_codegen_gdn_multi_prog
#undef omega_blackwell_codegen_gdn_copy_prog
#undef omega_blackwell_codegen_gdn_prep_prog
#undef omega_blackwell_codegen_gdn_scan_prog
#undef omega_blackwell_codegen_gdn_post_prog
#undef omega_gdn_prog_stats
#undef omega_gdn_pf_prog_stats
#undef omega_gdn_multi_prog_stats
#include "omega_gpu_om_ir.h"
#include "cudapp_native.h"
extern int omega_gdn_shape(const OmegaGdnSpec *, OmegaGdnShape *);
extern int omega_blackwell_codegen_gdn_prog(const OmegaGdnSpec *, BlackwellIRProgram *);
extern int omega_blackwell_codegen_gdn_multi_prog(const OmegaGdnSpec *, BlackwellIRProgram *);
extern int omega_blackwell_codegen_gdn_copy_prog(const OmegaGdnSpec *, BlackwellIRProgram *);
extern int omega_blackwell_codegen_gdn_prep_prog(const OmegaGdnSpec *, BlackwellIRProgram *);
extern int omega_blackwell_codegen_gdn_scan_prog(const OmegaGdnSpec *, BlackwellIRProgram *);
extern int omega_blackwell_codegen_gdn_post_prog(const OmegaGdnSpec *, BlackwellIRProgram *);
extern int omega_gdn_prog_stats(const OmegaGdnSpec *, uint32_t *, uint32_t *);
extern int omega_gdn_pf_prog_stats(unsigned, const OmegaGdnSpec *, uint32_t *, uint32_t *);
extern int omega_gdn_multi_prog_stats(const OmegaGdnSpec *, uint32_t *, uint32_t *);

typedef int (*Build)(const OmegaGdnSpec *, BlackwellIRProgram *);
static const char *const names[6] = {"decode", "multi", "copy", "prep", "scan", "post"};
static const Build oracle[6] = {oracle_decode, oracle_multi, oracle_copy, oracle_prep, oracle_scan, oracle_post};
static const Build om[6] = {omega_blackwell_codegen_gdn_prog, omega_blackwell_codegen_gdn_multi_prog,
                            omega_blackwell_codegen_gdn_copy_prog, omega_blackwell_codegen_gdn_prep_prog,
                            omega_blackwell_codegen_gdn_scan_prog, omega_blackwell_codegen_gdn_post_prog};
static BlackwellIRProgram a, b;
static unsigned cases, refused, per[6];

static int equal(unsigned k, const OmegaGdnSpec *s) {
    int ra = oracle[k](s, &a), rb = om[k](s, &b);
    if ((ra == 0) != (rb == 0)) goto fail;
    /* A refused program has no specified IR output; compare the refusal, not stale scratch state. */
    if (ra) { refused++; return 1; }
    if (a.count != b.count || memcmp(a.insns, b.insns, a.count * sizeof a.insns[0]) ||
        memcmp(&a.regalloc, &b.regalloc, sizeof a.regalloc))
        goto fail;
    static uint8_t x[BW_MAX_IR_INSNS * 16], y[BW_MAX_IR_INSNS * 16];
    size_t nx = 0, ny = 0;
    if (omega_bw_encode_program(&a, x, sizeof x, &nx) || omega_bw_encode_program(&b, y, sizeof y, &ny) || nx != ny ||
        memcmp(x, y, nx))
        goto fail;
    cases++; per[k]++;
    return 1;
fail:
    fprintf(stderr, "GDN DIFF %s mutant=%d copy=%d ratio=%u bf16=%u C=%d Omega=%d count=%zu/%zu\n", names[k], s->mutant,
            s->copy, s->ratio, s->state_bf16, ra, rb, a.count, b.count);
    for (size_t i = 0; i < a.count && i < b.count; i++)
        if (memcmp(&a.insns[i], &b.insns[i], sizeof a.insns[i])) {
            fprintf(stderr, "first IR difference %zu op=%d/%d ctrl=%x/%x dst=%d/%d src=%d,%d,%d/%d,%d,%d imm=%x/%x u=%d/%d\n", i,
                    a.insns[i].op, b.insns[i].op, a.insns[i].control, b.insns[i].control, a.insns[i].dst_vreg,
                    b.insns[i].dst_vreg, a.insns[i].src1_vreg, a.insns[i].src2_vreg, a.insns[i].src3_vreg,
                    b.insns[i].src1_vreg, b.insns[i].src2_vreg, b.insns[i].src3_vreg, a.insns[i].imm, b.insns[i].imm,
                    a.insns[i].ureg, b.insns[i].ureg);
            break;
        }
    return 0;
}

static int same_shape(const OmegaGdnSpec *s) {
    OmegaGdnShape x, y;
    memset(&x, 0xa5, sizeof x); memset(&y, 0xa5, sizeof y);
    int rx = oracle_shape(s, &x), ry = omega_gdn_shape(s, &y);
    if (rx != ry || memcmp(&x, &y, sizeof x)) {
        fprintf(stderr, "GDN SHAPE DIFF ratio=%u bf16=%u C=%d Omega=%d\n", s->ratio, s->state_bf16, rx, ry);
        return 0;
    }
    return 1;
}

static int same_stats(const OmegaGdnSpec *s) {
    uint32_t i1 = 1, g1 = 2, i2 = 1, g2 = 2;
    int ok = oracle_stats(s, &i1, &g1) == omega_gdn_prog_stats(s, &i2, &g2) && i1 == i2 && g1 == g2;
    i1 = 1; g1 = 2; i2 = 1; g2 = 2;
    ok = ok && oracle_multi_stats(s, &i1, &g1) == omega_gdn_multi_prog_stats(s, &i2, &g2) && i1 == i2 && g1 == g2;
    for (unsigned kind = 0; ok && kind < 4; kind++) {
        i1 = 1; g1 = 2; i2 = 1; g2 = 2;
        ok = oracle_pf_stats(kind, s, &i1, &g1) == omega_gdn_pf_prog_stats(kind, s, &i2, &g2) && i1 == i2 && g1 == g2;
    }
    if (!ok)
        fprintf(stderr, "GDN DIFF stats mutant=%d copy=%d ratio=%u bf16=%u insns=%u/%u gprs=%u/%u\n", s->mutant, s->copy,
                s->ratio, s->state_bf16, i1, i2, g1, g2);
    return ok;
}

static uint32_t corrupt(uint64_t *r, uint64_t k, uint64_t x, uint64_t y, uint64_t z, void *b0, uint64_t nb, void *m,
                        uint64_t nm) {
    uint32_t rc = omega_cg_gdn(r, k, x, y, z, b0, nb, m, nm);
    if (!rc && !*r) ((uint64_t *)b0)[6] ^= 1;
    return rc;
}

int main(void) {
    /* The whole finite domain the builders branch on: every mutant selector and one past each end, both copy modes and
     * the values around them, every supported ratio plus 1, 4 and UINT32_MAX, and state_bf16 0, nonzero and UINT32_MAX. */
    const uint32_t ratios[] = {0, 1, 2, 3, 4, UINT32_MAX};
    const uint32_t bf16s[] = {0, 1, 2, UINT32_MAX};
    for (int mutant = -1; mutant <= 5; mutant++)
        for (int copy = -1; copy <= 3; copy++)
            for (unsigned r = 0; r < sizeof ratios / sizeof ratios[0]; r++)
                for (unsigned f = 0; f < sizeof bf16s / sizeof bf16s[0]; f++) {
                    OmegaGdnSpec s = {mutant, copy, ratios[r], bf16s[f]};
                    if (!same_shape(&s)) return 1;
                    for (unsigned k = 0; k < 6; k++)
                        if (!equal(k, &s)) return 1;
                    if (!same_stats(&s)) return 1;
                }
    OmegaGdnSpec s = {0, 0, 0, 0};
    for (unsigned k = 0; k < 6; k++)
        if (om[k](NULL, &b) != -1 || om[k](&s, NULL) != -1) { fprintf(stderr, "GDN NULL spec/prog accepted\n"); return 1; }
    if (omega_gdn_prog_stats(NULL, NULL, NULL) != -1 || omega_gdn_multi_prog_stats(NULL, NULL, NULL) != -1 ||
        omega_gdn_pf_prog_stats(0, NULL, NULL, NULL) != -1) {
        fprintf(stderr, "GDN NULL stats accepted\n");
        return 1;
    }
    /* Negative control: one flipped bit in the first emitted record must surface as an IR difference. */
    if (oracle_decode(&s, &a) || omega_gpu_om_ir_replay(corrupt, 0, 0, 0, 0, &b, NULL)) return 1;
    if (a.count == b.count && !memcmp(a.insns, b.insns, a.count * sizeof a.insns[0])) {
        fprintf(stderr, "record negative control escaped\n");
        return 1;
    }
    puts("CODEGEN GDN RECORD NEGATIVE CONTROL CAUGHT");
    printf("CODEGEN GDN DIFF PASS (%u programs: decode %u multi %u copy %u prep %u scan %u post %u; %u refusals)\n", cases,
           per[0], per[1], per[2], per[3], per[4], per[5], refused);
    return 0;
}
