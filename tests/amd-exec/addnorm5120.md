# Residual plus RMSNorm fusion

Intent: replace the existing residual5120/rmsnorm5120 pair with one launch while preserving both the residual write and every arithmetic rounding point.

`addnorm5120.isa` is a new seeded producer input. Existing kernels are unchanged. The new ABI has five global pointers, in order: `h`, `delta`, `weight`, `h_new`, `y`. All point to a hidden-width FP32 vector; buffers are disjoint. Launch dimensions, offsets, hashes and measured extents are generated in the acceptance report and notebook bundle list.

For each element, the first pass rounds both inputs to BF16, adds them in FP32, rounds the sum to BF16, writes the widened residual, and accumulates its square with the same lane assignment and FMA order as the original norm. BF16 rounding is nearest-even with canonical NaN. The XOR reduction order, rounded mean constant, epsilon, reciprocal-square-root instruction and separate output multiplies are unchanged. The norm weight is `1 + weight`, not `weight`. A wait drains residual stores before the second pass reloads them. Saved pointers and the reduction accumulator do not overlap BF16-rounding scratch registers.

The fused kernel is a single wave. More workgroups would require a different synchronization/reduction design and are outside this exact fusion. Fewer launches is not itself a measured speedup: the card comparison must determine whether the additional arithmetic in one wave pays off.

## Acceptance

Under a free queen pool lock, with `nice`, run:

```
python tests/amd-exec/addnorm5120.py --omc /path/to/pinned/omc --out /new/private/output
```

The instrument snapshots all source dependencies, checks deterministic seeded compilation/object production and LLVM encoding/metadata/descriptor agreement, and compares every output, input and guard byte against the existing two-kernel sequence. Cases include random inputs, zeros, cancellation, tiny values, overflow, subnormals and exceptional IEEE values. Both grid dimensions are overdispatched.

The residual has an independent NumPy bit oracle. Finite norm cases use the existing Fraction-based instruction-boundary oracle and the documented reciprocal-square-root interval. Overflow and exceptional norm outputs must still match the original sequence bit-for-bit.

Controls deliberately omit the residual write, normalize with the pre-add input, or change epsilon. They must produce a numerical mismatch while remaining in bounds. Each buffer also has a one-byte-short negative control. Commands, exits, pins and bounds are recorded in the private output directory. Hardware and performance remain NOT_RUN until a card log is available.

The separate `notebook-probe/v12/addnorm` cell times graph replay of fused launches versus back-to-back original pairs. Torch on identical data is labelled REFERENCE. Keeping this cell separate leaves the already qualified state-cell wrapper unchanged.
