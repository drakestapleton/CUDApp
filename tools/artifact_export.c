/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* artifact-export: offline handoff for one CUDApp-generated GPU realization.
 *
 *   artifact-export matmul    --m M --k K --n N --contract-id HEX --out PREFIX --commit HEX
 *   artifact-export attention --q-heads HQ --kv-heads HKV --contract-id HEX --out PREFIX --commit HEX
 *
 * Opens no device. Emits a cubin, zero-VA QMD templates, cbank bytes, a
 * canonical manifest and a JSON mirror. An artifact is immutable: export
 * refuses (I/O) when any PREFIX.* output already exists or is a symlink, and
 * refuses a PREFIX too long for every output path.
 * Exit: 0 ok, 2 usage, 3 refusal, 4 I/O. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "omega_blackwell_codegen.h"
#include "omega_blackwell_matmul.h"
#include "omega_blackwell_qmd.h"
#include "omega_gpu_attention_api.h"
#include "omega_gpu_matmul_api.h"
#include "omega_native_artifact.h"
#include "sha256.h"
#ifdef CUDAPP_MIGRATION_PENDING_C_TOOLS_ARTEXP
static void usage(void) {
    fprintf(stderr,
            "usage: artifact-export matmul --m M --k K --n N --contract-id HEX --out PREFIX --commit HEX\n"
            "       artifact-export attention --q-heads H --kv-heads H --contract-id HEX --out PREFIX --commit HEX\n");
}

static uint32_t num(const char *s) {
    char *e = NULL;
    unsigned long v = strtoul(s, &e, 0);
    return (e && *e == 0 && v <= 0xfffffffful) ? (uint32_t)v : 0;
}

static void hex32(const uint8_t b[32], char out[65]) {
    static const char h[] = "0123456789abcdef";
    for (int i = 0; i < 32; i++) { out[2 * i] = h[b[i] >> 4]; out[2 * i + 1] = h[b[i] & 15]; }
    out[64] = 0;
}

