# Model graph hardware qualification

Status: **NOT_RUN**. No GPU, live RM, VFIO, driver, module or service window is authorized for this lane. Do not touch ports 18091 or 18173–18178, PID 68451, omega-tool services, the GB10 lock, NVIDIA modules or VFIO.

## Before requesting a window

1. Finish actual nested-recorder byte parity for every supported Q36/Q38/decode configuration, including exact heads, scalar and batched heads, prefill mixer/MoE windows, all sequence boundaries and at least 10,000 generated configurations/positions.
2. Run generated-unit byte checking, five recompiled semantic mutants, record-flip controls, focused host checks and warp simulation. Every host/test/sim/mutant command must use `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 ...`.
3. Integrator runs the integrated full gate, at most once with a 3600-second cap. A timeout is INCONCLUSIVE, not PASS.
4. Obtain exact current operator authorization through the current hardware-window procedure. A historical receipt or free-looking lock is not authorization.

## Authorized hardware procedure

Use the repository's existing model runners and pinned model artifacts. For old C and default Omega builds, save complete model configuration, source/compiler/artifact hashes, input token/sequence positions, recorded list bank/QMD metadata, queue submissions per generated word, output tokens and error state. Compare those records and token outputs before claiming parity. Include GDN and attention layers, Q38 BF16 split-down, exact output-head configurations and Q36 prefill/MoE paths. Use the same non-contracting floating-point oracle settings where appropriate.

Compare one complete generated word's actual queue-submission count, not merely the number of calls into a recording wrapper. Record timeline/ordering and semaphores; a host fake-platform result is not chip qualification. Run an authorized negative control and verify the output/list discrepancy is detected without weakening an admission gate. Restore only the specifically authorized service state, recording before/after evidence. Do not merge, deploy or switch serving as part of this lane.
