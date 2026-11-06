/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* ampcorp_diff: drives omega_ampere_corpus_capture (tools/ampere_corpus.c, either build) through the
 * partition of tests/tools-om/README.md. This file defines omega_gpu_encode_program_target, so the
 * encoder outcome (rc, bytes, stray writes into the buffer) is chosen per case. Each case runs in a child
 * in a fresh folder; the report holds the child's exit or signal, its stderr and every file it wrote
 * (name, size, FNV-1a 64; a link is listed by name), with the child's pid replaced by "PID". The two builds must report the same. */
#include "omega_ampere_codegen.h"
#include "omega_blackwell_codegen_qmma.h"
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

void omega_ampere_corpus_capture(const BlackwellIRProgram *prog, const uint8_t *sm121, size_t len);

static struct { int rc; size_t bytes; long poke; size_t fill; } enc;
static long calls;

int omega_gpu_encode_program_target(const BlackwellIRProgram *prog, uint32_t target_sm, uint8_t *code_buf,
                                    size_t max_len, size_t *out_len)
{
    (void)prog;
    calls++;
    if (target_sm != 86 || max_len != 16384) abort();
    for (size_t i = 0; i < enc.fill && i < max_len; i++) code_buf[i] = (uint8_t)(i * 7 + 3);
    if (enc.poke >= 0) code_buf[enc.poke] ^= 1;
    *out_len = enc.bytes;
    return enc.rc;
}

static BlackwellIRProgram prog;
static uint8_t sm121[16384];

typedef struct {
    const char *dir; /* NULL: unset; "@N": a folder name making "<dir>/<pid>-0.sm121.bin" N bytes long */
    int kind;        /* program: see build() */
    size_t count;
    int rc;
    size_t bytes, len, fill;
    long poke;
    int repeat;      /* captures in a row (serial 0..repeat-1); negative: safe, unsafe, safe */
} Case;

static void build(int kind, size_t count)
{
    memset(&prog, 0, sizeof(prog));
    prog.count = count;
    for (size_t i = 0; i < count; i++) {
        prog.insns[i].op = (BlackwellIROpcode)(i % (BW_IR_MUFU_LG2 + 1));
        prog.insns[i].imm = (uint32_t)(i * 2654435761u);
    }
    if (!count) return;
    BlackwellIRInsn *last = &prog.insns[count - 1];
    switch (kind) {
    case 1: last->op = BW_IR_MUFU_LG2; break;
    case 2: last->op = BW_IR_SHFL_BFLY; break;
    case 3: last->op = BW_IR_ATOMG_ADD_F32_STRONG_GPU; last->imm = 0xffffffffu; break;
    case 4: last->op = BW_IR_HMMA_BF16; last->imm = OMEGA_QMMA_MARK; break;
    case 5: last->op = BW_IR_HMMA_BF16; last->imm = OMEGA_QMMA_MARK ^ 1u; break;
    case 6: last->op = BW_IR_HMMA_BF16; last->imm = OMEGA_QMMA_MARK_F2E4; break;
    case 7: last->op = BW_IR_FMNMX_MAX; last->imm = OMEGA_QMMA_MARK_F2E4; break;
    case 8: last->op = BW_IR_FMNMX_MAX; last->imm = OMEGA_QMMA_MARK; break;
    case 9: last->op = BW_IR_FMNMX_MAX; last->imm = OMEGA_QMMA_MARK_F2E4 + 1u; break;
    case 10: prog.insns[0].op = BW_IR_HMMA_BF16; prog.insns[0].imm = OMEGA_QMMA_MARK; break;
    default: break;
    }
}

/* -DAMPCORP_GCOV (coverage build only): a child ends in _exit or abort, neither of which writes gcov data,
 * so dump it explicitly on both paths. */
#ifdef AMPCORP_GCOV
extern void __gcov_dump(void);
static void on_abort(int sig) { __gcov_dump(); signal(sig, SIG_DFL); raise(sig); }
#define GCOV_START() signal(SIGABRT, on_abort)
#define GCOV_DUMP() __gcov_dump()
#else
#define GCOV_START() ((void)0)
#define GCOV_DUMP() ((void)0)
#endif

static unsigned ndig(unsigned long v) { unsigned n = 1; while (v /= 10) n++; return n; }

