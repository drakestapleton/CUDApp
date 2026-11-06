/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define CUDAPP_MIGRATION_PENDING_C_DISASM_WORDS 1
#include "omega_dis_words.h"
uint64_t oracle_word(const uint8_t *b, uint64_t mode, uint64_t at, uint64_t prev) {
    return omega_dis_word(b, mode, at, prev);
}
