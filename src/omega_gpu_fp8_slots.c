/* omega_gpu_fp8_slots.c: versioned replaceable FP8 expert slots (see the header). */
#include "omega_gpu_fp8_slots.h"
#include "omega_gpu_fp8_internal.h"
#include "omega_fp8_slot_table.h"
#include "omega_gpu_session.h"
#include "omega_gpu_fmt.h"
#include <stdlib.h>
#include <string.h>
#include "omega_api_shells.h"
#include "omega_api_views_rest.h"

struct OmegaGpuFp8Slots {
    uint32_t slots, n, k;
    OmegaGpuFp8Tensor *t;
    OmegaFp8SlotTable tab;
    pthread_mutex_t mu; /* bytes_loaded / load_ns */
    uint64_t bytes_loaded, load_ns;
};

const char *omega_gpu_fp8_slots_rc_name(int rc) {
    switch (rc) {
    case OMEGA_GPU_FP8_SLOTS_BUSY: return "BUSY";
    case OMEGA_GPU_FP8_SLOTS_STALE: return "STALE";
    default: return omega_gpu_fp8_rc_name(rc);
    }
}

/* The table's codes are the pool's codes (OK 0, BAD_ARGS -1, BUSY -6, STALE -7). */
_Static_assert(OMEGA_FP8_SLOT_BUSY == OMEGA_GPU_FP8_SLOTS_BUSY && OMEGA_FP8_SLOT_STALE == OMEGA_GPU_FP8_SLOTS_STALE &&
               (int)OMEGA_FP8_SLOT_BAD_ARGS == (int)OMEGA_GPU_FP8_BAD_ARGS, "slot table codes");

int omega_gpu_fp8_slots_alloc(uint32_t slots, uint32_t n, uint32_t k, OmegaGpuFp8Slots **out) {
    if (out) *out = NULL;
    if (!out || slots == 0) return OMEGA_GPU_FP8_BAD_ARGS;
    OmegaGpuFp8Slots *p = calloc(1, sizeof *p);
    if (!p) return OMEGA_GPU_FP8_BAD_ARGS;
    const uint32_t shape[3] = {slots, n, k};
    ar_words(p, 3, shape, 3);
    int rc = omega_gpu_fp8_tensor_alloc_uncached(slots, n, k, &p->t);
    if (rc != OMEGA_GPU_FP8_OK) { free(p); return rc; }
    if (omega_fp8_slot_table_init(&p->tab, slots) != 0 || pthread_mutex_init(&p->mu, NULL) != 0) {
        omega_gpu_fp8_tensor_free(p->t); omega_fp8_slot_table_fini(&p->tab); free(p); return OMEGA_GPU_FP8_BAD_ARGS;
    }
    *out = p;
    return OMEGA_GPU_FP8_OK;
}

int omega_gpu_fp8_slots_free(OmegaGpuFp8Slots *p) {
    if (!p) return OMEGA_GPU_FP8_OK;
    if (!omega_fp8_slot_table_idle(&p->tab)) return OMEGA_GPU_FP8_SLOTS_BUSY;
    omega_gpu_fp8_tensor_free(p->t);
    omega_fp8_slot_table_fini(&p->tab);
    pthread_mutex_destroy(&p->mu);
    free(p);
    return OMEGA_GPU_FP8_OK;
}

const OmegaGpuFp8Tensor *omega_gpu_fp8_slots_tensor(const OmegaGpuFp8Slots *p) { return p ? p->t : NULL; }

