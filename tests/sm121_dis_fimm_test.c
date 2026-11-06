/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* sm121_dis_fimm_test.c -- the .om float-immediate printer (fimm of om/disasm/sm121_dis.om, reached through the
 * test-only entry om/disasm/fimm_probe.om) against the C decoder's fimm (printf %.*g with the shortest precision
 * that sscanf reads back). No row prints a float immediate today, so the word differential cannot reach it.
 * make test-om-disasm-fimm generates the probe binding with the pinned omc and bridge and runs:
 *   (no argument)   every binary32 pattern at a stride (default 1000003), every exponent with its 32 lowest and 32
 *                   highest mantissas in both signs, and 1.0 plus each low-bit run; every text must be identical.
 *   --plant         one byte of one .om text is changed; the run must report FAIL. Host only. */
#define CUDAPP_MIGRATION_PENDING_DISASM_DIFF 1
#include "../src/omega_sm121_disasm.c"

uint32_t omega_fimm_probe(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);

static long bad, n, plant = -1;

static void one(uint32_t u) {
    char a[64];
    Out w = { a, sizeof a, 0, 1 };
    a[0] = 0;
    fimm(&w, u);
    uint64_t in[1] = { u }, r = 0;
    unsigned char o[OM_DIS_TEXT];
    uint32_t st = omega_fimm_probe(&r, in, 1, o, sizeof o);
    if (n++ == plant && r) o[0] ^= 1;
    if (st || r > 63 || memcmp(a, o, r) || a[r]) {
        if (bad++ < 20) printf("DIFF %08x status %u C: %s om: %.*s\n", u, st, a, (int)(r > 63 ? 0 : r), o);
    }
}

int main(int argc, char **argv) {
    uint32_t step = 1000003;
    if (argc > 1 && strcmp(argv[1], "--plant") == 0) plant = 777;
    for (uint64_t u = 0; u < (1ull << 32); u += step) one((uint32_t)u);
    for (uint32_t e = 0; e < 256; e++)
        for (uint32_t k = 0; k < 64; k++) {
            uint32_t m = k < 32 ? k : 0x7fffffu - (k - 32);
            one(e << 23 | m);
            one(1u << 31 | e << 23 | m);
        }
    for (int k = 0; k <= 24; k++) one(0x3f800000u + (1u << k) - 1);
    printf("%s %ld floats, %ld differ (.om vs C)\n", bad ? "FAIL" : "PASS", n, bad);
    return bad != 0;
}
