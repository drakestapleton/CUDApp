/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Stand-in for ldd and nm. The zero-CUDA argv test copies this binary onto PATH
 * under both names. It records argv and prints a CUDA line only when the named
 * file itself contains a marker. It does not run a shell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *base_name(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

static void record_argv(int argc, char **argv) {
    const char *log_path = getenv("ZERO_CUDA_ARGV_LOG");
    FILE *log;
    int i;

    if (!log_path || !log_path[0]) return;
    log = fopen(log_path, "a");
    if (!log) return;
    fprintf(log, "argc %d\n", argc);
    for (i = 0; i < argc; i++) {
        size_t n = strlen(argv[i]);
        fprintf(log, "arg %d %zu\n", i, n);
        fwrite(argv[i], 1, n, log);
        fputc('\n', log);
    }
    fclose(log);
}

int main(int argc, char **argv) {
    const char *tool = base_name(argv[0]);
    const char *path = NULL;
    FILE *f;
    char buf[4096];
    size_t n;

    record_argv(argc, argv);
    if (strcmp(tool, "nm") == 0) {
        if (argc >= 3) path = argv[2];
    } else if (argc >= 2) {
        path = argv[1];
    }
    if (!path) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    buf[n] = '\0';
    if (strcmp(tool, "ldd") == 0 && strstr(buf, "LIBCUDA_NEEDED")) {
        printf("\tlibcuda.so.1 => /tmp/libcuda.so.1 (0x0000)\n");
    }
    if (strcmp(tool, "nm") == 0 && strstr(buf, "CU_SYMBOL")) {
        printf("                 U cuInit\n");
    }
    return 0;
}
