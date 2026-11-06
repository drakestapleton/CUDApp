# Omega CUDA-stack integration

Tracking branch: `integ/omega-cuda-stack`. Base: `b774723f537cec16a359fe082b8100317a5b5490`.

This branch is not permission to merge or deploy. Host acceptance does not qualify GPU behavior. No GPU window is authorized.

| Branch | SHA | PR | Merged-at | Focused verdict | Conflicts resolved |
| --- | --- | --- | --- | --- | --- |
| feat/omega-codegen-a | af70a66cf94e85042e508c08a6e7d6780e6b544f | #275 | 2026-11-05T20:41:13-06:00 (`b7af547`) | Namespace long runs RUNNING; prior unnamespaced attempt INCONCLUSIVE/interrupted | None |
| feat/omega-codegen-b | f219a04eff8ff28e2d8d0b8cd65f210e552bbd54 | #274 | 2026-11-05T21:10:38-06:00 (`52d071e`) | PASS: pf copy 18 instructions, mutant caught (`/tmp/integ/codegen-b-tern.log`) | Additive units/header/identity entries retained; codegen crumb regenerated |
| feat/omega-codegen-tern | bc22ce1e75883eeea7bdafd11f37909a5bbf0247 | #279 | 2026-11-05T21:12:39-06:00 (`9dcefcc`) | PASS: 213120 programs, 14258 refusals; 24 source-mutant controls; simulator 34 checks (`/tmp/integ/codegen-b-tern.log`, rc 0) | Additive registrations retained; missing common-source composition repaired |
| feat/c4-streams-om | daee8b6fc054df8fe40830a04127f10e21ba9184 | #272 | 2026-11-05T21:47:59-06:00 (`7eac35d`) | PASS: host/simulator/differential and four source mutants (`/tmp/integ/streams.log`, rc 0) | Additive units/header/identity entries retained |
| feat/omega-codegen-tern (strengthening) | 3277100 | #279 | 2026-11-05T22:29:47-06:00 (`62640e9`) | PASS under bwrap: 213120 programs, 14258 refusals; 68 source-mutant controls; simulator 34 checks (`/tmp/integ/tern-strengthened.log`, rc 0); units/crumb checks PASS | Kept integrated TGEMM and PF registrations |
| feat/omega-codegen-tc | 57b19749567118e0400800ba9c10191ca627a499 | #278 | 2026-11-05T22:42:32-06:00 (`0976ed7`) | PASS under bwrap: 180517 cases, 52380 matching refusals; six source mutants caught; TC/TCGEMM/PFGEMM simulator controls PASS (`/tmp/integ/codegen-tc.log`, rc 0); units/crumb checks PASS | Unioned native header and identity entries; retained all unit registrations |

## Acceptance

QMMA and exhaustive TGEMM namespace runs: `bg_906` and `bg_907`, source `62640e93b39ca6a6a33a1795d5463db1297eb81f`, isolated binaries under `build/integ-long-62640`. Commands: `OUT_DIR=build/integ-long-62640 bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-codegen-qmma-diff` and the same wrapper with `test-codegen-tgemm-exhaustive`. Outputs `/tmp/integ/codegen-a-qmma-bwrap.log` and `/tmp/integ/codegen-a-tgemm-exhaustive-bwrap.log`; `.sha` and `.rc` companions preserve provenance and actual status. Both tests execute compiled binaries and perform no runtime source-file reads, so subsequent integration does not mutate their running images. The interrupted pre-namespace attempt (`bg_401`) remains INCONCLUSIVE and `/tmp/integ/codegen-a-qmma.log` is retained.

Running executable PIDs recorded for handoff: QMMA `3766518` (`bg_906`), TGEMM `3766524` (`bg_907`). Leave both running. QMMA requires `CODEGEN QMMA STRUCTURE EXHAUSTIVE NITER BIT-BASIS PASS (94 values, zero refused)`, `CODEGEN QMMA NITER DEPENDENCE MUTANT CAUGHT`, `CODEGEN QMMA MUTANT CAUGHT`, and `CODEGEN QMMA DIFF PASS (<measured> cases)`. TGEMM requires `CODEGEN TGEMM MUTANT CAUGHT` and `CODEGEN TGEMM EXHAUSTIVE DIFF PASS (<measured> accepted cases)`. These are expected markers, not observed final verdicts.

