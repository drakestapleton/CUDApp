# Security hardening host receipt

Fresh clone of `b198cdf18a0787f74049aceaac01f25c6891a3f8` on 2026-11-02.
This directory was added afterwards. It is not an input to `make`.

```sh
git clone --branch cursor/security-hardening-baseline-0743 https://github.com/aien-dev/CUDApp.git
git rev-parse HEAD
```

`git rev-parse HEAD` printed `b198cdf18a0787f74049aceaac01f25c6891a3f8`.
The clone had no `build/` directory.
Host: Linux 6.12.94+ x86_64, gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0, GNU Make 4.3.
`awk '/MemAvailable|MemTotal/{print}' /proc/meminfo` printed `MemTotal: 16387280 kB` and `MemAvailable: 5059312 kB`.
`CUDAPP_MEM_RESERVE_MIB=0` for this run. The source default stays 4096. On this VM the balloon leaves MemAvailable too low for that reserve, and `test-gpu-serving-alloc` then refuses the serving upload. The platform test still checks the 4096 default against a fake meminfo.
No `/dev/nvidia*` node was opened. GB10 hardware tests are NOT RUN.

Started 2026-11-02T01:00:24Z. Finished 2026-11-02T01:09:07Z.

## Gates

`CUDAPP_MEM_RESERVE_MIB=0 make -j4 test` exited 0.
Last line: `CUDA++ HOST TESTS PASS (73 targets)`.
The log contains intentional `FAIL` lines from mutation tests and negative controls. The exit code and the last line are the result.
Inside that run:

- `test-zero-cuda-tool-argv: PASS`, including `PASS tmpdir removed`
- `bf16 sweep: all 2^32 inputs equal the hosted __bf16 cast`
- `test-om-spec-encoder`, `test-portable-receipts`, and `test-model-capacity` printed NOT_RUN (pinned AArch64 omc; `nvdisasm` is not installed here). Those three are not counted as passes. `test-omega-target-only` reports them and exits 0.

`make test-native-artifact` exited 0.
Last line: `test_gpu_artifact_export: 47 checks, 0 failed`.

`make zero-cuda-gate` exited 0.
The log shows `ZERO_CUDA FAIL` for the stub probe, then `ZERO_CUDA SELF-TEST PASS`, then `ZERO_CUDA PASS` for `build/hello_gpu` and `build/gpu_matmul_api_test`.

`./build/hello_gpu --sim` exited 0.
1048576 elements, 0 wrong, output sha256 `72d0c1b2928779e83518f0b4c7d101d933cf89f210cd4627e138c33c9ce9b6a6`, last line `PASS`.

`make doctor` exited 2.
The log contains `NOT READY: 5 problems above` (module not loaded, `/dev/nvidiactl`, `/dev/nvidia0`, `/dev/nvidia-uvm`, no PCI GPU) and `NOT_RUN` for the seeded target table.
It does not contain `READY. Next: make hello-gpu`.
Make prints `Error 1` because the doctor process is non-zero. Make's own status is 2. That is the expected diagnostic on this machine.

`python3 tools/figures/make_figures.py --check` exited 0.
Last line: `figures current: 10`.

GB10: NOT RUN.
