/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Hosted adapters only: pointer/null admission, bit transport, allocation and
 * diagnostic text. Numeric decisions belong to the om/numeric Omega units. */
#if defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC_REST_DIFF)
#undef omega_numeric_submit_check
#undef omega_numeric_patch_words
#undef omega_numeric_check_patch
#undef omega_numeric_element_checked
#undef omega_numeric_reference
#undef omega_numeric_cpu_realize
#undef omega_numeric_parity
#undef omega_numeric_seed_bound
#undef omega_numeric_reference_ftz
#endif

#if !defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC_REST)
uint64_t omega_numeric_rest(unsigned mode, uint64_t a, uint64_t b,
                            uint64_t c, uint64_t d, uint64_t e) {
    uint64_t result;
    if (omega_numeric_rest_run(&result, mode, a, b, c, d, e) != OMEGA_OK) __builtin_trap();
    return result;
}

/* Only strings live here; no machine words or encodings are computed in C. */
typedef struct { const char *text, *key; } NumericPatchLabel;
#define NL(t,k) { t, k }
#define NS NL("STG.E desc[UR4][R6.64], R9", NULL)
#define NV NL("STG.E desc[UR4][R6.64], R9 (waits SB0)", NULL)
#define NX NL("EXIT", NULL)
#define NC(p,k) NL("FSETP." p ".AND P0, PT, R2, R5, PT", "FSETP_" k "_R2_R5"), NL("FSEL R9, R2, R5, P0", "FSEL_R2_R5_P0"), NS, NX
static const NumericPatchLabel numeric_patch_labels[OMEGA_NOP_COUNT][OMEGA_NUMERIC_PATCH_MAX] = {
    [OMEGA_NOP_FADD] = { NL("FADD R9, R2, R5", "FADD") },
    [OMEGA_NOP_FSUB] = { NL("FADD R9, R2, -R5", "FSUB") },
    [OMEGA_NOP_FMUL] = { NL("FMUL R9, R2, R5", "FMUL") },
    [OMEGA_NOP_FFMA] = { NL("FFMA R9, R2, R5, R1", "FFMA") },
    [OMEGA_NOP_FSETP_SEL] = { NC("GE", "GE") },
    [OMEGA_NOP_FSEL] = { NL("FSETP.GE.AND P0, PT, R2, RZ, PT", "FSETP_GE_R2_RZ"), NL("FSEL R9, R5, R2, P0", "FSEL_R5_R2_P0"), NS, NX },
    [OMEGA_NOP_FMNMX_MIN] = { NL("FMNMX R9, R2, R5, PT", "FMNMX_MIN") },
    [OMEGA_NOP_FMNMX_MAX] = { NL("FMNMX R9, R2, R5, !PT", "FMNMX_MAX") },
    [OMEGA_NOP_I2FP] = { NL("I2FP.F32.S32 R9, R2", "I2FP") },
    [OMEGA_NOP_F2I] = { NL("F2I.TRUNC.NTZ R9, R2", "F2I"), NV, NX },
    [OMEGA_NOP_MUFU_RCP] = { NL("MUFU.RCP R9, R2", "MUFU_RCP"), NV, NX },
    [OMEGA_NOP_MUFU_RSQ] = { NL("MUFU.RSQ R9, R2", "MUFU_RSQ"), NV, NX },
    [OMEGA_NOP_SHFL_DOWN] = { NL("SHFL.DOWN PT, R9, R2, 0x1, 0x1f", "SHFL_DOWN_1"), NV, NX },
    [OMEGA_NOP_LDS_STS] = { NL("SHF.L.U32 R8, R0, 0x2, RZ", "SHF_L_R8_R0_2"), NL("LOP3.LUT R10, R8, 0xfc, RZ, 0x3c, !PT", "LOP3_R10_R8_XOR_FC"), NL("STS [R8+URZ], R2", "STS_R8_R2"), NL("BAR.SYNC.DEFER_BLOCKING 0x0", "BAR_SYNC_0"), NL("LDS R9, [R10+URZ]", "LDS_R9_R10"), NV, NX },
    [OMEGA_NOP_REDUCE_SUM] = { NL("SHFL.DOWN PT, R9, R2, 0x10, 0x1f", "SHFL_DOWN_16"), NL("FADD R2, R2, R9", "FADD_R2_R2_R9"), NL("SHFL.DOWN PT, R9, R2, 0x8, 0x1f", "SHFL_DOWN_8"), NL("FADD R2, R2, R9", "FADD_R2_R2_R9"), NL("SHFL.DOWN PT, R9, R2, 0x4, 0x1f", "SHFL_DOWN_4"), NL("FADD R2, R2, R9", "FADD_R2_R2_R9"), NL("SHFL.DOWN PT, R9, R2, 0x2, 0x1f", "SHFL_DOWN_2"), NL("FADD R2, R2, R9", "FADD_R2_R2_R9"), NL("SHFL.DOWN PT, R9, R2, 0x1, 0x1f", "SHFL_DOWN_1"), NL("FADD R9, R2, R9", "FADD_R9_R2_R9"), NS, NX },
    [OMEGA_NOP_FSETP_LT_SEL] = { NC("LT", "LT") },
    [OMEGA_NOP_FSETP_LE_SEL] = { NC("LE", "LE") },
    [OMEGA_NOP_FSETP_GT_SEL] = { NC("GT", "GT") },
    [OMEGA_NOP_FSETP_EQ_SEL] = { NC("EQ", "EQ") },
    [OMEGA_NOP_FSETP_NE_SEL] = { NC("NE", "NE") },
    [OMEGA_NOP_FSETP_NUM_SEL] = { NC("NUM", "NUM") },
    [OMEGA_NOP_FSETP_NAN_SEL] = { NC("NAN", "NAN") },
    [OMEGA_NOP_FSETP_LTU_SEL] = { NC("LTU", "LTU") },
    [OMEGA_NOP_FSETP_LEU_SEL] = { NC("LEU", "LEU") },
    [OMEGA_NOP_FSETP_GTU_SEL] = { NC("GTU", "GTU") },
    [OMEGA_NOP_FSETP_GEU_SEL] = { NC("GEU", "GEU") },
    [OMEGA_NOP_FSETP_EQU_SEL] = { NC("EQU", "EQU") },
    [OMEGA_NOP_FSETP_NEU_SEL] = { NC("NEU", "NEU") },
    [OMEGA_NOP_F2I_FLOOR] = { NL("F2I.FLOOR.NTZ R9, R2", "F2I_FLOOR"), NV, NX },
    [OMEGA_NOP_F2I_CEIL] = { NL("F2I.CEIL.NTZ R9, R2", "F2I_CEIL"), NV, NX },
    [OMEGA_NOP_F2I_RNI] = { NL("F2I.NTZ R9, R2", "F2I_RNI"), NV, NX },
    [OMEGA_NOP_F2U] = { NL("F2I.U32.TRUNC.NTZ R9, R2", "F2U"), NV, NX },
    [OMEGA_NOP_I2FP_U32] = { NL("I2FP.F32.U32 R9, R2", "I2FP_U32") },
    [OMEGA_NOP_F32_TO_F16] = { NL("F2F.F16.F32 R9, R2", "F2F_F16_F32"), NV, NX },
    [OMEGA_NOP_F32_TO_BF16] = { NL("F2F.BF16.F32 R9, R2", "F2F_BF16_F32"), NV, NX },
    [OMEGA_NOP_F16_TO_F32] = { NL("HADD2.F32 R9, -RZ, R2.H0_H0", "HADD2_F32_R2_H0" ) },
    [OMEGA_NOP_BF16_TO_F32] = { NL("SHF.L.U32 R9, R2, 0x10, RZ", "SHF_L_R9_R2_16") },
    [OMEGA_NOP_FFMA_V] = { NL("LDC.64 R10, c[0x0][0x3a0]", "LDC64_R10_C3A0"), NL("IMAD.WIDE.U32 R10, R9, 0x4, R10", "IMAD_WIDE_R10_R9_4"), NL("LDG.E R11, desc[UR4][R10.64]", "LDG_R11_R10"), NL("FFMA R9, R2, R5, R11", "FFMA_R9_R2_R5_R11"), NS, NX }
};
#undef NC
#undef NX
#undef NV
#undef NS
#undef NL