int main(int argc, char **argv) {
    const char *mode, *out = NULL, *contract = NULL, *commit = NULL;
    uint32_t m = 0, k = 0, n = 0, hq = 0, hkv = 0;
    if (argc < 2) { usage(); return 2; }
    mode = argv[1];
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) out = argv[++i];
        else if (strcmp(argv[i], "--contract-id") == 0 && i + 1 < argc) contract = argv[++i];
        else if (strcmp(argv[i], "--commit") == 0 && i + 1 < argc) commit = argv[++i];
        else if (strcmp(argv[i], "--m") == 0 && i + 1 < argc) m = num(argv[++i]);
        else if (strcmp(argv[i], "--k") == 0 && i + 1 < argc) k = num(argv[++i]);
        else if (strcmp(argv[i], "--n") == 0 && i + 1 < argc) n = num(argv[++i]);
        else if (strcmp(argv[i], "--q-heads") == 0 && i + 1 < argc) hq = num(argv[++i]);
        else if (strcmp(argv[i], "--kv-heads") == 0 && i + 1 < argc) hkv = num(argv[++i]);
        else { usage(); return 2; }
    }
    if (!out || !contract || strlen(contract) != 64) { usage(); return 2; }
    if (!commit || strlen(commit) != 40) { fprintf(stderr, "--commit must be a 40-hex source revision\n"); usage(); return 2; }
    for (const char *h = commit; *h; h++) {
        if (!((*h >= '0' && *h <= '9') || (*h >= 'a' && *h <= 'f'))) {
            fprintf(stderr, "--commit must be lowercase 40-hex\n"); return 2;
        }
    }
    /* The frozen v1 ids and bounds. An export under a frozen id but outside the
     * frozen range would let a consumer admit an artifact the contract forbids
     * (REVIEW C-11). */
    if (strcmp(mode, "matmul") == 0) {
        if (strcmp(contract, "647116fc1ba1e1394982fc8ba181ce565293f160a6b56afc0f02b2128a675704") != 0) {
            fprintf(stderr, "contract id is not matmul-bf16-v1\n"); return 3;
        }
        if (m > 8192u || k > 16384u || n > 8192u) {
            fprintf(stderr, "shape outside matmul-bf16-v1 bounds (m<=8192,k<=16384,n<=8192)\n"); return 3;
        }
    } else if (strcmp(mode, "attention") == 0) {
        if (strcmp(contract, "003696e422bba7274877ba9e01a61076ea0f862f32760cbaa06c60f85a115359") != 0) {
            fprintf(stderr, "contract id is not gqa-causal-v1\n"); return 3;
        }
        if (hq > 64u || hkv > 64u) {
            fprintf(stderr, "heads outside gqa-causal-v1 bounds (hq<=64,hkv<=64)\n"); return 3;
        }
    }

    OmegaNativeArtifact a;
    memset(&a, 0, sizeof a);
    OmegaBlackwellKernel kern;
    memset(&kern, 0, sizeof kern);
    uint32_t q0[OMEGA_ARTIFACT_QMD_WORDS], q1[OMEGA_ARTIFACT_QMD_WORDS];
    uint32_t cbank[32];
    OmegaCubinParam params[16];
    size_t nparams = 0;
    char spec_hex[65], *spec_hexp = spec_hex;
    memset(q0, 0, sizeof q0); memset(q1, 0, sizeof q1); memset(cbank, 0, sizeof cbank);
    OmegaBlackwellQmdConfig cfg;
    memset(&cfg, 0, sizeof cfg);

    if (strcmp(mode, "matmul") == 0) {
        OmegaMatMulSpec spec;
        memset(&spec, 0, sizeof spec);
        if (!m || !k || !n) { usage(); return 2; }
        /* Both kernels below tile C in 16 x 8 blocks and step K by 16; they have no edge handling. */
        if (m % 16u || n % 8u || k % 16u) {
            fprintf(stderr, "matmul-bf16-v1 export needs m%%16 == 0, n%%8 == 0, k%%16 == 0 (got %ux%ux%u)\n", m, k, n);
            return 3;
        }
        if (omega_matmul_spec_init(&spec, m, k, n, OMEGA_MATMUL_PRECISION_BF16) != 0) { fprintf(stderr, "spec refused\n"); return 3; }
        /* K = 16: the single-tile kernel (the G2 W1 artifact, evidence/MAX-INTEROP/G2). Its CTA (x, y) computes
         * exactly the 16 x 8 tile at row 16y, column 8x and has no column loop, so grid_x must be n / 8; the
         * exporter used to write grid_x = 1, which leaves columns 8..n-1 of C unwritten whenever n > 8.
         * K > 16: the looped kernel (omega-mv 5e29b826, FB-1 cut 1b, chip parity 18/18), which accumulates the
         * whole K on the chip; its CTA (x, y) owns row tile y and column tiles x, x + grid_x, ... The single-tile
         * kernel refuses K != 16, so without the looped kernel every export above K = 16 inside the contract's
         * k <= 16384 was refused. Both read row-major A (m x k bf16) and B (k x n bf16) and write row-major
         * C (m x n f32) through the same three pointer words. The looped grid_x follows the matmul API's CTA
         * budget (launch_shape). */
        uint32_t gx = n / 8u;
        if (k == 16u) {
            if (omega_blackwell_codegen_matmul(&spec, &kern) != 0) { fprintf(stderr, "codegen refused\n"); return 3; }
        } else {
            uint32_t budget = OMEGA_GPU_MATMUL_MAX_CTAS / (m / 16u);
            if (budget == 0) budget = 1;
            if (gx > budget) gx = budget;
            if (omega_blackwell_codegen_matmul_tensor_loop(&spec, gx, 0, &kern) != 0) { fprintf(stderr, "codegen refused\n"); return 3; }
        }
        hex32(spec.spec_id, spec_hex);
        cfg.threads_per_block = 32; cfg.threads_x = 32;
        cfg.grid_x = gx; cfg.grid_y = m / 16u;
        cfg.num_elements = m * n; cfg.shared_bytes = 1024;
        omega_blackwell_build_cbank_args_matmul(cbank, 0, 0, 0, m, k, n);
        params[0] = (OmegaCubinParam){ 0x380, 8, 1 };
        params[1] = (OmegaCubinParam){ 0x388, 8, 1 };
        params[2] = (OmegaCubinParam){ 0x390, 8, 1 };
        nparams = 3;
        a.op = "matmul"; a.contract_name = "matmul-bf16-v1";
        a.precision = "bf16"; a.accumulation = "fp32";
        a.tolerance = "abs<=1e-5*max(1,abs(ref))";
        a.grid_x = gx; a.grid_y = cfg.grid_y; a.block_x = 32; a.block_y = 1; a.block_z = 1;
        a.shared_bytes = 1024; a.cbank_words = 10;
    } else if (strcmp(mode, "attention") == 0) {
        if (!hq || !hkv || hq % hkv != 0) { usage(); return 2; }
        uint32_t ratio = hq / hkv, log2gqa = 0;
        while ((1u << log2gqa) < ratio) log2gqa++;
        if ((1u << log2gqa) != ratio) { fprintf(stderr, "GQA ratio is not a power of two\n"); return 3; }
        int rc = omega_gpu_attention_codegen_hd(true, 12, log2gqa, 128, &kern);
        if (rc != 0) { fprintf(stderr, "attention codegen refused: %d\n", rc); return 3; }
        {
            char specbuf[160];
            uint8_t d[32];
            int n = snprintf(specbuf, sizeof specbuf,
                             "omega-attention-spec/1\nhead_dim=128\nq_heads=%u\nkv_heads=%u\nlog2_gqa=%u\n",
                             hq, hkv, log2gqa);
            sha256_hash((const uint8_t *)specbuf, (size_t)n, d);
            for (int i = 0; i < 32; i++) sprintf(spec_hex + 2 * i, "%02x", d[i]);
            spec_hex[64] = 0;
        }
        (void)spec_hexp;
        uint32_t hpc = omega_gpu_attention_heads_per_cta(128, log2gqa);
        uint32_t shared = (3u * hpc + 1u) * 128u * 4u;
        cfg.threads_per_block = 128; cfg.threads_x = 128;
        cfg.grid_x = hq / hpc; cfg.grid_y = 1;
        cfg.num_elements = hq * 128u; cfg.shared_bytes = shared;
        params[0] = (OmegaCubinParam){ 0x380, 8, 1 };
        params[1] = (OmegaCubinParam){ 0x388, 8, 1 };
        params[2] = (OmegaCubinParam){ 0x390, 8, 1 };
        params[3] = (OmegaCubinParam){ 0x398, 8, 1 };
        for (int i = 0; i < 8; i++) params[4 + i] = (OmegaCubinParam){ (uint32_t)(0x3a0 + 4 * i), 4, 0 };
        nparams = 12;
        a.op = "gqa-causal-attention"; a.contract_name = "gqa-causal-v1";
        a.precision = "f32-kv"; a.accumulation = "fp32";
        a.tolerance = "rel<=1e-3,abs<=1e-4";
        a.grid_x = cfg.grid_x; a.grid_y = 1; a.block_x = 128; a.block_y = 1; a.block_z = 1;
        a.shared_bytes = shared; a.cbank_words = 16;
        /* Offline template: pointers and runtime strides are loader-supplied;
         * the query-head count is a contract constant, so bind it. */
        cbank[14] = hq;
    } else {
        usage(); return 2;
    }
    /* Declare peak + 2 rounded up to 8, as every launcher does. A cubin whose REGCOUNT is the allocator
     * peak loads but faults at run (GB10, G2 window 20261030T235858Z-10fa8c: E5 at peak 32 gave 715,
     * H1/H2 at 40 and H4 at 34 ran bit-exact; evidence/MAX-INTEROP/G2). */
    cfg.gpr_count = omega_bw_gpr_declared(kern.gpr_count);
    if (!omega_bw_gpr_declared_ok(cfg.gpr_count, kern.gpr_count)) { fprintf(stderr, "declared registers refused\n"); return 3; }
    if (omega_blackwell_build_qmd0(q0, 0, 0) != 0 || omega_blackwell_build_qmd1(q1, &cfg) != 0) {
        fprintf(stderr, "QMD template refused\n"); return 3;
    }
    static char shapebuf[128], boundsbuf[128];
    if (strcmp(mode, "matmul") == 0) {
        snprintf(shapebuf, sizeof shapebuf, "m=%u,k=%u,n=%u", m, k, n);
        snprintf(boundsbuf, sizeof boundsbuf, "m<=8192,k<=16384,n<=8192");
    } else {
        snprintf(shapebuf, sizeof shapebuf, "r>=1,base=0,hq=%u,hkv=%u,hd=128", hq, hkv);
        snprintf(boundsbuf, sizeof boundsbuf, "hq<=64,hkv<=64,hd=128,rows+base<=4096");
    }
    a.shape = shapebuf;
    a.bounds = boundsbuf;
    a.contract_id_hex = contract;
    a.semantic_spec_id_hex = spec_hex;
    a.kernel_function = (strcmp(mode, "matmul") == 0) ? "omega_matmul_bf16" : "omega_gqa_attention_causal";
    a.text = kern.code; a.text_len = kern.code_size;
    a.params = params; a.nparams = nparams;
    a.gpr_count = cfg.gpr_count;
    a.cbank_offset = 0x380;
    a.cbank_args = cbank;
    a.qmd0 = q0; a.qmd1 = q1;
    a.min_driver = "580.173.02";
    a.requires = "sm_121,nvidia-cubin-abi-v8";
    a.source_commit = commit;
    a.tool_version = "cudapp-artifact-export/1";
    a.qualification = "offline-disassembly=PASS;device-load=NOT_RUN;device-exec=NOT_RUN";

    char id[OMEGA_ARTIFACT_ID_MAX], err[256];
    int rc = omega_native_artifact_export(&a, out, id, err, sizeof err);
    if (rc != OMEGA_ARTIFACT_OK) {
        fprintf(stderr, "artifact-export refused: %s: %s\n", omega_native_artifact_rc_name(rc), err);
        omega_blackwell_kernel_free(&kern);
        return 3;
    }
    printf("artifact_id=%s\nop=%s\ncontract_id=%s\ncubin=%s.cubin\n", id, a.op, contract, out);
    omega_blackwell_kernel_free(&kern);
    return 0;
}
#else
/* Reading the command line, every check against the frozen v1 contracts, the kernel choice and its
 * grid, the launch configuration and parameter table, the attention semantic-spec text, the manifest
 * texts, every message and the exit status are om/tools/artexp.om. This shell calls the library when
 * the unit asks and writes the unit's text. The former C above keeps its line numbers. */
