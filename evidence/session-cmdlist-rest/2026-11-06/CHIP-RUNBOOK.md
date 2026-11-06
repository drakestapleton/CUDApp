# Chip runbook: session and command-list decisions

Status: NOT_RUN. No GB10 window was authorized for this lane. Host and simulator results are not chip qualification.

## What a window would exercise

The Omega units are pure computation. The chip question is whether the C shell that calls them still drives the device correctly: session open, one launch, command-list submit, and recovery. The decisions themselves do not touch the device.

## Preconditions

- An exclusive window authorized by Drake, with the live server on :18091 confirmed down and the GB10 lock free. This runbook does not take that lock and does not stop any process.
- Do not touch :18091, :18173-18178, PID 68451, /tmp/aien-gb10.lock, the nvidia modules, vfio, or /dev/nvidia* outside the authorized window procedure.
- Build at most -j3, on cores 10-14.

## Steps

1. Rebuild the engine in the window with the pinned omc:
   `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-gpu-engine-gb10-compile`
2. Run the device-free gates first, so a chip failure is not a logic failure:
   `make -j3 test-om-cmdlist test-om-streams test-gpu-cmdlist`
3. On the chip, open one session, submit one recorded command list, and confirm the completion semaphore advances and the session closes without a latched teardown fault. Record the platform diag text on any refusal.
4. Negative control: submit a list recorded before a forced recovery and confirm it is refused as stale, then confirm a fresh list submits.

## Record

File the chip receipt under evidence/session-cmdlist-rest/<date>/chip/ with the window id, the omc sha256, and the raw diag text. Label it QUALIFIED only after that receipt exists. Until then the tier is NOT_RUN.
