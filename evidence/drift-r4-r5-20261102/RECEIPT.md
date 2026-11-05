# Drift port of omega-mv train r4 and train r5

Host receipt for the CUDA++ copies of the GPU paths that `tools/extract/drift.sh` compares with omega-mv main.

## Commit and machine

- Code measured: `d7c2e259995002ddb1ae1fa564c1bfb97cc833f4` (branch `cursor/port-omega-train-r4-d95b`).
- Omega compared: `19b8340dbae70888c49fcc98299be928831bfd26` (Merge #996 train r5, which contains Merge #995 train r4 `f9a9a40b598928fc6f92d7ffa6ff386941182663`).
- Machine: Linux 6.12.94+ x86_64, gcc 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1). No GPU. `CUDAPP_MEM_RESERVE_MIB=0` for the suite, because MemAvailable on this VM under-reports.
- Date: 2026-11-02.

## Commands

Fresh clone of the code commit, then:

```
CUDAPP_MEM_RESERVE_MIB=0 make -j8 test
```

Last line: `CUDA++ HOST TESTS PASS (80 targets)`. Exit 0. See `make-test-tail.txt`. The suite log contains intentional `FAIL` and `KILLED` lines from mutation tests and negative controls. Judge the exit code and the last line.

Drift, from the same commit, omega checkout at `19b8340dbae7`:

```
tools/extract/drift.sh --self-test <omega> 19b8340dbae70888c49fcc98299be928831bfd26
```

Result: `DRIFT-GATE: PASS (920 GPU paths, omega 19b8340dbae7, none moved away from CUDA++ since the baseline)` and `SELF-TEST: PASS (all controls red, real check green)`. Exit 0. See `drift-self-test.txt`.

Before the port, the same command against omega `f9a9a40b5989` (train r4 only) printed `DRIFT-GATE: FAIL (16 of 914 GPU paths ...)`. After r5 landed, the unported tree printed `DRIFT-GATE: FAIL (35 of 920 GPU paths ...)`.

## What was ported

- Train r4 (`f9a9a40`): LDG.E immediates, KT1 coalesced stores, Qwen shape and bf16 layer path on the q38 graph.
- Train r5 (`19b8340`): SHFL.BFLY, F2FP.BF16.F32.PACK_AB, ATOMG.E.ADD.F32.STRONG.SYS and .STRONG.GPU, the prefill and training-GEMM sources, and the matching cases in `om/encoder/bw_enc_sm121.om`.
- SPDX lines and CUDA++-ahead code stay. Where the bytes still differ, `tools/extract/drift-baseline.tsv` records omega's blob at `19b8340`.

## Not run

- The sm_121 cases live in `om/encoder/bw_enc_sm121.om`. `om/units.sh` regenerates the portable bindings from the pinned omc (`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`, omega-mv `e5b32567`) and omega-bridge `3dc4a00e` (`om/omega-bridge.lock`). `make test-om-encoder` (inside `make test`) checks the result: golden words pass, and the `.om` binding and the C encoder agree on 145 instructions and 14 programs.
- GB10 chip runs: BLOCKED_OPERATOR. No authorized GPU window. That includes the KT1 coalesced kernel, the f32 atomic-add probe, and the training GEMM chip gate.
- EXP and LOG whole-program kernels: not copied. They live in omega-only `src/omega_numeric_divsqrt_gb10.c`, which is outside the drift path set. `omega_numeric_submit_check` still refuses EXP and LOG. A chip receipt is required before those flags can say a kernel exists here.
- `tests/gpu_tern_test.c` and the new Omega Makefile targets: not copied. The ternary host test's reference is Omega's algebra oracle. The Makefile comment already says so. Host checks of the coalesced predicate are in `tests/gpu_tern_sched_test.c`. Mutant 4 stays the I2FP refusal already tested here. Omega's slot-race mutant is 5 here, and its clamp mutant is 6.
