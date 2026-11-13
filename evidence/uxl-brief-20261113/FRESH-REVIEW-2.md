# Second review: revised BRIEF.md and evidence archive (round 2)

Reviewed 13 November 2026 on the author's Mac.

**Reviewer.** I am Claude, made by Anthropic; the harness reports the model id as `anthropic/claude-fable-5-2`. I used no memory, recall, archive or past-session tool, read nothing under `~/handoffs/`, and made no ssh connection. Inputs: the two round 2 files, my own round 1 scratch folder (`../work/`, including the public clones made then), and the files inside the new archive.

**Verdict in one line:** SEND. Details at the end.

## 0. Packet identity and archive checksums

| File | Expected sha256 | Result |
|---|---|---|
| `round2/BRIEF.md` | `72f2776d…183bab5d` | OK |
| `round2/uxl-brief-evidence-20261113.tar.gz` | `94262c8d…cc496737` | OK |

Archive extracted to `round2/work/uxl-brief-evidence-20261113/` (138 files; round 1 had 135). `shasum -a 256 -c SHA256SUMS` in every folder that has one:

| Folder | Entries | Files in folder | Not OK |
|---|---|---|---|
| top level | 137 | 138 (all but the list itself) | 0 |
| `amd-repeat-20261113T1907Z/` | 43 | 46 | 0 |
| `level2-rebuild/` | 30 | 31 | 0 |
| `spark-20261113T1913Z/` | 57 | 58 | 0 |

Compared with the round 1 archive (`diff -rq`), the only changes are: three README files edited, `launch-x86` and `vadd.hsaco` copied into `amd-repeat-20261113T1907Z/` (hashes `08a50491…91e1` and `10eb12e0…e55f`, equal to the `level2-rebuild/` copies and to my round 1 rebuild), a new `SHA256SUMS` in the Spark folder, and the two regenerated checksum lists. No raw log or measurement file changed.

## 1. Status of the 15 round 1 findings

| # | Finding | Status | What shows it |
|---|---|---|---|
| 1 | Archive checksum files contradicted the brief | **Fixed** | All four checks pass (table above). Brief section 7 now reads "Its top-level `SHA256SUMS` lists every file", which is true (137 of 137). |
| 2 | Stale Level 2 note said the brief's commands fail | **Fixed** | New first lines of `level2-rebuild/README.md`: "this test was run against an earlier draft of the brief (sha256 `5f94d683…`). The three corrections it lists are in the brief you received. Run B below is the Level 2 block as the brief now prints it." I compared `logs/runB-clone.sh` and `logs/runB-gate.sh` with the brief's block: same commands in the same order, with logging added. |
| 3 | "other tenants' devices refuse access" | **Fixed** | Section 1 now: "Only one is readable from my container, and my launcher uses that one and skips the others (reading their properties returns "not permitted"). I did not try to open any other device." Supported by `uxl-env.log` lines 143 to 183 (see new item N2 for an optional tightening). |
| 4 | Working notes in the archive | **Partly fixed, by explanation; acceptable** | Nothing was removed. The top README now says: "Some carry notes written for my own use: local folder paths, job-scheduling lines, and how the runs were driven. I work with AI coding agents…". The Level 2 note adds "Lane, lock and board lines are my own job-scheduling notes." Home paths, hostname `spark-b87b`, process lists and the browser line are still there. None is a secret, and the reader is now told what they are. |
| 5 | Brief did not say who ran the work | **Fixed** | Section 6: "I work with AI coding agents. They write much of the code and tooling, and they ran these measurements under my direction. The commands and logs in the archive are exactly what ran." (See N3.) |
| 6 | Level 3 needed two files only available from arm64 Linux | **Fixed** | Section 6: "If you have no arm64 Linux machine, the same two files are in the evidence archive under `level2-rebuild/`", with the `vadd.hsaco` hash printed. Both hashes verified. The `libamdhip64` requirement and the `mkdir out` note were added too. |
| 7 | Uncheckable Spark load and GPU-process claims | **Fixed** | "3.4 to 4.2" and "not there on 1 November" are gone. New text: "10.5 to 16.3 during the warmup and timed requests, where the public 1 November record holds one sample, 3.7. Two small GPU processes were also resident." All three match `measurements.json` and the public record. |
| 8 | 4.9 and 2.6 ratios stated first, withdrawn after | **Fixed** | "Neither PyTorch row is a speed baseline. For the record, they were 4.9 and 2.6 times slower per token than my path." Section 7 now says the FP8 row "adds a community kernel package from the Hugging Face Hub, in fallback mode". The table row is "The notebook's stock pipeline call inside my timing script (`uxl-stock.py`)". |
| 9 | HIP row clock | **Fixed** | "The HIP figure is synchronized wall time for one replayed step and leaves out the host reading the token back, so part of the 2.0 ms may be the clock and not the launch path." Matches `driver-real2.py` line 338. |
| 10 | Two commit pairs | **Fixed** | New header paragraph names `ab58f171` and `64d6db3b`. I checked the new claim: `git show 64d6db3b:seed/out/omc` hashes to `26540e6d…8991`, and `ab58f171` is dated 10:09 UTC, before the 10:12 UTC run. |
| 11 | Small number wordings | **Fixed where a fact existed** | "32.8 s on 1 November and 35.5 to 35.7 s on 13 November"; "38.55 GB after loading"; row label "(1000 / the row above)" plus the server's own 7.93 and 7.30 to 7.34; "(the 1 November record lists 81 files checked there)". No reason is given for 81 against 68. Stating both counts is enough. |
| 12 | Hash check passes when the gate fails | **Fixed** | "The run is good only if `check.py` exits 0 and prints the `AMD-KFD-LAUNCH CPU PASS` line. The hash line alone is not enough.", and the `apt-get` line, which is the one I ran in round 1. |
| 13 | "about 20 seconds" | **Fixed** | "in under a minute". |
| 14 | Stale paths in the AMD repeat README | **Fixed by note** | New first lines map each old name to its name in the archive. `analyze-results.json` still embeds `/Users/drakestapleton/handoffs/…` paths; the top README's warning about local folder paths covers that. |
| 15 | Seed README in `omega-mv` points at `seed/challenge/run.sh` | **Not fixed; not in the packet** | This is in the public repository, not the brief. The brief names the right script. Does not hold the brief. |

