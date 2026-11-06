/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_OM_IR_H
#define OMEGA_GPU_OM_IR_H
/*
 * omega_gpu_om_ir: replay of BlackwellIR records emitted by a seeded Omega unit (om/mathlibs/mathlibs_ir.om,
 * bound through src/cudapp_om_units.c) into a BlackwellIRProgram, then register allocation, the loop live-range
 * check and encoding (the .om sm_121 encoder behind omega_blackwell_kernel_encode_into).
 *
 * MIGRATION-PENDING C: this file is the register-allocation and replay glue only. Instruction selection,
 * control words, loops and the fixed-pair schedule are decided in the .om unit. Successor: a seeded-Omega
 * live-interval allocator next to om/encoder/bw_enc_sm121.om, after which the records go straight to the
 * .om encoder and this file is deleted.
 *
 * Record (8 x u64): op, dst, src1, src2, src3 (vreg, or all-ones for none), ureg (or all-ones), imm | control << 32,
 * uniform | p0 << 1 | not << 2 | dst_sub << 16 | src1_sub << 24 | src2_sub << 32 | src3_sub << 40 | bar_reg << 48.
 * Meta (512 x u64): 0 records, 1 vregs, 2 uvregs, 4 emitter error, 5 closed loops,
 * 6 requested exclusive GPR limit at successful return (0 retains the initializer's historical default;
 * otherwise BW_PHYS_GPR_MAX < limit <= BW_PHYS_GPR_HW_LIMIT). Emitters may use slot 6 as scratch before
 * successful return, but must clear it unless requesting the extended allocator.
 * 16 + 2L / 17 + 2L loop L first record / closing BRA, 64 + v vreg kind (1 scalar, 2 pair, 4 quad), 320 + u uvreg kind.
 * 7 exact-liveness flag: 0 preserves the default wire; 1 enables last-use extensions for at most 160 vregs.
 * Meta[352 + v], 0 <= v < 160: zero means absent, otherwise the requested instruction index. Every override
 * must be >= the computed definition and last-use indices and < records; it never shortens or clamps.
 * With flag 1, vreg count > 160 is refused, so index 512 (v=160) cannot be interpreted. Other flag values
 * have a distinct refusal. Slots 0..7 are header fields; 16..63 contain at most 24 loop pairs;
 * 64..319 contain 256 vreg widths; 320..351 contain 32 uniform widths; 352..511 fit META=512 exactly.
 */
#include <stddef.h>
#include <stdint.h>
#include "omega_blackwell_codegen.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OMEGA_OM_IR_WORDS 8192u /* record words: 1024 records */
#define OMEGA_OM_IR_META 512u

enum {
    OMEGA_OM_IR_OK = 0,
    OMEGA_OM_IR_REFUSED = 1,    /* the unit refused the arguments */
    OMEGA_OM_IR_EMIT = 2,       /* the unit's emitter failed (meta[4] holds its code) */
    OMEGA_OM_IR_BRIDGE = 3,     /* the bound call itself failed */
    OMEGA_OM_IR_REPLAY = 4,     /* a record or register the IR rejects */
    OMEGA_OM_IR_REGALLOC = 5,   /* the allocator failed */
    OMEGA_OM_IR_LOOP = 6,       /* a value defined in a loop is read after it */
    OMEGA_OM_IR_BUDGET = 7,     /* declared registers x threads past 65536, or past the allocation limit */
    OMEGA_OM_IR_ENCODE = 8,     /* the encoder refused an instruction or the buffer */
    OMEGA_OM_IR_LIVE_FLAG = 9,  /* exact-liveness flag is not 0 or 1 */
    OMEGA_OM_IR_LIVE_RANGE = 10 /* invalid extension, or flag 1 with more than 160 vregs */
};

/* Checked portable unit ABI. Replay does not encode or apply a launch budget. */
typedef uint32_t (*OmegaOmIrUnit)(uint64_t *, uint64_t, uint64_t, uint64_t, uint64_t,
                                 void *, uint64_t, void *, uint64_t);
int omega_gpu_om_ir_replay(OmegaOmIrUnit unit, uint64_t kind, uint64_t a, uint64_t b, uint64_t c,
                           BlackwellIRProgram *prog, uint64_t *emit_err);

/* Calls the unit (kind, a, b, c), replays its records into *prog, allocates, checks and encodes into code
 * (capacity code_cap) for a launch of `threads` threads per CTA. On success *kernel points into code and
 * *declared holds the QMD register declaration (omega_bw_gpr_declared). *emit_err gets meta[4] (may be NULL). */
int omega_gpu_om_ir_build(uint64_t kind, uint64_t a, uint64_t b, uint64_t c, uint32_t threads,
                          BlackwellIRProgram *prog, uint8_t *code, size_t code_cap,
                          OmegaBlackwellKernel *kernel, uint32_t *declared, uint64_t *emit_err);

/* Raw unit call for data kinds (kind 2: FFT twiddles): words receives OMEGA_OM_IR_WORDS u64. */
int omega_gpu_om_ir_data(uint64_t kind, uint64_t a, uint64_t b, uint64_t c, uint64_t *words);

const char *omega_gpu_om_ir_rc_name(int rc);

#ifdef __cplusplus
}
#endif
#endif
