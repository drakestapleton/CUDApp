/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* HOST_ONLY: omega_gpu_live_marker (nvrm/nvrm.c) on fake-RM memfd pages mapped through the native client, as
 * nvrm/submit_test.c maps them. Never nvrm_open(), never a GPU. Checks: the right Nvrm returns its own mapping,
 * offset and transport context, and the platform's marker poll (plat_wait_ge, a second generated runtime) reads the
 * word through that context; a foreign Nvrm has no owner for the address and its context refuses the token; NULL rm,
 * NULL outputs, an out-of-range and a quarantined address are refused with *ctx NULL. */
#include "nvrm.c"
#include <assert.h>

uint32_t omega2_ctx_plat_wait_ge(struct omega_native_context *context, uint64_t *result, uint64_t o_handle,
                                 uint64_t o_off, uint64_t o_want, uint64_t o_polls);

enum { PAGE = 0x10000 };

static void *map_page(Nvrm *rm, uintptr_t at) {
    int fd = memfd_create("cudapp-live-marker-fixture", MFD_CLOEXEC);
    assert(fd >= 0 && ftruncate(fd, PAGE) == 0);
    void *p = (void *)at;
    assert(nvrm_cpu_reserve(rm, at, PAGE) == p);
    assert(nvrm_cpu_map(rm, fd, PAGE, p) == p);
    rm->live[rm->live_count] = (NvrmLiveAlloc){.va = at, .size = PAGE, .cpu = p, .cpu_live = 1};
    nvrm_mapping_transfer(rm, &rm->live[rm->live_count]);
    assert(rm->live[rm->live_count].omega_mapping);
    rm->live_count++;
    return p;
}

int main(void) {
    static Nvrm rm, other;
    uint8_t *page = map_page(&rm, (uintptr_t)UINT64_C(0x7000200000));
    uint8_t *opage = map_page(&other, (uintptr_t)UINT64_C(0x7000300000));
    assert(rm.omega_context && other.omega_context && rm.omega_context != other.omega_context);

    /* The right context: token, offset and rm's own transport context. */
    uint64_t tk = 0, of = 0, r = 0;
    struct omega_native_context *ctx = NULL;
    assert(omega_gpu_live_marker(&rm, page + 0x40, &tk, &of, &ctx) == 0);
    assert(tk == rm.live[0].omega_mapping && of == 0x40 && ctx == rm.omega_context);
    /* The platform poll reads the marker through it: ready at the value, not ready one past (wrap-aware >=). */
    *(volatile uint32_t *)(page + 0x40) = 0x6a5a5a5au;
    assert(omega2_ctx_plat_wait_ge(ctx, &r, tk, of, 0x6a5a5a5au, 1) == 0 && (int64_t)r == 0);
    assert(omega2_ctx_plat_wait_ge(ctx, &r, tk, of, 0x6a5a5a5bu, 1) == 0 && (int64_t)r == -1);

    /* A foreign Nvrm owns no mapping at rm's address; its context refuses rm's token. */
    uint64_t ftk = 7, fof = 7;
    ctx = (struct omega_native_context *)&r;
    errno = 0;
    assert(omega_gpu_live_marker(&other, page + 0x40, &ftk, &fof, &ctx) == -1 && errno == EFAULT && ctx == NULL);
    r = 0;
    assert(omega2_ctx_plat_wait_ge(other.omega_context, &r, tk, 0x40, 0x6a5a5a5au, 1) != 0 || (int64_t)r < 0);
    /* Each Nvrm answers for its own page only. */
    assert(omega_gpu_live_marker(&other, opage + 8, &ftk, &fof, &ctx) == 0);
    assert(ftk == other.live[0].omega_mapping && fof == 8 && ctx == other.omega_context);

    /* Refusals, *ctx always NULL. */
    ctx = (struct omega_native_context *)&r;
    errno = 0;
    assert(omega_gpu_live_marker(NULL, page + 0x40, &tk, &of, &ctx) == -1 && errno == EINVAL && ctx == NULL);
    assert(omega_gpu_live_marker(&rm, page + 0x40, NULL, &of, &ctx) == -1 && errno == EINVAL);
    assert(omega_gpu_live_marker(&rm, page + 0x40, &tk, &of, NULL) == -1 && errno == EINVAL);
    ctx = (struct omega_native_context *)&r;
    assert(omega_gpu_live_marker(&rm, page + PAGE - 2, &tk, &of, &ctx) == -1 && errno == EFAULT && ctx == NULL);
    rm.live[0].quarantined = 1;
    ctx = (struct omega_native_context *)&r;
    assert(omega_gpu_live_marker(&rm, page + 0x40, &tk, &of, &ctx) == -1 && errno == EFAULT && ctx == NULL);
    rm.live[0].quarantined = 0;

    printf("NVRM LIVE MARKER PASS (own token+offset+context, cross-runtime poll, foreign/NULL/straddle/quarantined refused; HOST_ONLY)\n");
    return 0;
}
