/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* om/codegen/cg_bind.om against the CUDAPP_MIGRATION_PENDING_C_BIND oracle: spec bytes, realization binding,
 * machine descriptor, matmul dispatch, vector geometry, program padding/fit/insns and the digest admission, over
 * every partition boundary (0, 1, the limits, limit+1, uint32 and size_t wrap). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "omega_blackwell_codegen.h"
#include "omega_blackwell_encoder.h"
#include "omega_blackwell_matmul.h"
#include "omega_blackwell_realize.h"
#include "sha256.h"

int legacy_omega_matmul_spec_init(OmegaMatMulSpec *, uint32_t, uint32_t, uint32_t, OmegaMatMulPrecision);
int legacy_omega_blackwell_bind_matmul_realization(const OmegaMatMulSpec *, const OmegaBlackwellKernel *,
                                                   OmegaBlackwellRealizationIdentity *);
int legacy_omega_blackwell_codegen_matmul(const OmegaMatMulSpec *, OmegaBlackwellKernel *);
int legacy_omega_blackwell_get_machine_id(SemanticId *);
int legacy_omega_blackwell_realization_compute_id(OmegaBlackwellRealization *);
int legacy_omega_blackwell_realize_vector(const OmegaVectorSpec *, OmegaBlackwellRealization *);
size_t legacy_omega_bw_program_code_bytes(const BlackwellIRProgram *);
int legacy_omega_blackwell_kernel_encode_into(const BlackwellIRProgram *, uint8_t *, size_t, OmegaBlackwellKernel *);

static unsigned long cases, refused;
static uint64_t rng = 0x9e3779b97f4a7c15ull;
static uint64_t next(void) {
    rng ^= rng << 13;
    rng ^= rng >> 7;
    rng ^= rng << 17;
    return rng;
}
static void fail(const char *what, unsigned long a, unsigned long b) {
    printf("FAIL bind %s %lu %lu\n", what, a, b);
    exit(1);
}

