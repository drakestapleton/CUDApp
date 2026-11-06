# Resident kernel host acceptance, 2026-11-06

Compiler: omega-mv source e5b32567, omc sha256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Stack: CodegenA generic replay commit `22ebf8f`, PR #275.
Hardware tier: **NOT_RUN**; no GPU authorization was granted or used.

The initial focused host build/run completed with rc=0 and:
`GPU RESIDENT PASS (reference, interleaved CTAs, barrier and descriptor controls); chip NOT_RUN`.
The tool transcript is `artifact://2415`; it is an initial exercise, not the
final source-mutant or full-host acceptance receipt.

Final focused gate: **PASS**, rc=0 (`focused.log`):

```sh
taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge \
  make -j3 test-gpu-resident
```

Final line: `GPU RESIDENT SOURCE MUTANTS PASS`. All six source mutants compiled,
then were killed with rc=1: dropped grid barrier, wrong GEMV multiplication,
missing output bound, FMA contraction, stale generation, wrong queue stride.
The reference/simulator line was `GPU RESIDENT PASS (reference, interleaved CTAs,
barrier and descriptor controls); chip NOT_RUN`. It also encodes the generated IR.

Device-free chip guard: **PASS**, expected rc=2 with no arguments, as recorded in
`chip-guard.log`. The binary reported
`NOT_RUN: --chip and an authorized SPARK_WINDOW are required`.

Full host gate: running once; receipt is `make-test.log`. Its final rc and line
will be recorded after it settles. No timeout retry is permitted by the parent.

The chip command, preconditions and launch-count comparison are in
`om/resident/README.md`. SC/coherent multi-CTA simulation does not qualify
physical GPU memory ordering, occupancy or forward progress.
