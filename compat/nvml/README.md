<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# compat/nvml: CUDA++ libnvidia-ml subset

Every file here is **generated** from the seeded Omega unit `native/nvml/api.om`
by the omega-bridge cabi-1 C ABI library emitter (omega-bridge#83). Never edit
them; edit `api.om` and regenerate:

| File | Bridge mode |
| --- | --- |
| `libnvidia-ml.c` | `tools/bridge-seeded lib-c native/nvml/api.om` |
| `nvml.h` | `tools/bridge-seeded lib-h native/nvml/api.om` |
| `libnvidia-ml.map` | `tools/bridge-seeded lib-map native/nvml/api.om` (version script: the 11 exports, `local: *;`) |

The bridge checkout is `OMEGA_CABI_BRIDGE`; its `src/toc.om` and
`tools/bridge-seeded` are pinned by sha256 in `om/omega-bridge-cabi.lock`. This
second pin is temporary (Manager(Aien) D2, unify under Linear DRA-154) and is
used only for `api.om`; the 53 other units stay on `om/omega-bridge.lock`
through `om/units.sh`. `tests/nvml/gate.om` regenerates all three files and
refuses any byte difference.

The library answers this subset of NVML (CUDA 13.0 `nvml.h`):
`nvmlInit_v2`, `nvmlShutdown`, `nvmlErrorString`, `nvmlDeviceGetCount_v2`,
`nvmlDeviceGetHandleByIndex_v2`, `nvmlDeviceGetName`, `nvmlDeviceGetUUID`,
`nvmlDeviceGetTemperature`, `nvmlDeviceGetMemoryInfo`,
`nvmlDeviceGetUtilizationRates`, `nvmlDeviceGetComputeRunningProcesses_v3`,
and the unversioned `nvmlInit`/`nvmlDeviceGetCount`/... names through the same
macros NVIDIA's header uses (off with `NVML_NO_UNVERSIONED_FUNC_DEFS`). Types,
values and prototypes follow the CUDA 13.0 header; no NVIDIA text is copied.

`api.om` owns the protocol loop, the dispatch of each step action to the #281
transport (`omega2_nvrm_open_file`, `omega2_nvrm_close_file`,
`omega2_nvrm_ioctl`, `native/nvrm/transport.om`), copy bounds, opaque handles,
the error-string cache and the process lock. RM request construction and error
policy stay in `native/nvml/{nvml,query,step,errstr}.om`, imported by `api.om`.
NVIDIA's libnvidia-ml is never linked or loaded.

## Build

```sh
cc -fPIC -shared -Wl,-soname,libnvidia-ml.so.1 \
   -Wl,--version-script=compat/nvml/libnvidia-ml.map \
   -o libnvidia-ml.so.1 compat/nvml/libnvidia-ml.c
```

The library imports the transport symbols from the process that loads it
(the test caller links `tests/nvml/transport.c`; production links the #281
transport), so `-Wl,-z,defs` is not used. The gate instead checks the
undefined symbols against an allowlist.

## Behaviour changes from the removed handwritten shell

The handwritten `cudapp_nvml.c` is gone. The generated library is stricter in
four cases, each reachable only if a step returns malformed output:

- a record output (memory, utilization) must be exactly 0 or the record size;
- process-information bytes must be a multiple of 24 (`nvmlProcessInfo_v2_t`);
- Name/UUID output must be non-empty and end in a NUL;
- a handle slot that was never issued is invalid.

The 476 checks and 22 mutants observe the same results as before.

## Status

| Tier | Verdict |
| --- | --- |
| Generated `libnvidia-ml.so.1` + C caller over fake RM (`tests/nvml/transport.c`) | HOST_TESTED: 476 checks |
| Seeded Omega model over fake RM | HOST_TESTED: 476 checks; 22/22 mutants with the original failure counts |
| ABI against CUDA 13.0 `nvml.h` | HOST_TESTED: 55 facts; 11 generated prototypes compile against it; a changed type is rejected |
| Exports, soname and imports (ORACLE nm/readelf) | HOST_TESTED: exactly 11 `T` exports, soname `libnvidia-ml.so.1`, allowlisted imports; an object calling `ioctl` is rejected |
| Concurrent callers | NOT_RUN |
| Live resource manager | NOT_RUN: opening `/dev/nvidia*` needs an authorized GPU window |
| Utilization over the #281 transport | NOT_RUN: 55888-byte control vs the e5b32567 devcall bound (aien-dev/omega-mv#1048) |

Evidence: `evidence/NVML-365-2026-11-07/`. No hardware qualification or
deployment claim. CI on this change is NOT_RUN while aien-dev Actions is
externally blocked.
