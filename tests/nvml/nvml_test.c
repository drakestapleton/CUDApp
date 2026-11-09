/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host test of the CUDA++ NVML subset (compat/nvml).
 * Default: Omega RM fake (tests/nvml/rmfake.om). usage: nvml_test ERRSTR_ORACLE.tsv
 * Live (no fake, no libnvidia-ml): compile with -DCUDAPP_NVML_LIVE and link
 * compat/nvml/cudapp_nvml.c, src/cudapp_om_nvml_step.c, src/cudapp_om_nvml_errstr.c
 * and the #281 transport src/cudapp_om_nvrm_transport.c. usage: nvml-live --live
 * Chip run belongs in an exclusive spark-window. This file does not open the GPU by itself
 * except through nvmlInit_v2 when --live is passed. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nvml.h"
#ifndef CUDAPP_NVML_LIVE
#include "transport.h"
#endif

#ifdef CUDAPP_NVML_LIVE

/* Read-only probe. One line per field so the nvidia-smi oracle can join on the key.
 * Negative control: handle index == device count must be NVML_ERROR_INVALID_ARGUMENT (2).
 * Exit 0 only when init, every query, the negative control and shutdown all succeed.
 * Exit 3 when init does not succeed (driver not available): not a chip pass. */
static int live_fail(const char *what, nvmlReturn_t got, nvmlReturn_t want) {
    fprintf(stderr, "FAIL %s: rc %d, want %d\n", what, (int)got, (int)want);
    nvmlShutdown();
    return 1;
}

static int live_probe(void) {
    unsigned count = 0;
    nvmlReturn_t rc = nvmlInit_v2();
    if (rc != NVML_SUCCESS) {
        printf("live init rc=%d\n", (int)rc);
        fprintf(stderr, "NOT_RUN: nvmlInit_v2 rc %d (no chip pass)\n", (int)rc);
        return 3;
    }
    rc = nvmlDeviceGetCount_v2(&count);
    if (rc != NVML_SUCCESS) return live_fail("count", rc, NVML_SUCCESS);
    printf("live count=%u\n", count);

    for (unsigned i = 0; i < count; i++) {
        nvmlDevice_t dev = NULL;
        char name[NVML_DEVICE_NAME_V2_BUFFER_SIZE];
        char uuid[NVML_DEVICE_UUID_V2_BUFFER_SIZE];
        unsigned temp = 0;
        unsigned procs_n = 0;
        nvmlMemory_t mem;
        nvmlProcessInfo_t infos[64];
        char what[64];

        snprintf(what, sizeof what, "handle %u", i);
        rc = nvmlDeviceGetHandleByIndex_v2(i, &dev);
        if (rc != NVML_SUCCESS) return live_fail(what, rc, NVML_SUCCESS);
        snprintf(what, sizeof what, "name %u", i);
        rc = nvmlDeviceGetName(dev, name, sizeof name);
        if (rc != NVML_SUCCESS) return live_fail(what, rc, NVML_SUCCESS);
        snprintf(what, sizeof what, "uuid %u", i);
        rc = nvmlDeviceGetUUID(dev, uuid, sizeof uuid);
        if (rc != NVML_SUCCESS) return live_fail(what, rc, NVML_SUCCESS);
        snprintf(what, sizeof what, "temperature %u", i);
        rc = nvmlDeviceGetTemperature(dev, NVML_TEMPERATURE_GPU, &temp);
        if (rc != NVML_SUCCESS) return live_fail(what, rc, NVML_SUCCESS);
        snprintf(what, sizeof what, "memory %u", i);
        rc = nvmlDeviceGetMemoryInfo(dev, &mem);
        if (rc != NVML_SUCCESS) return live_fail(what, rc, NVML_SUCCESS);
        procs_n = 64;
        snprintf(what, sizeof what, "procs %u", i);
        rc = nvmlDeviceGetComputeRunningProcesses_v3(dev, &procs_n, infos);
        if (rc != NVML_SUCCESS) return live_fail(what, rc, NVML_SUCCESS);

        printf("live gpu%u name=%s\n", i, name);
        printf("live gpu%u uuid=%s\n", i, uuid);
        printf("live gpu%u temperature_c=%u\n", i, temp);
        printf("live gpu%u memory_total_bytes=%llu\n", i, (unsigned long long)mem.total);
        printf("live gpu%u memory_used_bytes=%llu\n", i, (unsigned long long)mem.used);
        printf("live gpu%u procs=%u\n", i, procs_n);
        for (unsigned p = 0; p < procs_n; p++) {
            printf("proc pid=%u used_bytes=%llu\n", infos[p].pid,
                   (unsigned long long)infos[p].usedGpuMemory);
        }
    }

    /* Negative control: one past the last real index. A passing probe that also accepts
     * this index has not established the bound. */
    {
        nvmlDevice_t bad = NULL;
        rc = nvmlDeviceGetHandleByIndex_v2(count, &bad);
        printf("live negative index=%u rc=%d want=%d\n", count, (int)rc,
               (int)NVML_ERROR_INVALID_ARGUMENT);
        if (rc != NVML_ERROR_INVALID_ARGUMENT) {
            fprintf(stderr, "FAIL negative control: index %u rc %d, want %d\n", count, (int)rc,
                    (int)NVML_ERROR_INVALID_ARGUMENT);
            nvmlShutdown();
            return 1;
        }
    }
    rc = nvmlShutdown();
    if (rc != NVML_SUCCESS) return live_fail("shutdown", rc, NVML_SUCCESS);
    printf("live shutdown rc=0\n");
    printf("CUDAPP-NVML LIVE QUERIES DONE\n");
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2 || strcmp(argv[1], "--live") != 0) {
        fprintf(stderr, "usage: nvml-live --live\n");
        return 2;
    }
    return live_probe();
}

