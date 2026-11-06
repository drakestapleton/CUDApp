# Seeded Omega DNN layers

The integration branch is `feat/dnn-layers-om`, stacked on attention #282,
any-shape GEMM #283 (`04f16fe8`) and C1's append-only unsigned-conversion
foundation (`3bf3ac4`). No GPU execution is authorized by this work.

## Operator coverage and limits

| Operator | Seeded source and contract | CPU-only entry | Input/output |
| --- | --- | --- | --- |
| Rectangular online-softmax attention | [`attn-any`](../attn-any/README.md): sequences 1..65535, head dimension 1..256, GQA, top-left causal or non-causal, finite scale including zero/negative | `omega_gpu_attn_any_codegen` | f32/bf16/f16 Q/K/V, f32 output |
| Row softmax, layernorm, rmsnorm | [`dnn-rows`](../dnn-rows/README.md): rows/length 1..65535, bounded total elements; pure norms with positive finite normal f32 epsilon | `omega_gpu_dnn_rows_codegen` | shared f32/bf16/f16 input, f32 output |
| ReLU, erf-GELU, tanh-GELU, SiLU | [`dnn-activations`](../dnn-activations/README.md): contiguous counts 1..UINT32_MAX, finite inputs, tail exits and stable saturation | `omega_gpu_dnn_activations_codegen` | shared f32/bf16/f16 input, f32 output |
| Strict implicit-GEMM conv2d | [`conv2d`](../conv2d/README.md): NCHW/NHWC, OIHW filters, groups/depthwise, stride, independent zero padding and dilation | `omega_gpu_conv2d_plan`, `omega_gpu_conv2d_codegen` | shared f32/bf16/f16 image/filter, f32 output |

These are experimental bounded APIs, not complete cuDNN coverage or
performance replacements for the unchanged tuned model kernels. Public
signatures, distinct refusal codes and caller buffer/finite-arithmetic
preconditions are in `docs/runtime/API.md` sections 12 and 13 and the
individual headers. Shape admission, addressing, tails, tiling and kernel
arithmetic are seeded Omega. Execution C is allocation, upload/download,
pointer stepping and session/launch lifetime handling, not a host math kernel.

Convolution does not materialize a global im2col tensor. Its virtual image
addresses feed `gs_dot` in `om/gemm-any/gemm_ir.om`, the same reduction used
by default scalar f32 dense GEMM. The former C scalar generator is retained
only behind default-OFF `CUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER`; the
existing tensor GEMM configuration/API is unchanged.

Independent double host references use libm only as verification oracles.
Each operator guide records its exact Wolfram query/result and **MODELED**
numerical test envelope. Simulator comparisons exercise the emitted IR;
neither the model nor simulated MUFU behavior proves chip accuracy.

## Device-free host acceptance

Set `OMEGA_OMC` and `OMEGA_BRIDGE` to the pinned paths below. After writers
finish, the integration owner runs focused checks inside the device-free
namespace:

```sh
export OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc"
export OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge"
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/units.sh --check
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 \
  test-dnn-half test-dnn-half-mutants test-om-ir-boundary \
  test-gpu-attn-any test-gpu-attn-any-mutants \
  test-gpu-dnn-rows test-gpu-dnn-rows-mutants \
  test-gpu-dnn-activations test-gpu-dnn-activations-mutants \
  test-gpu-conv2d test-gpu-conv2d-mutants test-gpu-conv2d-gemm-f32 \
  test-runtime-api test-runtime-api-semver test-omega-binding-portable
```

