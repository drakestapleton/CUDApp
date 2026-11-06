/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_om_ir.h"
#include "omega_bw_fields.h"
#include "omega_cubin.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int legacy_omega_gpu_om_ir_replay(OmegaOmIrUnit, uint64_t, uint64_t, uint64_t, uint64_t,
                                  BlackwellIRProgram *, uint64_t *);
int legacy_omega_bw_encode_program(const BlackwellIRProgram *, uint8_t *, size_t, size_t *);
int sass_omega_bw_encode_program(const BlackwellIRProgram *, uint8_t *, size_t, size_t *);
int legacy_omega_blackwell_encode_one(const BlackwellIRInsn *, uint32_t *);
int legacy_omega_gpu_om_ir_build(uint64_t, uint64_t, uint64_t, uint64_t, uint32_t,
    BlackwellIRProgram *, uint8_t *, size_t, OmegaBlackwellKernel *, uint32_t *, uint64_t *);
int legacy_omega_gpu_om_ir_data(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t *);
const char *legacy_omega_gpu_om_ir_rc_name(int);
static uint64_t tape[OMEGA_OM_IR_WORDS], metadata[OMEGA_OM_IR_META];
static uint32_t emit_status;
static uint64_t emit_result;
static unsigned cases, images;
static uint32_t emitter(uint64_t *r, uint64_t k, uint64_t a, uint64_t b, uint64_t c,
                        void *ins, uint64_t ni, void *meta, uint64_t nm) {
    (void)k; (void)a; (void)b; (void)c;
    assert(ni == OMEGA_OM_IR_WORDS && nm == OMEGA_OM_IR_META);
    memcpy(ins, tape, sizeof tape); memcpy(meta, metadata, sizeof metadata); *r = emit_result; return emit_status;
}
static void reset(void) {
    memset(tape, 0, sizeof tape); memset(metadata, 0, sizeof metadata);
    metadata[0] = 1; emit_result = 0; emit_status = 0;
    for (unsigned i = 0; i < BW_MAX_IR_INSNS; ++i) {
        tape[8 * i] = BW_IR_EXIT;
        for (unsigned f = 1; f <= 5; ++f) tape[8 * i + f] = UINT64_MAX;
    }
    for (unsigned v = 0; v < BW_MAX_VREGS; ++v) metadata[64 + v] = 1;
    for (unsigned u = 0; u < BW_MAX_UVREGS; ++u) metadata[320 + u] = 1;
}
static void compare_images(const BlackwellIRProgram *p, const BlackwellIRProgram *q) {
    uint8_t a[OMEGA_BW_CODE_CAP_MAX], b[OMEGA_BW_CODE_CAP_MAX], c[OMEGA_BW_CODE_CAP_MAX];
    size_t an = 0, bn = 0, cn = 0;
    int ar = omega_bw_encode_program(p, a, sizeof a, &an);
    int br = legacy_omega_bw_encode_program(q, b, sizeof b, &bn);
    int cr = sass_omega_bw_encode_program(q, c, sizeof c, &cn);
    assert(ar == br && ar == cr);
    if (ar) return;
#ifdef CODEGEN_BASE_RED
    a[0] ^= 1;
#endif
    assert(an == bn && memcmp(a, b, an) == 0);
    assert(an == cn && memcmp(a, c, an) == 0);
    uint8_t *ac = NULL, *bc = NULL; size_t al = 0, bl = 0; char err[128];
    assert(omega_cubin_pack("codegen_base", a, an, NULL, 0, 1024, 64, 0, &ac, &al, err, sizeof err) == 0);
    assert(omega_cubin_pack("codegen_base", b, bn, NULL, 0, 1024, 64, 0, &bc, &bl, err, sizeof err) == 0);
    assert(al == bl && memcmp(ac, bc, al) == 0);
    omega_cubin_free(ac); omega_cubin_free(bc); ++images;
}
static int check(int encode) {
    BlackwellIRProgram p, q; uint64_t pe = UINT64_MAX, qe = UINT64_MAX;
    memset(&p, 0, sizeof p); memset(&q, 0, sizeof q);
    int a = omega_gpu_om_ir_replay(emitter, 0, 0, 0, 0, &p, &pe);
    int b = legacy_omega_gpu_om_ir_replay(emitter, 0, 0, 0, 0, &q, &qe);
    if (a != b) { fprintf(stderr, "case %u replay %d C %d\n", cases, a, b); abort(); }
    assert(pe == qe); ++cases;
    if (a == 0) {
        assert(memcmp(&p, &q, sizeof p) == 0);
        if (encode) compare_images(&p, &q);
    }
    return a;
}
static void boundaries(void) {
    static const struct { unsigned slot; uint64_t values[16]; unsigned count; } fields[] = {
        {0, {0,1,2,7,8,9,31,32,33,1023,1024,1025,UINT32_MAX}, 13},
        {1, {0,1,2,159,160,161,255,256,257,UINT32_MAX}, 10},
        {2, {0,1,2,31,32,33,UINT32_MAX}, 7},
        {5, {0,1,2,23,24,25,UINT32_MAX}, 7},
        {6, {0,1,127,128,129,245,246,247,UINT32_MAX}, 9},
        {7, {0,1,2,UINT32_MAX}, 4}
    };
    reset(); assert(check(1) == 0);
    for (unsigned f = 0; f < sizeof fields / sizeof fields[0]; ++f)
        for (unsigned v = 0; v < fields[f].count; ++v) {
            reset(); metadata[fields[f].slot] = fields[f].values[v];
            check(fields[f].slot == 0);
        }
    static const uint64_t widths[] = {0,1,2,3,4,5,UINT32_MAX};
    for (unsigned v = 0; v < sizeof widths / sizeof widths[0]; ++v) {
        reset(); metadata[1] = 1; metadata[64] = widths[v]; check(0);
        reset(); metadata[2] = 1; metadata[320] = widths[v]; check(0);
    }
    static const uint64_t regs[] = {0,1,30,31,32,33,254,255,256,257,UINT32_MAX,UINT64_MAX};
    for (unsigned f = 1; f <= 5; ++f)
        for (unsigned v = 0; v < sizeof regs / sizeof regs[0]; ++v) {
            reset(); tape[f] = regs[v]; check(0);
        }
    for (unsigned op = 0; op <= (unsigned)BW_IR_ATOMG_ADD_F32_STRONG_GPU + 1; ++op) {
        reset(); tape[0] = op; check(1);
    }
    reset(); tape[0] = UINT32_MAX; check(0);
    /* All control/flag wire bits go through the real encoder, not only the field unit. */
    for (unsigned bit = 0; bit < 64; ++bit) {
        reset(); metadata[1] = 1; tape[0] = BW_IR_MOV_IMM; tape[1] = 0;
        tape[6] = UINT64_C(1) << bit; check(1);
        tape[7] = UINT64_C(1) << bit; check(1);
    }
    reset(); metadata[7] = 1; metadata[1] = 161; assert(check(0) == OMEGA_OM_IR_LIVE_RANGE);
    for (unsigned s = 0; s < 4; ++s) {
        reset(); emit_status = s; metadata[4] = 42; check(0);
        reset(); emit_result = s; metadata[4] = 42; check(0);
    }
}
static void loops(void) {
    reset(); metadata[0] = 5; metadata[1] = 2; metadata[5] = 1;
    metadata[16] = 1; metadata[17] = 3;
    tape[0] = BW_IR_MOV_IMM; tape[1] = 0; tape[6] = 42;
    tape[8] = BW_IR_MOV_IMM; tape[9] = 1;
    tape[16] = BW_IR_IADD3; tape[17] = 1; tape[18] = 0;
    assert(check(1) == 0); /* v0 is live into the loop and must extend from record 2 to 3 */
    tape[34] = 1; assert(check(0) == OMEGA_OM_IR_LOOP);
    tape[33] = 1; tape[34] = UINT64_MAX; assert(check(0) == 0); /* redefine before a later read */
    metadata[0] = 6; tape[42] = 1; assert(check(0) == 0);
    tape[39] = 1; assert(check(0) == OMEGA_OM_IR_LOOP); /* uniform destination is not a redefinition */
    tape[39] = 0; tape[42] = UINT64_MAX;
    for (unsigned f = 2; f <= 4; ++f) {
        tape[33] = UINT64_MAX; tape[32 + f] = 1;
        assert(check(0) == OMEGA_OM_IR_LOOP);
        tape[32 + f] = UINT64_MAX;
    }
    /* Definition after the loop, pre-loop-only use, and a use after closure. */
    reset(); metadata[0] = 5; metadata[1] = 2; metadata[5] = 1;
    metadata[16] = 1; metadata[17] = 3;
    tape[0] = BW_IR_MOV_IMM; tape[1] = 0;
    tape[32] = BW_IR_MOV_IMM; tape[33] = 1; assert(check(1) == 0);
    tape[34] = 0; assert(check(1) == 0);
    tape[34] = UINT64_MAX; tape[26] = 0; assert(check(1) == 0); /* last use at closure */
    reset(); metadata[0] = 5; metadata[1] = 1; metadata[7] = 1;
    tape[0] = BW_IR_MOV_IMM; tape[1] = 0;
    static const uint64_t hints[] = {0,1,3,4,5,6,UINT32_MAX};
    for (unsigned i = 0; i < sizeof hints / sizeof hints[0]; ++i) {
        metadata[352] = hints[i]; check(0);
    }
    reset(); metadata[1] = 1; metadata[7] = 1; metadata[352] = 1;
    assert(check(0) == OMEGA_OM_IR_LIVE_RANGE); /* never defined */
    static const uint64_t endpoints[] = {0,1,2,3,4,5,UINT32_MAX,UINT64_MAX};
    for (unsigned s = 0; s < sizeof endpoints / sizeof endpoints[0]; ++s)
        for (unsigned e = 0; e < sizeof endpoints / sizeof endpoints[0]; ++e) {
            reset(); metadata[0] = 5; metadata[1] = 1; metadata[5] = 1;
            metadata[16] = endpoints[s]; metadata[17] = endpoints[e]; check(0);
        }
    reset(); metadata[0] = 5; metadata[5] = 2; metadata[17] = 3; metadata[19] = 2;
    assert(check(0) == OMEGA_OM_IR_REPLAY);
    metadata[19] = 3; assert(check(0) == 0); /* two loops sharing their closing instruction */
}
static void wire_fields(void) {
    static const uint32_t values[] = {0,1,7,8,9,15,16,17,31,32,33,255,256,257,UINT32_MAX};
    for (unsigned slot = 0; slot < 16; ++slot)
        for (unsigned v = 0; v < sizeof values / sizeof values[0]; ++v) {
            uint64_t raw[16] = {0}, rec[8] = {0}, r = 0, expected[8] = {0};
            raw[slot] = values[v];
            for (unsigned f = 0; f < 6; ++f) expected[f] = raw[f];
            expected[6] = raw[6] | raw[7] << 32;
            expected[7] = raw[8] | raw[9] << 1 | raw[10] << 2 | raw[11] << 8 |
                          raw[12] << 16 | raw[13] << 24 | raw[14] << 32 | raw[15] << 40;
            assert(omega_cg_fields(&r, 0, raw, 16, rec, 8) == 0 && r == 0);
            assert(memcmp(rec, expected, sizeof rec) == 0);
        }
    for (unsigned bit = 0; bit < 64; ++bit) {
        uint64_t raw[16] = {0}, rec[8] = {0}, r = 0;
        rec[6] = rec[7] = UINT64_C(1) << bit;
        assert(omega_cg_fields(&r, 1, raw, 16, rec, 8) == 0 && r == 0);
        assert(raw[6] == (uint32_t)rec[6] && raw[7] == rec[6] >> 32);
        assert(raw[8] == (rec[7] & 1) && raw[9] == ((rec[7] >> 1) & 1) && raw[10] == ((rec[7] >> 2) & 1));
        assert(raw[11] == ((rec[7] >> 48) & 255));
        for (unsigned f = 0; f < 4; ++f) assert(raw[12 + f] == ((rec[7] >> (16 + 8 * f)) & 255));
    }
    uint64_t raw[16], rec[8] = {0}, r = 0;
    for (unsigned i = 0; i < 16; ++i) raw[i] = i + 1;
    assert(omega_cg_fields(&r, 2, raw, 16, rec, 8) == 0 && r == 0);
    assert(raw[0] == 0x04030201 && raw[1] == 0x08070605 && raw[2] == 0x0c0b0a09 && raw[3] == 0x100f0e0d);
    assert(omega_cg_fields(&r, 3, raw, 16, rec, 8) == 0 && r == OMEGA_OM_IR_REPLAY);
    for (unsigned v = 0; v < BW_MAX_VREGS; ++v) {
        BlackwellIRInsn in = { .op = BW_IR_MOV_IMM, .dst_vreg = (int)v, .src1_vreg = -1,
            .src2_vreg = -1, .src3_vreg = -1, .ureg = -1, .imm = UINT32_MAX };
        uint32_t a[4], b[4];
        assert(omega_blackwell_encode_one(&in, a) == legacy_omega_blackwell_encode_one(&in, b));
        assert(memcmp(a, b, sizeof a) == 0);
    }
}
static void budgets(void) {
    static const uint32_t vals[] = {0,1,2,7,8,9,63,64,65,127,128,129,245,246,247,1023,1024,1025,UINT32_MAX};
    for (unsigned pi = 0; pi < sizeof vals / sizeof vals[0]; ++pi)
        for (unsigned ti = 0; ti < sizeof vals / sizeof vals[0]; ++ti) {
            uint32_t peak = vals[pi], th = vals[ti], decl = omega_bw_gpr_declared(peak);
            if (decl < 64) decl = 64;
            uint64_t expected = th == 0 ? UINT64_MAX :
                peak > BW_PHYS_GPR_MAX || !omega_bw_gpr_declared_ok(decl, peak) || (uint64_t)decl * th > 65536 ? 0 : decl;
            uint64_t r = 0;
            assert(omega_cg_budget(&r, peak, omega_bw_gpr_declared(peak), th) == 0);
            assert(r == expected);
        }
}
static void builders(void) {
    static const uint32_t threads[] = {0,1,63,64,65,1024,1025,UINT32_MAX};
    BlackwellIRProgram p, q; uint8_t a[OMEGA_BW_CODE_CAP_MAX], b[OMEGA_BW_CODE_CAP_MAX];
    OmegaBlackwellKernel x, y; uint32_t pd, qd; uint64_t pe, qe;
    for (unsigned kind = 0; kind <= 3; ++kind)
        for (unsigned arg = 0; arg <= 12; ++arg)
            for (unsigned t = 0; t < sizeof threads / sizeof threads[0]; ++t)
                for (unsigned cap = 0; cap < 2; ++cap) {
                    size_t n = cap ? sizeof a : 0;
                    int ar = omega_gpu_om_ir_build(kind, arg, 0, 0, threads[t], &p, a, n, &x, &pd, &pe);
                    int br = legacy_omega_gpu_om_ir_build(kind, arg, 0, 0, threads[t], &q, b, n, &y, &qd, &qe);
                    assert(ar == br);
                    if (!ar) {
                        assert(pd == qd && x.code_size == y.code_size);
                        assert(memcmp(a, b, x.code_size) == 0 && memcmp(&p, &q, sizeof p) == 0);
                    }
                }
    for (unsigned kind = 0; kind <= 3; ++kind) {
        uint64_t a[OMEGA_OM_IR_WORDS], b[OMEGA_OM_IR_WORDS];
        assert(omega_gpu_om_ir_data(kind, 2, 0, 0, a) == legacy_omega_gpu_om_ir_data(kind, 2, 0, 0, b));
        assert(memcmp(a, b, sizeof a) == 0);
    }
    for (int rc = -1; rc <= 12; ++rc)
        assert(strcmp(omega_gpu_om_ir_rc_name(rc), legacy_omega_gpu_om_ir_rc_name(rc)) == 0);
    assert(omega_gpu_om_ir_data(0, 0, 0, 0, NULL) == OMEGA_OM_IR_REPLAY);
    assert(legacy_omega_gpu_om_ir_data(0, 0, 0, 0, NULL) == OMEGA_OM_IR_REPLAY);
}
int main(void) {
    boundaries(); loops(); wire_fields(); budgets(); builders();
    printf("CODEGEN BASE PASS: %u replay partitions; %u byte-identical SASS/cubin images; wire bits and budget boundaries\n", cases, images);
    return 0;
}
