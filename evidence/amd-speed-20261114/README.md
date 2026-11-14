# amd-speed-20261114: Omega on one AMD Instinct MI300X, decode speed work of 13 and 14 November 2026

The report is `REPORT.md` (and `REPORT.pdf`, the same text). Status: FINAL, 14 November 2026. The card items still open
are listed in the report's section 1, each to be done when equipment is available.

## Check it (any machine with Python 3, no GPU)

```sh
sh recompute/run-all.sh
```

It recomputes every MEASURED, DERIVED and COUNT number from the raw card logs under `raw/`, rebuilds `numbers.json`,
regenerates the card-session tables of the report, checks the report against `numbers.json` (report section 2 says
exactly what is checked), then runs everything a second time and requires `numbers.json`, `REPORT.md` and the
intermediate files to be byte-identical. Expected last lines: `CHECK-REPORT ... MISMATCH 0 dashes=0`,
`CHECK-SELFTEST PASS ...`, `REPEATABLE PASS ...` and `RUN-ALL rc=0`. Python standard library only. The output of the
run made when this folder was assembled is `recompute-run.txt`.

The recompute leaves every file listed in `SHA256SUMS` unchanged, so the file check passes after it:
`shasum -a 256 -c SHA256SUMS` (or `sh recompute/run-all.sh --verify-sums`, which does both).

## What is here

| Path | What |
|---|---|
| `REPORT.md`, `REPORT.pdf` | the report |
| `numbers.json` | every number in the report: value, unit, label (MEASURED, DERIVED, MODELED, QUOTED, COUNT, RECORD), source files with sha256, and how it is computed |
| `recompute/` | the scripts (measuring tools, not Omega), the quoted constants with their sources (`constants.json`), and the Wolfram checks (`WOLFRAM.md`) |
| `raw/` | raw evidence at the same relative paths the scripts and `numbers.json` use: card logs, gate logs, analysis files; the lanes' own notes are not included |
| `raw/unpacked/` | each card tarball unpacked; the tarballs themselves are kept beside their `.sha256` files under `raw/` |
| `MAIN-VERIFY.md` | the lead session's independent recomputation of the card results |
| `SHA256SUMS` | every file in this folder |

The first three sessions' evidence (`raw/amd-launch-gap/src/CUDApp-base/evidence/amd-qwen38-native-*`) is a copy of the
folders already published in this repository at commit `dc536882`.

Labels: MEASURED numbers are read from raw card logs. MODELED numbers come from stated assumptions (bytes per step, AMD's
quoted 5.3 TB/s) and are not measurements. NOT_RUN is not PASS.

## Machine paths in the evidence

The raw files are kept as they were written, so they name folders on the machines that produced them, as the published
brief folder does. None of these is needed to rerun anything here.

| Path as written | Machine | Files |
|---|---|---|
| `/home/drakestapleton/...` | the arm64 build host | `raw/amd-launch-gap/src/CUDApp-base/evidence/amd-qwen38-native-*/oracle/driver-real2.py` (already in this repository at `dc536882`) |
| `/home/drakestapleton/work/hive/...` | the DGX Spark (lane work folders) | `raw/amd-report/extracted/spark-logs-20261114/*.log` |
| `/home/aienalpha/...` | the DGX Spark (model folder) | `raw/four-bit/evidence/data.json` |
| `~/handoffs/...` and `/Users/drakestapleton/handoffs/...` | the Mac where the lanes kept their notes | `raw/amd-kernel-speed/kernels/FP8-R2-RESULT.md`, `raw/amd-kernel-speed/model/RANKING.md` |
| `~/work/...`, `spark:~/work/...`, `~/docs/...` | the DGX Spark | `raw/amd-kernel-speed/kernels/FP8-R2-RESULT.md`, `raw/four-bit/FORMAT.md`, `raw/amd-report/extracted/spark-logs-20261114/PROVENANCE.txt` |
| `/tmp/uxl-repro/...` | the Mac, a scratch clone of this repository | `raw/card-evidence/README.md` |
| `/tmp/qwen3.8-27b-fp8` | the MI300X notebook, where the weights were downloaded | card scripts and logs |
