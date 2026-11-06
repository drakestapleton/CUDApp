# Attention-family host typed views — 2026-11-06

## Scope and pins

Inventory slice 7, part A: host buffer walks, descriptor words and launch-argument marshalling in `omega_gpu_attention_api.c`, decode attention 128/256, prefill attention 128/256, router and bf16 GEMV. Public C API signatures and GPU kernels are unchanged.

Stacked on CUDApp #319 (`0e4417ae62ce2fbfbdda6be4a2c3e684d373c2cd`), carrying the #318 API-shell and #306 API-plan prerequisites. Registration is commit `8470e0d`; the separately committed runtime converter pin is `84c1100`. The implementation uses the explicitly approved successor to #319's converter:

- omega-bridge `79e8c3613a6f6f9e8eaaa9c013fc25bcb39e0538`, draft #73, stacked on draft #71. **Pending omega-bridge#71 and #73; do not merge this consumer first.**
- `toc.om` SHA-256 `d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538`.
- `bridge-seeded` SHA-256 `a3a455bd77381e132215924295763e8728889d7d578278ae376de78324be8629`.
- Seeded compiler source pin `e5b32567`; compiler SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

Attention data roots select `bridge-seeded --runtime-views`. Fixed argument/layout roots retain exact 32-element views. Existing freestanding units retain their profile; this is hosted checked-runtime evidence, not freestanding or native GPU-launch qualification. `om/omega-bridge.lock` records the pin and this distinction.

## Implementation

`om/api/attn_*.om` owns byte copy/fill, uint/f32 fill/count, signed-context conversion, bf16 classification/expansion, row and group tables, compaction, router poison/candidate scans, RoPE generation, ranges, resident/deduplicated tables, block checks, gather/stage copies, reset, head mapping, effective contexts, split selection and folding. Float arithmetic preserves the original operation order with `-ffp-contract=off`; non-NaN results compare bitwise, NaNs compare by class.

The C adapters acquire pointers, form scalar metadata and invoke checked Omega roots. The old C data-walk oracle exists only behind `CUDAPP_MIGRATION_PENDING_ATTN_VIEWS`, default **0**. Generated checked C is converter output, not a handwritten substitute for Omega.

The six-slot seed ABI is handled without imposing an overall data-capacity limit:

- Effective contexts: Omega copies `ctx` to the output, then an Omega pass clamps output against `nb`. Each pass has at most two unsized data views.
- Group descriptors: separate low-word and high-word passes over an unsized source and destination; one scalar selects the pass.
- Block tables: Omega copies IDs into the destination row, maps that row against the slot table, records touched IDs for newly allocated slots, then writes the context sentinel through the Omega fill root. The touch pass receives the preexisting slot count; it preserves prior touched bytes rather than rewriting existing slots. No intermediate allocation is needed. A new long-table differential caught an earlier unconditional touch pass changing those bytes; the corrected pass covers both preexisting and newly allocated slots, including the tail.
- Touch and stage use caller-chosen pointer windows, not C data walks. Each has two unsized data views, one numeric-capacity ID view and an additional scalar/metadata argument; making all three data views unsized would require seven physical slots. Touch's ID window has capacity 1,000,000 and receives the prior slot count; stage's ID declaration has capacity 16,777,216. Both C shells dispatch actual ID windows of at most 1,000,000 elements. Source/destination byte extents and the overall number of IDs remain unrestricted by these windows. These numeric declarations are per-call capacities, not public API ceilings. There is one tail path, no tail-size variants, and no overdeclared live view. A three-unsized touch prototype also exceeded the seed register budget when traversal was nested; no compiler budget or loop guard was weakened.
- Other long walks split traversal inside Omega. The existing 1,000,000-iteration guard is unchanged. An earlier GQA simulator run exposed a trap copying 2 MiB; bounded Omega helper traversal fixes it. The differential now crosses the boundary with 1,000,003 elements and exercises a stage tail. A real PROT_NONE mapping, never dereferenced, probes extent admission above the former 16,777,216-element ceiling instead of declaring a large view over a small allocation. That probe is admission evidence, not evidence of reading a large buffer.

All argument counts are actual element counts. Nonempty writable overlaps are refused by the generated view adapters; adjacent ranges and empty views are covered. Bounds traps and argument refusals stay distinct. No GPU, RM, VFIO, service or driver gate is weakened.

## Reproduction

From the lane checkout, set the exact compiler and bridge paths:

```sh
export OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc"
export OMEGA_BRIDGE="$HOME/work/cudastack/lanes/bridge-attn-71"
export OMEGA_SEED="$HOME/work/cudastack/omega-bridge/build/bootstrap/omc"
sh om/units.sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  env OMEGA_OMC="$OMEGA_OMC" OMEGA_BRIDGE="$OMEGA_BRIDGE" \
  sh tests/attn_views_run.sh --mutants --coverage --bench --api --out build/attn-views-receipt
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  env OMEGA_OMC="$OMEGA_OMC" OMEGA_BRIDGE="$OMEGA_BRIDGE" sh om/units.sh --check
```

