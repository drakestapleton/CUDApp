---
title: "Omega on one AMD Instinct MI300X: Qwen3.8-27B-FP8 decode speed, 13 and 14 November 2026"
author: "Drake Stapleton"
date: "14 November 2026"
---

# Omega on one AMD Instinct MI300X: Qwen3.8-27B-FP8 decode speed

Final report, 14 November 2026. Readers: engineers at Intel (UXL) and AMD. Status of every section is in its first line.

This report was written by the author's AI agents under the author's direction (section 2).

## 1. Summary

State: FINAL. COMPLETE for every result measured on the card. Four card items are to be done when equipment is
available; for each, the CPU work done so far is listed with its source (local, unreviewed and not landed):

- Several tokens per pass, timed on the card (section 7): the multi-token host loop passes its CPU gate on local branch `spec-decode2` (commit `f859c88f`), and the R-row kernels and the whole-pass plan gate pass theirs (their reports name no branch; section 7 names each file and the one mutant the whole-pass gate does not catch). The card bundle was rehearsed on the Spark CPU (`spec-decode/card-s4/dry4.log`): the launchers accepted all 11 plans in 14 plan checks (11 with `launch-spec`, 3 with `launch-x86`), the log ends `SPEC-S4 PASS`, a corrupted bundle was refused and a wrong id caught. The rehearsal script says the real launchers ran under qemu and stopped at the device, and a stand-in printed the timed output (`spec-decode/card-s4/dry_s4.sh` lines 4 to 5, `spec-decode/card-s4/fake_launch.py` lines 2 to 4); the 26.157 ms per token in the log's two `RUN` lines is the stand-in's output, not a measurement. The extra-row cost of the FP8 kernel is already measured on the card (section 7). To be done when equipment is available.
- 4-bit quality on the card and the four-row kernel's timing (section 6): the 4-bit gate, extended to `q4gemv_d8r4`, passes: 57 cases over the three kernels (19 for `q4gemv_d8r4`), 9 of 9 mutants caught, 2 of them specific to the four-row kernel (`four-bit/evidence/gate-r4.log`). To be done when equipment is available.
- Kernel step 2 (section 5): fused add and norm (1,126 to 998 dispatches) and merged delta-net a/b projection (48 fewer), both plan rewrites logged in the Spark simulation (`raw/amd-report/extracted/spark-logs-20261114/kernel-step2-sim-next.log`); FP8 kernels with 2 and 4 rows per wave. `addnorm5120` v2 passes its CPU gate, 6 cases, 3 mutants caught (`amd-kernel-speed/gates/gate-addnorm5120.log`); `fp8gemv_b128f` v3r2 (`9377954c`) and v3r4 (`5620081b`) are each row bit-identical to v1 on the CPU and pass their gates with 62 cases and 22 and 27 mutants caught (`raw/amd-kernel-speed/kernels/FP8-R2-RESULT.md` lines 12 to 13, 116 to 121 and 126 to 142). In the same simulation 31 of 31 runs of the card bundle exit 0; that the real launcher validated every plan and argument list in them is quoted from `amd-kernel-speed/RESULT.md`, a lane note not in this folder. To be done when equipment is available.
- Launcher run 4 (section 4): launcher modes `plan3a` (2 MiB buffer placement) and `plan3v` (token cell in VRAM). That they are built is quoted from `amd-launch-gap/RESULT.md`, a lane note not in this folder. The prepared card script runs both modes with launcher `657db853`, which it checks by sha256 (`amd-launch-gap/card/bundle4/gap-run4.sh`). That launcher's Spark gate is 308 checks, 0 failures (`amd-launch-gap/build-r15-spark-gate.log`); the log does not name the two modes, so it shows that the existing checks pass with that launcher, not that the new modes are gated. To be done when equipment is available.

**4-bit quality on the CPU: IN PROGRESS.** The frozen-bar comparison of the `fp8` and `q4` arms had not finished when
this report was written; its result is not in this report, so 4-bit weights are neither accepted nor rejected here
(section 6).

On 13 November an Omega program ran Qwen3.8-27B-FP8 on one AMD Instinct MI300X at a median decode time of 26.726 ms per
token, with no AMD user-space runtime in its process (the published run). Since then the work has gone after one goal set by
the author on 14 November: as fast as possible on one card.

**Headline (MEASURED, one card, one session 6e585e21, three interleaved 256-token runs per arm).** The fixed launcher and
the new kernels together decode at **16.226 ms per token (61.6 tokens per second)**, against 27.164 ms for the published
launcher and kernels in the same session: **1.674 times faster**. Every one of the 21 timed runs printed the published 256
token ids, and the altered-weights control differed at index 0. That run uses a plan sized for this request's 286
positions; on the general plan (131,072 positions) the same launcher and kernels take 16.519 ms (1.644 times). Each step
in the same session:

| Arm | What changed | Decode, ms per token (MEASURED) | Percent of 5.3 TB/s (estimate) |
|---|---|---|---|
| A | published launcher and kernels | 27.164 | 18.7 |
| B | launcher fix (plan3) | 25.425 | 20.0 |
| C | plus RMSNorm v2 and delta-net state update v2 | 19.619 | 25.9 |
| D2 | plus FP8 matrix-vector v2b2 (taken over from lane GpuBenchSync) | 16.519 | 30.7 |
| E2 | D2 on the 286-position plan | 16.226 | 31.3 |

The launcher fix is the first 1.739 ms; the RMSNorm and state rewrites 5.806 ms; the faster FP8 kernel 3.1 ms; the smaller
plan 0.293 ms. The output-identical floor for this plan, reading every weight byte once per token at the card's quoted
5.3 TB/s, is 5.08 ms per token (MODELED), so the best arm is still 3.19 times the floor and moves weights at 31.3 percent
of that bandwidth (an estimate: modelled bytes per step over measured time).

Earlier sessions found the pieces separately (sections 4 and 5): the launcher was 1.98 ms per token behind AMD's HIP
runtime replaying the same 18 kernels, because of system-scope memory fences on all 1,126 dispatches of a step and a single
doorbell after the host wrote the whole step; fixing both took 26.490 to 24.812 ms in one session and 26.641 to 24.891 in
another. A third session paired each Omega run with a HIP run over nine cycles: with the same clock on both sides, Omega
still trails HIP by 0.08 ms per token after each side is charged its own token read (paired median, 0.3 percent;
behind in 8 of 9 cycles; 0.096 ms on the raw step clocks), the launch path itself
costs the same on both, and the remaining gap is by elimination inside kernel execution (section 4).

4-bit weight kernels now run bit-exact on all 10 matrix shapes of the model on the card, at 2.98 to 3.18 TB/s isolated
against 2.09 to 2.15 for the v1 FP8 kernel on the same tensors. Their isolated times add up to 4.44 ms per token for
every FP8 linear and the head (MODELED), against 12.2 ms for the v1 FP8 kernel; v2b2, the fastest FP8 kernel of the
headline, was not timed by this method. But the measured whole step on the v1 kernels (21 to
27 ms) is about twice the isolated sum of its FP8 matrix kernels, so the kernels between and around the matrix products,
and the gaps between dispatches, matter as much as the weights (section 6). 4-bit quality is IN PROGRESS on the CPU.

