# Core utility chip runbook

Hardware verdict: **NOT_RUN**. No GPU, live-RM, VFIO or hardware window is authorized for this lane. Host, differential, mutation and warp-simulator results do not qualify a chip.

## Before a future hardware run

1. Obtain fresh, explicit operator authorization naming the target device, exclusive window and permitted service interruptions. Historical receipts and a free lock are not authorization. Do not touch ports 18091 or 18173–18178, PID 68451, omega-tool services, NVIDIA modules, VFIO or the GB10 lock under this assignment.
2. Integrate and qualify the single freestanding native aggregate. Its core sources are deterministically composed by `sh om/core/native.sh`; `--check` must pass. Confirm that the selected native float profile and pinned seeded compiler admit every core export, cubin export and Ampere export together. Never enable the legacy C macro to hide a missing native capability.
3. Run all host preflight commands below in the device-free namespace. Missing device nodes on a purported host/sim path are a defect, not a reason to rerun outside the namespace.
4. Record source revision, compiler/converter identities, device identity and authorization receipt. Use the existing platform-specific operator runbook to expose only the authorized device nodes. This document does not grant a full `/dev` or prescribe stopping any live service.

## Device-free host preflight

Set `OMEGA_OMC` to the pinned `omega-mv@e5b32567` seeded compiler and `OMEGA_BRIDGE` to the pinned portable bridge checkout. Use at most three build jobs:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-core-util test-core-mutants test-om-cubin
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/units.sh --check
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/core/native.sh --check
```

The SHA gate includes every supplied official CAVP short/long record and a random streaming C differential. The algorithm gate includes exhaustive binary16 inputs, canonicalization, formatter, vector, effect, evaluator, graph and floating host helpers. The negative control and recompiled mutants must be rejected by comparisons, not by a failed build.

## Authorized chip acceptance

After an authorized operator has completed the platform runbook, exercise the existing sm_121 vector execution route with the default Omega core bindings. Compare all outputs with the deterministic host oracle, including overflow probes and poison replacement; capture refusal controls as well as the green case. Pack and validate both target-specific cubin formats on the host, but do not load an sm_86 image onto an sm_121 device. An sm_86 execution check needs a separately authorized sm_86 machine.

A chip receipt must identify the exact execution command from the current platform runbook and record expected versus observed results, the negative control, final return code and device logs. If no authorized compatible device is available, retain **NOT_RUN**. Keep EXERCISED host/sim, QUALIFIED hardware, MERGED and DEPLOYED separate.