int main(void) {
    static const uint32_t dims[] = {0, 1, 2, 63, 64, 8191, 8192, 8193, 16383, 16384, 16385,
                                    262143, 262144, 262145, 0x7fffffffu, 0x80000000u, 0xffffffffu};
    static const uint32_t precs[] = {0, 1, 2, 3, 255, 256, 257, 0x7fffffffu, 0xffffffffu};
    const size_t nd = sizeof dims / sizeof dims[0], np = sizeof precs / sizeof precs[0];
    for (size_t a = 0; a < nd; a++)
        for (size_t b = 0; b < nd; b++)
            for (size_t c = 0; c < nd; c++)
                for (size_t p = 0; p < np; p++) {
                    OmegaMatMulSpec x, y;
                    memset(&x, 0xa5, sizeof x);
                    memset(&y, 0xa5, sizeof y);
                    int rx = legacy_omega_matmul_spec_init(&x, dims[a], dims[b], dims[c], (OmegaMatMulPrecision)precs[p]);
                    int ry = omega_matmul_spec_init(&y, dims[a], dims[b], dims[c], (OmegaMatMulPrecision)precs[p]);
                    cases++;
                    if (rx != ry || memcmp(&x, &y, sizeof x)) fail("spec", a * 10000 + b * 100 + c, p);
                    if (rx) refused++;
                    if (rx) continue;
                    OmegaBlackwellKernel k;
                    memset(&k, 0, sizeof k);
                    for (int i = 0; i < 32; i++) k.code_digest[i] = (uint8_t)next();
                    OmegaBlackwellRealizationIdentity ix, iy;
                    memset(&ix, 0x5a, sizeof ix);
                    memset(&iy, 0x5a, sizeof iy);
                    rx = legacy_omega_blackwell_bind_matmul_realization(&x, &k, &ix);
                    ry = omega_blackwell_bind_matmul_realization(&y, &k, &iy);
                    cases++;
                    if (rx != ry || memcmp(&ix, &iy, sizeof ix)) fail("bound", a * 10000 + b * 100 + c, p);
                }
    {
        OmegaMatMulSpec s;
        OmegaBlackwellKernel k;
        OmegaBlackwellRealizationIdentity id;
        memset(&s, 0, sizeof s);
        memset(&k, 0, sizeof k);
        if (omega_matmul_spec_init(NULL, 1, 1, 1, 0) != -1 || legacy_omega_matmul_spec_init(NULL, 1, 1, 1, 0) != -1)
            fail("spec null", 0, 0);
        if (omega_blackwell_bind_matmul_realization(NULL, &k, &id) != -1 ||
            omega_blackwell_bind_matmul_realization(&s, NULL, &id) != -1 ||
            omega_blackwell_bind_matmul_realization(&s, &k, NULL) != -1)
            fail("bound null", 0, 0);
        cases += 4;
        refused += 4;
    }
    /* dispatch: unsupported precisions refuse in both; supported ones generate the same kernel */
    for (size_t p = 0; p < np; p++) {
        OmegaMatMulSpec s;
        memset(&s, 0, sizeof s);
        s.m = 16;
        s.k = 16;
        s.n = 16;
        s.precision = (OmegaMatMulPrecision)precs[p];
        OmegaBlackwellKernel kx, ky;
        memset(&kx, 0, sizeof kx);
        memset(&ky, 0, sizeof ky);
        int rx = legacy_omega_blackwell_codegen_matmul(&s, &kx);
        int ry = omega_blackwell_codegen_matmul(&s, &ky);
        cases++;
        if (rx != ry) fail("dispatch", p, (unsigned long)(rx - ry));
        if (rx) {
            refused++;
            continue;
        }
        if (kx.code_size != ky.code_size || memcmp(kx.code, ky.code, kx.code_size) ||
            memcmp(kx.code_digest, ky.code_digest, 32))
            fail("dispatch kernel", p, 0);
        omega_blackwell_kernel_free(&kx);
        omega_blackwell_kernel_free(&ky);
    }
    {
        SemanticId mx, my;
        if (legacy_omega_blackwell_get_machine_id(&mx) || omega_blackwell_get_machine_id(&my) ||
            memcmp(&mx, &my, sizeof mx) || omega_blackwell_get_machine_id(NULL) != -1)
            fail("machine", 0, 0);
        cases += 2;
    }
    static const uint32_t archs[] = {0, 1, 121, 255, 256, 65535, 65536, 0x80000000u, 0xffffffffu};
    for (size_t i = 0; i < sizeof archs / sizeof archs[0]; i++)
        for (int r = 0; r < 64; r++) {
            OmegaBlackwellRealization x, y;
            memset(&x, 0, sizeof x);
            for (size_t j = 0; j < sizeof x; j++) ((uint8_t *)&x)[j] = (uint8_t)next();
            x.sm_architecture = archs[i];
            y = x;
            int rx = legacy_omega_blackwell_realization_compute_id(&x);
            int ry = omega_blackwell_realization_compute_id(&y);
            cases++;
            if (rx != ry || memcmp(&x, &y, sizeof x)) fail("compute id", i, (unsigned long)r);
        }
    if (omega_blackwell_realization_compute_id(NULL) != -1) fail("compute id null", 0, 0);
    static const uint32_t counts[] = {0, 1, 2, 63, 64, 65, 127, 128, 129, 0xffffffbfu, 0xffffffc0u,
                                      0xffffffc1u, 0xfffffffeu, 0xffffffffu};
    for (size_t i = 0; i < sizeof counts / sizeof counts[0]; i++) {
        OmegaVectorSpec v;
        memset(&v, 0, sizeof v);
        for (int j = 0; j < 32; j++) v.spec_id.bytes[j] = (uint8_t)next();
        v.element_count = counts[i];
        OmegaBlackwellRealization x, y;
        memset(&x, 0x33, sizeof x);
        memset(&y, 0x33, sizeof y);
        int rx = legacy_omega_blackwell_realize_vector(&v, &x);
        int ry = omega_blackwell_realize_vector(&v, &y);
        cases++;
        if (rx != ry || memcmp(&x, &y, sizeof x)) fail("vector", i, x.qmd_cfg.grid_width);
    }
    {
        uint8_t buf[3] = {1, 2, 3}, dx[32], dy[32];
        if (omega_blackwell_compute_code_digest(buf, 0, dx) != -1 || omega_blackwell_compute_code_digest(NULL, 1, dx) != -1 ||
            omega_blackwell_compute_code_digest(buf, 1, NULL) != -1)
            fail("digest refuse", 0, 0);
        for (size_t n = 1; n <= 3; n++) {
            sha256_hash(buf, n, dx);
            if (omega_blackwell_compute_code_digest(buf, n, dy) || memcmp(dx, dy, 32)) fail("digest", n, 0);
        }
        cases += 6;
        refused += 3;
    }
    static BlackwellIRProgram prog;
    static const size_t pc[] = {0, 1, 7, 8, 9, 24, 25, 31, 32, 33, 39, 40, 41, 2047, 2048,
                                SIZE_MAX / 16, SIZE_MAX - 8, SIZE_MAX - 7, SIZE_MAX - 6, SIZE_MAX};
    for (size_t i = 0; i < sizeof pc / sizeof pc[0]; i++) {
        prog.count = pc[i];
        cases++;
        if (legacy_omega_bw_program_code_bytes(&prog) != omega_bw_program_code_bytes(&prog)) fail("pad", i, 0);
    }
    if (omega_bw_program_code_bytes(NULL) != 0) fail("pad null", 0, 0);
    {
        omega_bw_ir_init(&prog);
        static uint8_t cx[1024], cy[1024];
        static const size_t caps[] = {0, 1, 511, 512, 513, 1024};
        for (size_t i = 0; i < sizeof caps / sizeof caps[0]; i++) {
            OmegaBlackwellKernel kx, ky;
            memset(cx, 0, sizeof cx);
            memset(cy, 0, sizeof cy);
            int rx = legacy_omega_blackwell_kernel_encode_into(&prog, cx, caps[i], &kx);
            int ry = omega_blackwell_kernel_encode_into(&prog, cy, caps[i], &ky);
            cases++;
            if (rx != ry) fail("fit", i, (unsigned long)(rx - ry));
            if (rx) {
                refused++;
                continue;
            }
            if (kx.code_size != ky.code_size || kx.insn_count != ky.insn_count || memcmp(cx, cy, sizeof cx))
                fail("fit kernel", i, 0);
        }
    }
    printf("PASS bind differential cases=%lu refused=%lu\n", cases, refused);
    return 0;
}
