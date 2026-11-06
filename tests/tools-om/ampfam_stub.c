/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Stand-ins for the 30 generators and the two encoders tools/ampere_families.c calls, linked into both
 * builds (the seeded Omega shell and the CUDAPP_MIGRATION_PENDING_C_TOOLS_AMPFAM oracle) by
 * tests/tools-om/ampfam.sh. Each call is appended to $AMPFAM_TRACE: the generator name and its
 * specification byte for byte (or its scalar arguments), the SM121 buffer size, and the SM86 target, size
 * and incoming length. $AMPFAM_CASE "K:KIND" changes the outcome at generator call K:
 *   g, g7    the generator returns 1, or -7
 *   r        the SM121 encoder returns 1
 *   a0keep   the SM121 encoder writes length 0 and the SM86 encoder leaves its length untouched
 *   rc1, rc0, rcm1, rcm2   the SM86 encoder returns 1, 0, -1, -2 (length as for a pass)
 *   bplus    the SM86 length is one more than a pass (a + 1 for an encode step, 1 for a refusal)
 *   bkeep    the SM86 encoder leaves its length untouched
 * Without a case every step passes: an encode step gets rc 0 and the SM121 length, a refusal rc -2 and
 * length 0. EXPECT is the source-derived plan of the 133 steps (E encode, R refuse); a wrong letter makes
 * the oracle fail its baseline, so the table is checked by every run. */
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char EXPECT[] =
    "EEEE" "RER" "EEEEEEEEEEEEEEEEE" "EERR" "EEEEEEEE" "RR" "RRRR"
    "EEEEEEEEEEEEEEEER" "EEEEEEEEEEEEEEEER"
    "EEERR" "REERR" "EEERR" "REERR" "EE"
    "EEEEEEEEEE" "EEEEEEEEEE" "EEEEEEEEEE" "EEEEE";
static unsigned call_no;
static size_t last_a;

static void trace(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#include <stdarg.h>
static void trace(const char *fmt, ...)
{
    const char *path = getenv("AMPFAM_TRACE");
    if (!path) return;
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) abort();
    va_list ap; va_start(ap, fmt); vdprintf(fd, fmt, ap); va_end(ap);
    close(fd);
}

static int case_is(const char *kind)
{
    const char *c = getenv("AMPFAM_CASE");
    char want[32];
    snprintf(want, sizeof(want), "%u:%s", call_no, kind);
    return c && strcmp(c, want) == 0;
}

static char expect(void)
{
    if (call_no == 0 || call_no > sizeof(EXPECT) - 1) { fprintf(stderr, "stub: call %u past the plan\n", call_no); abort(); }
    return EXPECT[call_no - 1];
}

static int generated(const char *name, const void *spec, size_t n)
{
    ++call_no;
    trace("gen %u %s", call_no, name);
    for (size_t i = 0; i < n; ++i) trace(" %02x", ((const unsigned char *)spec)[i]);
    trace("\n");
    if (case_is("g")) return 1;
    if (case_is("g7")) return -7;
    return 0;
}

