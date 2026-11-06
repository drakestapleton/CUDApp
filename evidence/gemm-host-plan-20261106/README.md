# GEMM host planning migration — 2026-11-06

Base: `feat/dnn-layers-om` at `2d87b5849fe5` (PR #307). First registration commit: `cf3d5dea`; narrow ABI registration: `b734707b`. The production policy is seeded Omega in `om/gemm-any/host.om`, exported through narrow scalar/config/plan/span units and the drive entry. The generated C is a checked bridge artifact, not a hand-written replacement. Public APIs and layouts are unchanged.

`CUDAPP_MIGRATION_PENDING_C_GEMM_HOST_PLAN` defaults to 0. Setting it to 1 selects the retained C policy oracle, independently of the GEMM kernel-builder oracle. The C default path only marshals fields, copies launch words, calls the callback, and performs session/allocation/environment effects. Packed allocation effects are not exercised without an authorized hardware window; their pure reuse/cache/alignment decisions are compared independently.

## Reproduction

Use the base's compiler/bridge pins; do not select another converter:

```sh
export OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc
export OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make test-gemm-host-plan-policy
```

The runner checks byte-for-byte regeneration, builds with `make -j3`, compiles the C oracle harness with `-O0 --coverage -ffp-contract=off`, compares every selected output field and launch argument, flips the expected comparator result, removes the Omega units from a copy of the real production archive and requires a missing-symbol link failure, then compiles and kills eight `.om` mutants. It also runs the existing GEMM host/simulator and GEMM-any simulator gates. `receipt.txt` records the observed command outcome and final lines; full local subprocess output is `build/gemm-host-plan/check.log`. Raw gcov reports and the function summary are retained under that build directory.

## Source-derived partitions

- Types 0..3 and invalid 4; unsigned maximum and neighbors in each config field. Tile choices 1/2/4 and neighbors; stall 1/15 and neighbors; mutant 0/3 and neighbors; band 0/1/2/4/8/16/32 and neighbors. Every valid tile/warp/band combination is compared.
- Shapes surround the source's 16/32/64/128/256 rounding boundaries and 16384 admission bound, including zero and `UINT32_MAX`. Each plan field (including cleared FFMA pack fields) is compared.
- All four layouts and invalid layout 4, dense/padded leading dimensions, exact `UINT32_MAX` span successes and overflow refusals, and `UINT64_MAX` leading dimensions. Nonzero dimensions are checked independently in plan and span.
- Drive records compare grids, thread counts and all ten words for each launch, dense/padded/oversized strides, FFMA's one-launch route, and callback refusal at each launch position.
- Scalar resource calculation includes unsigned-32 wrap; alignment includes unsigned-64 wrap; reuse includes null/non-null and capacity comparison; cache environment values include missing, empty, `0`, `00`, `1`, and `01`.

The eight non-equivalent mutants alter tile membership, max-dimension comparison, rounding, pack padding, span bound, band-grid axis, alignment and main-register arithmetic. Each must compile and fail the differential comparator. The separate red comparator and production missing-unit link check must also fail as expected. See `receipt.txt` for exact first mismatches. gcov is descriptive evidence, not a claim that hardware effects or every C-shell pointer guard ran.

WolframLanguageEvaluator: `{2^64-256, 2^32-1, FullSimplify[4 (4 r+2 c+r c)==16 r+8 c+4 r c]}` -> `{18446744073709551360, 4294967295, True}`. Other constants and formulas are copied directly from the named base C policy; expected values come from that retained C oracle, not an independent ad-hoc calculator.

## Tiers and hardware runbook

IMPLEMENTED: policy cutover and strong Omega entries. EXERCISED: see receipt for host/simulator/differential verdicts. QUALIFIED on GB10: NOT_RUN. MERGED: no. DEPLOYED: no.

Chip, live-RM and VFIO tests are NOT_RUN. After Drake grants the current exclusive GPU window and the operator completes the current safety/ownership checks, build this exact PR head with the pinned tools and run `build/gpu_gemm_test` without `--host-only`/`--sim`, followed by the existing GEMM-any hardware command in its runbook. Record exact head, device/session state, return codes and negative controls in new evidence. Do not execute this hardware tier under the device-hidden host wrapper, do not treat simulator success as a chip receipt, and do not disturb serving outside that authorization.

No language/compiler/bridge contract, source pin, public API, repository ownership or global gate changed; therefore no omega-harness skill example or pin update is required by this runtime-unit-only migration.