During the multi-lane integration wave, **Integrator alone** runs the single
full-suite attempt, after all candidate commits land:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test
```

No retry, no `pkill`/`killall`, and no stopping another lane's processes.
Judge full-suite success by both rc 0 and the final
`CUDA++ HOST TESTS PASS (<n> targets)` line; interruption is INCONCLUSIVE,
not PASS. These templates do not retroactively claim that historical checks
preceding the device-free policy ran inside a namespace.

The DNN and attention host/mutant links additionally wrap session
open/lock/unlock/free/allocation. Every wrapped entry aborts under `--sim`
or `--quick`; an explicit direct-open source mutant must die at that guard.
Separate `--shell-contract` mode has a fake backend solely to test NULL
versus non-NULL session pointer handling, and never forwards to a device.
The namespace remains mandatory even with those guards.

### Safety correction

The initial row epsilon-admission mutant, before wrappers were installed,
had a source-proven path from `--sim` refusal testing to `session_open` and
the Linux nvrm client. That path could attempt `/dev/nvidiactl`,
`/dev/nvidia-uvm` and the driver-reported `/dev/nvidia%u`; source reachability
does not establish a successful historical device open or launch.
`om/dnn-rows/README.md` preserves the original command and correction.
The inverted session-pointer checks were fixed, abort guards and direct-open
negative controls were added to all DNN slices and attention, and all new
host/simulator/mutant commands use the mandatory device-free namespace.

## Source and generation pins

Native seam: `9be13fb2595f1b3668288b36c4d17ef976bcb3a7`.
Seeded compiler: omega-mv `e5b32567`, omc SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Bridge checkout: `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4`;
`om/units.sh` enforces `om/omega-bridge.lock` and generates the committed
hosted/freestanding C and declarations. Generated identities are never
hand-edited to resolve a stacked-branch conflict.

## Native scalar half seam

`BW_IR_F16_TO_F32` is opcode 66 and `BW_IR_F32_TO_F16` is 67. Existing opcodes
0..65 retain their values. Generic replay admits `BW_IR_LAST` and refuses
`BW_IR_LAST+1`; the replay-boundary regression also kills a compiled source
mutant restoring the former bound of 63.

The sm_121 encoding authority is the already recorded E1_CVT golden words in
`src/omega_numeric.c`: `HADD2.F32 R9, -RZ, R2.H0_H0` has words
`ff097230 20000002 00004100`; `F2F.F16.F32 R9, R2` has
`00097304 00000002 00200800`. Widening uses HADD2, not an invented scalar
F2F widening instruction. The source is the low half of the input register;
narrowing is round-to-nearest-even and clears the upper half. Widening is
fixed latency; narrowing uses a dependency barrier before consuming its result.
The seeded encoder and migration-pending C differential encoder use the same
recorded forms; allocation and instruction selection remain separate.

WolframLanguageEvaluator query:

```wolfram
FromDigits[#,16]& /@ {"ff007230","20000000","4100","7304",
                       "00200800","010fca00","010e2800"}
```

Result: `{4278219312,536870912,16640,29444,2099200,17811968,17704960}`.
These are encoding integers, not measured performance or accuracy claims.

`tests/gpu_dnn_half_test.c` checks all binary16 bit patterns against the
independent integer E1 reference, all finite half round trips, and a deterministic
binary32 narrowing sweep. NaNs are compared by class, not payload. It separately
checks native words across registers and all three IR guard modes, plus an
unknown-opcode refusal. The simulator uses the host `_Float16` conversion,
not the integer reference. No tolerance is applied to finite conversion bits.
Both simulator headers run the same native conversion reference and guarded
warp-scheduler regression. Default/scalar instructions execute only for the
predicate-selected lanes. SHFL writes only those lanes but may read a source
register from a guard-inactive lane; an entirely guard-inactive SHFL does
nothing. Active SHFL retains the simulator's split/exited-lane refusals.
The scalar simulator repair follows C1 commit `7de6da63`; the pre-existing
dynamic simulator repair is carried by attention #282.


Host-only commands:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-dnn-half test-om-ir-boundary
mkdir -p build/dnn-half-bins
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 build/dnn_half_test build/dnn-half-bins/half.bin
# Compare the generated words with the seeded disassembler and offline nvdisasm:
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env SM121DIS_JOBS=3 tools/sm121dis/check.sh /path/to/sm121dis \
  build/dnn-half-bins build/dnn-half-oracle
```

Every host, simulator, source-mutant or offline-oracle command runs inside
that device-free namespace. Native encoding/disassembly does not require a
GPU node; an attempted session/device open during those paths is a defect.

The disassembler is a read-only verification oracle, not a production dependency.
Its exact source revision and the exercised commands belong in the dated receipt.
Listings and raw instruction binaries stay under ignored build directories.

Chip execution and qualification are **NOT_RUN**. A passing simulator or offline
disassembler check is not a chip receipt. The ROADMAP status is not upgraded.

Integrated implementation and host receipts are in
[`evidence/DNN-LAYERS-2026-11-06/REPORT.md`](../../evidence/DNN-LAYERS-2026-11-06/REPORT.md).
Draft [#307](https://github.com/aien-dev/CUDApp/pull/307) is stacked on attention
draft #282. The final full-suite gate belongs to Integrator2, not to these private
operator slices.
