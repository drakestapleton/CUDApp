<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# MathLibs gaps: Philox distributions, any-length FFT, cufftPlanMany layouts, unlocked plan walk

Date: 2026-11-06 (UTC). Branch `feat/mathlibs-gaps`, stacked on `feat/mathlibs-rfft-2d-philox` (PR #303, `10df33b`).
Receipts were produced from the tree committed as `199f3e9` (Philox distributions) and `f2ef5ff` (FFT, layouts, lock fix);
the documentation commit `4299a5f` changes no code.

Status: host tier PASS, simulator tier PASS, chip tier NOT_RUN (no GPU window is authorized for this lane).
The ROADMAP row "cuFFT, cuRAND" stays PARTIAL.

## What moved to Omega

`om/mathlibs/mathlibs_ir.om` (seeded unit, converted to `src/cudapp_om_mathlibs_ir.c` by `om/units.sh`) now also builds:

- the Philox distribution kernels: cuRAND's `curand_uniform`, `curand_uniform_double`, `curand_uniform2_double` and
  `curand_normal` (Box-Muller with a polynomial log1p, MUFU.RSQ plus one Newton step, Cody-Waite sin/cos), any offset;
- kind 7: a Stockham pass of radix 2, 3, 4, 5 or 7 (constants in `fn rcv`, section 1 of the Wolfram file);
- kind 8: the Bluestein steps (chirp pre-multiply into a power-of-two buffer M >= 2n - 1, pointwise product with the
  transformed chirp, post-multiply with 1/M);
- kind 9: the strided copy that reads and writes the cufftPlanMany layouts (stride, dist, embed; conjugate for the
  inverse; the in-place R2C/C2R row padding of 2 (n / 2 + 1) floats);
- kind 10 and 11: the twiddle and chirp tables, evaluated in f64 on a quadrant fold and rounded once;
- kind 12: the factor plan (radix list or Bluestein) per axis; kind 13: the magic-number division the copies use.

The C in `src/omega_gpu_mathlibs_api.c` validates layouts, stages buffers, scans for poison and launches what the unit
emits. `omega_gpu_fft_plan_walk` and `omega_gpu_fft_many_walk` now build the plan under the session lock and call the
callback after releasing it.

## Toolchain

- omc: `omega-mv/seed/out/omc` at omega-mv e5b32567, sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`
- omega-bridge: `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4` (= `om/omega-bridge.lock`)

## Files

Every receipt here comes from one rerun on 2026-11-06 at head `92f9aa4`, each command inside
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 <cmd>` (no NVIDIA device nodes inside,
so a path that opened the device would fail); builds at `-j3`. The rerun replaced the earlier development runs, whose
wrapping was not independently audited; host and sim output is line-for-line identical to them. The pre-fix reentry
probe was linked against a library built from a detached worktree at `199f3e9`.

| File | Command | Result |
|---|---|---|
| `host-run.txt` | `./build/gpu_mathlibs_test --host-only` | rc 0, `mathlibs host: PASS (477 checks)` |
| `sim-receipt.txt` | `./build/gpu_mathlibs_test --sim` | rc 0, `mathlibs sim: PASS (6265 checks)` |
| `negative-control.txt` | `./build/gpu_mathlibs_test --host-only --dist-fixture pd_bad.txt` (one curand_uniform row changed, `3f6169c6` to `3f6169c7`) | rc 1, `FAIL: philox dist 1 reference: 1 of 896 cuRAND rows disagree`, `mathlibs host: FAIL (1 of 477)` |
| `reentry-deadlock.txt` | `./reentry-probe` (`reentry-probe.c`: a plan_walk callback that calls `omega_gpu_fft_twiddles`) | before the fix (`199f3e9`): killed by its 20 s alarm, rc 1; after (`f2ef5ff`): rc 0 in 0.012 s |
| `wolfram-many-tolerances.txt` | radix constants, the curand_normal bound, the mixed-radix and Bluestein tolerances | each a Wolfram Language evaluation |

The simulator tier is CUDA++'s own sm_121 warp simulator, not the chip. `tests/gpu_mathlibs_test.c` also carries the
reentry regression (`host_reentry`, a forked child with an alarm) so `make test` would hang-fail on a regression.

## What the receipts check

- Philox distributions: 4480-row cuRAND 10.4.0 host-oracle fixture (`tests/fixtures/philox_curand_dist_oracle.txt`):
  uniform, uniform_double and uniform2_double bit-exact (896 of 896 rows each), normal within 2^-21 sqrt(-2 log u) on
  896 of 896 rows (worst 1.848 2^-24 s, 743 bit-exact). In the simulator 56 runs per distribution (4 seeds, 7 offsets,
  130 streams from subsequence 0 and 2 from 2^32 - 1): 0 words differ from the host model, and the runs cover all 896
  fixture rows of each distribution with 0 disagreements.
- FFT: C2C (both directions), R2C and C2R for N = 3, 5, 6, 7, 11, 12, 26, 97, 100, 1000 and 4095 (97, 11, 26 and
  4095 by Bluestein) and 2-D/3-D shapes {3,5}, {6,7,12}, {12,97}, {6,10}, {5,4,6}, {12,9}, against an f64 DFT within
  the Wolfram bounds (`tol` column of `sim-receipt.txt`). Every case runs under three warp schedules and the outputs
  must be bit-identical; output and scratch are poisoned, untouched gaps and the input must be unchanged.
- Layouts L1..L7: in-place R2C and C2R with padded rows, embedded strided 2-D, interleaved batches (istride 4,
  idist 1), embedded R2C with ostride 3, strided C2R at N = 97, in-place 3-D C2C.
- Host: factor plans for every length 2..4096 (3848 by Bluestein), the magic division on 8199 divisors, the twiddle
  and chirp tables within half an ulp of the f64 value (67,117,054 and 16,781,310 entries), 18 refused layouts.
- Mutants, each must be caught (`sim many_mutant`, `sim philox_mutantN`): Stockham pass in the wrong direction and
  without twiddles; Bluestein without the pre-chirp and without 1/M; strided copy without the conjugate and without
  the source batch stride; curand_uniform without the +2^-33, the uniform group skipping the previous block's words,
  normal with sine and cosine swapped, uniform2_double without the half step. One case is expected to be missed and
  the receipt says so: copy mutant 2 at batch 1 (`caught=0 expected=0`), because with one batch the dropped term is 0.

## The twiddle-table oracle fix

The first host run failed 84 table checks, all at angles of exactly 3 pi / 2. libm's `cos(2 pi k / n)` gives
-1.07e-15 there because 2 pi k / n is rounded first, while the unit, which folds the angle to a quadrant in integers,
returns exactly 0. The test's reference now does the same integer reduction (`ref_w`: q = 4k / n, r = 4k - q n,
theta = (pi / 2) r / n, then the rotation by q); the slack stays 1e-15. The unit was right; the oracle was not.

## Limits

- No `curand_normal_double`: it needs f64 arithmetic in the IR (CodegenF64 lane, CUDApp PR #312, not merged). The
  fixture carries its 896 rows; the API refuses the distribution.
- C2R is unnormalised (cuFFT convention: C2R(R2C(x)) = prod(n) x) and ignores Im of the DC and Nyquist bins.
- Lengths 2..4096 per axis, rank 1..3; buffers are staged through the host. The chip tier is NOT_RUN.
