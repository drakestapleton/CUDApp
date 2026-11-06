<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# Freestanding leftovers: inventory slice 5

## Ownership and source

Base: CUDApp `b774723f537cec16a359fe082b8100317a5b5490`.
Read all open/closed PRs returned by the required overlap listing; inspected the
file sets of #291 and #314 explicitly. Neither owns these target headers or
`omega_ampere_qmd.c`. #291 changes artifact admission implementations and #314
changes Ampere corpus/export tools, not the helpers in this slice. No stacking
on those PRs is needed. `om/units.sh`, `src/cudapp_native.h` and
`src/cudapp_om_units.ident` are the shared integration seams.

`om/components/leftovers.om` owns path alphabet/component/capacity policy,
ordinal normalization, atomic-acquisition result mapping, poison-state query,
context disposition and bounded graph-release extent. Its final `leftovers`
entry point selects scalar operations; earlier functions are unit-private.
The pinned freestanding frontend exports only the last function. There are no
OM2 arrays, globals, host words, arena allocation or runtime imports.
The first commit registers this unit separately from consumer migration.

`CUDAPP_MIGRATION_PENDING_LEFTOVERS=1` selects the old C policy solely as an
oracle; it is OFF by default. Tests explicitly include the old path header
under renamed functions, then the real default header, so their differential
calls both implementations, not two Omega wrappers.

## What remains C and why

| Target | Source audit / irreducible residue |
| --- | --- |
| `src/cudapp_freest_f16.c` | Already generated, not handwritten: `PURE_UNITS` maps it to `om/numeric/f16.om`; `cudapp_om_f16.ident` records its source, converter and compiler hashes. Regeneration checks custody. |
| `src/cudapp_freest_epoch.c` | Already generated from array-free `om/pipeline/epoch.om`; identity is `cudapp_om_epoch.ident`. No duplicate port. |
| `src/cudapp_freest_revision.c` | Already generated from array-free `om/pipeline/revision.om`; identity is `cudapp_om_revision.ident`. No duplicate port. |
| `src/omega_ampere_qmd.c/.h` | Descriptor admission and encoding already execute `om/encoder/amp_runtime.om`. C constructs the native input view, copies generated result cells to caller-owned u32 storage and adapts ABI status/null pointers. The header has declarations only. |
| `src/omega_types.h` | Types, enum values and constants only; no executable computation to port. |
| `src/omega_native_artifact_snapshot.h` | Types, constants and function declarations only; no executable computation to port. |
| `src/omega_gpu_attention_mirror.h` | Function declaration and contract only; no executable computation to port. |
| `src/omega_gpu_fp8_internal.h` | Function declarations and contract only; no executable computation to port. |
| `src/omega_path_check.h` | Public ABI adapter only. The default path asks `omega2_pathlen` for the index of the first 0 inside the destination capacity, then `omega2_pathcopy` classifies, checks components and stores that exact view. Neither call walks a pointer in C. `omega2_pathlen` stops at the first 0 and never reads past the bound, so a NUL-terminated source is not read past its allocation. Empty output, empty input and a missing 0 inside `cap` are refused here; a nonempty NULL source is refused by the binding. The old classifying pointer walk remains only under `CUDAPP_MIGRATION_PENDING_LEFTOVERS`. |
| `src/omega_numeric_lifecycle.h` | Retains C11 atomic loads/stores/CAS with their original sequential consistency. Omega maps acquisition outcome and queries the poison state. The compare-exchange itself must stay atomic, not split into Omega read/compute/write. |
| `src/omega_gpu_q36_native.h` | Retains native object/pointer projection and host lease effects. Existing context/graph/residency policy already calls Omega. Added scalar policy handles status, context disposition and bounded release extent. Prepared-lease iteration invokes host releases, not a plain-data kernel. |

