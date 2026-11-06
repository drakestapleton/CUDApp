# Attention workspace, scan verification and elementwise clock policy

IMPLEMENTED and EXERCISED on the host. Chip qualification NOT_RUN. Not merged or deployed.

Base: #340 `feat/api-kernel-gen-om` at `0463629a6d16`. Unit registration is first (`b3611ff8`). Existing API kernel generation and view migrations are preserved. Public APIs and launch/refusal/trust semantics are unchanged.

## Source and dependency pins

- First-party Omega: `om/api/attn_host.om`, `om/scan/scan_check.om`, `om/api/clock_ns.om`. The dispatcher `src/omega_attn_host_policy.h` supplies strong calls, not weak-symbol fallbacks. Generated bridge C and identities are committed through the existing `om/units.sh` path.
- Seeded compiler: omega-mv `e5b32567`, SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Hosted converter: inherited omega-bridge `79e8c3613a6f6f9e8eaaa9c013fc25bcb39e0538`, `toc.om` SHA-256 `d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538` (`/tmp/bridge-73-apiviews`). No second converter pin or production runtime dependency is introduced.
- Generation uses `OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/tmp/bridge-73-apiviews OMEGA_SEED=~/work/cudastack/omega-bridge/build/bootstrap/omc sh om/units.sh`.

Default-OFF C oracles: `CUDAPP_MIGRATION_PENDING_C_ATTENTION_HOST_POLICY`, `CUDAPP_MIGRATION_PENDING_C_SCAN_CHECK_REST`, `CUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS`. The clock flag affects only this assignment's elementwise caller. Other engine/m16 clock work is outside this PR.

Host effects stay in C: allocator/free, launch submission, clock_gettime, error reporting, and byte-moving pointer windows. Thread/shared-memory rounding, allocation/chunk sizes and offsets, poison/prefix/sorted-neighbor reductions and seconds/nanoseconds arithmetic execute in seeded Omega by default.

## Differential partitions

`tests/attn_host_fixture.c` builds the real dispatchers twice, default Omega and compile-time C oracle. `tests/attn_host_diff.c` compares outputs and the complete transport array after each operation.

- Thread counts and shared bytes: 0, 1, tile/warp boundaries v-1/v/v+1, dimensions 127/128/129 and 255/256/257, 1023/1024/1025, UINT32_MAX-1 and UINT32_MAX. Unsigned wrap is retained, including the original 32-bit offset/parameter truncation.
- Workspace and chunks: rows/head-count cross-product, with/without block IDs, cursor 0/last-row/end/one-past-end; rows per launch boundary v-1/v/v+1; max-block and offset transport boundaries. No new policy guard is introduced.
- Scan checks: 0,1,2,31/32/33,255/256/257 and seed-loop chunk 999999/1000000/1000001; modular uint32 prefix differences include wrap; all-poison output, adjacent poison constants, sorted output and descending neighbor red controls. The source splits verification into bounded chunks without lowering the public input limit or weakening the loop guard.
- Timestamp conversion: zero, one, source unit-scale boundaries, UINT32_MAX and UINT64_MAX edges for both inputs. Exact unsigned wrap agrees with the original C expression.

Known-good prefix/sort outputs pass; corrupted last prefix word and descending neighbors fail. Removing any one of `omega_ah_plan`, `omega_sc_check`, `omega_clock_ns` makes the link fail. `commands.json` records each command, rc and output. Paths are normalized from `/home/drakestapleton` to `~`. No binary, cubin, PTX or disassembly is committed.

## Commands actually run

Every test and simulator invocation uses this device-hiding prefix:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

| Command after prefix | rc | Final line |
| --- | --- | --- |
| `env OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/tmp/bridge-73-apiviews python3 tests/attn_host_check.py --mutants --out build/attn-host-check` | 0 | `ATTN-HOST-CHECK PASS` |

## Seeded source mutants

Each mutant is made in `.om`, recompiled using the pinned seeded converter/compiler, linked instead of only its generated unit, and rejected by the same differential. Exact substitutions and return codes are in `mutants.json`.

| Unit | Semantic mutation | Verdict |
| --- | --- | --- |
| attn_host | thread rounding +30 rather than +31 | KILLED |
| attn_host | shared size scale 8 rather than 16 | KILLED |
| attn_host | workspace rows stride 3 rather than 4 | KILLED |
| attn_host | reverse chunk minimum comparison | KILLED |
| attn_host | head-map byte offset scale 8 rather than 4 | KILLED |
| attn_scanchk | wrong poison word | KILLED |
| attn_scanchk | add rather than subtract prefix neighbors | KILLED |
| attn_scanchk | reverse sorted-neighbor comparison | KILLED |
| attn_clock | wrong timestamp unit scale | KILLED |

## Constant provenance

`WolframLanguageEvaluator: {FromDigits["7fa00bad",16],FromDigits["ffffffe0",16]} -> {2141195181,4294967264}`. These are encodings of source constants, not measurements.

## Chip runbook and remaining qualification

Chip/live-RM/VFIO/HW tiers are NOT_RUN: no GPU window is authorized. Full repository integration gate belongs to the integrator and was not run here. Host/simulator results are not chip qualification.

In a future explicitly authorized exclusive window, follow the then-current repository hardware procedure, verify actual device/service ownership, rebuild at the candidate revision and rerun the host gates. Run the documented chip modes of `build/gpu_attention_test`, `build/gpu_elementwise_test` and `build/gpu_scan_test` with their comparator/negative-control tiers; record resulting receipts in a new evidence directory. For scans include prefix f32/u32 and radix keys/pairs so the post-launch verification callsites execute. This runbook grants no permission to touch a live service, GPU lock, driver or device. No chip result is claimed here.

## Focused family receipt

The same device-hiding prefix ran `make -j3 test-gpu-attention test-gpu-elementwise test-gpu-scan`, rc 0, final line `PASS: 348 checks, 0 failed`. `family-gates.log` is the complete receipt. Elementwise host checks: 24, zero failed. Attention host checks: 137 for each standard/head-dimension tier; attention simulator tiers and sweep all passed, including general sweep 318 of 318 configurations and five caught general mutants. Scan: `C3 scan sim: PASS (263 checks)`. Expected divergent-warp/shared-range simulator diagnostics are caught red controls in the existing attention gate, not unexplained failures. The elementwise API gate has host and chip modes but no simulator mode; no nonexistent elementwise simulator tier is claimed.

Host differential: PASS. Content and missing-unit red controls: PASS. Nine seeded .om mutants: KILLED. Family host checks: PASS. Attention/scan simulator checks: PASS. Chip/live-RM/VFIO/HW: NOT_RUN.
