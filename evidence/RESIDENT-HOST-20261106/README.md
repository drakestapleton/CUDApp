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

Full host gate: **INCONCLUSIVE**. The single run reached the outer 3600-second
deadline; no complete `make` rc or final `CUDA++ HOST TESTS PASS (...)` line was
observed. It was not retried, per the parent's serialized integration policy.
Receipt: `make-test.log`; invocation:

```sh
taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge \
  OMEGA_MV=/home/drakestapleton/work/cudastack/omega-mv \
  make -j3 test
```

That run exposed a real owned registration defect: the new resident C/header
had no portability-table rows. Both rows were added to
`docs/runtime/portability_table.tsv`; focused registry and existing simulator
regression verdicts are recorded separately in `registry-simulator.log`.
The integrated full gate belongs to Integrator (draft #280), not this receipt.

The repaired focused registry/simulator gate is **PASS**, rc=0:

```sh
taskset -c 10-14 nice -n 10 make -j3 \
  test-runtime-portability-check test-gpu-cmdlist test-gpu-imadhi-rc
```

`registry-simulator.log` records the unchanged default scheduler regression:
cmdlist host-only `27 checks, 0 failed: PASS`; cmdlist simulator
`74 checks, 0 failed: PASS`; IMADHI tests pass for all four simulator headers,
including `bw_warp_sim_k.h`. Final line:
`PASS: 152 rows, every omega_gpu_* / omega_blackwell_* / omega_bw_* / omega_ampere_* file in src/ has one and the rows agree with the includes`.

The chip command, preconditions and launch-count comparison are in
`om/resident/README.md`. SC/coherent multi-CTA simulation does not qualify
physical GPU memory ordering, occupancy or forward progress.
