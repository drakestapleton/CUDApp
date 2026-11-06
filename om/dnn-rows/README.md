# Seeded row softmax and normalization

Experimental API; host/simulator exercise is not chip qualification. Chip qualification: **NOT_RUN**.

Source base: CUDApp native-half seam 9be13fb2595f1b3668288b36c4d17ef976bcb3a7.
Compiler: omega-mv e5b32567, SHA-256

2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f.
The generated C and identities come from ${OMEGA_OMC} and ${OMEGA_BRIDGE} via om/units.sh; no handwritten generated implementation.

## API contract

Include src/omega_gpu_dnn_rows_api.h. OmegaGpuDnnRowsShape fields are rows,
length, dtype, op, epsilon. Input is contiguous [rows][length]. Output is f32
of the same shape, including for bf16/f16 input. Types use shared DNN constants:
0 f32, 1 bf16, 2 IEEE binary16. Operations: 0 stable softmax, 1 layernorm,
2 rmsnorm. The norms are pure normalization, with no affine or fusion parameters.

- softmax: exp(x - row_max) / sum(exp(x - row_max)).
- layernorm: (x - row_mean) / sqrt(mean((x - row_mean)^2) + epsilon).
- rmsnorm: x / sqrt(mean(x^2) + epsilon).

Rows and length are independently positive and at most 65535. At most
1073741824 elements are admitted. Size decisions and all byte addresses are
seeded Omega. Epsilon for norms must be positive finite **normal** f32, with
unsigned encoding in [8388608,2139095040). Zero, negative values, subnormals,
infinity and NaN are refused. Softmax ignores epsilon. Epsilon is loaded at
runtime, not embedded in generated instructions.

Finite input and finite f32 intermediate arithmetic are caller preconditions:
sums, differences, squares, variance plus epsilon, and output must remain
finite. The variance-plus-epsilon must remain normal and positive. Softmax
uses a maximum pass before exponentiation; subtracting two finite extremes
must still be finite. Input contents are not host-scanned. MUFU flush-to-zero
and approximate hardware operations are not represented by a universal
accuracy guarantee. bf16 and f16 widen to f32 through the shared emitter;
f16 is not reinterpreted as bf16.

omega_gpu_dnn_rows_codegen is CPU-only; caller supplies BlackwellIRProgram,
code buffer (64 KiB recommended), and OmegaBlackwellKernel. It performs
Omega admission, emits Omega IR, uses the one shared generic replay, then
encodes. omega_gpu_dnn_rows copies input, allocates output/code, launches one
32-thread warp per row, and copies output back. A valid GPU call requires an
operator-authorized exclusive window. Refusals occur before device/session
access. Null pointers return BAD_ARGS before admission.

Return values: OK=0, REFUSED=1 (shape/type/size/epsilon), BAD_ARGS=2,
CODEGEN=3 (allocation, bridge/replay/encoding or code capacity), CHIP=4
(session/allocation/synchronization/launch failure). The memory shell preserves
the session's uncertain-completion buffer ownership policy.

## Algorithm and independent tests

A warp traverses each row in uniform ceil(length/32) loops. Every lane loads
only valid elements, with native f16/bf16 widening and appropriate identity
values on tails. Butterfly warp reductions combine the lane accumulators.
Softmax uses maximum, exponential sum, and output passes. Layernorm uses mean,
centered variance, and output passes. RMS norm uses mean-square and output
passes. The allocation/lifetime metadata names every generated loop.

The independent C test oracle decodes input with host _Float16 and accumulates
naive double references using libm exp/sqrt; it does not call the production
unit to obtain answers. The warp simulator runs the entire emitted IR path,
with strict input/output memory ranges. The complete sweep covers lengths
1,2,7,31,32,33,64,65,513,65535, all three dtypes and operations, multiple rows,
constant and varied rows, and epsilon values 0.0009765625,0.03125,1. Refused
geometry/type/size/epsilon are also sent to the public execution API to show
that they return before opening the session. A deliberately wrong reference
must be rejected. Six Omega source mutants must generate and compile successfully:
maximum, normalization, load tail, epsilon, epsilon refusal, and native-half mode.
Five fail numerical/address behavioral assertions. The epsilon-admission mutant
aborts at the host device guard before session access. A separate compiled C
test-source mutant directly calls session_open under --sim and must abort at
that entry guard. Baseline and every mutant binary link the same guard wrappers
for session open/allocate/lock/unlock/free. Under --sim and --quick every wrapped
entry aborts, rather than returning a recoverable error. A separate explicit
--shell-contract mode uses only fake open/failed-allocation responses to check
the session pointer contract: NULL opens allocate nothing, non-NULL opens attempt
exactly one blocked allocation. Only --chip forwards calls to real session APIs.

## MODELED numerical envelope (not chip accuracy)

WolframLanguageEvaluator query:

	u=2^-24; k=Ceiling[65535/32]+5; g=k*u/(1-k*u);
	{k,g,N[g,12],N[16*g,12],N[2^-5],N[2^-10],
	 Simplify[(x-m)/Sqrt[v+e]],
	 FullSimplify[Sum[Exp[a[i]-m],{i,1,n}]/Sum[Exp[a[i]-m],{i,1,n}],Assumptions->n>0]}

Result: {2053,2053/16775163,0.000122383311566,
0.00195813298506,0.03125,0.0009765625,(-m+x)/Sqrt[e+v],1}.
Units: dimensionless relative roundoff and normalization output.

