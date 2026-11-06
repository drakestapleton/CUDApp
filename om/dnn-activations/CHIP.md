# Activation chip qualification: NOT_RUN

No GPU execution was authorized for this slice. Host simulation and successful
encoding are not hardware qualification. Do not run this procedure from an agent
session without a current operator-authorized exclusive GB10 window.

## Prerequisites and safety

The operator owns GPU scheduling and service coordination. Follow the current
GPU-window authorization procedure, including an exclusive window and a fresh
live-service check. A lock-file state is not permission. Do not disturb serving
ports, modules or watchdogs; do not use pkill/killall. If the required window is
not available, record NOT_RUN and stop without opening the device.

Use the exact candidate commit and the seeded compiler/bridge pinned by
`om/units.sh`. Record `git rev-parse HEAD`, `uname -a`, the compiler SHA256,
bridge revision, GPU/driver identity and window authorization in a new dated
evidence directory. Never overwrite historical evidence. The supported chip is
GB10 sm_121 with the repository's supported open driver version. Stop on a
mismatch rather than widening the qualified-chip table.

## Host prerequisites (no device access)

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-gpu-dnn-activations test-gpu-dnn-activations-mutants
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 build/gpu_dnn_activations_chip_test
```

Set the compiler/bridge environment exactly as described in README.md before
running the mutation target. The normal host executable cannot enter chip mode.
The separate chip executable requires `--chip`. Both link session wrappers;
only the explicitly compiled chip runner in `--chip` mode forwards those wrappers
to the real session functions. Under `--sim`/`--quick`, any call to session
open/lock/unlock/free or allocation aborts before forwarding. Only the separate
explicit `--shell-contract` mode uses an isolated fake backend to test NULL and
non-NULL open semantics; it never enters the real driver.

## Operator-only chip command

Only after checking the authorization at the point of execution:

```sh
taskset -c 10-14 nice -n 10 ./build/gpu_dnn_activations_chip_test --chip
```

Capture stdout, stderr, exit code and the exact command in the new receipt.
Expected exit code is 0. Expected final line is:

```
dnn-activations chip: PASS (4 operators, 3 types, tails, independent double references, refusal-before-open, wrong-result control)
```

The runner exercises all four operations and three storage formats with counts
1,2,7,31,32,33,64,65,513,65535. It includes both signed zeros, positive/negative
small values, positive/negative saturation and largest finite storage values.
Each launch compares f32 output with the independent double reference under the
README tolerance. A deliberately wrong reference must disagree. Refused shapes
are checked before valid launches. Any failed value, nonfinite output, nonzero
exit, timeout, device fault or sentinel failure is FAIL, not a reason to loosen
tolerance. Stop; preserve the receipt and diagnose before any authorized rerun.
An interrupted/killed run is INCONCLUSIVE, not PASS.

This command's correctness sweep does not measure performance. It does not
qualify maximum-count allocations, every finite input, other GPUs, every driver,
or whole-model inference. Record those as NOT_RUN. No merge/deployment/status
upgrade follows automatically from a successful chip run.
