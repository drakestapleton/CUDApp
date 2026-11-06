/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Copyright 2026 Drake Stapleton (AIEN / aien-dev)
 *
 * omega_path_check.h: a checked copy of a file system path that came from outside the program (an
 * environment variable, a command's output, a caller's argument) before anything opens it.
 *
 * A path is accepted only when it is 1..cap-1 bytes long, every byte is in the POSIX portable file name set
 * [A-Za-z0-9._-] or one of / + , = @ : ~ (no space, quote, control, shell or non-ASCII byte), and no
 * component is "..". Absolute and relative paths are both accepted. The accepted path is copied byte by byte
 * through omega_path_byte, so the bytes the caller later opens are exactly the bytes that were checked.
 * A refused path leaves out as the empty string.
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
/* Pointer custody and stores stay here; the scalar Omega transducer owns
 * alphabet, component and length decisions. No fixed-size path truncation. */
static inline int omega_path_byte(int c) {
    uint64_t value = 256;
    if (omega_leftovers(&value, 0, (uint64_t)(int64_t)c, 0, 0, 0) != OMEGA_OK)
        return -1;
    return value == 256 ? -1 : (int)value;
}
static inline int omega_path_copy(char *out, size_t cap, const char *in) {
    if (!out || cap == 0) return -1;
    out[0] = '\0';
    if (!in) return -1;
    uint64_t state = 0;
    for (size_t k = 0;; ++k) {
        unsigned char value = (unsigned char)in[k];
        uint64_t next = 4;
        if (omega_leftovers(&next, 1, state, value, k, cap) != OMEGA_OK ||
            next == 4) {
            out[0] = '\0';
            return -1;
        }
        out[k] = (char)value;
        if (next == 5) return 0;
        state = next;
    }
}
#endif

#endif /* OMEGA_PATH_CHECK_H */