#else
static unsigned checks, fails;

#define CHECK(cond, ...)                                                                           \
    do {                                                                                           \
        checks++;                                                                                  \
        if (!(cond)) {                                                                             \
            fails++;                                                                               \
            printf("FAIL line %d: ", __LINE__);                                                    \
            printf(__VA_ARGS__);                                                                   \
            printf("\n");                                                                          \
        }                                                                                          \
    } while (0)

#define RC(call, want, what)                                                                       \
    do {                                                                                           \
        nvmlReturn_t got_ = (call);                                                                \
        CHECK(got_ == (want), "%s: rc %d, want %d", what, (int)got_, (int)(want));                 \
    } while (0)

static const char *const UUID0 = "GPU-fb8e53f0-3207-e89b-d02f-acdfe2695b59";
static const char *const UUID1 = "GPU-fb8e53f0-3207-e89b-d02f-acdfe2695b5a";

/* Every fd closed, every RM object freed, no request outside the RM contract. */
static void clean(const char *what) {
    CHECK(fake_get(14) == 0, "%s: open fd mask %llx", what, (unsigned long long)fake_get(14));
    CHECK(fake_get(15) == 0, "%s: %llu live RM objects", what, (unsigned long long)fake_get(15));
    CHECK(fake_get(16) == 0, "%s: root client still allocated", what);
    CHECK(fake_get(8) == fake_get(9), "%s: %llu opens, %llu closes", what,
          (unsigned long long)fake_get(8), (unsigned long long)fake_get(9));
    CHECK(fake_get(13) == 0, "%s: %llu RM contract violations", what,
          (unsigned long long)fake_get(13));
}

static void errstrings(const char *path) {
    FILE *f = fopen(path, "r");
    CHECK(f != NULL, "open %s", path);
    if (f == NULL) return;
    char line[256];
    unsigned rows = 0;
    while (fgets(line, sizeof line, f)) {
        char *tab = strchr(line, '\t');
        if (tab == NULL) continue;
        *tab = 0;
        char *want = tab + 1;
        want[strcspn(want, "\n")] = 0;
        long long code = strtoll(line, NULL, 10);
        const char *got = nvmlErrorString((nvmlReturn_t)(int)(unsigned)code);
        CHECK(got != NULL && strcmp(got, want) == 0, "nvmlErrorString(%lld) = \"%s\", want \"%s\"",
              code, got ? got : "(null)", want);
        rows++;
    }
    fclose(f);
    CHECK(rows == 37, "oracle rows %u, want 37", rows);
    CHECK(nvmlErrorString(NVML_SUCCESS) == nvmlErrorString(NVML_SUCCESS), "stable string pointer");
    CHECK(nvmlErrorString((nvmlReturn_t)31) == nvmlErrorString((nvmlReturn_t)999),
          "one Unknown Error string");
}

