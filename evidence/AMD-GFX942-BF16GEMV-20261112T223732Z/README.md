# BF16 GEMV host acceptance receipt

Intent (Main's assignment): implement gfx942 row-major little-endian BF16 GEMV for Qwen weights, preserving FP32 x and ascending-k single-rounded FMA, with host-checkable instruction semantics and independent reference outputs.

Candidate branch: amdserve/gfx942-bf16gemv. The commit containing this receipt is the candidate revision; `base.commit` records its parent. No push, merge, hardware access, services, installation or deployment occurred.

## Command and observed exits

Working directory: `/home/drakestapleton/work/hive/amdserve/Gfx942Exec/bf16gemv`.
The only acceptance run (first attempt) was:

```sh
flock /home/drakestapleton/handoffs/hive/queen/pool-14.lock nice -n 10 sh -c 'AMD_OUT=/home/drakestapleton/work/hive/amdserve/Gfx942Exec/bf16-host-out-1 sh tests/amd/run.sh /home/drakestapleton/work/hive/amdserve/AmdGfx942/pin/omc > ../bf16-host-1.log 2>&1; h=$?; echo host_exit=$h; AMD_EXEC_OUT=/home/drakestapleton/work/hive/amdserve/Gfx942Exec/bf16-exec-out-1 sh tests/amd-exec/run.sh /home/drakestapleton/work/hive/amdserve/AmdGfx942/pin/omc > ../bf16-exec-1.log 2>&1; e=$?; echo exec_exit=$e; cat ../bf16-host-1.log ../bf16-exec-1.log; test "$h" -eq 0 && test "$e" -eq 0'
```

Observed `host_exit=0`, `exec_exit=0`, combined command exit0. `gate-summary.txt` is extracted from those logs, not a separate check. `host-last15.txt` and `exec-last15.txt` preserve their last15 lines. `compile/` preserves compiler stderr; no compiler or assembler error was encountered in this slice. No gate was retried or weakened.

## Design and acceptance boundary

- BF16-to-FP32 is exact bit widening: unsigned16 shifted left16. Initialize the weight VGPR to zero once, then use GLOBAL_LOAD_SHORT_D16_HI on each iteration. Its low16 preservation avoids a per-weight shift; this is an instruction-count choice, not measured GPU performance.
- The only new executor opcode is that high-half load. Exactly two little-endian bytes are read; destination low16, flags and inactive lanes are preserved. Manual-derived single-step cases include special BF16 encodings, lane63, negative offset, final-halfword and short-memory boundaries, refused alternate forms and a low-half-clearing mutant.
- Whole-memory comparison checks outputs and guards for the required Cartesian M/K table, finite/special inputs, isolated special-weight classes, a two-byte-aligned/non-dword-aligned W base and overdispatch. Expected values use exact Fraction BF16 interpretation and rational nearest-even FMA.
- Executable controls swap the weight's byte pair and truncate x to BF16 before FMA. Both run successfully and are caught by output-bit mismatches. All older controls remain; the summary derives counts from case tables.
- Two producer builds, two executor builds and two emissions of every kernel are byte-identical. Previously accepted vadd, fsum16, corrected SGEMM and FP8 GEMV object digests are checked unchanged. Complete BF16 instruction/metadata/descriptor comparison against LLVM18 passes, with ET_REL entry displacement handled separately.

PROVENANCE section14 cites every kernel instruction's CDNA3 manual page and LLVM18.1.3 encoding source. The exact manual file digest is in `authorities.sha256`. `inputs.sha256` records gate inputs, and `artifacts.sha256` records retained build results. Python3.12.3 and LLVM18.1.3 are independent test oracles, not production dependencies.

IMPLEMENTED / EXERCISED on host. Hardware qualification, AMD dispatch, ROCr loading, NaN payload parity, timing/throughput, Qwen end-to-end inference, MERGED and DEPLOYED: **NOT_RUN**. No child agent or running process remains. Parent Main owns independent review and integration.
