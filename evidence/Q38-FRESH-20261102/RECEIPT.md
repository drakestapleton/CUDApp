# Fresh-clone host gates for the Qwen3.8 head

Cloned from `https://github.com/aien-dev/CUDApp.git` and checked out detached
`6e2adc77fef9ff1af8257fe525f37deacc42f59e`. `git status --porcelain` was empty.
Started 2026-11-02T00:39:39Z. Linux 6.12.94+ x86_64, gcc 13.3.0, GNU Make 4.3.
No GPU device was opened.

`CUDAPP_MEM_RESERVE_MIB=0` on this VM only. MemAvailable under-reports because
of the hypervisor balloon, and the default 4096 MiB reserve then refuses the
Qwen-shaped upload. The platform test unsets the variable and still checks
the 4096 default. This is not a Spark setting.

This directory was added after that run. It is not an input to `make`.
The GB10 serving screen is not this receipt.

| Gate | Result | Exit |
|---|---|---|
| `make -j4 test` | PASS. Last line: `CUDA++ HOST TESTS PASS (73 targets)` | 0 |
| `make -j4 test-native-artifact` | PASS. `test_gpu_artifact_export: 47 checks, 0 failed` | 0 |
| `make -j4 zero-cuda-gate` | PASS. `hello_gpu` needs libm and libc. `gpu_matmul_api_test` needs libc. The self-test rejected a CUDA probe | 0 |
| `./build/hello_gpu --sim` | PASS. 1048576 elements, 0 wrong. machine-code sha256 `9e28bd9c46f80584d77238a752e99cecddbc8d622ffbe320885ed2f499e1f075` | 0 |
| `make doctor` | NOT READY, expected on this VM. No NVIDIA module and no `/dev/nvidia*`. GNU make exits 2 when that recipe fails | 2 |
| GB10 chip and serving | NOT RUN | |

`make test` logs contain intentional `FAIL` lines from mutation tests and
negative controls. The exit code and the last line are the result.