int omega_numeric_patch_words(OmegaNumericOp op, OmegaNumericPatchInsn out[OMEGA_NUMERIC_PATCH_MAX]) {
    uint64_t n;
    if (omega_numeric_patch(&n, 0, (unsigned)op, 0, 0) != OMEGA_OK) __builtin_trap();
    if (n > OMEGA_NUMERIC_PATCH_MAX) return (int32_t)(uint32_t)n;
    for (size_t i = 0; i < n; ++i) {
        for (unsigned k = 0; k < 4; ++k) {
            uint64_t word;
            if (omega_numeric_patch(&word, 1, op, i, k) != OMEGA_OK) __builtin_trap();
            out[i].w[k] = (uint32_t)word;
        }
        out[i].text = numeric_patch_labels[op][i].text;
        out[i].provenance_key = numeric_patch_labels[op][i].key;
    }
    return (int)n;
}

int omega_numeric_check_patch(OmegaNumericOp op, const OmegaNumericPatchInsn *p, int n,
                              const uint32_t *qmd, char *err, size_t err_len) {
    if (err && err_len) err[0] = '\0';
    uint64_t rc = omega_numeric_rest(29, (unsigned)op, (unsigned)n, p && qmd, 0, 0);
    if (rc) return bad(err, err_len, "check_patch: missing patch/QMD or patch length outside 1..15");
    uint64_t words[60] = {0}, launch[96];
    for (int i = 0; i < n; ++i)
        for (unsigned k = 0; k < 4; ++k) words[4*i+k] = p[i].w[k];
    for (unsigned i = 0; i < 96; ++i) launch[i] = qmd[i];
    if (omega_numeric_check(&rc, (unsigned)op, (unsigned)n, words, 60, launch, 96) != OMEGA_OK) __builtin_trap();
    if (rc) return bad(err, err_len, "check_patch: Omega structural CHECK %llu refused", (unsigned long long)rc);
    return OMEGA_NUMERIC_OK;
}

