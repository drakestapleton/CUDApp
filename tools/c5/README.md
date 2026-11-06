# C5 sharing and D14 recovery

## What existed

At CUDApp `b774723f537cec16a359fe082b8100317a5b5490`,
`tests/gpu_isolation_test.c` already exercises two chip processes, a real
nonterminating kernel, a latched session, device-close recovery, a successful
new launch in the same process and refusal of stale command lists.
`src/omega_gpu_session.c` parks uncertain resources and only clears its latch
after platform device teardown succeeds. At that revision its marker waits
each received a timeout independently, not an absolute submission-wide deadline.

`build.sh` builds that test, a wrong-values mutant, and `c5_matrix.om`.
`chip.sh` runs the hardware matrix beside a PyTorch CUDA tenant.
`selftest.sh` uses stand-in processes to check orchestration only. Neither the
stand-ins nor a process-group kill prove same-process channel recovery.

## Added policy and model

`policy.om` contains pure decision functions for an overflow-checked absolute
submission deadline, completion/fault observation, admission, process/context
handle ownership, bounded memory slices and acknowledged recovery sequencing.
`model.om` is a deterministic host RM/channel simulator consuming those
functions. It is not the Blackwell instruction simulator and does not execute
SASS. A nonterminating simulated kernel never emits completion; a working
kernel stores eight words and emits two completion markers. The original
deadline covers both markers. Timeout wins at the deadline, including a
simultaneous completion. A backward clock observation latches the context.

The model has two independently owned contexts, channels and private mappings
at identical virtual offsets. Handles carry process identity and context epoch.
Cross-owner access and stale generations are refused before touching memory.
Recovery refuses admission throughout these acknowledged actions:

1. Latch admission.
2. Quiesce the submission queue.
3. Destroy all channels of this process.
4. Destroy the RM context/client, invalidate mappings and advance the epoch.
5. Recreate the context.
6. Recreate its channel.
7. Rebuild private mappings, then reopen admission.

Failed acknowledgements leave the state and resources unchanged. Generation
exhaustion refuses recreation rather than reusing old tokens. No simulated
mapping is reclaimed while the old grid can still run. The other process's
memory and channel remain untouched.

`runtime.om` adapts these decisions to the hosted and freestanding production
session through the import-free generated `omega_native_c5` binding. The default
path computes one overflow-checked absolute nanosecond deadline before ringing
the doorbell. Both markers and the spin window share it; completion at or after
the deadline latches. Polls use one millisecond only when it fits, otherwise a
zero-ms poll until the remaining sub-ms interval expires. A backwards clock
latches. Timeout still parks resources: it never guesses that an old grid died.
The existing explicit `omega_gpu_session_recover()` acknowledges the platform's
bundled quiesce/channel/client close through the Omega recovery states. A failed
close cannot clear the latch; a successful close permits the next `open()` to
recreate the device/channel in the same process. No new RM ioctl contract is
introduced. The old C timing/recovery decisions remain only behind
`CUDAPP_MIGRATION_PENDING_C_SESSION_TIMEOUT=1` (default OFF).

The real session fake-RM fixture includes marker releases at 3 ms and 6 ms with
a 5 ms submission budget: the new path times out, explicitly recovers, then
successfully submits again in the same PID. The old per-marker path incorrectly
succeeds and is required to fail that exact case as a negative control.

## Host acceptance

```
OMEGA_OMC=/path/to/pinned/seed/out/omc \
  taskset -c 10-14 nice -n 10 make -j3 test-c5-policy test-gpu-session-platform
```

`policy-test.sh` checks `omc.pin`, compiles the concatenated Omega units and
requires the host-reference and simulator assertions to succeed. It then
compiles each mutant and requires its exact assertion exit, not a compiler
failure or arbitrary trap. Mutants cover deadline equality, overflow, owner,
epoch, range, failed teardown acknowledgement, action order, epoch advance and
stale-channel completion. Logs and source/binary hashes are in
`build/c5-policy/`. The target is in `make test`; without an explicitly runnable
pinned compiler it is target-only on non-AArch64 hosts.

Boundary vectors were checked with WolframLanguageEvaluator:
`{2^64-1,(2^64-1)-8,10+8,17-10,18-10,2*8,Table[100*p+i,{p,1,2},{i,0,7}]}`
returned `{18446744073709551615,18446744073709551607,18,7,8,16,
{{100,101,102,103,104,105,106,107},{200,201,202,203,204,205,206,207}}}`.
Times in the deterministic simulator are logical ticks, not measured hardware
nanoseconds. State numbers and assertion exits are enumeration labels.

Additional WolframLanguageEvaluator boundary queries:
`{Mod[(2^64-1)+2,2^64],10+9,10+20,30+20}` returned `{1,19,30,50}`.
`{Floor[(2^64-1)/10^6],10^6,25*10^6,5*10^6,3*10^6,Floor[(5*10^6-3*10^6)/10^6]}`
returned `{18446744073709,1000000,25000000,5000000,3000000,2}`.
Here the conversion factor is nanoseconds per millisecond; runtime timeout
vectors are nanoseconds. The delayed second marker is 6 ms, outside the 5 ms
budget, but only 3 ms after the first marker.
`{6*10^6,6-3,6-5}` returned `{6000000,3,1}` (nanoseconds, milliseconds,
milliseconds respectively).

## Chip runbook: NOT_RUN

Only Drake's current exclusive GPU-window procedure authorizes these commands.
The lock alone is not authorization. Do not run them while a serving tenant
must remain live. Keep all existing receipts immutable; use a new output path.

Outside the GPU hold, build the existing chip programs:

```
OMC=/path/to/pinned/seed/out/omc \
  taskset -c 10-14 nice -n 10 sh tools/c5/build.sh
```

Inside the authorized window, first run the two real CUDA++ processes and real
hang test without depending on PyTorch:

```
timeout -k 10 300 build/gpu_isolation_test --values-only
timeout -k 10 300 build/gpu_isolation_test
```

Require the wrong-values mutant to exit 1 with a wrong-values diagnostic:

```
timeout -k 10 300 build/gpu_isolation_test_mutant --values-only
```

The full existing coexistence/pressure/kill matrix additionally needs PyTorch
as a verification-only CUDA tenant, not a production dependency:

```
PY=/path/to/torch/python sh tools/c5/chip.sh /new/receipt/path
```

Capture source revision, compiler/source/binary hashes, command exits, final
verdicts and kernel logs. The hang test must show timeout, refusal while
latched, confirmed teardown, correct new launches in that same child PID,
refusal of pre-recovery command lists, and an unaffected second process.
A teardown failure is FAIL and must remain latched; do not waive it or restart
the process and call that same-process recovery. None of these chip commands
were run for this policy/model implementation. C5/D14 hardware status is not
upgraded.
