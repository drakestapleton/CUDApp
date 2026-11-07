/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* MIGRATION: handwritten production ABI/protocol pending aien-dev/omega-bridge#83.
 * Exercised protocol successor: tests/nvml/host.om (fake transport only); production successor
 * is native/nvml/api.om plus bridge-generated ABI output, not implemented by this C file. */
/* CUDA++ NVML subset: the C ABI shell over the Omega unit nvml_step (native/nvml).
 * Each entry point stores its op and arguments, then alternates omega_nvml_step with the
 * one action the step requests, performed by the Omega RM transport (omega2_nvrm_*, native/nvrm/
 * transport.om), until the step reports done. Every return code, decode and teardown decision is
 * made in Omega. The shell only maps nvmlDevice_t to an index, bounds the copies and serializes
 * calls; a refused or trapped step, a malformed request or a runaway loop is NVML_ERROR_UNKNOWN. */
#include "nvml.h"

#include <pthread.h>
#include <stdint.h>
#include <string.h>

#include "cudapp_native.h"

/* native/nvrm/transport.om lowered by omega-bridge (aien-dev/CUDApp#281). */
uint32_t omega2_nvrm_open_file(uint64_t *result, const uint8_t *o_path, uint64_t o_path_len);
uint32_t omega2_nvrm_close_file(uint64_t *result, uint64_t o_fd);
uint32_t omega2_nvrm_ioctl(uint64_t *result, uint64_t o_fd, uint64_t o_nr, uint8_t *o_h,
                           uint64_t o_h_len, uint8_t *o_p, uint64_t o_p_len);

/* native/nvml/nvml.om: SN, BN, PO, OO; at most 8 GPUs. */
enum { NVML_SN = 128, NVML_BN = 83296, NVML_PO = 4608, NVML_OO = 60496, NVML_GPUS = 8 };
enum { NVML_STEPS = 4096 };
#define NVML_MAGIC 0x434450504e564d4cull

struct nvmlDevice_st {
    uint64_t magic;
    uint64_t index;
};

static pthread_mutex_t nvml_lock = PTHREAD_MUTEX_INITIALIZER;
static uint64_t nvml_s[NVML_SN];
static uint8_t nvml_b[NVML_BN];
static struct nvmlDevice_st nvml_dev[NVML_GPUS] = {
    {NVML_MAGIC, 0}, {NVML_MAGIC, 1}, {NVML_MAGIC, 2}, {NVML_MAGIC, 3},
    {NVML_MAGIC, 4}, {NVML_MAGIC, 5}, {NVML_MAGIC, 6}, {NVML_MAGIC, 7},
};
static char nvml_text[32][64];
static uint8_t nvml_text_set[32];

/* The index of a handle this library issued, or UINT64_MAX; Omega rejects that as an argument. */
static uint64_t nvml_index(nvmlDevice_t d) {
    uintptr_t p = (uintptr_t)d, lo = (uintptr_t)&nvml_dev[0];
    if (p < lo || p >= lo + sizeof nvml_dev || (p - lo) % sizeof nvml_dev[0] != 0) return UINT64_MAX;
    return d->magic == NVML_MAGIC ? d->index : UINT64_MAX;
}

static int nvml_act(void) {
    uint64_t *s = nvml_s, r = 0;
    uint32_t st;
    switch (s[8]) {
    case 1:
        if (s[11] > NVML_PO || s[12] > NVML_OO - NVML_PO) return -1;
        st = omega2_nvrm_ioctl(&r, s[9], s[10], nvml_b, s[11], nvml_b + NVML_PO, s[12]);
        break;
    case 2:
        if (s[13] == 0 || s[13] > NVML_OO - NVML_PO) return -1;
        st = omega2_nvrm_open_file(&r, nvml_b + NVML_PO, s[13]);
        break;
    case 3:
        st = omega2_nvrm_close_file(&r, s[9]);
        break;
    default:
        return -1;
    }
    s[14] = r;
    s[15] = st;
    return 0;
}

