<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# compat/nvml: CUDA++ libnvidia-ml subset

A C ABI shell that answers this subset of NVML (CUDA 13.0 `nvml.h`, line of
each prototype in `nvml.h` here):

`nvmlInit_v2`, `nvmlShutdown`, `nvmlErrorString`, `nvmlDeviceGetCount_v2`,
`nvmlDeviceGetHandleByIndex_v2`, `nvmlDeviceGetName`, `nvmlDeviceGetUUID`,
`nvmlDeviceGetTemperature`, `nvmlDeviceGetMemoryInfo`,
`nvmlDeviceGetUtilizationRates`, `nvmlDeviceGetComputeRunningProcesses_v3`,
and the unversioned `nvmlInit`/`nvmlDeviceGetCount`/... names through the
same macros NVIDIA's header uses (off with `NVML_NO_UNVERSIONED_FUNC_DEFS`).

- `nvml.h` is our own header: only the types, values and prototypes of the
  subset, each cited to the CUDA 13.0 `nvml.h` line. No NVIDIA text is copied.
  `tests/nvml/abi.c` checks 55 sizes, offsets and values against the toolkit
  header, and `tests/nvml/run.sh` compiles the shell against NVIDIA's
  prototypes (header only).
- `cudapp_nvml.c` is the shell. It holds a mutex, the step state and buffer,
  the handle table and the `nvmlErrorString` cache. Every decision (argument
  checks, RM requests, reply decoding, error mapping, teardown) is in the
  Omega units `nvml_step` and `nvml_errstr` (`native/nvml`); the shell only
  performs the action each step asks for. `tests/nvml/run.sh` refuses a shell
  that names an OS call, a device path or an RM command.

The shell reaches the OS only through `omega2_nvrm_open_file`,
`omega2_nvrm_close_file` and `omega2_nvrm_ioctl`
(`native/nvrm/transport.om`, aien-dev/CUDApp#281). It is not in `SRCS`: a
library build links `cudapp_nvml.c`, `src/cudapp_om_nvml_step.c`,
`src/cudapp_om_nvml_errstr.c` and the #281 transport. NVIDIA's
libnvidia-ml is never linked or loaded.

## Status

| Tier | Verdict |
| --- | --- |
| Host, fake resource manager (`make test-nvml`) | PASS (476 checks, 22/22 mutants caught), 2026-11-06 |
| ABI against CUDA 13.0 `nvml.h` | PASS (55 facts) |
| Live resource manager | NOT_RUN: opening `/dev/nvidia*` needs an authorized GPU window |
| Utilization over the #281 transport | NOT_RUN: 55888-byte control vs the e5b32567 devcall bound (aien-dev/omega-mv#1048) |

Evidence: `evidence/NVML-2026-11-06/`.
