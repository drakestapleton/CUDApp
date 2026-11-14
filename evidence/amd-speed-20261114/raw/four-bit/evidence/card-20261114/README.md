# Card run 2026-11-14T01:46–01:59Z: session jupyter-user-261114014516-c715b669 (MI300X VF, gfx942, 304 CUs)

| | |
|---|---|
| Bundle | sha256 `10c93d3d…` (`card/cells/BUNDLE.json`): kernels q4gemv_d8 and q4gemv_d8r2; the r4 variant was not yet gated, so it is not included |
| Run | `card_q4.py` through the Omega KFD launcher `launch-x86` 08a50491; HIP is not in the measured path |
| Tarball | `q4card.tar.gz`, sha256 `24f6cc26…`, equal to the value the card printed; 469 of 469 files in `tar/out/SHA256SUMS` check OK |
| Summary | `summary.txt` |
| Saved state | `card_q4.log`, `card_q4.state.json`, `tar/` |
| Exit | `CARD-Q4 FAIL` (verdict explained below) |

## Verdict: CARD-Q4 FAIL, caused by a hung launch, not by a wrong output

- **Hang 1:** the q4gemv_d8 repeat-1 launch for gdn out_proj spun at 100 % CPU for 7 min with RSS 2.7 MB. I stopped it with SIGTERM (rc=-15), so that shape has no q4gemv_d8 timing.
- **Hang 2:** the FP8 baseline launch for mlp gate_proj spun the same way for 1:52. The process had opened `/dev/kfd` and the render node, held 1.3 GB of VA, and had RSS 2.7 MB. Details in `hang-2.txt`. I killed it, so that shape has no FP8 baseline.
- **Outputs:** every 4-bit output that ran, from both kernels, is bit-exact against the oracle. 19 kernel×shape results, every row equal.

## Container identity
The on-card conversion of layers-0, layers-3 and outside has the same sha256 as the Spark's. Card numpy 2.4.4, Spark numpy 2.5.3.

## Method
- Each shape is tiled to at least 600 MB of 4-bit weights per dispatch, more than the 256 MB Infinity Cache.
- The FP8 baseline is tiled by the same count.
- Time = (median of three repeat-50 runs − median of three repeat-1 runs) / 49, on the host monotonic clock.
- Per-tensor time = that time divided by the tile count.
