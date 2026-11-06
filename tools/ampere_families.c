/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Producer coverage absent from the older SM121 corpus: head-dim256 attention,
 * multi-token variants, all GDN generators and ternary layouts/epilogues. */
#include "omega_ampere_codegen.h"
#include "omega_blackwell_codegen_attn256.h"
#include "omega_blackwell_codegen_pattn256.h"
#include "omega_blackwell_codegen_gdn.h"
#include "omega_blackwell_codegen_tern.h"
#include "omega_blackwell_codegen_attn.h"
#include "omega_blackwell_codegen_bf16.h"
#include "omega_blackwell_codegen_dec.h"
#include "omega_blackwell_codegen_gemm.h"
#include "omega_blackwell_codegen_moe2.h"
#include "omega_blackwell_codegen_pattn.h"
#include "omega_blackwell_codegen_pf.h"
#include "omega_blackwell_codegen_pfhost.h"
#include "omega_blackwell_codegen_qg.h"
#include "omega_blackwell_codegen_qmma.h"
#include "omega_blackwell_codegen_router.h"
#include "omega_blackwell_codegen_tc.h"
#include "omega_blackwell_codegen_tc2.h"
#include <stdio.h>
#ifdef CUDAPP_MIGRATION_PENDING_C_TOOLS_AMPFAM
static BlackwellIRProgram program;
static int encode(void)
{
    uint8_t reference[16384], ampere[16384]; size_t a = 0, b = 0;
    return omega_bw_encode_program(&program, reference, sizeof(reference), &a) ||
           omega_gpu_encode_program_target(&program, 86, ampere, sizeof(ampere), &b) || a != b;
}
#define EMIT(call) do { if ((call) || encode()) { fprintf(stderr, "Ampere family producer failed at line %d\n", __LINE__); return 1; } } while (0)