int omega_gpu_fp8_slots_load(OmegaGpuFp8Slots *p, uint32_t slot, uint64_t key, const uint8_t *w, const uint16_t *scale,
                             uint64_t *version) {
    if (version) *version = 0;
    if (!p || !w || !scale || !version) return OMEGA_GPU_FP8_BAD_ARGS;
    if (omega_gpu_session_is_blocked()) return OMEGA_GPU_FP8_CHIP_FAIL;
    const uint64_t t0 = omega_gpu_session_now_ns();
    int rc = omega_fp8_slot_begin_load(&p->tab, slot);
    if (rc != OMEGA_FP8_SLOT_OK) return rc;
    /* From here the slot is LOADING: invisible to pin, so the bytes below are never read half-written. */
#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
    const size_t wbytes = (size_t)p->n * p->k, scount = (size_t)(p->n / OMEGA_FP8_BLOCK) * (p->k / OMEGA_FP8_BLOCK);
#else
    uint64_t plan[32] = {3, p->slots, p->n, p->k};
    (void)ar_plan(plan);
    const size_t wbytes = plan[17], scount = plan[16];
#endif
    uint8_t *wd = NULL; float *sd = NULL;
    size_t first = 0;
    if (omega_gpu_fp8_tensor_cpu_view(p->t, slot, &wd, &sd) != 0) rc = OMEGA_GPU_FP8_BAD_ARGS;
    else if (omega_fp8_pack_scan_nan(w, wbytes, &first)) {
        char e[120]; omega_gpu_fmt(e, sizeof e, "fp8 slot load: e4m3 NaN code at byte %zu", first);
        omega_gpu_session_set_error(e);
        rc = OMEGA_GPU_FP8_NAN_WEIGHT;
    } else if (omega_fp8_pack_scales(scale, scount, sd, 0) != 0) {
        omega_gpu_session_set_error("fp8 slot load: NaN, inf or overflowing scale");
        rc = OMEGA_GPU_FP8_BAD_ARGS;
    } else {
#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
        memcpy(wd, w, wbytes);
#else
        ar_copy(wd, w, wbytes);
#endif
        if (omega_gpu_fp8_tensor_sync_to_gpu(p->t) != 0) rc = OMEGA_GPU_FP8_CHIP_FAIL;
    }
    int frc = omega_fp8_slot_finish_load(&p->tab, slot, key, rc == OMEGA_GPU_FP8_OK, version);
    if (rc == OMEGA_GPU_FP8_OK && frc != OMEGA_FP8_SLOT_OK) rc = frc;
    if (rc == OMEGA_GPU_FP8_OK) {
        pthread_mutex_lock(&p->mu);
        p->bytes_loaded += wbytes + scount * 4u;
        p->load_ns += omega_gpu_session_now_ns() - t0;
        pthread_mutex_unlock(&p->mu);
    }
    return rc;
}

int omega_gpu_fp8_slots_pin(OmegaGpuFp8Slots *p, uint32_t slot, uint64_t version) {
    return p ? omega_fp8_slot_pin(&p->tab, slot, version) : OMEGA_GPU_FP8_BAD_ARGS;
}
int omega_gpu_fp8_slots_unpin(OmegaGpuFp8Slots *p, uint32_t slot, uint64_t version) {
    return p ? omega_fp8_slot_unpin(&p->tab, slot, version) : OMEGA_GPU_FP8_BAD_ARGS;
}
int omega_gpu_fp8_slots_evict(OmegaGpuFp8Slots *p, uint32_t slot) {
    return p ? omega_fp8_slot_evict(&p->tab, slot) : OMEGA_GPU_FP8_BAD_ARGS;
}

int omega_gpu_fp8_slots_entry(OmegaGpuFp8Slots *p, uint32_t slot, uint64_t version, uint64_t *w_va, uint64_t *s_va) {
    if (!p || !w_va || !s_va) return OMEGA_GPU_FP8_BAD_ARGS;
    OmegaFp8SlotRec r;
    int rc = omega_fp8_slot_query(&p->tab, slot, &r);
    if (rc != OMEGA_FP8_SLOT_OK) return rc;
#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
    uint64_t gate[16] = { r.state, r.pins, r.key, r.version };
    int gate_rc = omega_api_slot(7, version, gate);
#else
    uint64_t result = 0;
    ar_check(omega_arsgate(&result, version, &r, 3));
    int gate_rc = (int)(int64_t)result;
#endif
    if (gate_rc == OMEGA_GPU_FP8_SLOTS_STALE) return gate_rc;
    if (gate_rc != 0) { omega_gpu_session_set_error("fp8 slot entry: slot not pinned (pin before taking its address)"); return gate_rc; }
    return omega_gpu_fp8_tensor_entry(p->t, slot, w_va, s_va);
}