static void child(const Case *c)
{
    char dir[8192];
    GCOV_START();
    if (c->dir && c->dir[0] == '@') {
        /* "<dir>/<pid>-0.sm121.bin": dir length = N - pid digits - 13 */
        long n = atol(c->dir + 1) - (long)ndig((unsigned long)getpid()) - 13;
        memset(dir, 'x', (size_t)n);
        dir[n] = 0;
        for (long i = 200; i < n; i += 201) dir[i] = '/';
        setenv("OMEGA_AMPERE_CORPUS_DIR", dir, 1);
    } else if (c->dir && c->dir[0] == '!') {
        /* "!SUFFIX": folder d, with "d/<pid>-0.SUFFIX" a link to /dev/full, so the write fails (ENOSPC) */
        snprintf(dir, sizeof dir, "d/%ld-0.%s", (long)getpid(), c->dir + 1);
        if (symlink("/dev/full", dir)) _exit(9);
        setenv("OMEGA_AMPERE_CORPUS_DIR", "d", 1);
    } else if (c->dir) {
        setenv("OMEGA_AMPERE_CORPUS_DIR", c->dir, 1);
    } else {
        unsetenv("OMEGA_AMPERE_CORPUS_DIR");
    }
    build(c->kind, c->count);
    enc.rc = c->rc; enc.bytes = c->bytes; enc.fill = c->fill; enc.poke = c->poke;
    int reps = c->repeat < 0 ? 3 : c->repeat;
    const char *safe = getenv("OMEGA_AMPERE_CORPUS_DIR");
    char keep[8192];
    snprintf(keep, sizeof keep, "%s", safe ? safe : "");
    for (int r = 0; r < reps; r++) {
        if (c->repeat < 0) setenv("OMEGA_AMPERE_CORPUS_DIR", r == 1 ? "d/../d" : keep, 1);
        omega_ampere_corpus_capture(&prog, sm121, c->len);
    }
    printf("  encoder calls %ld\n", calls);
    fflush(stdout);
    GCOV_DUMP();
    _exit(0);
}

static uint64_t fnv(const unsigned char *p, size_t n)
{
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; i++) h = (h ^ p[i]) * 1099511628211ull;
    return h;
}

/* Print s with every occurrence of the decimal pid replaced by PID. */
static void norm(FILE *o, const char *s, pid_t pid)
{
    char ps[32];
    int pl = snprintf(ps, sizeof ps, "%ld", (long)pid);
    while (*s) {
        if (!strncmp(s, ps, (size_t)pl)) { fputs("PID", o); s += pl; } else fputc(*s++, o);
    }
}

static int cmpstr(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }

static void listdir(const char *path, pid_t pid)
{
    DIR *d = opendir(path);
    if (!d) return;
    char *names[64];
    int n = 0;
    for (struct dirent *e; (e = readdir(d)) && n < 64;)
        if (e->d_name[0] != '.' && e->d_type != DT_DIR) names[n++] = strdup(e->d_name);
    closedir(d);
    qsort(names, (size_t)n, sizeof names[0], cmpstr);
    for (int i = 0; i < n; i++) {
        char full[4096];
        snprintf(full, sizeof full, "%s/%s", path, names[i]);
        struct stat lst;
        int entry = open(full, O_PATH | O_NOFOLLOW | O_CLOEXEC);
        int is_link = entry >= 0 && fstat(entry, &lst) == 0 && S_ISLNK(lst.st_mode);
        if (entry >= 0) close(entry);
        if (is_link) {
            printf("  link ");
            norm(stdout, names[i], pid);
            printf("\n");
            unlink(full);
            free(names[i]);
            continue;
        }
        FILE *f = fopen(full, "rb");
        static unsigned char buf[1 << 16];
        size_t sz = f ? fread(buf, 1, sizeof buf, f) : 0;
        if (f) fclose(f);
        printf("  file ");
        norm(stdout, names[i], pid);
        printf(" %zu %016llx\n", sz, (unsigned long long)fnv(buf, sz));
        unlink(full);
        free(names[i]);
    }
}

