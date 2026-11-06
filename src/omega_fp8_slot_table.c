/* omega_fp8_slot_table.c: host-only slot state machine (see the header). */
#include "omega_fp8_slot_table.h"
#include <stdlib.h>
#include "omega_api_shells.h"
#include "omega_api_views_rest.h"
#include <string.h>

int omega_fp8_slot_table_init(OmegaFp8SlotTable *t, uint32_t slots) {
    if (!t || slots == 0) return OMEGA_FP8_SLOT_BAD_ARGS;
    memset(t, 0, sizeof *t);
    t->rec = calloc(slots, sizeof *t->rec);
    if (!t->rec) return OMEGA_FP8_SLOT_BAD_ARGS;
    if (pthread_mutex_init(&t->mu, NULL) != 0) { free(t->rec); t->rec = NULL; return OMEGA_FP8_SLOT_BAD_ARGS; }
    t->slots = slots;
    t->next_version = 1;
    return OMEGA_FP8_SLOT_OK;
}

void omega_fp8_slot_table_fini(OmegaFp8SlotTable *t) {
    if (!t || !t->rec) return;
    pthread_mutex_destroy(&t->mu);
    free(t->rec);
    t->rec = NULL;
    t->slots = 0;
}

#define LOCKED(t, body) do { pthread_mutex_lock(&(t)->mu); body; pthread_mutex_unlock(&(t)->mu); } while (0)

#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
static void pack_rec(const OmegaFp8SlotTable *t, const OmegaFp8SlotRec *r, int ok, uint64_t rec[16]) {
    memset(rec, 0, sizeof(*rec) * 16);
    rec[0] = r->state; rec[1] = r->pins; rec[2] = r->key; rec[3] = r->version;
    rec[4] = t->next_version; rec[5] = (uint64_t)t->mutant; rec[6] = (uint64_t)(ok != 0);
    rec[8] = t->stats.loads; rec[9] = t->stats.load_failures; rec[10] = t->stats.busy_refusals;
    rec[11] = t->stats.stale_refusals; rec[12] = t->stats.evictions; rec[13] = t->stats.pins;
    rec[14] = t->stats.pinned_now; rec[15] = t->stats.pinned_high;
}
static void unpack_rec(OmegaFp8SlotTable *t, OmegaFp8SlotRec *r, const uint64_t rec[16]) {
    r->state = (uint32_t)rec[0]; r->pins = (uint32_t)rec[1]; r->key = rec[2]; r->version = rec[3];
    t->next_version = rec[4];
    t->stats.loads = rec[8]; t->stats.load_failures = rec[9]; t->stats.busy_refusals = rec[10];
    t->stats.stale_refusals = rec[11]; t->stats.evictions = rec[12]; t->stats.pins = rec[13];
    t->stats.pinned_now = (uint32_t)rec[14]; t->stats.pinned_high = (uint32_t)rec[15];
}
static int step_slot(OmegaFp8SlotTable *t, uint32_t slot, int op, uint64_t aux, int ok) {
    uint64_t rec[16];
    pack_rec(t, &t->rec[slot], ok, rec);
    int rc = omega_api_slot((uint64_t)op, aux, rec);
    unpack_rec(t, &t->rec[slot], rec);
    return rc;
}
#else
_Static_assert(sizeof(OmegaFp8SlotRec) == 3 * sizeof(uint64_t), "slot record view");
_Static_assert(offsetof(OmegaFp8SlotRec, key) == 8, "slot key view");
_Static_assert(offsetof(OmegaFp8SlotRec, version) == 16, "slot version view");
_Static_assert(sizeof(OmegaFp8SlotStats) == 7 * sizeof(uint64_t), "slot statistics view");
_Static_assert(offsetof(OmegaFp8SlotStats, pinned_now) == 48, "slot pin statistics view");
static int step_slot(OmegaFp8SlotTable *t, uint32_t slot, int op, uint64_t aux, int ok) {
    uint64_t cfg[8] = {t->next_version, (uint64_t)t->mutant, (uint64_t)ok, (uint64_t)op, aux};
    uint64_t rc = 0;
    ar_check(omega_arslot(&rc, cfg, 8, &t->rec[slot], 3, &t->stats, 7));
    t->next_version = cfg[0];
    return (int)(int64_t)rc;
}
#endif