| One MI300X, lighthouse request, greedy, 256 tokens | Decode, ms per token (median) | Label | Session (short id) |
|---|---|---|---|
| Published Omega run, 13 Nov 10:12 UTC | 26.726 | MEASURED | 76e03a28 |
| Repeat from public instructions, 13 Nov 19:07 UTC | 26.721 | MEASURED | 46d1b9c5 |
| AMD HIP graph replay of the same 18 Omega kernels, same session | 24.741 | MEASURED | 46d1b9c5 |
| Launcher before (plan), 14 Nov run 1 | 26.490 | MEASURED | 52ef268c |
| Launcher after (plan2), same session | 24.812 | MEASURED | 52ef268c |
| HIP graph replay, same session | 24.722 | MEASURED | 52ef268c |
| Launcher plan3 (plan2 plus agent-scope acquire on decode steps), 14 Nov run 2 | 24.891 | MEASURED | e8cddcb1 |
| Launcher before (plan) and HIP replay, same session | 26.641 and 24.741 | MEASURED | e8cddcb1 |
| Kernels before (full plan, v1 kernels), one run | 27.007 | MEASURED | e787e65d |
| Kernels after (v2 RMSNorm and state kernels, 286-position plan), one run | 20.968 | MEASURED | e787e65d |
| Combined: published launcher and kernels (A) | 27.164 | MEASURED, 3 runs | 6e585e21 |
| Combined: launcher fix and all new kernels, general plan (D2) | 16.519 | MEASURED, 3 runs | 6e585e21 |
| Combined: the same on the 286-position plan (E2) | 16.226 | MEASURED, 3 runs | 6e585e21 |
| Several tokens per pass (speculative decoding with the model's own head) | NOT_RUN on the card | CPU gates PASS | |
| 4-bit weights, whole model | NOT_RUN | kernels bit-exact on 10 shapes, `q4gemv_d8r2` 2.98 to 3.18 TB/s isolated; quality IN PROGRESS | c715b669 |
| v1 FP8 linears and BF16 head, sum of isolated kernel times | 12.2 | MODELED: no launch gaps, no non-matrix kernels | c715b669 |
| 4-bit linears and 4-bit head, same method | 4.44 | MODELED: same exclusions, quality IN PROGRESS | c715b669 |
| Output-identical lower bound at 5.3 TB/s, this plan | 5.08 | MODELED | |
| 4-bit floor at 5.3 TB/s, candidate A bytes, head kept in BF16 (4-bit linears with BF16 head above are 1.70 times it) | 2.93 | MODELED: not output-identical, needs the quality bar | |

Two numbers for the same arm in different sessions differ (the published launcher and kernels on the full plan measured
26.726, 26.721, 26.490, 26.641, 27.007, 27.164 and 27.051 ms per token in seven sessions), so every before and after pair above is compared only inside
one session. Drift on this hosted card is as large as the remaining gap to HIP, so that gap is measured only by paired
runs (section 4, run 3).

## 2. Machine and method

State: COMPLETE.

- **Card.** AMD Instinct MI300X, gfx942, reported as "AMD Instinct MI300X VF" (one virtual function), in AMD's hosted
  One-Click notebook "Qwen/Qwen3.8-27B on AMD Instinct MI300X", opened from the author's AMD developer sign-in. Linux
  6.12.48, ROCm 7.2.2 in the container; root inside the container with `/dev/kfd`. The card is one GPU of a shared
  host; exclusive use of the card was not established.
- **Model.** `Qwen/Qwen3.8-27B-FP8`, revision `017b9c7af6b5689d5dd426a76e0bc077eb5ca20a`, text path only, every file
  checked against pinned sha256 values before use.
- **Request.** The lighthouse essay prompt, 30 prompt tokens fed as recorded ids, greedy, 256 generated tokens. The
  launcher runs the prompt one token at a time, so the first 30 steps are prefill and the remaining 255 are decode.
- **Clock.** The launcher prints the wall time of every step (`OMEGA_STEP i tok=T us=U`), from the first packet write to
  the completion signal. Decode time is the median of the 255 decode steps of one run; an arm's figure is the median of
  its runs' medians. HIP figures are synchronized wall time for one replayed step, from the same Python driver.
- **Interleaving.** In the launcher session every arm ran one warmup and its timed runs in turn (old, new, HIP), so a
  drifting card affects all arms alike. The first kernel session ran one run per candidate; the combined session ran all
  seven arms in turn, three times.
- **Software in the process.** `omega-launch`, an Omega program compiled by the seeded Omega compiler (omc sha256
  `26540e6d...`, omega-mv `2ee0a70d`), and 18 gfx942 code objects written by Omega's own code-object writer. Python,
  PyTorch and HIP are measuring and comparison tools outside that process.
- **Numbers.** `numbers.json` lists each checked number with its label (MEASURED, DERIVED, MODELED, QUOTED, COUNT,
  RECORD), unit, source file and sha256. `recompute/run-all.sh` recomputes the MEASURED, DERIVED and COUNT values from the
  raw logs; the QUOTED and MODELED constants (`recompute/constants.json`) are listed with their sources and not
  recomputed. `recompute/check_report.py` then checks this report: every decimal number outside code spans equals a value
  in `numbers.json` to its printed precision; a decimal followed by a unit word (ms, us, percent, times, TB/s, GB, tokens
  per second, tokens per pass) must equal a value of that unit; and the claims listed in `recompute/claims.json` (the
  headline ratios and the counts of runs, launches and gate cases, integers included) must equal
  their named values. Other integers, and decimals in tables without a unit word, are checked only by the first rule or
  not at all. A self-test changes one number at a time in a copy and requires the check to fail. Ratios and percentages
  were also checked in Wolfram (`recompute/WOLFRAM.md`). A few QUOTED and MODELED figures come from the lanes' own
  notes, named where they are used (for example `amd-kernel-speed/RESULT.md`, `spec-decode/model/wolfram.md`); those
  notes are not in the publication folder. Paths in this report are written relative to the evidence folder `raw/`
  (for example `amd-launch-gap/HANG-20261114.md` means `raw/amd-launch-gap/HANG-20261114.md`), except where a sentence
  says it quotes a lane note; those notes are named by their own path and are not in the folder. One planning note is
  referred to by a file in `raw/` but is not published: `multi-card/ACCESS.md` (named in
  `raw/multi-card/model/WOLFRAM.md`).
- **Who did the work.** The author directed AI coding agents, which ran every card session and wrote this report. A
  *lane* is one agent's line of work, named after it: AmdLaunchGap (launcher), AmdKernelSpeed (kernels), FourBit (4-bit
  weights), SpecDecode and SpecDecode2 (several tokens per pass), GpuBenchSync (an earlier kernel-timing lane), MultiCard
  (eight cards), AmdReport (this report). *Main* is the coordinating agent that assigned the work and decided between
  lanes. *The Spark* is the author's own NVIDIA DGX Spark, used for CPU gates and CPU reference runs; *the Mac* is the
  author's workstation, where the agents ran and the files were kept.
- **Terms.** The *lighthouse request* is the 30-token essay prompt above with 256 greedy tokens. The *published run*
  is the 13 November run in CUDApp `dc536882`. *Level 3* is the brief's on-card repeat procedure. `plan`, `plan2`,
  `plan3`, `plan3c`, `plan3t` are launcher modes (section 4). *v1* is a published kernel, *v2* and *v3* its rewrites;
  `v1all` and `cap286` are plans (section 5). *Delta-net* is the gated delta-net recurrent layer of this model. *S4* is
  the on-card step of several tokens per pass (section 7). The *frozen bar* is the 4-bit quality threshold fixed before
  any 4-bit result (section 6).
- **Bandwidth percentages are estimates.** They divide the modelled weight bytes per step (26.92 GB: every weight and
  block scale once, the BF16 output head in full, one embedding row) by the measured time, against AMD's quoted 5.3 TB/s.
  No memory counter was read.
- **Paths.** A path in this report is under `raw/` unless it starts with `raw/` or `recompute/`, or is named as a lane
  note not in this folder.

## 3. Baseline

State: COMPLETE.

| Run | Run medians, ms per token | Decode median | Prefill, 30 tokens |
|---|---|---|---|
| Published, session 76e03a28, launcher `08a50491` | 26.731, 26.642, 26.675, 26.726, 26.76 | 26.726 | 779.042 ms |
| Repeat from the public clone, session 46d1b9c5 | 26.743, 26.803, 26.706, 26.671, 26.721 | 26.721 | 779.599 ms |

Same session as the repeat (46d1b9c5):

| Path | Decode, ms per token |
|---|---|
| Omega launcher and Omega kernels | 26.721 |
| The same 18 Omega kernels replayed by AMD's HIP runtime (graph replay) | 24.741 |
| PyTorch with transformers on ROCm, same FP8 checkpoint, plain greedy loop: eager, transformers' fast path not available, FP8 kernels from an older community kernel package on the Hugging Face Hub falling back from DeepGEMM to Triton | 131.011 |
| The notebook's stock pipeline, BF16 checkpoint, 70 prompt tokens, thinking on (not like for like) | 70.628 |

Neither PyTorch row is a speed baseline; both are correctness references under the conditions in the rows. No vLLM
or SGLang ran on the card. The comparison that matters for the launcher is
the HIP row, because it runs the same kernels in the same order.

## 4. Launcher

State: COMPLETE for runs 1, 2 and 3. Run 3 (paired) shows a small remaining gap to HIP; its cause is open, owner
AmdLaunchGap. Run 4 is to be done when equipment is available (section 1).

**The gap.** In session 46d1b9c5 the Omega launcher took 26.721 ms per token and HIP graph replay of the same kernels
24.741: 1.98 ms per token.

**The clocks were not the cause.** Both timers stop before the host reads the token back, and both include the on-device
argmax (lane analysis from source: Omega `plan.om` lines 549 to 565 at `dc536882`; HIP driver `amd-launch-gap/src/CUDApp-base/evidence/amd-qwen38-native-20261113/oracle/driver-real2.py`
lines 206 to 210). HIP's interval contains extra host work, so the clock choice, if anything, understates the gap.

