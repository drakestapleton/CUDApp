# Chip qualification

NOT_RUN. No GPU authorization was granted to this lane. No device execution,
performance measurement, speedup, hardware accuracy or chip qualification is
claimed. Host simulation and generated-code checks are separate evidence tiers.

The host acceptance executables use link-time abort guards on session open,
lock, unlock and free (also allocation). Their `--sim` mode cannot reach a real
session. The explicit `--shell-contract` mode uses a memory-only fake backend
solely to prove pointer-success and failed-open/failed-allocation control flow.
A compiled direct-open source mutant must terminate with SIGABRT under `--sim`.

Hardware qualification remains the integrator/operator's separately authorized
responsibility. This lane does not run a full suite, merge or deploy.

## Operator-only runbook (NOT_RUN)

1. Obtain fresh authorization for an exclusive GPU window under the current
   fleet procedure. Confirm the integrator-approved source revision, service
   coexistence restrictions, device ownership and recovery plan. A historical
   receipt or apparently free lock is not authorization. Do not stop services
   or alter watchdogs, drivers, gates or boot settings to run this check.
2. Reproduce the host acceptance and byte-for-byte generation checks first.
   Build the distinct hardware executable without running it:

   ```
   bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 build/gpu_conv2d_chip_test
   ```

   This binary deliberately is not linked with fake-backend wrappers. It
   accepts only `--chip`; the normal headless binary does not accept that mode.
3. Only within that authorized window, with explicit permission to expose
   the real devices instead of the headless namespace, execute:

   ```
   taskset -c 10-14 nice -n 10 build/gpu_conv2d_chip_test --chip
   ```

   This is a future operator command, not a command authorized or executed by
   this lane. It uploads the real image/filter/code buffers and runs all
   storage types/layouts, grouped/depthwise/batched, padded/dilated/strided and
   tail cases against the same independent double reference. The wrong-group
   negative control must disagree. Any CODEGEN/CHIP error fails the check.
4. Record the exact commit, compiler and generated identities, device/driver,
   authorization receipt, command exit status and complete output. A successful
   line is `conv2d chip: PASS (... checks, 0 failures)`. Preserve failures;
   do not turn a missing device or aborted run into PASS, retry a full suite,
   or weaken admission/tolerance. Restore only the state authorized by the
   operator's recovery plan.

No timings are collected by this correctness run. PASS here would not establish
a performance result or qualify an untested shape/dtype outside these cases.
