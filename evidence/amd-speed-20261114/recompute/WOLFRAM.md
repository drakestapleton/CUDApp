# Wolfram checks of the report's derived numbers

Tool: `mcp__wolfram_wolframlanguageevaluator` (hosted Wolfram Language), 2026-11-14. The recompute scripts compute the same
quantities from the raw files; this record checks their arithmetic independently. Inputs are the MEASURED values printed by
`recompute/out/*.txt`. Values agree with the scripts to the printed rounding (the scripts use unrounded inputs, so a
percentage computed here from a rounded TB/s can differ by 0.1 in the last place).

Query:

```
bytes=26919925760; peak=53*10^11;
{N[bytes/peak*10^3,6], N[100*bytes/(#/10^3)/peak,4]&/@{26.721,27.007,26.723,25.614,22.845,20.968}, N[27.007/20.968,5],
 N[20.968/(bytes/peak*10^3),4], 26.49-25.014, 26.49-26.2925, 26.49-24.812, 24.812-24.7221, N[1000/24.812,5],
 26.721-24.741, N[100*(26.49-24.812)/26.49,4], N[26.721/20.968,5],
 N[100*{2.81,2.82,2.83,2.87,2.79,3.08,3.16,3.28,3.26,3.16}/5.3,4], N[17609529200/30866866928,4]}
```

Result:

| Quantity | Wolfram | Report |
|---|---|---|
| floor, ms per token (26,919,925,760 B at 5.3 TB/s) | 5.07923 | 5.08 (MODELED) |
| percent of 5.3 TB/s at 26.721 / 27.007 / 26.723 / 25.614 / 22.845 / 20.968 ms | 19.01 / 18.81 / 19.01 / 19.83 / 22.23 / 24.22 | 19.0 / 18.8 / 19.0 / 19.8 / 22.2 / 24.2 |
| ladder speed-up, full-a over v1all | 1.28801 | 1.288 |
| v1all over the floor | 4.128 | 4.13 |
| planscope gain, planstage gain, plan2 gain (ms) | 1.476, 0.1975, 1.678 | same |
| plan2 minus HIP graph (ms) | 0.0899 | 0.090 |
| plan2 tokens per second | 40.303 | 40.30 |
| repeat gap, Omega minus HIP (ms) | 1.98 | 1.98 |
| plan2 gain as percent of plan | 6.334 | 6.3 |
| published repeat over v1all | 1.2744 | 1.27 |
| 4-bit percent of 5.3 TB/s, q4 then q4r2 (from rounded TB/s) | 53.0, 53.2, 53.4, 54.2, 52.6; 58.1, 59.6, 61.9, 61.5, 59.6 | 53 to 54; 58 to 62 |
| container over source bytes | 0.5705 | 0.570 |

Run 2 (session e8cddcb1), added 2026-11-14 when it landed. Query:
`{26.641-24.891, 24.891-24.7409, 24.8975-24.7409, 24.8975-24.891, N[1000/24.891,5], N[100*(26.641-24.891)/26.641,4]}`
-> `{1.75, 0.1501, 0.1566, 0.0065, 40.175, 6.5688}`: plan3 gain over plan 1.75 ms, plan3 and plan2 behind HIP 0.150 and
0.157 ms, plan2 minus plan3 0.0065 ms (2 against 5 runs), plan3 40.18 tokens per second, gain 6.6 percent of plan.