static int refused(void)
{
    uint8_t reference[16384], ampere[16384]; size_t a = 0, b = 1;
    return omega_bw_encode_program(&program, reference, sizeof(reference), &a) ||
           omega_gpu_encode_program_target(&program, 86, ampere, sizeof(ampere), &b) != -2 || b != 0;
}
#define REFUSE(call) do { if ((call) || refused()) { fprintf(stderr, "Ampere unsupported family accepted at line %d\n", __LINE__); return 1; } } while (0)
static int remaining_families(void)
{
    for (unsigned depth = 1; depth <= 4; ++depth) {
        OmegaDecodeAttnSpec s = { .depth = depth };
        EMIT(omega_blackwell_codegen_decode_attn_prog(&s, &program));
    }
    /* FUSED and ATTEND emit SHFL.BFLY and F2FP.BF16.F32.PACK_AB. APPEND returns
     * before that softmax, so SM86 still encodes it. */
    for (unsigned mode = OMEGA_PATTN_MODE_FUSED; mode <= OMEGA_PATTN_MODE_ATTEND; ++mode) {
        OmegaPrefillAttnSpec s = { .warps = 4, .mode = mode, .mirror = mode == OMEGA_PATTN_MODE_APPEND };
        if (mode == OMEGA_PATTN_MODE_APPEND) EMIT(omega_blackwell_codegen_prefill_attn_prog(&s, &program));
        else REFUSE(omega_blackwell_codegen_prefill_attn_prog(&s, &program));
    }
    OmegaBf16GemvSpec bf = { .k = 256, .rows = 1, .warps = 4, .depth = 1 };
    EMIT(omega_blackwell_codegen_bf16_gemv_prog(&bf, &program));
    bf.mask = 1; EMIT(omega_blackwell_codegen_bf16_gemv_prog(&bf, &program));
    bf.mask = 0; bf.ystride = 256; bf.xbatch = 2;
    EMIT(omega_blackwell_codegen_bf16_gemv_prog(&bf, &program));
    OmegaFp8GemvSpec mv = { .k = 256, .lane_bytes = 8, .rows = 1, .warps = 8 };
    EMIT(omega_blackwell_codegen_fp8_gemv_prog(&mv, &program));
    mv.depth = 1; EMIT(omega_blackwell_codegen_fp8_gemv_prog(&mv, &program));
    EMIT(omega_blackwell_codegen_fp8_gemv2_prog(&mv, &program));
    OmegaFp8GemmSpec gemm = { .k = 256, .lane_bytes = 8, .mt = 2, .warps = 8 };
    EMIT(omega_blackwell_codegen_fp8gemm_prog(&gemm, &program));
    gemm.rowmap = 1; EMIT(omega_blackwell_codegen_fp8gemm_prog(&gemm, &program));
    OmegaRouterSpec router = { .warps = 8, .depth = 1 };
    EMIT(omega_blackwell_codegen_router_prog(&router, &program));
    router.experts = 256; router.shared = 1;
    EMIT(omega_blackwell_codegen_router_prog(&router, &program));
    EMIT(omega_blackwell_codegen_pf_copy_prog(&program));
    OmegaRlSpec rl = { .k = 128, .e = 32, .t = 1 };
    EMIT(omega_blackwell_codegen_rl_prog(&rl, &program));
    OmegaMcSpec mc = { .h = 128, .top_k = 4 };
    EMIT(omega_blackwell_codegen_mc_prog(&mc, &program));
    EMIT(omega_blackwell_codegen_tc_probe_prog(0, &program));
    EMIT(omega_blackwell_codegen_tc_peak_prog(2, 2, 8, 128, &program));
    EMIT(omega_blackwell_codegen_tc_bw_prog(2, 1, 128, &program));
    OmegaTcGemmSpec tc = { .k = 128, .rt = 2, .ct = 2, .warps = 4 };
    EMIT(omega_blackwell_codegen_tcgemm_prog(&tc, &program));
    for (unsigned epi = OMEGA_TC2_EPI_F32; epi <= OMEGA_TC2_EPI_SWIGLU; ++epi)
        for (unsigned direct = 0; direct <= 1; ++direct) {
            OmegaTc2Spec s = { .k = 128, .rt = 2, .ct = 2, .warps = 4, .aqn = 2, .epi = epi, .direct = direct, .n = 256 };
            /* SwiGLU stores through F2FP.BF16.F32.PACK_AB. The f32 epilogue does not. */
            if (epi == OMEGA_TC2_EPI_F32) EMIT(omega_blackwell_codegen_tc2_prog(&s, &program));
            else REFUSE(omega_blackwell_codegen_tc2_prog(&s, &program));
        }
    for (unsigned kind = OMEGA_DEC_AXPY; kind <= OMEGA_DEC_XBOUND; ++kind) {
        OmegaDecSpec s = { .kind = (OmegaDecKind)kind };
        if (kind == OMEGA_DEC_AXPY) s.stride = 256;
        if (kind == OMEGA_DEC_RES) s.mode = OMEGA_DEC_RES_ADD | OMEGA_DEC_RES_NORM;
        if (kind == OMEGA_DEC_GATHER) { s.width = 256; s.stride = 256; }
        EMIT(omega_blackwell_codegen_dec_prog(&s, &program));
    }
    for (unsigned in_type = OMEGA_QG_IN_F32; in_type <= OMEGA_QG_IN_BF16; ++in_type) {
        OmegaPqSpec s = { .k = 128, .in_type = in_type };
        REFUSE(omega_blackwell_codegen_pq_prog(&s, &program));
    }
    OmegaQgSpec qg = { .k = 128, .rt = 4, .warps = 8, .n = 128 };
    REFUSE(omega_blackwell_codegen_qgemm_prog(&qg, &program));
    REFUSE(omega_blackwell_codegen_qmma_probe_prog(0, 8, &program));
    REFUSE(omega_blackwell_codegen_qmma_peak_prog(2, 2, 8, 128, &program));
    REFUSE(omega_blackwell_codegen_f2e4_probe_prog(&program));
    return 0;
}
int main(void)
{
    if (remaining_families()) return 1;
    for (unsigned shape = 0; shape < 2; ++shape) {
        for (unsigned depth = 1; depth <= 4; ++depth) {
            for (unsigned prep = 0; prep < 2; ++prep) for (unsigned multi = 0; multi < 2; ++multi) {
                OmegaDecodeAttn256Spec s = { .depth = depth, .prep = prep, .multi = multi };
                if (shape) { s.q_heads = 16; s.kv_heads = 2; s.q_per_cta = 4; }
                EMIT(omega_blackwell_codegen_decode_attn256_prog(&s, &program));
            }
        }
        OmegaPrefillAttn256Spec s = {0};
        if (shape) { s.q_heads = 16; s.kv_heads = 2; }
        REFUSE(omega_blackwell_codegen_prefill_attn256_prog(&s, &program));
    }
    for (unsigned ratio = 2; ratio <= 3; ++ratio) for (unsigned bf16 = 0; bf16 < 2; ++bf16) {
        OmegaGdnSpec s = { .ratio = ratio, .state_bf16 = bf16 };
        /* bf16 state stores F2FP.BF16.F32.PACK_AB. f32 state stays on SM86. */
        if (bf16) REFUSE(omega_blackwell_codegen_gdn_prog(&s, &program));
        else EMIT(omega_blackwell_codegen_gdn_prog(&s, &program));
        EMIT(omega_blackwell_codegen_gdn_multi_prog(&s, &program));
        EMIT(omega_blackwell_codegen_gdn_prep_prog(&s, &program));
        /* The scan and the post epilogue reduce with SHFL.BFLY. */
        REFUSE(omega_blackwell_codegen_gdn_scan_prog(&s, &program));
        REFUSE(omega_blackwell_codegen_gdn_post_prog(&s, &program));
    }
    for (unsigned kind = OMEGA_GDN_COPY; kind <= OMEGA_GDN_ZERO; ++kind) {
        OmegaGdnSpec s = { .copy = (int)kind };
        EMIT(omega_blackwell_codegen_gdn_copy_prog(&s, &program));
    }
    const unsigned shapes[][2] = { { OMEGA_TERN_LAYOUT_ROWMAJOR, 1 }, { OMEGA_TERN_LAYOUT_ROWMAJOR, 2 },
        { OMEGA_TERN_LAYOUT_ROWMAJOR, 4 }, { OMEGA_TERN_LAYOUT_BLOCK, 2 }, { OMEGA_TERN_LAYOUT_BLOCK, 4 } };
    const unsigned ks[] = { 256, 2560, 8192, OMEGA_TERN_MAX_K };
    for (unsigned k = 0; k < sizeof(ks)/sizeof(ks[0]); ++k)
        for (unsigned shape = 0; shape < sizeof(shapes)/sizeof(shapes[0]); ++shape)
            for (unsigned epi = 0; epi < 2; ++epi) {
                /* The existing generator supports the multi-chunk epilogue
                 * only for integer output, independently of GPU target. */
                if (ks[k] > OMEGA_TERN_CHUNK_MAX && epi == OMEGA_TERN_EPI_F32) continue;
                OmegaTernGemvSpec s = { ks[k], shapes[shape][0], shapes[shape][1], 4, 1, epi, 0 };
                EMIT(omega_blackwell_codegen_tern_gemv_prog(&s, &program));
            }
    puts("Ampere extended existing generator families encoded (host only)");
    return 0;
}
#else
/* The step order, every specification, which steps must encode and which must be refused, the
 * short-circuit of a failing step and the message (with the source line of the former C above) are
 * om/tools/ampfam.om. This shell only calls what the unit asks for: the generator with the fields it
 * gives, the SM121 encoder or the SM86 encoder, and prints the unit's text. The former C keeps its line
 * numbers, so the messages name the same lines in both builds. */
