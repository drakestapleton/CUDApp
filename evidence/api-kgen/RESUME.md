# API kernel generator integration receipt

Implementation and assigned host acceptance are complete. This is not a merge,
deployment or chip-qualification receipt.

- Worktree: `lanes/KernGenApiOm`, branch `feat/api-kernel-gen-om`.
- Draft PR: https://github.com/aien-dev/CUDApp/pull/340, base `feat/api-views-attn-om`.
- Exact source/test revision exercised by the final combined gate:
  `e727962885a3f5878065498448ef1bd4402465c9`.
- Verified stacked base: `c1fec738fb3dfe4fd48bf9115a19ca9d4d3d6367`.
- Prerequisite merges: `7113117` (#309 replay), `bac386c` (#294 scan).
- EW registration/cutover: `73b11c57`, `c331928f`.
- Attention registration: `f965e67a`, `a61802d5`, `20b33b52`, `aff2669c`.
- Attention cutover: `913f5bec`; full-cubin acceptance follow-up: `e7279628`.

## Proven acceptance

`integrated-differential.log` records the final serial EW and attention
IR/allocator/text/cubin differential, red controls and mutation gates, exit 0.
EW: 127 accepted / 73 refused; shape-plan partition 4189 admitted / 4001 refused.
Attention: 39423 cases / 39392 complete cubins. Five EW and five attention seeded
semantic mutants were killed. Altered IR records and altered final cubins were
detected as required failures, not accepted as green results.

`default-api-targets.log`: default-Omega elementwise, reconvergence and attention
host/model/simulator targets, exit 0.

`decode-prefill-targets.log`: decode (including maxpos), decode256 and the existing
prefill-host aggregate, exit 0. Seven of seven maxpos controls killed. The prefill
aggregate includes attention host and simulator modes; no separately named
`test-gpu-prefill-attn*` target exists on the assigned base.

`simulator-targets.log`: hello, gemm-ex and engine simulator targets enumerated
from `mk/`, exit 0.

`units-crumb.log`: full pinned `om/units.sh --check`, crumb compile and verification,
exit 0; 46 current crumbs. Compiler, converter, opcode, replay ABI and trust/gate
contracts are unchanged by the final fixture-only follow-up.

Every test uses private `/dev` under the mandatory bwrap wrapper with CPU10–14,
nice10 and build parallelism at most 3. No full `make test` was run. Chip remains
NOT_RUN, requiring a fresh explicitly authorized exclusive window and the README
runbook. No device, service, GPU lock or serving process was changed.

## Integration custody

The attention child has completed its owned source/test/evidence handoff and
reported no remaining jobs. Integrator5 is Main's current integration owner;
Integrator4 has been superseded. This branch retains the single draft PR. Final
commit and claim-close receipts are supplied in the owner's final handoff, not
inferred from this file. No merge or deployment authority exists here.
