# Wide-load block-scaled FP8 GEMV

`fp8gemv_b128f` is an additional kernel; the existing kernels are unchanged.
The production input is `backend/amd/gfx942/fp8gemv_b128f.isa`, consumed by
seeded Omega `hsaco.om`. The adjacent LLVM assembly is an independent encoding
oracle, not a production dependency. The ABI remains that of `fp8gemv_b128`.

Each lane owns four consecutive columns in a 256-column chunk. The main loop
prefetches four W dwords and four x dwordx4 vectors per lane before waiting.
The two half-waves use their respective 128-column BF16 scales; each fetched
scale is reused for the lane's four products. Identical scale addresses within
a half-wave are coalesced, not claimed to be a single scalar memory instruction.
A lane computes a four-product partial, scales it into its running accumulator,
and performs one XOR32/16/8/4/2/1 reduction after all columns. Scalar predicated
loads handle the residual chunk without reading past K or the scale grid.
BF16 d16-hi destinations are zero-initialized: correctness does not depend on
whether hardware preserves or clears the unused register half.

## Numerical contract

Moving the reduction outside the K loop changes floating-point summation order.
Bit identity with `fp8gemv_b128` is therefore **not** the contract for this fast
variant. The acceptance report records exact baseline/fast output-bit witnesses,
including real checkpoint rows. No tolerance was increased: the notebook uses
v11's unchanged `oracle`, `gamma`, and `mismatches` functions for both kernels.
The host acceptance evaluates the same bound against an exact rational sum:

```
n = ceil(K/128) + 8
gamma(n,u) = n*u / (1-n*u)
bound = (gamma(n,2^-24) + gamma(2*K+n,2^-53))*sum(abs(W*s*x)) + 2^-126
```

The separate exact Fraction oracle specifies every FMA and reduction rounding
in the fast kernel, and checks every byte of the resulting memory image.
This is tested numerical evidence, not a proof for arbitrary overflowing inputs
or a claim of model-level logit/token parity. OCP E4M3 decoding includes both
NaNs and signed zero; native gfx942 FNUZ is not used as the reference format.

## CPU acceptance

Run from the repository root under an externally held free queen CPU-pool lock with `nice` and CPU affinity. Set `OMEGA_OMC`, `AMD_WEIGHTS` and `AMD_DOC_OUT` as in `fp8block.md`; run that document first to produce the baseline bundle. Use `python3` on the caller's PATH. Missing dependencies are NOT_RUN.

```sh
python3 tests/amd-exec/fp8block_fast.py \
  --omc "$OMEGA_OMC" \
  --weights "$AMD_WEIGHTS" \
  --out "$AMD_DOC_OUT/fp8block_fast"
```

The run checks the compiler pin, deterministic seeded producer/executor and
objects, LLVM text/metadata/descriptor parity, all FP8 bytes, zero dimensions,
partial and wide-load boundary cases, and sampled checkpoint rows at every
requested real shape. Bounds mode records each buffer's last access. Rebuilt
per-row-scale, transposed-scale and dropped-tail controls must fail both exact
comparison and the unchanged v11 tolerance; shortened-span controls must exit 68.
Commands, source snapshots, checkpoint hashes and results are retained in the
new run directory. Failed run directories must not be reused or overwritten.

After committing the tested sources, create a fresh notebook bundle under the
same CPU-lock discipline:

```sh
python3 tests/amd-exec/fp8block_fast_bundle.py \
  "$AMD_DOC_OUT/fp8block_fast" "$AMD_DOC_OUT/fp8block-fast-bundle" "$AMD_DOC_OUT/fp8block-bundle"
```

The bundle checks committed source hashes and retains the baseline's exact
source head, object, table and acceptance evidence. See
[`fp8block_fast_notebook/README.md`](fp8block_fast_notebook/README.md) for the
notebook build and simulated-device acceptance. Only the relay can provide
hardware correctness or timing evidence.
