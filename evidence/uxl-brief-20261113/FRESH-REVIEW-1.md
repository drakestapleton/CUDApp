# Fresh review of BRIEF.md ("Omega running Qwen3.8-27B on an AMD Instinct MI300X")

Reviewed 13 November 2026 on the author's Mac (arm64, macOS).

**Reviewer.** I am Claude, made by Anthropic; the harness reports the model id as `anthropic/claude-fable-5-2`. I used no memory, recall, archive or past-session tool, read nothing under `~/handoffs/` or any other notes folder, and made no ssh connection. The harness preloads some session notes into my instructions; I did not rely on them, and every finding below cites a file I opened or a command I ran in this session. Inputs: the two packet files, the three public git repositories the brief names, and public vendor pages.

**Verdict in one line:** SEND AFTER FIXES (findings 1 to 6). Details at the end.

## 0. Packet identity

| File | Expected sha256 | Result |
|---|---|---|
| `BRIEF.md` | `fab8275e…ab541cc` | OK (`shasum -a 256 -c`) |
| `uxl-brief-evidence-20261113.tar.gz` | `880ee84b…1dbc5c1` | OK |

Archive extracted to `work/uxl-brief-evidence-20261113/` (135 files). Top-level `SHA256SUMS`: 132 entries, all OK.

## 1. Findings

Severity: **blocker** (do not send until fixed), **should fix**, **minor**. There are no blockers.

### Should fix

**1. The archive's checksum files do not match what the brief says about them.**
- Text: section 7, "They are in the archive sent with this brief … each folder with its own `SHA256SUMS`."
- Evidence: `spark-20261113T1913Z/` has no `SHA256SUMS`. In `amd-repeat-20261113T1907Z/`, `shasum -a 256 -c SHA256SUMS` reports `launch-x86: FAILED open or read` and `vadd.hsaco: FAILED open or read` (43 entries, 2 missing: the two files live in `level2-rebuild/`). `level2-rebuild/SHA256SUMS` passes (30 of 30). The top-level file passes but does not cover the two nested `SHA256SUMS` files.
- Why it matters: this is the first command a careful reader runs on the archive, and one folder answers FAILED.
- Fix: either drop the two lines from `amd-repeat-20261113T1907Z/SHA256SUMS` or copy the two files into that folder, and add a `SHA256SUMS` to the Spark folder. Or change the sentence to: "The archive's top-level `SHA256SUMS` lists every file."

**2. `level2-rebuild/README.md` describes an older draft and says the brief's commands fail.**
- Text in the archive: "Brief tested: `../BRIEF.md`, sha256 `5f94d683…` (not edited)"; "**The brief's Level 2 block fails if pasted after Level 1 in the same shell**"; "**`qemu-x86_64` is required, not optional**"; "**`sh seed/challenge/run.sh` is the wrong command for this brief**"; a section headed "Corrections for the brief"; `logs/runA-literal.log` with exit codes 2, 64, 1.
- Text in the brief: section 6, "Every level below has been run as printed."
- Evidence: the brief being sent has sha256 `fab8275e…`, not `5f94d683…`. All three corrections are already in it (the `cd ..` line, the `qemu-x86_64` requirement, `sh seed/omega-from-seed.sh`). I pasted the current Level 1 and Level 2 blocks literally into one shell on a fresh arm64 Ubuntu 24.04 VM and every command exited 0 (section 3). So the brief is right and the archive note is stale, but the reader cannot know that.
- Fix: put two lines at the top of `level2-rebuild/README.md`: "This test was run against an earlier draft (sha256 `5f94d683…`). The three corrections it lists are in the brief you received; run B below is the Level 2 block as now printed." Or delete the "Run A" and "Corrections for the brief" sections and `logs/runA-literal.*` from the copy that is sent.

**3. "the other tenants' devices refuse access" reads as if you tried to get into other customers' GPUs on AMD's service.**
- Text: section 1, "Not established: That I had the physical host to myself. The host shows 64 GPU devices, and the other tenants' devices refuse access."
- Evidence: the launcher log (`amd-repeat-20261113T1907Z/uxl-native-run1.txt`, first lines) shows `OMEGA step topology_node fail errno=1` for nodes 7, 5, 3, 8, then `topology selected node=6`. `uxl-env.sh` line 5 loops over every `/sys/class/kfd/kfd/topology/nodes/*`. It is a read of sysfs properties, which is harmless, but the sentence does not say so and the reader may forward it to AMD.
- Fix: "The container's `/dev/dri` lists 64 cards. The KFD topology shows several GPU nodes; only one is readable from my container, and my launcher uses that one and skips the others (reading their sysfs properties returns EPERM). I did not try to open any other device."
- Related, could not check: whether AMD's terms for the One-Click notebook allow publishing timings of the service and of its stock cell. Worth one look before sending to a foundation where AMD is a member.

