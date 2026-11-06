/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Differential driver for tools/own30b_decode_profile.c. Built twice from the same source: with
 * -DCUDAPP_MIGRATION_PENDING_C_TOOLS_OWN30B (the C oracle) and without (om/tools/own30b_prof.om). Both builds read
 * the same splitmix64 stream, print every classification to stdout, and dump the profile to stderr at exit; the
 * two outputs must be byte-identical.
 *
 * usage: own30b_diff SEED TOKENS MUT_PER_MILLE
 *   TOKENS decode steps are driven; each record is mutated with probability MUT_PER_MILLE / 1000 (wrong or
 *   other stage, failed launch, null launch, random shape, wrapped clock, huge byte count). With
 *   MUT_PER_MILLE >= 100 every 64th classification also uses a fully random launch. TOKENS above 4096
 *   exercises the dropped counter. */
#define OWN30B_PROFILE_DIFF 1
#include "../../tools/own30b_decode_profile.c"
#include <stdlib.h>

static uint64_t rng;
static uint64_t next(void) {
    uint64_t z = (rng += 0x9e3779b97f4a7c15ull);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}

static uint32_t args[32];
static OmegaGpuLaunch L;

/* A launch that the profile should classify as stage s. */
static void shape(int s) {
    memset(args, 0, sizeof args);
    memset(&L, 0, sizeof L);
    L.args = args;
    switch (s) {
    case QKV: L.n_args = OMEGA_FP8_ARGS_WORDS; L.grid_y = 1; args[8] = 5120 * 4 + (uint32_t)(next() & 3); break;
    case O: L.n_args = OMEGA_FP8_ARGS_WORDS; L.grid_y = 1; args[8] = 2048 * 4 + (uint32_t)(next() & 3); break;
    case GU: L.n_args = OMEGA_FP8_ARGS_WORDS; L.grid_y = 8; args[8] = 1536 * 4; break;
    case DOWN: L.n_args = OMEGA_FP8_ARGS_WORDS; L.grid_y = 8; args[8] = 2048 * 4; break;
    case ATTN: L.n_args = OMEGA_DATTN_ARGS_WORDS; L.num_elements = 4096; args[18] = (next() & 1) ? (uint32_t)next() : (uint32_t)(next() & 4095); break;
    case ROUTER: L.n_args = OMEGA_ROUTER_ARGS_WORDS; L.num_elements = 2048; break;
    case HEAD: L.n_args = OMEGA_BF16_ARGS_WORDS; L.num_elements = 151936; break;
    default: L.n_args = 3; break;
    }
}

/* A launch with every classified field drawn from the boundary sets or at random. */
static void wild(void) {
    static const uint32_t na[] = { OMEGA_FP8_ARGS_WORDS, OMEGA_DATTN_ARGS_WORDS, OMEGA_ROUTER_ARGS_WORDS, OMEGA_BF16_ARGS_WORDS, 0, 9, 11, 24, 26, 32 };
    static const uint32_t a8[] = { 5120 * 4, 2048 * 4, 1536 * 4, 2048 * 4 + 3, 5120 * 4 - 1, 1536 * 4 + 4, 0, 0xffffffffu };
    static const uint32_t gy[] = { 0, 1, 2, 7, 8, 9, 0xffffffffu };
    static const size_t ne[] = { 4096, 2048, 151936, 0, 4095, 2049, 151937, (size_t)-1 };
    memset(&L, 0, sizeof L);
    for (int i = 0; i < 32; i++) args[i] = (uint32_t)next();
    L.args = (next() & 15) ? args : NULL;
    L.n_args = (next() & 7) ? na[next() % 10] : (uint32_t)(next() & 31);
    args[8] = (next() & 3) ? a8[next() % 8] : (uint32_t)next();
    L.grid_y = (next() & 3) ? gy[next() % 7] : (uint32_t)next();
    L.num_elements = (next() & 3) ? ne[next() % 8] : (size_t)next();
}

static uint64_t clk;
static unsigned nclass;

static void step(int want, unsigned mut) {
    int s = want, rc = 0, nul = 0;
    uint64_t bytes, begin, end, elapsed, extra = 0;
    if (mut >= 100 && ++nclass % 64 == 0) wild(); else shape(want);
    if (mut && next() % 1000 < mut) {
        switch (next() % 8) {
        case 0: shape((int)(next() % 8)); break;
        case 1: shape(OTHER); break;
        case 2: rc = (next() & 1) ? -1 : (int)(next() % 200) + 1; break;
        case 3: nul = 1; break;
        case 4: wild(); break;
        case 5: clk -= next() % 100000; break;
        case 6: extra = next(); break;
        default: clk = next(); break;
        }
    }
    s = classify(nul ? NULL : &L, &bytes);
    printf("C %d %" PRIu64 "\n", s, bytes);
    begin = clk + next() % 5000;
    end = begin + next() % 200000;
    if (extra && (extra & 1)) end = begin - (extra >> 40);
    elapsed = (next() & 255) ? next() % 150000 : next();
    record(s, bytes + (extra ? extra : 0), begin, end, elapsed, rc);
    clk = end;
}

int main(int argc, char **argv) {
    if (argc != 4) { fprintf(stderr, "usage: own30b_diff SEED TOKENS MUT_PER_MILLE\n"); return 2; }
    rng = strtoull(argv[1], NULL, 0);
    unsigned long toks = strtoul(argv[2], NULL, 0);
    unsigned mut = (unsigned)strtoul(argv[3], NULL, 0);
    clk = next() >> (next() & 63);
    step(HEAD, 0);
    for (unsigned long t = 0; t < toks; t++) {
        for (unsigned i = 0; i < 288; i++) step((int)(i % 6), mut);
        step(HEAD, mut);
        /* Occasionally record without a head first, or a stray launch between steps. */
        if (mut && next() % 50 == 0) step((int)(next() % 8), mut);
    }
    printf("RETAINED %" PRIu64 "\n", prof_used());
    return 0;
}