Generation is not behavioral evidence. All differential, mutant, benchmark, API host/simulator and debugger test processes run behind the device-hiding bwrap boundary. The runner records every subprocess command, exit status and complete captured output in `commands.json`; build concurrency is `-j3`. Focused Make targets are `test-attn-views`, `test-attn-views-mutants`, `test-attn-views-coverage` and `bench-attn-views`.

## Domain partitions and oracle

Argument packing exhausts address counts 0..16 and scalar tails through 32 words, with invalid address count 17, word count 33 and a too-short argument layout. Address high/low halves and uint32 truncation are compared. Sparse layout covers valid masks and the forbidden final address slot.

Host fixtures cover zero/positive extents, self-copy no-op, byte patterns, UINT32_MAX, uint32 row/offset wrap, signed INT32_MIN/-1/0/1/INT32_MAX contexts, negative/valid compaction IDs, finite/nonfinite bf16 words and NaNs, both poison outcomes, both candidate/count outcomes, RoPE identity/rotary branches, two widths and theta values, range clipping, three table modes, duplicate/new/existing slots, block and resident-fit refusal, strided gather/stage, reset, split selection, empty folding and finite/underflowing fold gaps. New long-buffer cases cross the unchanged iteration guard.

Shape/alignment/overflow/refusal decisions already migrated in #306/#318 retain their existing API tests; this lane does not duplicate or relax them. `omega_api_plan.h.gcov` is incidental dependency coverage, not this lane's claim to requalify every family plan. The owned C oracle is `omega_attn_views.h` plus `omega_attn_tables.h`.

## C retained by file

- `omega_gpu_attention_api.c`: kernel-IR construction (outside this host-data slice), session/cache pointer acquisition, allocation/free/synchronization, actual launch/record calls and ABI diagnostics. Host numeric buffers and descriptors call Omega.
- `omega_gpu_decode_attn_api.c` and `omega_gpu_decode_attn256_api.c`: cache/session pointer and resource orchestration, kernel lookup, launch/record calls and ABI diagnostics; RoPE, copies/fills, arguments and scans call Omega.
- `omega_gpu_prefill_attn_api.c` and `omega_gpu_prefill_attn256_api.c`: cache/mirror pointer acquisition and transport, resource/launch sequencing, callbacks and ABI diagnostics. The remaining metadata loop reads fields through kernel pointers; the plain hit-count reduction calls Omega.
- `omega_gpu_router_api.c`: FP8 tensor-entry pointer acquisition, cache/session/launch orchestration and ABI diagnostics. Buffer tables, poison/fill/count work and arguments call Omega.
- `omega_gpu_bf16_gemv_api.c`: cache/session pointer acquisition, allocations/transfers/launches, kernel lookup and ABI diagnostics. Host bf16 expansion/classification, fills/counts/copies and arguments call Omega.
- `omega_attn_views.h` / `omega_attn_tables.h`: checked-call/pointer/metadata glue and the default-OFF C verification oracle. The only C traversal introduced for data units is the explicitly described pointer-window dispatch; it never reads or writes elements.

No additional language or converter extension is required for the implemented call granularity. Native/freestanding variable-view support would be a separate qualified compiler/ABI extension; it is not claimed here.

## Hardware runbook — NOT_RUN

No chip, real RM, VFIO or serving-model gate was run. A coordinated, explicitly authorized exclusive window is required before exposing GPU device nodes. The operator must follow the current CUDApp window/ownership procedure, confirm current serving state and record fresh hardware/driver identity. Do not use this host receipt or an unlocked historical lock as authorization.

After authorization, run the default chip modes with fresh receipt paths:
`build/gpu_attention_test --out <fresh-attention.json>`,
`build/gpu_decode_attn_test --out <fresh-decode.json>`,
`build/gpu_decode_attn256_test --out <fresh-decode256.json>`,
`build/gpu_prefill_attn_test --out <fresh-prefill.json>`,
`build/gpu_prefill_attn256_test --out <fresh-prefill256.json>`,
`build/gpu_router_test --out <fresh-router.json>` and
`build/gpu_bf16_gemv_test --out <fresh-bf16.json>`.
Preserve the existing positive and kernel-mutant controls.
The device-hiding host bwrap command cannot qualify the chip; exposure requires
that separate authorization procedure. Integrator4 owns the serialized full
`make test`, integration, merge and any subsequent hardware qualification.

## Final exercised host receipt

The final source was exercised by job `bg_1418` (rc 0, measured wall time
616.01 seconds; session stdout `artifact://8138`). The reproduction sequence
above used `--out build/attn-views-qualified-final`: generation, the complete
focused runner and `om/units.sh --check`, in that order. Every test, mutant,
coverage executable, API simulation and benchmark
inherited the device-hiding bwrap wrapper. No source changed after that run.

