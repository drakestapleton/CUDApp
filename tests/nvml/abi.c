/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Prints the NVML ABI facts the CUDA++ subset depends on. tests/nvml/run.sh builds it once with
 * compat/nvml/nvml.h and once with the CUDA toolkit's nvml.h (header only, nothing is linked) and
 * requires identical output. */
#include <stddef.h>
#include <stdio.h>

#include "nvml.h"

#define SHOW(x) printf("%s %lld\n", #x, (long long)(x))

int main(void) {
    SHOW(sizeof(nvmlDevice_t));
    SHOW(sizeof(nvmlReturn_t));
    SHOW(sizeof(nvmlTemperatureSensors_t));
    SHOW(sizeof(nvmlUtilization_t));
    SHOW(offsetof(nvmlUtilization_t, gpu));
    SHOW(offsetof(nvmlUtilization_t, memory));
    SHOW(sizeof(nvmlMemory_t));
    SHOW(offsetof(nvmlMemory_t, total));
    SHOW(offsetof(nvmlMemory_t, free));
    SHOW(offsetof(nvmlMemory_t, used));
    SHOW(sizeof(nvmlProcessInfo_t));
    SHOW(offsetof(nvmlProcessInfo_t, pid));
    SHOW(offsetof(nvmlProcessInfo_t, usedGpuMemory));
    SHOW(offsetof(nvmlProcessInfo_t, gpuInstanceId));
    SHOW(offsetof(nvmlProcessInfo_t, computeInstanceId));
    SHOW(sizeof(nvmlProcessInfo_v2_t));
    SHOW(NVML_VALUE_NOT_AVAILABLE);
    SHOW(NVML_TEMPERATURE_GPU);
    SHOW(NVML_TEMPERATURE_COUNT);
    SHOW(NVML_DEVICE_NAME_BUFFER_SIZE);
    SHOW(NVML_DEVICE_NAME_V2_BUFFER_SIZE);
    SHOW(NVML_DEVICE_UUID_BUFFER_SIZE);
    SHOW(NVML_DEVICE_UUID_V2_BUFFER_SIZE);
    SHOW(NVML_SUCCESS);
    SHOW(NVML_ERROR_UNINITIALIZED);
    SHOW(NVML_ERROR_INVALID_ARGUMENT);
    SHOW(NVML_ERROR_NOT_SUPPORTED);
    SHOW(NVML_ERROR_NO_PERMISSION);
    SHOW(NVML_ERROR_ALREADY_INITIALIZED);
    SHOW(NVML_ERROR_NOT_FOUND);
    SHOW(NVML_ERROR_INSUFFICIENT_SIZE);
    SHOW(NVML_ERROR_INSUFFICIENT_POWER);
    SHOW(NVML_ERROR_DRIVER_NOT_LOADED);
    SHOW(NVML_ERROR_TIMEOUT);
    SHOW(NVML_ERROR_IRQ_ISSUE);
    SHOW(NVML_ERROR_LIBRARY_NOT_FOUND);
    SHOW(NVML_ERROR_FUNCTION_NOT_FOUND);
    SHOW(NVML_ERROR_CORRUPTED_INFOROM);
    SHOW(NVML_ERROR_GPU_IS_LOST);
    SHOW(NVML_ERROR_RESET_REQUIRED);
    SHOW(NVML_ERROR_OPERATING_SYSTEM);
    SHOW(NVML_ERROR_LIB_RM_VERSION_MISMATCH);
    SHOW(NVML_ERROR_IN_USE);
    SHOW(NVML_ERROR_MEMORY);
    SHOW(NVML_ERROR_NO_DATA);
    SHOW(NVML_ERROR_VGPU_ECC_NOT_SUPPORTED);
    SHOW(NVML_ERROR_INSUFFICIENT_RESOURCES);
    SHOW(NVML_ERROR_FREQ_NOT_SUPPORTED);
    SHOW(NVML_ERROR_ARGUMENT_VERSION_MISMATCH);
    SHOW(NVML_ERROR_DEPRECATED);
    SHOW(NVML_ERROR_NOT_READY);
    SHOW(NVML_ERROR_GPU_NOT_FOUND);
    SHOW(NVML_ERROR_INVALID_STATE);
    SHOW(NVML_ERROR_RESET_TYPE_NOT_SUPPORTED);
    SHOW(NVML_ERROR_UNKNOWN);
    return 0;
}
