/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Admission messages against the C snprintf format, Ampere packing against the
 * inline sign-extend formula, and a cubin image written only under /tmp.
 * WolframLanguageEvaluator: {Mod[56, 64], 128/64, 192/64, 256 + 32,
 * BitAnd[16^^FFFFFFFF, 2147483648]} -> {56, 2, 3, 288, 2147483648}. */
#include "omega_cubin.h"
#include "cudapp_native.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

static int fails;

static void fail(const char *what) {
    fprintf(stderr, "RESIDUE FAIL %s\n", what);
    fails++;
}

static uint64_t sext32(uint64_t v) {
    v &= 0xffffffffull;
    if (v & 0x80000000ull) v |= 0xffffffff00000000ull;
    return v;
}

static void oracle0(const uint64_t src[16], uint64_t dst[8]) {
    int i;
    for (i = 0; i < 6; i++) dst[i] = sext32(src[i]);
    dst[6] = (src[6] & 0xffffffffull) | ((src[7] & 0xffffffffull) << 32);
    dst[7] = src[8] | (src[9] << 1) | (src[10] << 2) | (src[15] << 8)
           | (src[11] << 16) | (src[12] << 24) | (src[13] << 32) | (src[14] << 40);
}

/* 0 match, 2 mismatch, 1 status error. */
static int ampack_one(uint64_t mode, const uint64_t src[16], int flip) {
    uint64_t got[16], want[16];
    size_t n = mode == 0 ? 8 : 16;
    size_t i;
    memset(got, 0x5a, sizeof got);
    if (omega_ampack(&(uint64_t){0}, mode, src, 16, got, 16) != 0) {
        fail("ampack status");
        return 1;
    }
    if (mode == 0) oracle0(src, want);
    else for (i = 0; i < 16; i++) want[i] = sext32(src[i]);
    if (flip) got[0] ^= 1;
    if (memcmp(got, want, n * sizeof(uint64_t)) != 0) return 2;
    return 0;
}

static int ampack_cases(void) {
    static const uint64_t edge[] = {0, 1, 0x7fffffffull, 0x80000000ull, 0xffffffffull};
    size_t a;
    for (a = 0; a < sizeof edge / sizeof edge[0]; a++) {
        uint64_t src[16];
        int i;
        for (i = 0; i < 16; i++) src[i] = edge[(a + (size_t)i) % 5] ^ (uint64_t)(i * 17);
        src[1] = 0xffffffff80000000ull;
        src[6] = 0x1122334455667788ull;
        src[7] = 0xaabbccddeeff0011ull;
        src[15] = 7;
        if (ampack_one(0, src, 0) != 0 || ampack_one(1, src, 0) != 0) {
            fail("ampack bytes");
            return 1;
        }
    }
    return 0;
}

static int admit_one(uint32_t sm, size_t nparams, size_t cap) {
    char err[128], want[128];
    uint8_t text[16];
    uint8_t *out = NULL;
    size_t len = 0;
    int rc, want_rc = 0;
    memset(text, 0x11, sizeof text);
    memset(err, 0xa5, sizeof err);
    want[0] = 0;
    rc = omega_cubin_pack_target(sm, "k", text, sizeof text, NULL, nparams, 1024, 8, 0,
                                 &out, &len, cap ? err : NULL, cap);
    if (sm != 86 && sm != 121) {
        if (cap) snprintf(want, cap, "unsupported cubin target sm_%u", sm);
        want_rc = OMEGA_CUBIN_E_TARGET;
    } else if (nparams) {
        if (cap) snprintf(want, cap, "null parameter array");
        want_rc = OMEGA_CUBIN_E_ARG;
    }
    if (rc != want_rc) {
        fprintf(stderr, "RESIDUE FAIL admit rc sm=%u np=%zu got=%d want=%d\n", sm, nparams, rc, want_rc);
        fails++;
        omega_cubin_free(out);
        return 1;
    }
    if (cap && strcmp(err, want) != 0) {
        fprintf(stderr, "RESIDUE FAIL admit msg sm=%u got='%s' want='%s'\n", sm, err, want);
        fails++;
        omega_cubin_free(out);
        return 1;
    }
    if (want_rc && (out || len)) fail("refusal left an image");
    omega_cubin_free(out);
    return fails ? 1 : 0;
}

static int image_tmp(void) {
    uint8_t text[16];
    uint8_t *a = NULL, *b = NULL;
    size_t na = 0, nb = 0;
    char err[128];
    char path[] = "/tmp/core-residue-XXXXXX";
    int fd;
    FILE *f;
    uint8_t magic[4];
    memset(text, 0x3c, sizeof text);
    if (omega_cubin_pack_target(86, "k", text, sizeof text, NULL, 0, 1024, 8, 0, &a, &na, err, sizeof err) != 0 ||
        omega_cubin_pack_target(121, "k", text, sizeof text, NULL, 0, 1024, 8, 0, &b, &nb, err, sizeof err) != 0) {
        fail("accepted target refused");
        omega_cubin_free(a);
        omega_cubin_free(b);
        return 1;
    }
    if (na < 64 || nb < 64 || memcmp(a, "\177ELF", 4) != 0 || memcmp(b, "\177ELF", 4) != 0) {
        fail("image missing ELF magic");
        omega_cubin_free(a);
        omega_cubin_free(b);
        return 1;
    }
    fd = mkstemp(path);
    if (fd < 0 || write(fd, a, na) != (ssize_t)na) {
        fail("tmp cubin write");
        if (fd >= 0) close(fd);
        omega_cubin_free(a);
        omega_cubin_free(b);
        return 1;
    }
    close(fd);
    f = fopen(path, "rb");
    if (!f || fread(magic, 1, 4, f) != 4 || memcmp(magic, "\177ELF", 4) != 0) fail("tmp cubin reread");
    if (f) fclose(f);
    if (unlink(path) != 0) fail("tmp cubin unlink");
    omega_cubin_free(a);
    omega_cubin_free(b);
    return fails ? 1 : 0;
}

int main(int argc, char **argv) {
    static const uint32_t sms[] = {0, 85, 86, 87, 120, 121, 122, 0xffffffffu};
    uint64_t src[16];
    size_t i;
    int flip = argc > 1 && strcmp(argv[1], "--flip") == 0;
    for (i = 0; i < sizeof sms / sizeof sms[0]; i++) {
        if (admit_one(sms[i], 0, sizeof(char[128]))) return 1;
        if (admit_one(sms[i], 1, 8)) return 1;
    }
    if (admit_one(0, 0, 0)) return 1;
    if (image_tmp()) return 1;
    if (flip) {
        memset(src, 0, sizeof src);
        src[1] = 0xffffffff80000000ull;
        /* Exit 0 only if the corrupted word still matches. The makefile treats that as a broken control. */
        if (ampack_one(0, src, 1) == 2) {
            fputs("CORE-RESIDUE ampack flip caught\n", stderr);
            return 1;
        }
        return 0;
    }
    if (ampack_cases()) return 1;
    puts("CORE-RESIDUE PASS: admission, /tmp cubin, ampack");
    return fails ? 1 : 0;
}
