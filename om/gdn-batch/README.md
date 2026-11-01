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
the binary opens the GPU. `GDN_BATCH_TEST_TIMING=0` omits optional timing reports;
absent or `1` requests them. An explicit timing request by the existing oracle
is always honored, including its timing loop. The host selector regression
checks all four combinations. This setting exists only in that test.

The complete 85-target host suite passes on production commit `09dbc67b`.
The test-only timing-selector correction passes the full dispatch target on
`e74b047f`. Physical reference, carried-state and next-decode checks pass in
timed and optional-untimed arms (54 checks each); the BF16 state arm passes
8 checks. All six complete-model score vectors and selected tokens match in
ordinary, untimed-batch and timed-batch arms. The independent frozen-score
comparison and damaged-output refusal also pass. See
[`followup-validation.json`](followup-validation.json) for exact identities.

These are shared-machine screens. The external selector exercises the production
Omega policy and native transport, but Core integration and server behavior are
NOT_RUN. Quiet performance qualification, independent review and deployment
remain open. No default selection changes.
Tracking issue: aien-dev/aien-sovereign-core-mv#710. The parent attention candidate
is CUDApp #260; this is a separate, optional follow-up.
