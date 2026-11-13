# Review lessons

Before requesting review of the native AMD model path, run the launcher gate on
the complete release tree. Its Qwen object manifest must rebuild byte-identically
to the objects exercised on the card. The notebook once carried device-stepped
kernels that were missing from the proposed release tree; smoke tests alone did
not detect that gap. The enforced source-manifest check catches missing kernels,
changed object bytes and changed kernarg sizes before review. Update its fixture
only with a new corresponding hardware receipt.

## Packet metadata must meet an independent ABI

AMD kernel descriptors put group then private sizes at offsets 0 and 4. HSA
dispatch packets put private then group at offsets 24 and 28. A test that copies
the launcher's offset assumptions can certify the same mistake. The launcher
gate now reuses the separately gated backend/amd/kfd/aql.om encoder for a byte
comparison with an observed published packet. Accepted nonzero-LDS ELF objects
exercise both generic and real plan-template paths; swapping either writer
must be caught. Zero-size model kernels alone cannot qualify these fields.

## Trace the source conversion before changing a topology formula

ROCT topology converts sysfs array_count / simd_arrays_per_engine into
NumShaderBanks; queues.c divides that by NumArrays again. Both native divisions
are deliberate. The array-bound-dominant fixture and single-division mutant
exercise this mapping instead of relying only on small CU-limited fixtures.

## Mutation controls must reach the intended parser on every architecture

An a64 command prefix retained the original plan path before the mutant. That
rejected the argument count instead of checking the malformed plan. Build the
executable/mode prefix separately from input paths. The malformed plan table
now reaches the parser on native a64 and qemu-x86_64. Do not infer a meaningful
negative control from an exit code alone.

## A measurement alarm is not an execution policy

Plan and vadd have no automatic deadline unless the caller supplies one. Generic
argument-file timeout zero also means unlimited. A 10 s warning remains visible
during a wait, once per dispatch, without preventing a slow correct operation.
The gate exercises default/explicit-zero slow completion, explicit timeout
refusal and malformed timeout inputs; its external watchdog is unchanged.
Raw timing summaries are recomputed from step records before receipt acceptance.

## RCCA launcher round 2: independent controls and observable refusals

The packet fix introduced an ABI reversal and its tests copied that belief.
Both packet writers now run through the independently gated AQL encoder with
private=20 and group=4096 as well as private=0. The batch publication test calls
plantemplate itself. Real ELF admission still refuses nonzero private segments;
that refusal has an explicit gate row. Do not infer scratch-memory support from
the synthetic ABI unit.

The a64 plan negative table once rejected its own valid twin due to a duplicated
argument. Every refusal now shares the argv builder with a freshly run positive
twin reaching open_kfd, and asserts its specific machine-readable parser reason.
The row receipt retains commands, timestamps, raw logs and digests. Removing a
parser check must fail its reason assertion, even if a later check also exits64.

A presence-only alarm test missed false alarms and absent re-arming. Fast native
generic and plan simulations now assert no alarm. Repeated slow generic dispatches
and three plan steps assert one alarm per dispatch. Threshold-zero and missing
re-arm mutations are explicitly detected. The simulated clock advances at each
published batch and stays monotonic across start, wait and completion reads.

Producer-source references are vendored unmodified with their own licenses,
URLs and SHA256SUMS under tests/amd-kfd-launch/reference. The gate checks their
bytes and the cited mapping lines offline. External field derivations require
producer source, not an inference from field names.

The CA6 close-out deleted each individual plan guard and found nine reachable
boundaries without rows. The table now covers data layout, empty names, buffer
size, zero/data-buffer indices, relocation buffer, grid, relocation count and
final dispatch end. The word-width helper guard has a separate explicit unit;
production plan calls use constant widths, so no fake input row claims to reach it.
Every malformed numeric deadline also has a fresh valid zero twin and a specific
reason from the existing shared unsigned parser, preserving exit2. Its six refusal
sites are tested, including a too-long argument that argread already refuses.

Reference URLs, digests and raw-file ranges now have one canonical manifest,
reference/sources.json. The existing gate verifies each quoted range and checks
that SHA256SUMS and the README table are exact derived views. PROVENANCE points
to the manifest instead of maintaining another copy of the raw line numbers.

The shared CLI fragment is appended after main by the original KFD seed tools.
New diagnostic globals broke that compiler mode, and stdout diagnostics polluted
binary packet output. Keep the diagnostic label local, preserve empty payload
stdout on refusal, and send errors to stderr through the full-write helper.
Before reviewing a shared CLI change, run both the pinned older KFD gate and the
new launcher gate. Numeric field IDs0..5 identify the six existing parser checks;
the plan table verifies every ID on stderr and asserts stdout stays empty.
