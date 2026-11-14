# Per-kernel timing of the real Qwen3.8-27B plan on the MI300X, 2026-11-13

State: EXERCISED on hardware once. Unpublished, unreviewed, not landed. Measuring tools are the
uxl-brief/AmdKernelSpeed lane's Python and shell (`../../measure/`); the launcher and kernels are the
unmodified CUDApp dc536882 objects (launcher sha256 08a50491...).

- Session `jupyter-user-261113232737-7605956f`, AMD One-Click "Qwen/Qwen3.8-27B on AMD Instinct MI300X",
  launched 2026-11-13T23:27:37Z. Driven by Developer/GpuBenchSync (Drake: "ok I signed in, take it over from
  the other session and run it") from an omp-managed Chromium through the Jupyter kernel websocket and the
  contents API. No cell paste, no Chrome window.
- Bundle as built by the lane: cell.py sha256 5008ca46..., cell2.py sha256 07a475a7....
- attempt1/, attempt2/: every launcher run exited 74 errno=2 in under a second. Cause: card-measure.sh passed the
  plan.cmd arguments without shell expansion, so the launcher was given the literal `$AMDSERVE_WEIGHTS/...`.
  attempt2 was my first, wrong fix. The run that counts used `card-measure-fixed.sh` (sha256 22999145..., one
  changed line, the `args=` line). The lane's files on the Mac are unchanged.
- Result run: 35 launcher runs, all exit 0 (23:38:14Z to 23:51Z). `aks-results.tar.gz` sha256 d6502d89...
  equals the value computed on the card; `tar/out/SHA256SUMS` checks; the tarball's run files equal the ones
  mirrored during the run.

## Result

`out.json` is the lane's `analyze.py` (sha256 a74d08b3...) on `tar/out/runs`. It prints ANALYZE FAIL for
full-a, full-b, cap286 and v1all because it compares the 256-token runs with the Spark fixture
(`lighthouse256.json`), from which the AMD result is known to differ at generated index 109. Against the
published AMD ids (`../../../card-evidence/uxl-native-run1.txt`) all five 256-token runs are equal on all 256
ids. Every 64-token variant printed the first 64 published ids; the altered-weights control differs at index 0.

Per-dispatch times are in `out.json` (`classes.*.us_per_dispatch`). Per-token totals (count in the plan x time
per dispatch), 256-token medians and group sums are in `wolfram-totals.txt` and below
(WolframLanguageEvaluator; inputs in that file).

- 64-token base decode median 26,309 us (three base runs, spread +106 / 0 / -42 us).
- Sum over the 24 timed classes: 23.2227 ms of the 26.309 ms step; 3.0863 ms is not attributed.
- 256-token decode medians, us: full-a 26,498, full-b 26,383, cap286 26,069, v2rms 24,925, v1all 24,581.

## Limits

- One session, one card, one request. Host wall clock per step, not GPU time.
- Inserted dispatches run at the end of the step, after argmax (lane DESIGN.md): small kernels may read faster
  than in place. `step_advance` is not timed.
- No AMD-library or HIP comparison was run in this session (it ended before there was time).
- v2rms and v1all use the lane's rmsnorm5120 v2 object; that kernel is the lane's unreviewed candidate.