#define SPEC(fn, type) int fn(const type *spec, BlackwellIRProgram *prog) { (void)prog; return generated(#fn, spec, sizeof(*spec)); }
SPEC(omega_blackwell_codegen_bf16_gemv_prog, OmegaBf16GemvSpec)
SPEC(omega_blackwell_codegen_decode_attn256_prog, OmegaDecodeAttn256Spec)
SPEC(omega_blackwell_codegen_decode_attn_prog, OmegaDecodeAttnSpec)
SPEC(omega_blackwell_codegen_dec_prog, OmegaDecSpec)
SPEC(omega_blackwell_codegen_fp8gemm_prog, OmegaFp8GemmSpec)
SPEC(omega_blackwell_codegen_fp8_gemv2_prog, OmegaFp8GemvSpec)
SPEC(omega_blackwell_codegen_fp8_gemv_prog, OmegaFp8GemvSpec)
SPEC(omega_blackwell_codegen_gdn_copy_prog, OmegaGdnSpec)
SPEC(omega_blackwell_codegen_gdn_multi_prog, OmegaGdnSpec)
SPEC(omega_blackwell_codegen_gdn_post_prog, OmegaGdnSpec)
SPEC(omega_blackwell_codegen_gdn_prep_prog, OmegaGdnSpec)
SPEC(omega_blackwell_codegen_gdn_prog, OmegaGdnSpec)
SPEC(omega_blackwell_codegen_gdn_scan_prog, OmegaGdnSpec)
SPEC(omega_blackwell_codegen_mc_prog, OmegaMcSpec)
SPEC(omega_blackwell_codegen_pq_prog, OmegaPqSpec)
SPEC(omega_blackwell_codegen_prefill_attn256_prog, OmegaPrefillAttn256Spec)
SPEC(omega_blackwell_codegen_prefill_attn_prog, OmegaPrefillAttnSpec)
SPEC(omega_blackwell_codegen_qgemm_prog, OmegaQgSpec)
SPEC(omega_blackwell_codegen_rl_prog, OmegaRlSpec)
SPEC(omega_blackwell_codegen_router_prog, OmegaRouterSpec)
SPEC(omega_blackwell_codegen_tc2_prog, OmegaTc2Spec)
SPEC(omega_blackwell_codegen_tcgemm_prog, OmegaTcGemmSpec)
SPEC(omega_blackwell_codegen_tern_gemv_prog, OmegaTernGemvSpec)

int omega_blackwell_codegen_pf_copy_prog(BlackwellIRProgram *prog) { (void)prog; return generated("pf_copy", "", 0); }
int omega_blackwell_codegen_f2e4_probe_prog(BlackwellIRProgram *prog) { (void)prog; return generated("f2e4_probe", "", 0); }
int omega_blackwell_codegen_tc_probe_prog(int mutant, BlackwellIRProgram *prog)
{
    (void)prog; return generated("tc_probe", &mutant, sizeof(mutant));
}
int omega_blackwell_codegen_qmma_probe_prog(int mutant, uint32_t wait_nops, BlackwellIRProgram *prog)
{
    (void)prog; int64_t v[2] = { mutant, wait_nops }; return generated("qmma_probe", v, sizeof(v));
}
int omega_blackwell_codegen_tc_bw_prog(uint32_t iters, uint32_t groups, uint32_t threads, BlackwellIRProgram *prog)
{
    (void)prog; uint32_t v[3] = { iters, groups, threads }; return generated("tc_bw", v, sizeof(v));
}
int omega_blackwell_codegen_tc_peak_prog(uint32_t iters, uint32_t chains, uint32_t stall, uint32_t threads, BlackwellIRProgram *prog)
{
    (void)prog; uint32_t v[4] = { iters, chains, stall, threads }; return generated("tc_peak", v, sizeof(v));
}
int omega_blackwell_codegen_qmma_peak_prog(uint32_t iters, uint32_t chains, uint32_t stall, uint32_t threads, BlackwellIRProgram *prog)
{
    (void)prog; uint32_t v[4] = { iters, chains, stall, threads }; return generated("qmma_peak", v, sizeof(v));
}

int omega_bw_encode_program(const BlackwellIRProgram *prog, uint8_t *code_buf, size_t max_len, size_t *out_len)
{
    (void)prog; (void)code_buf;
    trace("ref %u max %zu in %zu\n", call_no, max_len, *out_len);
    if (case_is("r")) return 1;
    last_a = case_is("a0keep") ? 0 : 100 + call_no;
    *out_len = last_a;
    return 0;
}

int omega_gpu_encode_program_target(const BlackwellIRProgram *prog, uint32_t target_sm, uint8_t *code_buf,
                                    size_t max_len, size_t *out_len)
{
    (void)prog; (void)code_buf;
    trace("tgt %u sm %u max %zu in %zu\n", call_no, target_sm, max_len, *out_len);
    int enc = expect() == 'E';
    int rc = enc ? 0 : -2;
    size_t b = enc ? last_a : 0;
    if (case_is("rc1")) rc = 1;
    if (case_is("rc0")) rc = 0;
    if (case_is("rcm1")) rc = -1;
    if (case_is("rcm2")) rc = -2;
    if (case_is("bplus")) b = enc ? last_a + 1 : 1;
    if (!case_is("bkeep") && !case_is("a0keep")) *out_len = b;
    return rc;
}