static void healthy(void) {
    unsigned n = 77, t = 0;
    nvmlDevice_t d = NULL, d1 = NULL;
    char buf[128];
    fake_reset(0);

    RC(nvmlDeviceGetCount_v2(&n), NVML_ERROR_UNINITIALIZED, "count before init");
    RC(nvmlDeviceGetHandleByIndex_v2(0, &d), NVML_ERROR_UNINITIALIZED, "handle before init");
    RC(nvmlShutdown(), NVML_ERROR_UNINITIALIZED, "shutdown before init");
    CHECK(fake_get(8) == 0, "no open before init");

    RC(nvmlInit_v2(), NVML_SUCCESS, "init");
    RC(nvmlDeviceGetCount_v2(&n), NVML_SUCCESS, "count");
    CHECK(n == 1, "count %u, want 1", n);
    RC(nvmlDeviceGetCount_v2(NULL), NVML_ERROR_INVALID_ARGUMENT, "count NULL");

    RC(nvmlDeviceGetHandleByIndex_v2(0, &d), NVML_SUCCESS, "handle 0");
    CHECK(d != NULL, "handle 0 non-NULL");
    RC(nvmlDeviceGetHandleByIndex_v2(1, &d1), NVML_ERROR_INVALID_ARGUMENT, "handle 1");
    RC(nvmlDeviceGetHandleByIndex_v2(0, NULL), NVML_ERROR_INVALID_ARGUMENT, "handle NULL");

    memset(buf, 'x', sizeof buf);
    RC(nvmlDeviceGetName(d, buf, NVML_DEVICE_NAME_V2_BUFFER_SIZE), NVML_SUCCESS, "name 96");
    CHECK(strcmp(buf, "NVIDIA GB10") == 0, "name \"%.64s\"", buf);
    RC(nvmlDeviceGetName(d, buf, 11), NVML_ERROR_INSUFFICIENT_SIZE, "name length 11");
    memset(buf, 'x', sizeof buf);
    RC(nvmlDeviceGetName(d, buf, 12), NVML_SUCCESS, "name length 12");
    CHECK(strcmp(buf, "NVIDIA GB10") == 0, "name 12 \"%.64s\"", buf);
    RC(nvmlDeviceGetName(NULL, buf, 64), NVML_ERROR_INVALID_ARGUMENT, "name NULL device");
    RC(nvmlDeviceGetName((nvmlDevice_t)(void *)buf, buf, 64), NVML_ERROR_INVALID_ARGUMENT,
       "name foreign device");
    RC(nvmlDeviceGetName(d, NULL, 64), NVML_ERROR_INVALID_ARGUMENT, "name NULL buffer");

    memset(buf, 'x', sizeof buf);
    RC(nvmlDeviceGetUUID(d, buf, NVML_DEVICE_UUID_V2_BUFFER_SIZE), NVML_SUCCESS, "uuid 96");
    CHECK(strcmp(buf, UUID0) == 0, "uuid \"%.96s\"", buf);
    RC(nvmlDeviceGetUUID(d, buf, 40), NVML_ERROR_INSUFFICIENT_SIZE, "uuid length 40");
    memset(buf, 'x', sizeof buf);
    RC(nvmlDeviceGetUUID(d, buf, 41), NVML_SUCCESS, "uuid length 41");
    CHECK(strcmp(buf, UUID0) == 0, "uuid 41 \"%.96s\"", buf);
    RC(nvmlDeviceGetUUID(d, NULL, 80), NVML_ERROR_INVALID_ARGUMENT, "uuid NULL buffer");

    RC(nvmlDeviceGetTemperature(d, NVML_TEMPERATURE_GPU, &t), NVML_SUCCESS, "temperature");
    CHECK(t == 40, "temperature %u, want 40", t);
    RC(nvmlDeviceGetTemperature(d, NVML_TEMPERATURE_COUNT, &t), NVML_ERROR_INVALID_ARGUMENT,
       "temperature sensor 1");
    RC(nvmlDeviceGetTemperature(d, NVML_TEMPERATURE_GPU, NULL), NVML_ERROR_INVALID_ARGUMENT,
       "temperature NULL");

    nvmlMemory_t m;
    memset(&m, 0xa5, sizeof m);
    RC(nvmlDeviceGetMemoryInfo(d, &m), NVML_SUCCESS, "memory");
    CHECK(m.total == 131072000000ull && m.free == 102400000000ull && m.used == 28672000000ull,
          "memory %llu/%llu/%llu", (unsigned long long)m.total, (unsigned long long)m.free,
          (unsigned long long)m.used);
    RC(nvmlDeviceGetMemoryInfo(d, NULL), NVML_ERROR_INVALID_ARGUMENT, "memory NULL");

    nvmlUtilization_t u = {77, 77};
    RC(nvmlDeviceGetUtilizationRates(d, &u), NVML_SUCCESS, "utilization");
    CHECK(u.gpu == 42 && u.memory == 13, "utilization %u/%u, want 42/13", u.gpu, u.memory);
    RC(nvmlDeviceGetUtilizationRates(d, NULL), NVML_ERROR_INVALID_ARGUMENT, "utilization NULL");

    nvmlProcessInfo_t p[5];
    n = 0;
    RC(nvmlDeviceGetComputeRunningProcesses_v3(d, &n, NULL), NVML_ERROR_INSUFFICIENT_SIZE,
       "procs count 0");
    CHECK(n == 2, "procs needed %u, want 2", n);
    n = 1;
    RC(nvmlDeviceGetComputeRunningProcesses_v3(d, &n, p), NVML_ERROR_INSUFFICIENT_SIZE,
       "procs count 1");
    CHECK(n == 2, "procs needed %u, want 2", n);
    n = 2;
    RC(nvmlDeviceGetComputeRunningProcesses_v3(d, &n, NULL), NVML_ERROR_INVALID_ARGUMENT,
       "procs NULL infos");
    RC(nvmlDeviceGetComputeRunningProcesses_v3(d, NULL, p), NVML_ERROR_INVALID_ARGUMENT,
       "procs NULL count");
    memset(p, 0, sizeof p);
    n = 5;
    RC(nvmlDeviceGetComputeRunningProcesses_v3(d, &n, p), NVML_SUCCESS, "procs count 5");
    CHECK(n == 2, "procs %u, want 2", n);
    CHECK(p[0].pid == 68451 && p[0].usedGpuMemory == 351272960ull, "proc 0 %u %llu", p[0].pid,
          (unsigned long long)p[0].usedGpuMemory);
    CHECK(p[1].pid == 4242 && p[1].usedGpuMemory == 278003712ull, "proc 1 %u %llu", p[1].pid,
          (unsigned long long)p[1].usedGpuMemory);
    CHECK(p[0].gpuInstanceId == 0xFFFFFFFFu && p[0].computeInstanceId == 0xFFFFFFFFu &&
              p[1].gpuInstanceId == 0xFFFFFFFFu && p[1].computeInstanceId == 0xFFFFFFFFu,
          "proc instance ids not NVML_VALUE_NOT_AVAILABLE");
    CHECK(p[2].pid == 0, "proc 2 written");

    RC(nvmlShutdown(), NVML_SUCCESS, "shutdown");
    RC(nvmlDeviceGetCount_v2(&n), NVML_ERROR_UNINITIALIZED, "count after shutdown");
    RC(nvmlDeviceGetName(d, buf, 64), NVML_ERROR_UNINITIALIZED, "name after shutdown");
    RC(nvmlShutdown(), NVML_ERROR_UNINITIALIZED, "second shutdown");
    clean("healthy");
}