**Root cause, two parts** (read from ROCm 7.2.2 ROCclr source and the Omega launcher):

1. *System-scope fences on every packet.* Every one of the 1,126 AQL packets in a step carried header `0x1502`: acquire
   and release at system scope. On gfx942 that writes back and invalidates the L2 for system-coherent lines at every
   kernel boundary. HIP marks packets with agent scope between kernels and uses system scope only at the step boundary.
2. *One doorbell after the whole step.* The host wrote all 1,126 packets and then rang the doorbell once, so the card sat
   idle while the host wrote. HIP rings a doorbell after batches of 1, 2, 4 and so on up to 256 packets, so the card
   starts after the first packet.

**The fix** (CUDApp local branch `amd-launch-gap`, commit `139bb6a9`, two commits on `dc536882`; not landed, not reviewed
yet by the author's order to review at the end). New launcher modes, the old `plan` mode unchanged:

- `plan2`: system acquire on packet 0 and system release on the last packet of each step, agent scope between; doorbells
  after batches of 1, 2, 4 up to 256 packets.
- `planscope` and `planstage`: one of the two changes each, for attribution.
- `plan3`: plan2 plus an agent-scope acquire on packet 0 of decode steps, where the host wrote nothing a kernel reads.

CPU gate: `tests/amd-kfd-launch/check.py` 308 checks PASS, 0 failures, device rows NOT_RUN, at commit `d957526e` (the script is a CUDApp repository path, not in this folder),
which adds the alarm diagnostics of the hang paragraph below to this fix (`amd-launch-gap/build-r14-spark-gate.log`; the
log names the x86 launcher it built, `a6aee5ef`, the one run 3 used, and no commit; `amd-launch-gap/HANG-20261114.md`
line 20 ties `d957526e` to `a6aee5ef`). Section 11 lists each commit with its launcher.

**Measured, one card, one session** (run 1, session 52ef268c, 14 Nov from 00:50 UTC; 1 warmup plus timed runs, arms
interleaved; every run printed the published 256 ids):

| Arm | Runs | Decode, ms per token | Gain against plan, ms |
|---|---|---|---|
| plan (published launcher `08a50491`) | 5 | 26.490 | |
| plan, rebuilt binary (contract unchanged) | 2 | 26.4905 | |
| planstage (doorbells only) | 2 | 26.2925 | 0.1975 |
| planscope (fence scope only) | 2 | 25.014 | 1.476 |
| plan2 (both) | 5 | 24.812 | 1.678 |
| HIP graph replay, same kernels | 5 | 24.722 | |

plan2 is 6.3 percent faster than plan in the same session (40.30 tokens per second). It is 0.090 ms per token behind HIP
(0.36 percent). The two parts add up: 1.476 plus 0.1975 is within a few microseconds of the 1.678 measured together. The
lane had modelled at most 1.854 ms for the fence change and 0.126 ms for the doorbells before the card run.

**Run 2, one card, one session** (session e8cddcb1, 14 Nov from 01:26 UTC; old plan, plan3 and HIP interleaved, 1 warmup
plus 5 timed runs each, then two plan2 runs and a plan3 altered-weights control; all 20 non-control runs printed the
published 256 ids, the control differed at index 0):

| Arm | Runs | Decode, ms per token | Behind HIP, ms |
|---|---|---|---|
| plan (published launcher `08a50491`) | 5 | 26.641 | |
| plan2 | 2 | 24.8975 | 0.1566 |
| plan3 | 5 | 24.891 | 0.1501 |
| HIP graph replay, same kernels | 5 | 24.741 | |

plan3 is 1.75 ms per token faster than plan in this session (6.6 percent, 40.18 tokens per second), which repeats the
run 1 result. plan3 against plan2 is 0.0065 ms, from 5 against 2 runs, far inside the drift described next, so this run
shows no effect of the plan3 change and cannot rule one out.

**Drift is as large as the remaining gap** (found by Main's independent recomputation, `MAIN-VERIFY.md`; numbers
recomputed here). The unchanged published launcher measured 26.490 ms in run 1 and 26.641 ms in run 2, 0.151 ms apart.
Within run 2 its five timed runs rose in every step, 26.602, 26.626, 26.641, 26.653, 26.658 (0.056 ms first to last),
after a warmup of 26.489; HIP rose over the same interleaved period (24.704 to 24.747). The gap to HIP read 0.090 ms in
run 1 and 0.150 ms in run 2, the same size as that drift. So runs 1 and 2 could not establish whether Omega still trails
HIP at all: that needs interleaved, paired arms (each Omega run next to a HIP run, the difference taken per pair) and more
than five repeats. Run 3 below is designed that way. The 1.7 ms launcher gain is many times the drift and repeats in every
session.

**A launcher hang (section 13).** In the 4-bit rerun, 2 of 175 single-kernel launches of the published launcher never
finished and spun until killed. Why their completion never arrived is not established. Commit
`d957526e` adds the card's queue pointers and the signal value to the 10 s slow-wait alarm (CPU gate PASS); it ran on
the card in run 3 as launcher `a6aee5ef`, where no wait reached 10 s, so the new alarm rows have not yet fired. The next
occurrence will show whether the card consumed the packet. None of the 71 plan processes behind this section and section
5's earlier sessions hung or raised the alarm, nor did any of the 22 in the combined session (each launch there also ran
under a 150 s timeout) or the 67 in run 3 (64 of them with `a6aee5ef`).
A hung launch prints no timing, so no number here is affected.

**The launcher fix in the combined session.** With the same v1 kernels and full plan, arm B (plan3) took 25.425 ms per
token against 27.164 for the published launcher (arm A): 1.739 ms, 6.4 percent, the same size as in runs 1 and 2. That
session had no HIP arm.

**Run 3, paired against HIP (MEASURED, session 7f66e322, 14 Nov from 02:23 UTC).** New launcher `a6aee5ef` (commit
`d957526e`), published launcher `08a50491`, both hash-checked on the card; the card-emitted plan is the published
`2a292dad`. The tarballs' sha256 equal the card-side values and every file in the run's checksum list matches. The run
log's start stamps show nine strictly interleaved cycles, each plan3, then plan3c (the same with a cached ring), then HIP
graph replay of the same 18 kernels, after one warmup of each. The published launcher ran three times after the nine
cycles, so it is not interleaved with them.

| Decode, ms per token | c1 | c2 | c3 | c4 | c5 | c6 | c7 | c8 | c9 | Median |
|---|---|---|---|---|---|---|---|---|---|---|
| plan3 | 25.153 | 25.155 | 25.147 | 25.148 | 25.243 | 25.289 | 25.312 | 25.303 | 25.222 | 25.222 |
| plan3c | 25.148 | 25.152 | 25.172 | 25.159 | 25.212 | 25.201 | 25.29 | 25.315 | 25.247 | 25.201 |
| HIP `decode_ms` | 25.048 | 25.078 | 25.07 | 25.17 | 25.147 | 25.135 | 25.169 | 25.153 | 25.181 | 25.147 |
| HIP `forward_wall_ms` | 25.202 | 25.278 | 25.226 | 25.373 | 25.345 | 25.29 | 25.325 | 25.363 | 25.381 | |
| Like for like: (plan3 + 0.00051) minus (HIP + 0.0158) | 0.09 | 0.062 | 0.061 | -0.037 | 0.08 | 0.139 | 0.127 | 0.134 | 0.026 | 0.08 |

*Which clock is fair.* Like for like here means "the next token id is in host memory": Omega's step time plus its own
token read (0.00051 ms, from the plan3t phase timer), against HIP's `decode_ms` plus HIP's 4-byte token readback (0.0158
ms, median of 15.0 to 17.4 us measured in the micro-benchmark below, not inside these model runs). The table uses these
unrounded medians. HIP's `forward_wall_ms`
is not that clock: around the same call it also times a blocking copy of all 248,320 logits (about 0.99 MB) to the host,
the token readback and the Python call (`driver-real2.py` line 210). It runs 0.198 ms above `decode_ms` (median; 0.154
to 0.209). Omega never copies the logits, and a program that only generates tokens has no need to, so charging HIP for
that copy would make Omega look faster than it is: against `forward_wall_ms` plan3 is ahead in all 9 cycles (plan3 minus
wall, median -0.079 ms), and that comparison is not used here. Raw `decode_ms` without either readback gives plan3 minus HIP of 0.096
ms (median; -0.022 to 0.154).

