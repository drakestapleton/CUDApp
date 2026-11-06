# Seeded Omega pfhost builder receipt

Recorded 2026-11-06 UTC. IMPLEMENTED and EXERCISED on the host; chip NOT_RUN.
Not MERGED, DEPLOYED or hardware QUALIFIED.

Branch `feat/omega-codegen-pfhost`, draft [CUDApp#276](https://github.com/aien-dev/CUDApp/pull/276), stacks on [#274](https://github.com/aien-dev/CUDApp/pull/274), base `f219a04eff8f`.
Implementation commit `32554c4be2ca4480bc911649d23886a316d444c3`;
portable mutation-registration follow-on `4015f3d` does not change the generated unit or builders.

## Pins and execution environment

- Seeded compiler source: omega-mv `e5b32567a7aff3ce594e3254a622a846a6cd190b`.
- Compiler SHA-256: `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Checked converter: omega-bridge `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4`, as required by `om/omega-bridge.lock`.
- Generated portable C is the checked output of the first-party Omega unit, not a second handwritten builder.
- All build/test commands below were prefixed with `taskset -c 10-14 nice -n 10`; make used at most `-j3`.
- Logs normalize `/home/drakestapleton` to `~`. No other log content was edited.

```sh
export OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc
export OMEGA_BRIDGE=~/work/cudastack/omega-bridge
export OMEGA_MV=~/work/cudastack/omega-mv
export OMEGA_BRIDGE_DIR=~/work/cudastack/omega-bridge
```

## Observed checks

| Command (affinity/nice prefix omitted) | Result | Final line / evidence |
|---|---|---|
| `sh om/units.sh` | PASS, rc 0 | `wrote checked portable Omega units to .../src` |
| `make -j3 test-codegen-pfhost` | PASS, rc 0 | `PASS pfhost Omega source mutants killed`; `focused.log` |
| `make -j3 test-om-units build/gpu_pfhost_test` | PASS, rc 0 | `PASS portable Omega units regenerate byte for byte`; `units-check.log` |
| `build/gpu_pfhost_test --sim` | PASS, rc 0 | `GPU_PFHOST: PASS`; `sim.log` |
| `make -j3 test` | INCONCLUSIVE | 3600-second eval deadline; no make rc or `CUDA++ HOST TESTS PASS` final line observed; `full-test-deadline.txt` |
| Chip | NOT_RUN | No exclusive GPU window authorized |

The full run was not retried, under Main's contention/deadline policy. Its stdout was buffered in `subprocess.run` and was not persisted before interruption; the deadline receipt explicitly records that limitation. The integrated full gate belongs to Integrator (`integ/omega-cuda-stack`, draft #280). This receipt does not claim that gate has passed.

## Exhaustive differential and controls

The test was written first and initially failed to compile because the generated Omega entry did not exist; see `red-first.txt`.

The test enumerates every config accepted by the unchanged C spec validators, not a boundary sample:

- RL: `k=128..16384` in steps of 128, `e=32..1024` in steps of 32, `t=1..16`, every mutant selector 0, 1 and 2, filtered only by `omega_rl_spec_ok`.
- MC: `h=128..4096` in steps of 128, `top_k=4,8,12,16`, every mutant selector 0, 1 and 2.
- Observed successful comparisons: RL `19150`, MC `384`; matching builder refusals `50` (RL mutant 2 configurations with no remaining chunk).
- Comparison includes every IR instruction, full solved register allocation and the complete encoded instruction bytes. Builder return values must also agree.
- Flipping one instruction immediate must fail the comparator: PASS.
- Invalid/null API inputs and an invalid Omega unit kind must refuse: PASS.
- Isolated router-address Omega source mutant: killed by the differential at `k=256,e=32,t=1,mutant=0`, with both programs containing 65 records.
- Isolated combine-address Omega source mutant: killed at `h=128,top_k=4,mutant=0`, with both programs containing 67 records.

The simulator exercised every existing RL/MC fixture in both scheduling orders with zero differing outputs, and caught both numerical mutant selectors in both families. These are simulator observations, not chip measurements.

The 16-live-value compiler limit was routed around by separating the RL math phase and naming the intermediate MC virtual register before the nested loop. No compiler feature, ABI, replay gate, or trust path was changed.

## Chip runbook (not executed)

Obtain the current exclusive GPU window using the operator procedure; verify that no protected service/model is disturbed. Build `build/gpu_pfhost_test` with the same pins. Within that window, run:

```sh
build/gpu_pfhost_test --chip --tokens 2048 --reps 7
```

The executable also checks the short router and combine cases in chip mode. Record the process rc, every case result and final `GPU_PFHOST: PASS` independently from these host/simulator receipts. No chip run or change to a GPU lock, live model, driver or GPU module was performed by this lane.

## Integration and guide impact

`om/units.sh` is append-only and guarded by an unset `OMEGA_UNITS`. The generated `cudapp_native.h` and `cudapp_om_units.ident` need normal stack integration. CodegenA owns `om/codegen/.crumb`; its generated digest is deliberately unstaged here and must be regenerated after integration. No public header signature changed.

The existing `docs/OMEGA_HOST_TESTS.md` documents this builder, retained C oracle switch, differential, mutants, simulator and chip boundary. No language, compiler, converter, host-word, repository-ownership, or gate policy changed, so no omega-harness skill command or pin needed updating.
