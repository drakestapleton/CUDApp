# D8 process tax: shell driver replaced by `.om` (CUDApp#366), host evidence

Host: DGX Spark (aarch64, Linux 7.0.0-1019-nvidia), CPU core 18 only (`taskset -c 18 nice -n 10`), 2026-11-07.
No GPU was opened. The chip row (`make d8-process-tax`) is NOT_RUN. Status: IMPLEMENTED and EXERCISED on the
host. It is not QUALIFIED.

Compiler: omega-mv e5b32567a7aff3ce594e3254a622a846a6cd190b `seed/out/omc`, sha256
2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f. Bridge: omega-bridge
29d9cff72676bcef41a067bdcf4d8b0711042bcd (`om/omega-bridge.lock`). `tools.txt` lists every tool, its version
and the sha256 of each source and binary.

| File | What it shows |
|---|---|
| `old-shell-dry-run.log` | `bash tests/run_d8_process_tax.sh --dry-run` (the removed driver), exit 0 |
| `new-om-dry-run.log` | `build/d8_process_tax dry-run "" HELLO HELLO_CONTROL`, exit 0 |
| `check-gpu/` | Twelve recorded window directories, judged by the old `--check-gpu` and the new `check-gpu`: `good2` (a true PASS, exit 0), `good` and `blind` (blind ballast control), `corrupt` (corrupted D8 line), `g-mainbad` (the line's sums disagree), `g-nodrv` (no driver report), `g-tax` (tax above the bar), `shortpass` (short control passes), `nomain`, `nodev`, `empty` and `missingdir`. `compare.txt`: output and exit status are byte-identical in all twelve |
| `mutant-*.diff` | The three D8 semantic mutants of `om/graph/hello-plan.om`, built through `om/units.sh` and linked into `tests/test_graph_residue.c`. Each changes the residue output, so each is killed |
| `mutants-negative-controls.log` | An identity mutant survives and fails the gate with exit 1. An absent mutation site fails it with exit 1 |
| `final-checks.log` | `sh om/units.sh --check`, `make test-hello-sim test-graph-residue test-graph-residue-mutants d8-process-tax-dry-run`, `bash tools/extract/audit-selftest.sh`; all exit 0 |

NOT_RUN: the chip row (`make d8-process-tax`, which needs a spark-window lease) and the `inside-window` and `gpu`
paths. Without a lease, `inside-window` printed `NOT_RUN: not inside an exclusive spark-window lease` and exited 2,
as the shell version did.
