/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Test-only ABI glue. Language/checker/lowering remain in the pinned Omega
 * checkout. Encoding and cubin packaging link CUDApp's default Omega units. */
#include "omega_cubin.h"
#include "omega_osc_gpu.h"
#include <elf.h>
#include "bw_warp_sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned c1_images;
static int c1_guard_fixture(void) {
    static const struct { BlackwellIROpcode op; int d, a; unsigned inv; uint32_t w0, w1, w2; } fx[] = {
        {BW_IR_MOV_IMM, 2, -1, 0, 0x00020802, 5, 0xf00},
        {BW_IR_MOV_IMM, 2, -1, 1, 0x00028802, 5, 0xf00},
        {BW_IR_EXIT, -1, -1, 0, 0x0000094d, 0, 0x03800000},
        {BW_IR_EXIT, -1, -1, 1, 0x0000894d, 0, 0x03800000},
        /* WolframLanguageEvaluator:
         * IntegerString[BitOr[BitAnd[FromDigits[#,16],FromDigits["ffff0fff",16]],
         * BitShiftLeft[2,16]],16,8]& /@ {"7245","7305"} -> {"00020245","00020305"}. */
        {BW_IR_I2FP_U32, 2, 1, 0, 0x00020245, 1, 0x00201000},
        {BW_IR_F2I_U32, 2, 1, 0, 0x00020305, 1, 0x0020f000}
    };
    for (size_t i = 0; i < sizeof fx / sizeof fx[0]; ++i) {
        BlackwellIRInsn in = {0}; uint32_t w[4];
        in.op = fx[i].op; in.dst_vreg = fx[i].d; in.src1_vreg = fx[i].a;
        in.src2_vreg = in.src3_vreg = in.ureg = -1;
        in.imm = fx[i].op == BW_IR_MOV_IMM ? 5 : 0;
        in.predicate_p0 = true; in.predicate_not = fx[i].inv;
        if (omega_blackwell_encode_one(&in, w) || w[0] != fx[i].w0 || w[1] != fx[i].w1 || w[2] != fx[i].w2) return -1;
    }
    BlackwellIRInsn bad = {0}; uint32_t words[4];
    bad.op = (BlackwellIROpcode)(BW_IR_LAST + 1);
    if (!omega_blackwell_encode_one(&bad, words)) return -1;
    return 0;
}

/* Read the emitted ELF rather than searching for a byte substring: .text must
 * be a single exact payload. This tests the container path, not device loading. */
static int c1_text(const uint8_t *image, size_t n, const OscGpuKernel *k) {
    Elf64_Ehdr e; if (n < sizeof e) return -1; memcpy(&e, image, sizeof e);
    if (memcmp(e.e_ident, ELFMAG, SELFMAG) || e.e_machine != 190 || e.e_shentsize != sizeof(Elf64_Shdr) ||
        e.e_shoff > n || e.e_shnum > (n - e.e_shoff) / sizeof(Elf64_Shdr) || e.e_shstrndx >= e.e_shnum) return -1;
    Elf64_Shdr names; memcpy(&names, image + e.e_shoff + e.e_shstrndx * sizeof names, sizeof names);
    if (names.sh_offset > n || names.sh_size > n - names.sh_offset) return -1;
    unsigned found = 0;
    for (unsigned i = 0; i < e.e_shnum; ++i) {
        Elf64_Shdr s; memcpy(&s, image + e.e_shoff + i * sizeof s, sizeof s);
        if (s.sh_name >= names.sh_size) return -1;
        const char *name = (const char *)image + names.sh_offset + s.sh_name;
        if (!memchr(name, 0, names.sh_size - s.sh_name)) return -1;
        if (strncmp(name, ".text.", 6)) continue;
        if (s.sh_offset > n || s.sh_size > n - s.sh_offset || s.sh_size != k->code_size ||
            memcmp(image + s.sh_offset, k->code, k->code_size)) return -1;
        ++found;
    }
    return found == 1 ? 0 : -1;
}

static int c1_pack(OscGpuKernel *k) {
    int rc = 0; char err[256] = {0};
    OmegaCubinParam p[OMEGA_CUBIN_MAX_PARAMS]; size_t np = 0;
    for (unsigned i = 0; i < k->nparams; ++i) {
        const OscGpuParam *q = &k->params[i];
        int buffer = q->kind == OSC_GPU_P_VIEW || q->kind == OSC_GPU_P_EDIT;
        if (np + (buffer ? 2u : 1u) + 3u > OMEGA_CUBIN_MAX_PARAMS) { osc_gpu_free(k); return -1; }
        p[np++] = (OmegaCubinParam){OSC_GPU_ARG_BASE + q->off, buffer ? 8u : 4u, buffer};
        if (buffer) p[np++] = (OmegaCubinParam){OSC_GPU_ARG_BASE + q->off + 8u, 4, 0};
    }
    p[np++] = (OmegaCubinParam){OSC_GPU_ARG_BASE + k->status_off, 8, 1};
    p[np++] = (OmegaCubinParam){OSC_GPU_ARG_BASE + k->blocks_off, 4, 0};
    p[np++] = (OmegaCubinParam){OSC_GPU_ARG_BASE + k->batches_off, 4, 0};
    uint8_t *image = NULL; size_t bytes = 0;
    rc = omega_cubin_pack(k->name, k->code, k->code_size, p, np, OSC_GPU_ARG_BASE + k->arg_words * 4u,
                         omega_bw_gpr_declared(k->gpr_count), k->shared_bytes, &image, &bytes, err, sizeof err);
    if (!rc) rc = c1_text(image, bytes, k);
    if (!rc) {
        /* Negative control: changed text must not round-trip as the original. */
        k->code[0] ^= 1;
        int caught = c1_text(image, bytes, k) != 0;
        k->code[0] ^= 1;
        if (!caught) rc = -1;
    }
    omega_cubin_free(image);
    if (rc) { fprintf(stderr, "C1 cubin %s: %s\n", k->name, err); osc_gpu_free(k); return -1; }
    ++c1_images; return 0;
}
#define omega_blackwell_verify_codegen_fixtures_guard c1_guard_fixture
#define OSC_GPU_AFTER_COMPARE(k) do { if (c1_pack(k)) { CHECK(0, "C1 cubin packaging failed"); return -1; } } while (0)
#define main c1_sim_main
#include C1_TEST_SOURCE
#undef main
int main(int argc, char **argv) {
    int rc = c1_sim_main(argc, argv);
    if (!c1_images) rc = 1;
    printf("C1 SIM %s: cubin text roundtrips=%u; changed-text mutants caught\n", rc ? "FAIL" : "PASS", c1_images);
    return rc;
}