#include "cudapp_native.h"

static uint8_t tok[262144], txt[262144];

int main(int argc, char **argv) {
    const uint64_t f[8] = { OMEGA_GPU_MATMUL_MAX_CTAS, OMEGA_ARTIFACT_OK };
    uint64_t s[128];
    OmegaMatMulSpec spec;
    OmegaBlackwellKernel kern;
    OmegaBlackwellQmdConfig cfg;
    OmegaNativeArtifact a;
    uint32_t q0[OMEGA_ARTIFACT_QMD_WORDS], q1[OMEGA_ARTIFACT_QMD_WORDS], cbank[32];
    OmegaCubinParam params[16];
    memset(s, 0, sizeof s); memset(&spec, 0, sizeof spec); memset(&kern, 0, sizeof kern);
    memset(&cfg, 0, sizeof cfg); memset(&a, 0, sizeof a);
    memset(q0, 0, sizeof q0); memset(q1, 0, sizeof q1); memset(cbank, 0, sizeof cbank);
    memset(params, 0, sizeof params);
    s[3] = (uint64_t)argc;
    for (;;) {
        uint64_t act = 0;
        if (omega_artexp(&act, f, 8, s, 128, tok, sizeof tok, txt, sizeof txt) != 0) abort();
        switch (act & 255) {
        case 1: {
            /* Linux caps one argument at MAX_ARG_STRLEN (131072 bytes with its NUL), well inside tok. */
            const char *arg = argv[s[1]];
            size_t n = strnlen(arg, sizeof tok);
            if (n == sizeof tok) abort();
            memcpy(tok, arg, n + 1);
            s[2] = n;
            break;
        }
        case 2:
            fwrite(txt, 1, (size_t)(act >> 8), stderr);
            omega_blackwell_kernel_free(&kern);
            return (int)s[25];
        case 3:
            s[18] = (uint64_t)(int64_t)omega_matmul_spec_init(&spec, (uint32_t)s[13], (uint32_t)s[14], (uint32_t)s[15],
                                                              OMEGA_MATMUL_PRECISION_BF16);
            break;
        case 4: s[18] = (uint64_t)(int64_t)omega_blackwell_codegen_matmul(&spec, &kern); break;
        case 5:
            s[18] = (uint64_t)(int64_t)omega_blackwell_codegen_matmul_tensor_loop(&spec, (uint32_t)s[19], 0, &kern);
            break;
        case 6: memcpy(tok, spec.spec_id, 32); break;
        case 7:
            omega_blackwell_build_cbank_args_matmul(cbank, 0, 0, 0, (uint32_t)s[13], (uint32_t)s[14], (uint32_t)s[15]);
            break;
        case 8: s[18] = (uint64_t)(int64_t)omega_gpu_attention_codegen_hd(true, 12, (uint32_t)s[20], 128, &kern); break;
        case 9: sha256_hash(txt, (size_t)(act >> 8), tok); break;
        case 10: s[21] = omega_gpu_attention_heads_per_cta(128, (uint32_t)s[20]); break;
        case 11: cbank[14] = (uint32_t)s[16]; break;
        case 12:
            s[22] = kern.gpr_count;
            s[23] = omega_bw_gpr_declared(kern.gpr_count);
            s[24] = omega_bw_gpr_declared_ok((uint32_t)s[23], kern.gpr_count) != 0;
            break;
        case 13:
            cfg.threads_per_block = (uint32_t)s[32]; cfg.threads_x = (uint32_t)s[33];
            cfg.grid_x = (uint32_t)s[34]; cfg.grid_y = (uint32_t)s[35];
            cfg.num_elements = (uint32_t)s[36]; cfg.shared_bytes = (uint32_t)s[37];
            cfg.gpr_count = (uint32_t)s[38];
            s[18] = omega_blackwell_build_qmd0(q0, 0, 0) != 0 || omega_blackwell_build_qmd1(q1, &cfg) != 0;
            break;
        case 14: {
            const char *v[13], *c = (const char *)txt;
            for (int i = 0; i < 13; i++) { v[i] = c; c += strlen(c) + 1; }
            a.op = v[0]; a.contract_name = v[1]; a.precision = v[2]; a.accumulation = v[3]; a.tolerance = v[4];
            a.shape = v[5]; a.bounds = v[6]; a.semantic_spec_id_hex = v[7]; a.kernel_function = v[8];
            a.min_driver = v[9]; a.requires = v[10]; a.tool_version = v[11]; a.qualification = v[12];
            a.contract_id_hex = argv[s[7]];
            a.source_commit = argv[s[10]];
            for (uint64_t i = 0; i < s[47]; i++) {
                uint64_t w = s[64 + i];
                params[i] = (OmegaCubinParam){ (uint32_t)(w & 0xffff), (uint32_t)((w >> 16) & 0xffff), (uint8_t)(w >> 32) };
            }
            a.params = params; a.nparams = (size_t)s[47];
            a.text = kern.code; a.text_len = kern.code_size;
            a.gpr_count = cfg.gpr_count;
            a.grid_x = (uint32_t)s[40]; a.grid_y = (uint32_t)s[41];
            a.block_x = (uint32_t)s[42]; a.block_y = (uint32_t)s[43]; a.block_z = (uint32_t)s[44];
            a.shared_bytes = (uint32_t)s[45];
            a.cbank_offset = 0x380;
            a.cbank_args = cbank; a.cbank_words = (size_t)s[46];
            a.qmd0 = q0; a.qmd1 = q1;
            char id[OMEGA_ARTIFACT_ID_MAX], err[256];
            id[0] = 0; err[0] = 0;
            int rc = omega_native_artifact_export(&a, argv[s[6]], id, err, sizeof err);
            s[18] = (uint64_t)(int64_t)rc;
            const char *part[3] = { id, argv[s[7]], argv[s[6]] };
            if (rc != OMEGA_ARTIFACT_OK) { part[0] = omega_native_artifact_rc_name(rc); part[1] = err; }
            size_t at = 0;
            for (int i = 0; i < 3; i++) {
                size_t n = strlen(part[i]);
                if (at + n + 1 > sizeof tok) abort();
                memcpy(tok + at, part[i], n + 1);
                at += n + 1;
            }
            break;
        }
        case 15:
            fwrite(txt, 1, (size_t)(act >> 8), stdout);
            omega_blackwell_kernel_free(&kern);
            return 0;
        default:
            abort();
        }
    }
}
#endif
