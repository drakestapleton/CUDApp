<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# native/nvml: the Omega side of the CUDA++ NVML subset

Two pure Omega units carry RM request/reply policy and error-string encoding.
The production C shell still contains call orchestration, copy bounds, handle
mapping and cache management; see `compat/nvml/README.md` for its migration HOLD.

| Unit | Sources (composed in this order, each followed by a newline) | Export |
| --- | --- | --- |
| `nvml_step` | `nvml.om`, `query.om`, `step.om` | `omega_nvml_step(result, s[128 ulong], b[83296 uchar])` |
| `nvml_errstr` | `errstr.om` | `omega_nvml_errstr(result, code, o[64 uchar])` |

`om/units.sh` lowers both through omega-bridge (`bridge-seeded`, pinned
omc e5b32567) into the committed `src/cudapp_om_nvml_step.c` and
`src/cudapp_om_nvml_errstr.c`; their prototypes are in `src/cudapp_native.h`
and their identities in `src/cudapp_om_nvml_*.ident` and
`src/cudapp_om_units.ident`. Never edit the generated C; edit the `.om` and
rerun `om/units.sh`. `tests/nvml/gate.om` refuses a stale generated file.

## The step protocol

A pure unit cannot touch the OS, so `nvml_step` is a resumable state machine.
The C shell stores the op (s0) and its arguments (s1..s4), then loops:

1. call `omega_nvml_step`;
2. if s8 asks for an action (1 ioctl, 2 open, 3 close), perform exactly that
   action through `omega2_nvrm_ioctl`, `omega2_nvrm_open_file` or
   `omega2_nvrm_close_file` (`native/nvrm/transport.om`, aien-dev/CUDApp#281)
   with the fd, nr, header length and params length the step chose, write the
   transport result into s14 and its omega2 status into s15;
3. stop when s8 is 0; s6 is the `nvmlReturn_t`, s7 the output bytes at OO, s5
   the scalar output and s27 whether to store it.

The full slot map is the header comment of `nvml.om`. Transport results are a
`long` carried as its two's-complement bits in a ulong (−1 is 2^64−1).

## What each op asks the resource manager

All layouts are cited in the sources to open-gpu-kernel-modules 580.173.02
(commit 20e4e6e1), `src/common/sdk/nvidia/inc`.

| NVML call | RM traffic |
| --- | --- |
| `nvmlInit_v2` (op 1) | open `/dev/nvidiactl`; `NV_ESC_CARD_INFO` (nr 200); root client (`NV01_ROOT_CLIENT`, nr 43); per GPU: `GET_ID_INFO_V2` (0x205), open `/dev/nvidia<minor>`, `NV_ESC_REGISTER_FD` (nr 201), device (0x80) and subdevice (0x2080) alloc, `CLASSLIST_V2` (0x800292) to find `BLACKWELL_COMPUTE_B` (0xCEC0). Reference counted. |
| `nvmlShutdown` (op 2) | last reference only: free subdevices, devices, the root client; close every fd. |
| `GetName` (op 5) | `GPU_GET_NAME_STRING` (0x20800110), ASCII flavour. |
| `GetUUID` (op 6) | `GPU_GET_GID_INFO` (0x2080014a), binary SHA-1 form, printed `GPU-8-4-4-4-12`. |
| `GetTemperature` (op 7) | two `THERMAL_SYSTEM_EXECUTE_V2` (0x20800513) passes: enumerate targets and sensors, then read the first sensor on a GPU-type target. |
| `GetMemoryInfo` (op 8) | `FB_GET_INFO_V2` (0x20801303): TOTAL_RAM_SIZE, HEAP_FREE (KiB), IS_ZERO_FB (NOT_SUPPORTED when set). |
| `GetUtilizationRates` (op 9) | `PERF_GET_GPUMON_PERFMON_UTIL_SAMPLES_V2` (0x20802096): newest sample of the 72-entry ring. |
| `GetComputeRunningProcesses_v3` (op 10) | `GPU_GET_PIDS` (0x2080018d) for class 0xCEC0, then `GPU_GET_PID_INFO` (0x2080018e) in chunks of at most 200 for video-memory usage. |

Error policy (`nvml.om`): an omega2 status other than 0 is UNKNOWN; ioctl
EPERM/EACCES is NO_PERMISSION, other errnos UNKNOWN; open ENOENT/ENXIO/ENODEV
is DRIVER_NOT_LOADED; RM status 0x1B is NO_PERMISSION, 0x0F GPU_IS_LOST,
0x56 NOT_SUPPORTED, anything else UNKNOWN. A failed `nvmlInit_v2` tears down
everything it opened and leaves the refcount at 0.

## Known limits

- At most 8 GPUs; more is UNKNOWN at init.
- `GetUtilizationRates` sends 55888 bytes of params. The pinned omc e5b32567
  bounds program-mode `devcall` at 16383 bytes; whether the lowered #281
  transport can carry this control is NOT_RUN (live RM is not authorized). The
  program-mode `cudapp-smi` needs aien-dev/omega-mv#1048 for the same control.
- A trap inside the teardown after a failed init can leave descriptors open;
  the shell then reports UNKNOWN.
- Live resource-manager behavior is NOT_RUN: every test runs against
  `tests/nvml/rmfake.om`.

Constants: `evidence/NVML-2026-11-06/wolfram.txt`. Tests: `tests/nvml/gate.om`
(`make test-nvml`). The 476 checks in `tests/nvml/host.om` exercise a pure
Omega port of the shell protocol against the fake transport, not a production
shared library. Generated NVML ABI support is tracked by
[omega-bridge#83](https://github.com/aien-dev/omega-bridge/issues/83).
