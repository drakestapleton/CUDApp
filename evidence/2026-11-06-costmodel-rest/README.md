# Costmodel residual host evidence

IMPLEMENTED and EXERCISED on host only. Hardware/chip/live-RM/VFIO: NOT_RUN. MERGED/DEPLOYED: NOT_RUN.

Compiler: omega-mv e5b32567, omc SHA-256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f. Existing pinned bridge, no new host dependency and no API narrowing. Binary64 remains canonical; finite values are bit-exact, NaNs by class. The glibc drift printed by the inherited math gate is not the canonical algorithm differential.

## Command ledger

Each log records the exact command, output and rc. Every executing test/mutant/coverage driver uses `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`. Builds use at most -j3.

| Log | rc | Final observed result |
|---|---:|---|
| focused-final.log | 0 | CM SOURCE mutants PASS (10 attributed arithmetic/header/digest/decision mutants) |
| route-oracle-final.log | 0 | PASS cm route mutants (6 killed) |
| host-policy.log | 0 | CM SORT mutants PASS |
| regeneration-check.log | 0 | PASS portable Omega units regenerate byte for byte |
| coverage-run.log | 0 | CM STATE differential PASS initializations=21 observations=4083 calibrations=3 charges=4 |
| coverage-blob-run.log | 0 | CM BLOB/DIGEST differential PASS (native/big-endian headers, bit mutants, SHA-256 block/tail boundaries) |
| route-coverage-run.log | 0 | PASS cm route C oracle, SHA block boundaries and blob transport |
| crumb-final.log | 0 | crumb verify: OK: 39 crumbs current |

`generate.log` and each coverage *build.log also record rc 0. The focused gate includes test-costmodel (127 checks, zero failures), test-dagsim (203 checks, zero failures and both named mutants), six new route mutants, ten predecessor numerical mutants, exact decision/prediction comparison and the contraction/red verifier controls. The measured integrated comparison is 21058 decisions and 8617 predictions with max_numeric_ulp=0. State comparison covers 21 initializations, 4083 observations, 3 calibrations and 4 charges.

Initial `focused.log` is retained as a failed rc 2 receipt: new binding was missing from the two dagsim mutant hardcoded link lists. Those scripts now include cm_route and disable FP contraction; focused-final is the corrected run. The dagsim parser is untouched.

## Coverage and justified exclusions

The full retained C oracle is compiled with --coverage -O0 -ffp-contract=off and shared by numerical and blob drivers. gcov reports 96.61% of 384 lines, 99.36% of 312 branches executed and 80.45% of 312 branch edges taken. These are gcov measurements, not extrapolated coverage claims. A separate --coverage -O0 route driver selects C_CM_ROUTE=1 with other computation still Omega, exercising every new cm_route2 branch, the retained SHA block plan and 0..130 SHA byte lengths, plus blob boundaries/corruption. Its raw coverage includes other default Omega wrappers it intentionally does not call.

Every untaken edge in the raw reports is listed below by source line and gcov branch ID, with an explicit driver-specific reason. This does not claim the retained #291 algorithms have exhaustive state coverage.

### rx_costmodel.c.gcov

