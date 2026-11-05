/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host test for omega_blackwell_verify_zero_libcuda_linkage and
 * omega_blackwell_verify_zero_cuda_symbols. binary_path is one pathname, including
 * when it contains spaces or shell metacharacters. A shell must not split it or run it.
 *
 * The tools on PATH are copies of tests/zero_cuda_tool_stub.c. The stub prints a CUDA
 * line only when that exact file contains a marker, and it appends the argv it received. */
#include "omega_blackwell_submit.h"

#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char g_cleanup_dir[128];

static int rm_tree_entry(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    (void)sb;
    (void)typeflag;
    (void)ftwbuf;
    return remove(path);
}

static void cleanup_tmpdir(void) {
    int left;
    if (!g_cleanup_dir[0]) return;
    left = chdir("/tmp");
    if (left != 0) left = chdir("/");
    (void)left;
    nftw(g_cleanup_dir, rm_tree_entry, 16, FTW_DEPTH | FTW_PHYS);
    g_cleanup_dir[0] = '\0';
}

static int g_failed;

static void expect_int(const char *name, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s: got %d want %d\n", name, got, want);
        g_failed++;
    } else {
        printf("PASS %s\n", name);
    }
}

static FILE *open_mode(const char *path, int flags, mode_t mode, const char *fdmode) {
    int fd = open(path, flags, mode);
    FILE *f;
    if (fd < 0) return NULL;
    f = fdopen(fd, fdmode);
    if (!f) close(fd);
    return f;
}

static int write_file(const char *path, const char *text) {
    FILE *f = open_mode(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW, 0600, "w");
    if (!f) {
        fprintf(stderr, "FAIL open %s: %s\n", path, strerror(errno));
        return -1;
    }
    if (fputs(text, f) < 0) {
        fclose(f);
        return -1;
    }
    if (fclose(f) != 0) return -1;
    return 0;
}

static int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    FILE *out;
    char buf[8192];
    size_t n;

    if (!in) return -1;
    out = open_mode(dst, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW, 0700, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(in);
            fclose(out);
            return -1;
        }
    }
    fclose(in);
    if (fclose(out) != 0) return -1;
    return 0;
}