TC focused command: `bash /tmp/integ/gate.sh codegen-tc test-codegen-tc test-codegen-tc-mutants test-codegen-tc-sim`. That script exports all four pinned Omega environment variables and runs `bash om/units.sh --check && make -j3 ... && crumb compile . && crumb verify .` under the mandatory namespace/taskset/nice wrapper. Completed `bg_915` rc 0, `/tmp/integ/codegen-tc.log`, `/tmp/integ/codegen-tc.rc`; 41 crumbs current. An additional namespace-only crumb pass `bg_954` returned rc 0 with no rewrites.

Full host suite: INCONCLUSIVE, interrupted pre-namespace attempt `bg_601`, source `071ad44`, output `/tmp/integ/full-1.log`. Both jobs were cancelled after Main's device-namespace safety rule; the later instruction allowing an already-running full suite arrived after cancellation.

Every subsequent test, mutant, simulator, differential, and full-suite command runs inside `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 ...`. This hides NVIDIA, CUSE, and VFIO device nodes; a host/simulator device-open path is a defect, not permission to retry outside the namespace.

Replay bound repair: pending integration of C1Kernels' `BW_IR_LAST` declaration and AttentionC's Main-approved `a8c6a22` bound repair. Required boundary cases: opcodes 64 and 65 accepted, `BW_IR_LAST + 1` refused. Literal 66 is refused only before the subsequent DNN 66/67 extension lands.

## Hosted native-library boundary

Selected integration boundary (not yet wired into the integrated build): one CUDApp-wide bridged native library per architecture, produced by one bridge invocation over every device-word Omega input. NVRM, Streams submission, PlatformAien, and subsequent Linux-platform and zero-copy inputs use one process-wide native runtime and common context implementation. Separately generated effect libraries define duplicate `omega_native_context_create` / `omega_native_context_destroy` and `ombr_*` symbols and must not be linked together. Generated symbols are not renamed or edited. Per-lane Omega sources and tests remain owned by their lanes. Shared output paths are `src/cudapp_om_hosted.c`, `src/cudapp_hosted_native.h`, and `src/cudapp_om_hosted.ident`; `native/hosted/units.sh` will own generation and `CUDAPP_HOSTED_NATIVE_OBJECT` will name its one link object.

The selected AArch64 device compiler is omega-mv draft #1047, source `dbedb2cac6afe33d4e8958ce6f44292b8d0468c4`, omc SHA-256 `b8f8f1371bc0303f8da02db8b754e085d5dd852620c8d8d676c5769c233d9195`, paired with omega-bridge draft #69, source `248a198480485101cb6c4bb1c2c926ef3fd4ce90`. It preserves the context/reserve/bind/quarantine ABI and adds the exact-five A ioctls. Matching x86 superset qualification is pending. Existing lane pins stay frozen as the provenance of their own tests. Non-device units keep e5b32567.

EXERCISED host-only candidates: one immutable #69@248a198 bridge invocation generated `/tmp/integ/hosted-248-six` from NVRM transport, Streams submit, Aien effects, Linux effects/wait, and ZeroCopy sharing. `/tmp/integ/hosted-six-smoke` passed cross-unit NVRM mapping → Streams/Linux stores and polling → Aien drop, with foreign-context and stale-handle negative controls. The unchanged `tests/share/native-share-test.c`, linked against that same combined library as `/tmp/integ/hosted-six-share-test`, returned rc 0 and `ZERO-COPY SHARE NATIVE PASS` (seccomp fake RM). These memfd/fake-RM candidates opened no NVIDIA device and predate the namespace rule; they are not integrated full-suite or hardware qualification.

Device-word acceptance remains dependent on Drake approving these draft compiler/bridge changes. Hardware: NOT_RUN.

## Resume handoff

Main will resume Integrator to continue the same goal. The full integrated host gate is still pending; no successful final line or target count is claimed. Do not rerun completed focused gates merely to recover their output.

Immediate pending queue, preserving dependencies:

