/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Link-only recorder for OWN-30B decode. No production launcher changes.
 * Link with -Wl,--wrap=omega_gpu_session_launch, this object and src/cudapp_om_own30b_prof.c.
 * Measures HOST times: elapsed_ns is submit-to-second-marker, NOT GPU cycles.
 * Only complete sequences of 48 x (qkv, attention, o, router, gu, down) + head
 * following a head are retained. Prefill, controls and partial steps are rejected.
 * No allocation or I/O in the measured path. Dump raw integer totals at exit.
 *
 * The classifier, the sequence check, the token table and the dump text are the seeded Omega unit
 * om/tools/own30b_prof.om (omega_own30b_prof). This file keeps the wrap: the clock, the real launch and the
 * stderr writes. -DCUDAPP_MIGRATION_PENDING_C_TOOLS_OWN30B builds the former C body, kept as the oracle.
 */
#include "omega_gpu_session.h"
#include "omega_blackwell_codegen_moe.h"
#include "omega_blackwell_codegen_attn.h"
#include "omega_blackwell_codegen_router.h"
#include "omega_blackwell_codegen_bf16.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

enum { QKV, ATTN, O, ROUTER, GU, DOWN, HEAD, OTHER, STAGES = OTHER };
__attribute__((unused)) static const char *names[STAGES] = { "qkv", "attention", "o", "router", "gate_up", "down", "lm_head" };

#ifdef CUDAPP_MIGRATION_PENDING_C_TOOLS_OWN30B
typedef struct { uint64_t calls, bytes, session_ns, submit_wait_ns, gap_before_ns; } Row;
typedef struct { Row stage[STAGES]; uint64_t end_ns, start_ns; uint32_t pos; } Token;
static Token pending, tokens[4096];
static size_t used, dropped;
static int armed, invalid;
static uint64_t last_end;

static int classify(const OmegaGpuLaunch *l, uint64_t *bytes) {
    *bytes = 0;
    if (!l || !l->args) return OTHER;
    if (l->n_args == OMEGA_FP8_ARGS_WORDS) {
        uint32_t n = l->args[8] / 4u, g = l->grid_y, k;
        int s;
        if (g == 1 && n == 5120) { s = QKV; k = 2048; }
        else if (g == 1 && n == 2048) { s = O; k = 4096; }
        else if (g == 8 && n == 1536) { s = GU; k = 2048; }
        else if (g == 8 && n == 2048) { s = DOWN; k = 768; }
        else return OTHER;
        *bytes = (uint64_t)g * n * k;
        return s;
    }
    if (l->n_args == OMEGA_DATTN_ARGS_WORDS && l->num_elements == 4096) {
        *bytes = 2ull * 4 * ((uint64_t)l->args[18] + 1) * 256;
        return ATTN;
    }
    if (l->n_args == OMEGA_ROUTER_ARGS_WORDS && l->num_elements == 2048) {
        *bytes = 128ull * 2048 * 2;
        return ROUTER;
    }
    if (l->n_args == OMEGA_BF16_ARGS_WORDS && l->num_elements == 151936) {
        *bytes = 151936ull * 2048 * 2;
        return HEAD;
    }
    return OTHER;
}

static void record(int s, uint64_t bytes, uint64_t begin, uint64_t end, uint64_t elapsed, int rc) {
    if (armed) {
        int expected = pending.pos < 288 ? (int)(pending.pos % 6) : HEAD;
        if (rc || s != expected || pending.pos > 288) invalid = 1;
        if (!invalid) {
            Row *r = &pending.stage[s];
            r->calls++;
            r->bytes += bytes;
            r->session_ns += end - begin;
            r->submit_wait_ns += elapsed;
            r->gap_before_ns += begin - last_end;
            pending.pos++;
        }
    }
    if (s == HEAD) {
        if (armed && !invalid && pending.pos == 289) {
            pending.end_ns = end;
            if (used < sizeof tokens / sizeof tokens[0]) tokens[used++] = pending;
            else dropped++;
        }
        memset(&pending, 0, sizeof pending);
        pending.start_ns = end;
        armed = !rc;
        invalid = 0;
    }
    last_end = end;
}

#ifndef OWN30B_PROFILE_TEST
__attribute__((destructor)) static void dump(void) {
    fprintf(stderr, "DECODE_PROFILE semantics=host_session_and_submit_wait weight_bytes_exclude_scales pure_gpu_time=UNKNOWN tokens=%zu dropped=%zu\n", used, dropped);
    for (size_t t = 0; t < used; ++t) {
        fprintf(stderr, "DECODE_TOKEN token=%zu wall_ns=%" PRIu64 " launches=%u\n", t, tokens[t].end_ns - tokens[t].start_ns, tokens[t].pos);
        for (int s = 0; s < STAGES; ++s) {
            Row *r = &tokens[t].stage[s];
            fprintf(stderr, "DECODE_STAGE token=%zu stage=%s calls=%" PRIu64 " bytes=%" PRIu64 " session_ns=%" PRIu64 " submit_wait_ns=%" PRIu64 " gap_before_ns=%" PRIu64 "\n",
                    t, names[s], r->calls, r->bytes, r->session_ns, r->submit_wait_ns, r->gap_before_ns);
        }
    }
}
#endif

