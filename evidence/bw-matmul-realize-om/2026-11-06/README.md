# Blackwell matmul/realization and native-header migration

IMPLEMENTED and host EXERCISED; not chip QUALIFIED, MERGED or DEPLOYED.
Base b774723, stacked on regalloc #309 at 63d18a2. The first registration commit is
46b6286. The stack merge is b1c4232. No API signature or wire structure changes.

## Ownership and overlap

The #310 PR is not the cmdlist-program port. The actual cmdlist-program PR is
#304 (581c048); it does not modify the assigned native/codelist/allocation headers.
EngineSubmit2 owns platform.c and engine/submit/wait/memory/serving implementations.
Their source is unchanged here. #309 already owns the `omega_cg_bind` encodings,
validation, dispatch and vector launch geometry. This branch uses that unit rather
than introducing a second encoding. It adds hashing of the cg_bind wire only.

## Complete executable inventory

| Assigned file/function | Default hosted owner | C remaining |
|---|---|---|
| matmul.c `omega_matmul_spec_init` | cg_bind kind 0; bw_identity SHA | null guard, struct stores |
| matmul.c `omega_fp16_to_fp32` | bw_scalar 1; bounded scalar normalization, no loop | bit memcpy |
| matmul.c `omega_bf16_to_fp32` | bw_scalar 2 | bit memcpy |
| matmul.c `omega_fp32_to_fp16` | existing native_f16 | bit memcpy |
| matmul.c `omega_fp32_to_bf16` | bw_scalar 3 via rne wrapper | bit memcpy |
| matmul.h inline `omega_fp32_to_bf16_rne` | bw_scalar 3 | bit reinterpretation |
| matmul.c `omega_blackwell_bind_matmul_realization` | cg_bind kinds 1/2/9; bw_identity SHA | fixed copies and pointer checks |
| matmul.c `omega_blackwell_codegen_matmul` | cg_bind kind 3 | dispatch to existing generated encoders |
| matmul.c `omega_blackwell_kernel_free` | host allocation release | pointer check, free, memset |
| matmul.c `omega_matmul_cpu_oracle_i32/f16/bf16` | named C verification oracles, per Main decision | verification-only reference loops, never production dispatch |
| realize.c `omega_blackwell_get_machine_id` | cg_bind kind 2; bw_identity SHA | null guard |
| realize.c `omega_blackwell_realization_compute_id` | cg_bind kind 1; bw_identity SHA | fixed wire copies |
| realize.c `omega_blackwell_realize_vector` | cg_bind kinds 4/9 and existing Omega vector encoder | struct projection, pointer guards |
| code_alloc.h `omega_gpu_code_alloc_bytes` | bw_scalar 4 | size_t conversion; default-OFF existing NO_TAIL mutant selector |
| native_runtime.h `omega_native_cache_call` | existing native_cache; bw_scalar 7 | pointer projection and signed bit copy |
| native_runtime.h `omega_native_context` | fixed native ABI fields | memset, epoch host call, architecture constant |
| native_runtime.h `omega_native_cache_refresh` | native_cache op17 | fixed request packing |
| native_runtime.h `omega_native_shape_key` | bw_scalar15 bounds, existing native_key, bw_scalar8 status | pointer guard and bounded memcpy |
| native_runtime.h `omega_native_cache_reserve` | native_cache18, bw_scalar9 slot normalization | fixed request packing, reservation-owner load |
| native_runtime.h `omega_native_cache_finish` | bw_scalar10 selects commit/abort; native_cache | fixed token packing |
| native_runtime.h `omega_native_cache_victim` | bw_scalar11 selects victim/erase; native_cache | fixed token packing |
| native_runtime.h `omega_native_cache_lease` | native_slotrev, bw_scalar12 | lease struct construction and epoch host call |
| native_runtime.h `omega_native_live_call` | native_lease, bw_scalar13 boolean, bw_scalar7 status | pointer guards, token packing, signed bit copy |
| native_runtime.h `omega_native_lease_op` | preceding helpers | request storage |
| native_runtime.h `omega_native_payload` | native_validate, bw_scalar8 status | pointer guard |
| native_runtime.h `omega_native_admitted` | bw_scalar14 size admission, native_templates/native_validate, bw_scalar8 | pointer guards, bounded string length for memcpy, fixed wire projection |
| native_runtime.h `omega_native_launch` | native_binding, bw_scalar8 | pointer projection |
| native_runtime.h `omega_native_bound` | existing payload/lease units and bw_scalar4 charge | pointer guards and fixed request packing |
| matmul.h / realize.h / gpu_platform.h / gpu_cmdlist_native.h other content | declarations/types/constants only | no further executable decision/arithmetic/encode functions |

