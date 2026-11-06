<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->

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
| matmul.c `omega_matmul_cpu_oracle_i32/f16/bf16` | bw_cpui/bw_cpuf16/bw_cpubf16 runtime views; bw_scalar16/17 extent arithmetic | null guards and ABI calls; C loops only freestanding/default-OFF oracle |
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
| native_runtime.h `omega_native_admitted` | bw_name bounded terminator scan, bw_scalar14 size admission, native_templates/native_validate, bw_scalar8 | pointer guards, fixed wire projection; strnlen only freestanding/default-OFF oracle |
| native_runtime.h `omega_native_launch` | native_binding, bw_scalar8 | pointer projection |
| native_runtime.h `omega_native_bound` | existing payload/lease units and bw_scalar4 charge | pointer guards and fixed request packing |
| matmul.h / realize.h / gpu_platform.h / gpu_cmdlist_native.h other content | declarations/types/constants only | no further executable decision/arithmetic/encode functions |

New omega_bw_policy.h is ABI/pointer/bit glue, not a second policy implementation.
Its failed scalar ABI trap is fail-closed; identity maps checked status through
bw_scalar8. Default-OFF C_BIND preserves the #309 C oracle. Independent frozen C
matmul/realize sources live under tests and compile only when
CUDAPP_MIGRATION_PENDING_BW_MATMUL=1 is explicitly requested by the differential.
The independent policy oracle is also only in the guarded differential test.
The final fixed snapshot-name scan is registered separately in a1e2601, using
the published hosted checked-view profile. `CUDAPP_MIGRATION_PENDING_BW_NAME`
is default-OFF. `OMEGA_GPU_NO_POSIX` keeps the explicit bounded strnlen exception
pending omega-bridge#72; no #72 converter or compiler is adopted.


## Converter pin and explicit freestanding exception

Main explicitly authorized bridge#73 at 79e8c3613a6f6f9e8eaaa9c013fc25bcb39e0538.
Separate pin commit e2c0e70 updates the lock and regenerates the existing fixed-view
units. Registration commit 31ce9df adds the hosted CPU runtime-view units. The PR
is pending omega-bridge#71 and #73 merge. The seeded compiler remains e5b32567.
The generator uses the published `bridge-seeded --runtime-views` profile, not a
private generator convention. Numeric fixed-view units retain their existing path.

Under OMEGA_GPU_NO_POSIX, checked hosted views cannot provide aggregate SHA/binding
or runtime CPU-view units. The C_BIND encoding/hash path and the three named CPU
reference loops remain explicit **freestanding exceptions pending omega-bridge#72
and an authorized pin**. No #72 pin is adopted. Hosted/default uses the registered
.om units with checked SHA identities. Scalar half/bfloat conversion and native
allocator/header policies use import-free .om on both builds.

The hosted CPU loops use three unsized typed views (six physical slots). For
positive dimensions, a=m*k, b=k*n, c=m*n imply n²=b*c/a. The shape helper cancels
gcd(a,b), then gcd(the remaining a,c), refuses an uncancelled denominator or
overflow, and takes an integer square root. It verifies an exact square, exact
k=b/n and m=a/k divisions, all three extents, and u32 dimensions. This avoids
overflow even for general u32 dimensions; no envelope restriction or extra shape
ABI is introduced. Zero dimensions retain the original -1 refusal.

The old C arithmetic wraps uint32 input/output indices and integer accumulation.
The .om dot helpers preserve those masks explicitly; integer multiply/add widens
to ulong before masking. Float operations round separately through the generated
volatile f32 arithmetic helper. Literal u32/u64 loop bounds plus explicit extent
returns avoid the seeded one-million guard on implicit runtime-valued `for` loops,
without changing a compiler limit or a gate. A dot beyond that guard is exercised.
The pressure fix follows the consultant's decisive point: pass the flat output
index to the dot helper instead of packing coordinates in the call expression.

### CPU reference buffer contract

Output must be disjoint from both inputs; read-only inputs may overlap. Main
approved this checked-view contract after a caller audit. submit.c:405/648 allocates
separate h_expected buffers; cubin_probe.c:120-121 separately allocates A/B/ref;
regalloc_om/sim.c:42/54 declares separate inputs and expected arrays; the focused
differential uses distinct arrays. No caller supplies aliased output. Direct ABI
and all three public wrappers have alias-refusal red controls. Header and API.md
section 10 carry the contract. These are INTERNAL verification helpers, not
manifest entries, so the public manifest is intentionally unchanged.

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
CPU tests cover deterministic rectangular shapes with random integer/half/bfloat
contents, NaN-class comparison, all null/zero refusals, full-u32 shape admission,
and invalid reconstructed extents (non-square quotient and k mismatch), without
touching the output on refusal. The original C loops remain independent oracles.
Dispatch covers all three precisions and invalid precision, nulls, byte equality
and free(NULL). There is no device access.
The snapshot-name scan checks every possible terminator position in its 64-byte
field against the independent C strnlen oracle, plus unterminated and short-view
refusals. Its terminator-predicate mutant must compile and fail the differential.

