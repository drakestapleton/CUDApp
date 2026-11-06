/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Test-only IR capture. Linked into the existing host-only corpus producers;
 * no driver API is called here. A refusal is legal for an explicit FP8 marker,
 * or for a training R5 opcode SM86 does not implement (op > BW_IR_MUFU_LG2).
 *
 * The decisions are seeded Omega (om/tools/ampcorp.om through src/cudapp_om_ampcorp.c): the folder
 * check, the IR metadata, the refusal classification, the check of the encoder outcome and the text of
 * the paths and of the message. This file keeps the I/O: getenv, the SM86 encoder call, open/fdopen/
 * fwrite, the serial counter and abort. The former C is the oracle behind the default-OFF switch
 * CUDAPP_MIGRATION_PENDING_C_TOOLS_AMPCORP (tests/tools-om/ampcorp.sh). */
#include "omega_ampere_codegen.h"
#include "omega_blackwell_codegen_tc.h"
#include "omega_blackwell_codegen_qmma.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef CUDAPP_MIGRATION_PENDING_C_TOOLS_AMPCORP
static int is_safe_corpus_dir(const char *dir)
{
    if (!dir || !*dir) return 0;
    if (strchr(dir, '\\')) return 0;
    for (const char *component = dir; *component;) {
        while (*component == '/') ++component;
        const char *start = component;
        while (*component && *component != '/') ++component;
        if ((size_t)(component - start) == 2 &&
            start[0] == '.' && start[1] == '.') return 0;
    }
    return 1;
}

static void save(const char *dir, unsigned serial, const char *suffix, const void *data, size_t size)
{
    char path[4096];
    int n = snprintf(path, sizeof(path), "%s/%ld-%u.%s", dir, (long)getpid(), serial, suffix);
    if (n < 0 || (size_t)n >= sizeof(path)) abort();
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) { perror(path); abort(); }
    FILE *f = fdopen(fd, "wb");
    if (!f) { perror(path); close(fd); abort(); }
    if (fwrite(data, 1, size, f) != size || fclose(f) != 0) abort();
}

void omega_ampere_corpus_capture(const BlackwellIRProgram *prog, const uint8_t *sm121, size_t len)
{
    const char *dir = getenv("OMEGA_AMPERE_CORPUS_DIR");
    if (!is_safe_corpus_dir(dir)) return;
    static unsigned serial;
    const unsigned id = serial++;
    uint8_t code[16384];
    uint32_t metadata[BW_MAX_IR_INSNS][2];
    int fp8 = 0, blackwell_only = 0;
    for (size_t i = 0; i < prog->count; ++i) {
        const BlackwellIRInsn *in = &prog->insns[i];
        metadata[i][0] = (uint32_t)in->op;
        metadata[i][1] = in->imm;
        if (in->op > BW_IR_MUFU_LG2) blackwell_only = 1;
        if ((in->op == BW_IR_HMMA_BF16 && in->imm == OMEGA_QMMA_MARK) ||
            (in->op == BW_IR_FMNMX_MAX && in->imm == OMEGA_QMMA_MARK_F2E4)) fp8 = 1;
    }
    memset(code, 0xa5, sizeof(code));
    size_t bytes = 0;
    int rc = omega_gpu_encode_program_target(prog, 86, code, sizeof(code), &bytes);
    /* SM121 already holds the published bytes. SM86 refuses the new opcodes in
     * its preflight and writes nothing. There is no post-encode patch. */
    if (fp8 || blackwell_only) {
        if (rc != OMEGA_GPU_ENCODE_UNSUPPORTED || bytes != 0) abort();
        for (size_t i = 0; i < sizeof(code); ++i) if (code[i] != 0xa5) abort();
        save(dir, id, "refused", metadata, prog->count * sizeof(metadata[0]));
        return;
    }
    if (rc != 0 || bytes != len) {
        fprintf(stderr, "SM86 unexpectedly refused an existing kernel (%zu instructions, rc %d)\n", prog->count, rc);
        abort();
    }
    save(dir, id, "sm121.bin", sm121, len);
    save(dir, id, "sm86.bin", code, bytes);
    save(dir, id, "ir", metadata, prog->count * sizeof(metadata[0]));
}
#else
#include "cudapp_native.h"

/* The unit's arrays (sizes fixed by om/tools/ampcorp.om). The folder buffer holds any environment
 * string: Linux caps one at MAX_ARG_STRLEN (131072 bytes) with its name and zero. Not reentrant, as
 * the former C's serial counter already was not. */
static uint64_t g_facts[16];
static uint8_t g_dir[131072];
static uint64_t g_ins[2048];
static uint8_t g_code[16384];
static uint8_t g_out[20480];

static uint64_t decide(void)
{
    uint64_t r = 0;
    if (omega_ampcorp(&r, g_facts, 16, g_dir, sizeof(g_dir), g_ins, 2048, g_code, sizeof(g_code),
                      g_out, sizeof(g_out)) != OMEGA_OK) abort();
    return r;
}

static void save(const char *path, const void *data, size_t size)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) { perror(path); abort(); }
    FILE *f = fdopen(fd, "wb");
    if (!f) { perror(path); close(fd); abort(); }
    if (fwrite(data, 1, size, f) != size || fclose(f) != 0) abort();
}

void omega_ampere_corpus_capture(const BlackwellIRProgram *prog, const uint8_t *sm121, size_t len)
{
    const char *dir = getenv("OMEGA_AMPERE_CORPUS_DIR");
    if (!dir) return;
    size_t dn = strlen(dir);
    if (dn >= sizeof(g_dir) || prog->count > BW_MAX_IR_INSNS) abort();
    memcpy(g_dir, dir, dn);
    memset(g_facts, 0, sizeof(g_facts));
    g_facts[1] = dn;
    if (!decide()) return;
    static unsigned serial;
    const unsigned id = serial++;
    for (size_t i = 0; i < prog->count; ++i) {
        g_ins[2 * i] = (uint32_t)prog->insns[i].op;
        g_ins[2 * i + 1] = prog->insns[i].imm;
    }
    memset(g_code, 0xa5, sizeof(g_code));
    size_t bytes = 0;
    int rc = omega_gpu_encode_program_target(prog, 86, g_code, sizeof(g_code), &bytes);
    const uint64_t facts[16] = { 1, dn, prog->count, (uint64_t)(int64_t)rc, bytes, len, (uint64_t)(long)getpid(), id,
                                 BW_IR_MUFU_LG2, BW_IR_HMMA_BF16, OMEGA_QMMA_MARK, BW_IR_FMNMX_MAX,
                                 OMEGA_QMMA_MARK_F2E4, (uint64_t)(int64_t)OMEGA_GPU_ENCODE_UNSUPPORTED };
    memcpy(g_facts, facts, sizeof(facts));
    uint64_t r = decide();
    const char *path = (const char *)g_out + 8192;
    switch (r & 255) {
    case 1:
        save(path, g_out, prog->count * 8);
        return;
    case 2:
        save(path, sm121, len);
        save((const char *)g_out + 12288, g_code, bytes);
        save((const char *)g_out + 16384, g_out, prog->count * 8);
        return;
    case 4:
        fwrite(path, 1, (size_t)(r >> 8), stderr);
        abort();
    default: /* 3: the refusal wrote bytes or did not refuse; 5: the path does not fit 4096 bytes */
        abort();
    }
}
#endif
