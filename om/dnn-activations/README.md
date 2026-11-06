# Seeded elementwise activations

IMPLEMENTED seeded Omega admission and Blackwell IR for contiguous ReLU,
erf-form GELU, tanh-form GELU and SiLU. GPU execution is NOT_RUN. This is not
a replacement for the existing model-tuned activation kernels.

## API

`src/omega_gpu_dnn_activations_api.h` exports:

```c
typedef struct {
    uint64_t elements;
    uint32_t operation, dtype;
} OmegaGpuDnnActivationsShape;
int omega_gpu_dnn_activations_codegen(const OmegaGpuDnnActivationsShape *shape,
    BlackwellIRProgram *program, uint8_t *code, size_t capacity,
    OmegaBlackwellKernel *kernel);
int omega_gpu_dnn_activations(const OmegaGpuDnnActivationsShape *shape,
    const void *input, float *output);
```

Operation constants: `OMEGA_GPU_DNN_RELU=0`, `OMEGA_GPU_DNN_GELU_ERF=1`,
`OMEGA_GPU_DNN_GELU_TANH=2`, `OMEGA_GPU_DNN_SILU=3`. Storage constants come
from `omega_gpu_dnn_types.h`: f32=0, bf16=1, f16=2. Input is contiguous and
has `elements` scalars. Output has `elements` f32 scalars. Accumulation and
arithmetic use f32, including widening bf16/f16 inputs before evaluation.

The finite-input precondition excludes NaN and infinity; values are not scanned
on the host. Callers provide enough readable/writable storage. Input/output
may overlap because the execution shell uploads input before downloading output.
Nonempty counts through `UINT32_MAX` are admitted. No alignment or warp-multiple
restriction applies to the element count. Maximum-count admission is exercised
without allocating that tensor. Physical allocation failure remains a chip error.

Return codes: 0 OK, 1 BAD_ARGS (null required argument), 2 REFUSED (zero count,
count above `UINT32_MAX`, unknown operation or storage type), 3 CODEGEN (bridge,
allocation, replay or encoding failure), 4 CHIP (session, buffer, transfer or
launch failure). Shape/type refusal happens before session/device access; the
host and mutant links wrap session open/lock/unlock/free and allocation. In
`--sim`/`--quick`, every such call aborts without forwarding to the device.
Refused execution shapes must cause zero attempts. A separately compiled
direct-open mutant must abort under `--sim`, proving the linked guard. The
separate explicit `--shell-contract` fake-backend mode checks pointer semantics:
NULL open causes zero allocations; non-NULL open reaches one deliberately failed
allocation. No fake-mode exception exists under `--sim`.

`*_codegen` is host-only. The execution API opens the GPU and requires an
operator-authorized exclusive window. Its C implementation is a memory/session
shell only. `plan.om` owns count/type validation, byte sizes and grid geometry.
`kernel.om` owns loads, addressing, tail exits, format widening and all arithmetic.
It composes the read-only common `attn-any/emit.om` and uses the single generic
`omega_gpu_om_ir_replay`. No host libm production kernel or external production
math library is introduced. Host libm is verification-only.

Each CTA has one warp. Each active lane processes one component at
`block*32+lane`. Out-of-range lanes exit before loading or storing; the
independent simulator checks exact input/output ranges and an output sentinel.
Parameter words are input pointer (two u32 words), output pointer (two), count
(one), starting at c0 byte offset 896.

## Mathematical and approximation contract

ReLU is `x < 0 ? 0 : x`. Signed zero is preserved for every operation, including
ReLU at negative zero. Strictly negative ReLU values produce positive zero.

Erf GELU is `x*(1+erf(x/sqrt(2)))/2`. The device evaluates the A&S 7.1.26
complementary-error-function polynomial at `z=abs(x)/sqrt(2)`:

```
t = 1/(1 + 0.3275911*z)
q = t*(0.254829592 + t*(-0.284496736 + t*(1.421413741
    + t*(-1.453152027 + t*1.061405429)))) * exp(-z*z)
y = x*(1-q/2)  for nonnegative x; x*q/2 for negative x
```

Tanh GELU is `x*(1+tanh(sqrt(2/pi)*(x+0.044715*x^3)))/2`. It is evaluated
using the equivalent sigmoid identity and the absolute value of its argument.
SiLU is `x/(1+exp(-x))`. Both use `e=exp(-abs(argument))`, then the stable
positive multiplier `1/(1+e)` or negative multiplier `e/(1+e)`. The negative
multiplier is formed before multiplying by x. This prevents positive-exp
overflow and negative-tail cancellation.

Before square/cube evaluation, GELU absolute inputs are clamped to 10; their
exp terms are forced to exactly zero when the original magnitude is at least
10. SiLU uses magnitude 80 similarly. Hence saturation returns x on the
positive side and negative zero on the negative side, including largest finite
f32/bf16 values. No finite tail is multiplied by an overflowing cube or exp.
MUFU.EX2 receives a nonpositive argument, scaled by log2(e); MUFU.RCP handles
the positive denominators. The polynomial, f32 rounding, and MUFU approximation
mean results are not bitwise libm results.

The tested envelope is `abs(got-reference) <= 3e-6 + 3e-6*abs(reference)` for
all formats after decoding storage. This is a deliberately conservative test
envelope, not a proven universal error bound or chip guarantee. MODELED grid
errors below use ideal real/double evaluation, not the device instruction path.
MEASURED simulator results are reported separately in the acceptance receipt.

### Wolfram query/result record (MODELED)

All constants and modeled calculations were checked with the mounted Wolfram
Language evaluator. Queries and results:

* `Integrate[x^2,{x,0,1}] -> 1/3` and WolframAlpha `sqrt(27)/2 -> (3 sqrt(3))/2`
  verified tool access.