#include "cudapp_native.h"
#include <stdlib.h>

static BlackwellIRProgram program;
static int generate(const uint64_t *v, uint64_t id)
{
#define U(i) ((uint32_t)v[i])
    switch (id) {
    case 1: { OmegaDecodeAttnSpec s = { .depth = U(0) }; return omega_blackwell_codegen_decode_attn_prog(&s, &program); }
    case 2: { OmegaPrefillAttnSpec s = { .warps = U(0), .mode = U(1), .mirror = U(2) };
              return omega_blackwell_codegen_prefill_attn_prog(&s, &program); }
    case 3: { OmegaBf16GemvSpec s = { .k = U(0), .rows = U(1), .warps = U(2), .depth = U(3), .mask = U(4),
                                      .ystride = U(5), .xbatch = U(6) };
              return omega_blackwell_codegen_bf16_gemv_prog(&s, &program); }
    case 4: case 5: { OmegaFp8GemvSpec s = { .k = U(0), .lane_bytes = U(1), .rows = U(2), .warps = U(3), .depth = U(4) };
              return id == 4 ? omega_blackwell_codegen_fp8_gemv_prog(&s, &program) : omega_blackwell_codegen_fp8_gemv2_prog(&s, &program); }
    case 6: { OmegaFp8GemmSpec s = { .k = U(0), .lane_bytes = U(1), .mt = U(2), .warps = U(3), .rowmap = U(4) };
              return omega_blackwell_codegen_fp8gemm_prog(&s, &program); }
    case 7: { OmegaRouterSpec s = { .warps = U(0), .depth = U(1), .experts = U(2), .shared = U(3) };
              return omega_blackwell_codegen_router_prog(&s, &program); }
    case 8: return omega_blackwell_codegen_pf_copy_prog(&program);
    case 9: { OmegaRlSpec s = { .k = U(0), .e = U(1), .t = U(2) }; return omega_blackwell_codegen_rl_prog(&s, &program); }
    case 10: { OmegaMcSpec s = { .h = U(0), .top_k = U(1) }; return omega_blackwell_codegen_mc_prog(&s, &program); }
    case 11: return omega_blackwell_codegen_tc_probe_prog((int)v[0], &program);
    case 12: return omega_blackwell_codegen_tc_peak_prog(U(0), U(1), U(2), U(3), &program);
    case 13: return omega_blackwell_codegen_tc_bw_prog(U(0), U(1), U(2), &program);
    case 14: { OmegaTcGemmSpec s = { .k = U(0), .rt = U(1), .ct = U(2), .warps = U(3) };
               return omega_blackwell_codegen_tcgemm_prog(&s, &program); }
    case 15: { OmegaTc2Spec s = { .k = U(0), .rt = U(1), .ct = U(2), .warps = U(3), .aqn = U(4), .epi = U(5),
                                  .direct = U(6), .n = U(7) };
               return omega_blackwell_codegen_tc2_prog(&s, &program); }
    case 16: { OmegaDecSpec s = { .kind = (OmegaDecKind)v[0], .stride = U(1), .mode = U(2), .width = U(3) };
               return omega_blackwell_codegen_dec_prog(&s, &program); }
    case 17: { OmegaPqSpec s = { .k = U(0), .in_type = U(1) }; return omega_blackwell_codegen_pq_prog(&s, &program); }
    case 18: { OmegaQgSpec s = { .k = U(0), .rt = U(1), .warps = U(2), .n = U(3) };
               return omega_blackwell_codegen_qgemm_prog(&s, &program); }
    case 19: return omega_blackwell_codegen_qmma_probe_prog((int)v[0], U(1), &program);
    case 20: return omega_blackwell_codegen_qmma_peak_prog(U(0), U(1), U(2), U(3), &program);
    case 21: return omega_blackwell_codegen_f2e4_probe_prog(&program);
    case 22: { OmegaDecodeAttn256Spec s = { .depth = U(0), .prep = U(1), .multi = U(2), .q_heads = U(3),
                                            .kv_heads = U(4), .q_per_cta = U(5) };
               return omega_blackwell_codegen_decode_attn256_prog(&s, &program); }
    case 23: { OmegaPrefillAttn256Spec s = { .q_heads = U(0), .kv_heads = U(1) };
               return omega_blackwell_codegen_prefill_attn256_prog(&s, &program); }
    case 24: case 25: case 26: case 27: case 28: {
        OmegaGdnSpec s = { .ratio = U(0), .state_bf16 = U(1) };
        return id == 24 ? omega_blackwell_codegen_gdn_prog(&s, &program)
             : id == 25 ? omega_blackwell_codegen_gdn_multi_prog(&s, &program)
             : id == 26 ? omega_blackwell_codegen_gdn_prep_prog(&s, &program)
             : id == 27 ? omega_blackwell_codegen_gdn_scan_prog(&s, &program)
             : omega_blackwell_codegen_gdn_post_prog(&s, &program); }
    case 29: { OmegaGdnSpec s = { .copy = (int)v[0] }; return omega_blackwell_codegen_gdn_copy_prog(&s, &program); }
    case 30: { OmegaTernGemvSpec s = { U(0), U(1), U(2), U(3), U(4), U(5), 0 };
               return omega_blackwell_codegen_tern_gemv_prog(&s, &program); }
    }
#undef U
    abort();
}