*Result.* Like for like, plan3 trails HIP by a paired median of 0.08 ms per token (0.3 percent), ranging from -0.037 to
0.139 over the 9 cycles, behind in 8 of 9. The gap is not closed. Within the session plan3 drifted by 0.165 ms between
cycles and HIP by 0.133 ms, which is why only the paired differences are used. plan3c against plan3 is -0.003 ms (median;
-0.088 to 0.025), so the cached ring has no effect. The published launcher read 27.051 ms after the cycles, 1.829 ms
above plan3, the same size of gain as in the earlier sessions (not interleaved here).

**Run 3 micro-benchmark: the launch path itself (MEASURED, same session).** Each step is N dispatches of the model's own
`step_advance` kernel with its real arguments and no weight traffic, 100 steps per process, nine interleaved cycles of
plan3, plan3c and HIP.

| us per step, median over 9 cycles | N = 1 | N = 1,126 | Per dispatch, (N=1,126 minus N=1) / 1,125 |
|---|---|---|---|
| Omega plan3 | 7 | 1743 | 1.543 |
| Omega plan3c | 7 | 1744 | 1.543 |
| HIP graph replay | 20.0 | 1744.3 | 1.533 |
| Paired plan3 minus HIP, median (range) | -12.8 (-16.0 to -11.8) | -10.2 (-23.6 to 3.4) | 0.0025 (-0.0067 to 0.0139) |

So the cost of launching 1,126 back-to-back dispatches is the same on both runtimes, 1.743 ms per token under Omega and
1.744 ms under HIP, and Omega's fixed per-step part is about 13 us shorter. This holds for a trivial kernel; it is the
launch path, not the model's kernels.

**What is established, and what is not.** Established: Omega's dispatch, doorbell, completion detection and token read
are not slower than HIP's; the cached ring makes no difference; with the fair clock, plan3 still trails HIP by about
0.08 ms per token in 8 of 9 paired cycles. Not established: where that 0.08 ms goes. By elimination it is inside kernel
execution, the same 18 kernels running slightly slower under Omega's memory setup [INFERENCE: not measured directly].
The lane names two differences in that setup: one large VRAM region with buffers at 4 KiB offsets against PyTorch's
per-tensor allocations, and the token cell in host-coherent memory against VRAM. Run 4 (2 MiB buffer placement and the
token cell in VRAM, interleaved against HIP the same way) is prepared. That its modes are built is quoted from
`amd-launch-gap/RESULT.md`, a lane note not in this folder; the card script runs them with launcher `657db853`, whose
Spark gate covers the existing checks only (section 1). To be done when equipment is available.

**Node selection (CPU-gated, not run on a card).** At commit `94af5a38` the launcher opens only the KFD topology
node named by `OMEGA_KFD_NODE=N` and refuses to start when the variable is unset or malformed; `omega-launch discover`
prints the node once and opens no device. Spark gate: 318 checks, 0 failures
(`amd-launch-gap/build-r16-spark-gate-94af5a38.log`, launcher `8b6005e8`).

## 5. Kernels

State: COMPLETE. The combined run (launcher fix and all new kernels, three interleaved runs per arm) is the result; the
earlier one-run ladder is kept as history.

**Floor (MODELED).** One decode step reads 26.92 GB of weights and block scales (`amd-kernel-speed/model/classes.tsv`).
At 5.3 TB/s that is 5.08 ms per token: a modelled lower bound for an implementation that reads every weight byte once per token.
With the delta-net state and key/value reads at 286 positions it is 5.14 ms. Only changes that alter arithmetic (a
smaller output head, fewer weight bits) go below it, and those change output.

**Per-kernel ranking** (MEASURED, session e787e65d). Method: N extra copies of one kernel are inserted into the real
plan; the time per dispatch is the median step-time increase against three base runs, divided by N. Ordered by time per
step:

| Kernel class | Dispatches per step | us per dispatch | ms per step | Percent of 5.3 TB/s (weights) |
|---|---|---|---|---|
| FP8 matrix-vector 17408x5120 (MLP gate and up) | 128 | 49.62 | 6.352 | 33.9 |
| FP8 matrix-vector 5120x17408 (MLP down) | 64 | 65.93 | 4.22 | 25.5 |
| gated delta-net state update (v1) | 48 | 63.86 | 3.065 | n/a |
| FP8 matrix-vector 5120x6144 (delta-net out, attention o) | 64 | 24.45 | 1.565 | 24.3 |
| FP8 matrix-vector 10240x5120 (delta-net qkv) | 48 | 31.03 | 1.49 | 31.9 |
| RMSNorm 5120 (v1) | 129 | 10.22 | 1.319 | n/a |
| FP8 matrix-vector 6144x5120 (delta-net z) | 48 | 22.85 | 1.097 | 26.0 |
| attention partial, 131,072-position grid | 16 | 55.54 | 0.889 | n/a |
| BF16 output head 248320x5120 | 1 | 798.83 | 0.799 | 60.1 |
| FP8 matrix-vector 12288x5120 (attention q) | 16 | 36.04 | 0.577 | 32.9 |
| BF16 matrix-vector 48x5120 | 96 | 5.64 | 0.541 | 1.6 |
| all other classes (residual, SiLU, small delta-net, attention, argmax, embedding) | | | 0.397 or less each | |
| empty dispatch, 1 workgroup (control) | | 2.87 | | |

The classes sum to 23.612 ms against a 64-token base step of 26.973 ms; the rest (3.36 ms) is not attributed by this
method. Inserted copies run after the dependent kernel, with warmer caches, so these are lower bounds for the
in-place cost.

**What changed** (CPU-gated, seeded Omega objects, byte-checked against an LLVM 18 build of the same assembly):

- `rmsnorm5120` v2: 40 serial load round trips per wave become 2 waits plus a bounding wait. Gate: 7 cases, 3 mutants
  caught.
- `gdnstate_step` v2: 256 serial round trips per wave become 8-row chunks with the next chunk in flight. Gate PASS on 5
  cases (both state banks, zeros, a wide range with signed zeros and subnormals, NaN and infinities), relaxed-wait and
  removed-wait mutants caught.
- `cap286`: the plan is emitted for the 286 positions this request uses instead of 131,072, so the attention partial
  grid shrinks. This is a serving-envelope change: output-identical for this request, not a general replacement.

**Measured ladder** (session e787e65d, ONE run per candidate, each printed the published 256 ids). Each row is a single
run until the combined run lands with repeats. The two runs of the unchanged plan in this session (full-a and full-b) differ
by 0.15 ms, so differences between rows smaller than that are not resolved:

| Version | Decode, ms per token (MEASURED) | TB/s (estimate: modelled bytes / measured time) | Percent of 5.3 TB/s (estimate) |
|---|---|---|---|
| v0, full plan, first run (full-a) | 27.007 | 0.997 | 18.8 |
| v0, full plan, second run (full-b) | 27.157 | 0.991 | 18.7 |
| cap286 only | 26.723 | 1.007 | 19.0 |
| RMSNorm v2 only | 25.614 | 1.051 | 19.8 |
| state update v2 only | 22.845 | 1.178 | 22.2 |
| cap286 with both v2 kernels (v1all) | 20.968 | 1.284 | 24.2 |

The ladder gives 1.288 times against full-a and 1.295 against full-b in the same session. The lane's additive estimate for the two kernels without
cap286 was 21.45 ms (MODELED); that combination has not run alone. 20.968 ms is still 4.13 times the floor.

**Next by measured gap.** The FP8 matrix-vector kernels run at 24 to 34 percent of bandwidth in place and take most of the
step; the lane's reading of the code is that each wave reloads the activation chunk for every output row (four times the
weight traffic from cache). Dispatch count (1,126 per step, an empty dispatch costs 2.87 us) is next. Section 6 adds that
the whole step is about twice the isolated time of its matrix kernels, so both matter.

**Combined run (MEASURED, session 6e585e21, 14 Nov from 02:05 UTC).** Seven arms, each a single 256-token run in turn,
three rounds (`amd-kernel-speed/combined/card-2026-11-14T0206Z/`, script `card-combined.sh`, the same bytes as the
one prepared earlier). Objects by sha256: published launcher `08a50491`, plan3 launcher `de67f27e`, `rmsnorm5120` v2
`7d63972f`, `gdnstate_step` v2 `c689890d`, FP8 matrix-vector v2b1 `00686ea5` and v2b2 `21f4265d`. The card emitted the
full plan with the published sha256 (`2a292dad`) and the 286-position plan from the same script. The tarball's sha256
equals the value the card computed, and every file in its own checksum list matches.