int omega_numeric_submit_check(const char *name, const float *a, const float *b, const float *c,
                               const float *out, size_t count, char *err, size_t err_len) {
    if (err && err_len) err[0] = '\0';
    for (size_t v = 0; name && v < REFUSED_VARIANT_COUNT; ++v)
        if (!strcmp(name, REFUSED_VARIANTS[v].name))
            return refuse(err, err_len, OMEGA_NUMERIC_ERR_NOT_ENCODED, "variant %s is not encoded for GB10: %s", name, REFUSED_VARIANTS[v].reason);
    const OmegaNumericOpInfo *info = omega_numeric_op_find(name);
    if (!info) return refuse(err, err_len, OMEGA_NUMERIC_ERR_BAD_ARGS, "unknown numeric op '%s'%s", name ? name : "(null)", NULL);
    uint64_t flags = (uint64_t)!!a | ((uint64_t)!!b << 1) | ((uint64_t)!!c << 2) | ((uint64_t)!!out << 3);
    int rc = -(int)omega_numeric_rest(27, info->op, count, flags, info->arity, info->gb10_encoded);
    if (rc) return refuse(err, err_len, rc, "op %s: numeric launch operands refused%s", info->name, NULL);
    bool uniform = omega_numeric_rest(32, info->op, 0, 0, 0, 0) != 0;
    for (size_t i = 1; uniform && c && i < count; ++i)
        if (!omega_numeric_rest(28, info->op, omega_float_to_bits(c[0]), omega_float_to_bits(c[i]), 0, 0))
            return refuse(err, err_len, OMEGA_NUMERIC_ERR_OPERANDS, "op %s: c must be the same value in every element%s", info->name, NULL);
    if (omega_numeric_op_whole_kernel(info->op)) return OMEGA_NUMERIC_OK;
    OmegaNumericPatchInsn patch[OMEGA_NUMERIC_PATCH_MAX];
    int n = omega_numeric_patch_words(info->op, patch);
    if (n <= 0) return refuse(err, err_len, OMEGA_NUMERIC_ERR_NOT_ENCODED, "op %s has no patch words%s", info->name, NULL);
    uint32_t qmd[OMEGA_BW_QMD_WORDS];
    OmegaBlackwellQmdConfig cfg = {0};
    cfg.num_elements = (uint32_t)count;
    omega_numeric_launch_shape(count, &cfg.threads_per_block, &cfg.grid_width);
    if (omega_blackwell_build_qmd1(qmd, &cfg)) return refuse(err, err_len, OMEGA_NUMERIC_ERR_BAD_ARGS, "op %s: launch QMD could not be built%s", info->name, NULL);
    return omega_numeric_check_patch(info->op, patch, n, qmd, err, err_len);
}

bool omega_numeric_element_checked(OmegaNumericOp op, size_t index) {
    return omega_numeric_rest(13, (unsigned)op, index, 0, 0, 0) != 0;
}

