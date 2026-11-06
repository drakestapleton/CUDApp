# Codegen base/common replay — host evidence, 2026-11-06

**IMPLEMENTED and EXERCISED on host. Not chip-qualified, merged or deployed.**
Baseline CUDApp #309: `63d18a2cc92cb05f800d17e46b35f0cd6237b3b0`.
Registration commit: `79de6f9` (before caller cutover).
Compiler omega-mv `e5b32567`, omc SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
No compiler or bridge profile was repinned.

## Exact exercised commands

Environment for every command:

```sh
export OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc
export OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge
export OMEGA_BRIDGE_DIR=$OMEGA_BRIDGE
```

| Command | rc | Observed final line / evidence |
|---|---|---|
| `sh om/units.sh` | 0 | `wrote checked portable Omega units to .../src` (registration and signed-sentinel regeneration) |
| `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-codegen-base-om-mutants` | 0 | `CODEGEN BASE HOST ACCEPTANCE PASS`; final run is `coverage-host.log` |
| `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j1 test-regalloc-om test-om-encoder test-codegen-freestanding` | 0 | `PASS: freestanding codegen contract (14 fixtures, 0 failing checks)`; `existing-targets.log` |

`sh om/units.sh --check` runs inside both host targets and reports
`PASS portable Omega units regenerate byte for byte`. All simulator and
mutant subprocesses inherit the same device-hidden namespace. No full
`make test` was run; that belongs to the integration owner.

Final dedicated differential (`coverage-host.log`):

- `CODEGEN BASE PASS: 413 replay partitions; 206 byte-identical SASS/cubin images; wire bits and budget boundaries`.
- Three paths agree: new Omega base + Omega encoder; original C base wire
  computation + Omega encoder; original C encoder. The whole padded SASS
  images and cubin containers compare byte for byte. Public single-insn
  word decoding also agrees for every identity-map vreg entry.
- `PASS: one flipped SASS bit rejected by byte comparison`.
- Five independently generated seeded-source mutants are killed:
  `cg_fields`, `cg_replay`, `cg_life`, `cg_invar`, `cg_budget` (each process rc -6).
- `CODEGEN BASE MUTANTS PASS: 5 killed`.
- Real mathlibs common-build/data entry points also compare against the
  retained C path over emitter kinds, shape admission, empty/full capacities
  and thread/budget boundaries. Diagnostic strings agree.

Existing target results (`existing-targets.log`, one successful make invocation):

- `test-regalloc-om`: register/matmul policy cases=10000; allocator differential
  cases=181614, refused=179089, encoded=2525, record-flip caught; signed-GE
  host-reference cases=98; matmul warp-sim cases=15 CPU oracle bit-identical;
  hazard decode cases=431078; hazard check cases=40495; bind cases=50686.
  Target's last verdict: `PASS bind differential cases=50686 refused=38401`.
- `test-om-encoder`: hosted golden fixtures pass; `.om and C byte-identical on
  145 golden instructions and 14 fixture programs`; last line:
  `red check: one flipped bit aborts the encoder diff, as it must`.
- `test-codegen-freestanding`: 14 fixtures, 24448 code bytes agree; malloc
  allowlist control is refused; the existing exhaustive bf16 sweep agrees on
  all 2^32 input patterns. Final line is in the command table above. The
  freestanding side is the inherited C migration oracle, not an Omega
  freestanding cutover.

Earlier incremental snapshots (`host.log`, `host-expanded.log`,
`final-host.log`) are retained as such, not substituted for final evidence.
`host-expanded.log` includes a stale gcov checksum warning from adding
partitions. The runner now clears only its own generated build profiles
before compilation; the final run has no such warning. During implementation,
the undefined-live sentinel case first exposed a checked unsigned-to-signed
cast trap. Explicit signed reinterpretation in `cg_life.om` and `cg_invar.om`
resolved it; the negative sentinel and signed closure cases are in the final
partition. No refusal was converted to a C fallback.

## Source coverage and gap disposition

