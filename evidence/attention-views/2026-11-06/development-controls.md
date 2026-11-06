# Development failures and repairs

These failures are not counted as successful verification. Final commands and results are in the adjacent receipt.

- An earlier full GQA simulator run trapped while copying a 2 MiB host buffer. A bwrap-contained gdb reproduction located the unchanged checked-runtime loop guard. Long data traversals now call bounded Omega helpers; no guard was raised. The original seven API host/sim gates subsequently completed successfully in `build/attn-views-unrestricted3` (job bg_1381, rc 0, final line `PASS attention typed-view host gate`). This earlier run is not substituted for the final regression receipt.
- The first nested RoPE/touch traversal revisions failed generation with `omc: ... error 12` (seed register pressure). RoPE moved the row loop into a private helper. Touch uses a caller pointer window rather than a nested traversal over three unsized views. Generation failure is not a killed mutant.
- A new long-table fixture found an unconditional touch reconstruction changing an existing touched word. The device-hidden `sh tests/attn_views_run.sh --out build/attn-views-repro-final` returned 1; its differential child returned 9 and printed `result[28]: 1 != 18446744069414584321`. The fix passes the prior slot count to the Omega touch pass and writes only newly allocated slots. The regression covers both previously allocated and new slots over 1,000,003 IDs, including the tail, and preserves the old touched bytes for existing slots.
- Empty pointer-window dispatch now passes the original pointer at offset zero instead of performing pointer arithmetic on NULL. Empty stage and table fixtures cover this path.
- An early bf16 benchmark had its source/destination arguments reversed. That run's timing table is excluded. The committed benchmark uses the corrected argument order; only the final raw samples are reported.

All test/simulator/mutant/benchmark/debugger processes above used `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`. Generation and ordinary compiler diagnostics do not constitute behavior evidence. Hardware remains NOT_RUN.