### Suggestions left out, and whether any holds the brief

None does.

- **No Omega language link, no kernel source path, "refusal" unexplained, no glossary.** These were clarity suggestions. The revision added the two things that matter most for orientation: "For the AMD work, start at `backend/amd/kfd/launch/README.md`" (the file exists at `dc536882`) and "a different program … It is not the launcher above" for the Spark engine. Leaving out facts that could not be confirmed is the right call. The reader can ask.
- **Power figures.** Replaced by "The Spark is a desktop machine and the MI300X is a data-centre card", which is enough.
- **Intel.** Added: "Nothing in this brief ran on an Intel GPU." The wording is careful and correct: the AMD host's CPU is an Intel Xeon (`uxl-env.log`), so "Intel GPU" is the accurate scope.
- **AMD's notebook terms.** Still not something I can check. It is the author's call and not a defect in the text.
- **Items I could not check in round 1** (Token Lab 37.8 tokens per second, the 18 October Rust reference, session length, 273 GB/s from NVIDIA's page, KFD 1.14 as a tested bound): unchanged, and each is either disclosed in the brief as unlogged or a detail no number depends on.

## 2. New or changed sentences, checked against raw files

Every line that `diff BRIEF.md round2/BRIEF.md` reports was checked. All are supported. Three small points, none of which needs another round.

| New or changed text | Checked against | Result |
|---|---|---|
| "The 10:12 UTC run used the launcher built at `CUDApp` commit `ab58f171` with the compiler at `omega-mv` commit `64d6db3b` … rebuild the same bytes (same sha256), which Level 2 checks." | Repo `host/qualification.json`; `git show 64d6db3b:seed/out/omc \| shasum` = `26540e6d…`; my round 1 Level 2 run at the later commits produced `08a50491…` | Holds |
| "The front page of the `CUDApp` repository is still about NVIDIA … start at `backend/amd/kfd/launch/README.md`." | Repo `README.md` title "CUDA++ for the NVIDIA DGX Spark"; the named file exists | Holds |
| "The Spark is a desktop machine and the MI300X is a data-centre card with 19 times its memory bandwidth." | 5300 / 273 = 19.4 (round 1) | Holds |
| "The small example, a vector add on the card, is the first step of Level 3." | Section 6 | Holds |
| "The container's `/dev/dri` lists 64 cards. The KFD topology shows several GPU nodes. Only one is readable…" | `uxl-env.log`: 64 `cardN`; ten topology nodes, 0 and 1 are CPU nodes, 6 is the GPU (`gpu_id 36258`), and 2, 3, 4, 5, 7, 8, 9 return "Operation not permitted" | Holds. See N2 |
| "(the 1 November record lists 81 files checked there)" | `Core@5c95594a` qualification README: "all 81 files checked" | Holds |
| "10.5 to 16.3 during the warmup and timed requests … one sample, 3.7" | `measurements.json` load before and after warmup through decode5: 10.50 to 16.34; public record 3.71 | Holds |
| "A 24-token request in the same server log, a few minutes earlier, ran at 125.5 ms per token" | `run/server.log`, first request line | Holds. See N1 |
| "7.93 tokens per second on 1 November and 7.30 to 7.34 on 13 November" | Public record "7.9348 tokens/s"; `measurements.json` 7.31, 7.33, 7.32, 7.34, 7.30 | Holds |
| "32.8 s on 1 November and 35.5 to 35.7 s on 13 November"; "38.55 GB after loading" | 32.797; 35.496 to 35.670; server ready line | Holds |
| HIP clock sentence | `driver-real2.py` line 338 | Holds |
| "4.9 and 2.6 times slower per token than my path" | 4.903, 2.643 (round 1, Wolfram) | Holds |
| "community kernel package on the Hugging Face Hub, an older version, which fell back from DeepGEMM to Triton" | `uxl-reference-fresh.txt` | Holds |
| "under a minute"; the `apt-get` line; "The check itself needs no root" | My round 1 VM run: 29.7 s for both levels, that exact install line, check run as a normal user | Holds |
| "also need ROCm's HIP library (`libamdhip64`)" | `driver-real2.py` line 103 | Holds |
| `vadd.hsaco` hash in section 6 | Archive file and my rebuild | Holds |
| Archive README: "Each of the three evidence folders also has its own." | Checks above | Holds |
| AMD repeat README note: path mapping, files copied in, meaning of "stock notebook cell" | Files are where it says; hashes equal | Holds |

**N1 (minor, optional).** "a few minutes earlier" is about one minute. The engine was ready at 19:14:25 UTC, the correctness requests ran straight after, and the warmup began at 19:15:32. The same log also shows the slide happening inside that one check: 125.5, 124.9, 125.6, 126.5, 127.2, 127.3, then 135.0 and 134.3, before the first 256-token request at 135.7. That makes the sentence's conclusion stronger than it claims, since one server process went from 125 to 135 with no rebuild in between. Suggested: "The first requests in the same server log, about a minute earlier, ran at 125 to 127 ms per token, the 1 November speed, and the last two before the timed runs were already at 134 to 135. So I read the slowdown as contention and not a slower build."

**N2 (minor, optional).** "The KFD topology shows several GPU nodes" is an inference for the seven unreadable nodes, since their type cannot be read. Exact wording from the log: "The KFD topology has ten nodes. Two are CPU nodes, one is the GPU I used, and seven return "not permitted"."

**N3 (minor, optional).** "The commands and logs in the archive are exactly what ran." One log is a trimmed copy, and the archive says so itself (`level2-rebuild/logs/runC-controls.NOTE.txt`: the shell trace lines of a symlink loop were dropped, 110 of 7,987 lines kept). It is a negative-control log and no number depends on it. If you want the sentence literally true: "The commands and logs in the archive are what ran; one long trace log is trimmed, with a note beside it."

Nothing else new. I found no new unsupported number, no new unfair comparison, and no new private detail: the changed archive files are three README notes and two binaries that were already in the archive.

## 3. Level 1 and Level 2 blocks

Both fenced blocks are byte-identical to round 1 (extracted with `awk` from each brief and compared with `diff`). No rerun was needed and none was made. The round 1 results stand: Level 1 on macOS and arm64 Linux, Level 2 on a fresh arm64 Ubuntu 24.04 VM, every command exit 0, 274 checks, 0 failures, hashes as printed.

The only change inside any fenced block is a comment on the Level 3 line `mkdir out    # in a fresh clone; remove an old out folder first`, which needs a GPU and cannot be run here.

## Verdict

**SEND.**

Every round 1 finding that concerned the packet is fixed or openly explained, all four checksum checks now pass, and each new sentence is supported by a raw file or the public repositories. The three remaining points (N1 to N3) are wording refinements that do not change any claim, so they are not worth holding the brief for.