/* nvmlInit/nvmlShutdown keep a reference count; only the last shutdown tears down (nvml.h). */
static void refcount(void) {
    unsigned n = 0;
    fake_reset(0);
    RC(nvmlInit(), NVML_SUCCESS, "unversioned nvmlInit");
    uint64_t opens = fake_get(8);
    CHECK(opens == 2, "init opened %llu files, want 2", (unsigned long long)opens);
    RC(nvmlInit_v2(), NVML_SUCCESS, "second init");
    CHECK(fake_get(8) == opens, "second init reopened a device (%llu opens)",
          (unsigned long long)fake_get(8));
    RC(nvmlShutdown(), NVML_SUCCESS, "first shutdown");
    RC(nvmlDeviceGetCount(&n), NVML_SUCCESS, "count while one reference remains");
    CHECK(fake_get(9) == 0, "first shutdown closed a device");
    RC(nvmlShutdown(), NVML_SUCCESS, "last shutdown");
    RC(nvmlDeviceGetCount_v2(&n), NVML_ERROR_UNINITIALIZED, "count after last shutdown");
    clean("refcount");
    RC(nvmlInit_v2(), NVML_SUCCESS, "re-init");
    RC(nvmlShutdown(), NVML_SUCCESS, "re-init shutdown");
    clean("re-init");
}

enum call { NONE, COUNT, NAME, UUID, TEMP, MEM, UTIL, PROCS };

struct fault {
    char letter;
    nvmlReturn_t init;
    enum call call;
    nvmlReturn_t rc;
};

