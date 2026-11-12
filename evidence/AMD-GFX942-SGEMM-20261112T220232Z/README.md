# gfx942 SGEMM: host-exercised, not hardware-qualified

Intent: build Omega capable of serving Qwen 27B on AMD GPUs; this slice supplies a checkable row-major SGEMM code object and exact host instruction execution. It is not a serving implementation or an AMD hardware result.

## Scope and evidence

`base.commit` records the starting revision. `inputs.sha256` pins the implementation, independent assembly and test oracles. `SHA256SUMS` covers this receipt. `host-gate.log` and `exec-gate.log` are the final successful runs; `exit-codes.txt` records their exits. Compiler identity and oracle versions are in those logs. Production implementation is seeded Omega; LLVM and Python are test-only instruments.

Final commands, from this worktree, in execution order:

```sh
flock "$HOME/handoffs/hive/queen/pool-14.lock" nice -n 10 \
  env AMD_EXEC_OUT="$HOME/work/hive/amdserve/Gfx942Exec/sgemm-exec-out-8" \
  sh tests/amd-exec/run.sh "$HOME/work/hive/amdserve/AmdGfx942/pin/omc"
flock "$HOME/handoffs/hive/queen/pool-14.lock" nice -n 10 \
  env AMD_OUT="$HOME/work/hive/amdserve/Gfx942Exec/sgemm-host-out-4" \
  sh tests/amd/run.sh "$HOME/work/hive/amdserve/AmdGfx942/pin/omc"
```

The actual wrapper redirected each command's stdout/stderr to the corresponding external log and printed its exit after reading that log. Paths in copied logs are normalized from the local home prefix to `$HOME`; all other bytes are retained. Raw external logs remain alongside the private worktree.

## Execution design

- `exec RAW_TEXT MEMORY GRID_X GRID_Y` consumes instruction words, not table records. Work-item extents map to wave64 groups; s0:s1 is the kernarg pointer, s2/s3 the X/Y group IDs and v0 the lane. SGPR/VGPR, EXEC, VCC and SCC are explicit state.
- Decode validates every word, including unreachable words, before execution; branch destinations must be instruction boundaries. Unsupported instruction/operand forms refuse with exit 66; malformed/truncated code or invalid branches use 65; memory-range violations use 67. Existing CLI/I/O/resource refusals remain documented in PROVENANCE.md.
- SGEMM maps each lane to one column and each Y workgroup to one row. It masks row/column tails, traverses K in increasing order, and writes +0 for K=0. Full-width address arithmetic uses scalar/vector carries. Signed SOPP offsets are checked before object emission; label storage grows with input.
- `v_fmac_f32` uses an exact signed integer accumulator followed by one RN-even binary32 rounding, preserving denormals. `fma-width.txt` records the format-derived limb calculation. NaNs use the explicit canonical-NaN model policy, not an assertion about MI300 payload propagation.
- Independent LLVM gfx942 assembly supplies instruction/metadata/descriptor bytes; Python Fraction supplies exact FMA results. Test fixtures compare the complete memory image, including input and guard regions, and deliberately overdispatch both axes.

The host gate checks the `v_fmac_f32` encoding and descriptor workgroup-Y enable against LLVM, including the actual rsrc2 bit. It also exercises both signed branch limits and rejects overflow, missing and duplicate labels. The executor gate retains all vadd/fsum16 cases, exercises finite and special-value SGEMM shapes plus a fused-sensitive case, and catches separately-rounded multiplication/addition and an illegal constant-bus operand. The final summaries derive SGEMM shape/case counts from the oracle cases rather than a separately maintained count. Both old kernel objects remain byte-identical to the earlier receipts, and both producer/executor builds are reproducible.

## Failures encountered and repairs

Earlier attempt logs and every nonempty compiler diagnostic are retained under `attempts/`; they are not PASS evidence.

- Writer `labword at ...`: compiler error 16 at line 75. `at` is reserved; renamed the parameter `off`.
- Composed executor line 134: compiler error 3. `mod` is not an Omega word; the nonnegative modulo-64 operation now uses a six-bit mask.
- Composed executor line 153: compiler error 16. A local `carry` collided with the existing `carry` function in the shared namespace; renamed it `cy`.
- Composed executor line 160: compiler error 9. Boolean-to-ulong casts are refused by this pin; explicit `fmbool` returns integer zero or one.
- Composed executor line 539: compiler error 12. Combined pair-register validation exhausted the register pool; split it into two checks.
- Composed executor line 680: compiler error 12. Repeated nested global-state reads exhausted the register pool; cache the opcode and SCC in locals before branch predicates.
- Standalone FMA probe line 22: compiler error 6. Concatenating the FMA unit without the `om2` header made hexadecimal literals invalid; the probe composition now starts with that header.
- LLVM refused `v_addc_co_u32 v11, vcc, s19, v11, vcc`: the scalar source plus VCC violates the gfx942 constant-bus restriction. Move the scalar high stride to v7 and use that vector operand; the executor rejects the illegal form and its control verifies exit 66 without writes.
- Host attempt 3 exited zero but its SGEMM object digest differed from the executor run. Byte comparison showed only the ELF section-table pointer changed in the original prefix, followed by appended bytes. The new SGEMM extraction had omitted llvm-objcopy's explicit output path and therefore rewritten its input. Use the existing throwaway-output convention and compare the original again after all oracle operations. Final host attempt 4 has the same original SGEMM digest as executor attempt 8. The earlier digest is not adopted as producer evidence.

## Limits and status

IMPLEMENTED and EXERCISED on the host. Hardware execution, ROCr loading, performance, MI300 NaN-payload behavior, and comparison against Spark inference are NOT_RUN here. Linked-oracle whole-object parity is NOT_RUN because this host has no ld.lld; section and descriptor parity are exercised instead. This kernel is correctness-first, not a tiled or performance-qualified implementation. No merge, publication, deployment, GPU access or service change was performed.