static int parse_log(const char *path, int *argc_out, char **args, int max_args) {
    FILE *f = fopen(path, "r");
    int argc = 0;
    int i;

    if (!f) return -1;
    if (fscanf(f, "argc %d\n", &argc) != 1 || argc < 0 || argc > max_args) {
        fclose(f);
        return -1;
    }
    for (i = 0; i < argc; i++) {
        int idx = -1;
        size_t len = 0;
        int nl;
        if (fscanf(f, "arg %d %zu\n", &idx, &len) != 2 || idx != i || len > 4095) {
            fclose(f);
            return -1;
        }
        args[i] = malloc(len + 1);
        if (!args[i] || fread(args[i], 1, len, f) != len) {
            fclose(f);
            return -1;
        }
        args[i][len] = '\0';
        nl = fgetc(f);
        if (nl != '\n') {
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    *argc_out = argc;
    return 0;
}

static void free_args(char **args, int argc) {
    int i;
    for (i = 0; i < argc; i++) free(args[i]);
}

static void expect_argv(const char *name, const char *log_path, int want_argc, const char **want) {
    char *args[8];
    int argc = 0;
    int i;

    memset(args, 0, sizeof(args));
    if (parse_log(log_path, &argc, args, 8) != 0) {
        fprintf(stderr, "FAIL %s: argv log missing or unreadable\n", name);
        g_failed++;
        free_args(args, 8);
        return;
    }
    if (argc != want_argc) {
        fprintf(stderr, "FAIL %s: argc %d want %d\n", name, argc, want_argc);
        g_failed++;
        free_args(args, argc);
        return;
    }
    for (i = 0; i < argc; i++) {
        if (strcmp(args[i], want[i]) != 0) {
            fprintf(stderr, "FAIL %s: arg %d got [%s] want [%s]\n", name, i, args[i], want[i]);
            g_failed++;
            free_args(args, argc);
            return;
        }
    }
    printf("PASS %s\n", name);
    free_args(args, argc);
}

static void reset_log(const char *log_path) {
    FILE *f = open_mode(log_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW, 0600, "w");
    if (f) fclose(f);
}

int main(void) {
    const char *stub = getenv("ZERO_CUDA_TOOL_STUB");
    char dir[] = "/tmp/cudapp-argv-XXXXXX";
    char bindir[256], workdir[256], log_path[256], path_env[4096];
    char ldd_path[300], nm_path[300];
    char spaced_cuda[1024], spaced_clean[1024], dollar_path[1024], nm_spaced[1024];
    char pwn[512], dollar_pwn[512];
    const char *ldd_cuda_argv[2];
    const char *ldd_clean_argv[2];
    const char *ldd_dollar_argv[2];
    const char *nm_spaced_argv[3];
    const char *nm_inject = "nosuch; printf '                 U cuInit\n'";
    const char *nm_inject_argv[3];
    const char *old_path;
    int rc;

    if (!stub || !stub[0]) {
        fprintf(stderr, "FAIL ZERO_CUDA_TOOL_STUB is unset\n");
        return 1;
    }
    if (!mkdtemp(dir)) {
        fprintf(stderr, "FAIL mkdtemp: %s\n", strerror(errno));
        return 1;
    }
    snprintf(g_cleanup_dir, sizeof(g_cleanup_dir), "%s", dir);
    atexit(cleanup_tmpdir);
    snprintf(bindir, sizeof(bindir), "%s/bin", dir);
    snprintf(workdir, sizeof(workdir), "%s/space dir", dir);
    snprintf(log_path, sizeof(log_path), "%s/argv.log", dir);
    snprintf(ldd_path, sizeof(ldd_path), "%s/ldd", bindir);
    snprintf(nm_path, sizeof(nm_path), "%s/nm", bindir);
    snprintf(pwn, sizeof(pwn), "%s/pwned", dir);
    snprintf(dollar_pwn, sizeof(dollar_pwn), "%s/dollar_pwned", dir);
    snprintf(spaced_cuda, sizeof(spaced_cuda), "%s/lib with cuda", workdir);
    snprintf(spaced_clean, sizeof(spaced_clean), "%s/ok; touch pwned", workdir);
    snprintf(dollar_path, sizeof(dollar_path), "%s/$(touch dollar_pwned)", workdir);
    snprintf(nm_spaced, sizeof(nm_spaced), "%s/has cu symbol", workdir);
    if (mkdir(bindir, 0755) != 0 || mkdir(workdir, 0755) != 0) {
        fprintf(stderr, "FAIL mkdir: %s\n", strerror(errno));
        return 1;
    }
    if (copy_file(stub, ldd_path) != 0 || copy_file(stub, nm_path) != 0) {
        fprintf(stderr, "FAIL copy stub\n");
        return 1;
    }
    if (write_file(spaced_cuda, "LIBCUDA_NEEDED\n") != 0 ||
        write_file(spaced_clean, "clean elf stand-in\n") != 0 ||
        write_file(dollar_path, "clean elf stand-in\n") != 0 ||
        write_file(nm_spaced, "CU_SYMBOL\n") != 0) {
        return 1;
    }
    rc = omega_blackwell_verify_zero_libcuda_linkage("/bin/true");
    expect_int("real ldd on /bin/true reports no libcuda", rc, 0);
    rc = omega_blackwell_verify_zero_cuda_symbols("/bin/true");
    expect_int("real nm -u on /bin/true reports no cu symbol", rc, 0);

    old_path = getenv("PATH");
    if (!old_path) old_path = "/usr/bin:/bin";
    snprintf(path_env, sizeof(path_env), "%s:%s", bindir, old_path);
    if (setenv("PATH", path_env, 1) != 0 || setenv("ZERO_CUDA_ARGV_LOG", log_path, 1) != 0) {
        fprintf(stderr, "FAIL setenv\n");
        return 1;
    }
    if (chdir(dir) != 0) {
        fprintf(stderr, "FAIL chdir %s: %s\n", dir, strerror(errno));
        return 1;
    }

    reset_log(log_path);
    rc = omega_blackwell_verify_zero_libcuda_linkage(spaced_cuda);
    expect_int("ldd reads a path with spaces and reports libcuda.so", rc, -1);
    ldd_cuda_argv[0] = "ldd";
    ldd_cuda_argv[1] = spaced_cuda;
    expect_argv("ldd argv keeps the spaced path", log_path, 2, ldd_cuda_argv);

    reset_log(log_path);
    rc = omega_blackwell_verify_zero_libcuda_linkage(spaced_clean);
    expect_int("ldd on a clean file with spaces and a semicolon returns 0", rc, 0);
    ldd_clean_argv[0] = "ldd";
    ldd_clean_argv[1] = spaced_clean;
    expect_argv("ldd argv keeps the semicolon path", log_path, 2, ldd_clean_argv);
    expect_int("semicolon path did not run touch", access(pwn, F_OK) == 0, 0);

    reset_log(log_path);
    rc = omega_blackwell_verify_zero_libcuda_linkage(dollar_path);
    expect_int("ldd on a clean command-substitution path returns 0", rc, 0);
    ldd_dollar_argv[0] = "ldd";
    ldd_dollar_argv[1] = dollar_path;
    expect_argv("ldd argv keeps the command-substitution path", log_path, 2, ldd_dollar_argv);
    expect_int("command substitution did not run touch", access(dollar_pwn, F_OK) == 0, 0);

    reset_log(log_path);
    rc = omega_blackwell_verify_zero_cuda_symbols(nm_spaced);
    expect_int("nm reads a path with spaces and reports cuInit", rc, -1);
    nm_spaced_argv[0] = "nm";
    nm_spaced_argv[1] = "-u";
    nm_spaced_argv[2] = nm_spaced;
    expect_argv("nm argv keeps -u and the spaced path", log_path, 3, nm_spaced_argv);

    reset_log(log_path);
    rc = omega_blackwell_verify_zero_cuda_symbols(nm_inject);
    expect_int("nm does not treat a printf injection as a CUDA symbol", rc, 0);
    nm_inject_argv[0] = "nm";
    nm_inject_argv[1] = "-u";
    nm_inject_argv[2] = nm_inject;
    expect_argv("nm argv keeps the printf injection as one path", log_path, 3, nm_inject_argv);

    if (g_failed) {
        fprintf(stderr, "test-zero-cuda-tool-argv: FAIL (%d)\n", g_failed);
        return 1;
    }
    cleanup_tmpdir();
    if (access(dir, F_OK) == 0) {
        fprintf(stderr, "FAIL tmpdir still present: %s\n", dir);
        return 1;
    }
    printf("PASS tmpdir removed\n");
    printf("test-zero-cuda-tool-argv: PASS\n");
    return 0;
}
