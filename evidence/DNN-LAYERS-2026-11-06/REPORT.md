# Seeded attention and DNN layers — 2026-11-06

## Candidate and scope

- Branch: `feat/dnn-layers-om`; code candidate `1eaa654`.
- Draft: https://github.com/aien-dev/CUDApp/pull/307, based on `feat/attn-any` / draft #282 (`23ab86c`).
- The stack includes CodegenA generic replay `22ebf8f`, C3 scalar-GEMM work #283, and the isolated C1 encoder foundation `3bf3ac4f71910fdae2de1f79ca36446b2dd0f526`. It does not import the unfinished full C1 component.
- Native-half seam: `9be13fb2595f1b3668288b36c4d17ef976bcb3a7`. Replay-bound repair: `a8c6a22`. Scalar simulator guard repair and dual-header fixture: `a3a9e98`.
- Rows: `9885f4a60db30d4387fc353672e772c56cf0222e`; activations: `a3e2d2b63a7cacd1d7b10fbcaa4cd75cb9a5e70b`; convolution/shared scalar GEMM: `1f2e630e20bf536966aa6f20d0030e5973efff24`.

IMPLEMENTED means the first-party planners and kernel emitters are seeded Omega, committed generated C is its bridge output, and the C APIs perform allocation, pointer stepping, replay and launches. It does not mean hardware qualification, merging or deployment. Tuned attn/pattn/attn256/pattn256 work is not part of this implementation. ROADMAP statuses remain unchanged; the additions are dated notes only.

### Implemented operators

- Attention: independent query/key lengths, including one and non-tile tails; head dimensions 1–256; GQA with KV-head divisibility; causal or noncausal mask; caller softmax scale; online f32 maximum/sum/output accumulation. Inputs f32, bf16 or native f16; output f32. The seeded planner owns admission, ownership, offsets, sizes and launch geometry.
- Row softmax, pure layer normalization and RMS normalization: lengths 1–65535, f32/bf16/f16 inputs and f32 accumulation/output. Norm epsilon must be positive, finite and normal f32; softmax ignores epsilon. Affine gamma/beta are not silently included. Finite-input/intermediate requirements and the tested MUFU envelope are documented, not claimed as a universal error proof.
- ReLU, erf GELU, tanh GELU and SiLU: f32/bf16/f16 inputs and f32 output, seeded count admission and tail handling. GELU and SiLU use documented stable finite-extrema forms and signed-zero behavior.
- Convolution: implicit GEMM over NCHW or NHWC inputs and OIHW weights, batch, groups/depthwise, asymmetric padding, stride, dilation and spatial/channel tails. No im2col allocation. It calls the same seeded `gs_dot` reduction as the default scalar f32 GEMM path. The old handwritten scalar builder remains only behind its default-OFF migration switch.
- Public headers, runtime API 0.15.0, manifest, API reference and operator guides are updated together.

Attention's independent naive double reference and its bounded fixture tolerance are documented in `om/attn-any/README.md`. The recorded Wolfram query `N[16 ((256 2^-24)/(1-256 2^-24)+(513 2^-24)/(1-513 2^-24)+2^-20)]` returned `0.0007486530236091783`; `N[2^-10]` returned `0.0009765625`, the chosen absolute test envelope. This is MODELED for the documented dyadic fixture domain, not a universal numerical proof or a measured chip error.

## Pins and native f16 encoding

Compiler source `omega-mv@e5b32567`; omc SHA256:

```
2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
```

Bridge checkout: `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4`. The checked bridge inputs enforced by `om/units.sh` are:

```
toc.om        69d629df170a24ba0c2cb6d41dbfaed520e248e0d723236dd32abf0581524d84
bridge-seeded 23ff7a2356bb5d980060a4685a27ac46587013069b7d24b37c2873d7ee659b37
freest.om     836cbf9ef8a3c4ee82b0458070884405bc8296f83b10f933556bf27750bb3661
bridge-freest 51b41794e7001c5606b715fd7de8a34ec935a6e4e4bc1da36ad08102c009537d
```

IR 64/65 are the C1 unsigned conversions; 66 is f16-to-f32 and 67 is f32-to-f16, making `BW_IR_LAST` 67 in this profile. Replay uses `BW_IR_LAST`, not the obsolete literal 63. Its boundary fixture accepts LAST and rejects LAST+1; the compiled reverting mutant is part of `test-om-ir-boundary` (there is no separate `test-om-ir-boundary-mutant` target).

The native forms follow the encoder's existing `E1_CVT` sm_121 authority in `src/omega_numeric.c`:

