# GB10 latency receipt, 2026-11-01

Chip run of the latency kit. The measured columns in `sm121_vs_NAK.md` were filled by `tools/latmeas/fill_template.py` from `raw.json` on the machine. `sha256sum raw.json` is `21735d4b96d339b9b62040db0b7ee985ac168d75eed64ac16c30175d96e8047a`. That file matches a fresh fill from the same `raw.json` (diff empty).

This receipt is the kernels launched on that day. It is not a receipt for later edits that store every lane or that read all four HMMA destinations.

## What ran

- Command: `sh tools/latmeas/run_spark.sh latmeas-out4`
- Result: launch rc 0, `raw.json` `"error": ""`, 11 runs, stall spread 0 on every case.
- Tested tree: `cb5bd7087470336b6d59da53e99ef4dc5edcb7f2`, a merge of PR #254 `03cdcc5a4401043da2196f4f5efcb9b379400f66` and PR #251 `6a93cb567b33b67bc1185fb45a3c50fc4d88d30e`, with no other edits. PR #251 is the dependency that admits SM version `0xa04`. `head.txt` is the tested tree.
- Machine line, copied from `RECEIPT.txt`: Linux spark-b87b, 7.0.0-1019-nvidia, aarch64. Open kernel module 580.173.02. GCC 13.3.0. NVIDIA GB10, PCI `0000000F:01:00.0`. Clocks reported there: 1787 MHz current, 3003 MHz max.
- Measured at 2026-11-01 12:08:46 -0600 (18:08Z), as printed in `RECEIPT.txt`.
- `launch.log` is the runner log. Its last line named a home directory; that path is replaced here with `latmeas-out4/raw.json`.
- The upload did not contain `diff.md`. The disagreement list is `python3 tools/latmeas/diff_nak.py --md`, which does not read the chip.
- `sass/` is `sm121dis --pairs` on the bins that ran, for the smallest passing stall and the stall under it. The two `??` lines in each listing are the CS2R clock sites. The stall field is bits 9..12 of the control word (the high 32 bits of the second hex column).

## Smallest passing stall

| case | stall | sm100 CSV base | NAK encoded (CSV + padding) |
| --- | --- | --- | --- |
| alu_alu | 4 | 4 | 5 |
| fma_fma | 4 | 4 | 5 |
| mixed | 5 | 5 | 6 |
| pred | 13 | 13 | 14 |
| fmnmx_alu | 4 | 4 | 5 |
| imad_wide_lo | 3 | 3 | 4 |
| hmma_reader | 20 | 19 | 28 |

Lane 0's primary result word is `run0_result` in `raw.json`. For the six fixed-latency cases the failing stalls show the sentinel (`0x7f800000` or the wrapped sum `0xf0000000`, or `0xdeadbeef` for the predicate). `imad_wide_lo` stalls 1 and 2 stored 32, and stall 3 stored 528. HMMA stalls 1..19 stored 0 and stall 20 stored `1098907648` (`0x41800000`, 16.0f). `reference_result` for HMMA is that same word from the stall-36 launch, which this run used as the oracle. How those numbers relate to NAK's delay field is `UNITS.md`.
