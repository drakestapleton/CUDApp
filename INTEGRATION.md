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

### Stage-6 Half A receipts (Integrator6A, 2026-11-06)

Half A uses bridge #73 `79e8c361` and the pinned e5b32567 compiler. Every
execution is isolated by bwrap with an empty `/dev`, cores 10–14 and nice 10.
These are host/simulator results, not chip qualification.

#339 `e2b8e8a523088a0132ae849cf51afa3d150181cb` repairs the JSON receipt.
The regenerated `/tmp/integ/6a-339-backend.json` parses losslessly: 109 rows,
100 KILLED, 9 SURVIVED_REDUNDANT, zero gaps. Main accepted these current counts
instead of the older 120/107/13/0. `/tmp/integ/6a-339-reconcile.tsv` maps every
historical row: 109 retained, 10 relocated to `tests/om_eng_mutants.sh`, one
redundant page-minimum guard removed by #321 `1ecaf8c40802dee63c9534944fae492005076033`.
The relocated rows account for seven historical kills and three historical
redundancies; the removed guard accounts for the fourth redundancy. All ten
relocated rows are now killed in the 16/16 semantic sweep.

Logs: `/tmp/integ/6a-339.log` records byte-identical regeneration, JSON PASS,
engine C49/49 and OM21/21 kills, and backend109/100/9/0. Its aggregate rc2 came
from a second invocation with a missing relative receipt directory. The absolute
rerun `/tmp/integ/6a-339-final.log` completed the valid backend receipt before
its deadline interrupted the semantic sweep. `/tmp/integ/6a-339-semantic.log`
returned rc0: `PASS om eng mutants: 16/16 killed`, with 44584 differential records
identical. The runner now fails immediately if it cannot open its receipt; the
JSON regression includes that negative control. Historical evidence is untouched.

An inherited clean-build repair delays `mk/host-policy.mk` until its cost-model
object and binding variables exist, so target prerequisites include their providers.
`6a-339-close.log` exercises that repair: decision differential21058, predictions8617,
contraction negative control PASS, and crumb verify OK.

#337 `3821a8b3b3858b44af8402e2e708a975fb6c2dc0`: full generation/check remained
byte-identical. `/tmp/integ/6a-337-host.log/.rc` returned0: tern schedule PASS,
simulator34/34,68 source-mutant/config controls, differential213120 programs/
14258 refusals, and crumb88 current. Explicit ternary/codegen links now name the
newly integrated API/IR providers rather than relying on an incomplete old list.
`test-codegen-freestanding` is **DEFERRED_TO_UNION**, not PASS:
`/tmp/integ/6a-337-focused.log` rc2 records inherited missing `omega_av_*`
providers. Main authorized deferral because Half B `638975ca` owns the bridge#77
root repair; final integration must regenerate under bridge `29d9cff7` and pass
this gate. Half A retains its assigned bridge#73 pin.

#331 `79dac21d2fccae9a86239c2ab13cb2bcd8ffffb5`: generation/check byte-identical
(`6a-331.log`); repaired standalone parser/corpus and existing mutant links to
retain integrated cost-model/SHA providers alongside dag_parse/dag_fmt.
`/tmp/integ/6a-331-focused.log/.rc` returned0: parser492/0, formatter3631/0,
CLI25/0, six parser mutants killed, unit203/0, five scheduler mutants killed,
corpus1200/0 with driver/record-flip negatives, and the full focused tools family
including artexp116 cases/266 files/10 mutants. Final: `crumb verify: OK: 89 crumbs current`.

#341 `56baf91039954a14fdf47ce4fe91b5e61cb67e02`: `/tmp/integ/6a-341.log/.rc`
returned0. Full units regeneration byte-identical; resolver4498/0, nine mutants,
red and missing-scheduler controls passed. Parser/scheduler/corpus/tools and
cost-model families passed. Final `crumb verify: OK: 90 crumbs current`.
Merge retained integrated runtime/SHA providers in all standalone oracle links
and combined dag_res/dag_diff roots rather than dropping earlier registrations.

#344 `cfe1f8180c42c04257b99bb40e903659acf8be40`: `/tmp/integ/6a-344.log/.rc`
returned0. JSON byte differential all256, exact stream parity, red/link and five
mutants passed; parser/resolver/scheduler families passed and units regenerate
byte-for-byte. Combined dag_jstr with integrated runtime providers in direct
links. Final `crumb verify: OK: 90 crumbs current`.

