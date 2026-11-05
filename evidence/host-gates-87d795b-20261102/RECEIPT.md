# Host gates from a fresh clone of 87d795b

## What this folder is

The commands below ran from a fresh clone of `87d795b4446a762a0b12e05721f41b8abbde0c36`. That is the head that was tested. The commit that adds this folder only adds these evidence files. It does not change code, docs, workflows, or any older receipt.

`evidence/ampere-r5-sm86-refusal-20261102/RECEIPT.md` still matches its `1c3d33f` bytes. The CI cancellation note that must not be edited into that file is in `evidence/host-gates-84553c1-20261102/RECEIPT.md`.

Machine: Linux 6.12.94+ x86_64, gcc 13.3.0. No GPU was opened. Date: 2026-11-02.

## Commands and exit codes

```
make test-native-artifact
```

Exit 0. `test_gpu_artifact_export: 47 checks, 0 failed`.

```
make zero-cuda-gate
```

Exit 0. `ZERO_CUDA SELF-TEST PASS (CUDA probe rejected, clean library accepted)`. `ZERO_CUDA PASS build/hello_gpu needs=[libm.so.6 libc.so.6]`. `ZERO_CUDA PASS build/gpu_matmul_api_test needs=[libc.so.6]`.

```
build/hello_gpu --sim
```

Exit 0. Last line `PASS`. `checked 1048576 of 1048576 elements: 0 wrong, 0 never written`. Host simulator, not a GB10 result.

```
make doctor
```

Exit 2. The doctor printed `NOT READY: 5 problems above` (no NVIDIA kernel module, no `/dev/nvidiactl`, no `/dev/nvidia0`, no `/dev/nvidia-uvm`, no NVIDIA GPU in sysfs). That is the expected diagnostic on this machine. The recipe returns 1, and GNU make exits 2. The seeded target table printed `NOT_RUN` because `OMEGA_OMC` was not set. No device was opened.

## Why 80 targets and 83 targets are both right

`make test` prints `CUDA++ HOST TESTS PASS ($(words $(HOST_TESTS)) targets)`. Neither count is a stale number.

On x86-64 with `OMEGA_OMC` unset, `Makefile` drops `test-om-spec-encoder`, `test-portable-receipts`, and `test-model-capacity`, and adds `test-omega-target-only`. `mk/q38-head.mk` leaves `test-q38-head-bounds` out of `CARRIED_HOST_TESTS` unless `CUDAPP_OMC_HOST` is set. That list is 80. The receipts on `d7c2e25` and `1c3d33f` ran `make test` that way.

`.github/workflows/ampere-x86.yml` exports `OMEGA_OMC` before `make test`. `CUDAPP_OMC_HOST` is then 1, so those three targets stay, `test-omega-target-only` is not added, and `test-q38-head-bounds` is added. 80 minus 1 plus 3 plus 1 is 83. That is the line in the native-x86-host log. `.github/workflows/host-x86.yml` does not export `OMEGA_OMC`, so that job stays at 80.

## Not run

GB10 chip runs: NOT RUN. BLOCKED_OPERATOR. No authorized GPU window.

`ATOMG.E.ADD.F32.FTZ.RN.STRONG.SYS`, `ATOMG.E.ADD.F32.FTZ.RN.STRONG.GPU`, and destination presence slot 15 stay in the tree. `src/omega_blackwell_codegen.h` is byte-identical to omega-mv `19b8340d` and declares opcodes 62 and 63. Removing those declarations makes that file differ, and omega's blob is not the baseline blob, so the drift gate goes red. The presence slot `put o 15 pres d` is outside the drift path set (`om/` is not compared). Omega `19b8340d` has the same line. Removing it does not by itself turn the drift gate red, and it does make the hosted golden-word check for a negative destination (RZ, not R0) fail. That check stays. Both are NOT QUALIFIED until a GB10 receipt exists (`docs/status.md`).
