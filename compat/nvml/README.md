<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# compat/nvml: CUDA++ libnvidia-ml subset

**MIGRATION / HOLD:** production remains handwritten C until
[omega-bridge#83](https://github.com/aien-dev/omega-bridge/issues/83)
generates the shared-library ABI and lowers the Omega protocol/state.
The current C ABI shell answers this subset of NVML (CUDA 13.0 `nvml.h`):

`nvmlInit_v2`, `nvmlShutdown`, `nvmlErrorString`, `nvmlDeviceGetCount_v2`,
`nvmlDeviceGetHandleByIndex_v2`, `nvmlDeviceGetName`, `nvmlDeviceGetUUID`,
`nvmlDeviceGetTemperature`, `nvmlDeviceGetMemoryInfo`,
`nvmlDeviceGetUtilizationRates`, `nvmlDeviceGetComputeRunningProcesses_v3`,
and the unversioned `nvmlInit`/`nvmlDeviceGetCount`/... names through the
same macros NVIDIA's header uses (off with `NVML_NO_UNVERSIONED_FUNC_DEFS`).

- `nvml.h` is our own header: only the types, values and prototypes of the
  subset, each cited to the CUDA 13.0 `nvml.h` line. No NVIDIA text is copied.
  `tests/nvml/abi.c` checks 55 sizes, offsets and values against the toolkit
  header, and `tests/nvml/gate.om` compiles the shell against NVIDIA's
  prototypes (header only).
- `cudapp_nvml.c` is the handwritten production shell (170 physical lines;
  167 before migration labels). It retains the mutex, step state/buffer,
  handle mapping, action/call protocol, copy bounds and string cache.
  RM request construction/decoding and primary error policy are Omega units.
  The thin-shell guard detects OS calls/device paths/RM commands, not arbitrary
  C logic; passing it does not satisfy the pure-Omega directive.
- `tests/nvml/host.om` is a 942-line pure-Omega model: protocol/wrappers/cache
  at lines 225–444, fake transport at 144–223, plus test helpers and checks.
  Its 476 checks use `rmfake.om`; they are **not production .so behavior**.
  The production C path remains until a generated ABI exists.
- The target is `native/nvml/api.om` plus generated C/header/export-map
  output, including nullable outputs, stable strings, opaque handles,
  process globals and a lock, and exact-name soname exports. This target
  is specified in omega-bridge#83, not implemented by relabelling this C.

The shell reaches the OS only through `omega2_nvrm_open_file`,
`omega2_nvrm_close_file` and `omega2_nvrm_ioctl`
(`native/nvrm/transport.om`, aien-dev/CUDApp#281). It is not in `SRCS`: a
library build links `cudapp_nvml.c`, `src/cudapp_om_nvml_step.c`,
`src/cudapp_om_nvml_errstr.c` and the #281 transport. NVIDIA's
libnvidia-ml is never linked or loaded.

## Status

| Tier | Verdict |
| --- | --- |
| Seeded Omega model over fake RM | EXERCISED: 476 checks; 22/22 mutants with the original failure counts |
| C ABI ORACLE over fake RM | EXERCISED: 476 checks, same healthy summary |
| ABI against CUDA 13.0 `nvml.h` | EXERCISED: 55 facts; prototype compilation |
| Generated production shared library and concurrency | NOT_RUN: omega-bridge#83 |
| Live resource manager | NOT_RUN: opening `/dev/nvidia*` needs an authorized GPU window |
| Utilization over the #281 transport | NOT_RUN: 55888-byte control vs the e5b32567 devcall bound (aien-dev/omega-mv#1048) |

Evidence: `evidence/NVML-2026-11-07/seeded/`. No hardware qualification,
deployment, or READY claim. CI on this change is NOT_RUN while aien-dev
Actions is externally blocked; older green heads do not cover these edits.
