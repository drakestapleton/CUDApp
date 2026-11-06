/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_BW_FIELDS_H
#define OMEGA_BW_FIELDS_H
#include "omega_blackwell_codegen.h"
#include "cudapp_native.h"
/* C owns struct layout and pointers; cg_fields owns every wire bit position. */
static inline int omega_bw_fields_pack(const BlackwellIRInsn *in, uint64_t record[8]) {
    uint64_t raw[16] = {
        (uint64_t)(int64_t)in->op, (uint64_t)(int64_t)in->dst_vreg,
        (uint64_t)(int64_t)in->src1_vreg, (uint64_t)(int64_t)in->src2_vreg,
        (uint64_t)(int64_t)in->src3_vreg, (uint64_t)(int64_t)in->ureg,
        in->imm, in->control, in->is_uniform, in->predicate_p0, in->predicate_not,
        in->bar_reg, in->dst_subreg, in->src1_subreg, in->src2_subreg, in->src3_subreg
    }, result = 0;
    return omega_cg_fields(&result, 0, raw, 16, record, 8) != OMEGA_OK || result != 0 ? -1 : 0;
}
static inline int omega_bw_fields_unpack(uint64_t record[8], BlackwellIRInsn *in) {
    uint64_t raw[16] = {0}, result = 0;
    if (omega_cg_fields(&result, 1, raw, 16, record, 8) != OMEGA_OK || result != 0) return -1;
    in->op = (BlackwellIROpcode)raw[0];
    in->dst_vreg = (int)(int64_t)raw[1]; in->src1_vreg = (int)(int64_t)raw[2];
    in->src2_vreg = (int)(int64_t)raw[3]; in->src3_vreg = (int)(int64_t)raw[4];
    in->ureg = (int)(int64_t)raw[5]; in->imm = (uint32_t)raw[6]; in->control = (uint32_t)raw[7];
    in->is_uniform = (int)raw[8]; in->predicate_p0 = (int)raw[9]; in->predicate_not = (int)raw[10];
    in->bar_reg = (uint8_t)raw[11]; in->dst_subreg = (uint8_t)raw[12];
    in->src1_subreg = (uint8_t)raw[13]; in->src2_subreg = (uint8_t)raw[14]; in->src3_subreg = (uint8_t)raw[15];
    return 0;
}
#endif
