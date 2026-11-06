/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_DIS_WORDS_H
#define OMEGA_DIS_WORDS_H
#include "cudapp_native.h"
#include <stdlib.h>
#ifndef CUDAPP_MIGRATION_PENDING_C_DISASM_WORDS
#define CUDAPP_MIGRATION_PENDING_C_DISASM_WORDS 0
#endif
static inline uint64_t omega_dis_word(const uint8_t *b, uint64_t mode, uint64_t at, uint64_t prev) {
#if CUDAPP_MIGRATION_PENDING_C_DISASM_WORDS
    uint64_t v = 0;
    for (int i = 7; i >= 0; i--) v = v << 8 | b[i];
    return mode ? ((v & 0xfff) == 0x94d ? at + 16 : prev) : v;
#else
    uint64_t r;
    if (omega_dis_words(&r, b, 8, mode, at, prev)) abort();
    return r;
#endif
}
#endif
