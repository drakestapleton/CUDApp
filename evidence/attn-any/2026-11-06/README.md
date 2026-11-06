# Any-shape attention host evidence — 2026-11-06

Status: IMPLEMENTED; host/simulator EXERCISED as recorded below. Hardware QUALIFIED: NOT_RUN. MERGED / DEPLOYED: NOT_RUN. This directory records selected terminal output, not an invented raw transcript. Home paths are rendered as `~` in this receipt.

Source: branch `feat/attn-any`; attention implementation `06b2a82`; simulator predicate repair `732e10a`; prerequisite generic replay `22ebf8f`. Compiler omega-mv `e5b32567`, omc SHA256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. New API and kernel documentation: `om/attn-any/README.md`. Draft PR: https://github.com/aien-dev/CUDApp/pull/282.

## Seeded generation — PASS

Command (rc 0):

```sh
taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=~/work/cudastack/omega-bridge sh om/units.sh
```

Final line: `wrote checked portable Omega units to .../lanes/Attention/src`.

## Predicate regression — red then green

Command: `taskset -c 10-14 nice -n 10 make -j3 test-sim-predicates`.

Before the simulator repair: rc 2, final test line `warp-sim predicates: FAIL`.
After the repair: rc 0, final test line:

```text
warp-sim predicates: PASS (masked LDG/ST, unmasked negatives, guarded SHFL)
```

The regression checks masked lane memory operations against a one-word allowed range; deliberately unmasked LDG and STG must be rejected by the memory checker. Guarded SHFL must update only guarded destination lanes but can source a guard-inactive lane's pre-write register value.

## Existing simulator consumers — PASS

Job `bg_441`, rc 0, measured wall time 1279.14 seconds. Commands:

```sh
taskset -c 10-14 nice -n 10 make -j3 \
  test-gpu-decode-attn test-gpu-router test-gpu-bf16-gemv \
  test-gpu-fp8gemm test-gpu-cmdlist test-gpu-decode-attn256 test-gpu-gdn \
  build/gpu_gdn_multi_test build/gpu_exact_head_test
taskset -c 10-14 nice -n 10 build/gpu_gdn_multi_test --sim
taskset -c 10-14 nice -n 10 build/gpu_exact_head_test --sim
```

All nine existing simulator consumers found by include/dependency search passed, including their existing negative controls. No existing target depended on the incorrect unconditional default-op execution. Selected final simulator lines:

```text
gpu_router_test: 15 checks, 0 failed: PASS
gpu_bf16_gemv_test: 23 checks, 0 failed: PASS
gpu_fp8gemm_test: 21 checks, 0 failed: PASS
sim: 74 checks, 0 failed: PASS
gpu_decode_attn256_test: 14 checks, 0 failed: PASS
gpu_gdn_test: 46 checks, 0 failed: PASS
gpu_decode_attn_test: 12 checks, 0 failed: PASS
gpu_gdn_multi_test: 14 checks, 0 failed: PASS
gpu_exact_head_test: 9 checks, 0 failed: PASS
```

## Full host gate — NOT_RUN here

Main's updated full-suite policy directs lanes that have not started `make test` to skip it because concurrent lanes overload the shared build cores. Integrator owns the deadline-free full gate on `integ/omega-cuda-stack`, draft PR #280. No `CUDA++ HOST TESTS PASS (...)` line is claimed by this lane.

## Chip — NOT_RUN

No GPU window authorized; no chip execution performed. Runbook: `om/attn-any/CHIP.md`. Simulator success is not chip qualification and does not upgrade the ROADMAP row.

## Attention reference, shape sweep and source mutants — PASS

Job `bg_473` ran the following focused chain:

```sh
taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=~/work/cudastack/omega-bridge make -j1 \
  test-gpu-attn-any-mutants test-gpu-attn-any test-runtime-api \
  test-runtime-portability test-runtime-api-semver
```

Observed attention output:

```text
attn-any mutant KILLED: mask
attn-any mutant KILLED: group
attn-any mutant KILLED: normalize
attn-any mutant KILLED: tail
attn-any mutant KILLED: refusal
attn-any source mutants: PASS (5 killed after successful generation)
attn-any host+warp-sim: PASS (reference, shape sweep, refusals, wrong-mask control)
```

The chain reached and then failed `test-runtime-api` (overall rc 2, measured
wall time 795.76 seconds). The manifest's version constants were stale at 13
while its ABI line/header were 0.14.0, and the docs lacked the checker's
inline-code mention of the codegen export. Both registration defects were
corrected; the attention implementation and passing attention checks did not
change. The remaining API/portability targets are exercised separately.

## Runtime API, portability and semver — PASS after repair

Job `bg_504`, rc 0, measured wall time 990.47 seconds:

```sh
taskset -c 10-14 nice -n 10 make -j1 \
  test-runtime-api test-runtime-portability test-runtime-api-semver
```

Selected final lines:

```text
RESULT: PASS (runtime API 0.14.0: 307 functions, 48 structs, 465 fields, 240 constants match the headers; surface 744 names, 197 INTERNAL)
RESULT: PASS (every negative control was caught)
PASS: 152 rows, every omega_gpu_* / omega_blackwell_* / omega_bw_* / omega_ampere_* file in src/ has one and the rows agree with the includes
ALL CONTROLS BEHAVED AS REQUIRED
RESULT: PASS (version 0.13.0 -> 0.14.0; the entry changes need MINOR)
```

No attention implementation changes occurred between its passing shape/mutant
checks and this documentation/manifest registration repair.
