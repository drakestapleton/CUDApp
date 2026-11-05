# Host gates from a fresh clone of 84553c1

## What this folder is

The commands below ran from a fresh clone of `84553c1a429fec094d719641b2fb5315b4190ec4`. That commit is the code under test: `docs/status.md`, the restored receipt, and the f32 atomic comment. The commit that adds this folder only adds evidence files. It does not change code, docs, or workflows.

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

Exit 0. Last line `PASS`. `checked 1048576 of 1048576 elements: 0 wrong, 0 never written`. This is the host simulator, not a GB10 result.

```
make doctor
```

Exit 2. The doctor printed `NOT READY: 5 problems above` (NVIDIA kernel module not loaded, `/dev/nvidiactl`, `/dev/nvidia0`, and `/dev/nvidia-uvm` missing, no NVIDIA GPU in sysfs). That is the expected diagnostic on this machine. The recipe returns 1, and GNU make then exits 2. The seeded target table printed `NOT_RUN` because `OMEGA_OMC` was not set. No device was opened.

```
bash tools/extract/drift.sh --self-test <omega> 19b8340dbae70888c49fcc98299be928831bfd26
```

Exit 0. `DRIFT-GATE: PASS (920 GPU paths, omega 19b8340dbae7, none moved away from CUDA++ since the baseline)`. `SELF-TEST: PASS (all controls red, real check green)`.

## Why receipts say 80 targets and a CI x86 log says 83

`make test` prints `CUDA++ HOST TESTS PASS ($(words $(HOST_TESTS)) targets)`. The count is the manifest, not a fixed number.

On x86-64 with `OMEGA_OMC` unset, `Makefile` drops `test-om-spec-encoder`, `test-portable-receipts`, and `test-model-capacity`, and adds `test-omega-target-only`. `mk/q38-head.mk` leaves `test-q38-head-bounds` out of `CARRIED_HOST_TESTS` unless `CUDAPP_OMC_HOST` is set. That list is 80 targets. The receipts on `d7c2e25` and `1c3d33f` ran `make test` that way.

`.github/workflows/ampere-x86.yml` exports `OMEGA_OMC` before `make test`. `CUDAPP_OMC_HOST` is then 1, so the three targets stay in the list, `test-omega-target-only` is not added, and `test-q38-head-bounds` is added. 80 minus 1 plus 3 plus 1 is 83. That is the `83 targets` line in the native-x86-host log. `.github/workflows/host-x86.yml` does not export `OMEGA_OMC`, so that job stays at 80.

## Note that used to sit in an older receipt

`89ba4e8` appended a "CI on 1c3d33f" section to `evidence/ampere-r5-sm86-refusal-20261102/RECEIPT.md`, which `1c3d33f` had already committed. `84553c1` restores that file byte for byte to the `1c3d33f` content. The note belongs here.

GitHub Actions run 41387070372, job `native-x86-host`, commit `1c3d33fa7c3ef4c8fcff9010632136755a1392a4`, was cancelled at the 40 minute job cap. The log had already printed `CUDA++ HOST TESTS PASS (83 targets)` and `Ampere extended existing generator families encoded (host only)`, then `python3 tools/ampere_nvdisasm.py` ran until `The operation was canceled`. `89ba4e8` raised that job's budget to 60 minutes, the same budget `host-x86.yml` gives the x86 suite. The commands and the corpus compare were not changed.

## Not run

GB10 chip runs: NOT RUN. BLOCKED_OPERATOR. No authorized GPU window.

`ATOMG.E.ADD.F32.FTZ.RN.STRONG.SYS`, `ATOMG.E.ADD.F32.FTZ.RN.STRONG.GPU`, and destination presence slot 15 are NOT QUALIFIED (pending a GB10 receipt). See `docs/status.md`. `tests/gpu_atomf32_probe_test.c` was not run on a chip.
