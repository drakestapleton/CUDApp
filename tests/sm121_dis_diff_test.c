/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* sm121_dis_diff_test.c -- the .om sm_121 decoder (om/disasm/sm121_dis.om, generated src/cudapp_om_sm121_dis.c)
 * against the migration-pending C decoder it replaces, and both against golden words (make test-om-disasm).
 *
 *   (no argument)   golden words: each must print its recorded text; the same word with its destination bit 16
 *                   flipped (the corrupted-word control) must not. Then the differential: per row of the C table,
 *                   N random words inside the row's operand fields (guard and control bits random too), 20 N fully
 *                   random words (half with a known opcode), 5 N IMAD-immediate words (every power of two, Rc = RZ,
 *                   0 and 1), then all 32 powers of two with Rc = RZ (the IMAD.SHL boundary), N = 300. Every word
 *                   must give the same status and the same text from both decoders.
 *   --plant         the comparator's red check: one byte of one .om text is changed; the run must report FAIL.
 *   --corrupt       the golden red check: the corrupted words are checked as if they were the golden ones; FAIL.
 * Host only: no GPU, no NVIDIA tool. The golden texts are what nvdisasm 13.0.85 -b SM121 printed for these words on
 * 2026-11-05 (the tools/test_bw_spec_encoder.sh samples, and the HADD2.F32 and F2F.F16.F32 fixture words of the native
 * f16 IR ops); make test-sm121dis is where nvdisasm itself is compared. */
#define CUDAPP_MIGRATION_PENDING_DISASM_DIFF 1
#include "../src/omega_sm121_disasm.c"

static uint64_t s = 88172645463325252ull;
static uint64_t rnd(void) { s ^= s << 13; s ^= s >> 7; s ^= s << 17; return s; }
static long bad, n, plant = -1;

static void one(uint64_t lo, uint64_t hi, uint64_t pc) {
    char a[OM_DIS_TEXT], b[OM_DIS_TEXT];
    int ra = om_sm121_disasm(lo, hi, pc, a, sizeof a);
    int rb = c_sm121_disasm(lo, hi, pc, b, sizeof b);
    if (n++ == plant && a[0]) a[0] ^= 1;
    if (ra != rb || strcmp(a, b) != 0) {
        if (bad++ < 20)
            printf("DIFF %016llx %016llx pc 0x%llx\n  om %d: %s\n  C  %d: %s\n", (unsigned long long)lo,
                   (unsigned long long)hi, (unsigned long long)pc, ra, a, rb, b);
    }
}

static const struct { uint64_t lo, hi; const char *text; } GOLD[] = {
    { 0x0000000000007918ull, 0x000fc00000000000ull, "NOP" },
    { 0x0000000201007224ull, 0x001fca00078e0203ull, "IMAD R0, R1, R2, R3" },
    { 0x0000000605047221ull, 0x004fc40000000000ull, "FADD R4, R5, R6" },
    { 0x000000050c0a7981ull, 0x000f22000c1e1900ull, "LDG.E R10, desc[UR5][R12.64]" },
    { 0x00000000003c7947ull, 0x000fea0003800000ull, "BRA 0x100" },
    { 0x0000000201207c24ull, 0x001fca000f8e0203ull, "IMAD R32, R1, UR2, R3" },
    { 0x000000ffff047212ull, 0x001fca00078e0007ull, "LOP3.LUT R4, RZ, RZ, R7, 0x0, !PT" },
    { 0x000000ffff047219ull, 0x001fca0000000607ull, "SHF.L.U32 R4, RZ, RZ, R7" },
    { 0x000000ffff047227ull, 0x001fca00078e0007ull, "IMAD.HI.U32 R4, RZ, RZ, R7" },
    { 0x20000002ff097230ull, 0x0000000000004100ull, "HADD2.F32 R9, -RZ, R2.H0_H0" },
    { 0x0000000200097304ull, 0x0000000000200800ull, "F2F.F16.F32 R9, R2" },
};

