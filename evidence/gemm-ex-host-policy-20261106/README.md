# GEMM EX host policy migration — 2026-11-06

Base: `feat/gemm-ex-typed-views` at `0e4417ae62ce` (PR #319). Unit registration precedes the consumer cutover in `29e21b9d`. The six census functions `gx_min`, `gx_arow`, `gx_bcol`, `gx_brow`, `gx_pack`, and `gx_conv` now delegate scalar decisions and index/stride/count/byte-offset derivation to `om/gemmex/policy.om`. The C shell retains byte copies and the existing typed row-unit call. Public APIs are unchanged.

The retained policy C is default-OFF behind `CUDAPP_MIGRATION_PENDING_C_GEMM_EX_HOST_POLICY`. It is independent of `CUDAPP_MIGRATION_PENDING_GEMM_EX`: this change does not enable or reintroduce GX_C, the numerical C path. Production policy references the Omega entry strongly and aborts on an impossible scalar-unit fault rather than falling back to hand-written C.

## Reproduction

The converter pin is inherited from this base, not replaced by the other lane's pin:

```sh
export OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc
export OMEGA_BRIDGE=$HOME/work/cudastack/lanes/BridgeViews
export OMEGA_SEED=$HOME/work/cudastack/omega-bridge/build/bootstrap/omc
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make test-gemm-ex-host-policy
```

The compiler pin is `e5b32567`, binary SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. The base's `om/units.sh` checks its existing BridgeViews converter/frontend pins and byte-for-byte generated output. The runner performs that check, builds the archive with `make -j3`, compiles the old policy as an oracle with `-O0 --coverage -ffp-contract=off`, then compiles and runs the same copy-shell harness on the default Omega path. It also runs the existing `test-gpu-gemm-ex` and `test-gpu-gemm-ex-sim` targets, without changing their numerical backend.

The comparator must turn red when its expected result is flipped. A copy of the real production library with `cudapp_om_gemm_ex_policy.o` removed must fail to link the existing public GEMM EX test with `omega_gemm_ex_policy` undefined. Each of eight non-equivalent `.om` mutants must compile and fail the scalar differential comparator. Full subprocess output stays at `build/gemm-ex-policy/check.log`; `receipt.txt` contains the final observed outcomes and first mismatch for each mutant. Raw gcov and the selected function summary are under the same build directory.

`coverage.txt` preserves raw branch counters for all six owned C-oracle functions. There are no untaken branches in those functions; session/device effects outside this policy slice are not part of that claim.

## Partitions and negative paths

- Both transpose choices, every source operand/row/column orientation, minimum ordering/equality, zero, one, boundary neighbors, `UINT32_MAX` and `UINT64_MAX`; unsigned index multiplication/addition wraps exactly like the C oracle.
- Packing covers operations 0..9, source and conversion widths 1/2/4/8, nonzero row lengths around 4096, 16384, 32768 and 65536, and unsigned-32 maximum. Zero-width and zero-row pack divisors are excluded because the validated caller never passes them and the C expression has no defined result there. Huge `UINT64_MAX` pack products are likewise outside the validated pack domain; that value is exercised on min/index/stride operations instead.
- Conversion policy compares contiguous single-row, dense multirow, and padded multirow cases, including exact width equality. The actual shell copies conversion results and leaves padding sentinels untouched, compared against independent row-unit calls. An oversized typed-view row returns GX_FAULT before the strided memcpy shell executes, on both C-policy and Omega-policy builds.
- Mutants alter minimum ordering, transposed index, row stride, excluded plane operation, window quota, contiguous predicate, row byte offset and column stride.
- Floating-point values are not computed by this unit. Existing GEMM EX host/simulator gates retain the source's type/class/numerical checks; the focused copy test uses zero-valued BF16 input to isolate layout and padding from numerical conversion.

WolframLanguageEvaluator: `{32769*2, 32769*2>65536, (2-1)*(2^32-2)+1, (2-1)*(2^32-1)+1}` -> `{65538, True, 4294967295, 4294967296}`. The first pair documents the typed-view refusal boundary; the second pair documents the other lane's exact valid/invalid 32-bit span edge. Other expected scalar values are computed by the retained source C oracle, not by a new host implementation.

## Tiers and chip runbook

IMPLEMENTED: host policy cutover. EXERCISED: host/simulator/differential verdicts are recorded in `receipt.txt`. QUALIFIED on GB10: NOT_RUN. MERGED: no. DEPLOYED: no.

Chip/live-RM/VFIO tiers are NOT_RUN. Only after Drake grants the current exclusive GPU window and the operator satisfies the current device/service/ownership runbook: regenerate at this exact PR head, build `build/gpu_gemm_ex_test`, and run it with no `--host-only`/`--sim` flag. Retain the existing route/type/leading-dimension/batch negative controls and record a new hardware receipt with exact head and device/session evidence. Do not run the chip tier inside the device-hidden host wrapper or interrupt serving without that authorization.

This runtime policy unit does not change language/compiler semantics, bridge capabilities/pins, public APIs, repository ownership or a global gate, so no omega-harness skill example or pin requires updating.