static const struct fault FAULTS[] = {
    {'o', NVML_ERROR_DRIVER_NOT_LOADED, NONE, 0},
    {'a', NVML_ERROR_NO_PERMISSION, NONE, 0},
    {'d', NVML_ERROR_DRIVER_NOT_LOADED, NONE, 0},
    {'P', NVML_ERROR_NO_PERMISSION, NONE, 0},
    {'A', NVML_ERROR_NO_PERMISSION, NONE, 0},
    {'9', NVML_ERROR_UNKNOWN, NONE, 0},
    {'0', NVML_SUCCESS, COUNT, NVML_SUCCESS},
    {'2', NVML_SUCCESS, COUNT, NVML_SUCCESS},
    {'c', NVML_SUCCESS, NAME, NVML_ERROR_NOT_SUPPORTED},
    {'G', NVML_SUCCESS, NAME, NVML_ERROR_GPU_IS_LOST},
    {'n', NVML_SUCCESS, NAME, NVML_ERROR_UNKNOWN},
    {'E', NVML_SUCCESS, NAME, NVML_ERROR_UNKNOWN},
    {'S', NVML_SUCCESS, NAME, NVML_ERROR_UNKNOWN},
    {'g', NVML_SUCCESS, UUID, NVML_ERROR_UNKNOWN},
    {'t', NVML_SUCCESS, TEMP, NVML_ERROR_UNKNOWN},
    {'h', NVML_SUCCESS, TEMP, NVML_ERROR_UNKNOWN},
    {'v', NVML_SUCCESS, TEMP, NVML_ERROR_NOT_SUPPORTED},
    {'z', NVML_SUCCESS, MEM, NVML_ERROR_NOT_SUPPORTED},
    {'x', NVML_SUCCESS, MEM, NVML_ERROR_NOT_SUPPORTED},
    {'m', NVML_SUCCESS, MEM, NVML_ERROR_UNKNOWN},
    {'w', NVML_SUCCESS, UTIL, NVML_ERROR_NOT_SUPPORTED},
    {'y', NVML_SUCCESS, UTIL, NVML_ERROR_NO_PERMISSION},
    {'e', NVML_SUCCESS, UTIL, NVML_ERROR_UNKNOWN},
    {'r', NVML_SUCCESS, UTIL, NVML_ERROR_UNKNOWN},
    {'Z', NVML_SUCCESS, UTIL, NVML_ERROR_UNKNOWN},
    {'X', NVML_SUCCESS, UTIL, NVML_ERROR_UNKNOWN},
    {'k', NVML_SUCCESS, PROCS, NVML_ERROR_NOT_SUPPORTED},
    {'l', NVML_SUCCESS, PROCS, NVML_SUCCESS},
    {'p', NVML_SUCCESS, PROCS, NVML_ERROR_UNKNOWN},
    {'b', NVML_SUCCESS, PROCS, NVML_SUCCESS},
    {'i', NVML_SUCCESS, PROCS, NVML_SUCCESS},
    {'j', NVML_SUCCESS, PROCS, NVML_ERROR_UNKNOWN},
    {'f', NVML_SUCCESS, NONE, 0},
};

static nvmlProcessInfo_t procs[256];