/* Test accessors: retained tokens, and word w of token t in the Omega layout (5 s + f, 35 end, 36 start, 37 pos). */
__attribute__((unused)) static uint64_t prof_used(void) { return used; }
__attribute__((unused)) static uint64_t prof_tok(size_t t, unsigned w) {
    const Token *k = &tokens[t];
    if (w < 35) {
        const Row *r = &k->stage[w / 5];
        const uint64_t f[5] = { r->calls, r->bytes, r->session_ns, r->submit_wait_ns, r->gap_before_ns };
        return f[w % 5];
    }
    return w == 35 ? k->end_ns : w == 36 ? k->start_ns : k->pos;
}
#else
#include "cudapp_native.h"
#include <stdlib.h>

/* Omega state: see om/tools/own30b_prof.om. 64 header words plus 4096 tokens of 38 words. */
enum { OW_STATE = 155712, OW_TOK0 = 64, OW_TOKW = 38, OW_OUT = 2048 };
static uint64_t ow_state[OW_STATE];
static uint8_t ow_out[OW_OUT];

static uint64_t ow_call(uint64_t op, const uint64_t a[16]) {
    uint64_t r = 0;
    if (omega_own30b_prof(&r, op, ow_state, OW_STATE, a, 16, ow_out, OW_OUT)) {
        fputs("own30b_decode_profile: omega_own30b_prof trapped\n", stderr);
        abort();
    }
    return r;
}

static int classify(const OmegaGpuLaunch *l, uint64_t *bytes) {
    int has = l && l->args;
    uint64_t a[16] = { (uint64_t)has, has ? l->n_args : 0, has && l->n_args > 8 ? l->args[8] : 0, has ? l->grid_y : 0,
                       has ? l->num_elements : 0, has && l->n_args > 18 ? l->args[18] : 0, OMEGA_FP8_ARGS_WORDS,
                       OMEGA_DATTN_ARGS_WORDS, OMEGA_ROUTER_ARGS_WORDS, OMEGA_BF16_ARGS_WORDS };
    int s = (int)ow_call(0, a);
    *bytes = ow_state[5];
    return s;
}

static void record(int s, uint64_t bytes, uint64_t begin, uint64_t end, uint64_t elapsed, int rc) {
    const uint64_t a[16] = { (uint64_t)s, bytes, begin, end, elapsed, rc != 0 };
    ow_call(1, a);
}

#ifndef OWN30B_PROFILE_TEST
__attribute__((destructor)) static void dump(void) {
    uint64_t a[16] = { 0 };
    fwrite(ow_out, 1, ow_call(2, a), stderr);
    for (uint64_t t = 0; t < ow_state[0]; ++t) {
        a[0] = t;
        fwrite(ow_out, 1, ow_call(3, a), stderr);
    }
}
#endif

__attribute__((unused)) static uint64_t prof_used(void) { return ow_state[0]; }
__attribute__((unused)) static uint64_t prof_tok(size_t t, unsigned w) { return ow_state[OW_TOK0 + t * OW_TOKW + w]; }
#endif

#if !defined(OWN30B_PROFILE_TEST) && !defined(OWN30B_PROFILE_DIFF)
int __real_omega_gpu_session_launch(const OmegaGpuLaunch *, uint64_t *, uint32_t *);
int __wrap_omega_gpu_session_launch(const OmegaGpuLaunch *l, uint64_t *elapsed, uint32_t *marker) {
    uint64_t bytes, local_elapsed = 0;
    int s = classify(l, &bytes);
    uint64_t begin = omega_gpu_session_now_ns();
    int rc = __real_omega_gpu_session_launch(l, &local_elapsed, marker);
    uint64_t end = omega_gpu_session_now_ns();
    if (elapsed) *elapsed = local_elapsed;
    record(s, bytes, begin, end, local_elapsed, rc);
    return rc;
}
#endif

#ifdef OWN30B_PROFILE_TEST
#include <assert.h>
int main(void) {
    uint32_t args[32] = {0};
    OmegaGpuLaunch l = { .args = args, .n_args = OMEGA_FP8_ARGS_WORDS, .grid_y = 8 };
    uint64_t bytes;
    args[8] = 2048 * 4;
    assert(classify(&l, &bytes) == DOWN && bytes == 8ull * 2048 * 768);
    l.grid_y = 7;
    assert(classify(&l, &bytes) == OTHER);
    record(HEAD, 0, 0, 1, 1, 0);
    for (unsigned i = 0; i < 289; i++) record(i < 288 ? i % 6 : HEAD, 10, 2 + i * 3, 4 + i * 3, 1, 0);
    assert(prof_used() == 1 && prof_tok(0, 37) == 289);
    uint64_t sum = 0;
    for (unsigned s = 0; s < STAGES; s++) sum += prof_tok(0, 5 * s + 2) + prof_tok(0, 5 * s + 4);
    assert(sum == prof_tok(0, 35) - prof_tok(0, 36));
    /* Swapped/missing/failed stages must never become a decode timing. */
    record(O, 0, 1000, 1001, 1, 0);
    record(HEAD, 0, 1002, 1003, 1, 0);
    assert(prof_used() == 1);
    record(QKV, 0, 1004, 1005, 1, -1);
    record(HEAD, 0, 1006, 1007, 1, 0);
    assert(prof_used() == 1);
    printf("PROFILE_HOST_PASS sequence, accounting, malformed shape, missing stage, failed launch; %s\n", names[HEAD]);
}
#endif