| Arm | Launcher and objects | Plan | Run medians, ms per token | Median | Spread | TB/s (estimate: modelled bytes / measured time) | Percent of 5.3 TB/s (estimate) |
|---|---|---|---|---|---|---|---|
| A | published, v1 | full | 27.211, 27.164, 27.149 | 27.164 | 0.062 | 0.991 | 18.7 |
| B | plan3, v1 | full | 25.498, 25.425, 25.349 | 25.425 | 0.149 | 1.059 | 20.0 |
| C | plan3, RMSNorm v2, state v2 | full | 19.619, 19.623, 19.611 | 19.619 | 0.012 | 1.372 | 25.9 |
| D1 | C plus FP8 v2b1 | full | 17.056, 17.124, 17.127 | 17.124 | 0.071 | 1.572 | 29.7 |
| D2 | C plus FP8 v2b2 | full | 16.527, 16.517, 16.519 | 16.519 | 0.01 | 1.63 | 30.7 |
| E1 | D1 | 286 positions | 16.712, 16.797, 16.721 | 16.721 | 0.085 | 1.61 | 30.4 |
| E2 | D2 | 286 positions | 16.226, 16.151, 16.237 | 16.226 | 0.086 | 1.659 | 31.3 |

All 21 timed runs printed the 256 published ids (checked here against the published run, not only against each other);
the altered-weights control, plan3 with all v2 objects, differed at index 0.

Steps, each within this session: launcher 1.739 ms; the RMSNorm and state rewrites 5.806 ms (B to C); FP8 v2b1 2.495 ms
and v2b2 3.1 ms (C to D1, D2); the 286-position plan 0.403 ms on v2b1 and 0.293 ms on v2b2. Every per-arm spread is at
most 0.149 ms, so each of these steps is resolved; v2b2 is faster than v2b1 by 0.605 ms on the full plan. Overall A to E2
is 10.938 ms, 1.674 times; A to D2, without the request-specific plan, 1.644 times. E2 is 3.19 times the 5.08 ms floor.

**The FP8 candidates** were written by lane GpuBenchSync and taken over by AmdKernelSpeed. By the lane's write-up
(`amd-kernel-speed/RESULT.md`), v2b1 "adds a fast FP8 decode when no byte in a chunk is ±0, 0x7f or 0xff" (the special
values), and v2b2 "also issues two chunks of loads before one wait". CPU gates, rerun by AmdKernelSpeed on the Spark and
checked here from the logs (`amd-kernel-speed/takeover-gpubenchsync/`): `FP8GEMV-B128F-V2-GATE PASS` with 39 cases for
each, 5 of 5 mutants caught for v2b1 and 10 of 10 for v2b2, each log naming the object the card ran; and the repository's
exact-rounding check on real checkpoint rows, 27 cases and 8 controls, PASS for both. One limit, stated by the lane: no
wait checker that is sound for loops exists. The linear wait checker refuses both tables because of their loops and
branches, and finds no other error, but that is not a proof; the CPU executor completes loads at once and cannot see a
missing wait. So the wait counts of these two kernels are covered only by the card, where every run with them printed
the published ids (6 runs each: arms D1 and E1 for v2b1, D2 and E2 for v2b2).

## 6. 4-bit weights

State: kernels COMPLETE on the card (rerun, session c715b669): both 4-bit kernels bit-exact on all 10 shapes, two of 30
timings missing because two launches hung (section 13). Quality against the frozen bar: IN PROGRESS on the CPU,
result not in this report, so 4-bit is neither accepted nor rejected (section 1). Whole-model 4-bit decode on the card:
to be done when equipment is available.

**Format, Q4D8-1.** Symmetric signed int4 weights with one BF16 scale per group of at most 128 weights (32 for the
delta-net input projections); the activation becomes a 16-bit per-token fixed-point vector split into four signed base-16
digits, so the inner product is `V_DOT8C_I32_I4` (eight int4 products per instruction). gfx942 has no FP4 decode, which is
why an integer path. Container: 66 shards, 17.61 GB against 30.87 GB for the FP8 source, ratio 0.570.

**Bytes and floor (MODELED).** Candidate A (every FP8 linear in 4 bits, output head BF16) reads 15.54 GB per token, 0.577
of FP8, a floor of 2.93 ms at 5.3 TB/s. These are not output-identical to FP8: they need the quality bar.

**Kernels.** `q4gemv_d8` (one row per wave) and `q4gemv_d8r2` (two rows per wave), written as gfx942 code objects by
Omega's writer. CPU gate: 38 cases PASS, 7 of 7 mutants caught. The seeded gfx942 executor gained `V_DOT8C_I32_I4` and
`V_CVT_F32_I32` for this (local branch `fourbit/q4gemv`).

**Card (MEASURED, session c715b669, 14 Nov 01:46 to 01:59 UTC).** Each shape is tiled to at least 600 MB of 4-bit
weights per dispatch, more than the card's 256 MB Infinity Cache, and the FP8 baseline is tiled by the same count. Time
per dispatch is (median of three runs at 50 repeats minus median of three at 1 repeat) / 49, host clock, through the
published Omega launcher (no HIP in the path). The card converted its own copy of the three source shards it used and got
the same sha256 as the Spark container (3 of 3). The tarball's sha256 equals the value the card printed.

| Shape | q4gemv_d8, TB/s | q4gemv_d8r2, TB/s | v1 FP8 kernel on the same tensor, TB/s | Output words bit-equal to the oracle |
|---|---|---|---|---|
| delta-net qkv 10240x5120 | 2.72 | 3.06 | 2.12 | 204800 of 204800, both kernels |
| delta-net z 6144x5120 | 2.85 | 3.1 | 2.12 | 208896 of 208896, both |
| delta-net out 5120x6144 | hung, killed | 3.18 | 2.14 | 189440 of 189440, q4gemv_d8r2 (q4gemv_d8: in session f591aa44) |
| MLP gate 17408x5120 | 2.67 | 3.05 | hung, killed | 226304 of 226304, both |
| MLP up 17408x5120 | 2.68 | 2.98 | 2.15 | 226304 of 226304, both |
| MLP down 5120x17408 | 2.79 | 3.16 | 2.09 | 66560 of 66560, both |
| attention q 12288x5120 | 2.68 | 3.07 | 2.14 | 233472 of 233472, both |
| attention k 1024x5120 | 2.67 | 3.04 | 2.13 | 223232 of 223232, both |
| attention o 5120x6144 | 2.83 | 3.08 | 2.13 | 189440 of 189440, both |
| output head 248320x5120 | 2.67 | 3.02 | BF16 kernel 3.29 | 248320 of 248320, both |

In the rerun 19 kernel and shape results ran and all 19 are bit-exact; the one `q4gemv_d8` result missing there (delta-net
out) was bit-exact in the earlier session f591aa44, so each of the 10 shapes has a bit-exact result for both kernels.
Ranges: `q4gemv_d8r2` 2.98 to 3.18 TB/s (56.3 to 60.0 percent of 5.3 TB/s, 10 shapes), `q4gemv_d8` 2.67 to 2.85 (50.4 to
53.7 percent, 9 shapes), the v1 FP8 kernel 2.09 to 2.15 (39.5 to 40.6 percent, 8 shapes). The earlier session had read
2.79 to 2.87 and 3.08 to 3.28 on its five shapes; on the 9 kernel and shape pairs timed in both sessions the rerun read
-1.3 to 8.6 percent lower (median 0.1 percent), a session-to-session difference on the same kernels. The rerun is the
record.

**Per-token sums (MODELED from MEASURED per-tensor times).** Adding the isolated per-tensor times of every FP8 linear and the head,
weighted by how many times each runs per token:

| Linears and output head | ms per token (MODELED) |
|---|---|
| v1 FP8 kernel, BF16 head | 12.2 |
| `q4gemv_d8r2` linears, BF16 head | 4.99 |
| `q4gemv_d8r2` linears, 4-bit head | 4.44 |

The 4-bit sum is 2.75 times smaller than the v1 FP8 one. Against its own floor, 2.93 ms per token for candidate A's bytes
(which keep the head in BF16) at 5.3 TB/s, the 4-bit sum with the same BF16 head is 1.70 times; that it is below the 5.08 ms FP8 bound only reflects reading fewer bytes. v2b2, the fastest
FP8 kernel in section 5, was not timed by this method, so the 2.75 ratio is against the older kernel. These sums leave out the
launch gaps between dispatches and every kernel that is not a matrix product (delta-net state update, attention, norms,
residual, SiLU, argmax), and assume back-to-back dispatches behave like the tiled measurement. Two inputs are filled, not
measured: the FP8 time of MLP gate uses MLP up's (same shape; its own launch hung), and the value projection uses the key
projection's (same shape, not run). They are not a decode time; no 4-bit model step has run.

