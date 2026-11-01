# Experimental prompt matrix shapes

The Omega policy admits the existing 64-row, eight-warp matrix kernel, an experimental 32-row, four-warp kernel, and the 128-row experiment described below. The default remains 64 rows. The current parameterized emitter, arithmetic order, wait distance, tail masks and device ownership checks are retained.

The 32-row shape reduces padded work for short prompts. This is a hypothesis until chip and full-model measurements establish a benefit. The emitter stages scales in paired row groups; a 16-row shape is not admitted. Shapes outside the explicit admission list remain refused.

`make test-prefill-shape` checks the portable admission boundary, host numerical references and simulated direct/indexed matrix cases, including row tails, natural SwiGLU order and deliberate faults. It does not open a GPU. `om/units.sh --check` verifies generated bindings with the pinned Omega compiler and Bridge.

Hardware reference: NVIDIA PTX ISA9.4, physical page605, warp matrix instruction examples. This change reuses the existing instruction generator and changes its work grouping. The manual is not evidence that this implementation runs correctly; simulator and chip receipts supply that evidence.

The existing `omega_gpu_pf_qgemm_set_shape(2, 4, 45)` test knob selects the candidate. Call it before sizing prefill buffers or building tile tables. Never change shape while a model operation is in flight. No serving option or automatic policy is introduced here.

The separate serving screen in `serving-validation.json` passes two128-token reference runs with a seeded sampled request between them. First-visible latency remains about0.5seconds on the shared machine. The four-matrix chip comparison is mixed; this shape is not promoted as a speed improvement. Longer contexts, the full conversation/streaming gate and controlled pairs remain pending.


## 128-row experiment

This branch additionally admits `rt=8, warps=8` and restores the original `rt=4, warps=16` policy entry. The latter still fails the emitter's existing staging-slot divisibility guard, as it did before the policy was extracted. Only the 128-row candidate opts its generated program into the existing extended register limit. The launcher retains its declared-register and thread-product checks. Defaults, arithmetic order, instruction encodings and wait distance stay unchanged.

The hardware basis is NVIDIA Blackwell Tuning Guide, physical page13: 255 registers per thread and64K registers per SM. The project already provides a conservative246-register allocation limit and rounds the declared requirement separately. These limits make an experiment reasonable; they do not prove it is faster or even that this generated kernel passes.

The checked Omega policy also bounds the actual largest emitted store offset, including the final column offset for F32 output. This replaces the conservative full-tile extent test that rejected a valid 27B projection. Width, epilogue and row-group bounds are checked before multiplication. The independent oracle enumerates the emitted offsets at both sides of the admission boundary, and mutation controls remove each important part of the bound.

Before the change, all four 27B projection shapes refused 128-row generation. The candidate is still UNQUALIFIED: host generation and simulator checks pass locally, but chip/full-model parity and timing remain pending. The existing 64-row kernel bytes and resource metadata match the original build for the tested direct and indexed 27B projections. Earlier32-row receipts remain historical and do not certify this branch. The current64-row/eight-warp default is unchanged.

[The host receipt](large-shape-host-validation.json) pins the changed sources and archive and records the targeted host, simulator and mutation checks. The full host suite was still running when this draft was published. No chip or full-model result is implied by this receipt.

The subsequent [full host-suite receipt](large-shape-full-host-validation.json) records all 77 targets passing on the candidate source. It includes the failed absolute-output-path invocation and its correction. GPU and full-model qualification remain pending.

## Grouped work-order experiment

The separate grouped-order candidate revisits the mechanism in historical Omega commit `86211a39965bd903b6be482cfc784c7dd58733a5`. It schedules nearby matrix jobs over the same weight panel before advancing to another panel. Historical chip screens motivated that work, but their shared-machine measurements and sampled-row checks do not qualify this implementation or a full model.

`order.om` owns the grid and coordinate-remapping instruction plan. Group 0 keeps the original order. Groups 1 through 5 gather consecutive row tiles into groups of powers of two. The Omega plan checks input and grid bounds before returning success. Jobs beyond the last tile branch out before reading the tile table. The existing C emitter interprets only the plan's primitive instructions; it does not calculate the coordinate mapping.

The private plan is 64 unsigned words: grid x, grid y, instruction count, then five-word records `(operation, destination, source1, source2, immediate)`. Register roles are column coordinate, row coordinate, temporary and tile count. Operations are AND-immediate, shift-right-immediate, multiply-add-immediate, constant load, unsigned greater-or-equal predicate, and predicate branch. The prefix emits the mapping plus a tail guard; the output-store prefix repeats the same mapping. Matrix arithmetic and the existing wait distance are unchanged.

Select a candidate with `omega_gpu_pf_qgemm_set_group(group)` before model work begins. Never change it while a model operation is in flight. This composes with the existing shape test knob. Default group 0 remains unchanged. `order-check.py` independently interprets the plan and verifies coverage, uniqueness and boundaries; `order-mutants.py` checks source faults. The matrix simulator exercises groups 0, 1, 3 and 4 across direct/indexed inputs and partial tiles. GPU and full-model correctness and speed remain unqualified.

The complete 78-target host suite and API version check pass. On the full 27B reader, group4 and group0 preserve every output-score bit and next-token choice in all six frozen cases. Repeated 5,017-token reading was 18.817 seconds with group0 and 16.199 seconds with group4 in this shared-machine screen. This is not quiet-machine qualification. Group0 remains the default. `order-validation.json` records source identities, checks and per-case observations. The separate 128-row full-model attempt was refused by an older prefill-cache register cap before its first matrix; that correction is being verified on the parent branch and is not part of this tested group4 binary.