The test uses abs(error) <= 16 gamma_2053 (1+abs(reference)). This is a
conservative **MODELED test envelope** based on the maximum per-lane serial
length and butterfly depth, with headroom for transformations. It is not a
proof for ill-conditioned inputs, not a bound on MUFU approximation, and not
universal or measured chip accuracy. The independent sweep, not the formula,
provides the host numerical evidence for its stated finite test domain.
Tool availability checks: Integrate[x^2,{x,0,1}] -> 1/3; WolframAlpha
sqrt(27)/2 -> (3 sqrt(3))/2.

## Focused acceptance

Set OMEGA_OMC to the pinned seeded compiler and OMEGA_BRIDGE to the pinned
companion checkout. Regenerate with:

    bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/units.sh
    bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-gpu-dnn-rows test-gpu-dnn-rows-mutants

Do not run the full suite from this slice. The integrator owns its single
attempt. Mutation evidence is kept under build/dnn-rows-evidence/mutants.
See CHIP.md for the separate operator-only NOT_RUN gate.

Final focused command (with the two pinned OMEGA_* environment variables):

    taskset -c 10-14 nice -n 10 make -j3 test-gpu-dnn-rows test-gpu-dnn-rows-mutants

Observed exit status: **0**. Final lines:

    dnn-rows host+warp-sim: PASS (all lengths/types/epsilon, constant+varied rows, tail ranges, refusals, wrong-result control)
    dnn-rows shell contract: PASS (NULL open zero allocs; non-NULL open one blocked alloc; fake backend only)
    dnn-rows direct-open guard mutant: PASS (compiled; killed by abort under --sim)
    dnn-rows source mutants: PASS (5 behavioral failures, refusal guard abort, direct-open guard abort; all compiled)

Persistent full output and exit status:
build/dnn-rows-evidence/focused-abort-guard.log and
build/dnn-rows-evidence/focused-abort-guard.rc. Each mutation retains its generated
source/C, generation log, test log and exact behavior_rc in the mutants subtree.
The five numerical/address mutants each returned 1; the refusal and direct-open
controls each returned 134 with the expected named HOST DEVICE GUARD diagnostic.
The generated units were compiled through om/units.sh with exit 0 before these
focused targets. Their checked .ident files pin source, compiler, converter and
frontend digests. No full suite was run from this lane.

### Initial host-test isolation correction

The initial focused attempt's epsilon-admission mutant could reach real
session_open through the refusal fixture. The original host test did not yet
interpose session entry points. Source-only tracing established reachability
through the default Linux platform to nvrm_open, whose potential opens are
/dev/nvidiactl, /dev/nvidia-uvm (twice), and /dev/nvidia%u with a driver-reported
minor. This is not evidence of a successful open or launch; the initial artifact
cannot support a no-device-access claim. No GPU, dmesg, or lock probing was used
to investigate it. Initial output: artifact://3953 (focused make exit 2 due to
a later test-wrapper link mismatch). Return-error wrappers subsequently passed
the focused gates (artifact://4047), but were superseded by abort-on-entry
wrappers and an explicit direct-open abort control. A later in-flight run
(bg_770) was cancelled when the stronger isolation requirement arrived.
The chip gate remains NOT_RUN; no chip accuracy or operational-safety claim
is inferred from host evidence.

## Integration records (AttentionC owns shared files)

Public declarations:

    int omega_gpu_dnn_rows_codegen(const OmegaGpuDnnRowsShape *shape, BlackwellIRProgram *program, uint8_t *code, size_t capacity, OmegaBlackwellKernel *kernel);
    int omega_gpu_dnn_rows(const OmegaGpuDnnRowsShape *shape, const void *input, float *output);

Proposed manifest records below use the existing format; the integrator owns
the shared manifest, its compiled layout blessing, API.md, version and ROADMAP.

    header src/omega_gpu_dnn_rows_api.h
    struct OmegaGpuDnnRowsShape 20 4 # EXPERIMENTAL
    field OmegaGpuDnnRowsShape rows 0 4 u32 1
    field OmegaGpuDnnRowsShape length 4 4 u32 1
    field OmegaGpuDnnRowsShape dtype 8 4 u32 1
    field OmegaGpuDnnRowsShape op 12 4 u32 1
    field OmegaGpuDnnRowsShape epsilon 16 4 f32 1
    const OMEGA_GPU_DNN_SOFTMAX 0
    const OMEGA_GPU_DNN_LAYERNORM 1
    const OMEGA_GPU_DNN_RMSNORM 2
    const OMEGA_GPU_DNN_ROWS_OK 0
    const OMEGA_GPU_DNN_ROWS_REFUSED 1
    const OMEGA_GPU_DNN_ROWS_BAD_ARGS 2
    const OMEGA_GPU_DNN_ROWS_CODEGEN 3
    const OMEGA_GPU_DNN_ROWS_CHIP 4
    fn omega_gpu_dnn_rows_codegen fn(*const OmegaGpuDnnRowsShape, *mut BlackwellIRProgram, *mut u8, usize, *mut OmegaBlackwellKernel) -> i32 # EXPERIMENTAL
    fn omega_gpu_dnn_rows fn(*const OmegaGpuDnnRowsShape, *const c_void, *mut f32) -> i32 # EXPERIMENTAL
