# API kernel IR migration

## Scope and pins

The elementwise API's complete generator and scheduler are seeded Omega in
`om/kgen/ew_{emit,scalar,reconv,prime,ir}.om`. C dispatches to the existing single
`omega_gpu_om_ir_replay` entry. No IR opcode or replay entry was added. The replay
ABI already solves register allocation; the API does not solve it twice.
The old C generator is retained only under
`CUDAPP_MIGRATION_PENDING_C_API_KGEN=1` (default **0**) as the independent oracle.
This macro is not enabled in production builds.

Base: `feat/api-views-attn-om` at `c1fec738fb3dfe4fd48bf9115a19ca9d4d3d6367`.
The base lacked replay and scan registration. `git merge-tree` reported shared
registration conflicts; Main explicitly authorized their union and regeneration.
Prerequisites merged: #309 `63d18a2cc92cb05f800d17e46b35f0cd6237b3b0`
(merge `7113117`), then #294 `220fe47dfdecb715e0d49be223808c211b889169`
(merge `bac386c`). No #327 merge was needed.
Elementwise registration-before-cutover commit: `73b11c5`.

Compiler: `omega-mv/seed/out/omc`, SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Converter: bridge #73 `79e8c36`, `/tmp/integ/bridge-73-79e8c36`;
`om/units.sh` verifies its converter hashes. Bootstrap executable for freest
bindings: `omega-bridge/build/bootstrap/omc`, SHA-256
`191b776ef98821597c3e30655da6d680a9b66edbc9ceaca3c39c61ea6df098e9`.
The bridge-generated C is checked output from seeded Omega, not handwritten C
computation. The existing host compiler and libc remain host-build dependencies;
this migration changes neither the host ABI nor the foreign-runtime boundary.

## Elementwise acceptance partition

`tests/api_kgen_ew_diff_test.c` exhausts each `OmegaGpuEwOp` plus adjacent invalid
enum values, each supported mutant 0..3 and both launch/test-path modes. It compares
instruction structs byte-for-byte, the entire solved allocator, encoded text and
complete cubin bytes with the retained C generator. Observed: **127 accepted,
73 refused**, all dispositions identical. This includes scalar, shared, divergent
sieve, loop, reconvergence, depth-16 and refused depth-17 variants.

EW kernel IR has no shape argument: runtime n, dimensions, epsilon and sieve
limits are launch constant-memory data. Thus the complete generator-spec
comparison applies to every admitted runtime shape, not just selected ones.
The same test additionally cross-checks the unchanged seeded shape-plan path
against its C oracle over the source-derived boundary partition from
`om/components/api_ew_plan.om`: zero, minimum even dimension 2, divisibility by
128, row clamp 1024, dimension caps 2048/16384, element clamp 131072 and mutant
0..3 admission, with v-1/v/v+1 representatives where representable. All modes
0..6 and range lower/upper comparisons are included, including the signed-input
boundary at 9223372036854775807 and UINT64_MAX. Observed: **4189 admitted,
4001 refused** with identical admission and output words.

WolframLanguageEvaluator:
`{9223372036854775807-1,9223372036854775807,9223372036854775807+1}`
-> `{9223372036854775806,9223372036854775807,9223372036854775808}`.

The red control flips the first IR immediate and must exit 1 with `FAIL IR`.
Five separately regenerated seeded semantic mutants were killed: descriptor
offset, B-pointer offset, CTA special register, tightened pair latency and EX2
dispatch. Compilation/generation failures do not count as killed mutants.

Commands exercised so far (all with private `/dev` and cores 10–14):

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-api-kgen-ew
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc" OMEGA_BRIDGE=/tmp/integ/bridge-73-79e8c36 OMEGA_SEED="$HOME/work/cudastack/omega-bridge/build/bootstrap/omc" make -j3 test-api-kgen-ew-mutants
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-gpu-elementwise test-gpu-reconv
```

Production-default host results: elementwise **24 checks, 0 failed**;
reconvergence **119 checks, 0 failed**. These include the existing software-model
and disassembly controls, not chip execution.

`elementwise-differential.log` records the final boundary/differential/red and
seeded-mutant gate. `simulator-targets.log` records all three named `mk/`
simulator targets listed below, with exit 0. Logs replace the user's home
directory with `~` where present; no outputs or verdicts are otherwise changed.

## Existing target inventory

`Makefile` defines `test-gpu-elementwise`, `test-gpu-attention`,
`test-gpu-decode-attn`, `test-gpu-decode-attn-maxpos`, `test-gpu-reconv` and
`test-gpu-prefill-host`. The latter runs the prefill-attention binary in both
host and simulator modes; there is no separately named
`test-gpu-prefill-attn*` target on this base. `mk/carried-tests.mk` defines
`test-gpu-decode-attn256` (host and sim). The explicitly named simulator targets
in `mk/` are `test-hello-sim` (`dx-sim.mk`), `test-gpu-gemm-ex-sim`
(`gemm-ex.mk`) and `test-gpu-engine-sim` (`omega-gpu-engine.mk`).
Attention's own target includes normal, sweep, HD128, divergent and general sim
modes. No full `make test` is part of this gate.

## Chip runbook — NOT_RUN

No GPU window was authorized. Host byte equivalence and software simulation do
not establish hardware qualification. No GPU device, live service, GPU lock or
serving process is touched by these host gates.

In a separately authorized exclusive chip window, the operator must first follow
the current Spark window and device-safety procedure, pin the final commit and
compiler/converter identities, and build the normal default-Omega library.
Run `build/gpu_elementwise_test` and `build/gpu_attention_test` using their
existing chip modes and the current gate procedure; include supported HD64,
HD128 and general attention shapes and scalar/shared/reconvergent EW kernels.
Run the existing decode and prefill attention chip gates from the same build.
Capture exact commands, program outputs, device identity and kernel-driver
fault evidence, compare output bytes against the existing references, and run
the named negative controls. Do not run a refusal/mutant that the current chip
procedure does not authorize. Record hardware results separately from the host
receipts. This document grants no device or service authority.