New omega_bw_policy.h is ABI/pointer/bit glue, not a second policy implementation.
Its failed scalar ABI trap is fail-closed; identity maps checked status through
bw_scalar8. Default-OFF C_BIND preserves the #309 C oracle. Independent frozen C
matmul/realize sources live under tests and compile only when
CUDAPP_MIGRATION_PENDING_BW_MATMUL=1 is explicitly requested by the differential.
The independent policy oracle is also only in the guarded differential test.

## Explicit exceptions and concrete bridge proposal

Under OMEGA_GPU_NO_POSIX, the portable view converter cannot provide the aggregate
SHA/binding units. The pre-existing C_BIND encoding/hash path remains an explicit
**freestanding exception pending omega-bridge#72 and an authorized pin**. The
hosted/default build uses the registered .om units and checked SHA identities.
No draft bridge pin is adopted. Scalar half/bfloat conversion and allocator/header
policies use import-free .om on both hosted and freestanding builds.

Main authorized keeping the three named CPU verification loops in C. Bridge#71 at
758d74aa validates its fixed numeric view lengths by equality, not by a runtime
extent. #319 (3139ba3) generated acc/epi/fp8 wrappers confirm fixed capacities, so
those cannot honestly represent arbitrary caller buffers. Proposal sent to
BridgeDynViews: runtime-extent typed views must carry the caller's live element
count, use it in len/get/put and reject null/nonempty, misalignment and aliases;
provide read uint/ushort and edit float views with no FMA contraction. The proposed
unsized ABI uses two native slots per view: three matrix views consume all six
slots, leaving no m/k/n shape descriptor. Supporting this kernel additionally
needs an authorized shape-metadata ABI that stays within the seeded six-slot
limit; no new generator convention is invented here.

## Case partition and numerical rules

Differential: every binary16 input, every bf16 expansion input, targeted f32
zero/subnormal/normal/tie/overflow/infinity/NaN boundaries and deterministic f32
bit samples. Non-NaNs compare exact bits; NaNs compare class. Compilation uses
-ffp-contract=off. Spec dimensions cover zero, UINT32_MAX and each extent limit
plus neighbors. Hash checks cover every wire length from zero through 256, refusal
257, short extent and alias rejection. Realization inputs vary every struct byte,
including architecture, before comparing the complete initialized result.
Vector launch cases cover zero, block boundary and uint32 wrap. Allocation cases
cover tail/page boundaries and overflow; scalar status/lease/admission predicates
cover zero, signed limits, both decisions, invalid op and every admission bound.
C CPU oracles cover a rectangular shape and all pointer/zero-extent refusals.
Dispatch covers all three precisions and invalid precision, nulls, byte equality
and free(NULL). There is no device access.

WolframLanguageEvaluator queries:
- `Integrate[x^2,{x,0,1}] -> 1/3` (mount verification).
- `127-15+1 -> 113` (binary16 subnormal exponent base).
- `{2^64-1,2^64-4096,2^63,2^32-1} -> {18446744073709551615,18446744073709547520,9223372036854775808,4294967295}`.
All test counters are integer bookkeeping, not modeled performance.

## Chip runbook — NOT_RUN

No GPU/window permission exists. The host gates always hide devices using the
bwrap command below. Integrator3 owns full make test; it is not run by this lane.
After an explicitly authorized exclusive chip window, the operator must use the
current GPU/session gate and pinned firmware/driver/board identity, then run the
existing real matmul/vector execution oracles with the three precisions, compare
output to the named CPU oracles, record actual-versus-expected refusal cases and
receipts. This document grants no live-service, lock, driver or GPU authority.

## Reproduction

Every test/generator gate is run with:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 COMMAND
```

Generation/mutants use OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc and
OMEGA_BRIDGE=~/work/cudastack/omega-bridge, the existing pinned converter.
The pinned omc is e5b32567, SHA256
2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f.
Focused acceptance targets are in mk/om-bw-matmul.mk. The source inventory and
pin are unchanged by this port, so no harness language/converter guide changes
are required.
