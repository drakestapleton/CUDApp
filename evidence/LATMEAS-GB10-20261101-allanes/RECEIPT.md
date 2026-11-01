# GB10 all-lane latency run, 2026-11-01

Second chip run of the latency kit. The operator reported the window as 2026-11-01 18:38–18:40Z. This directory is that output. The script did not write a `RECEIPT.txt` (that writer was added afterwards). The upload's sha256 is `ae4cdbcbfe7d15d2766f8d661cf35cbb88b7358fde20afcca5c441bd7071464f`. `sha256sum raw.json` is `565ece7ec537961bb70c66cdac7afea1cd6fdb0463e4ee70325a3bc528fce403`.

`sm121_vs_NAK.md` is the table `fill_template.py` wrote from this `raw.json`. A fresh fill of the same file matches it.

## What ran

- Command: `sh tools/latmeas/run_spark.sh latmeas-out` (see `run.log`).
- Launch rc 0. `raw.json` has `"error": ""`, `"launches": 1408`, `"runs": 11`, stall spread 0 on every case.
- Tested tree: `c94c2c93442adf7fa24c22dee7e8a91bb1c93226` (`head.txt`). That commit is a merge of `4f3a4d0` (every lane stores four result words; HMMA reads `d0`..`d3`) and PR #251 `6a93cb567b33b67bc1185fb45a3c50fc4d88d30e`, with no other edits.
- Doctor, from `run.log`: Linux 7.0.0-1019-nvidia aarch64, open module 580.173.02, GPU `000f:01:00.0` NVIDIA GB10 (sm_121), compiler 13.3.0, READY. The binary printed a 71 s upper bound. The script also printed a stale 90 s line; the binary is the bound.
- Two home-directory paths in the log are replaced with `latmeas-out`.

## Smallest passing stall

Same as `evidence/LATMEAS-GB10-20261101/` (`03cdcc5`). From this `raw.json`:

| case | stall_required |
| --- | --- |
| alu_alu | 4 |
| fma_fma | 4 |
| mixed | 5 |
| pred | 13 |
| fmnmx_alu | 4 |
| imad_wide_lo | 3 |
| hmma_reader | 20 |

## What this file proves

`run0_result` is lane 0's first result word at each stall. `reference_result` is null for every case. There is no host oracle, no per-lane count, and no `d1`..`d3` word in the JSON.

The source at `4f3a4d0` sets a stall as passing only when all 32 lanes match all four result words (HMMA: the four `FADD` results; other cases: the result, the auxiliary word, and two zeros). A passing `stall_required` with spread 0 is what that check produced, and it landed on the same stalls as the lane-0 run. A reader of `raw.json` alone cannot see a lane bitmask or the four destination words. That recording is a later change and is not in this directory.
