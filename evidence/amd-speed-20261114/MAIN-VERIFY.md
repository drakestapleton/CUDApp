# Independent recomputation by the lead session (uxl-brief/Main)

Method: unpack the lane's evidence tarball, take every `OMEGA_STEP i tok=T us=U` line of a run, drop the first 30 steps (the prompt), take the median of the rest in ms. For HIP runs, read `decode_ms_median` from each JSON. Median of per-run medians where a row has several runs. Done 2026-11-14 about 01:45Z with a Python one-off; inputs named below.

## Launcher gap, run 1 (`amd-launch-gap/card/gap-evidence.tar.gz`, session jupyter-user-261114005045-52ef268c)

| Arm | Runs | Per-run medians (ms) | Median (ms) | Lane's figure | Agrees |
|---|---|---|---|---|---|
| Published launcher, `plan` | 5 | 26.494, 26.483, 26.468, 26.490, 26.490 | 26.490 | 26.490 | yes |
| New launcher, `plan2` | 5 | 24.809, 24.800, 24.833, 24.827, 24.812 | 24.812 | 24.812 | yes |
| HIP graph | 5 | 24.731, 24.658, 24.706, 24.749, 24.722 | 24.722 | 24.722 | yes |
| New binary in old `plan` mode | 2 | 26.493, 26.488 | 26.490 | 26.490 | yes |
| Scope change alone | 2 | 25.009, 25.019 | 25.014 | 25.014 | yes |
| Staging alone | 2 | 26.300, 26.285 | 26.293 | 26.293 | yes |

## Kernel speed, session 1 (`amd-kernel-speed/card/261114001549-e787e65d/aks-results.tar.gz`)

| Run file | Steps | Decode median (ms) | Lane's figure | Agrees |
|---|---|---|---|---|
| `full-a.txt` (baseline) | 285 | 27.007 | 27.007 | yes |
| `full-b.txt` (baseline again) | 285 | 27.157 | not quoted | n/a |
| `cap286.txt` | 285 | 26.723 | 26.723 | yes |
| `v2-gdnstate_step.txt` | 285 | 22.845 | 22.845 | yes |
| `v2-rmsnorm5120.txt` | 285 | 25.614 | 25.614 | yes |
| `v1all.txt` (all combined) | 285 | 20.968 | 20.968 | yes |

Each of these six is ONE run of 256 generated tokens. The two baselines differ by 0.150 ms, which is the visible run-to-run spread in that session. Not checked here: token ids against the published list, and the altered-weights control (the lane reports both; the report writer must verify them from the same files).

## Launcher gap, run 2 (`amd-launch-gap/card/run2/gap-evidence.tar.gz`, sha256 starts 8471a458, session jupyter-user-261114012602-e8cddcb1)

Recomputed 2026-11-14 about 02:00Z, same method.

| Arm | Runs | Per-run medians (ms) | Median (ms) | Lane's figure | Agrees |
|---|---|---|---|---|---|
| Published launcher, `plan` | 5 | 26.602, 26.626, 26.641, 26.653, 26.658 | 26.641 | 26.641 | yes |
| New launcher, `plan3` | 5 | 24.884, 24.893, 24.885, 24.891, 24.914 | 24.891 | 24.891 | yes |
| New launcher, `plan2` | 2 | 24.899, 24.896 | 24.8975 | 24.8975 | yes |
| HIP graph | 5 | 24.704, 24.726, 24.746, 24.741, 24.747 | 24.741 | 24.741 | yes |

Observations for the report: the same published launcher measured 26.490 ms in run 1 and 26.641 ms in run 2 (different sessions, 0.151 ms apart), and within run 2 its five runs rise monotonically from 26.602 to 26.658 while its warmup was 26.489. So session-to-session and within-session drift on this hosted card is of the same size as the gap being chased (0.090 ms in run 1, 0.150 ms in run 2). Any claim about closing that gap needs interleaved arms and more repeats than five.

## Combined run (`amd-kernel-speed/combined/card-2026-11-14T0206Z/akc-results.tar.gz`, sha256 starts 493d6c29, session jupyter-user-261114020544-6e585e21)

Recomputed 2026-11-14 about 02:40Z, same method; three interleaved runs per arm, 285 steps each.

| Arm | Per-run medians (ms) | Median (ms) | Lane's figure | Agrees |
|---|---|---|---|---|
| A published launcher | 27.211, 27.164, 27.149 | 27.164 | 27.164 | yes |
| B plan3 launcher | 25.498, 25.425, 25.349 | 25.425 | 25.425 | yes |
| C B + rmsnorm5120 v2 + gdnstate_step v2 | 19.619, 19.623, 19.611 | 19.619 | 19.619 | yes |
| D1 C + fp8 v2b1 | 17.056, 17.124, 17.127 | 17.124 | 17.124 | yes |
| D2 C + fp8 v2b2 | 16.527, 16.517, 16.519 | 16.519 | 16.519 | yes |
| E1 D1 + cap286 | 16.712, 16.797, 16.721 | 16.721 | not quoted | n/a |
| E2 D2 + cap286 | 16.226, 16.151, 16.237 | 16.226 | 16.226 | yes |

All 21 non-control runs print one and the same id list; the control run prints a different one. Wolfram: 27.164/16.226 = 1.6741; 1000/16.226 = 61.63 tokens per second; 26.92 GB per step at 16.226 ms is 31.30 percent of 5.3 TB/s. Not checked by me: that the common id list equals the published 256 (the report writer checks it).