## C-oracle coverage

The O0 `--coverage` receipts are `repeat-om_bw_matmul_oracle.c.gcov` and
`repeat-om_bw_realize_oracle.c.gcov`. Matmul reaches every line, branch and branch outcome.
Realization reaches every line and branch site. Three refusal outcomes cannot be
selected by any caller input:

- Oracle line 40: `omega_blackwell_get_machine_id(&out_real->machine_id)` cannot
  receive NULL after the outer `out_real` guard; this callee only refuses NULL.
- Oracle line 48: the fixed vector encoder receives a non-NULL stack buffer with
  exactly `OMEGA_BW_VECADD_CODE_SIZE` capacity and has no caller-dependent input.
  Its failure would require corrupting the fixed encoder or its generated ABI.
- Oracle line 49: digest input/output pointers are non-NULL stack/member storage,
  and the successful fixed encoder supplies its known nonzero code length.
  The digest helper's NULL/zero-length refusals are therefore unreachable here.

These are justified unreachable outcomes, not claimed covered or killed mutants.
The live encoder's guards are in `src/omega_blackwell_encoder.c`; no gate or
production behavior is weakened to force them.


WolframLanguageEvaluator queries:
- `Integrate[x^2,{x,0,1}] -> 1/3` (mount verification).
- `127-15+1 -> 113` (binary16 subnormal exponent base).
- `{2^64-1,2^64-4096,2^63,2^32-1} -> {18446744073709551615,18446744073709547520,9223372036854775808,4294967295}`.
- `{FullSimplify[(k n) (m n)/(m k),Assumptions->m>0&&k>0&&n>0],(2^32-1)^2,2^64-1,8192*16384*262144^2,2^54,2*3/1,1*8/2} -> {n^2,18446744065119617025,18446744073709551615,9223372036854775808,18014398509481984,6,4}`. The stated envelope product is 2^63, not 2^54; gcd cancellation handles the broader u32 domain.
- `{(2^32-1)^2+(2^32-1),(2^32-1)^2+(2^32-1)<2^64} -> {18446744069414584320,True}` bounds the widened integer multiply/accumulate.
All test counters are integer bookkeeping, not modeled performance.

## Final host receipts

All commands below use the exact bwrap/taskset/nice prefix in Reproduction and
the pinned environment below. `hosted-final-gates.log` records this complete
command with rc 0:

```
sh om/units.sh --check &&
make -j3 test-om-bw-matmul test-om-bw-matmul-mutants \
  test-om-bw-matmul-coverage test-gpu-code-alloc test-gpu-cmdlist \
  test-gpu-decode-graph test-gpu-native-runtime test-gpu-engine-sim \
  test-gpu-matmul-api test-native-artifact-manifest test-codegen-freestanding
```

Observed: byte-for-byte regeneration PASS; 12 compiled semantic mutants killed;
the differential red binary exits 1 as required; existing requested oracles pass;
matmul API 178 checks and its R4 receipt 7 checks pass; native session platform
27 passed/0 failed; all bf16 bit patterns match the hosted cast. The final line is
`PASS: freestanding codegen contract (14 fixtures, 0 failing checks)`.

That run exposed a stale gcov profile checksum warning after the test source
changed. Commit 8a5930f fixes the owned coverage target by clearing only its own
`diff-*.gcda` files before each run. `coverage-repeat-regression.log` records the
post-fix command with rc 0, including the added partial-output-alias refusal and
read-only-input-alias success cases:

```
make -j3 test-om-bw-matmul test-om-bw-matmul-mutants \
  test-om-bw-matmul-coverage &&
make -j3 test-om-bw-matmul-coverage
```

Observed final differential: `PASS bw matmul realization differential checks=521475`.
Red check killed; 12 mutants killed; repeated coverage has no checksum warning.
Both coverage runs report matmul 100% of 112 lines and 96 branch outcomes,
realization 100% of 32 lines and branch sites, 81.25% of 16 branch outcomes,
with the three unreachable refusal outcomes justified above. The final line is
`Lines executed:100.00% of 144`. Earlier receipts are retained separately rather
than rewritten. The name-unit development failures (compiler rc 16 for reserved
`out`, then converter rc 24 for the legacy profile) were fixed before these gates:
the local is `dst` and the registered profile is `--runtime-views`.

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

Generation/mutants use OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc,
OMEGA_BRIDGE=~/work/cudastack/lanes/BridgeDynViews and
OMEGA_SEED=~/work/cudastack/omega-bridge/build/bootstrap/omc. The pinned omc is
e5b32567, SHA256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f.
Focused acceptance targets are in mk/om-bw-matmul.mk. Integrator3 owns the full
integration gate and the cross-repository harness converter-guide refresh.
