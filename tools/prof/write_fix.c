/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Independent writer for the built-in profiler fixture. The Omega `fix` mode
 * must produce these same 176 bytes. It does not open a device. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

static void put_u32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static void put_u64(uint8_t *p, uint64_t v) {
    int i;
    for (i = 0; i < 8; i++) {
        p[i] = (uint8_t)(v >> (8 * i));
    }
}

static void put_kern(uint8_t *p, uint32_t id, uint64_t bytes, uint64_t flops,
                     uint32_t before, uint32_t after) {
    put_u32(p, id);
    put_u32(p + 4, 0);
    put_u64(p + 8, bytes);
    put_u64(p + 16, flops);
    put_u32(p + 24, before);
    put_u32(p + 28, after);
}

int main(int argc, char **argv) {
    uint8_t b[176];
    FILE *f;
    if (argc != 2) {
        return 2;
    }
    memset(b, 0, sizeof b);
    b[0] = 80;
    b[1] = 82;
    b[2] = 79;
    b[3] = 70;
    put_u32(b + 4, 1);
    put_u32(b + 8, 3);
    put_u32(b + 12, 4);
    put_kern(b + 16, 0, 195100000000ull, 115140000000ull, 0, 1);
    put_kern(b + 48, 1, 195100000ull, 115140000000000ull, 1, 2);
    put_kern(b + 80, 2, 97550000000ull, 57570000000000ull, 2, 3);
    put_u64(b + 112, 1000000000000ull);
    put_u64(b + 128, 1001000000000ull);
    put_u64(b + 144, 1002000000000ull);
    put_u64(b + 160, 1003000000000ull);
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    f = fd < 0 ? NULL : fdopen(fd, "wb");
    if (f == NULL && fd >= 0) close(fd);
    if (f == NULL) {
        return 5;
    }
    if (fwrite(b, 1, 176, f) != 176) {
        fclose(f);
        return 5;
    }
    if (fclose(f) != 0) {
        return 5;
    }
    return 0;
}
