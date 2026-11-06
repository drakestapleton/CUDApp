/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* CUDA++ NVML subset: an ABI-compatible declaration of the libnvidia-ml entry points
 * compat/nvml/cudapp_nvml.c implements. Written for CUDA++; not NVIDIA's header. Type layouts,
 * enum values and symbol names follow nvml.h from CUDA 13.0 (NVML_API_VERSION 13); the line
 * numbers below refer to /usr/local/cuda-13.0/targets/sbsa-linux/include/nvml.h. */
#ifndef CUDAPP_NVML_H
#define CUDAPP_NVML_H

#ifdef __cplusplus
extern "C" {
#endif

/* nvml.h:117-129: the unversioned names map to the current versions. */
#ifndef NVML_NO_UNVERSIONED_FUNC_DEFS
#define nvmlInit nvmlInit_v2
#define nvmlDeviceGetCount nvmlDeviceGetCount_v2
#define nvmlDeviceGetHandleByIndex nvmlDeviceGetHandleByIndex_v2
#define nvmlDeviceGetComputeRunningProcesses nvmlDeviceGetComputeRunningProcesses_v3
#endif

/* nvml.h:155 */
#define NVML_VALUE_NOT_AVAILABLE (-1)

/* nvml.h:157 */
typedef struct nvmlDevice_st *nvmlDevice_t;

/* nvml.h:244-248 */
typedef struct nvmlUtilization_st {
    unsigned int gpu;
    unsigned int memory;
} nvmlUtilization_t;

/* nvml.h:254-260 */
typedef struct nvmlMemory_st {
    unsigned long long total;
    unsigned long long free;
    unsigned long long used;
} nvmlMemory_t;

/* nvml.h:303-312 */
typedef struct nvmlProcessInfo_v2_st {
    unsigned int pid;
    unsigned long long usedGpuMemory;
    unsigned int gpuInstanceId;
    unsigned int computeInstanceId;
} nvmlProcessInfo_v2_t, nvmlProcessInfo_t;

/* nvml.h:934-940 */
typedef enum nvmlTemperatureSensors_enum {
    NVML_TEMPERATURE_GPU = 0,
    NVML_TEMPERATURE_COUNT
} nvmlTemperatureSensors_t;

/* nvml.h:1269-1304 */
typedef enum nvmlReturn_enum {
    NVML_SUCCESS = 0,
    NVML_ERROR_UNINITIALIZED = 1,
    NVML_ERROR_INVALID_ARGUMENT = 2,
    NVML_ERROR_NOT_SUPPORTED = 3,
    NVML_ERROR_NO_PERMISSION = 4,
    NVML_ERROR_ALREADY_INITIALIZED = 5,
    NVML_ERROR_NOT_FOUND = 6,
    NVML_ERROR_INSUFFICIENT_SIZE = 7,
    NVML_ERROR_INSUFFICIENT_POWER = 8,
    NVML_ERROR_DRIVER_NOT_LOADED = 9,
    NVML_ERROR_TIMEOUT = 10,
    NVML_ERROR_IRQ_ISSUE = 11,
    NVML_ERROR_LIBRARY_NOT_FOUND = 12,
    NVML_ERROR_FUNCTION_NOT_FOUND = 13,
    NVML_ERROR_CORRUPTED_INFOROM = 14,
    NVML_ERROR_GPU_IS_LOST = 15,
    NVML_ERROR_RESET_REQUIRED = 16,
    NVML_ERROR_OPERATING_SYSTEM = 17,
    NVML_ERROR_LIB_RM_VERSION_MISMATCH = 18,
    NVML_ERROR_IN_USE = 19,
    NVML_ERROR_MEMORY = 20,
    NVML_ERROR_NO_DATA = 21,
    NVML_ERROR_VGPU_ECC_NOT_SUPPORTED = 22,
    NVML_ERROR_INSUFFICIENT_RESOURCES = 23,
    NVML_ERROR_FREQ_NOT_SUPPORTED = 24,
    NVML_ERROR_ARGUMENT_VERSION_MISMATCH = 25,
    NVML_ERROR_DEPRECATED = 26,
    NVML_ERROR_NOT_READY = 27,
    NVML_ERROR_GPU_NOT_FOUND = 28,
    NVML_ERROR_INVALID_STATE = 29,
    NVML_ERROR_RESET_TYPE_NOT_SUPPORTED = 30,
    NVML_ERROR_UNKNOWN = 999
} nvmlReturn_t;

/* nvml.h:3804-3834 */
#define NVML_DEVICE_UUID_BUFFER_SIZE 80
#define NVML_DEVICE_UUID_V2_BUFFER_SIZE 96
#define NVML_DEVICE_NAME_BUFFER_SIZE 64
#define NVML_DEVICE_NAME_V2_BUFFER_SIZE 96

nvmlReturn_t nvmlInit_v2(void);                                          /* nvml.h:3731 */
nvmlReturn_t nvmlShutdown(void);                                         /* nvml.h:3765 */
const char *nvmlErrorString(nvmlReturn_t result);                        /* nvml.h:3786 */
nvmlReturn_t nvmlDeviceGetCount_v2(unsigned int *deviceCount);           /* nvml.h:4221 */
nvmlReturn_t nvmlDeviceGetHandleByIndex_v2(unsigned int index, nvmlDevice_t *device); /* 4289 */
nvmlReturn_t nvmlDeviceGetName(nvmlDevice_t device, char *name, unsigned int length); /* 4432 */
nvmlReturn_t nvmlDeviceGetUUID(nvmlDevice_t device, char *uuid, unsigned int length); /* 4816 */
nvmlReturn_t nvmlDeviceGetTemperature(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType,
                                      unsigned int *temp);               /* nvml.h:5571 */
nvmlReturn_t nvmlDeviceGetMemoryInfo(nvmlDevice_t device, nvmlMemory_t *memory); /* 6330 */
nvmlReturn_t nvmlDeviceGetUtilizationRates(nvmlDevice_t device,
                                           nvmlUtilization_t *utilization); /* nvml.h:6671 */
nvmlReturn_t nvmlDeviceGetComputeRunningProcesses_v3(nvmlDevice_t device, unsigned int *infoCount,
                                                     nvmlProcessInfo_t *infos); /* nvml.h:6985 */

#ifdef __cplusplus
}
#endif
#endif