**4. The archive carries working notes about your own machines and how the runs were driven. Nothing secret, but decide deliberately.** See section 5 for the list. The two that a reader will actually notice:
- `amd-repeat-20261113T1907Z/README.md` line 8: "Driven from the Mac through Drake's Chrome with the `computer` accessibility helpers: text pasted into a notebook cell …".
- `level2-rebuild/README.md` and `logs/`: "Lane `uxl-brief/PublicRebuild`, Drake session on the Mac", "hive locks", "Main confirmed it stays NOT_RUN", `logs/board-lines.txt`, `logs/pool.sh` with `/home/drakestapleton/handoffs/hive/queen`, "(`ssh spark`)", "no port 18091".
- Fix: either trim these (they are not evidence for any number) or keep them and add one sentence to the brief; see finding 5.

**5. The brief does not say who or what did the work, and the archive and repositories show it.**
- Text: the brief is first person throughout ("I measured it twice", "I followed only the public instructions").
- Evidence: the archive lines quoted in finding 4; `AGENTS.md` at the root of the Core and CUDApp clones and `AGENT_CODE_OF_CONDUCT.md` in Core; the Spark process list in `spark-20261113T1913Z/measurements.json` (`codex`, `omp`, `hermes`). I cannot tell from the packet how the work was divided, so I am not asserting it. But the reader asked for a technical brief, will open these files, and will then wonder which statements are the author's own checks.
- Fix: one plain sentence in section 2 or section 6, in whatever form is true, for example: "I work with AI coding agents. They wrote much of the test tooling and ran the measurement scripts under my direction; the commands and logs in the archive are exactly what ran."

**6. Level 3 needs two files that the brief says come only from Level 2, which needs an arm64 Linux machine. They are already in the archive.**
- Text: section 6, "Copy `launch-x86` and `vadd.hsaco` from the Level 2 output folder (`build/amd-kfd-launch/gate/`). They are the only two files that do not come from the public clone."
- Evidence: `level2-rebuild/launch-x86` (sha256 `08a50491…91e1`) and `level2-rebuild/vadd.hsaco` (sha256 `10eb12e0…e55f`) are in the archive, and both equal my own rebuild.
- Why it matters: someone at AMD with an MI300X and an x86 workstation would stop here.
- Fix: add "If you have no arm64 Linux machine, the same two files are in the archive under `level2-rebuild/`; check them against the hashes above."

**7. Four statements about the Spark's conditions cannot be checked from anything supplied or public.**
- Text: section 4, "one-minute load 10.5 to 16.3, against 3.4 to 4.2 on 1 November" and "two small GPU processes were resident that were not there on 1 November".
- Evidence: the public 1 November record (`Core@5c95594a`, `docs/qwen38/qualification-20261101/short/events.jsonl`) holds one load sample, `3.71 5.56 6.29`, and no GPU process list. The 13 November samples in `measurements.json` run from 9.98 (at ready) to 18.69 (after the stream); 10.50 to 16.34 is the span over the warmup and timed requests only. The two GPU processes on 13 November are there (pids 2744 and 3115, 335 MiB each).
- Also in the same log: the 24-token correctness request at 19:14 UTC ran at "graph chip 125.5 ms/token" (`run/server.log`), the same as 1 November, minutes before the 256-token runs at 135.2 to 135.9. That is useful to say, since it supports contention rather than a slower build.
- Fix: "one-minute load 10.5 to 16.3 during the warmup and timed requests (3.7 at the start of the 1 November run, the one sample in that record)", and either drop "that were not there on 1 November" or add the 1 November process list to the archive.

