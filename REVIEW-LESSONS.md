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
