# Scoped qgemm mutant refusal, follow-up on main

Host result after cherry-picking the #259 review fixes onto `183fa4f`. No GPU device was opened. `make hello-gpu` was not run. `/dev/nvidiactl`, `/dev/nvidia0` and `/dev/nvidia-uvm` were absent. No GB10 run.

## Fresh clone

The suite ran on `2b117d83e2303466453e5615f8beee810911acba`. This directory was added afterwards. It is not an input to `make`.

```sh
git clone --no-hardlinks --branch fix/prefill-scoped-mutant <local checkout> /tmp/cudapp-fresh-main
cd /tmp/cudapp-fresh-main
git rev-parse HEAD
# 2b117d83e2303466453e5615f8beee810911acba
```

Machine: Linux 6.12.94+ x86_64, gcc 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1), 4 cores. `MemTotal` 16387280 kB. `make doctor` reported 15.6 GiB system memory and 4.7 GiB available.

Completed commands, 2026-11-02T00:46:39Z to 2026-11-02T00:52:17Z:

```sh
CUDAPP_MEM_RESERVE_MIB=0 make -j4 test
make -j4 test-prefill-scoped
make doctor
```

`CUDAPP_MEM_RESERVE_MIB=0 make -j4 test` exited 0. Last line: `CUDA++ HOST TESTS PASS (80 targets)`. The count is 80 because this tree is `main` plus the cherry-picks. The earlier receipt at `932d954` printed 78 targets. The log is [make-test.log](make-test.log), sha256 `1186d295b0dbebf4a1a11a673ec1fb1a755b92671e3ae46b28f508b25ee50f04`. The log contains intentional `FAIL` lines from mutation tests and negative controls. The exit code and the last line are the result.

`make -j4 test-prefill-scoped` then exited 0. It printed `scoped qgemm isolation: PASS` and caught `ignore_scoped_options`, `leak_options_to_global`, `wrong_tile_height` and `drop_scoped_mutant_guard`. The same four lines also appear inside the full suite.

`make doctor` printed NOT READY (no NVIDIA module and no `/dev/nvidia*`). The recipe exited 1 and `make` exited 2. That is the expected diagnostic on this machine. The suite's fake-machine doctor printed `doctor tests: 9 passed, 0 failed`.

Inside the full suite:

- `hello --sim tests: 12 passed, 0 failed`
- `test_gpu_artifact_export: 47 checks, 0 failed`
- `ZERO_CUDA PASS build/hello_gpu` and `ZERO_CUDA PASS build/gpu_matmul_api_test`
- `ZERO_CUDA SELF-TEST PASS (CUDA probe rejected, clean library accepted)`

## 128K serving archive

Core receipt `serving.long128.native_archive_sha256` is `752564da817349a38ced23a8beda735f221afce78b1dce0a2e20ea09be1d5ce0` at native source `88379395`. This fresh clone's `build/libomega_gpu.a` sha256 is `bc2093fab1a3bb4287c048e555befc722b80d8cc38f0a924ee78553c86a9afd7`. The 128K archive does not match this head. This compiler is x86_64 gcc 13.3.0, and this head also changes `src/omega_gpu_pfgemm.c` relative to `88379395`.
