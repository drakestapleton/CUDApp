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

Hardware/chip, live-RM and VFIO tiers remain NOT_RUN. A host PASS is not hardware qualification. The full integrated host suite still requires its rc and final `CUDA++ HOST TESTS PASS` line; no target count is inferred.