**Finding: the whole step is about twice its matrix kernels.** Against the isolated FP8 sum of 12.2 ms, the measured
whole decode step of section 5 is 2.21 times larger for the full plan (27.007 ms) and 1.72 times for v1all (20.968 ms).
Inside the plan, the inserted-copy method of section 5 puts the same FP8 matrix-vector classes and the BF16 head at
16.36 ms per step, so the matrix kernels themselves run slower in the plan than isolated, and the rest of the step is the
non-matrix kernels, launch and host time. The isolated sums and the plan measurements come from different sessions and
methods, so this is a ratio of estimates, not a decomposition of one step. It means that faster weight kernels alone,
4-bit or not, cannot bring the step near its floor: the non-matrix kernels and the gaps between dispatches must shrink too.

**Quality, IN PROGRESS (CPU; result not in this report).** The bar was frozen before any 4-bit result existed (sha256 `3ac61cf9...`): perplexity ratio at most
1.03 on WikiText-2, top-1 agreement with FP8 at least 94 percent over 2,326 positions and 95 percent on the generated
positions, mean KL at most 0.05 nats, ARC-100 loss at most 3 items. The CPU arms `fp8` and `q4` had not finished on
the Spark when this report was written. If `q4` fails, candidate B keeps the delta-net input projections in FP8.

## 7. Several tokens per pass

State: the cost of extra rows MEASURED for the FP8 kernel on the card; the speculative step on the card to be done when
equipment is available. Design, CPU gates and acceptance measurement, varied prompts included, COMPLETE.

**Idea.** The checkpoint ships a one-layer multi-token-prediction head. Each pass feeds the last token and k drafts
through the 64 layers together, keeps the longest prefix of drafts equal to the model's own greedy choice, and emits
those plus one more. Output equals plain greedy by construction: every row's arithmetic is bit-identical to the one-row
kernels.

**Acceptance (MEASURED on a CPU float32 reference, lighthouse request, published AMD ids):**

| Drafter | k = 1 | k = 2 | k = 3 | best |
|---|---|---|---|---|
| model's head, full BF16 output | 1.718 | 2.151 | 2.37 | 2.462 |
| model's head, 32,768-id shortlist | 1.62 | 2.016 | 2.169 | 2.246 |

Tokens per pass. Acceptance stops growing after about three drafts. On six varied prompts (code, a word problem, a list
of facts, a translation, a short story, JSON; 96 tokens each, free-run on the same CPU reference) the head gives 1.71
to 1.96 tokens per pass at k = 1 (MEASURED, `spec-decode/acceptance/accept-mtp-freerun.json`).

**Gated on the CPU (seeded Omega and the seeded gfx942 executor, hardware NOT_RUN):** the pass decision (304,470 cases,
5 of 5 mutants caught; `spec-decode/om/gate-spec-results.json`); a three-state model proving speculative output equals
greedy for k = 1 to 8 (576 runs, 8 of 8 mutants caught; `spec-decode/om/gate-loop-results.json`); R-row FP8 and BF16
matrix-vector kernels that load weights once and match the one-row kernels bit for bit (`s2-run6`); ring-buffered
convolution and state kernels (`s2b-conv-run3`, `s2b-run4`); batched map kernels (`s2c-run2`, `s2d-run2`); an on-card
accept kernel (`s3a-run1`). Each named run is a `report.json` with verdict PASS under `spec-decode/kernels/`.

**What it would give (MODELED).** With today's kernels the per-row work that is not a matrix-vector product is 5.33 ms
per row after the batching built, 3.1 ms of it the state update. The lane's model puts the best case at 18.3 ms per token
(extra rows free in the matrix-vector kernels) and 22.8 ms if each extra row costs 30 percent more there, against its
26.309 ms anchor. The cost of extra rows on the card is the first measurement needed. A run in session 3a2e29e5 measured
the FP8 kernel (MEASURED, R-row variant of `fp8gemv_b128f` on an 81920x5120 matrix built by the timing script, not
checkpoint weights, median per dispatch of five trials, host clock; rho is the extra time per added row as a fraction of the one-row time, `(t_R / t_1 - 1) / (R - 1)`):

| Rows R | us per dispatch | rho | Each row's output against the one-row kernel |
|---|---|---|---|
| 1 | 198.592 | | |
| 2 | 260.8 | 0.3132 | bit-identical |
| 3 | 345.157 | 0.369 | bit-identical |
| 8 | 739.862 | 0.3894 | bit-identical |

The BF16 output head ran at R = 1 only (798.018 us); its R > 1 rows and the whole speculative step (S4) are NOT_RUN. With
this measured rho applied to every matrix-vector kernel, the lane's model (MODELED, `spec-decode/model/wolfram.md` Q10 and
Q11) gives 22.922 ms per token at k = 1 against its 26.309 ms anchor, 1.148 times faster, and 25.472 ms at k = 2: more
drafts cost more than they gain, because each extra row repeats the per-row work that is not a matrix-vector product.
Over the six varied prompts the same model gives 1.14 to 1.31 times faster at k = 1 (MODELED, Q12; the essay slowest,
code fastest).
These are unreviewed measurements from one session, and the model's anchor is the v1 kernels, not the faster combined
arms of section 5. Since this section was first written the lane has built and CPU-gated: on local branch
`spec-decode2` (unreviewed), the multi-token host loop in the launcher (312 checks, 0 failures at commit `f859c88f`;
`raw/amd-report/extracted/spark-logs-20261114/hostloop-gate-final-20261114T022658Z.log`); and, in gate reports that name
no branch, the per-row position and
draft hand-over kernels (`spec-decode/kernels/s3c-pos-run1`, `s3c-select-run1`) and a whole-pass gate in which the
emitted R = 2, R = 3 and head-drafter plans give the plain plan's ids on two prompts with all 8 required wiring mutants
caught, 48 rows of which 47 pass (`spec-decode/s3c/run3/report.json` and `run3.log`). One further, non-required mutant
(accept-after-mismatch in `spec_accept_r3`) is not caught by this whole-pass gate on either prompt: `run3.log` line 105
labels it "caught" in error, with `caught_by` false for both prompts. The accept kernel's own gate catches it (76 of 400
cases fail, `spec-decode/kernels/s3a-run1/report.json`). The R-row plan emitter (it reproduces the published plan
byte for byte at R = 1) is quoted from `spec-decode/RESULT.md`, a lane note not in this folder. Drafts 2 to k from the
head are not built.

## 8. Eight cards

State: design only. Hardware NOT_RUN by the author's decision of 14 November: the AMD effort is one MI300X, as fast as it can
go, and no money is spent on a multi-card machine.

What exists: a design that splits each layer across 2, 4 or 8 cards by output rows ("exact" mode, bit-identical to one
card) or by the Megatron column-then-row split ("fast" mode, changes summation order); a KFD topology scan, peer-memory
mapping and exchange protocol in seeded Omega; and a host gate, `MCGATE PASS cases=203`, on Linux AArch64 and on x86-64
under QEMU (emulated, not native). Nothing has opened a real multi-card `/dev/kfd`. The model puts the 8-bit weight
floor on 8 cards at 0.639 ms per token before exchange costs. The lane's own model also says that once per-card kernels
reach 80 percent of peak bandwidth, a host-mediated exchange at the measured 29 us round trip makes 2, 4 or 8 cards
slower than one in exact mode. This section stays as design. Files: `multi-card/DESIGN.md` and its Wolfram queries
`multi-card/model/WOLFRAM.md`, whose line 7 points to `ACCESS.md`, a lane note not in this folder; the 5.3 TB/s cited
there is AMD's figure, as quoted in `BRIEF.md` line 75.

## 9. Correctness

State: COMPLETE for every run in this report.

- **Same 256 ids.** Every timed run in sections 3, 4 and 5 printed exactly the 256 token ids of the published run (the
  recompute scripts compare all 256 for every run: 5 published, 5 repeat, all 24 non-control runs of launcher run 1 and
  all 20 of run 2 including warmups, every 256-token kernel candidate, all 21 timed runs of the combined session, and all
  34 non-control model runs of launcher run 3 including warmups) and the 64-token kernel-timing runs printed the first 64
  of them.
- **Altered-weights controls.** A plan with deliberately altered weights differs from the published ids at index 0: in
  the repeat session, for the old and the new launcher in launcher run 1, for plan3 in runs 2 and 3, in the kernel
  session, and for plan3 with all v2 objects in the combined session. So an identical-id result is not something any plan
  would produce.