- Widen: `HADD2.F32 Rd, -RZ, Ra.H0_H0`, fixed latency. Low half to f32; the negative zero addend preserves signed-zero behavior.
- Narrow: `F2F.F16.F32 Rd, Ra`, variable latency, RN-even, low-half result with cleared upper half.

The fixture compares both scalar simulator headers against an independent integer E1 reference over all half bit patterns, finite roundtrips, deterministic f32 narrowing patterns, register pairs and guard modes. Predicated execution is exercised with all-off, all-on and alternating predicates. For SHFL, destination execution is limited to guarded lanes, while guarded-off lanes still supply source data; an entirely false guard skips execution. Existing split/exited source restrictions remain explicit.

## Observed verification before final integration

The initial native-half offline oracle is historical: it predates the mandatory namespace policy. It is not relabelled as a namespace run. At seam `9be13fb`, Disasm `672e891` plus nvdisasm 13.0 V13.0.85 checked 24 native words with zero failed kernels and `VERDICT PASS` (artifact://3582, rc 0). The native-half compiled mutants and regeneration passed in artifact://3669 (rc 0). No GPU or chip qualification follows from disassembly.

Attention's namespace-isolated safety acceptance at `b4c616c` returned rc 0 (artifact://4303): the separate fake NULL/non-NULL session contract passed; the quick independent reference/simulator/refusal/wrong-mask controls passed; six compiled seeded mutants were killed; and the forced session-open test-source mutant died at its abort guard with status 134. The earlier numerical attention receipt artifact://3890 is historical and predates the namespace policy.

Two integrated attempts found and repaired real defects rather than suppressing checks:

1. artifact://4705, rc 2: API documentation indexing required explicit backtick function names in prose in addition to exact C signatures. Fixed in `41cbe5c`. The portable binding check, MINOR semver check and all four MUFU simulator FTZ checks passed in this attempt; this does not make the overall attempt PASS.
2. artifact://4717, rc 2: the new default-header native-half fixture initialized a `present` field that exists only in the alternate scalar simulator. Fixed in `1eaa654` by making that initialization header-specific. The API checker and all its negative controls passed before this compile failure:

```
RESULT: PASS (runtime API 0.15.0: 314 functions, 51 structs, 491 fields, 266 constants match the headers; surface 777 names, 197 INTERNAL)
RESULT: PASS (every negative control was caught)
```

## Safety correction: historical reachability is not historical non-access

The original Rows epsilon-admission mutant had a source-proven path from `--sim`/`--quick` refusal checks into `omega_gpu_session_open`, through platform device-open to `nvrm_open`. At original Rows revision `812a8c5`, the refusal fixture called the execution API, and widening epsilon admission admitted +Inf. The copied inverted pointer check did not prevent evaluating the opener. Reachable nodes were `/dev/nvidiactl`, `/dev/nvidia-uvm` and `/dev/nvidia%u` using the driver-reported minor; this does not establish that an open or launch succeeded. The source trace was sent to Main. Passive current lock/listener/holder state cannot establish historical non-access.

The final attention, Rows, activations and convolution fixtures link aborting session open/lock/unlock/free/allocation wrappers for simulator modes, and each includes a forced-opener negative control. The production memory shells reject NULL sessions correctly. A separate explicit fake pointer-contract mode never forwards to devices. The wrappers supplement, not replace, the device-free namespace.

All post-policy execution commands use:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 <command>
```

This exposes a minimal `/dev`, not the host NVIDIA, CUSE or VFIO nodes. A missing device node on a simulator path is a defect to repair, not permission to rerun outside the namespace. No device probes, lock manipulation, GPU workloads or service changes are part of this lane's acceptance.

## Integrated focused acceptance

At code candidate `1eaa654`, the following command completed with rc 0 (artifact://4794, 433.35 seconds). This is focused host/simulator acceptance, not the full `make test` gate:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge \
  OMEGA_MV=/home/drakestapleton/work/cudastack/omega-mv \
  OMEGA_BRIDGE_DIR=/home/drakestapleton/work/cudastack/omega-bridge \
  make -j3 \
    test-dnn-half test-dnn-half-mutants test-om-ir-boundary \
    test-gpu-attn-any test-gpu-attn-any-mutants \
    test-gpu-dnn-rows test-gpu-dnn-rows-mutants \
    test-gpu-dnn-activations test-gpu-dnn-activations-mutants \
    test-gpu-conv2d test-gpu-conv2d-mutants test-gpu-conv2d-gemm-f32 \
    test-gpu-mathlibs test-gpu-reconv test-gpu-attention test-gpu-fp8 \
    test-gpu-scan test-gpu-streams test-gpu-isolation
```

Observed results:

| Tier | Verdict | Observed evidence |
|---|---|---|
| Native half, both scalar simulator headers | PASS | Both fixtures print `dnn-half: PASS (native words, predicates, exhaustive half reference, narrowing controls)` |
| Native encoder mutants | PASS | Five compiled seeded mutants killed |
| Replay bound | PASS | LAST accepted, LAST+1 refused; compiled obsolete-bound mutant killed |
| Attention independent double reference / warp simulator | PASS | Full shape/tail/type/GQA/mask sweep, refusal and wrong-mask control; fake pointer contract |
| Attention source mutants | PASS | Six seeded mutants plus forced session-open abort control killed |
| Rows reference / simulator | PASS | All lengths/types/epsilon, constant and varied rows, tail ranges, refusals, wrong-result and fake pointer controls |
| Rows source mutants | PASS | Five numerical/address failures, widened-admission abort and direct-open abort; all compiled |
| Activations reference / simulator | PASS | Four operators, three input types, tails, double references, refusal and wrong-result controls; fake pointer contract |
| Activation source mutants | PASS | Seven compiled seeded mutants killed; compiled direct-open mutant aborts under `--sim` |
| Convolution reference / simulator | PASS | 1454 checks, zero failures; separate fake pointer contract 27 checks, zero failures |
| Convolution source mutants | PASS | Five compiled seeded mutants killed; direct-open mutant SIGABRT before forwarding |
| Shared scalar dense-GEMM differential | PASS | Seeded and legacy-oracle simulator logs identical; digest `93efbbc6b4debf83`; 1290 checks, zero failures |
| Existing scalar simulator users | PASS | Mathlibs, reconvergence, model-shape attention/general sweep, fp8, scan, streams and isolation targets complete successfully |

The existing users did not expose reliance on the repaired unguarded default execution. The model-shape attention target still catches its deliberate split-warp SHFL and out-of-bounds shared-memory controls; these diagnostic lines are expected negative controls, not suppressed failures. Its final line is `PASS: 348 checks, 0 failed`. All four MUFU simulator variants passed in the earlier namespace-wrapped attempt (artifact://4705). None of these results is a hardware timing or qualification result.

## Canonical generation

The fully composed unit set regenerated byte for byte after focused acceptance (session job `bg_981`, rc 0):

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge \
  OMEGA_MV=/home/drakestapleton/work/cudastack/omega-mv \
  OMEGA_BRIDGE_DIR=/home/drakestapleton/work/cudastack/omega-bridge \
  sh om/units.sh --check
```

Final line: `PASS portable Omega units regenerate byte for byte`.

## Portability inventory repair

Main reported that the native-half seam omitted `src/omega_gpu_dnn_types.h` from the portability inventory, which stopped a downstream full-suite attempt. Source inspection confirms that this header contains only storage-format constants and no includes: its correct classification is `PORTABLE` / `NO_HOSTED_INCLUDE`. The three integrated operator interfaces include `omega_blackwell_codegen.h` and are `CHIP_SPECIFIC` / `NO_HOSTED_INCLUDE`; their memory/session shells include `stdlib.h` and are `CHIP_SPECIFIC` / `HOSTED_LIBC`. All seven entries are now registered.

The repair and unchanged/failing/guarded-include controls were exercised with rc 0:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make test-runtime-portability
```

Observed check: `PASS: 159 rows, every omega_gpu_* / omega_blackwell_* / omega_bw_* / omega_ampere_* file in src/ has one and the rows agree with the includes`.

Final line: `ALL CONTROLS BEHAVED AS REQUIRED`. This focused repair does not turn the downstream failed full-suite attempt into a full-suite PASS.

Seam-only downstream consumers should cherry-pick `4593b23`, which adds only the dtype header's row. The integrated operator rows are separate so that the f64 seam consumer does not acquire inventory entries for operator files absent from its tree.

## External gates and chip runbooks

- Full `make test`: NOT_RUN by this lane. Integrator2 owns the single final attempt and its rc plus exact `CUDA++ HOST TESTS PASS (<n> targets)` line. Interrupted results are INCONCLUSIVE, not PASS, and are not retried by this lane.
- Hardware numerical qualification and performance: NOT_RUN; there is no GPU window.
- MERGED: no. DEPLOYED: no.
- Chip runbooks: `om/attn-any/CHIP.md`, `om/dnn-rows/CHIP.md`, `om/dnn-activations/CHIP.md`, `om/conv2d/CHIP.md`. The common guide is `om/dnn-layers/README.md`. They require fresh explicit authorization; source/simulator/oracle results are not chip receipts.

The three private implementation slices are integrated and their claims closed. Integrator2 owns final full-suite verification; Main owns central omega-harness guide integration and operator-facing hardware authorization. There are no remaining lane test processes or live AttentionC claims.
