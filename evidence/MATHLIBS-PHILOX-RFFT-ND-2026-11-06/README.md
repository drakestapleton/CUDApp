# MathLibs: Philox4x32-10, real FFT (R2C/C2R) and 2-D/3-D FFT, host and simulator tiers

Date: 2026-11-06 (UTC). Branch `feat/mathlibs-rfft-2d-philox`, base origin/main `b774723`.
Receipts below were produced at source commit `8258f94` (the library and test code are those of `30eb499`;
`8258f94` changes documentation and the API version macro only).

Status: host tier PASS, simulator tier PASS, chip tier NOT_RUN (no GPU window is authorized for this lane).
The ROADMAP row "cuFFT, cuRAND" stays PARTIAL.

## What moved to Omega

`om/mathlibs/mathlibs_ir.om` (seeded unit, converted to `src/cudapp_om_mathlibs_ir.c` by `om/units.sh`) now builds:

- kind 3: the Philox4x32-10 kernel (cuRAND `curandStatePhilox4_32_10_t` stream: key = seed, counter.zw = subsequence,
  counter.xy = offset/4);
- kind 4: the real-FFT split (R2C) and merge (C2R) kernels for N = 8..4096;
- kind 5: the batched transpose used between axes of a 2-D/3-D transform;
- kind 6: the pass plan (one word per launch: kernel kind, levels, source and destination buffer) that the C shell walks;
- kind 2: the twiddle table, extended to 4096 points.

The C in `src/omega_gpu_mathlibs_api.c` checks arguments, stages buffers and launches what the unit emits.

## Toolchain

- omc: `omega-mv/seed/out/omc` at omega-mv e5b32567, sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`
- omega-bridge: `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4` (= `om/omega-bridge.lock`)

## Files

| File | Command | Result |
|---|---|---|
| `host-run.txt` | `./build/gpu_mathlibs_test --host-only` | rc 0, `mathlibs host: PASS (337 checks)` |
| `sim-receipt.txt` | `./build/gpu_mathlibs_test --sim --out sim-receipt.txt` | rc 0, stdout `mathlibs sim: PASS (1958 checks)` |
| `negative-control.txt` | `./build/gpu_mathlibs_test --host-only --philox-fixture ph_bad.txt` (one cuRAND row word changed, `6627e8d5` to `6627e8d0`) | rc 1, `FAIL: philox reference: 1 of 1024 cuRAND fixture rows differ`, `mathlibs host: FAIL (1 of 337)` |
| `philox-kat-source.txt` | Random123 known-answer vectors and the cuRAND oracle provenance | |
| `wolfram-constants-and-tolerances.txt` | Philox constants and the FFT error bounds, each a Wolfram Language evaluation | |

All runs used `taskset -c 10-14 nice -n 10`. The simulator tier is CUDA++'s own sm_121 warp simulator, not the chip.

## What the simulator receipt checks

- Philox: the 3 Random123 vectors (0 mismatches; the 9-round control differs on all 3), the 1024-row cuRAND 10.4.0
  host-oracle fixture (0 mismatches), 32 seed/subsequence/offset streams against the host reference (0 mismatches).
- R2C and C2R for N = 8..4096 against an f64 DFT, within the Wolfram bounds in `wolfram-constants-and-tolerances.txt`.
- 2-D and 3-D transforms (forward and inverse, batched) against an f64 DFT within the per-axis product bound.
- Generated code: every twiddle within half an ulp of the f64 value (16376 entries); the five pass plans equal the
  host model word for word.
- Mutants: Philox with the k1 round key bumped by W0 instead of W1, and with 9 rounds (1040 of 1040 words differ
  each); R2C/C2R without twiddle and without the n/2 store; transpose as identity and without the batch high term;
  the complex FFT pass run in the wrong direction inside a 3-D transform. Each is caught.
  One case is expected to be missed and the receipt says so: transpose mutant 2 at batch 1 (`caught=0 expected=0`),
  because with one batch the dropped term is zero.

## Limits

- Philox returns raw 32-bit words only (no uniform or normal output); offset and per-stream count must be multiples of 4.
- C2R is unnormalized and requires Hermitian input (Im X[0] = Im X[N/2] = 0); the kernel does not clear those words.
- R2C/C2R N = 8..4096; 2-D/3-D dims 4..2048, batch times total points at most 2^27; powers of two only; no strided
  layouts and no in-place transforms; buffers are staged through the host.
- The cuRAND oracle ran on the host from toolkit headers; no cuRAND library and no GPU were used. The chip tier is NOT_RUN.
