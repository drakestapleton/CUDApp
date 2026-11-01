# Scoped GDN prefill batching

This candidate asks Omega to direct the existing preparation, scan and output
kernels through one ordered command list. It reuses the existing input staging,
scratch, output poison check and recurrent state. It changes no kernel arithmetic
or reduction order. One list belongs to each GDN object and is reused until that
object is freed. Installation is serialized; a redundant allocation is freed
outside the session lock. The command-list transport refuses a list from an
earlier session generation.

`omega_gpu_gdn_prefill_scoped` is an experimental per-call entry. Mode 0 uses
the ordinary path. Mode 1 batches a supported call. Other modes refuse. Existing
host, recorded and decode entries retain their behavior. No environment setting,
native default pin or deployed service changes.

`plan.om` checks mode, token bounds and layer bounds. `step.om` admits each
recording operation in order, then submission, readback and publication. Output
cannot advance after a failed operation, wrong submitted count, incomplete
completion or missing semaphore. `stamp.om` checks four strictly ordered reports
and computes the three intervals. When timing is requested, those intervals
include ordering barriers. Total call time includes list creation and reuse.

Host policy and actual-transport checks pass. They include malformed arguments,
incomplete or failed submission, bad timestamps, failed readback, surviving
poison, list reuse, stale-list refusal, redundant installation and cleanup.
Independent policy and transport source mutations must fail the checkers.
Python and the fake C transport are external verification instruments only.

Run `make test-gdn-batch-policy test-gdn-batch-dispatch` with the repository's
pinned Omega compiler and Bridge. Both targets are in `make test`. Build
`build/gdn-batch-chip` explicitly for the existing independent physical GDN
reference, recurrent-state, tail, next-decode and damaged-kernel checks redirected
to this scoped entry. `--host-only` and `--sim` remain host-only; without them
the binary opens the GPU. `GDN_BATCH_TEST_TIMING=0` selects the untimed test arm;
absent or `1` selects the timed arm. This setting exists only in that test.

Physical qualification, complete-model parity, server behavior and quiet timing
for this production candidate are NOT_RUN. Earlier external command-list screens
motivated the change but do not qualify it. Core integration is not yet present.
Tracking issue: aien-dev/aien-sovereign-core-mv#710. The parent attention candidate
is CUDApp #260; this is a separate, optional follow-up.