**8. The two speed ratios against PyTorch are stated first and withdrawn after.**
- Text: section 4, "My path was 4.9 times faster than the plain PyTorch loop on the same checkpoint, and 2.6 times faster than the stock notebook cell." Section 7: "The two PyTorch rows in section 4 are AMD's software as the notebook ships it".
- Evidence: the arithmetic holds (131.011 / 26.721 = 4.903, 70.628 / 26.721 = 2.643). But `uxl-reference-fresh.txt` shows the FP8 row ran a Hugging Face community kernel package fetched at run time ("version 2 of `kernels-community/finegrained-fp8`, but version 4 is available"), which fell back to Triton because "DeepGEMM on SM94 requires CUDA runtime ≥ 12.3, found 0.0", with the transformers fast path missing. That is not AMD software as shipped. The stock row is `uxl-stock.py`, your script around the notebook's pipeline call, with BF16 weights, 70 prompt tokens and thinking on (`uxl-stock.json`, `text_head` starts "We need to respond to user").
- A reader at AMD will call a printed "4.9 times" against a fallback path apples to oranges even with the caveat two bullets later.
- Fix for section 4: "Neither PyTorch row is a speed baseline. The FP8 row is a correctness reference running a community FP8 kernel package in its Triton fallback; the stock row runs different weights (BF16), a 70-token prompt and thinking on. For the record, they were 4.9 and 2.6 times slower per token than my path. I did not run vLLM or SGLang on the card, so this says nothing about the best AMD can do."
- Fix for section 7: "The two PyTorch rows use the PyTorch and transformers the notebook ships. The FP8 row adds a community kernel package from the Hugging Face Hub, in fallback mode. Neither is vLLM or SGLang."
- Also change "AMD's stock notebook cell" to "the notebook's stock pipeline call, inside my timing script (`uxl-stock.py`)". The archive does not contain AMD's original cell, so I could not confirm it is the same call.

### Minor

**9. The 2.0 ms gap between your launcher and HIP compares two slightly different clocks.**
- Text: section 4, "My own launch path is 2.0 ms per token slower than AMD's runtime driving the same kernels."
- Evidence: `oracle/driver-real2.py` line 338 prints the HIP timing rule: "synchronized wall time, graph capture excluded … host readback and token selection excluded". The native number is the launcher's whole step, first packet to completion signal. 26.721 − 24.741 = 1.980 is right as arithmetic.
- Fix: add "The HIP figure is synchronized wall time for one replayed graph and leaves out the host reading the token back, so part of the 2.0 ms may be the clock rather than the launch path."

**10. The source commits in the brief are not the ones the published receipt names.**
- Text: header, "Omega's GPU layer at commit `dc536882…` and … `omega-mv` at commit `2ee0a70d…`".
- Evidence: `evidence/amd-qwen38-native-refusal-20261113/host/qualification.json` gives `source_commit ab58f171…` and `compiler_commit 64d6db3b…`. Commit `dc536882` is dated 2026-11-13 04:45 −0600 (10:45 UTC), after the 10:12 UTC run. Both older commits exist in the public clones, `git diff --stat ab58f171 dc536882 -- backend/amd/kfd/launch` shows one changed line, and the binary and compiler hashes are identical, so nothing is wrong.
- Fix: "The 10:12 run used the launcher built at `ab58f171` with the compiler at `64d6db3b`. The commits above are later and rebuild the same bytes (same sha256), which Level 2 checks."

**11. Small number wording.**

| Brief text | Raw value | Suggested |
|---|---|---|
| "whole request 32.8 to 35.6 s" | 1 Nov median 32.797; 13 Nov timed runs 35.496 to 35.670 (`measurements.json`, decode5) | "32.8 s on 1 November, 35.5 to 35.7 s on 13 November" |
| "its process used 38.55 GB" | 38.55 GB at ready (VmRSS 37,644,288 kB); 38.78 GB after the requests (37,871,384 kB) | "38.55 GB after loading" |
| Tokens per second 7.97 and 7.37 | These are 1000 / graph time. The public 1 November record prints "Median steady decode 7.9348 tokens/s", and the 13 November server log prints 7.30 to 7.34 | Add to the row label: "(1000 / the row above; the server's own end-to-end figure is 7.93 and 7.31 to 7.34)" |
| "The Spark runs used the same checkpoint and revision, also hash-checked." | 1 November record says "all 81 files checked"; AMD side says 68 | "(81 files there, because the Spark copy includes the vision files)" if that is the reason |
| "Timed runs … 5" for 1 November | The record discarded `decode0` and replaced it. `decode0` was 125.0 ms, faster than the median, so the exclusion did not help the Spark | No change needed; one clause if you want it |

**12. Level 2: the last hash check passes even when the gate has failed.** On a stock Ubuntu 24.04 cloud image without `cc`, `check.py` exited 1 with `FileNotFoundError: [Errno 2] No such file or directory: 'cc'`, and the next line still printed the right `launch-x86` hash. Fix: add the install line that worked for me, `sudo apt-get install gcc libc6-dev linux-libc-dev qemu-user`, and "The run is good only if `check.py` exits 0 and prints the `AMD-KFD-LAUNCH CPU PASS` line."

**13. "about 20 seconds".** Levels 1 and 2 together took 29.7 s wall on my 4-core VM, clones included. Fine as written; "under a minute" is safer.

**14. Stale paths inside `amd-repeat-20261113T1907Z/README.md`.** It names `../card-run/analyze.py`, `card-evidence`, `../card-evidence-results.json` and `../level2/`. In the archive these are `tools/analyze.py`, the folder itself, `analyze-results.json` and `../level2-rebuild/`. `analyze-results.json` also embeds `/Users/drakestapleton/handoffs/uxl-brief-20261113/card-evidence/…` paths.

