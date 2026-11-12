# gfx942 SGEMM instruction-semantics correction

Intent: correct the manual-disproved scalar borrow semantics before extending the executor or SGEMM, audit every implemented instruction, and catch the old behaviors with independent single-instruction cases.

## Reproduction

Working directory: `/home/drakestapleton/work/hive/amdserve/Gfx942Exec/sgemm`.

Exact executed commands (each exited zero):

```sh
flock /home/drakestapleton/handoffs/hive/queen/pool-14.lock nice -n 10 sh -c 'AMD_EXEC_OUT=/home/drakestapleton/work/hive/amdserve/Gfx942Exec/sgemm-fix-exec-out-1 sh tests/amd-exec/run.sh /home/drakestapleton/work/hive/amdserve/AmdGfx942/pin/omc > ../sgemm-fix-exec-1.log 2>&1'
flock /home/drakestapleton/handoffs/hive/queen/pool-14.lock nice -n 10 sh -c 'AMD_TEST_OUT=/home/drakestapleton/work/hive/amdserve/Gfx942Exec/sgemm-fix-host-out-1 sh tests/amd/run.sh /home/drakestapleton/work/hive/amdserve/AmdGfx942/pin/omc > ../sgemm-fix-host-1.log 2>&1'
```

The host gate's output variable is `AMD_OUT`, not `AMD_TEST_OUT`; therefore that invocation used its normal temporary output directory. The full captured host log remains here; executor binaries and code objects remain in the explicit executor output directory and have recorded digests. No gate was rerun to hide this harmless recording mistake.

## Evidence and interpretation

- `base.commit`: parent revision, before this correction.
- `inputs.sha256`: exact implementation, gate, case table and provenance inputs.
- `authorities.sha256`: pinned compiler and local CDNA3 manual text.
- `artifacts.sha256`: actual producer, executor and emitted code-object digests.
- `exec-gate.log`: complete execution results, all prior kernel cases, all new manual-derived cases and each old-behavior mutant control.
- `host-gate.log`: complete independent LLVM text, descriptor, group-Y and metadata comparison results, plus producer/encoding controls.
- `exit-codes.txt`: observed gate exit codes.
- `compiler-logs.txt`: generated list of compiler diagnostic-file sizes; all are empty. **No compiler errors were encountered in this correction.**

The manual audit and per-instruction physical PDF page citations are in `PROVENANCE.md` section 12.1. The semantics table computes expected integer results from manual pseudocode and directly states floating-point edge-case bit patterns. The test adapter invokes production `step`; it does not duplicate instruction execution. Each correction has a compiled executable mutant restoring its old behavior, which fails the corresponding independently specified state or refusal expectation. The final PASS derives its case count from the case table. All existing kernel count/shape coverage is retained.

A misplaced mutation-table entry was caught by the edit tool's Python parser before any gate ran and was moved into the correct dictionary. This was a test-file editing error, not a compiler or behavioral failure. Both gates passed on their first execution for this correction.

The earlier SGEMM receipt remains immutable. Its successful self-consistent model and encoding checks did not detect inverted borrow semantics. This receipt supersedes that receipt's semantic inference, not its historical observations.

## Status

IMPLEMENTED and EXERCISED on the host. Two producer/executor builds and each code object are byte-identical. The existing vadd/fsum16 whole-object digests are unchanged; SGEMM intentionally changes its three conditional branch words. NOT_RUN: AMD hardware dispatch/qualification, hardware NaN payload behavior, XNACK and memory ordering, performance, Qwen inference. Not reviewed independently here; not merged, pushed, published or deployed. Parent owns independent review and integration.
