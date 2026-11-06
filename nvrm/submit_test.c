/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* HOST_ONLY fake device for the GPFIFO submit stores (C4 streams, CUDApp#115).
 * Two memfd pages stand in for the GPFIFO+USERD allocation and the usermode
 * doorbell page; both are mapped through the native client exactly as
 * nvrm_alloc_gpu_uncached / the usermode map do, so the Omega path stores
 * through its owned handles. Built twice: CUDAPP_MIGRATION_PENDING_C_SUBMIT=0
 * (Omega, native/streams/submit.om) and =1 (C oracle). Each build writes the
 * final bytes of both pages plus the put counters to argv[1]; the make rule
 * compares the two images byte for byte. Never nvrm_open(), never a GPU. */
#include "nvrm.c"
#include <assert.h>

enum { PAGE = 0x10000, USERD = 0x8000 };

static void map_fifo(Nvrm *rm, int fd, void *at, NvrmMem *mem, uint32_t slot) {
    assert(nvrm_cpu_reserve(rm, (uintptr_t)at, PAGE) == at);
    assert(nvrm_cpu_map(rm, fd, PAGE, at) == at);
    rm->live[slot] = (NvrmLiveAlloc){.va = (uintptr_t)at, .size = PAGE, .cpu = at, .cpu_live = 1};
    nvrm_mapping_transfer(rm, &rm->live[slot]);
    rm->live_count = slot + 1;
    assert(NVRM_C_SUBMIT || rm->live[slot].omega_mapping);
    *mem = (NvrmMem){.va = (uintptr_t)at, .size = PAGE, .cpu = at};
}

static int fresh(size_t n) {
    int fd = memfd_create("cudapp-streams-submit-fixture", MFD_CLOEXEC);
    assert(fd >= 0 && ftruncate(fd, (off_t)n) == 0);
    return fd;
}

int main(int argc, char **argv) {
    assert(argc == 2);
    int ffd = fresh(PAGE), cfd = fresh(PAGE), ufd = fresh(PAGE);
    static Nvrm rm;
    map_fifo(&rm, ffd, (void *)(uintptr_t)UINT64_C(0x7000000000), &rm.fifo, 0);
    rm.usermode_cpu = nvrm_cpu_map(&rm, ufd, PAGE, NULL);
    assert(rm.usermode_cpu != MAP_FAILED);
    rm.doorbell = (volatile uint32_t *)((uint8_t *)rm.usermode_cpu + NVC361_NOTIFY_CHANNEL_PENDING);
    rm.gpfifo = 1; rm.entries = 1024; rm.userd_off = USERD; rm.token = 0x1234abcd;
    rm.n_extra = 1;
    NvrmChan *c = &rm.extra[0];
    map_fifo(&rm, cfd, (void *)(uintptr_t)UINT64_C(0x7000100000), &c->fifo, 1);
    /* Non power of two ring and a uint32 put about to wrap: GPPut must be
     * ((put + 1) mod 2^32) mod entries, as the C counter arithmetic gives. */
    c->gpfifo = 1; c->entries = 6; c->userd_off = USERD; c->token = 0x5678;
    c->put = c->retired = UINT32_MAX - 2;
    NvrmMem pb = {.va = UINT64_C(0x123456789ab3), .size = 0x100000}; /* low bits set: the GP entry masks them */
    volatile uint32_t *chan_gpput = (volatile uint32_t *)((uint8_t *)c->fifo.cpu + USERD + offsetof(Nvc96fControl, GPPut));

    for (uint32_t i = 0; i < 2000; i++) {
        uint32_t off = (i * 52u) & 0xffffcu, n = 1u + (i * 7919u) % 0x1fffu;
        if (i == 7) n = 0x1fffff, off = 0, pb.size = 0x800000;
        assert(nvrm_submit(&rm, &pb, off, n) == 0 && !rm.faulted);
        nvrm_retire(&rm, rm.put);
        assert(nvrm_chan_submit(&rm, 1, &pb, off, n) == 0);
        assert(*chan_gpput == c->put % c->entries); /* every step, including the 2^32 wrap */
        nvrm_chan_retire(&rm, 1, c->put);
        pb.size = 0x100000;
    }
    /* Refusals before any store, both builds: zero words, unaligned span, full ring. */
    assert(nvrm_enqueue(&rm, &pb, 0, 0) == -1);
    rm.faulted = 0;
    assert(nvrm_chan_submit(&rm, 1, &pb, 2, 1) == -1);
    rm.faulted = 0;
    assert(nvrm_chan_submit(&rm, 1, &pb, 0, 1) == 0 && nvrm_chan_submit(&rm, 1, &pb, 0, 2) == 0);
    assert(nvrm_chan_submit(&rm, 1, &pb, 0, 3) == 0 && nvrm_chan_submit(&rm, 1, &pb, 0, 4) == 0);
    assert(nvrm_chan_submit(&rm, 1, &pb, 0, 5) == 0);
    assert(nvrm_chan_submit(&rm, 1, &pb, 0, 6) == -1);
    rm.faulted = 0;

#if !NVRM_C_SUBMIT
    /* Omega argument refusals: no store issued, the page is unchanged. */
    unsigned char before[PAGE], after[PAGE];
    memcpy(before, rm.fifo.cpu, PAGE);
    uint64_t r = 0, h = rm.live[0].omega_mapping;
    assert(omega2_ctx_streams_gp_publish(rm.omega_context, &r, h, 1, 0, USERD, 0, 1) == 0 && (int64_t)r == -EINVAL);
    assert(omega2_ctx_streams_gp_publish(rm.omega_context, &r, h, 8, 0, USERD + 2, 0, 1) == 0 && (int64_t)r == -EINVAL);
    assert(omega2_ctx_streams_gp_publish(rm.omega_context, &r, h, 8, 0, PAGE, 0, 1) == 0 && (int64_t)r == -EINVAL);
    assert(omega2_ctx_streams_gp_publish(rm.omega_context, &r, h, 8, 0, USERD, 0, 0) == 0 && (int64_t)r == -EINVAL);
    assert(omega2_ctx_streams_gp_publish(rm.omega_context, &r, h, 8, 0, USERD, 0, 0x200000) == 0 && (int64_t)r == -EINVAL);
    assert(omega2_ctx_streams_ring(rm.omega_context, &r, rm.omega_usermode_mapping, 0x91, 1) == 0 && (int64_t)r == -EINVAL);
    assert(omega2_ctx_streams_ring(rm.omega_context, &r, rm.omega_usermode_mapping, 0x90, UINT64_C(0x100000000)) == 0 && (int64_t)r == -EINVAL);
    /* A foreign handle is refused by the context, never dereferenced. */
    assert(omega2_ctx_streams_gp_publish(rm.omega_context, &r, h ^ 0x40000, 8, 0, USERD, 0, 1) != 0 || (int64_t)r < 0);
    /* Without an owned handle the enqueue refuses rather than storing through the pointer. */
    rm.live[0].omega_mapping = 0;
    uint32_t put_before = rm.put;
    assert(nvrm_enqueue(&rm, &pb, 0, 1) == -1 && rm.put == put_before);
    rm.live[0].omega_mapping = h; rm.faulted = 0;
    memcpy(after, rm.fifo.cpu, PAGE);
    assert(!memcmp(before, after, PAGE));
#endif

    FILE *out = fopen(argv[1], "wb");
    assert(out);
    assert(fwrite(rm.fifo.cpu, 1, PAGE, out) == PAGE);
    assert(fwrite(c->fifo.cpu, 1, PAGE, out) == PAGE);
    assert(fwrite(rm.usermode_cpu, 1, PAGE, out) == PAGE);
    assert(fwrite(&rm.put, sizeof rm.put, 1, out) == 1 && fwrite(&c->put, sizeof c->put, 1, out) == 1);
    assert(fclose(out) == 0);
    /* Spot checks independent of the other build: last GP entry and GPPut. */
    uint32_t gpput = *(volatile uint32_t *)((uint8_t *)c->fifo.cpu + USERD + offsetof(Nvc96fControl, GPPut));
    assert(gpput == c->put % c->entries);
    assert(*rm.doorbell == c->token);
    printf("STREAMS SUBMIT FIXTURE %s: put %u/%u, chan GPPut %u, doorbell 0x%x (HOST_ONLY, live tier NOT_RUN)\n",
           NVRM_C_SUBMIT ? "C" : "OMEGA", rm.put, c->put, gpput, *rm.doorbell);
    return 0;
}
