/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* om_eng_diff_test.c -- differential of om/engine/eng.om against the C oracles it replaced.
 *
 * The same driver is built twice (mk/om-qmd.mk, target test-om-eng-diff):
 *   om build  default: every shell asks om/engine/eng.om (src/cudapp_om_eng.c);
 *   C build   every CUDAPP_MIGRATION_PENDING_C_<PART> defined: every shell runs its C oracle.
 * Each build prints one transcript line per record. A record is one call of a public API (engine, wait, m16 wait,
 * m16 release, platform contract, handle table, memory reserve, cached alloc, serving reserve) or of a gate in
 * src/omega_eng_gates.h, with inputs drawn from a fixed-seed generator before the call, so both builds see the same
 * inputs whatever path they take. The transcripts must be byte-identical; `--compare A B` names the first record
 * that differs.
 *
 * Every call into the unit goes through --wrap=omega_eng: the om build must reach all ENG_OP_COUNT ops and the C build
 * none. ENG_TRACE_FLIP flips the answer of one unit call (negative control): the comparison must then fail.
 * Host only: no device is opened; the session, attention and matmul entry points are recording stubs below. */
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "m16_gpu_wait.h"
#include "m16_native.h"
#include "omega_gpu_attention_api.h"
#include "omega_gpu_engine.h"
#include "omega_gpu_matmul_api.h"
#include "omega_gpu_mem_cached.h"
#include "omega_gpu_mem_reserve.h"
#include "omega_gpu_platform.h"
#include "omega_gpu_platform_linux.h"
#include "omega_gpu_serving.h"
#include "omega_gpu_session.h"
#include "omega_gpu_wait.h"

#if defined(CUDAPP_MIGRATION_PENDING_C_LIFECYCLE)
#define ENG_C_PART 1
#endif
#include "omega_eng_dec.h"
#include "omega_eng_gates.h"

