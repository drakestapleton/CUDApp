/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define _POSIX_C_SOURCE 200809L
/* Host-data loops for elementwise, matmul bf16, gdn, fp8 and scan.
 * Build with CUDAPP_MIGRATION_PENDING_APIVIEWS_DIFF so each call is Omega
 * versus the C oracle. Frozen constants are from the unit's integer rule
 * and from Wolfram, not copied back out of a passing run. */
#include "omega_gpu_apiviews.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static int checks;
static int fails;

static void u32eq(const char *name, uint32_t got, uint32_t want) {
    checks++;
    if (got != want) {
        fprintf(stderr, "FAIL %s got %u want %u\n", name, got, want);
        fails++;
    }
}

static void u64eq(const char *name, uint64_t got, uint64_t want) {
    checks++;
    if (got != want) {
        fprintf(stderr, "FAIL %s got %llu want %llu\n", name,
                (unsigned long long)got, (unsigned long long)want);
        fails++;
    }
}

static uint32_t bits(float f) {
    uint32_t u;
    memcpy(&u, &f, 4);
    return u;
}

/* WolframLanguageEvaluator: gdn hk = Quotient[h, Quotient[vh, 16]]
 * -> 0, 127, 2048, 2175, 4096, 4224. */
static void gdn_cases(void) {
    float src[3] = {1.0f, 2.0f, 3.0f};
    float dst[4];
    float ng[2] = {1.0f, 4.0f};
    float dt[2] = {0.5f, 0.25f};
    float sc[4];
    u32eq("gdn0", av_gdn_channel(0, 0, 128), 0);
    u32eq("gdn127", av_gdn_channel(0, 127, 128), 127);
    u32eq("gdn2048", av_gdn_channel(0, 128, 128), 2048);
    u32eq("gdn2175", av_gdn_channel(0, 255, 128), 2175);
    u32eq("gdn4096", av_gdn_channel(0, 256, 128), 4096);
    u32eq("gdn4224", av_gdn_channel(1, 256, 128), 4224);
    av_gdn_slot(dst, src);
    u32eq("slot0", bits(dst[0]), 1065353216u);
    u32eq("slot3", bits(dst[3]), 0);
    u32eq("scales", (uint32_t)av_gdn_scales(2, ng, dt, sc), 0);
    u32eq("scale0", bits(sc[0]), 1065353216u);
    u32eq("scale1", bits(sc[1]), 1056964608u);
}

/* 1.0 -> code 56, 0.5 -> 48, 4.0 -> 72, |x|>=448 -> 126, -1.0 -> 184.
 * 4.0 is the e4m3 mutant witness: code 72 is above a lowered 64 cap. */
static void e4m3_cases(void) {
    u32eq("e4m3zero", av_e4m3(0.0f), 0);
    u32eq("e4m3one", av_e4m3(1.0f), 56);
    u32eq("e4m3half", av_e4m3(0.5f), 48);
    u32eq("e4m3four", av_e4m3(4.0f), 72);
    u32eq("e4m3sat", av_e4m3(448.0f), 126);
    u32eq("e4m3over", av_e4m3(512.0f), 126);
    u32eq("e4m3neg", av_e4m3(-1.0f), 184);
}

static void bf16_cases(void) {
    float in[4] = {1.0f, 0.0f, -1.0f, 0.5f};
    uint16_t out[4];
    float wide[4];
    u32eq("bf16", (uint32_t)av_bf16_n(0, 4, in, out), 0);
    u32eq("bf16one", out[0], 0x3f80);
    u32eq("bf16zero", out[1], 0);
    u32eq("bf16neg", out[2], 0xbf80);
    u32eq("bf16half", out[3], 0x3f00);
    u32eq("widen", (uint32_t)av_widen_n(4, out, wide), 0);
    u32eq("widenone", bits(wide[0]), 1065353216u);
    u32eq("bf16empty", (uint32_t)av_bf16_n(0, 0, in, out), 0);
}

static void fp8_cases(void) {
    uint64_t ent[2];
    uint16_t w[128 * 128];
    uint8_t q[128 * 128];
    uint16_t scale[1];
    uint32_t i;
    av_fp8_group(ent, 0, 10, 20, 4, 8);
    u64eq("fp8wv", ent[0], 10);
    u64eq("fp8sv", ent[1], 20);
    av_fp8_group(ent, 3, 10, 20, 4, 8);
    u64eq("fp8wv3", ent[0], 22);
    u64eq("fp8sv3", ent[1], 44);
    for (i = 0; i < 128u * 128u; i++) w[i] = 0x3f80;
    u32eq("fp8q", (uint32_t)av_fp8_quantize(128, 128, w, q, scale), 0);
    u32eq("fp8qbad", (uint32_t)av_fp8_quantize(129, 128, w, q, scale), 1);
}

