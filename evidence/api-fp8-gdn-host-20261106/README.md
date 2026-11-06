# FP8/GDN host policy and scan carry migration

Base: `feat/api-views-elem-om` at `64fd424e757ca2dd2bfd8ef39ab42c317059f08d` (#323).

## Implementation boundary

`om/apiviews/hostrest.om` owns FP8 environment-value interpretation, shape defaults, register-file capacity, deep staging choice, warp/coal choice, grouped CTA and grid policy, decimal accumulation, cache decision, GDN move admission and CTA count, prefill token/scratch/pointer admission, and prefill grid dimensions. `omega_gpu_api_hostrest.h` is its strong checked ABI shell. Its numeric operation selector is private, not a public API. Operations 0..24 map in source order to flag, variant, shape, register fit, group CTAs, digit, warps, coal, depth, refusal, grid, cache, move, tokens, scratch, prefill grid, host staging, row admission, budget, big-k, depth budget, comparison, quotient, pointer admission, and partial-depth eligibility.

The C numeric oracle is compiled only with `CUDAPP_MIGRATION_PENDING_C_API_FP8_GDN_HOST` (or the differential test define). Scan's original carry loop is compiled only with `CUDAPP_MIGRATION_PENDING_C_SCAN_REFERENCE_REST`. Neither flag is enabled by default. No weak entry or runtime C fallback is used.

The `.om` runtime-view scan carry executes each addition as `tot[e / 32 - 1] + out[e]`. It does not reassociate or reorder the recursive scan. Existing #323 typed transforms are unchanged. Existing `pf_layout` is unchanged so the #316/#280 Omega layout can be retained when this stacked branch is integrated. Public headers, manifest and API signatures are unchanged.

C retains effect acquisition (`getenv`, clock, memory allocation, session operations), byte movement, pointer-to-presence extraction, and effect sequencing. `memcmp` supplies cache identity; host `isfinite` remains the existing libm classification call. Generated C remains checked Omega Bridge output, not a second hand-written implementation. No dependency or compiler pin was changed.

## Reproduce

Regenerate using the existing base pins:

```sh
OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc \
OMEGA_BRIDGE=/tmp/bridge-71 \
OMEGA_BRIDGE_RT=/tmp/integ/bridge-73-79e8c36 sh om/units.sh
```

Compiler revision `e5b32567`, SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

```sh
make test-api-hostrest
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  make -j3 test-gpu-fp8 test-gpu-gdn test-gpu-scan
```

Every executable test/mutant/simulator runs under that device-isolating wrapper. The mutation runner uses temporary files and never edits production sources. `acceptance.log` and `families.log` contain commands and return codes.

## EXERCISED verdicts

- Host differential: PASS. Source-derived thresholds include neighbors of the lane, warp, group, grid, decimal-parser and register-file boundaries, zero and UINT32_MAX; pointer predicates cover all presence masks. Deep decisions are tested only on defined nonzero divisors, matching their supported shape contract.
- Scan host differential: PASS, both full recursive prefix and direct cross-tile carry. Lengths cross tile and recursive boundaries. Float fixtures include positive/negative zero, positive/negative subnormal, finite values, infinities and NaNs. Non-NaNs compare by bits, NaNs by class. All builds use `-ffp-contract=off`.
- Host red control: injected result corruption is caught (rc 134).
- Scan red control: subtract-instead-of-add source mutation is caught (rc 134).
- Strong-link negative controls: removing each Omega unit fails linkage.
- `.om` mutants: six killed for API_FP8_GDN_HOST, six killed for SCAN_REFERENCE_REST. Exact substitutions and rc values are in `acceptance.log`.
- FP8 host: PASS, 4997 checks; simulator: PASS, 214 checks.
- GDN host: PASS, 68 checks; simulator: PASS, 46 checks.
- Scan simulator: PASS, 139 checks. Its host reference is additionally covered by the recursive differential above.

## Qualification and chip runbook

QUALIFIED: host/simulator only. Chip, live-RM, VFIO, boot and hardware tiers: NOT_RUN. No GPU window was granted. Nothing is merged or deployed by this lane.

After the operator grants an exclusive GPU window through the current procedure, the integrator must verify the live service and GPU holders, use the repository's current FP8/GDN/scan chip runbooks and their accepted device/lock protocol, then run the family binaries' chip tier and negative controls against the default Omega path. Do not infer authorization from this document or reuse a historical lock receipt. Record exact revision, driver, device, command, expected and observed result, and negative control in new evidence. Until then no chip PASS or roadmap status upgrade is justified.

No language, bridge profile, compiler error, harness pin, repository ownership or gate definition changed, so no omega-harness skill update is required.
