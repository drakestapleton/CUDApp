# Seeded implicit conv2d

Public API: `src/omega_gpu_conv2d_api.h`. `omega_gpu_conv2d_plan(shape,launch,plan)` admits and maps one launch; `omega_gpu_conv2d_codegen(shape,program,code,capacity,kernel)` emits the complete kernel; `omega_gpu_conv2d(shape,image,filter,output)` is the session/memory shell. Status values: OK 0, BAD_ARGS 1, REFUSED 2, CODEGEN 3, CHIP 4.

Inputs and outputs are contiguous NCHW (layout 0) or NHWC (layout 1). Filters are always contiguous OIHW `[CO, CI/groups, KH, KW]`. This is cross-correlation with explicit independent top/bottom/left/right zero padding. Strides and dilations are positive. No bias, fused activation, scaling or SAME-padding convention. Dtypes follow `omega_gpu_dnn_types.h`: 0 f32, 1 bf16, 2 IEEE f16. Both operands use the selected storage; arithmetic and output are f32.

All positive dimensions/strides/dilations/groups are at most 65535; padding is 0..65535. Input and output channels must be divisible by groups. Empty output geometry is refused. Each input/filter/output element count and launch count must be below 2^30. The planner checks these before any device session access. Caller buffers must cover the returned byte counts; plan does not validate memory provenance.

## Composition

`om/gemm-any/gemm_ir.om` contains the one seeded scalar reduction, `gs_dot`, shared by dense F32 GEMM and convolution. Existing cfg8 tensor PACK/MAIN and public GEMM APIs are preserved. Default F32 no longer uses handwritten C FFMA generation. That implementation exists only behind default-OFF `CUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER` as a differential oracle.

Conv computes virtual-image and filter addresses inside the emitted kernel. One launch covers one batch/output-channel/output-row, with 128-thread tiles across output width. The seeded planner supplies every launch count, offset and grid dimension. Kernel loops over KH, KW, CI/groups and invokes the common scalar reduction. Invalid image coordinates read a valid location but mask its bits to positive zero before arithmetic; invalid output lanes never store. No host or global im2col is produced. Temporary device memory consists only of the original input/filter, f32 output and code.

The scalar implementation is a correctness path, not a tuned tensor-convolution performance claim. Existing tensor GEMM remains available and unchanged. Generic checked replay and native half conversion use the shared integration seams without alternate trust paths.

## MODELED arithmetic

WolframLanguageEvaluator query:

```
{Reduce[h+pt+pb-d (r-1)-1>=0 && s>=1,h,Integers],
 FullSimplify[Floor[(h+pt+pb-d (r-1)-1)/s]+1],
 Table[{k,N[(k/2^24)/(1-k/2^24),16]},{k,{1,9,27,45,81,135}}],
 FullSimplify[(c*kh*kw+y*kw+x)==((c*kh+y)*kw+x)]}
```

Result: admitted integer geometry requires `s>=1` and `h>=1-d-pb-pt+d*r`; output extent is `1+Floor[(-1+d+h+pb+pt-d*r)/s]`; filter flattening identity is True. F32 FMA forward-error bounds gamma_K for K=1,9,27,45,81,135 are respectively 5.96046483281045e-8, 5.36442090748478e-7, 1.60932799886799e-6, 2.68221620915707e-6, 4.82799953627362e-6, 8.04669179340554e-6. These are MODELED, not measured chip accuracy. Tests compare against independent naive double convolution with `gamma_K * sum(abs(products)) + 2^-126` for finite test inputs.

## Focused host gates

After source generation with the pinned compiler and bridge:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-gpu-conv2d test-gpu-conv2d-gemm-f32
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge make -j3 test-gpu-conv2d-mutants
```

The first target runs the complete generated IR in warpSIM, both layouts and all storage types, batch/group/depthwise, stride/pad/dilation, 1x1/3x3 and odd width/channel/spatial tails. Independent wrong-group reference must disagree. In addition to the empty-device namespace, host session link wrappers abort on any device access under `--sim`, including admission mutants. Only the explicit memory-only `--shell-contract` mode permits fake sessions: positive controls cover NULL-open/zero-allocation and nonNULL-open/one-failed-allocation semantics. Dense F32 runs emitted seeded and migration-C oracle kernels separately against double reference and compares output digests. Five source mutants must compile then fail numeric behavior; a separately compiled direct-open mutant must die with SIGABRT. Existing C3 focused regression gates remain required by integration and use the same namespace isolation plus `gpu_conv2d_guard.c` abort wrappers. No full suite, GPU or performance qualification is implied. See `CHIP.md` for the separately authorized operator-only hardware runbook.

## Exercised private host evidence

Implementation commit before final documentation: `9a2efca`; compiler source
`omega-mv@e5b32567`, omc SHA256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
No chip qualification is asserted.

Final device-isolated focused command (exit 0):

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge make -j3 test-gpu-conv2d test-gpu-conv2d-gemm-f32 test-gpu-conv2d-mutants build/gpu_conv2d_chip_test
```

Observed final lines:

```
conv2d host+warp-sim: PASS (1454 checks, 0 failures)
conv2d host+warp-sim: PASS (27 checks, 0 failures)
dense F32 digest 93efbbc6b4debf83
conv2d host+warp-sim: PASS (1290 checks, 0 failures)
conv2d source mutants: PASS (5 compiled, 5 behaviorally killed)
conv2d direct-open mutant: PASS (SIGABRT before device access)
```

The chip executable was compiled only, never executed. The five numerical
mutations were product/FMA operation, image stride, dilation, padding mask and
filter spatial flattening. Each compiled before its numeric mismatch.

Retained C3 focused command (exit 0), with abort guards on both default and
migration-oracle binaries:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 CC='cc tests/gpu_conv2d_guard.c -Wl,--wrap=omega_gpu_session_open,--wrap=omega_gpu_session_lock,--wrap=omega_gpu_session_unlock,--wrap=omega_gpu_session_free,--wrap=omega_gpu_session_alloc' test-gpu-gemm-any test-gpu-gemm-any-differential
```

Observed:

```
GEMM ANY SIM SWEEP PASS (286 distinct shapes, 2288 multiplies, 2552824 output words, bitwise model + per-K bound)
GEMM ANY HOST/SIM/MUTANTS PASS
GEMM ANY C/OMEGA SIM-EQUAL PASS (not byte-identical kernels)
```

Generated unit check (exit 0; generation only, not a device/test command):

```
taskset -c 10-14 nice -n 10 env OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge sh om/units.sh --check
PASS portable Omega units regenerate byte for byte
```

The full suite is NOT_RUN by this lane; the integrator owns its single attempt.
Hardware, performance, merge and deployment remain NOT_RUN/not performed.
