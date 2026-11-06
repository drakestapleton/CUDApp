/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Copyright 2026 Drake Stapleton (AIEN / aien-dev)
 *
 * omega_path_check.h: a checked copy of a file system path that came from outside the program (an
 * environment variable, a command's output, a caller's argument) before anything opens it.
 *
 * A path is accepted only when it is 1..cap-1 bytes long, every byte is in the POSIX portable file name set
 * [A-Za-z0-9._-] or one of / + , = @ : ~ (no space, quote, control, shell or non-ASCII byte), and no
 * component is "..". Absolute and relative paths are both accepted. The default path asks Omega for the
 * index of the first 0 inside the destination capacity, then asks Omega to classify and store that bounded
 * view. Neither call walks the pointer in C. A refused path leaves out as the empty string.
 */
#ifndef OMEGA_PATH_CHECK_H
#define OMEGA_PATH_CHECK_H

#include <stddef.h>
#include "cudapp_native.h"

#if defined(CUDAPP_MIGRATION_PENDING_LEFTOVERS) && CUDAPP_MIGRATION_PENDING_LEFTOVERS

/* c when it may appear in a checked path, -1 otherwise */
static inline int omega_path_byte(int c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) return c;
    switch (c) {
    case '.': case '_': case '-': case '/': case '+': case ',': case '=': case '@': case ':': case '~':
        return c;
    default:
        return -1;
    }
}

/* 0 with out = in when in is an acceptable path shorter than cap; -1 with out = "" otherwise */
static inline int omega_path_copy(char *out, size_t cap, const char *in) {
    if (!out || cap == 0) return -1;
    out[0] = '\0';
    if (!in || !in[0]) return -1;
    size_t k = 0;
    for (; in[k]; k++) {
        int b = omega_path_byte((unsigned char)in[k]);
        if (b < 0 || k + 1 >= cap) {
            out[0] = '\0';
            return -1;
        }
        out[k] = (char)b;
    }
    out[k] = '\0';
    for (size_t i = 0; i < k;) {
        size_t j = i;
        while (j < k && out[j] != '/') j++;
        if (j - i == 2 && out[i] == '.' && out[i + 1] == '.') {
            out[0] = '\0';
            return -1;
        }
        i = j + 1;
    }
    return 0;
}
#else
/* Default path. Length, classification and stores are Omega (src/cudapp_freest_pathcopy.c).
 * omega2_pathlen stops at the first 0 inside cap and never reads past that bound, so a NUL-terminated
 * source is not read past its allocation. omega2_pathcopy then receives exactly the bytes through that 0.
 * Empty NULL is a refusal here; a nonempty NULL is refused by the binding. */
uint32_t omega2_pathlen(uint64_t *result, const uint8_t *a, uint64_t a_n);
uint32_t omega2_pathcopy(uint64_t *result, const uint8_t *src, uint64_t src_n, uint8_t *dst, uint64_t dst_n);
static inline int omega_path_byte(int c) {
    uint64_t value = 256;
    if (omega_leftovers(&value, 0, (uint64_t)(int64_t)c, 0, 0, 0) != OMEGA_OK)
        return -1;
    return value == 256 ? -1 : (int)value;
}
static inline int omega_path_copy(char *out, size_t cap, const char *in) {
    uint64_t n = 0;
    uint64_t wrote = 0;
    if (!out || cap == 0) return -1;
    out[0] = '\0';
    if (!in) return -1;
    if (omega2_pathlen(&n, (const uint8_t *)in, (uint64_t)cap) != 0 || n == cap) return -1;
    if (omega2_pathcopy(&wrote, (const uint8_t *)in, n + 1, (uint8_t *)out, (uint64_t)cap) != 0 || wrote != 1) {
        out[0] = '\0';
        return -1;
    }
    return 0;
}
#endif

#endif /* OMEGA_PATH_CHECK_H */
