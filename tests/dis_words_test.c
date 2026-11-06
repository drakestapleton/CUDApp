/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_dis_words.h"
#include "omega_sm121_disasm.h"
#include <stdio.h>
#include <string.h>
extern uint64_t oracle_word(const uint8_t *, uint64_t, uint64_t, uint64_t);
static unsigned cases;
static int check(const uint8_t *b, uint64_t mode, uint64_t at, uint64_t prev, int plant) {
    uint64_t a = omega_dis_word(b, mode, at, prev), c = oracle_word(b, mode, at, prev);
    cases++;
    if (plant) a ^= 1;
    if (a != c) { fprintf(stderr, "word mismatch case %u\n", cases); return 1; }
    return 0;
}
int main(int argc, char **argv) {
    int plant = argc > 1 && !strcmp(argv[1], "--plant");
    uint8_t b[64] = {0};
    const uint64_t edges[] = {0,1,15,16,17,UINT32_MAX-1,UINT32_MAX};
    for (unsigned p = 0; p < 8; p++) for (unsigned v = 0; v < 256; v++) {
        memset(b, 0, sizeof b); b[p] = (uint8_t)v;
        if (check(b,0,0,0,plant)) return 1;
    }
    for (unsigned op = 0; op < 65536; op++) {
        b[0] = op; b[1] = op >> 8;
        for (unsigned e = 0; e < sizeof edges/sizeof edges[0]; e++) {
            if (check(b,1,edges[e],edges[e],0)) return 1;
        }
    }
    memset(b,0,sizeof b);
    if (omega_sm121_padding_start(NULL,64)) return 1;
    for (unsigned p = 0; p < 4; p++) {
        b[p*16] = 0x4d; b[p*16+1] = 0x79;
        for (unsigned n = 0; n <= 64; n++) {
            size_t want = 0;
            for (size_t at = 0; at + 16 <= n; at += 16)
                want = oracle_word(b+at,1,at,want);
            if (omega_sm121_padding_start(b,n) != want) return 1;
        }
    }
    printf("DISASM WORDS PASS cases=%u\n",cases);
    return 0;
}
