<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# NVML subset host tier, 2026-11-06

Branch `feat/cudapp-nvml` (CUDApp), base `feat/cudapp-smi` (#286, 85f48ad).
Compiler: omc at omega-mv e5b32567, sha256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`;
omega-bridge bc11b30b (`bridge-seeded` frontend 23ff7a23…). Every command ran
inside a device-free namespace on cores 10-14:

```sh
W="bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10"
$W make test-nvml OMEGA_OMC=$OMC OMEGA_BRIDGE=$BRIDGE          # rc 0, CUDAPP-NVML HOST PASS
env -u OMEGA_OMC -u OMEGA_BRIDGE $W make test-nvml              # rc 2, NOT_RUN
OMEGA_OMC=<omega-mv#1048 candidate omc> $W sh tests/nvml/run.sh # rc 69, REFUSED (omc digest)
env -u OMEGA_OMC -u OMEGA_BRIDGE $W sh tests/nvml/run.sh        # rc 77, NOT_RUN
```

`commands.txt` has the rc and last line of each; the logs are beside it
(home directory written as `~`).

## Verdicts

| Tier | Verdict |
| --- | --- |
| Committed generated C (`src/cudapp_om_nvml_{step,errstr}.c`) fresh against the `.om` | PASS |
| Fake resource manager (`tests/nvml/rmfake.om` behind `tests/nvml/transport.c`) | PASS, 476 checks |
| Error-string oracle negative control (one oracle row altered) | PASS: the run fails 1 of 476 |
| ABI: 55 sizes, offsets and values, `compat/nvml/nvml.h` vs CUDA 13.0 `nvml.h` | PASS |
| Shell compiles against NVIDIA's prototypes (header only) | PASS |
| Thin-shell guard, and its negative control (a shell calling `ioctl`) | PASS |
| Mutants of `native/nvml/*.om` | 22/22 caught |
| Live resource manager (`/dev/nvidiactl`, `/dev/nvidia0`) | NOT_RUN: no authorized GPU window |
| GB10 chip | NOT_RUN |

The fake transport replaces `omega2_nvrm_*` at link time, so no test opens a
device; the bwrap namespace (empty `/dev`) makes that hold by construction.

## Oracle provenance

`tests/nvml/errstr-oracle.tsv` holds 37 rows (codes −1, 0..32, 999, 1000,
4294967295). It was captured on this Spark with Python `ctypes` calling
`nvmlErrorString` from `/usr/lib/aarch64-linux-gnu/libnvidia-ml.so.580.173.02`
without `nvmlInit` (the call opens no device). The library is used only as
that oracle; nothing in CUDA++ links or loads it.

## Constants

`wolfram.txt`: every derived constant (packed path words, buffer offsets,
command values in hex, fake RM values, two's-complement errno words, packed
error strings) with the Wolfram Language query and its result.
