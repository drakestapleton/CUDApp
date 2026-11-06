<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# hello_gpu: seeded host planning and checking

The default executable uses `om/graph/hello.om` for input construction and
whole-output checking, and `hello-plan.om` for scoreboard controls, chunk sizes,
launch dimensions, byte counts, bandwidth reporting and simulator fault planning.
`CUDAPP_MIGRATION_PENDING_HELLO_HOST=1` selects the C differential oracle only.
It is OFF by default. The existing HELLO_CONTROL switch enables the intentional
short-launch negative control; it does not select the C host oracle.

C retains the IR instruction emission, register-allocation API calls, command-line
and diagnostic I/O, clock acquisition, allocations, simulator invocation, device
session and launches. No GPU launcher was moved into Omega. A result is checked
on the host, but the checker is first-party seeded Omega, not handwritten C.
Input values are finite binary32; the differential compares their bits exactly.
The poison sentinel is counted separately from wrong answers.

## Host acceptance

Every execution, including simulator, differential and mutant children, must run
inside the device-hiding wrapper:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 make -j3 \
  test-graph-residue test-graph-residue-mutants test-graph-residue-coverage \
  test-hello-sim
```

Set `OMEGA_OMC` and `OMEGA_BRIDGE` to the pins required by `om/units.sh` when
running the mutant gate. The coverage gate compiles its C oracles with
`-O0 --coverage -ffp-contract=off`. It also runs the actual oracle executable's
simulator path, plus past-end and short negative controls. Those controls must
exit 1, not merely any nonzero status. `test-graph-residue` compares deterministic
phase digests and diagnostics against the oracle and requires a deliberately
changed result to disagree. The mutant gate compiles each changed `.om` unit;
compile refusal is not counted as a killed mutant.

## Chip runbook — NOT_RUN

This migration has no current hardware qualification. Historical hello receipts
do not qualify these source bytes. No GPU window was authorized for this lane.

1. Obtain a current exclusive hardware window and follow the current Spark
   authorization procedure. Record the exact commit, generated identities,
   compiler and bridge digests, GPU identity, module version and window receipt.
   A free lock file alone is not authorization. Do not stop a service or change
   driver binding merely to run this example.
2. First complete the wrapped host gates above. Build `build/hello_gpu` and
   `build/hello_gpu_control` without executing a chip target. Record their hashes.
3. Inside the authorized hardware procedure only, run `make doctor`, then
   `make hello-gpu`. This step deliberately cannot use the host device-hiding
   bwrap: it needs the admitted GPU. Require exit 0, every element checked,
   zero wrong and unwritten elements, and a final `PASS`.
4. Run `make hello-gpu-control`. Its short final launch must produce exactly one
   missing result, executable exit 1, and the enclosing control target must pass.
   **Never run the past-end control on hardware.** It is simulator-only.
5. Compare the output hash with the same revision's wrapped simulator output.
   Keep stdout, stderr, exit codes and device admission evidence. A mismatch,
   timeout or missing negative-control observation is not PASS. Do not infer
   model-graph chip qualification from the vector-add receipt.

The host simulator cannot establish chip timing, fault recovery or device safety.