**15. Seed README points at the wrong script.** `omega-mv@2ee0a70d:seed/README.md` still tells the reader to run `sh seed/challenge/run.sh`. The brief names the right one (`sh seed/omega-from-seed.sh`), which I ran. A reader who opens the README will see the other. Not a brief defect; fix in the repository when convenient.

## 2. Every number and comparison checked

Arithmetic was recomputed with the Wolfram Language evaluator (one call, all ratios) and medians with my own Python script (`work/recompute.py`), independent of the archive's `tools/analyze.py`. "Repo" is `CUDApp@dc536882`, folder `evidence/amd-qwen38-native-refusal-20261113/` unless another is named. "Archive" is the tar.gz.

### Section 4 table and ratios

| Claim | Checked against | Result |
|---|---|---|
| Published run medians 26.731, 26.642, 26.675, 26.726, 26.760; median 26.726 ms | Repo `native/sample1..5.stdout`, 255 `OMEGA_STEP` lines after the first 30, `us=` field | Match exactly |
| Repeat medians 26.743, 26.803, 26.706, 26.671, 26.721; median 26.721 ms | Archive `amd-repeat…/uxl-native-run1..5.txt` | Match exactly |
| Spark 13 Nov 135.7, 135.5, 135.6, 135.2, 135.9; median 135.6 ms | Archive `spark…/measurements.json` `server_graph_chip_ms_per_token`, decode1..5; same lines in `run/server.log` | Match |
| Spark 1 Nov 125.4 ms | `Core@5c95594a` `docs/qwen38/qualification-20261101/short/events.jsonl`, decode1..5 = 124.8, 125.0, 125.4, 126.0, 126.4 | Median 125.4. Match |
| 37.42, 37.42, 7.37, 7.97 tokens/s | 1000 / each median = 37.417, 37.424, 7.375, 7.974 | Hold (see finding 11) |
| Prefill 779, 780, 607, 507 ms | Sum of first 30 step times: published median 779.042, repeat 779.599; Spark `server_prefill_ms` 603, 602, 633, 610, 607; 1 Nov 507, 531, 531, 480, 496 | Match |
| 5 timed runs plus 1 warmup, each column | File counts in each source | Match |
| Ratio 4.69 | 125.4 / 26.726 = 4.6921; / 26.721 = 4.6929 | Holds |
| Ratio 5.07 ("5.1" in the summary) | 135.6 / 26.721 = 5.0747 | Holds |
| "8 percent slower" | 135.6 / 125.4 − 1 = 8.13 % | Holds |
| "AMD prefill is 1.3 to 1.5 times slower" | 780 / 607 = 1.285; 779 / 507 = 1.537 | Holds |
| Spark wall 136.8 to 137.5 ms | `server_wall_ms_per_token` 137.3, 137.0, 137.1, 136.8, 137.5 | Match |
| Ready 19.2 s, first token 19.95 s, whole run 26.9 s | Repo `calculations.json`: 19.173, 19.954, 26.882. Repeat: `uxl-l3.log` run1 start to end = 26.91 s | Match |
| First visible text 0.49 to 0.61 s | 1 Nov 0.4900; 13 Nov 0.6114 | Match |
| Peak VRAM 37,556,044 KiB (35.8 GiB) | Repo `calculations.json` `sampled_peak_vram_kib`, `native/benchmark.stdout`; 37556044 / 2^20 = 35.816 | Match |
| "19 times the Spark's memory bandwidth" | 5300 / 273 = 19.41 | Holds |
| "at most about 1.0 TB/s … 17 to 19 percent" | 26.9 GB / 26.721 ms = 1006.7 GB/s = 19.0 %; 24.4 GB gives 913.1 GB/s = 17.2 % | Holds |
| "at most about 215 GB/s … 71 to 79 percent" | 26.9 / 0.1254 = 214.5 GB/s = 78.6 %; 24.4 gives 194.6 = 71.3 % | Holds |
| 26.9 GB of weights | Spark server line "weights 26.92 GB resident" (`run/server.log`) | Match |
| MI300X 5.3 TB/s | AMD MI300X data sheet PDF: "5.3 TB/s max. peak theoretical" | Match |
| HIP row 24.741 ms | Archive `uxl-graph-fresh.txt`: `decode_ms_median 24.7411889…` | Match |
| PyTorch row 131.011 ms; PyTorch 2.10.0, ROCm 7.2.2, transformers 5.12.1 | Archive `uxl-reference-fresh.txt`: 131.0108…; `uxl-env.log` | Match |
| Stock row 70.628 ms | Archive `uxl-stock.json`: run medians 70.498, 70.628, 68.604, 71.670, 71.829 | Median 70.628. Match |
| "2.0 ms per token slower" | 26.721 − 24.741 = 1.980 | Holds (finding 9) |
| 4.9 and 2.6 times | 4.903 and 2.643 | Hold (finding 8) |
| Stock: BF16, 70 prompt tokens, thinking on, default settings, missing fast path | `uxl-stock.json` (`dtype torch.bfloat16`, `prompt_tokens 70`, `text_head`), `uxl-stock.log` lines 3, 6, 7 | Match |
| PyTorch FP8: fast path missing, DeepGEMM to Triton, older kernel package | `uxl-reference-fresh.txt` | Match |
| All four between 19:07 and 19:18 UTC in session `…46d1b9c5` | `uxl-l3.log` and `uxl-stock.log` timestamps 19:06:51 to 19:17:57; hostname in `uxl-card.txt` | Match |

