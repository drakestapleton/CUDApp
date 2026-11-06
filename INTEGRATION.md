# Omega CUDA-stack integration

Tracking branch: `integ/omega-cuda-stack`. Base: `b774723f537cec16a359fe082b8100317a5b5490`.

This branch is not permission to merge or deploy. Host acceptance does not qualify GPU behavior. No GPU window is authorized.

| Branch | SHA | PR | Merged-at | Focused verdict | Conflicts resolved |
| --- | --- | --- | --- | --- | --- |
| feat/omega-codegen-a | af70a66cf94e85042e508c08a6e7d6780e6b544f | #275 | 2026-11-05T20:41:13-06:00 (`b7af547`) | PASS: completed namespaced QMMA/TGEMM receipts below; prior unnamespaced attempt INCONCLUSIVE/interrupted | None |
| feat/omega-codegen-b | f219a04eff8ff28e2d8d0b8cd65f210e552bbd54 | #274 | 2026-11-05T21:10:38-06:00 (`52d071e`) | PASS: pf copy 18 instructions, mutant caught (`/tmp/integ/codegen-b-tern.log`) | Additive units/header/identity entries retained; codegen crumb regenerated |
| feat/omega-codegen-tern | bc22ce1e75883eeea7bdafd11f37909a5bbf0247 | #279 | 2026-11-05T21:12:39-06:00 (`9dcefcc`) | PASS: 213120 programs, 14258 refusals; 24 source-mutant controls; simulator 34 checks (`/tmp/integ/codegen-b-tern.log`, rc 0) | Additive registrations retained; missing common-source composition repaired |
| feat/c4-streams-om | daee8b6fc054df8fe40830a04127f10e21ba9184 | #272 | 2026-11-05T21:47:59-06:00 (`7eac35d`) | PASS: host/simulator/differential and four source mutants (`/tmp/integ/streams.log`, rc 0) | Additive units/header/identity entries retained |
| feat/omega-codegen-tern (strengthening) | 3277100 | #279 | 2026-11-05T22:29:47-06:00 (`62640e9`) | PASS under bwrap: 213120 programs, 14258 refusals; 68 source-mutant controls; simulator 34 checks (`/tmp/integ/tern-strengthened.log`, rc 0); units/crumb checks PASS | Kept integrated TGEMM and PF registrations |
| feat/omega-codegen-tc | 57b19749567118e0400800ba9c10191ca627a499 | #278 | 2026-11-05T22:42:32-06:00 (`0976ed7`) | PASS under bwrap: 180517 cases, 52380 matching refusals; six source mutants caught; TC/TCGEMM/PFGEMM simulator controls PASS (`/tmp/integ/codegen-tc.log`, rc 0); units/crumb checks PASS | Unioned native header and identity entries; retained all unit registrations |

## Acceptance

QMMA and exhaustive TGEMM namespace runs: `bg_906` and `bg_907`, source `62640e93b39ca6a6a33a1795d5463db1297eb81f`, isolated binaries under `build/integ-long-62640`. Commands: `OUT_DIR=build/integ-long-62640 bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-codegen-qmma-diff` and the same wrapper with `test-codegen-tgemm-exhaustive`. Outputs `/tmp/integ/codegen-a-qmma-bwrap.log` and `/tmp/integ/codegen-a-tgemm-exhaustive-bwrap.log`; `.sha` and `.rc` companions preserve provenance and actual status. Both tests execute compiled binaries and perform no runtime source-file reads, so subsequent integration does not mutate their running images. The interrupted pre-namespace attempt (`bg_401`) remains INCONCLUSIVE and `/tmp/integ/codegen-a-qmma.log` is retained.

Completed host receipts: QMMA rc 0, `CODEGEN QMMA DIFF PASS (714275 cases)`, structure exhaustive NITER bit-basis PASS (94 values, zero refused), and both mutants caught. TGEMM rc 0, `CODEGEN TGEMM EXHAUSTIVE DIFF PASS (17276981 accepted cases)`, mutant caught. MoE #284 at `262275e`: `CODEGEN MOE DIFF PASS (1732053 programs, 10681 refusals)` with record negative control caught (`/tmp/integ/moe-final-diff-262275e-b.log`). The long-run processes are complete; these are observed host results, not hardware qualification.

