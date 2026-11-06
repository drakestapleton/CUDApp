# C3 GEMM host/simulator receipt

Recorded 2026-11-06T03:06:09Z on Linux AArch64. IMPLEMENTED and EXERCISED on the host; hardware qualification is NOT_RUN. Nothing was merged or deployed. This receipt does not upgrade ROADMAP C3 or the cuBLAS row to PASS.

## Source and tools

- Exercised implementation and test source: `cc5c929afbd21bdc76e235a6d15171242013919f`, branch `feat/omega-gemm-any`, based on CUDApp `b774723f537cec16a359fe082b8100317a5b5490`.
- Seeded compiler source: `e5b32567a7aff3ce594e3254a622a846a6cd190b`.
- Compiler binary SHA256: `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Bridge source: `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4`.
- No device words or candidate context compiler/bridge pair are required. The prebuilt compiler and bridge checkouts were not rebuilt or edited.

Paths below use `$HOME/work/cudastack`; this normalizes the local home path, not command semantics.

## Commands and observed results

Generation (rc=0):

```
taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc" \
  OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge" sh om/units.sh
```

Final generation output: `wrote checked portable Omega units to .../src`. The new binding is generated from `om/gemm-any/gemm_ir.om`, not hand-written C. Earlier development generation attempts encountered seeded error 12 (live-value pressure) and bridge error 11 for an unsupported trap statement; helpers and explicit metadata error returns replaced those failed versions before the successful generation and exercise.

Final focused acceptance (rc=0; stdout `build/gemm-any-focused.log`):

```
set -o pipefail
taskset -c 10-14 nice -n 10 make -j3 \
  test-gpu-gemm-any test-gpu-gemm-any-differential test-gpu-gemm \
  | tee build/gemm-any-focused.log
```

Observed verdicts:

```
C3 gemm sim: PASS (239 checks)
GEMM ANY SIM SWEEP PASS (286 distinct shapes, 2288 multiplies, 2552824 output words, bitwise model + per-K bound)
GEMM ANY MUTANT CAUGHT bf16 dropped K tail (149 differing output words)
GEMM ANY MUTANT CAUGHT bf16 off-by-one edge predicate (25 differing output words)
GEMM ANY MUTANT CAUGHT e4m3 dropped K tail (151 differing output words)
GEMM ANY MUTANT CAUGHT e4m3 off-by-one edge predicate (25 differing output words)
GEMM ANY HOST/SIM/MUTANTS PASS
GEMM ANY C/OMEGA SIM-EQUAL PASS (not byte-identical kernels)
```

The 286 dimension triples include each axis at 1 and 8192, coupled dimensions, and asymmetric multi-tile cases. Every triple runs BF16 and E4M3 in all four layouts; every output is compared against the bitwise tensor model and the independent double-sum bound. See README.md for exact accumulation rules, finite-input limits and quoted Wolfram queries. The C-oracle executable independently passes the same host checks, then its per-shape output digests match the Omega executable. The existing general-config simulator reports zero wrong multiplies for its tile/warp/band configurations and catches its six mutants. This establishes simulated output equality, not identical kernel bytes or equivalent hardware scheduling/performance.

One complete-suite attempt:

```
set -o pipefail
taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc" \
  OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge" \
  OMEGA_BRIDGE_DIR="$HOME/work/cudastack/omega-bridge" \
  OMEGA_MV="$HOME/work/cudastack/omega-mv" make -j3 test \
  | tee build/gemm-any-make-test.log
```

Full tier: **INCONCLUSIVE**. The tool terminated the command at its 3600-second deadline; no numeric process return code was returned and no final `CUDA++ HOST TESTS PASS` line was observed. The interrupted targets were `test-gpu-engine-backend-mutants`, `test-gpu-decode-attn-maxpos`, and `test-gpu-attention`; this is not a full-suite PASS. Local stdout is `build/gemm-any-make-test.log`; combined tool output is `artifact://1455`. Main explicitly directed no local retry because the cores were saturated and assigned the deadline-free integrated full gate to Integrator. The focused final command above completed after this interruption.

## Fingerprints

SHA256:

```
09545e0c2a3101582b4b7c0322eb1598e641db86feac96989a9d9f1db400c5a0  om/gemm-any/gemm_ir.om
f5ded11f18a445adb63e8faab26ea76f77bcf8776748567fa0c1b4bb10f3cf90  src/omega_gpu_gemm.c
8ad3720cdbd852d5b7125994e99512cacc9cf7b6f6823655d1cd654ed0d18bdf  tests/gpu_gemm_any_test.c
d9a96a88f76bfabbc6097c069cd831ddef97f772f738d500b8395adc607edf3c  build/gpu_gemm_any_test
dd666fd4eee8c7da00ace5b81aba7eb863501be86660a2228da017299ca317c6  build/gpu_gemm_any_c_oracle
f1b774351315a29d58f53fc78cb777ee837ba4746c6e58e843b0a70587fdde5d  build/gemm-any-focused.log
cabd061afdc66a08bb12c448d2977c5686b5bc4941f6294f6ef07bc000d149a8  build/gemm-any-make-test.log
```

Build artifacts and stdout logs are local, not committed binaries or immutable historical evidence. The final receipt/documentation commit does not change the exercised implementation or test source.

## Remaining gates

- Integrated `make test`: INCONCLUSIVE here; Integrator owns the deadline-free gate.
- Chip sweep, numerical mutants, measured ceilings and speed fractions: NOT_RUN. The exact authorized-window procedure is in README.md. No GPU access was authorized for this lane.
- Merge and deployment: not authorized and not performed.

Guide-impact check: this change uses existing seeded syntax, the pinned compiler/bridge, existing encoders and the established repository boundary. It changes no compiler feature, bridge ABI, host word, ownership or harness contract; no omega-harness skill change is required.