| Function | Untaken line:branch IDs | Justification |
|---|---|---|
| oracle_cm_cell | 108:b1 | The numerical corpus passes non-null feature records; the null ABI guard is checked by the public path, not dereferenced in this retained algorithm driver. |
| oracle_cm_basis | 116:b1, 117:b1 | The retained numerical corpus uses positive M/N. Zero-dimension feature policy is unchanged #291 code; no new feature formula is introduced. |
| cholesky | 189:b0 | The seeded positive prior plus observed updates produces positive pivots. A deliberately corrupted/non-positive-definite statistics matrix is not manufactured by this driver. |
| fit_predict | 219:b2, 221:b1, 228:b0 | The corpus reaches ordinary unknown/known fits; malformed Cholesky, alternate minimum-count short circuit and cancellation below the prior floor are retained #291 robustness alternatives, not exercised by these samples. |
| quiet_predict | 313:b1 | The caller normalizes a validated cell to the quiet-pressure class before this helper. Invalid scalar cell boundaries are exercised directly by the cm_route Cartesian partition. |
| oracle_cm_predict | 358:b1, 370:b1 | The retained corpus has calibrated scale >=1 and the observed residual DOF ordering. The other retained #291 branches are not claimed as covered. |
| oracle_cm_observe | 402:b1, 402:b3, 402:b4, 403:b1, 437:b0 | The numerical oracle receives valid non-null records, admitted arms and nonzero successful durations. Argument/cell scalar admission is independently partitioned in cm_route_unit and production refusal cases. |
| norm_inv | 477:b0, 478:b0, 480:b0, 485:b0 | The integrated decisions request the central confidence quantiles. Endpoint/inverse-normal tail branches are unchanged #291 math-oracle code, not exercised by this integrated corpus. |
| oracle_cm_t_quantile | 500:b0 | Integrated fitted predictions have positive DOF. Nonpositive-DOF fallback remains unchanged #291 oracle code. |
| expected_improvement | 523:b0, 523:b2, 523:b3 | The model prior/noise floor keeps sampled prediction variance positive. The zero-variance fallback is retained #291 code; this driver does not forge invalid fit internals. |
| oracle_cm_decide | 531:b1, 531:b3, 548:b6, 566:b2, 587:b2, 596:b1, 609:b2, 612:b8, 622:b2 | Some short-circuit combinations are absent from the fixed decision corpus (null model/features, eligibility/ranking/unknown and exploration conjunctions). Exact sampled decisions and threshold replay pass; this is not a claim of exhaustive #291 policy-state coverage. |
| oracle_cm_calibrate | 682:b1, 682:b3, 682:b5, 682:b7, 682:b9, 682:b11, 682:b12, 684:b0, 691:b2, 691:b5, 691:b6, 697:b1, 699:b1, 700:b0 | This driver supplies non-null vectors and interior q with successful allocation/sort. OOM, malformed vectors, degenerate residual classes, the defensive rank clamp and a below-one scale are intentionally not fault-injected. These are retained #291 alternatives. |
| oracle_cm_serialize | 897:b1, 897:b2, 908:b1 | Null/short buffer and optional output-length paths are separately exercised by cm_route_oracle; this instrumented predecessor blob driver focuses on valid serialization and bit/header corruption. |
| oracle_cm_deserialize | 913:b1, 913:b2, 915:b1, 929:b1, 929:b3, 929:b5, 929:b7, 930:b1, 930:b3, 931:b0, 960:b1, 960:b2 | The predecessor blob driver tests bit/header corruption; the independent route oracle also supplies null, short/long buffers, every header field and invalid scale classes. Any remaining IDs below are specific to this coverage driver, not a claim of unreachable production input. |

### rx_cm_sort.h.gcov

| Function | Untaken line:branch IDs | Justification |
|---|---|---|
| rx_cm_cmp | 15:b0, 15:b2, 15:b3, 16:b0, 16:b5 | The integrated calibration sample does not compare NaNs or both signed zeros in every ordering. Dedicated cm_sort_unit runs separately; ordering code is unchanged #291. |

### rx_cm_math.h.gcov

| Function | Untaken line:branch IDs | Justification |
|---|---|---|
| rx_host_math | 17:b1 | The scalar bridge wrapper passes its fixed correct ABI. The nonzero bridge-status fail-closed branch requires an invalid generated ABI/runtime, not a numerical input. |

### cm_oracle.c.gcov

| Function | Untaken line:branch IDs | Justification |
|---|---|---|
| oracle_cm_math_trace | 47:b1, 47:b2, 50:b0, 63:b0, 64:b0 | The actual-input tracing driver uses admitted modes and successful bridge status; it does not call log mode 0 or force every sign/ordering in its ULP accounting. This tracing helper is test-only. |