`om/components/pathcopy.om` is the freestanding borrowed-byte path copy. It is not a
`PURE_UNITS` scalar: the pinned scalar frontend has no views. `om/native-freest.sh`
regenerates `src/cudapp_freest_pathcopy.c` from omega-bridge draft #72
(`tools/native --freestanding`, both AArch64 and x86-64 halves). The committed C is
the artifact. `omega2_pathlen` stops at the first 0 inside the caller-supplied bound and refuses
a nonempty NULL view. `omega2_pathcopy` then receives exactly the bytes through that 0, copies an
accepted path and writes the trailing 0. Neither call inspects past its bound. Classification,
component checks and stores are Omega. The public header no longer walks the pointer.

Five source mutants (alphabet, punctuation, slash-state, invalid-byte, dot-dot) are
regenerated and must die with assertion exit 1. The scalar leftover mutants no longer
include dot-dot or capacity: those sites moved into `pathcopy.om`, and the scalar
unit's mode-1 path is not what the differential calls for them.

Atomic execution and host lease calls remain deliberately not modeled as ordinary Omega memory.

## Replay and acceptance

Every command below must run through:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 COMMAND
```

Set `OMEGA_OMC` to omega-mv `e5b32567`'s `seed/out/omc`, `OMEGA_MV` to its
checkout and `OMEGA_BRIDGE` to omega-bridge `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4`.
No pin is changed here.

- `sh om/units.sh --check`
- `make -j3 test-leftovers test-leftovers-mutants test-leftovers-coverage`
- `make -j3 test-native-artifact test-native-artifact-manifest test-omega-binding-portable`
- `make -j3 gpu-engine-freestanding test-gpu-engine-freestanding test-codegen-freestanding`
- Additional consumers: `make -j3 test-zero-cuda-tool-argv test-ampere-numeric-host test-ampere-f16-host`

`test-leftovers` compares old C and Omega paths, checks null/result refusal,
exercises ordinal, numeric latch and context/release policy and requires the
red executable (deliberately unequal differential result) to fail.
`test-leftovers-mutants` edits the `.om` source, regenerates each scalar unit
with the pinned bridge, and requires assertion exit 1 for every mutant. Compiler
failure or a signal is not a killed mutant. The scalar sites cover alphabet,
punctuation, ordinal, acquisition, poison, context and extent. Dot-dot and capacity
moved to `tests/pathcopy_mutants.py`.

## Partitions and coverage

Paths cover empty/null output and input, cap zero, exact capacity and adjacent
capacities, all byte values, ASCII range endpoints, ordinary and repeated
slashes, terminal slash, dot/dot-dot/longer-dot components and embedded refused
bytes. Signed ordinals cover INT_MIN, -1, 0, 1, INT_MAX. Atomic states cover idle,
busy, poison and a non-enumerated state, including late release after poison.
Context and release counts cover 0, 1, 31, 32, 33, u32 max and adjacent values,
signed boundary and u64 max. A null ABI result is refused. Step capacity tests
include u32/u64 max. The changed policy has no floating arithmetic.

`test-leftovers-coverage` compiles the actual C oracle with `--coverage -O0`.
Uncovered test assertion-failure branches are intentional: the separate red
check and mutants exercise checker failure. The generated scalar ABI cannot
refuse correctly aligned, valid scalar calls: the two unexecuted production
path error branches are defensive transport failures, not uncovered C oracle
policy. Header functions not emitted by this focused test perform host lease
projection; the existing native-runtime gate is the integration acceptance
path for those effects, not a claim of branch coverage in this oracle.

## Chip runbook — NOT_RUN

No device, live RM or VFIO execution is authorized. After an operator grants a
current exclusive window, the integrator must rerun the repository's native
runtime/artifact and platform admission checks at the integrated revision,
then use its existing documented GPU launch/admission procedure for a minimal
kernel. Retain the original comparison and refusal receipts and close the
window through the operator runbook. Do not infer authorization from a free
lock, a host PASS, a historical receipt or this document. No performance or
hardware qualification claim is made by this scalar migration.

No omega-harness guide semantics, compiler/bridge pin, repository ownership or
qualification procedure changed; no guide update is required.
