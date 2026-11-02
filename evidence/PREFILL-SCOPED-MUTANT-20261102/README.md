# Scoped qgemm mutant refusal

Host result for the scoped-call mutant guard. No GPU device was opened. `make hello-gpu` was not run. `/dev/nvidiactl`, `/dev/nvidia0` and `/dev/nvidia-uvm` were absent.

## Fresh clone

The suite ran on `932d9548d3ca8584316c9f2cd3307d2ec32535d7`. This directory was added afterwards. It is not an input to `make`.

Cloned from the local repository into `/tmp/cudapp-fresh-259` before these commits were pushed:

```sh
git clone --no-hardlinks --branch perf/q38-scoped-reading <local checkout> /tmp/cudapp-fresh-259
cd /tmp/cudapp-fresh-259
git rev-parse HEAD
# 932d9548d3ca8584316c9f2cd3307d2ec32535d7
```

Machine: Linux 6.12.94+ x86_64, gcc 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1), 4 cores. `MemTotal` 16387280 kB. `MemAvailable` was about 5.1 GiB.

`make -j4 test` with `CUDAPP_MEM_RESERVE_MIB` unset refused the Qwen-shaped upload. The line is in [default-reserve.txt](default-reserve.txt). That run was stopped while `make` was still waiting for other jobs. It is not a completed suite result. The default reserve was not changed. The platform test in the completed run still checks the 4096 MiB default against a fake meminfo and prints `test_omega_gpu_platform_linux (reference only, simulated driver): 17 passed, 0 failed`.

Completed command, 2026-11-02T00:31:46Z to 2026-11-02T00:37:18Z:

```sh
CUDAPP_MEM_RESERVE_MIB=0 make -j4 test
```

Exit code 0. Last line: `CUDA++ HOST TESTS PASS (78 targets)`. The log is [make-test.log](make-test.log), sha256 `f32d60d0017ab4c467a39fdd7e45e63b59d8efcb659900aa25e535611ba9cfde`. x86_64 drops three AArch64-only rows and adds `test-omega-target-only`, so the count is 78. The log contains intentional `FAIL` lines from mutation tests and negative controls. The exit code and the last line are the result.

Inside that run:

- `scoped qgemm isolation: PASS`
- `ignore_scoped_options: caught`
- `leak_options_to_global: caught`
- `wrong_tile_height: caught`
- `drop_scoped_mutant_guard: caught`
- `bf16 sweep: all 2^32 inputs equal the hosted __bf16 cast`
- `gpu_serving_alloc_count_test` plain, reserve, bounds, bounds-block, rollback, sweep, pin, seal-bare and update: `PASS (0 failed checks)`

## Negative control before the guard

On the working tree, after `om/prefill-shape/scoped-check.c` required `OMEGA_GPU_PF_BAD_ARGS` and before `src/omega_gpu_pfgemm.c` grew the guard, the library was still the `6820ff0f` objects. `make -j4 build/prefill-scoped-check && ./build/prefill-scoped-check` exited 1 and printed:

```
scoped call refuses a kernel mutant before device access
scoped mutant rc -2 opened 1 keyed 1
scoped qgemm isolation: FAIL
```

Return code -2 is `OMEGA_GPU_PF_TOO_LARGE`. The wrap had opened the device path and accepted the unmutated key. After the guard, `make -j4 test-prefill-scoped` printed `scoped qgemm isolation: PASS` and caught `drop_scoped_mutant_guard`.

## 128K serving archive

Core receipt `om/ribbon/q38-adaptive-final-validation.json` at `e9366221747040a01805a67bb544e9f2d3c1d468` records `serving.long128.native_archive_sha256` `752564da817349a38ced23a8beda735f221afce78b1dce0a2e20ea09be1d5ce0` and `native_source` `88379395296357571618ebf7f701a6e2cdf8be31`.

`git diff --name-only 88379395296357571618ebf7f701a6e2cdf8be31 6820ff0f95ca798a44d42767e5ecb1ee6592eeeb -- src m16 nvrm third_party mk` is empty. The reviewed head only changes `om/prefill-shape/README.md`, `om/prefill-shape/adaptive-validation.json` and `om/prefill-shape/.crumb`.

`make -j4 all` on that head, same machine and gcc, produced x86-64 `build/libomega_gpu.a` and `build/libcudapp.a`, both sha256 `c6b7052b8eb84e8fbe9b4bc9f87a6216af4ff55cc0d94073c28ac188d135be5b`. That is not `752564da817349a38ced23a8beda735f221afce78b1dce0a2e20ea09be1d5ce0`. This compiler does not reproduce the GB10 archive bytes.

The tested head `932d9548d3ca8584316c9f2cd3307d2ec32535d7` changes `src/omega_gpu_pfgemm.c` and `src/omega_gpu_prefill.h` relative to `88379395`. Its fresh-clone `build/libomega_gpu.a` sha256 is `5677d44e1f494089ac62ec3d14ee1c01f988a8b1847f38f22757016ebf26a0a2`. The 128K serving archive does not match this head.

## Model-level mutant

`docs/REVIEWING.md` section 2.1, followed by convention until the proposed CODEOWNERS file is activated, requires a deliberately broken kernel or input to be refused or detected by the change's own test. `make test-prefill-scoped` is that test. It does not require a full-model score-compare red run. That run is a GB10 result in another repository, and this session has no authorized GPU window.
