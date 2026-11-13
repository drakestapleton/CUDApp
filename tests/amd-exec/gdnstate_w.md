# Partitioned GDN state update

Intent: expose independent value columns to more workgroups without changing the existing `gdnstate` arithmetic, ABI, state layout, or BF16 core-output rounding.

`backend/amd/gfx942/gdnstate_w.isa` is the seeded Omega producer input. The adjacent assembly is an independent LLVM encoding oracle, not the production object source. Existing objects and sources are unchanged.

The X coordinate selects a value head within its key-head group and a block of eight value columns. Eight lanes operate on one column each. Y selects the key head. Each column retains the original serial row order, separately rounded FP32 multiplication and addition, intermediate decayed-state store/reload, and final BF16 round-to-nearest-even with canonical NaN. Columns do not communicate. Inactive lanes remain inactive through the inner NaN mask. Both grid dimensions reject overdispatch.

The change deliberately trades inactive lanes and repeated query/key reads for more independent workgroups. It does not establish a speedup; only the card comparison can do that. The generated acceptance report and bundle list contain the launch dimensions, descriptor, object hashes, and measured buffer accesses.

## Host acceptance

Run from the repository root under an externally held free queen CPU-pool lock with `nice` and CPU affinity. Set `OMEGA_OMC` to the absolute pinned compiler path and `AMD_DOC_OUT` to a new private output parent. The documented interpreter is `python3` on the caller's PATH; it must provide NumPy. Missing dependencies are NOT_RUN, never a pass using a substituted interpreter.

```sh
python3 tests/amd-exec/gdnstate_w.py --omc "$OMEGA_OMC" --out "$AMD_DOC_OUT/gdnstate_w"
```

NumPy is an outside verification dependency. The instrument snapshots its inputs before compiling. It checks deterministic seeded producer/executor compilation and object production, then compares LLVM instruction bytes, metadata and descriptor fields.

Every random and edge case compares the entire resulting memory image with the original `gdnstate` executor run. This covers all heads, rows, columns, input bytes and guards, not sampled locations. A separate NumPy oracle performs FP32 operations in scalar row order; finite values and infinities are compared bit-for-bit, and NaN classifications are compared independently. The original-versus-partitioned comparison additionally requires identical NaN bits.

The first random fixture measures read and write extents for every buffer. Deliberately shortened spans must fail with executor exit 68. Wrong-column, wrong-key-head and missing-beta mutant objects must execute in bounds and produce a numerical mismatch. Mutants are test artifacts, never production alternatives.

`commands.json` records actual subprocess arguments and exits. `report.json` records source pins, objects, input/output hashes, bounds and negative controls. A failed attempt is retained. Host acceptance is not hardware qualification and does not measure GPU performance.

## Card comparison

The `notebook-probe/v12` bundle is outside notebook glue. Its generated cell compares the two Omega objects on identical input data and times graph replay of independent model-layer state updates. Torch operations on the same input are labelled REFERENCE, never Omega. The bundle's selfcheck uses a simulated device solely to exercise result handling, fault isolation and negative controls. Only a real card log establishes device correctness or timing.
