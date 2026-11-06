<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# Slice 5 host migration receipts

Source base: CUDApp b774723f537cec16a359fe082b8100317a5b5490.
Compiler: omega-mv e5b32567, SHA-256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f.

New command receipts in this directory are host-only. Every execution uses
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.
No GPU authorization exists. Hardware qualification, merge and deployment: NOT_RUN.

WolframLanguageEvaluator: `Integrate[x^2, {x, 0, 1}]` -> `1/3`.
WolframAlpha: `sqrt(27)/2` -> `(3 sqrt(3))/2`.
WolframLanguageEvaluator: `{2^32-2,2^32-1,2^32,2^63,2^64-1}` ->
`{4294967294,4294967295,4294967296,9223372036854775808,18446744073709551615}`.

See `tests/LEFTOVERS.md` for source classification, branch partitions and chip runbook.

## Observed results

All recorded command groups exited 0:
- `units.log`: `PASS portable Omega units regenerate byte for byte`.
- `focused.log`: required artifact, manifest, portable binding, scalar
  differential/red, zero-CUDA argv and Ampere numeric/f16 host targets.
  Differential: `LEFTOVERS PASS checks=3230`; deliberate mismatch refused.
- `mutants.log`: `LEFTOVERS MUTANTS PASS killed=9`. Individual assertion-failure
  receipts are `mutant-*.log`; `red.log` retains the deliberate checker failure.
- `coverage.log`: C oracle compiled `--coverage -O0`; actual path oracle and
  emitted numeric/ordinal wrappers reach all basic blocks. See source-annotated
  output and `tests/LEFTOVERS.md` for defensive/unemitted branch explanations.
- `freestanding.log`: `PASS: freestanding codegen contract (14 fixtures, 0 failing checks)`.
  GPU engine/platform host-sim tests and the freestanding unresolved-symbol
  allowlist also pass in that command group.
- `native-runtime.log`: `test_omega_gpu_session_platform: 27 passed, 0 failed`.

IMPLEMENTED and EXERCISED on the host; chip QUALIFIED, MERGED and DEPLOYED are
NOT_RUN. The full `make test` is reserved for Integrator3 and was not run here.
The residual NUL-pointer traversal and concrete freestanding byte-view extension
proposal are explicit in `tests/LEFTOVERS.md`; this is not a claim that all C
pointer traversal has disappeared.