static nvmlReturn_t nvml_run(uint64_t op, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                             void *out, uint64_t cap, unsigned int *scalar) {
    uint64_t *s = nvml_s;
    nvmlReturn_t ret = NVML_ERROR_UNKNOWN;
    pthread_mutex_lock(&nvml_lock);
    s[0] = op;
    s[1] = a1;
    s[2] = a2;
    s[3] = a3;
    s[4] = a4;
    s[22] = 0;
    for (unsigned n = 0; n < NVML_STEPS; n++) {
        uint64_t r = 1;
        if (omega_nvml_step(&r, s, NVML_SN, nvml_b, NVML_BN) != OMEGA_OK || r != 0) break;
        if (s[8] != 0) {
            if (nvml_act() != 0) break;
            continue;
        }
        if (s[7] > cap || (s[7] != 0 && out == NULL) || (s[27] != 0 && (scalar == NULL || s[5] > UINT32_MAX)))
            break;
        if (s[7] != 0) memcpy(out, nvml_b + NVML_OO, s[7]);
        if (s[27] != 0) *scalar = (unsigned int)s[5];
        ret = s[6] <= 30 || s[6] == 999 ? (nvmlReturn_t)s[6] : NVML_ERROR_UNKNOWN;
        break;
    }
    s[22] = 0;
    pthread_mutex_unlock(&nvml_lock);
    return ret;
}

nvmlReturn_t nvmlInit_v2(void) { return nvml_run(1, 0, 0, 0, 0, NULL, 0, NULL); }

nvmlReturn_t nvmlShutdown(void) { return nvml_run(2, 0, 0, 0, 0, NULL, 0, NULL); }

nvmlReturn_t nvmlDeviceGetCount_v2(unsigned int *deviceCount) {
    return nvml_run(3, 0, deviceCount != NULL, 0, 0, NULL, 0, deviceCount);
}

nvmlReturn_t nvmlDeviceGetHandleByIndex_v2(unsigned int index, nvmlDevice_t *device) {
    unsigned int i = 0;
    nvmlReturn_t r = nvml_run(4, index, device != NULL, 0, 0, NULL, 0, &i);
    if (r == NVML_SUCCESS) {
        if (i >= NVML_GPUS) return NVML_ERROR_UNKNOWN;
        *device = &nvml_dev[i];
    }
    return r;
}

nvmlReturn_t nvmlDeviceGetName(nvmlDevice_t device, char *name, unsigned int length) {
    return nvml_run(5, nvml_index(device), name != NULL, length, 0, name, length, NULL);
}

nvmlReturn_t nvmlDeviceGetUUID(nvmlDevice_t device, char *uuid, unsigned int length) {
    return nvml_run(6, nvml_index(device), uuid != NULL, length, 0, uuid, length, NULL);
}

nvmlReturn_t nvmlDeviceGetTemperature(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType,
                                      unsigned int *temp) {
    return nvml_run(7, nvml_index(device), temp != NULL, (uint64_t)(unsigned int)sensorType, 0, NULL,
                    0, temp);
}

nvmlReturn_t nvmlDeviceGetMemoryInfo(nvmlDevice_t device, nvmlMemory_t *memory) {
    return nvml_run(8, nvml_index(device), memory != NULL, 0, 0, memory, sizeof *memory, NULL);
}

nvmlReturn_t nvmlDeviceGetUtilizationRates(nvmlDevice_t device, nvmlUtilization_t *utilization) {
    return nvml_run(9, nvml_index(device), utilization != NULL, 0, 0, utilization, sizeof *utilization,
                    NULL);
}

nvmlReturn_t nvmlDeviceGetComputeRunningProcesses_v3(nvmlDevice_t device, unsigned int *infoCount,
                                                     nvmlProcessInfo_t *infos) {
    uint64_t want = infoCount != NULL ? *infoCount : 0;
    return nvml_run(10, nvml_index(device), infoCount != NULL, want, infos != NULL, infos,
                    want * sizeof(nvmlProcessInfo_t), infoCount);
}

/* The text comes from the Omega unit nvml_errstr; each slot is filled once and stays stable. */
const char *nvmlErrorString(nvmlReturn_t result) {
    uint8_t o[64];
    uint64_t r = 0, slot;
    const char *t = "Unknown Error";
    pthread_mutex_lock(&nvml_lock);
    if (omega_nvml_errstr(&r, (uint64_t)(int64_t)result, o, sizeof o) == OMEGA_OK && (r & 255) < 64 &&
        (slot = r >> 8 & 255) < 32) {
        if (!nvml_text_set[slot]) {
            memcpy(nvml_text[slot], o, 64);
            nvml_text[slot][r & 255] = 0;
            nvml_text_set[slot] = 1;
        }
        t = nvml_text[slot];
    }
    pthread_mutex_unlock(&nvml_lock);
    return t;
}