- **HIP replay.** In the repeat session and in launcher runs 1 to 3 the same kernels replayed by AMD's HIP runtime gave
  the same 256 ids. That checks
  the launcher, not the model arithmetic.
- **CPU gates.** Launcher: 308 checks PASS at commit `d957526e` (`amd-launch-gap/build-r14-spark-gate.log`). Kernels: the v2 RMSNorm and state gates PASS with mutants
  caught; FP8 v2b1 and v2b2: 39 cases PASS each, 5 and 10 mutants caught, wait counts not checkable on the CPU (section 5);
  4-bit: 38 cases PASS, 7 of 7 mutants caught; several tokens per pass: the gates in section 7; eight cards:
  203 cases PASS.
- **4-bit kernels on the card.** Every output word of both 4-bit kernels equals the exact oracle on all 10 shapes, and a
  repeated launch gives the same output (section 6).
- **Not claimed.** Agreement with a different implementation or machine. On the same card, same checkpoint and same
  session, Omega's ids and PyTorch's are equal for the first 29 generated tokens and differ from index 29, a near tie
  (top two 0.125 apart on the PyTorch side). The published brief records that the AMD and Spark outputs first differ at
  generated index 109, also a near tie. These are read as rounding differences between implementations; that is not
  proven.

## 10. What this does not show

- **A combined speed beyond three runs in one session.** The headline is three interleaved runs per arm on one card in one
  session. The FP8 v2b kernels' wait counts have no sound CPU check; only the card runs cover them.
- **Why Omega still trails HIP by 0.08 ms per token.** Paired run 3 shows the gap and rules out the launch path; the
  cause inside kernel execution is an inference by elimination; run 4, which would test it, is to be done when
  equipment is available.
- **Serving.** One request at a time, one card, a command-line program. No batching, no HTTP server.
- **A tuned AMD baseline.** No vLLM or SGLang ran on the card; the HIP row runs Omega's own kernels.
- **Long contexts.** The request is 30 prompt tokens and 256 generated. `cap286` is sized for this request.
- **4-bit quality and speed.** Quality is IN PROGRESS on the CPU and not in this report; no 4-bit model output exists. The 4.44 ms per token is a sum of
  isolated kernel times, not a decode measurement.
- **Exclusive hardware.** The card is a virtual function on a shared host. Timing noise between sessions is visible
  (section 1).
- **Bandwidth measured by counters.** Percentages are modelled bytes over measured time.
- **Review.** None of the branches behind sections 4 to 8 has had its final review; the author ordered one review at the
  end. Three end reviews were made of earlier versions of this report (section 14).
  Nothing is landed.

## 11. How to repeat

- **Where to run the card steps.** On a machine you control, or on a shared host only with the host operator's
  agreement. The launcher that opens only the node it is given (section 4, node selection; `8b6005e8`, commit
  `94af5a38`) is CPU-gated only and has never run on a card. Every card run whose logs record a launcher hash used one
  of the launchers in the table below or, in the two earlier qualification sessions, the launchers named in section 12.
- **Check the numbers (no GPU).** In this folder: `sh recompute/run-all.sh --verify-sums`. It recomputes the
  MEASURED, DERIVED and COUNT values from the raw logs under `raw/`, rebuilds `numbers.json` and the session tables,
  runs the checks described in section 2 and their self-test, builds a second time to prove the output does not change,
  and then requires `SHA256SUMS` to verify. It exits 0 only if all of that passes. Python 3 standard library only; it
  leaves every file in `SHA256SUMS` byte-identical.
- **The published run.** The brief's three levels at CUDApp `dc536882` (Level 1 rechecks the published evidence, Level 2
  rebuilds the launcher and 18 kernels byte for byte on arm64 Linux, Level 3 runs on an MI300X).
- **Launcher modes.** On the `amd-launch-gap` branch, build as Level 2 does, then run the plan's command file with
  `omega-launch plan2` (or `plan3`) in place of `omega-launch plan`. The card scripts that ran are under
  `raw/amd-launch-gap/card/final/` (`gap-run.sh`, `cmd-*.sh`, `gap-verdict.py`).
- **Kernel ladder and combined run.** `raw/amd-kernel-speed/card/261114001549-e787e65d/out/card-measure.sh` and that `out/` hold
  the one-run ladder; `raw/amd-kernel-speed/combined/card-2026-11-14T0206Z/out/` holds the combined run's script
  (`card-combined.sh`), object hashes, plans' emission log and per-run logs.
- **Branches.** Local only today (not landed): CUDApp `amd-launch-gap` (latest commit with a gate log in this folder
  `94af5a38`), the kernel lane's v2 objects, `fourbit/q4gemv`, `spec-decode` `48d68c6d` and `spec-decode2` `f859c88f`.
  Whether this report and they are published, and when, is the author's decision.

Each x86 launcher (sha256 prefix), the commit it was built from and the card runs that used it:

| Launcher | Commit | Where this folder ties them | Card runs that used it |
|---|---|---|---|
| `08a50491` | CUDApp `dc536882` (published) | `amd-launch-gap/src/CUDApp-base/evidence/` | published run 76e03a28; repeat 46d1b9c5; the plan arm of runs 1 to 3; arm A of the combined run 6e585e21; kernel sessions 7605956f and e787e65d; 4-bit rerun c715b669; extra-row session 3a2e29e5 |
| `78e09581` | not recorded in this folder | `amd-launch-gap/card/final/logs/gap-run.sh` line 14, `run.log` line 204 | run 1, 52ef268c (plan2, planscope, planstage) |
| `de67f27e` | `9076c537`, squashed as `139bb6a9` | `amd-kernel-speed/combined/card-2026-11-14T0206Z/README.md` lines 7 to 8 | run 2, e8cddcb1 (plan2, plan3); arms B to E2 of the combined run 6e585e21 |
| `a6aee5ef` | `d957526e` | `amd-launch-gap/HANG-20261114.md` line 20; `amd-launch-gap/build-r14-spark-gate.log` line 317 | run 3, 7f66e322 (plan3, plan3c, plan3t) |
| `657db853` | not recorded in this folder | `amd-launch-gap/build-r15-spark-gate.log` line 317; `amd-launch-gap/card/bundle4/gap-run4.sh` line 11 | none: run 4 is prepared |
| `8b6005e8` | `94af5a38` | `amd-launch-gap/build-r16-spark-gate-94af5a38.log` (file name, line 327) | none: node selection, CPU gate only |

## 12. Card sessions

Every MI300X session behind this report. The UTC start is encoded in the session name. Hashes are of the evidence file
as kept; where a lane recorded the card-side hash of a tarball, the two are equal.

<!-- sessions:begin (generated by recompute/render_sessions.py) -->
| Session | UTC start | Lane | What ran | State |
|---|---|---|---|---|
| bb2a389c | 2026-11-13T07:22:40Z | earlier qualification (Main) | launcher 373e76cd, warmup + 5 lighthouse runs | COMPLETE |
| 5a0c78a1 | 2026-11-13T08:00:38Z | earlier qualification (Main) | launcher f646ea5d, warmup + 5 lighthouse runs | COMPLETE |
| 76e03a28 | 2026-11-13T10:10:13Z | published run (Main) | launcher 08a50491, warmup + 5 lighthouse runs, controls | COMPLETE |
| 46d1b9c5 | 2026-11-13T18:50:25Z | UXL brief repeat (Main) | public-clone repeat, HIP graph, PyTorch FP8, stock BF16, control | COMPLETE |
| 7605956f | 2026-11-13T23:27:37Z | GpuBenchSync | per-kernel timing, first build of the cells | COMPLETE |
| e787e65d | 2026-11-14T00:15:49Z | AmdKernelSpeed | per-kernel timing, v2 kernels, cap286, v1all ladder | COMPLETE |
| 52ef268c | 2026-11-14T00:50:45Z | AmdLaunchGap | run 1: plan, plan2, planscope, planstage, HIP graph, controls | COMPLETE |
| f591aa44 | 2026-11-14T01:18:36Z | FourBit | q4gemv_d8 and q4gemv_d8r2 bit checks and timing; ended early at about 01:26Z (section 13) | PARTIAL: 5 of 10 shapes |
| e8cddcb1 | 2026-11-14T01:26:02Z | AmdLaunchGap | run 2: plan, plan3, HIP graph, plan3 control, plan2 | COMPLETE |
| c715b669 | 2026-11-14T01:45:16Z | FourBit | rerun: q4gemv_d8, q4gemv_d8r2 and FP8/BF16 baseline on 10 shapes, two launches hung and were killed | COMPLETE for the 4-bit rerun, 2 timings missing |
| 6e585e21 | 2026-11-14T02:05:44Z | AmdKernelSpeed | combined run: 7 arms (launcher, v2 kernels, FP8 v2b1/v2b2, cap286) x 3 interleaved runs, control | COMPLETE |
| 7f66e322 | 2026-11-14T02:23:48Z | AmdLaunchGap | run 3: launch micro-benchmark; 9 paired cycles plan3, plan3c, HIP; plan3t phases; published launcher; control | COMPLETE |
| 3a2e29e5 | 2026-11-14T02:52:37Z | SpecDecode2 | extra-row timing of the R-row kernels (fp8 R=1,2,3,8; bf16 R=1) | PARTIAL: BF16 rows at R > 1 and the speculative step not reached |