Drift (Main's finding in MAIN-VERIFY.md), added 2026-11-14. Query:
`{26.641-26.49, 26.658-26.602, 27.157-27.007, N[27.157/20.968,5], N[27.007/20.968,5]}`
-> `{0.151, 0.056, 0.150, 1.2952, 1.2880}`: the published launcher run 2 minus run 1, its first to last run median within
run 2, the two baseline runs of the kernel ladder, and the ladder speed-up against full-b and full-a.

4-bit card rerun (session c715b669), added 2026-11-14. Rate extremes as percent of 5.3 TB/s, from the unrounded rates
printed by `fourbit.py` (q4gemv_d8 min/max, q4gemv_d8r2 min/max, FP8 min/max):
`N[100*{2.66988188, 2.84721250, 2.98304230, 3.17901402, 2.09090594, 2.15173986}/5.3, 4]`
-> `{50.37, 53.72, 56.28, 59.98, 39.45, 40.60}`; report 50.4 to 53.7, 56.3 to 60.0, 39.5 to 40.6.
Sums and ratios, from the rounded values (the script adds unrounded per-tensor times, hence 4.98 here against 4.99):
`{N[12.20/4.44,3], N[27.007/12.20,3], N[20.968/12.20,3], N[16.36/12.20,3], 11.43+0.7740, 4.21+0.7740, 4.21+0.2209}`
-> `{2.75, 2.21, 1.72, 1.34, 12.204, 4.984, 4.431}`: FP8 sum over 4-bit sum 2.75; measured whole step over the isolated
FP8 matrix sum 2.21 (full-a, 27.007 ms) and 1.72 (v1all, 20.968 ms); in-plan matrix time over isolated 1.34.

Combined run (session 6e585e21), added 2026-11-14. Arms A, B, C, D1, D2, E1, E2 in order, m = their decode medians:
`bytes=26919925760; peak=53*10^11; m={27.164,25.425,19.619,17.124,16.519,16.721,16.226};`
`{N[100*bytes/(#/10^3)/peak,4]&/@m, N[1000/#,4]&/@m, N[27.164/16.226,5], N[27.164/16.519,5], N[100*(27.164-25.425)/27.164,4],`
` N[16.226/(bytes/peak*10^3),4], N[16.519/(bytes/peak*10^3),4], 27.164-25.425, 25.425-19.619, 19.619-16.519, 16.519-16.226, 27.164-16.226}`
-> percent of 5.3 TB/s `{18.70, 19.98, 25.89, 29.66, 30.75, 30.38, 31.30}`; tokens per second
`{36.81, 39.33, 50.97, 58.40, 60.54, 59.81, 61.63}`; A/E2 1.6741; A/D2 1.6444; launcher gain 6.40 percent; E2 and D2 over
the floor 3.195 and 3.252; steps 1.739, 5.806, 3.100, 0.293 ms; total A minus E2 10.938 ms.

Launcher run 3 (session 7f66e322), added 2026-11-14. p3, h3, hw = plan3, HIP decode_ms and HIP forward_wall_ms per cycle
c1..c9 (unrounded, from the raw logs):
`{Median[p3-h3], MinMax[p3-h3], Median[(p3+0.0005)-(h3+0.016)], MinMax[(p3+0.0005)-(h3+0.016)], Median[hw-h3], MinMax[hw-h3],`
` Median[p3-hw], N[(1743-7)/1125,4], N[(1744.3-20.0)/1125,4], Median[old3]-Median[p3], Max[p3]-Min[p3], Max[h3]-Min[h3]}`
-> `{0.0957, {-0.0221, 0.1544}, 0.0802, {-0.0376, 0.1389}, 0.1981, {0.1540, 0.2093}, -0.0794, 1.543, 1.533, 1.829, 0.165, 0.133}`.
The like-for-like line here uses the rounded readbacks 0.0005 and 0.016 ms (as Main and the lane did: range -0.038 to
+0.138 after rounding to 3 places); `launcher.py` uses the measured medians (0.00051 and 0.0158 ms) and gets -0.037 to
+0.139. Both give median +0.080 ms and 8 of 9 cycles with Omega behind. Per-dispatch from the medians of the per-step
medians (1.543, 1.533 us) agrees with the per-cycle medians in `launcher.py`.

Extra-row timing, session 3a2e29e5, rerun 2026-11-14 about 04:05Z. rho = (t_R / t_1 - 1) / (R - 1), the definition in
`spec-decode/card-rho/spec_rho.py` line 118:
`one=198.592; {N[(260.800/one-1)/1,4], N[(345.157/one-1)/2,4], N[(739.862/one-1)/7,4]}`
-> `{0.3132452465356106, 0.36901033274250705, 0.3893625408590764}` (rho at R = 2, 3, 8; printed as 0.3132, 0.369, 0.3894).

Round-2 fixes, 2026-11-14 about 04:10Z.
- Section 5, part of the base step not attributed by the inserted-copy method: `26.973 - 23.612` -> `3.361` (printed 3.36;
  `kernels.py` computes it from the unrounded base step as `classes.unattributed_ms`).
- Section 4, run 3 like-for-like row with the printed readbacks, from the per-cycle medians rounded to 3 places:
  `p3={25.153,25.155,25.147,25.148,25.243,25.289,25.312,25.303,25.222}; h3={25.048,25.078,25.07,25.17,25.147,25.135,25.169,25.153,25.181};`
  `{Round[(p3+0.00051)-(h3+0.0158),0.001], Round[(p3+0.00049)-(h3+0.0158),0.001], Median[(p3+0.00051)-(h3+0.0158)]}`
  -> `{{0.09, 0.062, 0.062, -0.037, 0.081, 0.139, 0.128, 0.135, 0.026}, (same), 0.08071}`. Cells c2 and c4 (0.062, -0.037)
  and the median 0.08 agree with the table; c3, c5, c7 and c8 differ by 0.001 because the table uses the unrounded
  per-cycle medians, as the report says. The readback 0.00049 (lane RCCA correction) or 0.00051 (raw plan3t median,
  `launcher.py`) gives the same cells to 3 places.

Round-3 fixes, 2026-11-14.
- Section 6, the 4-bit sum against its own floor (candidate A, `four-bit/FORMAT.md` row A, 15,537,809,408 B per token at
  5.3 TB/s); `fourbit.py` passes the unrounded sum:
  `{N[15537809408/(5.3*10^12)*10^3,6], N[4.435160791942335/(15537809408/(5.3*10^12)*10^3),4], N[4.44/2.93,4]}`
  -> `{2.93166, 1.5128, 1.5154}`. Printed 2.93 ms and 1.51 times (from the unrounded sum; the rounded figures would give
  1.52).