`oracle-ir.gcov` and `oracle-base.gcov` are final gcov annotated C-oracle
sources. Their home paths are normalized to `~`; command logs are unmodified
runner output. `gcov-ir.log` reports lines executed 98.48% of 132, branches
executed 100.00% of 172, taken at least once 90.12% of 172, calls executed
100.00% of 24. These are gcov measurements, not chip results or claims of
full branch-direction coverage.

All uncovered migrated decision directions have one of these dispositions:

| Original C location | Disposition |
|---|---|
| `field`, `loop_invariant_ok`, `extend_loop` | Every branch direction exercised in final replay partitions: all register fields, each post-loop source position, definitions outside/inside/after closure, uniform/nonuniform redefinition, extension vs pre-loop/at-closure/after-closure last use. |
| replay emitter status/result | All status/result directions exercised by the supplied emitter. Optional null `emit_err` and null unit/program guards are ABI shell paths, not moved computation; dedicated partition uses nonnull diagnostic storage. |
| replay `omega_ra_live` bridge-error branches | The unchanged pinned checked binding has fixed valid view/scalar arguments; malformed hints produce the separately covered semantic refusal, not a bridge trap. Artificially corrupting its runtime is not the migration oracle. |
| replay setter rejection after metadata admission | Unreachable without breaking the already-ported setter: metadata has already admitted the same GPR-limit interval and program is nonnull. Both metadata bound directions are covered. |
| replay append refusal after admission | Unreachable without breaking the already-ported append: nonnull instruction/program and admitted count fit its fixed capacity. Count/operand failures and allocation refusals are separately covered. |
| build declaration floor's non-floor direction; peak-overlimit; invalid declaration | Real emitter builds do not produce these states: allocator enforces the peak bound and `omega_bw_gpr_declared` produces an aligned sufficient declaration. `budgets()` separately supplies the source-derived peak/thread cross product, including those floor/peak/refusal boundaries, 0 and UINT32_MAX, directly to the new unit and original source formula. Both CTA-budget directions are covered through the actual common build. |
| `call_unit` bridge/other-emitter-error branches | Unchanged status-dispatch shell; the mathlibs data corpus yields success/refused, while the injected common replay emitter exercises bridge/error dispatch. No argument validation was moved out of this helper. |
| build null-pointer guards | Unchanged ABI safety shell; zero-thread and code-capacity refusals are exercised. |
| base `bw_om_encode` oversize/checked-binding error | Count rejection is reached before encoding through public admission/capacity policy; checked-view arguments in this shell have fixed correct extents. Old/new field packing, word decoding and all successful loop directions are exercised independently. |
| base pointer, optional file-dump and allocation error branches | ARCH-0042 shell, not migrated computation. File capture is intentionally off: no cubin/SASS dump is committed. |
| base untouched fixture verifiers and prior #309 matmul/allocator functions | Not newly migrated. Their existing dedicated tests pass in `existing-targets.log`; this lane does not relabel uninstrumented runs as source coverage. |

The whole-file base gcov percentage includes untouched fixture/oracle and
allocation/file branches; it is not used as a migration pass percentage.
The exact function classification and source-derived partition are in
`tests/codegen_base/README.md` and the PR body.

## Tier verdicts

| Tier | Verdict |
|---|---|
| Seeded unit regeneration/check | PASS |
| Common replay / fields / budget differential | PASS |
| Whole SASS + cubin byte equality, one-bit red | PASS |
| Five seeded-source mutants | PASS |
| Existing hosted codegen/allocator targets | PASS |
| Existing matmul CPU warp simulator | PASS |
| Existing freestanding compatibility/C oracle | PASS |
| Full integration suite | NOT_RUN by this lane; integration owner runs it |
| GB10 chip / live RM / VFIO | NOT_RUN; no current authorization |
| Merge / deployment | NOT_RUN; draft PR only |

Hardware procedure: `tests/codegen_base/CHIP.md`.
The retained C call sites and baseline freestanding limitation are explicitly
listed in `tests/codegen_base/README.md`; no claim that every CUDApp C residue
has disappeared is made by this lane.