* `N[{Sqrt[2/Pi],1/Sqrt[2],Log[2,E]},16]` returned
  `{0.7978845608028654,0.7071067811865475,1.4426950408889634}`.
* `Round[10^9*{1/Sqrt[2],2*Sqrt[2/Pi],Log[2,E]}]` (the actual call spelled
  these as three separate `Round` expressions) returned
  `{707106781,1595769122,1442695041}`. The decimal literals used by the seeded
  compiler are therefore `0.707106781`, `1.595769122`, `1.442695041`, each
  rounded once to f32. The long exact decimal literal exceeded the pinned
  compiler's signed-significand limit; no compiler pin was changed.
* Defining `p=0.3275911; a={0.254829592,-0.284496736,1.421413741,-1.453152027,1.061405429};
  q[z_]:=With[{t=1/(1+p z)},t (a[[1]]+t (a[[2]]+t (a[[3]]+t (a[[4]]+t a[[5]])))) Exp[-z^2]];`
  then `Max[Table[Abs[q[z]-Erfc[z]],{z,0,10,0.0005}]]` returned
  `1.3937450349388314e-7`.
* With the same q, `Max[Table[Abs[(x/2) q[Abs[x]/Sqrt[2]]-(x/2) Erfc[Abs[x]/Sqrt[2]]],{x,-10,10,0.001}]]`
  returned `2.1112399716622862e-7`. This is a sampled approximation error, not
  an interval proof.
* `FullSimplify[(1+Tanh[t])/2-1/(1+Exp[-2 t]),Assumptions->Element[t,Reals]] -> 0`.
* `N[{80 Exp[-80],10 Erfc[10/Sqrt[2]]/2},12]` returned
  `{1.4438811102763321e-33,7.6198530241605261e-23}`. These are the ideal
  omitted SiLU and erf-GELU negative-tail magnitudes at the cutoffs.
* `N[Exp[-2 Sqrt[2/Pi](10+0.044715*10^3)]*10,12] -> 1.2040923482098023e-37`
  gives the ideal tanh-GELU omitted tail at its cutoff.
* `N[0.000003/2.1112399716622862*^-7,12] -> 14.209658969453615` records the
  absolute-envelope margin over the sampled ideal polynomial error. It is not
  a floating-point or MUFU error proof.
* `N[{1/2^23,2^-22.5},12] -> {1.1920928955078125e-7,1.6858739404357614e-7}`
  is modeled f32 epsilon and the existing EX2-relative-bound constant quoted
  in the IR header; it does not qualify this kernel on hardware.

## Focused acceptance

Use the compiler/bridge environment required by `om/units.sh`. Seed compiler:
Omega source `e5b32567`, omc SHA256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
The checked C and declarations are generated only by that script.

```sh
export OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc
export OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge
taskset -c 10-14 nice -n 10 sh om/units.sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-gpu-dnn-activations test-gpu-dnn-activations-mutants
```

The host gate exercises every operator/type over counts
`1,2,7,31,32,33,64,65,513,65535`. Small counts rotate the value corpus to cover
both signs, both zeros, small values and saturation even at count one. Larger
cases include largest finite f32/bf16 inputs. It compares the complete emitted
path with independent double erf/erfc, tanh and exp references. Deliberately
perturbed computed output must fail. Seven seeded source mutants must first compile with seeded
omc and generate checked C, then fail behavior: ReLU selection, erf coefficient,
tanh cubic coefficient, SiLU sign, block indexing, storage load and admission.
The admission mutant is killed by an expected session-lock guard abort; an
unrelated signal or compile failure never counts. A separate compiled C
direct-open guard control must die with the session-open abort marker.
Mutation evidence is retained under the printed ignored build directory.
No full suite or GPU run is part of these targets. See `CHIP.md` for the
operator-only hardware acceptance procedure.

### Private host receipt (MEASURED; no chip qualification)

First registration was committed separately as `8d491f2`. The implementation
and reference-first work-in-progress checkpoint was pushed as `9c9f00b`.
The final device-isolated focused command was:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge \
  make -j3 test-gpu-dnn-activations test-gpu-dnn-activations-mutants
```

Exit status: **0**. Local evidence: `build/dnn-activations-bwrap-final.log`;
mutation artifacts: `build/dnn-activations-mutants.A5n2G3/`. The private `/dev`
contains no GPU, CUSE or VFIO device nodes. Session-abort guards remain in
addition to this device namespace isolation. These build artifacts are ignored,
not historical committed chip receipts. Final lines:

```
dnn-activations host+warp-sim: PASS (4 operators, 3 types, tails, independent double references, refusal-before-open, wrong-result control)
dnn-activations shell-contract: PASS (NULL open: 0 allocs; non-NULL open: 1 failed alloc; isolated fake backend)
dnn-activations isolation control: PASS (compiled direct-open mutant aborts under --sim)
dnn-activations source mutants: PASS (7 compiled, 7 behaviorally killed)
```

The deliberate C direct-open control aborts with the session-open marker.
The widened seeded admission mutant aborts with the session-lock marker;
the remaining six seeded mutants return the expected behavioral-failure status.
The host test binary's five `__wrap_omega_gpu_session_*` symbols were also
observed with `nm -g --defined-only` before the guarded acceptance run.
The separately gated chip runner was built, not executed, in the prior focused
build recorded in `build/dnn-activations-final.log` (exit 0).

Generation by `om/units.sh`: PASS. Independent host references and warp
simulation: PASS for the enumerated corpus and counts, not an exhaustive input
proof. Seeded behavioral mutation gate: PASS. Hardware correctness/performance:
**NOT_RUN**. Full host suite: **NOT_RUN**, integration-owner-only.
No merge or deployment was performed.