### Sections 1 to 3, 5, 6

| Claim | Checked against | Result |
|---|---|---|
| Card name, model 0x74b5, 191.7 GiB | Archive `uxl-card.txt`: 205,822,885,888 bytes = 191.6875 GiB | Match |
| `gfx_target_version 90402`, vendor 4098, device 29877, 1216 SIMDs | Repo `native/environment.json`; 0x1002 = 4098, 0x74b5 = 29877 | Match |
| Session names and times | Repo `host/qualification.json`, `environment.json` `captured_utc 10:12:34`; archive README and `uxl-card.txt` | Match |
| Linux 6.12.48+deb13-amd64, glibc 2.39, ROCm 7.2.2, root | Repo `environment.json`; archive `uxl-env.log` | Match |
| "64 GPU devices" | Archive `uxl-env.log`: 64 `cardN` entries in `/dev/dri` | Match |
| Launcher sha256 `08a50491…` | Archive `level2-rebuild/launch-x86`; `uxl-plan-and-code.sha256` (file on the card); my own rebuild | All three equal |
| 18 kernels | `uxl-plan-and-code.sha256` lists 18 `.hsaco`; `tests/amd-kfd-launch/qwen38-objects.json` | Match. All 18 card hashes equal my rebuild |
| 1,126 dispatches per step | `uxl-l3.log` `dispatches_per_step 1126` | Match |
| Model revision, 68 files, about 30 GB, no vision tensors | Archive `pre.log`: "CHECKPOINT SHA256 verified 68 files at 017b9c7a…; no visual tensors loaded", `du` 29G | Match |
| Download took 12 minutes | `pre.log` 18:54:25 to 19:06:37 | 12 min 12 s |
| Plan sha256 `2a292dad…9c012` | `uxl-l3.log` | Match |
| Coding prompt: 24 of 24 IDs, wrong-weights copy caught | Repo `coding24/coding24.json`; same 24 IDs as the Spark golden in archive `run/correctness.log` | Match |
| 256 IDs identical across published run, repeat and fresh HIP replay | My script: all ten native runs equal repo `numerical/graph.json` `ids`; `analyze.py` `every_run_ids_equal_graph_fresh PASS` | Match |
| Altered-weights plan diverges at the first token, both sessions | Repo `native/control.stdout` and archive `uxl-native-control.txt`: first difference at index 0 | Match |
| Spark 13 Nov equals 1 Nov, 256 IDs | `token_ids_sha256 eed890a9…` in both records | Match |
| First 109 tokens identical, AMD vs Spark | `first_divergence 109` in repo `graph.json` and archive `uxl-graph-fresh.txt` | Match |
| Margins 0.19, 0.01 (index 138), 0.125 (index 29), 0.106 | Repo `evidence/amd-qwen38-native-20261113/README.md` lines 93 to 98: 0.1898, 0.0101, 0.125, 0.1057; `numerical/forced.json`; fresh reference `first_divergence 29` | Match |
| Engine binary sha256 `df936011…`, Core commit `46134247`, record at `5c95594a` | 1 November README in the Core clone; archive `measurements.json` | Match; both commits exist |
| Compiler sha256 `26540e6d…` | `sha256sum omega-mv/seed/out/omc` on the Mac and in the VM; rebuilt from seed in the VM | Match |
| 274 checks | My VM run: 274 `PASS` lines, 0 `FAIL`, final line `"checks": 274, "failures": 0` | Match |
| Tested setup (Ubuntu 24.04 arm64, 4 KiB pages, Python 3.12.3, gcc 13.3.0, qemu-user 8.2.2) | Archive `level2-rebuild/logs/env.txt` | Match |
| Licences | `LICENSE` and `CONTRIBUTING.md` in CUDApp; 1 November README for CC0 | Match |
| Interplane link | HTTP 200, page contains the README title | Works |
| Hugging Face revision | `huggingface.co/api/models/Qwen/Qwen3.8-27B-FP8/revision/017b9c7a…` HTTP 200 | Exists |

