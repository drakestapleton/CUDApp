<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# cudapp-smi

`cudapp-smi` is the CUDA++ successor of `nvidia-smi`: a read-only GPU status
reporter written in seeded Omega (`.om`) and compiled by the pinned omc
(omega-mv `e5b32567`, omc sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`).
It is a program-mode executable, not a libcudapp unit: it uses omc's bounded
`devcall` words, which the bridge cannot import into libcudapp today
(omega-bridge#29).

## Build and test

```sh
cat native/smi/smi.om native/nvrm/context.om native/smi/rm.om | "$OMEGA_OMC" > cudapp-smi
make test-smi                     # host tier; needs CUDAPP_OMC_HOST=1 and OMEGA_OMC
sh tests/smi/run.sh "$OMEGA_OMC"  # the same runner, directly
sh tests/smi/live.sh "$OMEGA_OMC" OUTDIR  # live read-only comparison against nvidia-smi
```

`tests/smi/run.sh` covers the fixture tier (`tests/smi/fixtures/gb10`, an
optional-absent tree, a no-GPU tree, 16 malformed-input refusals and usage
errors), the `--rm` tier against a host fake of the resource manager
(`tests/smi/rmfake.om`: one default report, one unsupported-pstate report, 9
faults, handle teardown counts and the one-GPU rule), a guard negative
control (an unrewritten `context.om` is refused, so no fake build can reach a
device), a checker negative control and 14 source mutants. Last line:
`CUDAPP-SMI HOST PASS`.

`tests/smi/live.sh` never passes `--rm`. It compares every field
`cudapp-smi` reads from `/proc` and `/sys` with `nvidia-smi --query-gpu`,
`--query-compute-apps` and `-q`, and runs a comparator negative control. Last
line: `CUDAPP-SMI LIVE PASS (<n> fields)`.

## Data sources

Without `--rm` the program opens only files under `ROOT/proc` and `ROOT/sys`
read-only and reads the `ROOT/proc/<pid>/fd` links. It opens no device.

| Source | Fields |
| --- | --- |
| `/proc/driver/nvidia/version` | `driver_version` |
| `/proc/driver/nvidia/gpus/<bus>/information` | `uuid`, `pci.bus_id`, `minor_number`, `vbios_version`, `gsp_firmware_version` |
| `/sys/bus/pci/devices/<bus>/` | `pci.device_id`, `pci.sub_device_id`, `pci.base_classcode`, `pci.sub_classcode`, `pcie.link.*` |
| `/proc/<pid>/fd`, `/proc/<pid>/cmdline` | `holder.k.pid`, `holder.k.process_name` (every process with an open `/dev/nvidia*` node) |

With `--rm` it also opens `/dev/nvidiactl` and `/dev/nvidia0` through
`native/nvrm/context.om` (`nvstart`/`nvstop`) and issues read-only subdevice
controls only: no memory allocation, channel or mapping. Layouts come from
NVIDIA open-gpu-kernel-modules tag `580.173.02` (commit
`20e4e6e19cc26ba47b5cbe23130a396be100c427`),
`src/common/sdk/nvidia/inc/ctrl/ctrl2080/`:

| Control | Header line | Params | Fields |
| --- | --- | --- | --- |
| `NV2080_CTRL_CMD_GPU_GET_NAME_STRING` 0x20800110 | ctrl2080gpu.h:318 | 132 B | `name` |
| `NV2080_CTRL_CMD_THERMAL_SYSTEM_EXECUTE_V2` 0x20800513 | ctrl2080thermal.h:135 | 1432 B | `temperature.gpu` |
| `NV2080_CTRL_CMD_PERF_GET_CURRENT_PSTATE` 0x20802068 | ctrl2080perf.h:885 | 4 B | `pstate` |
| `NV2080_CTRL_CMD_GPU_GET_PIDS` 0x2080018d | ctrl2080gpu.h:3521 | 3812 B | `compute_app.k.pid` |
| `NV2080_CTRL_CMD_GPU_GET_PID_INFO` 0x2080018e | ctrl2080gpu.h:3663 | 14408 B | `compute_app.k.used_memory[_bytes]` |

`NV_ERR_NOT_SUPPORTED` (0x56, nvstatuscodes.h:115) from the name, temperature
or pstate control prints `[N/A]` where the header allows it; other statuses,
such as `NV_ERR_INSUFFICIENT_PERMISSIONS` (0x1b, nvstatuscodes.h:56), refuse
with exit 31.

## Fields not reported

| nvidia-smi field | Why | Gate |
| --- | --- | --- |
| `utilization.gpu` | `NV2080_CTRL_CMD_PERF_GET_GPUMON_PERFMON_UTIL_SAMPLES_V2` 0x20802096 (ctrl2080perf.h:550) has 55888-byte params (16 + 72 × 776); omc `devcall` traps above 16383 bytes (omc.om L5586 at e5b32567) | omc devcall size extension (aien-dev/omega-mv issue) |
| `clocks.*` | ctrl2080clk.h in the 580.173.02 SDK defines no `NV2080_CTRL_CMD_` control | no public control |
| `power.draw` | ctrl2080pmgr.h defines only `PMGR_GET_MODULE_INFO` 0x20802609; no power reading control | no public control |
| `memory.total/used` | GB10 has no dedicated framebuffer; nvidia-smi itself prints `N/A` | none |

## Exit codes

| Code | Meaning |
| --- | --- |
| 0 | report written |
| 2 | usage |
| 3 | I/O error, or stdout write failed |
| 4 | no NVIDIA driver (`proc/driver/nvidia/version` absent) |
| 10 | malformed version file |
| 11 | information file lacks GPU UUID, Bus Location or Device Minor |
| 12 | malformed GPU entry |
| 13 | PCI vendor is not 0x10de |
| 14 | malformed sysfs number |
| 15 | unknown PCIe link speed |
| 19 | capacity exceeded |
| 30 | resource-manager open failed |
| 31 | a resource-manager control failed |
| 32 | malformed control reply |
| 33 | teardown failed |
| 34 | `--rm` needs exactly one GPU |

A refusal writes one `SMI REFUSE <reason>` line to stderr and nothing to
stdout. Runtime traps exit 100 + trap code.

## Status

Host fixture and fake-RM tiers: PASS. Live read-only tier without `--rm`:
PASS (`evidence/SMI-2026-11-05/`). Live `--rm`: NOT_RUN; it opens the GPU
device nodes and needs an authorized spark-window.
