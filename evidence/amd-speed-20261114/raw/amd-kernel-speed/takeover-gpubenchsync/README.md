# Taken-over fp8gemv_b128f candidates (GpuBenchSync), CPU gate rerun by uxl-brief/AmdKernelSpeed, 2026-11-14

- gate-b1.log / gate-b2.log: GpuBenchSync's kernel-gate (its copy) rerun on the Spark, pool-15, nice 10.
  - v2b1 00686ea5…: FP8GEMV-B128F-V2-GATE PASS, cases=39, mutants=5.
  - v2b2 21f4265d…: PASS, cases=39, mutants=10.
- acc-b1.log / acc-b2.log: the repository's tests/amd-exec/fp8block_fast.py on real checkpoint rows. PASS, cases=27,
  controls=8, for both.
- rungate.out, acc.out: driver output. MANIFEST.sha256: copy of the source tree as taken over.
- v2b1.isa, v2b2.isa: the gated tables.
- waitcheck-v2b*.log: this lane's waitcheck.py (linear scan; refuses loops and forward branches that skip memory
  operations) on both tables.
  - It reports FAIL only for branch structure: 4 (v2b1) and 5 (v2b2) backward branches, plus forward skips.
  - It reports no read-before-load, no outstanding-count and no VGPR-bound error, but its scan is not sound for
    loops.
  - There is no loop-aware wait checker. The wait counts of these two tables are therefore checked only by the
    executor's byte identity (which completes loads at once, so it cannot see waits) and by the card. On the card,
    3 runs of 256 tokens per arm, every run printed the published ids.