### Could not check

| Claim | Why |
|---|---|
| Spark load "3.4 to 4.2 on 1 November"; GPU processes "not there on 1 November" | Not in the public record or the archive (finding 7) |
| Token Lab "37.8 tokens per second on 12 November" | The brief says the log is not in the archive |
| CPU reference "Rust, run on the Spark's CPU on 18 October" | The fixture exists in Core (`gen-chat-max-24.json`, added 19 October, `prefill_seconds 32.08`); language and run date are not recorded where I looked |
| "My sessions there ended after 30 to 40 minutes" | No session end times in the archive |
| Spark bandwidth 273 GB/s as "NVIDIA's figure" | NVIDIA's product page did not return the number to a plain fetch. It agrees with what I know of NVIDIA's published specification; that is my recollection, not a check |
| That the notebook is AMD's "Qwen/Qwen3.8-27B on AMD Instinct MI300X" One-Click notebook and that the stock cell is AMD's | Needs an AMD sign-in. `notebooks.amd.com/oneclick` returns HTTP 200 |
| "KFD 1.14 or later" as a tested bound | Stated in `backend/amd/kfd/launch/README.md`; `uxl-env.log` shows `/sys/class/kfd/kfd/version: No such file or directory`, so the card's version is not in the logs |
| Level 1 "ran … on the AMD machine" | Confirmed by archive `pre.log` (`LEVEL1_RC=0`), not by me |
| Everything that needs a GPU | No hardware |

## 3. Repeatability

Scratch folder: `work/repro/` on the Mac. For Level 2, a throwaway Ubuntu 24.04 arm64 VM (Lima, `LIMA_HOME=work/lima`, no host mounts, deleted afterwards; a cached Ubuntu image may remain in `~/Library/Caches/lima`). VM: Linux 6.8.0-134, 4096-byte pages, Python 3.12.3. Full outputs: `work/vm-level12.out`, `work/vm-level12-bare.out`, `work/l2-mac.err`.

### Level 1 on the Mac (macOS arm64, Python 3.14.6), commands as printed

| Command | Exit | Output matched the brief |
|---|---|---|
| `git clone https://www.aienos.com/rad/z4RxD8vXigmWg61nM8miQ8DVPAXEP.git CUDApp` | 0 | 1.7 s, 115 MB |
| `cd CUDApp` | 0 | |
| `git checkout dc536882c06a692991be97dc65cd2d037d82b129` | 0 | "HEAD is now at dc536882" |
| `python3 -I evidence/amd-qwen38-native-refusal-20261113/verify.py` | 0 | Yes, character for character: `EVIDENCE PASS 65 hashed files; raw steps/IDs; independent graph; reference coding24; calculations` (0.12 s) |

"Five minutes" is generous; it took under five seconds.

### Level 2 on the Mac (not a supported host; run to see how it fails)

| Command | Exit | Result |
|---|---|---|
| `cd ..` | 0 | |
| `git clone …z6do9WDBhuSnvopDkgFdFr1yWA62.git omega-mv` | 0 | 6.5 s, 587 MB |
| `git -C omega-mv checkout 2ee0a70d…` | 0 | |
| `sha256sum omega-mv/seed/out/omc` | 0 | `26540e6d…8991`. Matches |
| `cd CUDApp` | 0 | |
| `python3 -I tests/amd-kfd-launch/check.py "$(realpath ../omega-mv/seed/out/omc)"` | 1 | `OSError: [Errno 8] Exec format error` on `omc`. Expected: the brief says arm64 Linux only |
| `sha256sum build/amd-kfd-launch/gate/launch-x86` | 1 | No such file |

### Levels 1 and 2 in the arm64 Linux VM, both fenced blocks pasted in order into one shell

After `sudo apt-get install gcc libc6-dev linux-libc-dev qemu-user` (gcc 13.3.0, qemu-user 8.2.2, `linux/kfd_ioctl.h` present):

