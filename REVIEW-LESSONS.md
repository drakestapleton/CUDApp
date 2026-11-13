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

`make test-amd-release` is the repository's combined pre-review gate. It runs the
original compiler's test-amd suites before the newer launcher's gate, so a shared
CLI change cannot pass release readiness by testing only its newer caller. Keep
both pins explicit; hardware qualification remains a separate exact-binary check.

Reference ranges are derived from unique first/last statement anchors in the
pinned raw files. The gate compares the generated README table with those actual
line positions, so merely widening a stored numeric range cannot pass.

A defensive helper unit is still a negative-control row. Declaring its invalid
input unreachable from the public parser does not exempt it from a real valid
twin. The word-width unit once left its map handle uninitialized: invalid width9
refused64, but legal width8 trapped103. Its guard-removal result therefore did
not establish a meaningful positive baseline. The shared word_width_unit builder
now initializes real mapped data for both paths; legal widths1,4,8 must read the
known bytes successfully before width9's specific refusal counts. The ring helper
has its own successful boundary table before the unrepresentable-size refusal.

The positive-twin rule must be enforced over both launcher gate result files,
not inferred from labels or checked only in the plan parser. Each now declares
success_exits and cases, and every refusal names a fresh passing twin with a
verified value, matching command setup and one changed input. This also covers
semantic mutants that exit zero. Inventory controls reject missing twins, trap
exits103/104, unrelated command setups, mismatched setup hashes and wrong kinds.
Both files use the same object envelope; consumers read .cases.

Search all CONTROL and non-success rows before review. That inventory exposed
fixtures where the valid timeout twin also failed: use the same simulated clock
and a deadline above one completed step, changing only the missing completion
fault for the refusal. Start output checks without a prior result file. The
argument-header pair now preserves a valid manifest except its magic, and the
ring helper has a separate largest-representable twin with the same single-count
setup. Refusal credit requires a twin from that same execution, not a compiler
success or an unrelated boundary table. An unexpected trap is a failed gate,
not a credited refusal; classify its mapping/bounds cause before a mutant claim.

A zero private_segment_fixed_size does not prove a descriptor needs no scratch.
The final self-check reproduced five independently enabled scratch ABI flags
accepted by the older launcher. Test each request in isolation, preserving the
valid vadd descriptor as the same-run positive twin. The launcher's queue has no
scratch backing, so refuse resource2 private-enable and properties private-buffer,
flat-scratch-init, private-size and dynamic-stack requests before allocation.
Pin the producer bit definitions, not just prose field names. A runtime admission
change produces a new executable: prior card receipts remain historical until
that exact executable is qualified on the card.
The word-width and ring-limit composed units now use identical assertions on both
paths. The legal-width8/illegal-width9 sources differ only in the pw input literal;
the largest legal/illegal ring sources differ only in their dispatch-count literal.
Do not remove the value assertion from the refusing source: rejection happens
before it. A direct source-difference check and fresh native runs verified each
pair succeeds0/refuses64.

CA12: a passing twin is necessary but a shared exit code alone cannot identify
the refusal site. Object guards now emit distinct stable field IDs on stderr
while preserving exit65 and empty payload stdout. The ELF mutation and missing
symbol rows assert the exact ID; manifest magic asserts its own stderr reason;
the ring-limit unit asserts field=ring-size. Search covered all existing exit65
sites in common.om/elf.om and all check.py inspect refusals, including the new
scratch requests on both targets. Keep raw class exits stable and test the
reason too, so a later guard cannot conceal a missing earlier one.
