# Omega CUDA-stack integration

Tracking branch: `integ/omega-cuda-stack`. Base: `b774723f537cec16a359fe082b8100317a5b5490`.

This branch is not permission to merge or deploy. Host acceptance does not qualify GPU behavior. No GPU window is authorized.

| Branch | SHA | PR | Merged-at | Focused verdict | Conflicts resolved |
| --- | --- | --- | --- | --- | --- |
| feat/omega-codegen-a | af70a66cf94e85042e508c08a6e7d6780e6b544f | #275 | b7af547c7be0 | NOT_RUN to completion; tracked acceptance running | None |
| feat/omega-codegen-b | f219a04eff8ff28e2d8d0b8cd65f210e552bbd54 | #274 | 52d071e | PASS: pf copy 18 instructions, mutant caught (`/tmp/integ/codegen-b-tern.log`) | Additive units/header/identity entries retained; codegen crumb regenerated |
| feat/omega-codegen-tern | bc22ce1e75883eeea7bdafd11f37909a5bbf0247 | #279 | 9dcefcc | PASS: 213120 programs, 14258 refusals; 24 source-mutant controls; simulator 34 checks (`/tmp/integ/codegen-b-tern.log`, rc 0) | Additive registrations retained; missing common-source composition repaired |
| feat/c4-streams-om | daee8b6fc054df8fe40830a04127f10e21ba9184 | #272 | 7eac35d | PASS: host/simulator/differential and four source mutants (`/tmp/integ/streams.log`, rc 0) | Additive units/header/identity entries retained |

## Acceptance

QMMA and exhaustive TGEMM outputs: `/tmp/integ/codegen-a-qmma.log` and `/tmp/integ/codegen-a-tgemm-exhaustive.log`. The corresponding `.rc` files record actual process statuses when complete.

Full host suite: NOT_RUN.

Replay bound repair: pending C1Kernels' `BW_IR_LAST` declaration. Required boundary cases: opcodes 64 and 65 accepted, 66 refused.

## Hosted native-library boundary

The integration uses one CUDApp-wide bridged native library per architecture, produced by one bridge invocation over every device-word Omega input. NVRM, Streams submission, PlatformAien, and subsequent Linux-platform and zero-copy inputs share a single process-wide native context. Separately generated effect libraries define duplicate `omega_native_context_create` / `omega_native_context_destroy` and `ombr_*` symbols and must not be linked together. Generated symbols are not renamed or edited. Per-lane Omega sources and tests remain owned by their lanes.

The integrated AArch64 device compiler is omega-mv draft #1047, source `dbedb2cac6afe33d4e8958ce6f44292b8d0468c4`, omc SHA-256 `b8f8f1371bc0303f8da02db8b754e085d5dd852620c8d8d676c5769c233d9195`, paired with BridgeHost's companion strict bridge profile when published. It preserves the context/reserve/bind/quarantine ABI and adds the exact-five A ioctls. A matching x86 superset and strict bridge profile are pending. Existing lane pins stay frozen as the provenance of their own tests. Non-device units keep e5b32567.

Device-word acceptance remains dependent on Drake approving these draft compiler/bridge changes. Hardware: NOT_RUN.