TC focused command: `bash /tmp/integ/gate.sh codegen-tc test-codegen-tc test-codegen-tc-mutants test-codegen-tc-sim`. That script exports all four pinned Omega environment variables and runs `bash om/units.sh --check && make -j3 ... && crumb compile . && crumb verify .` under the mandatory namespace/taskset/nice wrapper. Completed `bg_915` rc 0, `/tmp/integ/codegen-tc.log`, `/tmp/integ/codegen-tc.rc`; 41 crumbs current. An additional namespace-only crumb pass `bg_954` returned rc 0 with no rewrites.

Full host suite: INCONCLUSIVE, interrupted pre-namespace attempt `bg_601`, source `071ad44`, output `/tmp/integ/full-1.log`. Both jobs were cancelled after Main's device-namespace safety rule; the later instruction allowing an already-running full suite arrived after cancellation.

Every subsequent test, mutant, simulator, differential, and full-suite command runs inside `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 ...`. This hides NVIDIA, CUSE, and VFIO device nodes; a host/simulator device-open path is a defect, not permission to retry outside the namespace.

Replay uses the append-only `BW_IR_LAST` bound, including binary64 opcode 78; values above the bound are refused. The integrated allocator also reserves metadata slots 6 and 7 for the GPR-limit and live-hint protocols. MoE's emitter-local counters are copied to slots 1 and 2, then cleared before replay.

## Hosted native-library boundary

Selected integration boundary (not yet wired into the integrated build): one CUDApp-wide bridged native library per architecture, produced by one bridge invocation over every device-word Omega input. NVRM, Streams submission, PlatformAien, and subsequent Linux-platform and zero-copy inputs use one process-wide native runtime and common context implementation. Separately generated effect libraries define duplicate `omega_native_context_create` / `omega_native_context_destroy` and `ombr_*` symbols and must not be linked together. Generated symbols are not renamed or edited. Per-lane Omega sources and tests remain owned by their lanes. Shared output paths are `src/cudapp_om_hosted.c`, `src/cudapp_hosted_native.h`, and `src/cudapp_om_hosted.ident`; `native/hosted/units.sh` will own generation and `CUDAPP_HOSTED_NATIVE_OBJECT` will name its one link object.

The selected AArch64 device compiler is omega-mv draft #1047, source `dbedb2cac6afe33d4e8958ce6f44292b8d0468c4`, omc SHA-256 `b8f8f1371bc0303f8da02db8b754e085d5dd852620c8d8d676c5769c233d9195`, paired with omega-bridge draft #69, source `248a198480485101cb6c4bb1c2c926ef3fd4ce90`. It preserves the context/reserve/bind/quarantine ABI and adds the exact-five A ioctls. Matching x86 superset qualification is pending. Existing lane pins stay frozen as the provenance of their own tests. Non-device units keep e5b32567.

EXERCISED host-only candidates: one immutable #69@248a198 bridge invocation generated `/tmp/integ/hosted-248-six` from NVRM transport, Streams submit, Aien effects, Linux effects/wait, and ZeroCopy sharing. `/tmp/integ/hosted-six-smoke` passed cross-unit NVRM mapping → Streams/Linux stores and polling → Aien drop, with foreign-context and stale-handle negative controls. The unchanged `tests/share/native-share-test.c`, linked against that same combined library as `/tmp/integ/hosted-six-share-test`, returned rc 0 and `ZERO-COPY SHARE NATIVE PASS` (seccomp fake RM). These memfd/fake-RM candidates opened no NVIDIA device and predate the namespace rule; they are not integrated full-suite or hardware qualification.

Device-word acceptance remains dependent on Drake approving these draft compiler/bridge changes. Hardware: NOT_RUN.

## Staged host integration

