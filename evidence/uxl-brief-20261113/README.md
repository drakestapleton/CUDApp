> **Note, 14 November 2026:** run Level 3 only on hardware you control or with the host operator's agreement; Levels 1 and 2 need no GPU.

# UXL technical brief: Omega running Qwen3.8-27B on an AMD Instinct MI300X (13 November 2026)

This folder holds the technical brief I wrote for the UXL Open Source Working Group on 13 November 2026, with its raw evidence and its review record. The brief reports Omega's own launcher and 18 Omega-written gfx942 kernels running Qwen3.8-27B (FP8) on an AMD Instinct MI300X, compared with my DGX Spark. It gives three ways to check the result: recompute the numbers from the logs, rebuild the launcher and kernels from source, or run it on an MI300X. Read the brief as [BRIEF.md](BRIEF.md) or as a PDF, [BRIEF.pdf](BRIEF.pdf). The run that the brief calls the published run is in [`../amd-qwen38-native-refusal-20261113/`](../amd-qwen38-native-refusal-20261113/), added in `bb3bf43a` and present at `dc536882c06a692991be97dc65cd2d037d82b129`, the commit the brief names.

## Files

| Path | What it is |
|---|---|
| `BRIEF.md` | The brief. |
| `BRIEF.pdf` | The same brief as a PDF, made before this folder was published. One bullet in section 7 has its earlier wording (see below). |
| `archive/` | The raw evidence for the 13 November runs that are not in the published run's folder: the 19:07 UTC repeat on the MI300X with its same-session comparisons, the Level 2 rebuild, and the Spark run of the same evening. Start with `archive/README.md`. One script is withheld (see below). |
| `REVIEW.md` | The review record: who reviewed the brief, each round, what was changed. |
| `FRESH-REVIEW-1.md`, `FRESH-REVIEW-2.md` | The two rounds of the fresh review, as the reviewer wrote them. |

## How to check

```bash
cd evidence/uxl-brief-20261113/archive
sha256sum -c --ignore-missing SHA256SUMS   # every file except the withheld paired_runner.py (listed twice)
sed 's/^[0-9a-f]*  //' SHA256SUMS | while IFS= read -r f; do [ -e "$f" ] || echo "absent: $f"; done   # expect only the two paired_runner.py paths
for d in amd-repeat-20261113T1907Z level2-rebuild; do (cd "$d" && sha256sum -c SHA256SUMS); done
(cd spark-20261113T1913Z && sha256sum -c --ignore-missing SHA256SUMS)
# analyze.py checks against CUDApp at dc536882, the commit the brief names, so give it a checkout of that commit
git worktree add ../../../../CUDApp-dc536882 dc536882c06a692991be97dc65cd2d037d82b129
python3 tools/analyze.py amd-repeat-20261113T1907Z ../../../../CUDApp-dc536882   # recomputes every AMD number, prints ANALYZE PASS
```

On macOS use `shasum -a 256` in place of `sha256sum`. Section 6 of the brief gives the full steps for all three levels.

## Withheld: the Spark measurement driver

`archive/spark-20261113T1913Z/paired_runner.py` and its copy `archive/spark-20261113T1913Z/run/paired_runner.py` are not published here. That script drove the 13 November Spark run. It contains a fixed test credential string for a server it starts on 127.0.0.1 for the length of the run, and CUDApp's publication audit (`tools/extract/audit-scan.sh`) flags that string. The string is not a credential for any running service, but this repository does not carry anything its own audit flags. The script is available on request (drake@aienos.com). Both copies are byte-identical, sha256 `3f4143952b1285576791f7f657cdc3a5ddc108cf9c7efb992a95636311035986`, as the checksum lists say. Everything else in the archive is published unchanged, which is why the commands above use `--ignore-missing` for the two lists that name the script.

## What differs from the earlier text

- `BRIEF.md` differs from its pre-publication text in one bullet only, in section 7 ("Public logs for the 19:07 UTC repeat"). The earlier text said the raw logs were "not in the repository yet" and were in a tarball sent with the brief. The bullet now points at `archive/` in this folder and says one script is withheld. `BRIEF.pdf` still has the earlier wording of that bullet. The tarball it names is not published; `archive/` holds its contents, less the withheld script.
- `archive/README.md` and `archive/spark-20261113T1913Z/README.md` are evidence files listed in the checksums, so they were not edited. The first says the logs are "not in the public repository yet", which was true when it was written; this folder is where they now are. Its check command, `sha256sum -c SHA256SUMS`, now reports the two withheld `paired_runner.py` copies as missing (`FAILED open or read`); use the commands under "How to check" instead. The second names `paired_runner.py`, which is withheld as above.
- `REVIEW.md` names a `superseded/` folder holding an earlier archive. That folder is not published here.

## What is in the logs

These are working logs. As `archive/README.md` says, some carry notes written for my own use: local folder paths, job-scheduling lines, and how the runs were driven. The paths are under `/home/drakestapleton/` on the Spark and `/Users/drakestapleton/` on my Mac. The logs also show the Spark's host name, the hosted notebook's container name and the MI300X's GPU UUID.

## Licence

Code is Apache-2.0 WITH LLVM-exception. Measurement data is CC0-1.0.