| Command | Exit | Output matched the brief |
|---|---|---|
| Level 1, four commands | 0, 0, 0, 0 | Yes, same `EVIDENCE PASS` line |
| `cd ..` | 0 | |
| `git clone … omega-mv` | 0 | |
| `git -C omega-mv checkout 2ee0a70d…` | 0 | |
| `sha256sum omega-mv/seed/out/omc` | 0 | Yes |
| `cd CUDApp` | 0 | |
| `python3 -I tests/amd-kfd-launch/check.py "$(realpath …)"` | 0 | Yes. Second-to-last line `AMD-KFD-LAUNCH CPU PASS {… "checks": 274 … "failures": 0 …}`, last line `NOT_RUN every AMD device row: …`. 274 `PASS` lines, 0 `FAIL` |
| `sha256sum build/amd-kfd-launch/gate/launch-x86` | 0 | Yes, `08a50491…91e1` |
| `sh seed/omega-from-seed.sh` (in `omega-mv`) | 0 | Yes. 0.44 s, ends `RC=0`, `seed/out/omc` hash unchanged. Repeated with the checked-in `omc` moved away first: same hash |

Whole script 29.7 s wall. Also from that build: `launch-a64` and `vadd.hsaco` equal the archive's copies, and all 18 `.hsaco` hashes in the archive's `uxl-plan-and-code.sha256` (the files on the card) equal my rebuilt ones.

Same script on the bare image before installing anything: every command exit 0 except `check.py`, exit 1, `FileNotFoundError: … 'cc'` (finding 12).

### Archive tool

`python3 tools/analyze.py amd-repeat-20261113T1907Z ../repro/CUDApp`: exit 0, 14 `CHECK … PASS` lines, ends `ANALYZE PASS`, as the archive README says.

### Level 3 (needs an MI300X; not run)

The instructions look complete for someone who has the card. The archive's `pre.sh`, `uxl-env.sh` and `uxl-l3.sh` are the brief's commands with logging added, and their logs show exit 0 at each step and the expected lines (`OMEGA vadd=PASS count=256`, plan hash, 285 steps per run). Gaps a reader would hit:
- The two input files (finding 6).
- `chmod +x launch-x86` is printed in the brief but does not appear in the archived scripts. Harmless.
- The `graph` comparison needs `libamdhip64` loadable (`driver-real2.py` line 103). The requirements sentence names PyTorch and safetensors only. Add "and ROCm's HIP library for the comparison commands".
- `mkdir out` fails on a second attempt. Say "use a fresh clone or remove `out` first".
- The brief does not say how the reader gets root with `/dev/kfd` anywhere other than AMD's notebook. One clause is enough: "on your own host, run as root or as a user in the `render` and `video` groups" if that is true; I could not test it.

## 4. Fairness

A reader at Intel or AMD would, in my judgement, find the main comparison fairly stated. The brief says the 4.7 ratio is not a measure of the card, gives the bandwidth use on each side, quotes the smaller of two ratios, says which clock favours which side, reports that AMD's own runtime drives the same kernels 2 ms faster, reports slower prefill, and lists Token Lab's 37.8 tokens per second beside its own 37.4. I looked for cherry-picking in the raw runs and found none: the excluded 1 November sample was faster than the median, and the warmups on both sides are in line with the timed runs.

What a knowledgeable reader could still object to:
- The printed 4.9 and 2.6 ratios (finding 8). This is the one place the brief is open to "apples to oranges".
- The Spark conditions that cannot be checked (finding 7).
- The HIP row's clock (finding 9).

Left out, and worth a sentence each:
- **Power and class of machine.** The Spark is a small desktop; the MI300X is a 750 W data-centre card. The summary's bandwidth sentence covers most of this. "A desktop machine against a data-centre card" in the summary would close it.
- **One request shape.** One prompt, 30 tokens in, 256 out, positions 30 to 285 of a 131,072 context. Section 7 says "Long prompts"; add "and I have not measured decode speed deep into the context".
- **The card is a virtual function on a shared host.** Section 1 says so; the bandwidth estimate assumes the full card's 5.3 TB/s. Fine, since the estimate is an upper bound on the hardware.
- **Intel.** The reader works at Intel and leads a group built around oneAPI. The brief says nothing about Intel hardware, SYCL or Level Zero, not even "not attempted". He will ask. One line in section 7: "Intel GPUs: not attempted."

## 5. Clarity: where this reader would stop