### route-oracle-rx_costmodel.c.gcov

| Function | Untaken line:branch IDs | Justification |
|---|---|---|
| rx_cm_default_policy | 133:b1, 133:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| rx_cm_init | 148:b1 | Only the fail-closed bad bridge-status/stage edge is absent. Fixed-size packets are valid; corrupting a compiled unit is not an input. Separate source-mutant and ABI-refusal gates provide the red controls. |
| rx_cm_cell | 158:b0, 158:b1, 161:b1, 161:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| cm_host_basis | 169:b1, 169:b2, 170:b0, 170:b1 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| fit_predict | 249:b1, 249:b2, 249:b3, 249:b4 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| failures_of | 275:b0, 275:b1, 276:b1, 276:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| fail_p | 283:b0, 283:b1, 286:b1, 286:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| quiet_predict | 313:b1, 313:b2, 317:b1, 317:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| bucket | 326:b1, 326:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| rx_cm_predict | 335:b0, 335:b1, 337:b0, 337:b1, 337:b2, 337:b3, 338:b1, 338:b2, 340:b1, 340:b2, 347:b1, 347:b2, 380:b0, 380:b1, 389:b0, 389:b1, 391:b1, 391:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| rx_cm_observe | 402:b0, 402:b1, 402:b2, 402:b3, 402:b4, 402:b5, 403:b1, 403:b2, 405:b1, 405:b2, 412:b1, 412:b2, 414:b0, 414:b1, 415:b0, 415:b1, 417:b1, 417:b2, 422:b1, 422:b2, 428:b1, 428:b2, 428:b3, 428:b4 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| rx_cm_t_quantile | 508:b1, 508:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| rx_cm_decide | 646:b1, 646:b2, 647:b0, 647:b1, 648:b0, 648:b1, 652:b0, 652:b1, 673:b1, 673:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| rx_cm_calibrate | 711:b0, 711:b1, 711:b2, 711:b3, 711:b4, 711:b5, 711:b6, 711:b7, 714:b1, 714:b2, 714:b3, 714:b4, 716:b0, 716:b1, 721:b0, 721:b1, 729:b1, 729:b2, 732:b0, 732:b1, 735:b1, 735:b2, 738:b0, 738:b1, 740:b1, 740:b2, 742:b1, 742:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| rx_cm_charge | 761:b1, 761:b2 | This focused driver tests route/SHA/blob transport, not this unchanged #291 numerical caller. The integrated numerical/decision and public refusal gates exercise that family separately; no whole-file coverage claim is made for this driver. |
| cm_blob_check | 790:b1 | Only the fail-closed bad bridge-status/stage edge is absent. Fixed-size packets are valid; corrupting a compiled unit is not an input. Separate source-mutant and ABI-refusal gates provide the red controls. |
| cm_digest_step | 803:b1 | Only the fail-closed bad bridge-status/stage edge is absent. Fixed-size packets are valid; corrupting a compiled unit is not an input. Separate source-mutant and ABI-refusal gates provide the red controls. |
| cm_sha256 | 809:b1, 820:b1, 824:b1 | Only the fail-closed bad bridge-status/stage edge is absent. Fixed-size packets are valid; corrupting a compiled unit is not an input. Separate source-mutant and ABI-refusal gates provide the red controls. |
| pack | 875:b1 | Only the fail-closed bad bridge-status/stage edge is absent. Fixed-size packets are valid; corrupting a compiled unit is not an input. Separate source-mutant and ABI-refusal gates provide the red controls. |

## Remaining shell and qualification

Function-level classification and concrete bounded-record bridge extension proposal: tests/runtime/COSTMODEL_REST.md. Native struct layout/copies, pointer safety, packet marshaling and malloc/free stay C; there is no duplicate default numerical implementation. Chip runbook is the final section of that document. No compiler, encoding, bridge pin, API or ownership changes require an omega-harness skill revision; implementation guidance and this runbook are updated locally.