static const Case cases[] = {
    /* the folder */
    { NULL, 0, 4, 0, 32, 32, 32, -1, 1 },
    { "", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "./d", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d/", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d//", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "e/../d", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "../d", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d/..", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d/../", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "..", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "//..//", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d/...", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d/.x", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d/x.", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d/.", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d/a..", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d/..a", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d\\e", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "\\", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "d/\\..", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "nofolder", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "@4095", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "@4096", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "@4097", 0, 4, 0, 32, 32, 32, -1, 1 },
    { "@4097", 2, 4, -2, 0, 32, 0, -1, 1 },
    { "@4098", 2, 4, -2, 0, 32, 0, -1, 1 },
    /* the serial */
    { "d", 0, 4, 0, 32, 32, 32, -1, 3 },
    { "d", 0, 4, 0, 32, 32, 32, -1, -1 },
    { "d", 0, 4, 0, 32, 32, 32, -1, 12 },
    /* the program and the encoder outcome */
    { "d", 0, 0, 0, 0, 0, 0, -1, 1 },
    { "d", 0, 0, 0, 16, 0, 16, -1, 1 },
    { "d", 0, 1024, 0, 16384, 16384, 16384, -1, 1 },
    { "d", 1, 9, 0, 144, 144, 144, -1, 1 },
    { "d", 2, 9, -2, 0, 144, 0, -1, 1 },
    { "d", 3, 9, -2, 0, 144, 0, -1, 1 },
    { "d", 4, 9, -2, 0, 144, 0, -1, 1 },
    { "d", 5, 9, 0, 144, 144, 144, -1, 1 },
    { "d", 6, 9, 0, 144, 144, 144, -1, 1 },
    { "d", 7, 9, -2, 0, 144, 0, -1, 1 },
    { "d", 8, 9, 0, 144, 144, 144, -1, 1 },
    { "d", 9, 9, 0, 144, 144, 144, -1, 1 },
    { "d", 10, 9, -2, 0, 144, 0, -1, 1 },
    { "d", 10, 1024, -2, 0, 144, 0, -1, 1 },
    /* a refusal that is not clean */
    { "d", 4, 9, -2, 0, 144, 0, 0, 1 },
    { "d", 4, 9, -2, 0, 144, 0, 16383, 1 },
    { "d", 4, 9, -2, 0, 144, 0, 8191, 1 },
    { "d", 4, 9, -2, 1, 144, 0, -1, 1 },
    { "d", 4, 9, -1, 0, 144, 0, -1, 1 },
    { "d", 4, 9, 0, 0, 144, 0, -1, 1 },
    { "d", 2, 9, 0, 144, 144, 144, -1, 1 },
    /* an existing kernel the SM86 encoder does not take */
    { "d", 0, 9, -2, 0, 144, 0, -1, 1 },
    { "d", 0, 9, 0, 143, 144, 143, -1, 1 },
    { "d", 0, 9, 0, 145, 144, 145, -1, 1 },
    { "d", 0, 9, 7, 144, 144, 144, -1, 1 },
    { "d", 0, 9, -2147483647 - 1, 144, 144, 144, -1, 1 },
    { "d", 0, 9, 2147483647, 144, 144, 144, -1, 1 },
    { "d", 0, 1024, -1, 16384, 16384, 16384, -1, 1 },
    { "d", 0, 9, 0, 0, 0, 0, -1, 1 },
    /* a write that fails: in fclose (buffered, short data) and in fwrite (longer than the stdio buffer) */
    { "!refused", 4, 9, -2, 0, 144, 0, -1, 1 },
    { "!sm121.bin", 0, 1024, 0, 16384, 16384, 16384, -1, 1 },
    { "!ir", 0, 9, 0, 144, 144, 144, -1, 1 },
};

int main(void)
{
    for (size_t i = 0; i < sizeof sm121; i++) sm121[i] = (uint8_t)(i * 13 + 5);
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        const Case *c = &cases[i];
        char cwd[64];
        snprintf(cwd, sizeof cwd, "case%zu", i);
        mkdir(cwd, 0700);
        if (chdir(cwd)) return 2;
        mkdir("d", 0700);
        mkdir("e", 0700);
        printf("case %zu\n", i);
        fflush(stdout);
        int pfd[2];
        if (pipe(pfd)) return 2;
        pid_t pid = fork();
        if (pid == 0) {
            dup2(pfd[1], 2);
            close(pfd[0]);
            child(c);
        }
        close(pfd[1]);
        static char err[1 << 16];
        size_t en = 0;
        for (ssize_t r; (r = read(pfd[0], err + en, sizeof err - 1 - en)) > 0;) en += (size_t)r;
        err[en] = 0;
        close(pfd[0]);
        int st = 0;
        waitpid(pid, &st, 0);
        printf("  %s %d\n", WIFSIGNALED(st) ? "signal" : "exit",
               WIFSIGNALED(st) ? WTERMSIG(st) : WEXITSTATUS(st));
        if (en) { printf("  stderr "); norm(stdout, err, pid); if (err[en - 1] != '\n') printf("\n"); }
        listdir("d", pid);
        listdir(".", pid);
        rmdir("d");
        rmdir("e");
        if (chdir("..")) return 2;
        rmdir(cwd);
    }
    return 0;
}
