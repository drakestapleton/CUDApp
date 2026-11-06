# Costmodel residual migration

Base: CUDApp#291, `40ee08eb`. The public API is binary64, not f32. This PR preserves its non-contracting binary64 numerical contract and adds `cm_route.om` for remaining scalar admission and blob/block planning. `CUDAPP_MIGRATION_PENDING_C_CM_ROUTE=1` selects the old scalar arithmetic; it defaults to 0. The other C_CM switches remain default-OFF.

## Function classification

Every definition in `src/runtime/rx_costmodel.c` is accounted for below. Ported rows include their retained default-OFF C oracle, where present.

| Functions | Class | Implementation / retained C reason |
|---|---|---|
| rx_cm_default_policy, rx_cm_init, rx_cm_cell, rx_cm_basis, cm_host_basis, residual_basis, bucket, failures_of, fail_p | Ported #291 | cm_features.om; C transports typed fields and copies returned state. |
| cholesky, forward, backward, fit_predict | Ported #291 | cm_fit.om; oracle-only solver definitions; default fit_predict marshals the binary64 packet. |
| norm_cdf, norm_inv, rx_cm_t_quantile | Ported #291 | cm_math*, cm_quantile.om, including fitting/quantile tables. |
| explore_allowed, expected_improvement, rx_cm_decide, cm_decision_output | Ported #291 | cm_decision.om owns eligibility, thresholds, action and ranking; C gathers arm predictions and copies output fields. |
| quiet_predict | Computation moved / shell | Scalar cell admission now cm_route mode 2; feature lookup and fit already Omega. C selects a typed statistics address, copies the feature structure and transports output pointers. |
| rx_cm_predict | Computation moved / shell | Arm and cell admission now cm_route mode 2; prediction arithmetic already cm_prediction.om (#291). Null pointers and typed field access remain host ABI safety. |
| rx_cm_observe | Computation moved / shell | Arm/cell admission now cm_route; statistics already cm_stats.om (#291). C owns typed-address lookup and packet copying. |
| rx_cm_calibrate, rx_cm_charge | Ported #291 / shell | cm_calibration.om owns validation, residual arithmetic, rank, scale and charging; C owns malloc/free, gathering successful predictions and calling the existing Omega sort. |
| cm_blob_check | Ported #291 | cm_blob.om; C reports native byte order and transports the header. |
| cm_digest_step, cm_digest_equal | Ported #291 | cm_digest.om and cache/digest.om; byte transport remains C. |
| cm_sha256 | Computation moved / shell | cm_route mode 0 supplies full-block count, tail length and tail offset. Hash computation already Omega (#291); the C loop copies foreign memory blocks to the bounded bridge packet. |
| body_size, rx_cm_blob_size, cm_route2 | Computation moved | cm_route modes 1–4 own ABI-size summation, scalar admission and length addition/subtraction; sizeof and native layout are host ABI facts. |
| pack, rx_cm_serialize, rx_cm_deserialize, rx_cm_digest | Ported #291 + computation moved / shell | Blob validation/hash already Omega; route now owns digest payload and serialized length arithmetic. Native-layout memcpy, pointer advancement/differences, null checks and stack output storage remain C because the bridge cannot consume arbitrary C struct pointers. |

No foreign numeric implementation is adopted. Generated checked C is bridge output, not handwritten implementation. Finite prediction/calibration values are compared bit-exactly with `-ffp-contract=off`; NaNs are compared by class, including an explicit distinct-payload comparator case. State bytes remain an exact serialization/state contract.

## Gates

Run from this branch with the pinned compiler and bridge:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-costmodel test-dagsim test-cm-decision-differential test-cm-source-mutants
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/units.sh --check
```

The new route gate runs its source-derived partitions, explicit red result comparator and six independently compiled Omega source mutants. Block boundaries 63/64/65 and 127/128/129, 0, UINT32_MAX±1 and UINT64_MAX are included. Admission and offset pairs use the Cartesian product of those values; each ABI size position is varied independently. Existing binary64 decision/fitting/source-mutant gates remain in place. No dagsim parser files change; its `--wrap=rx_cm_predict` seam remains intact.

The same gate also runs the retained C route oracle, checks all SHA input lengths 0 through 130 against the independent SHA oracle, and exercises native blob round-trip, null/short/long buffers, corrupt checksums, each invalid header field and invalid scale classes. `test-host-policy` preserves the predecessor's blob/math/source-partition gates. Coverage commands, raw gcov output and every uncovered branch justification are in `evidence/2026-11-06-costmodel-rest/`.

### Remaining native-layout boundary

The pinned bridge accepts bounded primitive packets, not field projections over the caller's mixed-layout `RxCostModel`/`RxCmArmStats` C structs. The remaining gather/scatter and block-copy loops are ABI transport, not numerical implementations. Fixed primitive array views alone cannot describe a strided field across these mixed records without reinterpreting a C struct as a different typed array. A concrete bridge extension would accept a byte-backed bounded record view with caller-supplied checked field offsets, widths and record stride; reject offset/stride overflow and out-of-bounds fields before access; and expose bit-preserving field gather/scatter to seeded Omega. It must have mixed-field, padding, endianness, short-buffer and red-bound tests before replacing this shell. No such extension or production compatibility shim is introduced here.

## Chip runbook — NOT_RUN

No chip execution is required to qualify a CPU-only cost calculator. If the operator authorizes a future end-to-end chip window, first pin both drafts and their dependencies, run the host gates above, then use the existing workload's real observed measurements to exercise predict/observe/decide and compare its selection to the same serialized model on the host. Do not infer GPU availability from a lock file. Do not change ports, drivers or gates. Record chip and host verdicts separately; no chip/live-RM/VFIO qualification is claimed here.
