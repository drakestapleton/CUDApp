<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# Generated NVML library (CUDApp #365, omega-bridge #83), fresh clone, 2026-11-07

Status: HOST_TESTED on a fresh clone of `0472a6d0017ece542d57a00eec49cd0c466e9db9`
(the parent of the commit that adds this directory), aarch64 DGX Spark host, CPU
only, every command under `taskset -c 2 nice -n 10`. Not run in a device-free
namespace; nothing opens a device (the RM transport is the ORACLE fake in
`tests/nvml/transport.c`). `commands.sh` is the exact scratch runner, invoked as
`sh commands.sh 0472a6d0… 9172b525cdf0c521e2d717d17d0dfc818592d7c2`.

Tools (`env.txt`): omc at omega-mv e5b32567 (sha256 `2c824805…`), seed sha256
`191b776e…`. Both bridges are fresh clones built with `tools/build.sh`
(`build-b30.log`, `build-b83.log`, rc 0):

| Variable | omega-bridge commit | `src/toc.om` sha256 | `tools/bridge-seeded` sha256 | Pinned by |
| --- | --- | --- | --- | --- |
| `OMEGA_BRIDGE` | bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4 | `69d629df…` | `23ff7a23…` | `om/units.sh` CONVERTER_PIN/FRONTEND_PIN |
| `OMEGA_CABI_BRIDGE` | 9172b525cdf0c521e2d717d17d0dfc818592d7c2 (#83) | `218214ab…` | `be96a2c1…` | `om/omega-bridge-cabi.lock` |

ORACLE tools (test-only, never shipped): `cc` GCC 13.3.0, `nm`/`readelf` GNU
Binutils 2.42, `sha256sum`, CUDA `/usr/local/cuda/include/nvml.h` (sha256
`28b51fbd…`), the C caller `tests/nvml/nvml_test.c`, `tests/nvml/abi.c` and
`tests/nvml/transport.c`.

## Gate

`make test-nvml CUDAPP_OMC_HOST=1 OMEGA_OMC=… OMEGA_BRIDGE=<bc11b30> OMEGA_CABI_BRIDGE=<9172b525>`
(`test-nvml.log`), rc 0, last line `CUDAPP-NVML HOST PASS`. It includes:

- `PASS cabi bridge pins: src/toc.om and tools/bridge-seeded match om/omega-bridge-cabi.lock`
- `PASS committed compat/nvml/libnvidia-ml.c, nvml.h and libnvidia-ml.map are the cabi bridge output of native/nvml/api.om`
- `PASS generated library libnvidia-ml.so.1 with the C caller: CUDAPP-NVML HOST PASS (476 checks)`
- `PASS abi: 55 facts equal to /usr/local/cuda/include/nvml.h`
- `PASS generated nvml.h prototypes (11) compile against the CUDA nvml.h` and its negative control
- `PASS ORACLE nm/readelf: libnvidia-ml.so.1 exports exactly the 11 api.om functions (T) with soname libnvidia-ml.so.1`
- `PASS ORACLE undefined-symbol guard ...` and its negative control (an object calling `ioctl` is rejected)
- the seeded Omega fake tier (476 checks), the oracle negative control (1 of 476) and 22 of 22 mutants caught.

## Negative controls (fresh clone; each mutation restored with `git checkout` afterwards, `clone-status-after.txt` empty)

| Log | Mutation | Expected | Observed |
| --- | --- | --- | --- |
| `test-nvml-unconfigured.log` | `make test-nvml` without `OMEGA_CABI_BRIDGE` | NOT_RUN, exit 2 | `test-nvml: NOT_RUN (requires ...)`, rc=2 |
| `gate-no-cabi-env.log` | `build/nvml_gate` with `OMEGA_CABI_BRIDGE` unset | 77 | `NOT_RUN: set OMEGA_CABI_BRIDGE ...`, rc=77 |
| `neg-wrong-toc.log` | copy of the #83 clone with one `note` line appended to `src/toc.om` | 69 | `REFUSED: OMEGA_CABI_BRIDGE src/toc.om digest is not the om/omega-bridge-cabi.lock pin`, rc=69 |
| `neg-lock-pin.log` | first hex digit of `CABI_CONVERTER_PIN` changed | 69 | same refusal, rc=69 |
| `neg-lock-row.log` | `CABI_FRONTEND_PIN` row removed | 74 | `REFUSED: om/omega-bridge-cabi.lock needs exactly one CABI_CONVERTER_PIN and one CABI_FRONTEND_PIN row`, rc=74 |
| `neg-lock-dup.log` (`neg-lock-dup.diff`) | second, all-zero `CABI_CONVERTER_PIN` row appended | 74 | same refusal, rc=74 |
| `neg-stale-lib.log` | one space added to `compat/nvml/libnvidia-ml.c` | 1 | `CUDAPP-NVML HOST FAIL (committed file is not the cabi bridge output: compat/nvml/libnvidia-ml.c)`, rc=1 |
| `neg-stale-map.log` | `nvmlShutdown;` dropped from `compat/nvml/libnvidia-ml.map` | 1 | `... compat/nvml/libnvidia-ml.map)`, rc=1 |

## The other portable units stay byte-identical (Manager(Aien) D2 condition)

- `units-check-b30.log`: `om/units.sh --check` with `OMEGA_BRIDGE` = bc11b30 regenerates every
  unit listed in `om/units.sh` (52 units, `units-count.txt`; 52 rows in
  `src/cudapp_om_units.ident`) plus `cudapp_native.h` and `cudapp_om_units.ident`, and compares
  each with `src/`: `PASS portable Omega units regenerate byte for byte`, rc=0. (The decision
  text says 53; the list holds 52, of which `nvml_step` and `nvml_errstr` are the
  NVML step and error-string units.)
- `units-diff-since-1fae7beb.txt`: `git diff --stat 1fae7beb HEAD -- src om/units.sh om/omega-bridge.lock`
  is empty: #365 changes no generated unit, no unit pin and not the main bridge lock.
- `units-check-b83-refused.log`: `om/units.sh --check` with the #83 bridge refuses
  (`NOT_RUN: toc.om is not the pinned converter`, rc=3), so the #83 converter cannot
  regenerate those units.
- `cabi-users.txt`: every tracked reference to `OMEGA_CABI_BRIDGE` or
  `om/omega-bridge-cabi.lock` is in `mk/nvml.mk`, `tests/nvml/gate.om` and the two
  READMEs; in `gate.om` the #83 bridge (`ng_bridgek`) is invoked only with
  `native/nvml/api.om`.

## NOT_RUN

CI (GitHub Actions blocked), hardware or live RM (no `/dev/nvidia*` access was
used), concurrent callers of the generated library from several threads, device
tier.