| Input | Revision / required action |
| --- | --- |
| Numeric #288 | `1b78bfe99aba346e68274e4922f60791456295ab` |
| CodegenTern #279 | Isolated latency repair `1aa98b14e4e86d276082dbb9ccc3ce251fed099d` requires TC `57b1974` and Numeric `1b78bfe`; final lane head `1b7c4526def6c800e350b7f80ccec7536605a33b`. Preserve existing evidence; the repair's source paths are `tools/latmeas/catalog.json`, `diff_nak.py`, and `REPRODUCE.md`. |
| RuntimeHost #291 | `40ee08e`; union registrations with ApiPlanOm and ArtifactOm, not competing replacements |
| CmdList #304 | `581c048799f13b5b29a5172031c15889d6067672` |
| C1 #301 | `7de6da63c83dabec2f36569b75e67f60e9092390`; preserve predicate/SHFL semantics; `test-c1` stays outside carried host tests until its language draft merges |
| AttentionC | Early simulator `732e10a`, replay bound `a8c6a22`; native-half `9be13fb` then `2224f60`, safe simulator/session guard `b4c616c`, and C1-compatible scheduler guard `a3a9e98`. Do not pick the safety fix without its DNN prefix. |
| RegAllocOm | Signed GE simulator repair `6a855fc` before allocator cutover; meta[6] `5604269` before isolated meta[7] `af8358a`; allocator branch still evolving |
| Live-marker repair | APPROVED but NOT IMPLEMENTED; no commit exists. After NVRM + Streams submission + Linux devwords, define `omega_gpu_live_marker(const Nvrm *, cpu, mapping, offset, ctx)` in `nvrm/nvrm.c` through `nvrm_live_token(..., sizeof(uint32_t), ...)`, returning that same `rm->omega_context`. Update `m16/m16_native.h`, the sole callsite and #297 fixture in one commit. Fake-RM right-context, foreign-context, NULL, and wrong-context mutant checks are required. |
| NVML #305 | `d31e68e7`; depends on Smi, NVRM and shared hosted aggregate |
| Runtime API versions | Main superseded the single-0.14 proposal: HelloC2 #296, MathLibs #303, CmdList #304 receive sequential minors 0.14 / 0.15 / 0.16 in merge order. Change header, manifest and `API.md` together. |

Other ready lane inputs awaiting the same units/focused/crumb sequence:

| Lane / PR | Revision |
| --- | --- |
| CodegenAttn #277 | `463b49547bab01a6621160e8b213da488465daa6` |
| CodegenPfhost #276 | `1a56f317002e174c481641cc2cd082c6950f87a3` |
| C3Gemm #283 | `04f16fe8b553148ef6af3b15e484152b1414f57b` |
| ScanSort #294 | `220fe47dfdecb715e0d49be223808c211b889169` |
| GemmEx #295 | `891d6d42116ac61970990ef6e1678467b16f50bd` |
| Profiler #289 | `da228b26beee4bdb0ef01bef7837900aeaec266e` |
| Resident #290 | `7e47b45` |
| Smi #286 | `85f48ad056ba4f6ee7d9ad310d005c69dde1624f` |
| ZeroCopy #285 | `7217f5b7` |
| Sharing #300 | `e9bd0da228f420d523703744e2fc2c5657b52a6f` |
| HelloC2 #296 | `00f88e178f9bd31ee07e4cf6f7fc0df85ab10446` |
| MathLibs #303 | `10df33bbbf7c21b19a414ac7ac5e92f82d0c84cd` |
| ArtifactOm #299 | `88556d67337ac64c6ead1b819dcadb733d83a5a9`; inspect RuntimeHost ancestry before redundant integration |
| Nvrm #281 | `e2086965ba8ec60d7e385b98abd7565caa13293d` |
| Streams freest #287 / submit #293 | `dc8b3e9` / `16f11c47`; submit follows NVRM |
| PlatformLinux #273 / devwords #297 | `e4153cfd3a00a770d07049ea4fea56ecad250aeb` / `313b1249`; devwords follows NVRM + submit and requires live-marker repair |
| PlatformAien #292 / effects #302 | `36b2e237c51d731d12879919bde42199720b2aec` / `5f3b17d823aee2bc400960a0ed4dc8c3b5ac1a41` |

Shared hosted wiring is not implemented. The immutable six-unit candidate remains `/tmp/integ/hosted-248-six`, with detached bridge oracle `/tmp/integ/bridge-69-248a198` owned by Integrator. Matching x86 draft pair was subsequently published as omega-mv #1050 `9101aee8f4f47476a8dd056d132007221d691946` / compiler SHA `c5ac9979232afd0d750e1d5088af69692032997136f761e585a25aa9a1cfc87e`, omega-bridge #70 `2156ca177c954130fa2fdedf86da0f836e8c8a95`. These are not selected integration pins; BridgeHost reports x86 EMULATED gates passing but the existing A64 `NATIVE-V2 FAIL1` hoist failure remains. Pure OM2-array freestanding qualification is separate and still pending; do not silently use the old bridge profile.