int omega_gpu_fp8_slots_query(OmegaGpuFp8Slots *p, uint32_t slot, OmegaGpuFp8SlotInfo *out) {
    if (!p || !out) return OMEGA_GPU_FP8_BAD_ARGS;
    OmegaFp8SlotRec r;
    int rc = omega_fp8_slot_query(&p->tab, slot, &r);
    if (rc != OMEGA_FP8_SLOT_OK) return rc;
    _Static_assert(sizeof *out == sizeof r, "slot query view");
    _Static_assert(offsetof(OmegaGpuFp8SlotInfo, key) == offsetof(OmegaFp8SlotRec, key), "slot query key view");
    _Static_assert(offsetof(OmegaGpuFp8SlotInfo, version) == offsetof(OmegaFp8SlotRec, version), "slot query version view");
    ar_copy(out, &r, sizeof r);
    return OMEGA_GPU_FP8_OK;
}

void omega_gpu_fp8_slots_stats(OmegaGpuFp8Slots *p, OmegaGpuFp8SlotsStats *out) {
    if (!out) return;
    ar_words(out, sizeof *out / sizeof(uint32_t), NULL, 0);
    if (!p) return;
    OmegaFp8SlotStats s;
    omega_fp8_slot_table_stats(&p->tab, &s);
#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
    out->slots = p->slots; out->n = p->n; out->k = p->k;
    out->device_bytes = (uint64_t)p->slots * ((uint64_t)p->n * p->k + (uint64_t)(p->n / OMEGA_FP8_BLOCK) * (p->k / OMEGA_FP8_BLOCK) * 4u);
    out->loads = s.loads; out->load_failures = s.load_failures; out->busy_refusals = s.busy_refusals;
    out->stale_refusals = s.stale_refusals; out->evictions = s.evictions; out->pins = s.pins;
    out->pinned_now = s.pinned_now; out->pinned_high = s.pinned_high;
#else
    _Static_assert(sizeof *out == 12 * sizeof(uint64_t), "slot statistics descriptor extent");
    _Static_assert(offsetof(OmegaGpuFp8SlotsStats, device_bytes) == 16, "slot statistics device bytes");
    _Static_assert(offsetof(OmegaGpuFp8SlotsStats, loads) == 24, "slot statistics counters");
    _Static_assert(offsetof(OmegaGpuFp8SlotsStats, pinned_now) == 72, "slot statistics pin counts");
    _Static_assert(offsetof(OmegaGpuFp8SlotsStats, bytes_loaded) == 80, "slot statistics load counters");
    const uint64_t cfg[8] = {p->slots, p->n, p->k};
    uint64_t result = 0;
    ar_check(omega_arsstats(&result, cfg, 8, &s, 7, out, 12));
#endif
    pthread_mutex_lock(&p->mu);
    ar_copy(&out->bytes_loaded, &p->bytes_loaded, sizeof out->bytes_loaded + sizeof out->load_ns);
    pthread_mutex_unlock(&p->mu);
}

int omega_gpu_fp8_slots_gemv_grouped_f32(OmegaGpuFp8Slots *p, uint32_t groups, const uint32_t *slots, const uint64_t *versions,
                                         const float *x, uint32_t x_rows, float *y, OmegaGpuFp8Info *info) {
    if (!p || !slots || !versions || groups == 0) return OMEGA_GPU_FP8_BAD_ARGS;
    if (groups > OMEGA_GPU_FP8_MAX_GROUPS) return OMEGA_GPU_FP8_TOO_LARGE;
    uint32_t pinned = 0;
    int rc = OMEGA_GPU_FP8_OK;
    for (; pinned < groups; pinned++) {
        rc = omega_fp8_slot_pin(&p->tab, slots[pinned], versions[pinned]);
        if (rc != OMEGA_FP8_SLOT_OK) break;
    }
    if (rc == OMEGA_GPU_FP8_OK) rc = omega_gpu_fp8_gemv_grouped_f32(p->t, groups, slots, x, x_rows, y, info);
    /* The call returned after the launch's completion marker (or failed): no reader of these slots remains. */
    for (uint32_t i = 0; i < pinned; i++) (void)omega_fp8_slot_unpin(&p->tab, slots[i], versions[i]);
    return rc;
}

void omega_gpu_fp8_slots_test_set_mutant(OmegaGpuFp8Slots *p, int mutant) {
    if (p) omega_fp8_slot_table_set_mutant(&p->tab, mutant);
}