int main(void)
{
    const uint64_t facts[32] = { OMEGA_PATTN_MODE_FUSED, OMEGA_PATTN_MODE_ATTEND, OMEGA_PATTN_MODE_APPEND,
        OMEGA_TC2_EPI_F32, OMEGA_TC2_EPI_SWIGLU, OMEGA_DEC_AXPY, OMEGA_DEC_XBOUND, OMEGA_DEC_RES, OMEGA_DEC_GATHER,
        OMEGA_DEC_RES_ADD | OMEGA_DEC_RES_NORM, OMEGA_QG_IN_F32, OMEGA_QG_IN_BF16, OMEGA_GDN_COPY, OMEGA_GDN_ZERO,
        OMEGA_TERN_LAYOUT_ROWMAJOR, OMEGA_TERN_LAYOUT_BLOCK, OMEGA_TERN_MAX_K, OMEGA_TERN_CHUNK_MAX, OMEGA_TERN_EPI_F32 };
    uint64_t s[64] = {0}, act = 0;
    uint8_t text[128], reference[16384], ampere[16384];
    for (;;) {
        if (omega_ampfam(&act, facts, 32, s, 64, text, sizeof(text))) abort();
        if ((act & 255) == 1) s[2] = (uint64_t)(int64_t)generate(s + 16, s[9]);
        else if (act == 2) {
            size_t a = 0;
            s[2] = (uint64_t)(int64_t)omega_bw_encode_program(&program, reference, sizeof(reference), &a);
            s[3] = a;
        } else if (act == 3) {
            size_t b = s[10];
            s[2] = (uint64_t)(int64_t)omega_gpu_encode_program_target(&program, 86, ampere, sizeof(ampere), &b);
            s[4] = b;
        } else if ((act & 255) == 4 || (act & 255) == 5) {
            FILE *to = (act & 255) == 4 ? stderr : stdout;
            fwrite(text, 1, act >> 8, to);
            return (act & 255) == 4;
        } else abort();
    }
}
#endif