static int numeric_bulk_adapter(OmegaNumericOp op, const float *a, const float *b, const float *c,
                                float *out, size_t count, unsigned tier) {
    const OmegaNumericOpInfo *info = omega_numeric_op_at((unsigned)op);
    uint64_t flags = (uint64_t)!!a | ((uint64_t)!!b << 1) | ((uint64_t)!!c << 2) | ((uint64_t)!!out << 3) | ((uint64_t)omega_numeric_fpenv_ok() << 4);
    int rc = -(int)omega_numeric_rest(12, (unsigned)op, count, flags, info ? info->arity : 0, tier == 2);
    if (rc) return rc;
#if defined(__aarch64__)
    if (omega_numeric_rest(35, op, count, tier, 0, 0)) {
        rc = -(int)omega_numeric_rest(36, omega_numeric_cpu_has_bf16(), 0, 0, 0, 0);
        if (rc) return rc;
    }
#endif
    bool uses_c = omega_numeric_rest(33, op, 0, 0, 0, 0) != 0;
    bool uses_b = omega_numeric_rest(34, info->arity, 0, 0, 0, 0) != 0;
    for (size_t i = 0; i < count; ++i) {
        uint64_t action = omega_numeric_rest(17, op, i, 0, 0, 0), bits;
        if (tier == 2) {
            bits = omega_numeric_rest(16, op, omega_float_to_bits(a[i]), uses_b ? omega_float_to_bits(b[i]) : 0, uses_c ? omega_float_to_bits(c[i]) : 0, 0);
            if (bits > UINT32_MAX) return OMEGA_NUMERIC_ERR_OPERANDS;
        } else if (action == 1) {
            uint64_t tile[32];
            for (unsigned j = 0; j < 32; ++j) tile[j] = omega_float_to_bits(a[i+j]);
            if (omega_numeric_tile(&bits, 0, tile, 32) != OMEGA_OK) __builtin_trap();
        } else if (action == 2) {
            bits = 0;
        } else {
            size_t src = omega_numeric_rest(11, op, i, 0, 0, 0);
            bits = omega_numeric_rest(10, op, omega_float_to_bits(a[src]), uses_b ? omega_float_to_bits(b[i]) : 0, uses_c ? omega_float_to_bits(c[i]) : 0, tier);
        }
        out[i] = omega_bits_to_float((uint32_t)bits);
    }
    return OMEGA_NUMERIC_OK;
}
int omega_numeric_reference(OmegaNumericOp op, const float *a, const float *b, const float *c, float *out, size_t n) {
    return numeric_bulk_adapter(op, a, b, c, out, n, 0);
}
int omega_numeric_cpu_realize(OmegaNumericOp op, const float *a, const float *b, const float *c, float *out, size_t n) {
    return numeric_bulk_adapter(op, a, b, c, out, n, 1);
}
int omega_numeric_reference_ftz(OmegaNumericOp op, const float *a, const float *b, const float *c, float *out, size_t n) {
    return numeric_bulk_adapter(op, a, b, c, out, n, 2);
}

static void numeric_trace_copy(OmegaParityTrace *trace, const uint64_t state[16]) {
    trace->checked = state[0]; trace->mismatches = state[1]; trace->first_index = (long)state[2];
    trace->first_expect = (uint32_t)state[3]; trace->first_got = (uint32_t)state[4];
    trace->subnormal_expected = state[5]; trace->out_of_bound = state[6]; trace->bound_skipped = state[7];
}
static int numeric_trace_adapter(unsigned mode, OmegaNumericOp op, const float *ex, const float *got,
                                 size_t n, OmegaParityTrace *trace) {
    uint64_t state[16] = {0}, rc;
    const OmegaNumericOpInfo *info = omega_numeric_op_at((unsigned)op);
    state[9] = info ? info->compare : 0;
    state[11] = ex && got && trace;
    state[12] = omega_numeric_fpenv_ok();
    if (omega_numeric_trace(&rc, mode, (unsigned)op, 0, 0, state, 16) != OMEGA_OK) __builtin_trap();
    if (rc) {
        if (mode == 0 && info && ex && got && trace) numeric_trace_copy(trace, state);
        return -(int)rc;
    }
    for (size_t i = 0; i < n; ++i) {
        state[8] = i;
        if (omega_numeric_trace(&rc, mode + 2, op, omega_float_to_bits(ex[i]), omega_float_to_bits(got[i]), state, 16) != OMEGA_OK) __builtin_trap();
    }
    numeric_trace_copy(trace, state);
    return OMEGA_NUMERIC_OK;
}
int omega_numeric_parity(OmegaNumericOp op, const float *ex, const float *got, size_t n, OmegaParityTrace *trace) {
    return numeric_trace_adapter(0, op, ex, got, n, trace);
}
int omega_numeric_seed_bound(OmegaNumericOp op, const float *a, const float *got, size_t n, OmegaParityTrace *trace) {
    return numeric_trace_adapter(1, op, a, got, n, trace);
}
#endif