static void ew_cases(void) {
    uint64_t rec[8];
    float cs[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float sn[4] = {0.5f, 0.25f, 0.125f, 0.0625f};
    float tab[8];
    u32eq("ew0", (uint32_t)av_ew_rec(0, 100, 0, 0, rec), 0);
    u64eq("ew0n", rec[0], 100);
    u64eq("ew0g", rec[1], 1);
    u64eq("ew0t", rec[6], 128);
    u32eq("ewcap", (uint32_t)av_ew_rec(0, 200000, 0, 0, rec), 0);
    u64eq("ewcapn", rec[0], 131072);
    u32eq("ew0bad", (uint32_t)av_ew_rec(0, 0, 0, 0, rec), 1);
    u32eq("ewodd", (uint32_t)av_ew_rec(1, 4, 3, 0, rec), 1);
    u32eq("ewdim0", (uint32_t)av_ew_rec(2, 4, 0, 0, rec), 1);
    /* head_dim 16 is the ewarg mutant witness (lowered cap of 8). */
    u32eq("ewhead", (uint32_t)av_ew_rec(1, 4, 16, 0, rec), 0);
    u64eq("ewhf", rec[1], 8);
    u32eq("rope", (uint32_t)av_rope_head(4, cs, sn, tab), 0);
    u32eq("rope0", bits(tab[0]), 1065353216u);
    u32eq("ropeempty", (uint32_t)av_rope_head(0, cs, sn, tab), 0);
}

static void scan_cases(void) {
    float in[32];
    float out[32];
    uint32_t i;
    for (i = 0; i < 32; i++) in[i] = 0.0f;
    u32eq("scann0", (uint32_t)av_scan_tile(0, 0, in, out), 1);
    u32eq("scann33", (uint32_t)av_scan_tile(0, 33, in, out), 1);
    u32eq("scanop", (uint32_t)av_scan_tile(4, 1, in, out), 1);
    in[0] = 1.0f;
    u32eq("scansum", (uint32_t)av_scan_tile(0, 1, in, out), 0);
    u32eq("scanslot0", bits(out[0]), 1065353216u);
    u32eq("scanpre", (uint32_t)av_scan_tile(3, 1, in, out), 0);
    u32eq("scanpre0", bits(out[0]), 1065353216u);
    /* +0 and -0 under max: the scan mutant flips the both-zero fmax arm. */
    in[0] = 0.0f;
    in[1] = -0.0f;
    u32eq("scanmax0", (uint32_t)av_scan_tile(1, 2, in, out), 0);
    u32eq("scanmin", (uint32_t)av_scan_tile(2, 1, in, out), 0);
}

static void bench(void) {
    float in[32];
    float out[32];
    uint16_t w[128 * 128];
    uint8_t q[128 * 128];
    uint16_t scale[1];
    struct timespec a, c;
    uint32_t i;
    uint64_t ns;
    for (i = 0; i < 32; i++) in[i] = 1.0f;
    clock_gettime(CLOCK_MONOTONIC, &a);
    for (i = 0; i < 200; i++) av_scan_tile(0, 32, in, out);
    clock_gettime(CLOCK_MONOTONIC, &c);
    ns = (uint64_t)(c.tv_sec - a.tv_sec) * 1000000000ull + (uint64_t)(c.tv_nsec - a.tv_nsec);
    printf("bench scan_tile 200 x 32: %llu ns\n", (unsigned long long)ns);
    for (i = 0; i < 128u * 128u; i++) w[i] = 0x3f80;
    clock_gettime(CLOCK_MONOTONIC, &a);
    for (i = 0; i < 200; i++) av_fp8_quantize(128, 128, w, q, scale);
    clock_gettime(CLOCK_MONOTONIC, &c);
    ns = (uint64_t)(c.tv_sec - a.tv_sec) * 1000000000ull + (uint64_t)(c.tv_nsec - a.tv_nsec);
    printf("bench fp8_quantize 200 x 128x128: %llu ns\n", (unsigned long long)ns);
}

int main(int argc, char **argv) {
    gdn_cases();
    e4m3_cases();
    bf16_cases();
    fp8_cases();
    ew_cases();
    scan_cases();
    if (argc > 1 && strcmp(argv[1], "--bench") == 0) bench();
    if (fails) {
        fprintf(stderr, "FAIL apiviews host %d/%d\n", fails, checks);
        return 1;
    }
    printf("PASS apiviews host %d checks\n", checks);
    return 0;
}