/* corrupt = 0: each golden word must print its text and its corrupted copy must not; corrupt = 1: the corrupted
 * copies are held to the golden text, so every one must fail. Returns the number of failures. */
static int golden(int corrupt) {
    int fail = 0;
    for (size_t i = 0; i < sizeof GOLD / sizeof GOLD[0]; i++) {
        for (int flip = 0; flip < 2; flip++) {
            char t[OM_DIS_TEXT];
            int rc = omega_sm121_disasm(GOLD[i].lo ^ (flip ? 1ull << 16 : 0), GOLD[i].hi, 0, t, sizeof t); /* both */
            int same = rc == 0 && strcmp(t, GOLD[i].text) == 0;
            if (corrupt ? flip == 1 && !same : same != (flip == 0)) {
                printf("%s %s: got %d \"%s\"\n", flip ? "CORRUPTED WORD" : "GOLDEN WORD", GOLD[i].text, rc, t);
                fail++;
            }
        }
    }
    return fail;
}

int main(int argc, char **argv) {
    const char *mode = argc > 1 ? argv[1] : "";
    if (strcmp(mode, "--corrupt") == 0) {
        int f = golden(1);
        printf("%s corrupted words read as golden: %d of %zu differ\n", f ? "FAIL" : "PASS", f, sizeof GOLD / sizeof GOLD[0]);
        return f != 0;
    }
    if (strcmp(mode, "--plant") == 0) plant = 4321;
    int gf = golden(0);
    if (gf) { printf("FAIL %d golden words\n", gf); return 1; }
    printf("PASS %zu golden words; %zu corrupted words refused\n", sizeof GOLD / sizeof GOLD[0], sizeof GOLD / sizeof GOLD[0]);
    const long per = 300;
    for (size_t i = 0; i < sizeof ROWS / sizeof ROWS[0]; i++) {
        uint64_t lom, him;
        fields_mask(ROWS[i].fmt, &lom, &him);
        for (long k = 0; k < per; k++) {
            uint64_t lo = (ROWS[i].lov & ~lom & ~mask(12, 4)) | (rnd() & (lom | mask(12, 4)));
            uint64_t hi = (ROWS[i].hiv & ~him) | (rnd() & him) | (rnd() & ~HI_MASK);
            one(lo, hi, (rnd() & 0xffff) << 4);
        }
    }
    for (long k = 0; k < per * 20; k++) {
        uint64_t lo = rnd(), hi = rnd();
        if (k & 1) lo = (lo & ~0xfffull) | (ROWS[rnd() % (sizeof ROWS / sizeof ROWS[0])].lov & 0xfff);
        one(lo, hi, (rnd() & 0xffff) << 4);
    }
    for (long k = 0; k < per * 5; k++) {
        uint64_t lo = 0x824 | (rnd() & 0xffffffffff00f000ull), hi = 0x078e0200ull | (rnd() & 0xff) | (rnd() & ~HI_MASK);
        if (k % 4 == 0) lo = (lo & 0xffffffffull) | (uint64_t)(1u << (k / 4 % 32)) << 32;
        if (k % 4 == 1) hi = (hi & ~0xffull) | 0xff;
        if (k % 8 == 2) lo &= 0xffffffffull;
        if (k % 8 == 3) lo = (lo & 0xffffffffull) | 1ull << 32;
        one(lo, hi, 0);
    }
    for (uint64_t e = 0; e < 32; e++) /* every power-of-two immediate with Rc = RZ: the IMAD.SHL boundary */
        for (uint64_t r = 0; r < 4; r++) one(0x824 | r << 16 | r << 24 | (1ull << e) << 32, 0x078e02ffull, 0);
    printf("%s %ld words, %ld differ (.om vs C)\n", bad ? "FAIL" : "PASS", n, bad);
    return bad != 0;
}