int omega_fp8_slot_begin_load(OmegaFp8SlotTable *t, uint32_t slot) {
    if (!t || !t->rec || slot >= t->slots) return OMEGA_FP8_SLOT_BAD_ARGS;
    int rc = OMEGA_FP8_SLOT_OK;
    LOCKED(t, { rc = step_slot(t, slot, 1, 0, 0); });
    return rc;
}

int omega_fp8_slot_finish_load(OmegaFp8SlotTable *t, uint32_t slot, uint64_t key, int ok, uint64_t *version) {
    if (version) *version = 0;
    if (!t || !t->rec || slot >= t->slots) return OMEGA_FP8_SLOT_BAD_ARGS;
    int rc = OMEGA_FP8_SLOT_OK;
    LOCKED(t, { rc = step_slot(t, slot, 2, key, ok); if (version && rc == 0 && ok) *version = t->rec[slot].version; });
    return rc;
}

int omega_fp8_slot_pin(OmegaFp8SlotTable *t, uint32_t slot, uint64_t version) {
    if (!t || !t->rec || slot >= t->slots) return OMEGA_FP8_SLOT_BAD_ARGS;
    int rc = OMEGA_FP8_SLOT_OK;
    LOCKED(t, { rc = step_slot(t, slot, 3, version, 0); });
    return rc;
}

int omega_fp8_slot_unpin(OmegaFp8SlotTable *t, uint32_t slot, uint64_t version) {
    if (!t || !t->rec || slot >= t->slots) return OMEGA_FP8_SLOT_BAD_ARGS;
    int rc = OMEGA_FP8_SLOT_OK;
    LOCKED(t, { rc = step_slot(t, slot, 4, version, 0); });
    return rc;
}

int omega_fp8_slot_evict(OmegaFp8SlotTable *t, uint32_t slot) {
    if (!t || !t->rec || slot >= t->slots) return OMEGA_FP8_SLOT_BAD_ARGS;
    int rc = OMEGA_FP8_SLOT_OK;
    LOCKED(t, { rc = step_slot(t, slot, 5, 0, 0); });
    return rc;
}

int omega_fp8_slot_query(OmegaFp8SlotTable *t, uint32_t slot, OmegaFp8SlotRec *out) {
    if (!t || !t->rec || slot >= t->slots || !out) return OMEGA_FP8_SLOT_BAD_ARGS;
    LOCKED(t, { ar_words(out, sizeof *out / sizeof(uint32_t), (const uint32_t *)&t->rec[slot], sizeof *out / sizeof(uint32_t)); });
    return OMEGA_FP8_SLOT_OK;
}

int omega_fp8_slot_table_idle(OmegaFp8SlotTable *t) {
    if (!t || !t->rec) return 1;
    int idle = 1;
    LOCKED(t, {
#if CUDAPP_MIGRATION_PENDING_API_VIEWS_REST
        for (uint32_t i = 0; i < t->slots; i++)
            if (step_slot(t, i, 6, 0, 0) == 0) { idle = 0; break; }
#else
        uint64_t result = 0;
        ar_check(omega_aridle(&result, t->rec, (uint64_t)t->slots * 6));
        idle = result != 0;
#endif
    });
    return idle;
}
/* slot decisions live in om/api/apish_slot.om */

void omega_fp8_slot_table_stats(OmegaFp8SlotTable *t, OmegaFp8SlotStats *out) {
    if (!out) return;
    ar_words(out, sizeof *out / sizeof(uint32_t), NULL, 0);
    if (!t || !t->rec) return;
    LOCKED(t, { ar_words(out, sizeof *out / sizeof(uint32_t), (const uint32_t *)&t->stats, sizeof *out / sizeof(uint32_t)); });
}

void omega_fp8_slot_table_set_mutant(OmegaFp8SlotTable *t, int mutant) {
    if (!t || !t->rec) return;
    LOCKED(t, { t->mutant = mutant; });
}