`commands.json` records all 96 runner subprocess commands, return codes and
complete captured output. `commands-index.json` records their final nonempty
lines. Home-directory prefixes in those two files and the copied coverage
receipts were normalized from `/home/drakestapleton` to `~`; no measured values
were changed. An empty subprocess output is recorded as such, not as a PASS.

| Tier / command inside the recorded wrapper | rc | Observed result |
| --- | --- | --- |
| `sh tests/attn_views_run.sh --mutants --coverage --bench --api --out build/attn-views-qualified-final` | 0 | `PASS attention typed-view host gate` |
| `build/attn-views-qualified-final/diff` | 0 | `PASS attention host typed views differential; red check rejected; runtime bounds and aliases refused` |
| Nine compiled Omega mutants | nonzero for every mutant | Killed: argument/layout/copy/bf16/rows/table/compact/RoPE/fold mutations; exact rc and output in command index |
| `make -j3 test-gpu-attention` | 0 | `PASS: 348 checks, 0 failed` |
| `make -j3 test-gpu-decode-attn` | 0 | Host 43 and sim 12 checks, zero failures |
| `make -j3 test-gpu-decode-attn256` | 0 | Host 61 and sim 14 checks, zero failures |
| `make -j3 test-gpu-router` | 0 | Host 88 and sim 15 checks, zero failures |
| `make -j3 test-gpu-bf16-gemv` | 0 | Host 191 and sim 23 checks, zero failures |
| `build/gpu_prefill_attn_test --host-only` / `--sim` | 0 / 0 | 69 / 97 checks, zero failures |
| `build/gpu_prefill_attn256_test --host-only` / `--sim` | 0 / 0 | 19 / 30 checks, zero failures |
| `sh om/units.sh --check` with the recorded pins | 0 | `PASS portable Omega units regenerate byte for byte` |

IMPLEMENTED and EXERCISED: this host-data slice. Hardware QUALIFIED: NOT_RUN.
MERGED and DEPLOYED: no. The full integration `make test` is NOT_RUN here and
belongs to Integrator4. This is pending omega-bridge#71 and #73; it does not
establish native/freestanding runtime-view support.

## Oracle coverage

`gcov.txt` and the four adjacent `.gcov` files are the final `--coverage -O0`
C-oracle receipt. These percentages are gcov's measured output:

| Owned oracle header | Lines executed | Branch sites executed | Branch outcomes taken |
| --- | --- | --- | --- |
| `omega_attn_views.h` | 100% of 56 | 100% of 50 | 100% of 50 |
| `omega_attn_tables.h` | 100% of 58 | 100% of 74 | 98.65% of 74 |

The sole untaken owned outcome is the positive `x > 127` clamp in
`av_exp2_oracle`, source line 145. Fold calls it with a maximum-subtracted,
scaled finite score difference, hence a nonpositive argument; NaN does not
take that comparison either. `fold-domain.wl` records the Wolfram query
`Reduce[x > 127 && x <= 0, x, Reals] -> False`.
The fixture's three untaken branches and uncalled abort are allocation-failure
paths at `attn_views_cases.c:10`, not data-transform behavior. No OOM was induced.
The separately reported `omega_api_plan.h` coverage is incidental dependency
coverage of the earlier #306 plan, not a claim of complete coverage of that
other unit. Existing API gates above exercise its normal public callers.

## Measured host benchmark

Five alternating samples per case, setup outside the timed loop,
`CLOCK_MONOTONIC_RAW`, optimized C and checked hosted Omega, no FMA.
The table is median ns per operation; the ratio is Omega/C. Raw samples are
`bench.tsv`. Every Omega case is slower than its C oracle: this is not a
speedup or a GPU benchmark.

| Moved operation | Omega ns/op | C ns/op | Omega/C |
| --- | ---: | ---: | ---: |
| Nine launch addresses | 691.456 | 11.712 | 59.038 |
| Copy 65,537 bytes | 370959.500 | 1135.000 | 326.837 |
| Expand 65,537 bf16 values | 656787.500 | 153415.500 | 4.281 |
| Eight-row descriptor | 767.552 | 2.528 | 303.620 |
| RoPE 4×128 | 290340.000 | 52356.000 | 5.545 |
| Fold 4×2×8 | 4167.188 | 80.188 | 51.968 |

WolframLanguageEvaluator computed the table using the exact raw-data assignment
in `bench.wl` and the query
`Map[{#[[1]], N[Median[#[[3]]]/#[[2]],10], N[Median[#[[4]]]/#[[2]],10], N[Median[#[[3]]]/Median[#[[4]]],10]}&, data]`.
The returned values are in `bench-wolfram.json`; the separate Wolfram
`NumberForm` formatting query and result are in `bench-formatted.json`.
`constants.wl` records the exact binary64 constant conversions used by the
Omega fold polynomial. No host-language arithmetic supplied those constants,
medians or ratios.
