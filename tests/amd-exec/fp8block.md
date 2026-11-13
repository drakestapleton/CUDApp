# Checkpoint block-scaled FP8 GEMV

`backend/amd/gfx942/fp8gemv_b128.isa` is a declarative kernel table consumed by
seeded Omega `hsaco.om`, following the existing `fp8gemv2` convention. LLVM
assembly is an independent encoding oracle; it is not a production input.
Existing kernels and objects are unchanged.

The kernel implements the checkpoint's OCP E4M3 encoding, not native gfx942
FNUZ. It converts the unsigned magnitude through the hardware instruction,
rescales the bias, restores the sign (including negative zero), and explicitly
handles OCP NaNs. The test builds a separate rational decode table from the OCP
sign/exponent/fraction definition and exercises every byte encoding.

A wave owns an output row. Each lane consumes two columns of a block, reduces
within that block using an XOR tree, then accumulates its BF16-scaled block sum.
The row and column block indexes select the checkpoint scale grid in row-major
order. Masked loads protect both halves of the last partial block. All global
addresses include high words and carry propagation. A zero-column row produces
zero without reading inputs; an overdispatched row performs no buffer access.

## Reproduce

Run from the repository root under an externally held free queen CPU-pool lock,
with `nice` and CPU affinity. Set `OMEGA_OMC` to the absolute pinned compiler,
`AMD_WEIGHTS` to the local checkpoint directory with its manifest, and
`AMD_DOC_OUT` to a new private output parent. The harness snapshots its inputs.
Use `python3` on the caller's PATH; missing dependencies are NOT_RUN.

```sh
python3 tests/amd-exec/fp8block.py \
  --omc "$OMEGA_OMC" \
  --weights "$AMD_WEIGHTS" \
  --out "$AMD_DOC_OUT/fp8block"
```

The harness refuses an unpinned compiler before making an output directory.
It records every executed command and exit, deterministic producer/executor
builds, object hashes, LLVM section comparisons, checkpoint shard and sampled
row hashes, exact-oracle cases, and access bounds. Checkpoint shards are hashed
against the supplied local manifest and rows are read individually; no model
is held in memory.

Real-shape cases preserve full dimensions, physical strides, and allocation
sizes, but execute sampled rows including both sides of a row-block boundary
and the last row. They are not exhaustive executions of every output row.
Small cases exhaustively exercise their outputs, including all FP8 encodings,
empty dimensions, column-half boundaries, and partial blocks. The entire memory
image is compared, including untouched rows and padding.

The exact Fraction oracle specifies this kernel's reduction order. It does not
claim bit-for-bit agreement with the Rust reference's serial dot-product order,
nor does this kernel-level gate prove model logits or generated tokens.

Numerical mutants deliberately use per-row scales, transpose the scale-grid
indexes, or drop the partial block. Each must execute successfully and produce
a wrong answer. Instrumented Omega executors measure the last byte read and
written in each buffer, refusing cross-buffer accesses. Independent shortened
buffer controls must refuse the measured last access. `report.json` is the
source of truth for measurements and results; `commands.json` records actual
process exits. No device dispatch or performance claim is made.

## Notebook bundle

After the source and acceptance evidence have been committed, generate the
bundle from that exact evidence; do not maintain copied ABI lists by hand:

```sh
python3 tests/amd-exec/fp8block_bundle.py "$AMD_DOC_OUT/fp8block" "$AMD_DOC_OUT/fp8block-bundle"
```

This derives `list.json`, `list.md`, and `SHA256SUMS` from the tested table and
report, includes the correct object and deliberately wrong controls, and copies
the compiler/oracle source snapshots and command records. The executable ABI
is in the table's `K` and `A` records; generated lists are the notebook-facing
view. Hardware qualification and performance remain separate gates.
