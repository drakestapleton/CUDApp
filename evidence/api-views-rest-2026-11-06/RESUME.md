# Continue API views rest

This checkpoint is WIP because the user explicitly requested wrapping up the current step. Do not merge or treat the migration as complete. Integration owner: Integrator4. Worktree `~/work/cudastack/lanes/ApiViewsRest`, branch `feat/api-views-rest-om`, stacked on #319 0e4417ae and #318 f21ba39d. No GPU or live-service authority exists.

## Owned source paths

- src/omega_gpu_moe_api.c
- src/omega_gpu_tern_api.c
- src/omega_gpu_tcgemm_api.c
- src/omega_gpu_pfgemm.c
- src/omega_gpu_fp8gemm_api.c
- src/omega_gpu_fp8_slots.c
- src/omega_fp8_slot_table.c
- src/omega_gpu_tgemm_api.c
- src/omega_gpu_pfhost.c
- src/omega_gpu_fp8_pack.c

Shared integration paths: `om/units.sh`, `src/cudapp_native.h`, `src/cudapp_om_units.ident`, `src/cudapp_om_apish_shp.ident`, `om/api/apish_shp.om`, `om/api/.crumb`, `tests/api_shells_run.sh`, `tests/api_shells_mutants.py`. New owned roots: `om/apirest/`, `src/omega_api_views_rest.h`, `src/cudapp_om_ar_*`, this evidence directory. The bf16 canonical helper was copied from ApiViewsElem #323 33c33612; reuse it during integration instead of introducing a duplicate definition.

## Registered unit batches

Registration commits precede the C cutover. Latest registration is 90a13e8; earlier batches are ed14dd7, 5fa810c, d6d328a, de13732, 38c46ce, 66be537, 35e101c. The separate authorized `part` rename is f138336. Runtime bridge pin commit is 6e436ff. Bridge checkout `~/work/cudastack/lanes/ApiViewsRestBridge` remains detached at #73 79e8c3613a6f; keep it available until the pinned bridge changes land.

Generated units include bf16; zero/copy/count/argument transport; route/permutation/widen/gather; FP8 NaN/scale/fold and two-pass f64 acc/magnitude; tile/group/run iteration; tern packing/validation/reverse; prefill copy/ratio transforms; spec/key/cache/slot decisions; tensor-core descriptors; grouped GEMM descriptors and activation-row cache iteration. `ar_calc` operations 9..12 and `ar_victim`/`ar_mstep` are registered but not yet wired into MoE. Runtime-view extents avoid the fixed-view 2^24 limit.

## Provider interface — available, not adopted in this branch

CheckersRest2 pushed **343a061**, branch `feat/checkers-rest-om`. They own the canonical shared fragment packers and result_sentinels in `omega_blackwell_codegen.c`. Do not create private fragb/mrow duplicates.

Canonical source files:
- `om/codegen/cg_pack_len.om`
- `om/codegen/cg_pack_h.om`
- `om/codegen/cg_mrow.om`

ABI:
- `omega_cg_pklen(&result, p, 12)` computes p8/p9 and validates.
- `omega_cg_pkh(&result, p, 12, source_bytes, p[8], destination_u16, p[9])`.
- `omega_mrow(&result, q, 3, source_f32, source_count, destination_f32, destination_count)`.

Each returns uint32 bridge status and uint64 semantic result (zero success, signed -1 refusal). Metadata `p[12]={kind,m,k,mp,kp,n,nt0,nt1,source_bytes,destination_ushort_count,0,0}`; kind2=B; kinds0/1=ushort A/row0; kinds3/4=f32 A/row0. Row compaction q={m,n,np}; runtime bridge refuses overlapping read/edit views. `cg_pack_h` and `cg_mrow` require --runtime-views; `cg_pack_len` uses normal seeded lowering. Copy only canonical files/entries/generated units, not the peer's whole shared header/registry. Provider acceptance was reported by the peer but not independently reproduced here.

## Exact reachable work remaining

1. Wire MoE chunk packing and row compaction to the canonical providers above. Its only completed cutover is f32-to-bf16. `ar_victim` and `ar_mstep` plus `ar_calc` operations 9..12 are available to finish its cache search/victim selection, geometry, unroll decision, upload chunks, window admission and row-launch iteration. MoE still contains its original C cache scans, upload chunk loop, row compaction loop, arithmetic, descriptor fills and effect loops. Do not claim these are migrated.
2. Finish remaining scalar launch/spec/descriptor arithmetic in tern, tgemm, pfgemm and pfhost; their heavy data loops are cut over, but this is not a complete residue removal. Cache entry publication and immutable pointer/object transport must be distinguished from plain data decisions. Existing public effect calls remain C intentionally.
3. Expand the API-shell standalone runner link list. It currently adds only six ar_pack units; final host changes also require the generated ar_* slot/copy/pad/etc. units. Whole-library linkage and both oracle/default code-generation builds have NOT_RUN; syntax-only checks passed.
4. Run source-derived differential partitions, all boundaries v-1/v/v+1 and 0/UINT32_MAX, explicit short-view/refusal controls, a red control, and >=5 killed mutants. Float compile flags must include -ffp-contract=off; compare NaNs by class. Validate ar_perm and typed gather layouts against original C before relying on them.
5. Run existing affected host and simulator targets after the writer finishes, never full make test. Everything runs under `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`; build at most -j3. The final branch has no behavior qualification yet.
6. Benchmark moved loops against gated original C, three samples each. Compute medians and ratios with Wolfram. The FP8 row oracle deliberately performs two product passes because accumulator and magnitude are f64; Main approved this and forbade a seven-slot extension. Record the measured cost in the PR body.
7. Update the existing API documentation and matching harness guidance when the actual final behavior is settled. Keep this PR draft, marked pending omega-bridge#71/#73, until all acceptance is satisfied. Do not merge/deploy.

## C that intentionally remains

Pointer acquisition and opaque object access; session locks; allocation/free; GPU uploads/downloads; launch/record/submit; code generation and cache lifecycle effects; timers, diagnostics and error formatting remain host effects. These are not a blanket excuse for the unfinished pure loops listed above. All original moved pure algorithms are retained only behind default-OFF `CUDAPP_MIGRATION_PENDING_API_VIEWS_REST`.

No background job or test process remains. The lane claim will be closed at handoff; re-lease before continuation. No service, GPU lock, kernel module, driver, port or hardware state was changed.
