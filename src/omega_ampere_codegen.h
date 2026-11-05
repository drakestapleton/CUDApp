/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_AMPERE_CODEGEN_H
#define OMEGA_AMPERE_CODEGEN_H
#include "omega_blackwell_codegen.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Explicit target; never guesses from the host or a GPU. 121 preserves the existing
 * encoder. 86 lowers solved IR to Ampere, including parameter-bank offsets.
 * Native FP8 QMMA, E4M3 PACK, and every opcode past MUFU.LG2 are refused.
 * Those opcodes are the training R5 entries (SHFL.BFLY, F2FP.BF16.F32.PACK_AB,
 * ATOMG.E.ADD.F32.STRONG.SYS, ATOMG.E.ADD.F32.STRONG.GPU): SM86 has no instruction
 * for them (om/encoder/ampere_enc_sm86.om, valid: op > 59).
 * 0 success, -1 invalid arguments/capacity, -2 unsupported target/IR/native trap.
 * Refusal writes no code bytes and sets *out_len=0. No driver access occurs. */
#define OMEGA_GPU_ENCODE_UNSUPPORTED (-2)
int omega_gpu_encode_program_target(const BlackwellIRProgram *prog, uint32_t target_sm,
                                    uint8_t *code_buf, size_t max_len, size_t *out_len);
#ifdef __cplusplus
}
#endif
#endif