Evidence file and hash for each session (full session name; paths under `raw/`):

| Session | Evidence | sha256 |
|---|---|---|
| `jupyter-user-261113072240-bb2a389c` | `amd-launch-gap/src/CUDApp-base/evidence/amd-qwen38-native-final-20261113/host/qualification.json` | `a836b11106efb5060fdae656996fb98caaf89445efea65a89fef345b64d68b29` |
| `jupyter-user-261113080038-5a0c78a1` | `amd-launch-gap/src/CUDApp-base/evidence/amd-qwen38-native-compatible-20261113/host/qualification.json` | `e35dead2c6ec31829176815dc5b3a962357563521b467a3fd4dbcae84011b4e0` |
| `jupyter-user-261113101013-76e03a28` | `amd-launch-gap/src/CUDApp-base/evidence/amd-qwen38-native-refusal-20261113/host/qualification.json` | `57b382fe3943be5be7af42838c5aa8493611b1cd0dac931502409b39b856ab7c` |
| `jupyter-user-261113185025-46d1b9c5` | `card-evidence/uxl-evidence.tar.gz` | `94940b02854c2c9c07a41fe27d3a7107940f284d3f099b6ba6533b42104a7ac7` |
| `jupyter-user-261113232737-7605956f` | `amd-kernel-speed/card/GpuBenchSync-20261113T232737Z/aks-results.tar.gz` | `d6502d8957495f25a0ae56ae2f74a020903c733295a9d33d45e3501e47eb9be6` |
| `jupyter-user-261114001549-e787e65d` | `amd-kernel-speed/card/261114001549-e787e65d/aks-results.tar.gz` | `9894a97b9ddaa78c688aace5ce2d16814f435cadad0dbf43df6b77da3285baf2` |
| `jupyter-user-261114005045-52ef268c` | `amd-launch-gap/card/gap-evidence.tar.gz` | `3af4ee59322e06773d26cb1caace5ea85628a7c473d3a394e6ec541d5e9000b0` |
| `jupyter-user-261114011836-f591aa44` | `four-bit/evidence/card-partial-20261114/card_q4.log.partial` | `dbb33379bef1ac6c7267e7d043d832c3e395231fab3d6b0376157143377c795d` |
| `jupyter-user-261114012602-e8cddcb1` | `amd-launch-gap/card/run2/gap-evidence.tar.gz` | `8471a458c97f4339e4ff0b5b20449ac124cf91db0dfc732aaae4754b421a73ec` |
| `jupyter-user-261114014516-c715b669` | `four-bit/evidence/card-20261114/q4card.tar.gz` | `24f6cc267888bb2e175acd51b18419bfa13ad071f8056b7b270ed5398e206c01` |
| `jupyter-user-261114020544-6e585e21` | `amd-kernel-speed/combined/card-2026-11-14T0206Z/akc-results.tar.gz` | `493d6c29b79c7474659a9f69f877dcd8b597ffbb95a03b8230c5d21d57da9cee` |
| `jupyter-user-261114022348-7f66e322` | `amd-launch-gap/card/run3/gap-evidence.tar.gz` | `de086a5e56e56ce949615e9ff6158e7a327d1f4c798264a8d75df23c125e9af3` |
| `jupyter-user-261114025237-3a2e29e5` | `spec-decode/card-s4/results/jupyter-user-261114025237-3a2e29e5/rho-results.json` | `a136ad815610edacf15a6694dcf833e6dd20a59bd18c5b509f8d08c0da7c8c51` |
<!-- sessions:end -->

## 13. Runs with missing results

State: facts from the raw logs. None of these changes a number in this report; each missing result is named where it
would have appeared.

- **4-bit session f591aa44 ended early (14 November, about 01:26 UTC).** Another lane replaced the session by mistake,
  four minutes into FourBit's run, before its results were downloaded. The only record is the log text read through the
  notebook's contents API at 01:24:40 UTC, with each line cut at 900 characters: 5 of the 10 planned shapes survive with
  timings and bit checks. The rerun, session c715b669 (14 November from 01:45 UTC), recovered all of the rest except two
  timings (next item). The FourBit script that ran in both sessions is the one in bundle `10c93d3d`
  (`raw/amd-report/extracted/four-bit-bundle-10c93d3d/card_q4.py`, hash-checked against the bundle).
- **Two launches hung in the 4-bit rerun (launcher defect: where it waits is located, why is not established).** Of 175
  single-kernel launches of the published launcher `08a50491` in session c715b669, 173 completed and 2 never finished.
  Their captured output (`four-bit/evidence/card-20261114/q4card.tar.gz`) shows:
  - Hang 1, `q4gemv_d8` on delta-net out at 1 repeat: the dispatch was written and the launcher entered its wait for the
    completion signal, which never reached zero; it spun for 7 minutes until SIGTERM.
  - Hang 2, the FP8 baseline on MLP gate: the third launch at 50 repeats. Its output stops before the launcher reports the
    dispatch written, and it spun for 1 minute 52 seconds until killed (`hang-2.txt`: `/dev/kfd` and the render node open).
    Five launches of the same kernel and shape (three at 1 repeat, two at 50) had completed just before it.
  - Both printed the 10 s slow-wait alarm; no completed launch did.

  What is established (AmdLaunchGap, `amd-launch-gap/HANG-20261114.md` with its dated correction, checked against those
  logs): the caller passed no deadline (`wait_timeout_ns 0`), and with no deadline the launcher's waits spin forever by
  their contract, which explains the launcher's 100 percent CPU use and the hang. From the logs and the launcher source, hang 1 was in the
  completion wait and hang 2 in the wait for ring space (its 16-slot ring must wrap for 50 packets), so in both the card's
  packet processor stopped consuming or completing packets on that queue. What is not established: why it stopped, and in
  hang 2 whether it stopped part-way or never started. Fix, IMPLEMENTED and CPU-gated, never run on a card that hung:
  from commit `d957526e` the 10 s alarm prints the queue's read and write pointers and the signal value, in both
  waits and both launch paths, and callers are told to pass a positive deadline so a stuck wait exits 75 with
  diagnostics. Launcher run 3 used that launcher for 64 processes and none hung, so the diagnostics never fired.

  Effect on timings: none on the numbers in this report. A launch prints a timing only after its completion signal, so a
  hung launch gives none and is visibly incomplete, as these two were (they are the two missing timings of section 6). All
  71 launcher plan processes behind sections 4 and 5's earlier sessions printed their ids line and none raised the alarm,
  nor did any of the 22 in the combined session or the 67 in launcher run 3. A completed launch never waited 10 s;
  shorter stalls inside a completed launch are not excluded by this check.
- **Kernel timing, attempt 1 timed nothing.** In session e787e65d every first-attempt run exited 74 because the measuring
  script passed an unexpanded `$AMDSERVE_WEIGHTS` to the launcher. The fixed script expands it and refuses to start on a
  missing input; attempt 2 ran all 36 runs with exit 0. GpuBenchSync's earlier session 7605956f hit the same defect and
  fixed it separately.
- **Session-to-session drift.** The unchanged published launcher and kernels on the full plan measured 26.726, 26.721,
  26.490, 26.641, 27.007, 27.164 and 27.051 ms per token in seven sessions. Every comparison in this report is within one
  session.

## 14. Review

State: three end reviews were made of earlier versions of this report, each by a fresh agent with no memory or
past-session tools, given only the review packet. Their findings are applied, except those about material the author
chose to leave out of this publication. Reviewers: AI models; the first and third were Claude models made by Anthropic
(model ids `claude-fable-5-2` and `claude-opus-5-5`), the same model family as the agents that wrote this report. The
first reported that every headline number reproduces from the raw card logs, both by the packet's recompute and by its
own independent script; the third, that every number reproduces byte for byte from the raw logs. None of the branches
behind sections 4 to 8 has had its final review, and nothing is landed.
