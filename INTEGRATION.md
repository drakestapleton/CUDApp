# Omega CUDA-stack integration

Tracking branch: `integ/omega-cuda-stack`. Base: `b774723f537cec16a359fe082b8100317a5b5490`.

This branch is not permission to merge or deploy. Host acceptance does not qualify GPU behavior. No GPU window is authorized.

| Branch | SHA | PR | Merged-at | Focused verdict | Conflicts resolved |
| --- | --- | --- | --- | --- | --- |
| feat/omega-codegen-a | af70a66cf94e85042e508c08a6e7d6780e6b544f | #275 | b7af547c7be0 | NOT_RUN to completion; tracked acceptance running | None |

## Acceptance

QMMA and exhaustive TGEMM outputs: `/tmp/integ/codegen-a-qmma.log` and `/tmp/integ/codegen-a-tgemm-exhaustive.log`. The corresponding `.rc` files record actual process statuses when complete.

Full host suite: NOT_RUN.

Replay bound repair: pending C1Kernels' `BW_IR_LAST` declaration. Required boundary cases: opcodes 64 and 65 accepted, 66 refused.