1. **What Omega is.** "a program written in Omega, my own language" has no link, no one-line description and no pointer to a source file to read. Add a link to the language's entry page and name one kernel source file and the `vadd` source, so the "small example" can be read as well as run. The brief gives the launcher path (`backend/amd/kfd/launch/`) but no path for the 18 kernels.
2. **The repository's front page.** The clone opens on a README titled "CUDA++ for the NVIDIA DGX Spark". The brief explains the name `CUDApp` but not that the front page is about NVIDIA. Add "Start at `backend/amd/kfd/launch/README.md`."
3. **"refusal" in the evidence folder name** (`amd-qwen38-native-refusal-20261113`). It appears four times and is never explained. One clause.
4. **The "small example" he asked for.** It is under Level 3 and needs an MI300X. Say in the summary which part is the small example and that Levels 1 and 2 are what he can run without the card.
5. **Who built what.** Section 2 is good on Omega against Python. Missing: who ran it (finding 5), and that the Spark engine is a separate, older Rust and C program that shares no code path with the AMD launcher. The words are there ("its own GB10 code path") but a reader could still think "Omega" ran on both sides, since the Spark log says "engine omega".
6. **Unexplained terms:** "GB10", "thinking off", "block-scaled FP8", "gated delta-net layers", "code object", "plan", "Core repository", "forced" replay ("Feeding the Spark's tokens through the AMD kernels"). Most need three words each.
7. **Two commit pairs** (finding 10).
8. **Section 8** is clear and well judged.

## 6. Things that should not leave the author's machine

I found **no secrets**: no API keys, access tokens, passwords, private keys, cookies, IP addresses other than 127.0.0.1, or email addresses in the archive. The brief contains only the author's own email. I found **no other person's name** in the brief or the archive.

Present, not secret, for the author to decide:

| Item | Where |
|---|---|
| Bearer token literal `local-qualification-only`, commented "synthetic local test credential … not a secret", for a loopback server | `spark…/paired_runner.py` line 36 (and the copy under `run/`) |
| Home directory paths (`/home/drakestapleton/…` 151 times, `/Users/drakestapleton/handoffs/…` 15 times), including `work/hive/…`, `handoffs/hive/queen`, `yardsticks/windows/…`, `.local/share/omega-tool-model/round40-fast/server` | Spark folder, `level2-rebuild/logs/pool.sh`, `amd-repeat…/analyze-results.json` |
| Hostname `spark-b87b` and "`ssh spark`" | `level2-rebuild/logs/env.txt`, `TRANSCRIPT.txt`, `README.md` line 16 |
| Process snapshots of the Spark with other programs' names (`codex`, `omp`, `hermes`, `radicle-seed`, `radicle-node`, `sshd`) and two resident GPU programs with paths | `spark…/measurements.json`, `run/events.jsonl` |
| Local port numbers 18196 and 18091 | `paired_runner.py`, `level2-rebuild/README.md` |
| Internal coordination notes (finding 4) | `level2-rebuild/README.md`, `logs/board-lines.txt`, `logs/pool.sh` |
| How the AMD notebook was driven (browser automation) | `amd-repeat…/README.md` line 8 |
| AMD-side identifiers: session hostnames, GPU UUID `GPU-2fc7ece477638043`, host CPU model, 64-card device list | Brief section 1 (session names); `uxl-card.txt`, `uxl-env.log` |
| Tar metadata: owner `drakestapleton/wheel` | Archive headers |

The archive's top README does warn "Some carry notes written for my own use and local folder paths". If the notes stay, that warning is enough for the paths. The coordination notes and the browser line are the two I would cut or explain.

## 7. Verified and found correct

- Both packet hashes.
- Every figure in the section 4 tables, every listed run median, and every ratio and percentage in the brief, from raw files (section 2 above). No arithmetic error found.
- All three public clone URLs work, all named commits exist, and the named files are at the named paths.
- Level 1 reproduces on macOS and on arm64 Linux with the exact expected line.
- Level 2 reproduces on a fresh arm64 Ubuntu 24.04 VM from fresh public clones with the brief's blocks pasted literally: 274 checks, 0 failures, launcher hash as printed.
- The compiler rebuilds from its seed to the printed hash, with and without the checked-in binary present.
- The launcher and all 18 kernels that the archive says were on the card are byte-identical (by sha256) to what I rebuilt from public source.
- Token ID claims: 256 of 256 across ten native AMD runs and the HIP replay; divergence from the Spark at index 109; controls diverge at index 0; the Spark's two dates agree.
- The 1 November Spark figures, from the public Core record.
- MI300X 5.3 TB/s, from AMD's data sheet.
- The brief's own caveats (clock, startup, memory accounting, stock cell differences, PyTorch fallbacks) are each supported by the logs.
- No secrets or third-party names in the packet.

## Verdict

**SEND AFTER FIXES: findings 1, 2, 3, 4, 5 and 6** (archive checksums; the stale Level 2 note; the "other tenants" sentence; trim or explain the working notes; say who ran the work; point Level 3 at the two files in the archive). Findings 7 and 8 are wording changes I would make in the same pass.

Every number I could check is right and the no-GPU steps reproduce exactly as printed, so the technical content is ready. The fixes are all in the packaging and wording: as it stands, a careful reader's first checksum command on the archive fails and one archive note says the brief's own commands fail, which would cost the brief credibility it has earned.