static void one(const struct fault *f) {
    char what[32], buf[96];
    unsigned n = 0, t = 0;
    nvmlDevice_t d = NULL;
    nvmlMemory_t m;
    nvmlUtilization_t u;
    snprintf(what, sizeof what, "fault %c", f->letter);
    fake_reset(f->letter);
    nvmlReturn_t r = nvmlInit_v2();
    CHECK(r == f->init, "%s: init rc %d, want %d", what, (int)r, (int)f->init);
    if (r != NVML_SUCCESS) {
        RC(nvmlDeviceGetCount_v2(&n), NVML_ERROR_UNINITIALIZED, what);
        clean(what);
        return;
    }
    if (f->call != COUNT) RC(nvmlDeviceGetHandleByIndex_v2(0, &d), NVML_SUCCESS, what);
    switch (f->call) {
    case NONE:
        break;
    case COUNT:
        RC(nvmlDeviceGetCount_v2(&n), f->rc, what);
        if (f->letter == '0') {
            CHECK(n == 0, "%s: count %u, want 0", what, n);
            RC(nvmlDeviceGetHandleByIndex_v2(0, &d), NVML_ERROR_INVALID_ARGUMENT, what);
        } else {
            CHECK(n == 2, "%s: count %u, want 2", what, n);
            nvmlDevice_t d0 = NULL, d1 = NULL;
            RC(nvmlDeviceGetHandleByIndex_v2(0, &d0), NVML_SUCCESS, what);
            RC(nvmlDeviceGetHandleByIndex_v2(1, &d1), NVML_SUCCESS, what);
            CHECK(d0 != d1, "%s: distinct handles", what);
            RC(nvmlDeviceGetUUID(d1, buf, sizeof buf), NVML_SUCCESS, what);
            CHECK(strcmp(buf, UUID1) == 0, "%s: uuid 1 \"%s\"", what, buf);
            RC(nvmlDeviceGetUUID(d0, buf, sizeof buf), NVML_SUCCESS, what);
            CHECK(strcmp(buf, UUID0) == 0, "%s: uuid 0 \"%s\"", what, buf);
            RC(nvmlDeviceGetTemperature(d1, NVML_TEMPERATURE_GPU, &t), NVML_SUCCESS, what);
            CHECK(t == 41, "%s: temperature 1 %u, want 41", what, t);
            RC(nvmlDeviceGetHandleByIndex_v2(2, &d), NVML_ERROR_INVALID_ARGUMENT, what);
        }
        break;
    case NAME:
        RC(nvmlDeviceGetName(d, buf, sizeof buf), f->rc, what);
        break;
    case UUID:
        RC(nvmlDeviceGetUUID(d, buf, sizeof buf), f->rc, what);
        break;
    case TEMP:
        RC(nvmlDeviceGetTemperature(d, NVML_TEMPERATURE_GPU, &t), f->rc, what);
        break;
    case MEM:
        RC(nvmlDeviceGetMemoryInfo(d, &m), f->rc, what);
        break;
    case UTIL:
        RC(nvmlDeviceGetUtilizationRates(d, &u), f->rc, what);
        break;
    case PROCS:
        n = 256;
        memset(procs, 0, sizeof procs);
        RC(nvmlDeviceGetComputeRunningProcesses_v3(d, &n, procs), f->rc, what);
        if (f->letter == 'l') CHECK(n == 0, "%s: procs %u, want 0", what, n);
        if (f->letter == 'b') {
            CHECK(n == 250, "%s: procs %u, want 250", what, n);
            unsigned bad = 0;
            for (unsigned i = 0; i < 250; i++)
                if (procs[i].pid != 1000 + i ||
                    procs[i].usedGpuMemory != (unsigned long long)(1000 + i) * 65536ull)
                    bad++;
            CHECK(bad == 0, "%s: %u of 250 entries wrong", what, bad);
            n = 249;
            RC(nvmlDeviceGetComputeRunningProcesses_v3(d, &n, procs), NVML_ERROR_INSUFFICIENT_SIZE,
               what);
            CHECK(n == 250, "%s: needed %u, want 250", what, n);
        }
        if (f->letter == 'i') {
            CHECK(n == 2 && procs[0].usedGpuMemory == 351272960ull &&
                      procs[1].pid == 4242 &&
                      procs[1].usedGpuMemory == (unsigned long long)NVML_VALUE_NOT_AVAILABLE,
                  "%s: procs %u, memory %llu %llu", what, n,
                  (unsigned long long)procs[0].usedGpuMemory,
                  (unsigned long long)procs[1].usedGpuMemory);
        }
        break;
    }
    if (f->letter == 'f') {
        RC(nvmlShutdown(), NVML_ERROR_UNKNOWN, what);
        RC(nvmlDeviceGetCount_v2(&n), NVML_ERROR_UNINITIALIZED, what);
        CHECK(fake_get(14) == 0, "%s: fds left open %llx", what, (unsigned long long)fake_get(14));
        CHECK(fake_get(13) == 0, "%s: violations %llu", what, (unsigned long long)fake_get(13));
        return;
    }
    RC(nvmlShutdown(), NVML_SUCCESS, what);
    clean(what);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: nvml_test ERRSTR_ORACLE.tsv\n");
        return 2;
    }
    errstrings(argv[1]);
    healthy();
    refcount();
    for (unsigned i = 0; i < sizeof FAULTS / sizeof FAULTS[0]; i++) one(&FAULTS[i]);
    healthy();
    CHECK(fake_trapped() == 0, "fake trapped %llu times", (unsigned long long)fake_trapped());
    if (fails) {
        printf("CUDAPP-NVML HOST FAIL (%u of %u checks)\n", fails, checks);
        return 1;
    }
    printf("CUDAPP-NVML HOST PASS (%u checks)\n", checks);
    return 0;
}
#endif
