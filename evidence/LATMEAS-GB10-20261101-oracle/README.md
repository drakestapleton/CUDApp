# GB10 latency receipt with the host oracle, 2026-11-01

This is the headline chip receipt for the latency kit. `sm121_vs_NAK.md` is the table `fill_template.py` wrote from this directory's `raw.json`, and a fresh fill of that file matches it. Measured cells come from `stall_required` in `raw.json` and from nowhere else.

## What ran

- Command: `sh tools/latmeas/run_spark.sh latmeas-out-7ec0184` (`run.log`).
- `RECEIPT.txt` is the file the script wrote. It is copied here unchanged. `utc: 2026-11-01T18:47:06Z`. `head: fb91560dc3492082455576477bffd1794d27ab3c`, parents `7ec018409bbc2ca29e51fc88584585fdca3c7231` and `6a93cb567b33b67bc1185fb45a3c50fc4d88d30e` (PR #254 `7ec0184` merged with PR #251, no other edits; `worktree: clean`). Kernel 7.0.0-1019-nvidia, gcc 13.3.0, open module 580.173.02, device `000f:01:00.0`.
- `raw_json_sha256` in the receipt is `aa3fe3fee12ffe123f572e5a0c9b26b7e7fc82142635ea59442b6a008033534a`, the same as `sha256sum raw.json`.
- Launch rc 0, `"error": ""`, 1408 launches, 11 runs. Upload sha256 `872182a84618fc1c16d14b7ce689e012f36e19515aaadc18c146c7f3115d074c`. Two home-directory paths in `run.log` are replaced with `latmeas-out-7ec0184`.

`stall_required` and spread, from `raw.json`: alu_alu 4, fma_fma 4, mixed 5, pred 13, fmnmx_alu 4, imad_wide_lo 3, hmma_reader 20, each with spread 0. At that stall and every larger stall, `lanes_matched_min` is 32, `lane_fail_mask` is `0x00000000`, and `first_mismatch` is null. At the stall under it, `lanes_matched_min` is 0, the mask is `0xffffffff`, and `first_mismatch` is run 0, lane 0, word 2.

## What the file shows about the four words

`first_mismatch` on every failing stall names word 2 and no other word. That field keeps the first difference in a lane (`tests/gpu_latmeas_test.c` `lanes_report`, the loop at lines 1537-1545: `w[2 + k]` against `o[k]` for `k` from 0 to 3, and it records only the first `k` that differs). A word-2 miss therefore does not say whether words 3, 4, and 5 also differed. HMMA's `oracle` is four copies of `1098907648`. This file has no row whose first difference is word 3, 4, or 5.

A passing stall is different. `lanes_matched_min` counts a lane only when all four of those words equal the oracle (the same loop, `if (!bad) n++` at line 1547). 32 means every lane matched words 2, 3, 4, and 5. For HMMA that is `d0`..`d3`. That is what 32 at stall 20 and above shows, together with this code. It is not a separate per-word tally.

The host self-test does plant a miss that is only word 4: `tests/gpu_latmeas_test.c` lines 1648-1650 set lane 7's word 4 and require `lanes_report` to name lane 7 word 4. That test passed on the host (`make test-gpu-latmeas`). It is not a chip result. `word_mismatch_max` (four counts, words 2..5, the maximum across runs) is written by a later commit and is not in this `raw.json`.

## model: Unknown

`RECEIPT.txt` says `model: Unknown`. The script copied the proc file's `Model` line. On this machine that line is `Unknown`. The doctor, in `run.log`, names the same PCI id from the chip table: `GPU 000f:01:00.0: NVIDIA GB10 (sm_121)`. The receipt file is the one the script produced and is not rewritten. A later `run_spark.sh` fills `NVIDIA GB10 (sm_121)` from PCI `10de:2e12` when proc `Model` is `Unknown` (`src/omega_gpu_chip.c`).

## One second and the 71 second bound

`print_plan` in `tests/gpu_latmeas_test.c` (the `50` in the bound, lines 1269-1272) is `1408` launches times 50 ms, rounded up to whole seconds: 71. The 50 ms is an allowance for a cold shader invalidate. The printed line says the window is usually shorter. The operator reported the launch at about one second. The log has no elapsed-time field, so that duration is the operator's report, not a second clock in this directory. One second is inside the 71 second allowance.