Stage 2 was pushed at `249f7b25c1c8aa550ff68fde2ccd1058dd97870c`. Stage 3 starts from `f4a8a6985c65e302c066be7fcc49cf5da5892895`, including the API plans, model graphs, core utilities, C3Gemm, allocator, tools, API shells, GemmEx, engine-submit and freestanding-leftovers lane heads.

Stage-3 link repairs cover allocator leftovers, backend vector/spec/SHA units, DAG cost-model/SHA units, and engine NVRM admission/platform links. Native effect units remain mutually exclusive. The engine differential uses the test transport and the Linux effects/wait pair; it does not combine independent native effect runtimes.

| Gate | Observed status | Evidence |
| --- | --- | --- |
| stage3-c | FAIL: missing chip-admission C wrapper; wrapper logging failed after the make error | `/tmp/integ/stage3-c.log` |
| stage3-c2 | rc 2: backend vector-spec/SHA link dependencies missing | `/tmp/integ/stage3-c2.log`, `.rc` |
| stage3-c3 | rc 2: first supported MoE spec refused due to metadata scratch-slot collision | `/tmp/integ/stage3-c3.log`, `.rc` |
| stage3-c4 | rc 0: repaired replay, build and focused gates; final `crumb verify: OK: 81 crumbs current` | `/tmp/integ/stage3-c4.log`, `.rc`; source `84d2331` |
| stage4-pin | rc 0: archive rebuilt with the single #73 pin; final `crumb verify: OK: 81 crumbs current` | `/tmp/integ/stage4-pin.log`, `.rc`; source `d31dcda9` |

The stage3-c3 backend mutation sweep completed: 109 total, 100 killed, 9 documented redundant, 0 test gaps. The engine mutation sweep completed: `PASS om eng mutants: 16/16 killed`. These successful sweeps are retained rather than rerun solely for their output.

Stage 3 was pushed at `0e09c1996d867d0712b91b5c1b878175c2c9a687`. Stage 4 uses the single portable converter pin omega-bridge #73 at `79e8c3613a6f6f9e8eaaa9c013fc25bcb39e0538`. The ordered groups are converter regeneration, #319/#323, #325, #322 and #324, then finished residual lanes. #333 is now complete at `5dad87a569f4fcca3410c4558691b4fdcb9589ca`; it is eligible after adopting the canonical packers from #338. Main authorized stage 5 to adopt the combined #76 pin and #330/#335 after stage 4 passes; no second concurrent converter pin is introduced.