#282 `62abe5eadfdc674a1180f8e2b8a542e5b50c902d` merged cleanly on the newer
integrated API, retaining the successful non-NULL session-pointer contract.
`/tmp/integ/6a-282.log/.rc` returned0: host/warp-sim reference and shape sweep,
six generated source mutants plus the session-open abort control, NULL/non-NULL
shell contract, byte-identical units and `crumb verify: OK: 90 crumbs current`.

### Half A: #340 kernel-family API generation

Merged refreshed lane head `0463629a6d1651ed13e928e99b56a7b3f1b1d4e4`.
`/tmp/integ/6a-340.log/.rc`: rc0 under device-isolated bwrap, bridge #73.
Byte-identical unit regeneration; EW differential 127 accepted/73 refused,
shape partition 4189/4001; attention 39423 cases/39392 complete cubins;
five EW and five attention semantic mutants killed, IR/final-cubin red controls.
Elementwise, reconvergence, attention, decode/decode256, eight prefill host/sim,
hello-sim, GEMM-ex-sim and engine-sim gates passed. Final:
`crumb verify: OK: 91 crumbs current`.
Merge retained the existing root union and dated notes; no acceptance weakened.
Hardware/live-RM/VFIO/chip remain NOT_RUN.

#359 `bdb3d5d6f3532b9c0e1c9cf6b6933598ea4c3df7`:
`/tmp/integ/6a-359-focused.log/.rc` rc0, ATTN-HOST-CHECK with nine
compiled mutants and three link omissions; attention/elementwise/scan family
host+sim gates; byte-identical units; `crumb verify: OK: 91 crumbs current`.
Repair: dispatch OM2 `attn_host` and `attn_scanchk` through the existing
runtime-views converter profile. Initial `/tmp/integ/6a-359.log` rc24
exposed the omitted dispatch. Scalar clock retains its ordinary seeded ABI.

#343 `0c12e0151b3d5ee3a5017584a8322d128dfbcb44`:
`/tmp/integ/6a-343.log/.rc` rc0; q38 host9/9, complete rotary-tail
mapping and all five compiled mutants; byte-identical units;
`crumb verify: OK: 91 crumbs current`. Dated roadmap notes retained together.

#345 `c0d5e0316e13d64aacdfe9a28a4d44f7f66c2dd7`:
`/tmp/integ/6a-345.log/.rc` rc0; headscale, q38 host, graph warp/OM/nested
and compiled mutants PASS; byte-identical units;
`crumb verify: OK: 91 crumbs current`. Root union adds graph_headscale
to the existing flags/ABI runtime-views dispatch rather than stale profile syntax.

#342 `1ad73a6b8f05eddc7826c4ed9c7436312ccf2337`:
`/tmp/integ/6a-342.log/.rc` rc0; disasm words differential/red/link/five mutants,
Omega disasm and byte-identical units PASS; final `crumb verify: OK: 91 crumbs current`.
Additive root and dated-roadmap notes retained.

#347 `ee34d8d8b2c0892ac1230147ba6acf0143908913`:
`/tmp/integ/6a-347.log/.rc` rc0; TC host988323 cases, eight compiled mutants,
strong-link red, cubin identity25, TC differential180517/52380 refusals,
TC simulator PASS; byte-identical units; final `crumb verify: OK: 91 crumbs current`.
Root union and generated crumbs refreshed without altering historical evidence.

#349 `b0e5ff6df126c91689fe093a15e6550fee28fbfc`:
`/tmp/integ/6a-349.log/.rc` rc0; admission656275, seven mutants/strong-link,
11 identical cubins/one refusal; pfhost IR19150/384/50 refusals and two mutants;
host and simulator PASS; byte-identical units; final `crumb verify: OK: 91 crumbs current`.

#360 `467e2aca978e3b134f54deef3995a158bb93c740`:
`/tmp/integ/6a-360.log/.rc` rc2 after byte-identical generation exposed stale
standalone link providers. Repaired both QMMA test links and mutant source closure
with current codegen/allocator/format/cubin/SHA providers; prerequisites defined
before expansion. `/tmp/integ/6a-360-focused.log/.rc` rc0:
2949 differential cases, eight compiled mutants/two missing-unit reds,
selftest IR/encoded/cubin identity, QMMA714275/94 bit-basis, host and TGEMM sim PASS.
Final `crumb verify: OK: 91 crumbs current`. Chip/QMMA hardware NOT_RUN.
