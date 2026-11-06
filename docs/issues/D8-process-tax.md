# D8: measure what a CUDA++ process costs beyond its own buffers

Labels: D8

**The pain.** "The CUDA tax, which is 4.6 GB approximately per each model on a DGX spark." (r/LocalLLaMA). `docs/PAIN_POINTS.md` section 2.7.

**What exists.** A CUDA++ process maps no CUDA library (Omega `evidence/WHAT-OMEGA-IS-20261021/live-serve-no-cuda.txt`).

**What is left.** Measure it: resident memory of `hello_gpu` before open, after open and after its buffers are allocated, and the GPU memory the driver reports for the process, so the overhead beyond the program's own buffers is a number.

**Done when.** `hello_gpu` prints its own overhead line, and a chip receipt records it.

**Command (on CUDApp#280).** `hello_gpu` prints the overhead line
(`examples/hello_gpu/README.md`, "D8"). `tests/run_d8_process_tax.sh` holds the bar, which was fixed before any chip
run:

- B1: the run passes.
- B2: there is exactly one D8 line, and its printed differences add up.
- B3: `VmRSS` read from outside during `--hold-ms` is within 4096 KiB of the line.
- B4: the driver reports the pid with a whole number of MiB.
- B5: TAX = max(0, open cost, resident beyond own buffers) + max(0, driver MiB × 1024 − own buffers KiB).
  TAX must be ≤ 262144 KiB, which is 256 MiB.

The bar is 17.1 times below the 4.6 GB in the quote (WolframLanguageEvaluator: `N[4.6*10^9/(256*2^20)] -> 17.136`).
Two negative controls run in the same lease. `--bug=short` must fail B1. `--d8-ballast-mib=512` must show at least
524288 KiB of open cost and must fail B5. The verdict is PASS only when the main run passes and both controls are
caught.

- `make d8-process-tax-dry-run` runs on the host only, with the devices hidden. It exits 0 when the simulator run,
  the controls, the device-hidden refusal and the judge self-tests behave as specified.
- `make d8-process-tax` takes one `spark-window` lease (gb10 exclusive, 20 minutes, ledger DRA-158) and writes
  `build/d8-process-tax/<UTC>/`. That directory holds the logs, the outside readings, `env.txt`, `lease.txt` and
  `verdict.txt`.

**Status.** The command is IMPLEMENTED and the dry run is EXERCISED on the host. The chip row is NOT_RUN. It waits for
CUDApp#280 and an authorized window, and no chip receipt exists yet.