Stage-4 API views are hosted-only. The preexisting `OMEGA_GPU_NO_POSIX` scalar conversion boundary remains C pending a checked-view freestanding converter path (bridge #72 / combined #76 native work). Attempting to link the hosted runtime-view unit fails the unchanged freestanding undefined-symbol gate (`_exit`, setjmp/longjmp and 128-bit arithmetic helpers); that gate was not weakened. This is an explicit unconverted boundary, not an Omega or hardware completion claim.

Hardware/chip, live-RM and VFIO tiers remain NOT_RUN. A host PASS is not hardware qualification. The full integrated host suite still requires its rc and final `CUDA++ HOST TESTS PASS` line; no target count is inferred.

### Stage-4 residual checkpoint (Integrator5)

The initial detached stage-4 state `0d6302cb` was pushed after checking the remote
stage-3 head `0e09c199`. The following residual lane heads are integrated in order,
under the unchanged single #73 converter pin. These are focused **host** receipts,
not a full stage-4 suite result. Logs and numeric rc files are `/tmp/integ/<gate>.log`
and `/tmp/integ/<gate>.rc`; every command used the mandatory device-hiding bwrap boundary.

| PR | Exact lane head | Green gate | rc | Observed final line |
| --- | --- | --- | --- | --- |
| #326 | `12a0c41f8bf0b6e8bf26baf62e52af50e2841752` | stage4-core | 0 | `crumb verify: OK: 84 crumbs current` |
| #327 | `718e10bac1ade12436641f13852d64d0b439d4a6` | stage4-base4 | 0 | `crumb verify: OK: 86 crumbs current` |
| #328 | `e1a87f0d30efa0292fbc6da6ca07346ae49e79b5` | stage4-cost3 | 0 | `crumb verify: OK: 86 crumbs current` |
| #329 | `8697f02e18ad02f62a056145a226603776b99b9c` | stage4-platform2 | 0 | `crumb verify: OK: 86 crumbs current` |
| #332 | `c1fec738fb3dfe4fd48bf9115a19ca9d4d3d6367` | stage4-attnviews3 | 0 | `crumb verify: OK: 86 crumbs current` |
| #333 | `5dad87a569f4fcca3410c4558691b4fdcb9589ca` | stage4-apirest-shells | 0 | `crumb verify: OK: 88 crumbs current` |
| #336 | `b9ef8f14eaa2e5810e8fc9ed06db540ff92990d2` | stage4-nvrmstruct2 | 0 | `crumb verify: OK: 88 crumbs current` |
| #338 | `cd44785cc000ec2a7337d35252173ea3aa3cba06` | stage4-checkers / stage4-checkers-regalloc3 | 0 / 0 | `crumb verify: OK: 88 crumbs current` |

Additional #332 family gate `stage4-attnviews-api` returned rc 0 and
`PASS attention typed-view host gate`. The #333 differential/family portion passed
before its shell link exposed missing FP8 provider dependencies; the repaired shell
gate reported `PASS api-shells checks=753157`.

The #338 allocator runner initially failed to link #327 providers, then revealed
two stale liveness mutants that no longer reached the active policy. Its dependency
union is repaired and those mutants now target `cg_life.om` and `cg_replay.om`.
The final rerun killed all 20 semantic mutants; the shortened-override and v159
negative controls failed the liveness differential as expected. Historical failed
logs remain alongside the green reruns. The #338 backend JSON receipt is still
pending #339's escaping repair and must not be described as parsed valid JSON.

Stage 5, the NO_POSIX BF16 cutover and `full-2` remain **NOT_RUN** at this checkpoint.
The inherited C BF16 boundary above remains explicit. Chip/live-RM/VFIO/hardware
and merge-to-main are **NOT_RUN**. Integration is only on draft #280.

### Stage-6 Half C focused integration (Integrator6C)

Half C starts at `0d38145a697290f2cd00578fbaa168b824a38015` on
`integ/stage6c`. The single converter remains omega-bridge #73 at
`79e8c3613a6f6f9e8eaaa9c013fc25bcb39e0538`, with seeded compiler
`e5b32567`. All gates run in the device-hiding bwrap namespace, restricted to
cores 10–14 at nice 10, with builds at most `-j3`. Logs and numeric rc files
are `/tmp/integ/6c-<gate>.log` and `.rc`.

| PR | Refreshed lane head | Focused host gate | rc | Observed result |
| --- | --- | --- | --- | --- |
| #353 | `ba465bf3a22aab5700a0eee1a78fbe0b6e6a19ef` | `353-final` | 0 | GEMM HOST PLAN ACCEPTANCE PASS (8 mutants, red comparator, missing-unit link); host 209 and sim 239 checks |
| #354 | `4a17208309100b3774b564c09940f79c9f2d164f` | `354` | 0 | GEMM EX HOST POLICY ACCEPTANCE PASS; host 227 and sim 255 checks |
| #361 | `30a7270502f2803760edfec0a15df9d54b1df2d7` | `361`, then `361-fixtures-final` | engines 0; initial aggregate 1; repaired fixtures 0 | 44584 identical records; 16/16 engine mutants killed; engine 346 + bounds 6 checks; fixtures BUILD_ONLY, 7/7 mutants KILLED |
| #357 | `d7b54d683668d94d2100682b31f847960d69fd21` | `357-final` | 0 | NUMERIC REST OM MUTANTS PASS 6/6; differential 8418438 and scalar 152415164 checks; scan sim 263 checks |
| #363 | `fca2c7a9ee88ca51196e4c16c164adbf83c508a0` | `363` | 0 | MATHLIBS-HOST-OM GATE PASS (differential, red, strong-link negative, gcov, 8 mutants); mathlibs sim 6265 checks |
| #358 | `14914950c616274a8c91366645d10c1c29fb3770` | `358-final` | standalone targets 0; compat 1 (expected under #73) | scalar 1270080; 9/9 compiled mutants killed; eight family comparisons and eight red controls PASS; compat requires final #77 v3 identity |

Every completed merge regenerates the combined declarations/identity and
passes `sh om/units.sh --check` byte for byte; every green commit passes
`crumb compile . && crumb verify .` and is pushed before the next merge.
The engine fixture's BUILD_ONLY link needed the integrated NVRM serializers,
native transport and chip-admission providers. Numeric remainder differential,
mutant and coverage links needed `cudapp_om_bw_qmd.c` after the QMD cutover.
These dependency repairs retain the strong-link and negative controls.
Standalone integration retains the default-only unit registration so explicit
mutant subsets remain selective. Two external transport mutant anchors were
retargeted after the integrated adapter cutovers: wrong K-pointer before argument
packing and wrong tile height in the active planning request; both are caught.
The #73 converter changes the fixture executable hashes relative to frozen v2:
aarch64 `d7649f223e766b9dd007f723206b3c794c61bbda19a0d83b985fa4834c28d8b8`,
x86_64 `0601563603f26bac683f1ecfb5bbd9a9690f0edba581f5e7638075df807f4492`.
`compat/cuda/test.sh` built each identity twice identically, found both CUDA
import diffs empty, and refused all 14 negative controls; the two positive
admissions fail their frozen executable hashes. Main explicitly deferred the
additive v3 profiles to final converter #77 (`d90433a9`), rather than pinning
transient #73 artifacts or changing v1/v2 history. Final C0 acceptance is NOT_RUN
here under #77; `358-final` retains the expected #73 FAIL receipt.
The original failing receipts remain: `353` requested a nonexistent extra
target (the corrected policy gate already runs GEMM host/sim), `361` exposed
the fixture providers, and `357` exposed the QMD provider.

The full repository `make test` is NOT_RUN by Half C and remains Main's
`full-2` gate. Chip/live-RM/VFIO/hardware qualification, merge-to-main and
deployment remain NOT_RUN. Focused host/simulator receipts do not upgrade a
hardware status row.
### Stage-6 Half B converter adoption

Half B starts at `0d38145a697290f2cd00578fbaa168b824a38015` on
`integ/stage6b`. The single converter lock selects omega-bridge draft #76
`741c1ebb2b0fbae311a77ad2118b13d5ec7a17ee`. Its portable and freestanding
converter/frontend source digests match the preceding #73 selection.
Regenerating every portable unit changed no generated C, header or identity
file. The lock text is the only converter-switch change.

The first representative gate exposed an inherited #332 attention-view
freestanding provider defect: `test-gpu-native-runtime` failed with unresolved
`omega_av_*` symbols. Repeating the gate at the starting source revision with
#73 reproduced it (`/tmp/integ/6b-baseline73.log`, rc 2). This is not a
converter-regeneration difference. The unchanged native import allowlist
remains enforced. Full integrated host acceptance and hardware qualification
remain NOT_RUN.

The #335/#348/#352 union supplies nine native NO_POSIX roots. The shared
`CUDAPP_MIGRATION_PENDING_C_NO_POSIX_SESSION` remains default zero, and
`src/omega_gpu_apiviews.h` is the sole definition of `av_bf16_n`. The focused
session and BF16 differentials, production OFF/ON symbol controls and their
compiled mutants pass (`/tmp/integ/6b-pr352-final.log`). The BF16 C helper is
compiled only in the explicit NO_POSIX oracle arm, not merely because other
NO_POSIX apiview helpers still use their C snapshots.

The independent #333 API-rest and #356 attention/artifact-rest differential
harnesses originally shared a filename. Both remain: `tests/api_rest_diff.c`
and `tests/api_attn_rest_diff.c`, respectively. The latter is selected by
`tests/api_rest_check.py`. Its artifact fixture explicitly opens the retained
C-artifact shell while independently selecting new Omega versus C residual
computations; the production seeded artifact snapshot path remains unchanged.
Focused harness links include the prior Omega BW scalar provider. #350's scan
oracle additionally renames the later u32-prefix/radix exports to avoid
duplicate definitions, and its mutant runner uses the single configured
converter for both scalar and runtime-view units.

#346's prefill poison counting now uses its specific Omega root, retaining
earlier Omega copies/fills and #332 rope generation. Its six compiled mutants,
missing-unit/red controls and eight host/simulator prefill gates pass
(`/tmp/integ/6b-pr346.log`). #355's allocator union retains the existing chip
admission and serializer dependencies while adding four strong allocation
providers; its runtime-view selector is merged into the shared generator
convention. Both qualification scripts forward the allocator oracle flag;
their programs are compiled only and must not be executed without fresh
hardware authorization.

The single converter is now omega-bridge #77
(`d90433a90f26159d878a277d232f0269946ca669`, containing #76). Its checked
borrowed-view freestanding profile makes the #332 attention/view compositions,
the #356 matmul-oracle compositions, `cg_fields` and the cubin message/admission
compositions import-free. They are
generated once as `src/cudapp_freest_<root>.c` for both hosted and NO_POSIX
consumers; their obsolete `src/cudapp_om_<root>.c` copies are removed by the
generator and refused by `--check`. The corresponding per-unit identities and
aggregate custody identity change for the new converter/profile. Existing
hosted computations and the native computation compiler are unchanged.

The NO_POSIX partial-link import allowlist is unchanged. A new required
negative control removes each of `omega_av_set`, `omega_av_copy` and
`omega_av_fold`'s providers and requires the strong-provider link to fail.
Direct codegen/regalloc and attention/API mutant harnesses select the same
single generated implementation and emitter used in production. Explicit
codegen links retain `cg_fields`, BW scalar, pathcopy and the emitter/replay/
liveness/invariant providers; both regalloc oracle links retain pathcopy.

The cubin allocating shell calls the checked import-free Omega message and
admission providers even when it selects the caller-stack native packer. The
native call is explicit rather than a global preprocessor alias that changes
later hosted declarations. The broad unrelated core freestanding refusal gate
is unchanged. Its native differential links the same two import-free diagnostic
providers. No C computation fallback or additional import allowance is added.

The conversion replaces exactly 40 hosted generated C files with 40
freestanding generated C files. All 357 unit identities change to record the
converter pin. The fixed-extent repair then updates 317 identities, the profile
comments in the generated header and six freestanding bodies: `attn_args`,
`attn_lay`, `attn_mm_next`, `cg_fields`, `core_cubadmit` and `core_cubinmsg`.
These reject wrong fixed extents rather than treating them as capacities;
OM2 runtime views retain their checked capacity/alias contract. The production archive is recreated
before adding its current objects: incrementally updating it with `ar rcs`
otherwise retains the removed hosted provider members. A membership control
rejects the stale archive and requires all 40 replacement members and no
obsolete member after the rebuild.

The initial #77 checkpoint exposed `omega_av_pack` accepting a 31-element
view where its declared extent is 32 (differential exit5). The pinned repair
passes the full attention typed-view differential, nine compiled mutants and
all seven attention-family API host/sim gates; residual API differentials,
nine mutants and three missing-unit controls also pass. These results are
recorded in `/tmp/integ/6b-pin77-callers-repaired.log`. That compound command
stopped at missing explicit codegen/regalloc link providers, repaired without
changing the acceptance paths. The original failing receipts are retained.

The repaired-pin archive membership, artifact-format positive/red controls,
NO_POSIX native/session/BF16 differential and mutant gates, freestanding
engine/codegen, native runtime and byte-identical regeneration all pass in
`/tmp/integ/6b-pin77-final.log` (rc0; `crumb verify: OK: 88 crumbs current`).
The hosted formatting includes use equivalent nested NO_POSIX/core-oracle
guards so the existing portability scanner recognizes the freestanding
boundary; the implementation-selection predicate and scanner are unchanged.