/* ---- unit call counter (both builds link with -Wl,--wrap=omega_eng) ---- */
uint32_t __real_omega_eng(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t __wrap_omega_eng(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
static uint64_t g_opn[ENG_OP_COUNT + 1];
static uint64_t g_calls;
uint32_t __wrap_omega_eng(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n) {
    uint64_t op = ((const uint64_t *)p0)[0];
    uint32_t st = __real_omega_eng(result, p0, p0_n, p1, p1_n);
    g_opn[op < ENG_OP_COUNT ? op : ENG_OP_COUNT]++;
    g_calls++;
#if defined(ENG_TRACE_FLIP)
    if (g_calls == ENG_TRACE_FLIP) ((uint64_t *)p1)[0] ^= 1u;
#endif
    return st;
}

/* ---- generator ---- */
static uint64_t g_rng = 0x9e3779b97f4a7c15ull;
static uint64_t rnd(void) {
    g_rng ^= g_rng << 13;
    g_rng ^= g_rng >> 7;
    g_rng ^= g_rng << 17;
    return g_rng;
}
static uint64_t below(uint64_t n) { return n ? rnd() % n : 0; }
/* Values at the thresholds the oracles compare against, plus their neighbours. */
static const uint64_t EDGE[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 12, 13, 48, 49, 50, 63, 64, 65, 127, 128, 129, 150, 151, 255, 256, 4095, 4096, 4097,
    5000, 8191, 8192, 8193, 16383, 16384, 16385, 262143, 262144, 262145, 0x16000001, 0x16000002, 0x16c0ffee,
    0x19c0ffee, 0x46464646, 0x7fffffff, 0x80000000, 0x80000001, 2147483647, 2147483648ull, 2147483649ull,
    4294967231ull, 4294967232ull, 4294967233ull, 0xfffffffe, 0xffffffff, 0x100000000ull, 0x100000001ull,
    274877906943ull, 274877906944ull, 274877906945ull, 0x7fffffffffffffffull, 0x8000000000000000ull,
    0xffffffffffffefffull, 0xfffffffffffff000ull, 0xfffffffffffff001ull, 0xfffffffffffffffeull, 0xffffffffffffffffull};
static uint64_t pick(void) {
    switch (below(4)) {
    case 0: return EDGE[below(sizeof EDGE / sizeof EDGE[0])];
    case 1: return below(16);
    case 2: return rnd() & 0xffffffffu;
    default: return rnd();
    }
}
static uint32_t pick32(void) { return (uint32_t)pick(); }

static FILE *g_out;
static unsigned long g_rec;
#define REC(...) do { fprintf(g_out, "r%06lu ", g_rec++); fprintf(g_out, __VA_ARGS__); fputc('\n', g_out); } while (0)

/* ---- recording stubs: session, attention, matmul ---- */
static char g_err[256];
static char g_log[512];
static size_t g_logn;
static void logs(const char *s) {
    size_t n = strlen(s);
    if (g_logn + n + 1 >= sizeof g_log) return;
    memcpy(g_log + g_logn, s, n);
    g_logn += n;
    g_log[g_logn] = 0;
}
static void logu(const char *tag, uint64_t v) {
    char b[48];
    snprintf(b, sizeof b, "%s%" PRIx64 ",", tag, v);
    logs(b);
}
static void log_reset(void) { g_logn = 0; g_log[0] = 0; g_err[0] = 0; }

void omega_gpu_session_set_error(const char *stage) { snprintf(g_err, sizeof g_err, "%s", stage ? stage : "(null)"); }
static OmegaGpuSession g_sess;
static int g_dma_rc;
static uint64_t g_dma_len;
static uint32_t g_dma_buf[16];
static int fake_dma_alloc(void *ctx, OmegaGpuDeviceHandle dev, const OmegaGpuBufferDesc *d, OmegaGpuBufferHandle *out,
                          OmegaCpuView *cpu, OmegaGpuVa *gpu, OmegaGpuBusAddr *bus) {
    (void)ctx; (void)dev;
    logu("dk", d->kind); logu("dr", d->rights); logu("dc", d->gpu_cache); logu("da", d->align); logu("db", d->bytes);
    memset(out, 0, sizeof *out);
    cpu->ptr = g_dma_rc == OMEGA_GPU_PLAT_OK ? g_dma_buf : NULL;
    cpu->len = (size_t)g_dma_len;
    gpu->va = 0x100000;
    bus->iova = 0x100000;
    return g_dma_rc;
}
static int fake_dma_free(void *ctx, OmegaGpuBufferHandle b) { (void)ctx; (void)b; logs("free,"); return 0; }
static OmegaGpuPlatform g_plat = {.dma_alloc = fake_dma_alloc, .dma_free = fake_dma_free};
OmegaGpuSession *omega_gpu_session_open(void) { logs("open,"); return &g_sess; }
const OmegaGpuPlatform *omega_gpu_session_platform(void) { return &g_plat; }

static int g_attn_rc, g_rows_rc, g_mm_rc;
int omega_gpu_attention_reserve(uint32_t max_context, uint32_t max_seqs, uint32_t num_q_heads, uint32_t num_kv_heads,
                                uint32_t head_dim, uint32_t kv_block_size) {
    logu("ar", max_context); logu("", max_seqs); logu("", num_q_heads); logu("", num_kv_heads); logu("", head_dim);
    logu("", kv_block_size);
    return g_attn_rc;
}
int omega_gpu_attention_reserve_rows(uint32_t max_rows, uint32_t num_q_heads, uint32_t head_dim) {
    logu("rr", max_rows); logu("", num_q_heads); logu("", head_dim);
    return g_rows_rc;
}
void omega_gpu_attention_unreserve(void) { logs("au,"); }
int omega_gpu_matmul_reserve(uint32_t max_rows, uint32_t max_k, uint32_t max_n, uint32_t max_n_one_row, uint32_t kernel_slots) {
    logu("mr", max_rows); logu("", max_k); logu("", max_n); logu("", max_n_one_row); logu("", kernel_slots);
    return g_mm_rc;
}
void omega_gpu_matmul_seal(int on) { logu("ms", (uint64_t)on); }
void omega_gpu_matmul_unreserve(void) { logs("mu,"); }

/* ---- engine: scripted backend ---- */
enum { CB_OPEN, CB_CHAN, CB_ALLOC, CB_COPYIN, CB_POISON, CB_BUILD, CB_SUBMIT, CB_WM, CB_WM2, CB_WREL, CB_BAR, CB_COPYOUT,
       CB_FREE, CB_CLOSE, CB_N };
typedef struct {
    int rc[CB_N];
    int err[CB_N];
    uint32_t out_mode;   /* 0 poison back, 1 all changed, 2 some changed */
    uint32_t diag[4];
    const uint32_t *poison;
    size_t poison_count;
} Script;
static Script g_sc;
static const char CBNAME[CB_N] = {'o', 'c', 'a', 'i', 'p', 'b', 's', 'm', 'n', 'r', 'v', 'x', 'f', 'z'};
static int cb(int which) {
    char s[2] = {CBNAME[which], 0};
    logs(s);
    errno = g_sc.rc[which] ? g_sc.err[which] : 0;
    return g_sc.rc[which];
}
static int b_open(void *c) { (void)c; return cb(CB_OPEN); }
static int b_chan(void *c) { (void)c; return cb(CB_CHAN); }
static int b_alloc(void *c, int role, size_t bytes) { (void)c; logu("", (uint64_t)role); logu("", bytes); return cb(CB_ALLOC); }
static int b_copyin(void *c, int role, const void *src, size_t bytes) { (void)c; (void)src; logu("", (uint64_t)role); logu("", bytes); return cb(CB_COPYIN); }
static int b_poison(void *c, const uint32_t *w, size_t n) { (void)c; (void)w; logu("", n); return cb(CB_POISON); }
static int b_build(void *c, const OmegaGpuJob *j) { (void)c; (void)j; return cb(CB_BUILD); }
static int b_submit(void *c) { (void)c; return cb(CB_SUBMIT); }
static int b_wm(void *c, uint64_t t) { (void)c; logu("", t); return cb(CB_WM); }
static int b_wm2(void *c, uint64_t t) { (void)c; logu("", t); return cb(CB_WM2); }
static int b_wrel(void *c, uint64_t t) { (void)c; logu("", t); return cb(CB_WREL); }
static int b_bar(void *c, uint64_t t) { (void)c; logu("", t); return cb(CB_BAR); }
static int b_copyout(void *c, void *dst, size_t bytes) {
    (void)c;
    uint32_t *d = dst;
    size_t i, n = bytes / 4u;
    for (i = 0; i < n; i++) {
        uint32_t p = i < g_sc.poison_count && g_sc.poison ? g_sc.poison[i] : 0u;
        d[i] = g_sc.out_mode == 0 ? p : g_sc.out_mode == 1 ? ~p : ((i & 3u) ? p : ~p);
    }
    logu("", bytes);
    return cb(CB_COPYOUT);
}
static void b_diag(void *c, OmegaGpuBackendDiag *d) {
    (void)c;
    d->drv_rc = (int)g_sc.diag[0];
    snprintf(d->drv_text, sizeof d->drv_text, "drv %u", g_sc.diag[1]);
    d->sync_valid = g_sc.diag[2] & 7u;
    d->marker = g_sc.diag[3];
    d->marker2 = ~g_sc.diag[3];
    d->semaphore[0] = g_sc.diag[1];
}
static int b_free(void *c, int role) { (void)c; logu("", (uint64_t)role); return cb(CB_FREE); }
static int b_close(void *c) { (void)c; return cb(CB_CLOSE); }
static const OmegaGpuBackend g_be = {NULL, b_open, b_chan, b_alloc, b_copyin, b_poison, b_build, b_submit, b_wm, b_wm2,
                                     b_wrel, b_bar, b_copyout, b_diag, b_free, b_close};
static uint64_t g_ms;
static uint64_t g_ms_step;
static uint64_t eng_now(void) { g_ms += g_ms_step; return g_ms; }

static void trace_engine(void) {
    static uint32_t a[4100], b[4100], o[4100], poison[4100];
    static const unsigned char prog[64] = {1};
    static const uint32_t COUNTS[] = {0, 1, 2, 63, 64, 65, 1024, 4096, 4097, 0x40000000u, 0xffffffffu};
    OmegaGpuJob j;
    OmegaGpuResult r;
    uint32_t count = below(3) ? (uint32_t)(1 + below(64)) : COUNTS[below(sizeof COUNTS / sizeof COUNTS[0])];
    size_t bytes = (size_t)count * 4u;
    int fit = count <= 4096u;
    int i;
    memset(&j, 0, sizeof j);
    memset(&g_sc, 0, sizeof g_sc);
    for (i = 0; i < CB_N; i++) {
        uint64_t v = below(24);
        g_sc.rc[i] = v == 0 ? -1 : v == 1 ? OMEGA_GPU_BACKEND_TIMEOUT : v == 2 ? OMEGA_GPU_BACKEND_UNKNOWN_STATE : v == 3 ? 7 : 0;
        g_sc.err[i] = (int)below(40);
    }
    g_sc.out_mode = (uint32_t)below(3);
    for (i = 0; i < 4; i++) g_sc.diag[i] = (uint32_t)rnd();
    for (i = 0; i < 4100; i++) poison[i] = (uint32_t)rnd() | 1u;
    j.program = below(16) ? prog : NULL;
    j.program_len = below(16) ? 1 + below(64) : 0;
    j.layout = below(16) ? OMEGA_GPU_LAYOUT_VECTOR_1D : (int)below(4);
    j.element_count = count;
    j.input_a = below(16) ? a : NULL;
    j.input_b = below(16) ? b : NULL;
    j.output = below(16) ? (below(16) ? (void *)o : (void *)((char *)o + 2)) : NULL;
    j.input_a_len = fit && below(12) ? bytes : bytes + 4u;
    j.input_b_len = fit && below(12) ? bytes : bytes - 4u;
    j.output_len = fit && below(12) ? bytes : bytes + 8u;
    j.poison = below(16) ? poison : NULL;
    j.poison_count = below(12) ? count : count + 1u;
    j.timeouts.marker_ms = below(4) ? below(6000) : pick();
    j.timeouts.release_semaphore_ms = below(4) ? below(6000) : pick();
    j.timeouts.marker2_ms = below(4) ? below(6000) : pick();
    j.timeouts.visibility_ms = below(4) ? below(6000) : pick();
    j.flags = below(4) ? 0u : (uint32_t)below(4);
    if (!fit) {
        /* Never let a buffer claim more than it holds: these counts only test the refusals. */
        j.input_a_len = 16;
        j.output_len = 16;
    }
    g_sc.poison = j.poison;
    g_sc.poison_count = j.poison_count;
    g_ms = 1000;
    g_ms_step = 1 + below(700);
    int reset = below(5) != 0;
    int nullres = below(64) == 0;
    int nulljob = below(64) == 0;
    log_reset();
    if (reset) omega_gpu_engine_test_reset_block();
    memset(&r, 0xa5, sizeof r);
    int ret = omega_gpu_execute(nulljob ? NULL : &j, nullres ? NULL : &r);
    if (nullres) {
        REC("eng ret=%d blk=%d log=%s", ret, omega_gpu_engine_is_blocked(), g_log);
        return;
    }
    REC("eng ret=%d f=%d st=%d step=%d w=%d wt=%" PRIu64 " wd=%" PRIu64 " rc=%d en=%d txt=%s sv=%u m=%u m2=%u s=%u ret=%d cf=%d no=%zu"
        " un=%" PRIu64 " blk=%d log=%s",
        ret, r.failure, r.last_state, r.failed_step, r.wait, r.wait_timeout_ms, r.waited_ms, r.drv_rc, r.err_no,
        r.drv_text, r.sync_valid, r.marker, r.marker2, r.semaphore[0], r.retained, r.cleanup_failed, r.n_outputs,
        r.n_outputs ? r.output_unchanged_words[0] : 0u, omega_gpu_engine_is_blocked(), g_log);
}

/* ---- waits: scripted word, clock, snapshot, validity, abort ---- */
typedef struct {
    volatile uint32_t word, m2;
    uint32_t vals[64], m2v[64];
    uint32_t n, i;
    uint64_t t, step;
    uint32_t bad_at, abort_at, snap_mode;
    uint32_t barriers;
} WaitScript;
static WaitScript g_ws;
static uint64_t ws_now(void *c) { WaitScript *s = c; s->t += s->step; return s->t; }
static void ws_barrier(void *c) { ((WaitScript *)c)->barriers++; }
static void ws_relax(void *c) {
    WaitScript *s = c;
    if (s->i + 1 < s->n) s->i++;
    s->word = s->vals[s->i];
    s->m2 = s->m2v[s->i];
}
static void ws_snap(void *c, uint32_t out[2]) {
    WaitScript *s = c;
    out[0] = s->snap_mode ? s->i / s->snap_mode : 0;
    out[1] = 7;
}
static bool ws_valid(void *c, uint32_t v) { WaitScript *s = c; (void)v; return s->i != s->bad_at; }
static bool ws_abort(void *c) { WaitScript *s = c; return s->i == s->abort_at; }
static void ws_draw(void) {
    uint32_t k;
    memset(&g_ws, 0, sizeof g_ws);
    g_ws.n = 1 + (uint32_t)below(64);
    uint32_t base = (uint32_t)pick();
    for (k = 0; k < g_ws.n; k++) {
        g_ws.vals[k] = below(3) ? base + k / (1 + (uint32_t)below(4)) : (uint32_t)pick();
        g_ws.m2v[k] = below(2) ? 0x5au : (uint32_t)below(3);
    }
    g_ws.word = g_ws.vals[0];
    g_ws.m2 = g_ws.m2v[0];
    g_ws.t = 1000000 + below(1000);
    g_ws.step = 100000 + below(20000000);
    g_ws.bad_at = below(4) ? 1000u : (uint32_t)below(64);
    g_ws.abort_at = below(4) ? 1000u : (uint32_t)below(64);
    g_ws.snap_mode = (uint32_t)below(4);
}

static void trace_wait(int m16) {
    ws_draw();
    uint32_t target = below(2) ? g_ws.vals[below(g_ws.n)] : (uint32_t)pick();
    int seq = (int)below(2);
    uint32_t total = below(4) ? (uint32_t)(1 + below(60)) : (uint32_t)below(3);
    uint32_t prog = below(4) ? (uint32_t)(1 + below(30)) : 0u;
    int use_m2 = (int)below(3) == 0, use_snap = (int)below(2), use_valid = (int)below(3) == 0, use_abort = (int)below(3) == 0;
    uint32_t m2want = below(2) ? 0x5au : (uint32_t)below(3);
    /* A NULL cfg selects the real monotonic clock and the 5 s / 600 s defaults: not replayable, so not drawn. */
    int nullrpt = below(64) == 0, nullcfg = 0;
    if (total == 0 && prog == 0) prog = 1; /* the defaults (600 s) would make one record run minutes of fake polls */
    if (total == 0) total = 1;
    if (!m16) {
        omega_gpu_wait_cfg_t cfg;
        omega_gpu_wait_report_t r;
        memset(&cfg, 0, sizeof cfg);
        memset(&r, 0x5a, sizeof r);
        cfg.total_timeout_ms = total;
        cfg.progress_timeout_ms = prog;
        cfg.marker2 = use_m2 ? &g_ws.m2 : NULL;
        cfg.marker2_want = m2want;
        cfg.snapshot = use_snap ? ws_snap : NULL;
        cfg.snapshot_ctx = &g_ws;
        cfg.valid = use_valid ? ws_valid : NULL;
        cfg.abort = use_abort ? ws_abort : NULL;
        cfg.now_ns = ws_now;
        cfg.barrier = ws_barrier;
        cfg.relax = ws_relax;
        cfg.hook_ctx = &g_ws;
        bool ok = seq ? omega_gpu_wait_sequence(&g_ws.word, target, nullrpt ? NULL : &r, nullcfg ? NULL : &cfg)
                      : omega_gpu_wait_fixed(&g_ws.word, target, nullrpt ? NULL : &r, nullcfg ? NULL : &cfg);
        if (nullrpt) { REC("wait ok=%d i=%u bar=%u", ok, g_ws.i, g_ws.barriers); return; }
        REC("wait ok=%d k=%d res=%d exp=%u lo=%u lm2=%u s=%u,%u st=%" PRIu64 " el=%" PRIu64 " lp=%" PRIu64 " pc=%" PRIu64
            " po=%" PRIu64 " i=%u bar=%u",
            ok, (int)r.wait_kind, (int)r.result, r.expected, r.last_observed, r.last_marker2, r.snap[0], r.snap[1],
            r.start_ns, r.elapsed_ns, r.last_progress_ns, r.progress_count, r.polls, g_ws.i, g_ws.barriers);
    } else {
        m16_gpu_wait_cfg_t cfg;
        m16_gpu_wait_report_t r;
        memset(&cfg, 0, sizeof cfg);
        memset(&r, 0x5a, sizeof r);
        cfg.total_timeout_ms = total;
        cfg.progress_timeout_ms = prog;
        cfg.marker2 = use_m2 ? &g_ws.m2 : NULL;
        cfg.marker2_want = m2want;
        cfg.snapshot = use_snap ? ws_snap : NULL;
        cfg.snapshot_ctx = &g_ws;
        cfg.valid = use_valid ? ws_valid : NULL;
        cfg.abort = use_abort ? ws_abort : NULL;
        cfg.now_ns = ws_now;
        cfg.barrier = ws_barrier;
        cfg.relax = ws_relax;
        cfg.hook_ctx = &g_ws;
        bool ok = seq ? m16_gpu_wait_sequence(&g_ws.word, target, nullrpt ? NULL : &r, nullcfg ? NULL : &cfg)
                      : m16_gpu_wait_fixed(&g_ws.word, target, nullrpt ? NULL : &r, nullcfg ? NULL : &cfg);
        if (nullrpt) { REC("m16wait ok=%d i=%u bar=%u", ok, g_ws.i, g_ws.barriers); return; }
        REC("m16wait ok=%d k=%d res=%d exp=%u lo=%u lm2=%u s=%u,%u st=%" PRIu64 " el=%" PRIu64 " lp=%" PRIu64
            " pc=%" PRIu64 " po=%" PRIu64 " i=%u bar=%u fc=%s",
            ok, (int)r.wait_kind, (int)r.result, r.expected, r.last_observed, r.last_marker2, r.snap[0], r.snap[1],
            r.start_ns, r.elapsed_ns, r.last_progress_ns, r.progress_count, r.polls, g_ws.i, g_ws.barriers,
            m16_gpu_wait_failure_class(&r, m2want));
    }
}

static void trace_fclass(void) {
    m16_gpu_wait_report_t r;
    memset(&r, 0, sizeof r);
    r.wait_kind = (m16_gpu_wait_kind_t)(1 + below(2));
    r.result = (m16_gpu_wait_result_t)below(6);
    r.expected = pick32();
    r.last_observed = below(2) ? r.expected : pick32();
    r.last_marker2 = below(2) ? 0x5au : pick32();
    r.snap[0] = pick32();
    r.snap[1] = pick32();
    r.progress_count = below(3) ? below(3) : pick();
    r.polls = pick();
    uint32_t want = below(2) ? 0x5au : pick32();
    /* r is never NULL here: neither the C oracle nor the om shell accepts a NULL report (m16_gpu_wait.h). */
    REC("fclass %s", m16_gpu_wait_failure_class(&r, want));
}

static void trace_release(void) {
    uint32_t w[10];
    int i;
    for (i = 0; i < 10; i++) w[i] = 0xdeadbeefu;
    uint64_t va = pick();
    uint32_t payload = pick32(), cls = below(2) ? 0xcec0u : pick32();
    int nul = below(32) == 0;
    int n = m16_native_build_release(nul ? NULL : w, va, payload, cls);
    REC("rel n=%d %08x %08x %08x %08x %08x %08x %08x %08x %08x", n, w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7], w[8]);
}

/* ---- platform contract and handle table ---- */
static void caps_draw(OmegaGpuPlatformCaps *c) {
    c->abi_major = below(4) ? 1u : pick32();
    c->abi_minor = below(4) ? (uint32_t)below(4) : pick32();
    c->services = below(3) ? (uint32_t)(below(2) ? OMEGA_GPU_SVC_REQUIRED_SESSION : OMEGA_GPU_SVC_REQUIRED_V1) ^ (uint32_t)(below(2) << below(13))
                           : pick32();
    c->max_devices = below(2) ? 1u : pick32();
    c->max_buffers = pick32();
    c->max_queues = pick32();
    c->max_inflight = pick32();
    c->max_buffer_bytes = pick();
    c->max_total_bytes = pick();
    c->submit_prefix_words = pick32();
}
static void dummy_fn(void) {}
static void trace_negotiate(void) {
    OmegaGpuPlatformCaps p, q;
    caps_draw(&p);
    caps_draw(&q);
    if (below(2)) { q.abi_major = p.abi_major; q.services = p.services & q.services; }
    int np = below(64) == 0, nq = below(64) == 0;
    REC("nego %d", omega_gpu_platform_negotiate(np ? NULL : &p, nq ? NULL : &q));
}
static void trace_check(void) {
    OmegaGpuPlatform p;
    OmegaGpuPlatformCaps c;
    uint64_t mask = below(3) ? ~below(2) << below(24) : rnd();
    int k = 0;
    memset(&p, 0, sizeof p);
#define SETF(m) p.m = ((mask >> (k++)) & 1u) ? (__typeof__(p.m))dummy_fn : NULL
    SETF(time_now_ms); SETF(wait_ms); SETF(lock); SETF(unlock); SETF(mmio_read32); SETF(mmio_write32); SETF(dma_alloc);
    SETF(dma_free); SETF(sync_cpu_to_gpu); SETF(sync_gpu_to_cpu); SETF(poll_word); SETF(irq_wait); SETF(device_open);
    SETF(device_close); SETF(diag_text); SETF(queue_open); SETF(queue_close); SETF(queue_submit); SETF(queue_retire);
    SETF(time_now_ns); SETF(host_alloc); SETF(host_free);
#undef SETF
    caps_draw(&c);
    uint32_t req = below(3) ? (below(2) ? OMEGA_GPU_SVC_REQUIRED_SESSION : OMEGA_GPU_SVC_REQUIRED_V1) : pick32();
    int np = below(64) == 0, nc = below(64) == 0;
    REC("pchk %d %d", omega_gpu_platform_check_for(np ? NULL : &p, nc ? NULL : &c, req),
        omega_gpu_platform_check(np ? NULL : &p, nc ? NULL : &c));
}
static void trace_buffer(void) {
    OmegaGpuBufferDesc d;
    OmegaGpuPlatformCaps c;
    caps_draw(&c);
    d.kind = below(3) ? (uint32_t)(2 + below(3)) : pick32();
    d.rights = below(3) ? (uint32_t)below(64) : pick32();
    d.gpu_cache = below(4) ? (uint32_t)below(2) : pick32();
    d.align = below(3) ? (below(2) ? 0u : 1u << below(16)) : pick32();
    d.bytes = below(3) ? below(1u << 20) : pick();
    int nd = below(64) == 0, nc = below(64) == 0;
    REC("buf %d", omega_gpu_platform_check_buffer(nd ? NULL : &d, nc ? NULL : &c));
}
static OmegaGpuHandleTable g_ht;
static void trace_handles(int fresh) {
    if (fresh) omega_gpu_handles_init(&g_ht);
    uint32_t kind = below(4) ? (uint32_t)(1 + below(5)) : pick32();
    uint32_t idx = 0xeeeeeeeeu, gen = 0xeeeeeeeeu;
    int rc;
    switch (below(3)) {
    case 0: {
        uint64_t bytes = below(3) ? 1 + below(1u << 20) : pick();
        uint32_t ls = below(3) ? (uint32_t)(1 + below(64)) : pick32();
        uint64_t lb = below(3) ? 1 + below(1u << 26) : pick();
        int nt = below(128) == 0, ni = below(128) == 0;
        rc = omega_gpu_handles_open(nt ? NULL : &g_ht, kind, bytes, ls, lb, ni ? NULL : &idx, &gen);
        REC("hopen %d %u %u nl=%u tb=%" PRIu64, rc, idx, gen, g_ht.nlive, g_ht.total_bytes);
        break;
    }
    case 1:
        idx = below(3) ? (uint32_t)below(80) : pick32();
        gen = below(3) ? (uint32_t)below(4) : pick32();
        rc = omega_gpu_handles_check(below(128) ? &g_ht : NULL, idx, gen, kind);
        REC("hchk %d", rc);
        break;
    default:
        idx = below(3) ? (uint32_t)below(80) : pick32();
        gen = below(3) ? (uint32_t)below(4) : pick32();
        rc = omega_gpu_handles_close(below(128) ? &g_ht : NULL, idx, gen, kind);
        REC("hclose %d nl=%u tb=%" PRIu64, rc, g_ht.nlive, g_ht.total_bytes);
        break;
    }
}

/* ---- memory reserve, cached alloc, serving reserve ---- */
static char g_meminfo[64];
static void trace_reserve(void) {
    static const char *ENVS[] = {NULL, "", "0", "1", "512", "4096", "17592186044415", "17592186044416", "17592186044417",
                                 "18446744073709551615", "12x", "-1"};
    const char *e = ENVS[below(sizeof ENVS / sizeof ENVS[0])];
    char envb[32];
    if (below(3) == 0) { snprintf(envb, sizeof envb, "%" PRIu64, pick()); e = envb; }
    uint64_t avail = below(3) ? below(1ull << 27) : pick() & 0xffffffffffffull;
    int has_line = below(8) != 0;
    FILE *f = fopen(g_meminfo, "w");
    if (!f) { fprintf(stderr, "cannot write %s\n", g_meminfo); exit(2); }
    fprintf(f, "MemTotal: 131072000 kB\n");
    if (has_line) fprintf(f, "MemAvailable: %" PRIu64 " kB\n", avail);
    fclose(f);
    if (e) setenv("CUDAPP_MEM_RESERVE_MIB", e, 1); else unsetenv("CUDAPP_MEM_RESERVE_MIB");
    uint64_t bytes = below(3) ? below(1ull << 37) : pick();
    char diag[512];
    diag[0] = 0;
    int rc = omega_gpu_mem_reserve_check(bytes, diag, sizeof diag);
    REC("resv %d rb=%" PRIu64 " %s", rc, omega_gpu_mem_reserve_bytes(), diag);
}
static void trace_cached(void) {
    static const uint64_t B[] = {0, 1, 4095, 4096, 4097, 8191, 8192, 0xfffff000ull, 0xffffffffffffefffull,
                                 0xfffffffffffff000ull, 0xfffffffffffff001ull, 0xffffffffffffffffull};
    uint64_t bytes = below(2) ? B[below(sizeof B / sizeof B[0])] : (below(2) ? below(1u << 24) : rnd());
    OmegaGpuMem m;
    int nul = below(16) == 0;
    uint64_t v = below(4);
    g_dma_rc = v == 0 ? OMEGA_GPU_PLAT_REFUSED_LIMIT : OMEGA_GPU_PLAT_OK;
    g_dma_len = v == 1 ? 0 : 0xffffffffffffffffull;
    log_reset();
    memset(&m, 0x77, sizeof m);
    int rc = omega_gpu_session_alloc_cached((size_t)bytes, nul ? NULL : &m);
    REC("cache %d k=%u sz=%" PRIu64 " err=%s log=%s", rc, nul ? 0u : m.kind, nul ? 0u : m.size, g_err, g_log);
}
static void trace_serving(void) {
    OmegaGpuServingBounds b;
    b.max_context = pick32();
    b.max_seqs = below(2) ? (uint32_t)below(3) : pick32();
    b.num_q_heads = pick32();
    b.num_kv_heads = pick32();
    b.head_dim = pick32();
    b.kv_block_size = pick32();
    b.max_rows = below(2) ? (uint32_t)below(3) : pick32();
    b.max_k = pick32();
    b.max_n = pick32();
    b.max_n_one_row = below(2) ? 0u : pick32();
    b.kernel_slots = pick32();
    g_attn_rc = below(5) ? 0 : 3;
    g_rows_rc = below(5) ? 0 : 4;
    g_mm_rc = below(5) ? 0 : 5;
    int nul = below(32) == 0;
    log_reset();
    int rc = omega_gpu_reserve_serving(nul ? NULL : &b);
    REC("serve %d log=%s", rc, g_log);
}

/* ---- gates of src/omega_eng_gates.h ---- */
static void trace_gates(void) {
    int k = (int)below(16);
    switch (k) {
    case 0: case 1: case 2: case 3: {
        uint32_t kk = below(8) ? (uint32_t)below(17) : pick32();
        uint64_t a[6];
        int i;
        for (i = 0; i < 6; i++) a[i] = below(3) ? below(4) : pick();
        if (below(2)) a[1] = a[0];
        if (below(2)) a[5] = a[4];
        if (below(2)) a[3] = a[2];
        REC("life %u %d", kk, omega_gate_life(kk, a[0], a[1], a[2], a[3], a[4], a[5]));
        break;
    }
    case 4: {
        uint32_t kk = (uint32_t)below(4);
        int64_t a = (int64_t)pick();
        REC("reqf %u %d %u", kk, omega_gate_reqf(kk, a), omega_gate_reqf_payload());
        break;
    }
    case 5: {
        uint32_t kk = (uint32_t)below(4);
        uint64_t a = below(2) ? below(3) : pick(), b = below(2) ? below(3) : pick(), c = below(2) ? b : pick();
        REC("vis %u %d", kk, omega_gate_vis(kk, a, b, c));
        break;
    }
    case 6: {
        uint64_t out = 0x1234, v = pick();
        int rc = omega_gate_round_pages(v, &out);
        REC("pages %d %" PRIu64, rc, out);
        break;
    }
    case 7: {
        uint32_t g = 0x1234, t = 0x5678;
        uint64_t need = 0x9abc, n = pick();
        int rc = omega_gate_vec_shape(n, &g, &t, &need);
        REC("vec %d %u %u %" PRIu64, rc, g, t, need);
        break;
    }
    case 8: {
        uint32_t w9 = below(2) ? 0x6u : pick32(), w15 = pick32(), w16 = pick32(), w17 = below(2) ? 0u : pick32(),
                 w19 = pick32();
        uint64_t va = pick();
        REC("qmd1 %d", omega_gate_qmd1_release(w9, w15, w16, w17, w19, va));
        break;
    }
    case 9: {
        int env = (int)below(2), dev = (int)below(2), flt = (int)below(2), reuse = 0x55;
        unsigned char e0 = below(2) ? '1' : (unsigned char)below(256);
        int rc = omega_gate_persist(env, e0, dev, flt, &reuse);
        REC("persist %d %d", rc, reuse);
        break;
    }
    case 10: {
        uint32_t t[4] = {1, 2, 3, 4};
        int rc = omega_gate_timeouts(t);
        REC("toms %d %u %u %u %u", rc, t[0], t[1], t[2], t[3]);
        break;
    }
    case 11: {
        int ptrs = (int)below(2), code = (int)below(2);
        uint64_t m = pick(), kk = pick(), n = pick();
        REC("mmargs %d", omega_gate_matmul_args(ptrs, code, m, kk, n));
        break;
    }
    case 12: {
        uint32_t s[4] = {9, 9, 9, 9};
        int mma = (int)below(2);
        uint64_t m = pick(), n = pick();
        int rc = omega_gate_matmul_shape(mma, m, n, s);
        REC("mmshape %d %u %u %u %u", rc, s[0], s[1], s[2], s[3]);
        break;
    }
    case 13: {
        uint64_t p = below(2) ? below(4) : pick(), f = below(2) ? 1 : pick(), b = below(2) ? 2 : pick();
        REC("prec %d", omega_gate_precision(p, f, b));
        break;
    }
    case 14: {
        uint32_t kk = below(8) ? (uint32_t)below(6) : pick32();
        uint64_t a = below(2) ? below(4) : pick(), b = below(2) ? 0x16000001u : pick(), c = below(2) ? 0x16000002u : pick(),
                 d = below(2) ? 0x46464646u : pick();
        int ex = 0x55;
        int rc = omega_gate_conc(kk, a, b, c, d, &ex);
        REC("conc %u %d %d", kk, rc, ex);
        break;
    }
    default: {
        /* op 10, the signed serial compare the waits use: om answer against the C expression of omega_gpu_wait.c */
        uint64_t a = pick32(), b = below(2) ? (uint32_t)(a + below(5) - 2u) : pick32();
        uint64_t in[2] = {a, b}, w[16];
        int ge;
        if (eng_on()) ge = eng_ask(ENG_OP_SEQGE, in, 2, w) == 0 ? (int)w[0] : -1;
        else ge = (int32_t)((uint32_t)a - (uint32_t)b) >= 0;
        REC("seqge %d", ge);
        break;
    }
    }
}

/* Deterministic lifecycle threshold pass. Random draws reach an exact threshold
 * such as the 256 GiB churn floor too rarely, so every EDGE value is placed in
 * every argument position of every predicate, over three base vectors. */
static void trace_life_edges(void) {
    static const uint64_t base[3] = {0, 1, 3};
    uint32_t kk;
    size_t e, bi;
    int p, i;
    for (kk = 0; kk < 17; kk++)
        for (bi = 0; bi < 3; bi++)
            for (p = 0; p < 6; p++)
                for (e = 0; e < sizeof EDGE / sizeof EDGE[0]; e++) {
                    uint64_t a[6];
                    for (i = 0; i < 6; i++) a[i] = base[bi];
                    a[p] = EDGE[e];
                    REC("lifee %u %d", kk, omega_gate_life(kk, a[0], a[1], a[2], a[3], a[4], a[5]));
                }
}

/* ---- compare mode ---- */
static int compare(const char *pa, const char *pb) {
    FILE *a = fopen(pa, "r"), *b = fopen(pb, "r");
    static char la[4096], lb[4096];
    unsigned long n = 0;
    if (!a || !b) { fprintf(stderr, "FAIL cannot open transcripts\n"); return 2; }
    for (;;) {
        char *ra = fgets(la, sizeof la, a), *rb = fgets(lb, sizeof lb, b);
        if (!ra && !rb) break;
        if (!ra || !rb || strcmp(la, lb) != 0) {
            printf("FAIL om eng diff: record %lu differs\n  om: %s  C:  %s", n, ra ? la : "(end)\n", rb ? lb : "(end)\n");
            return 1;
        }
        n++;
    }
    printf("PASS om eng diff: %lu records identical (om vs C)\n", n);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 4 && strcmp(argv[1], "--compare") == 0) return compare(argv[2], argv[3]);
    if (argc != 2) { fprintf(stderr, "usage: %s transcript | --compare om C\n", argv[0]); return 2; }
    g_out = fopen(argv[1], "w");
    if (!g_out) return 2;
    snprintf(g_meminfo, sizeof g_meminfo, "/tmp/om_eng_diff_meminfo.%ld", (long)getpid());
    if (omega_gpu_platform_linux_test_set_meminfo(g_meminfo) != 0) return 2;
    omega_gpu_engine_set_backend(&g_be);
    omega_gpu_engine_test_set_clock(eng_now);
    int i;
    for (i = 0; i < 3000; i++) trace_engine();
    for (i = 0; i < 2000; i++) trace_wait(0);
    for (i = 0; i < 2000; i++) trace_wait(1);
    for (i = 0; i < 1000; i++) trace_fclass();
    for (i = 0; i < 1000; i++) trace_release();
    for (i = 0; i < 1000; i++) trace_negotiate();
    for (i = 0; i < 1000; i++) trace_check();
    for (i = 0; i < 1000; i++) trace_buffer();
    for (i = 0; i < 2000; i++) trace_handles(i % 250 == 0);
    for (i = 0; i < 1000; i++) trace_reserve();
    for (i = 0; i < 1000; i++) trace_cached();
    for (i = 0; i < 1000; i++) trace_serving();
    for (i = 0; i < 8000; i++) trace_gates();
    trace_life_edges();
    unlink(g_meminfo);
    fclose(g_out);
    unsigned missing = 0;
    for (i = 0; i < (int)ENG_OP_COUNT; i++) {
        fprintf(stderr, "op %2d calls %" PRIu64 "\n", i, g_opn[i]);
        if (g_opn[i] == 0) missing++;
    }
    fprintf(stderr, "unknown-op calls %" PRIu64 "\n", g_opn[ENG_OP_COUNT]);
#if defined(ENG_EXPECT_C)
    if (g_calls != 0) { printf("FAIL C build reached the unit %" PRIu64 " times\n", g_calls); return 1; }
    printf("C build: %lu records, 0 unit calls\n", g_rec);
#else
    if (missing) { printf("FAIL om build: %u ops never reached\n", missing); return 1; }
    printf("om build: %lu records, %" PRIu64 " unit calls, all %u ops reached\n", g_rec, g_calls, ENG_OP_COUNT);
#endif
    return 0;
}
