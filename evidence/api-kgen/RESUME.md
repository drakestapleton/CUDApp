# API kernel generator integration checkpoint

This is an active-work checkpoint, not a completion receipt.

- Worktree: `lanes/KernGenApiOm`, branch `feat/api-kernel-gen-om`.
- Draft PR: https://github.com/aien-dev/CUDApp/pull/340, base `feat/api-views-attn-om`.
- Published registration head at this checkpoint: `20b33b52`.
- Verified base: `c1fec738fb3dfe4fd48bf9115a19ca9d4d3d6367`.
- Prerequisite merge commits: `7113117` (#309 replay), `bac386c` (#294 scan).
- EW registration: `73b11c57`; EW cutover: `c331928f`.
- Attention source registration: `f965e67a`, export-order repair `a61802d5`, shared native registration `20b33b52`.

## Proven host evidence

`elementwise-differential.log`: complete generator-spec parity, solved allocation, text and cubin bytes; 127 accepted / 73 refused; shape-plan partition 4189 admitted / 4001 refused; red killed; five regenerated seeded semantic mutants killed.

`simulator-targets.log`: `test-hello-sim`, `test-gpu-gemm-ex-sim`, `test-gpu-engine-sim`, exit 0.

`decode-prefill-targets.log`: `test-gpu-decode-attn` (includes maxpos controls), `test-gpu-decode-attn256`, `test-gpu-prefill-host`, exit 0. Seven of seven maxpos controls killed. Prefill's eight binaries include attention host and sim.

Production-default EW and reconvergence targets passed: 24 checks / 0 failures, 119 checks / 0 failures. Full unit generation and crumb verification passed after attention registration.

All test commands use the mandatory bwrap private `/dev`, CPU 10–14 and nice 10 wrapper. No GPU qualification or deployment occurred.

## Ownership and acceptance state

Main's parent-managed `KernGenAttnOm` owns attention `.om` sources, attention C cutover and attention fixtures. It was instructed to merge `20b33b52` before its cutover. Its initial differential exposed a general-case metadata mismatch; that repair and final attention handoff remain in its active lane. This lane owns shared unit generation, mk integration, aggregate evidence and the single PR. Attention has not yet been cut over in this checkpoint.

Final combined attention differential/mutants, default attention target, final units `--check`, crumb verification and final PR receipt are not completion claims here. Chip remains NOT_RUN and requires a fresh explicitly authorized exclusive window. No merge authority exists.
