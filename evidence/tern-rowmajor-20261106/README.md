# Row-major tern simulator include custody, 2026-11-06

## Outcome and scope

IMPLEMENTED and EXERCISED, host only. No GPU qualification, merge or deployment.
The reported row-major mismatch is pre-existing on CUDApp main, not introduced
by the stacked Omega API or codegen migrations. The faulty component is the
simulator selected beside the external Omega test source, not the generator,
API packer or algebra reference. The simulator already in CUDApp main is correct.

The new regression in existing `tests/gpu_tern_sched_test.c` constructs its own
packed weights and expected rows. `Makefile` now tracks its simulator headers.
`docs/OMEGA_HOST_TESTS.md` records the include requirement. No production C or
Omega implementation changes were needed. This work does not port verification
oracles to Omega and does not change a public API, compiler pin or trust gate.
No omega-harness guide describes a changed interface here.

## Source pins

- CUDApp origin/main: `b774723f537cec16a359fe082b8100317a5b5490`, fresh detached `/tmp/tern-main`.
- IntegStage3 snapshot: `84d23311052559216defa1a456b61efadd52f035`, detached copy `/tmp/tern-integ`. The live integration tree was not edited.
- ApiViewsRest HEAD snapshot: `c1d68bb9854c3bc995a111f2cd7a366eb15da265`, detached copy `/tmp/tern-api`. Uncommitted lane work was not copied or edited.
- Regression source commit: `eeaba533205dbbb5fb696f9758b86e64ac0a0b75`, branch `fix/tern-rowmajor`.
- Omega reference test and compiler source: `e5b32567a7aff3ce594e3254a622a846a6cd190b`.
- `omc` SHA-256: `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Local verification bridge: `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4`.
- CUDApp simulator SHA-256: `160fb034317c73d7861bae21ab23c4a0bdca1928af9cea93cd9d85492189b5a2`.
- Omega reference simulator SHA-256: `e158dd6cd7d0fd5b06eed1f929c9cbf41b1ed9fd7e62f1fd268b3dad111d41e6`.
- Regression source SHA-256: `a69d9df1dcd5c91db697ed3f7eb6fcf72e9d5b1cfe58574603f587df6cbcff5f`.
- Host: Linux `7.0.0-1019-nvidia`, aarch64. GCC `13.3.0` (Ubuntu `13.3.0-6ubuntu2~24.04.1`), bubblewrap `0.9.0`.

Every execution in this receipt used:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 COMMAND
```

No device, live service, GPU lock or driver was touched. Build parallelism was
at most `-j3`. No full `make test` was run. Log copies replace the private home
prefix with `~`; expand it when reproducing. Original temporary logs remain
under `/tmp/tern-repro` on the execution host.

## Reproduction matrix

The external pinned `tests/gpu_tern_test.c` includes `bw_warp_sim_k.h` with
quotes. The preprocessor searches the external source directory before `-Itests`.
The stale simulator drops `in->imm` for scalar `BW_IR_LDG_E`. CUDApp main's
case adds the signed immediate, as do its wide-load cases. Row-major unrolled
weight loads need these offsets; the block-layout loads use the already-correct
wide-load path.

The exact external `sim_battery()` was compiled in each snapshot through a
verification-only entry adapter. It neither changes the test nor its reference.
Unreachable chip functions are discarded with function/data sections and linker
GC. Both the API and codegen from each named snapshot are used. The two variants
differ only in test-translation-unit pre-inclusion of CUDApp's simulator.

| Snapshot | External adjacent simulator | CUDApp pre-included simulator |
| --- | --- | --- |
| main | FAIL, rc 1, 34 checks, 14 failed | PASS, rc 0, 34 checks, 0 failed |
| IntegStage3 | FAIL, rc 1, 34 checks, 14 failed | PASS, rc 0, 34 checks, 0 failed |
| ApiViewsRest HEAD | FAIL, rc 1, 34 checks, 14 failed | PASS, rc 0, 34 checks, 0 failed |

See `{main,integ,api}-{stale,current}.log` and matching `-build.log` files for
exact compile commands and observed output. `battery-entry.c` is the adapter
used for those executions. Failed initial integration linkage, before completing
the migrated API and register-allocation source list, is not a behavior verdict.
The final logs contain the complete working source list.

`/tmp/integ/openpr-files.txt` identifies tern codegen/test ownership in #279 and
integration #280. Simulator changes also occur in #229, #282, #290 and #307.
PR #279's pre-existing adapter `tests/codegen_tern_sim_test.c` already selects
the current simulator. Its evidence directory `evidence/codegen-tern-20261106`
records the same stale-header trap. A stack bisect is therefore unnecessary:
unchanged origin/main reproduces the exact failure and all three snapshots pass
with the current simulator. No historical receipt was edited.

## Independent arithmetic

The smallest reported failing point is k=512, n=13, layout=0, rows=2, warps=2,
depth=2, grid=2, order=0. The pinned battery uses weight seed 101 and activation
seed 108. These are source values, not estimates.

WolframLanguageEvaluator independently generated the unpacked ternary matrix
and signed byte activation vector, without the API packer, codegen or algebra:

```wl
Module[{s=101,t=108,next,w,x},
 next[z_]:=Mod[1664525 z+1013904223,2^32];
 w=Table[s=next[s];If[s<Floor[3(2^32-1)/10],0,
   s=next[s];If[s>=2^31,1,-1]],{13},{512}];
 x=Table[t=next[t];With[{v=Quotient[t,2^24]},
   If[v>=128,v-256,v]],{512}];
 {Floor[3(2^32-1)/10],w.x}]
```

Result (MODELED integer dot products, in output accumulator units):

```text
{1288490188,
 {-3013,2779,-1089,-2155,-1793,-2083,-240,604,-1545,1900,-156,1931,-1037}}
```

The regression uses a second, simpler independent fixture. Row r has r+1 packed
words with every trit +1 for even r and -1 for odd r; remaining words are zero,
and every activation is 1. No packer or word-offset helper is used. Queries:

```wl
{Sum[2^(2i),{i,0,15}],Sum[2*2^(2i),{i,0,15}],
 Table[16(r+1)(-1)^r,{r,0,12}]}
```

WolframLanguageEvaluator result (MODELED packed words and integer dot products):

```text
{1431655765,2863311530,
 {16,-32,48,-64,80,-96,112,-128,144,-160,176,-192,208}}
```

## Regression and negative control

`regression-stale-build.log` compiles the new, unchanged regression against the
original external simulator through `-include`. `regression-stale.log` records
rc 1, 13 differing rows in each warp order, and `gpu_tern_sched_test: FAIL (2)`.
This is the exact unfixed header-selection path, not a relaxed oracle or an
artificial output mutation.

`regression-current.log` records the normal target:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-gpu-tern-sched
```

It exits 0: both orders have zero differing rows, the existing scheduling
checks and refusal controls pass, and the last line is
`gpu_tern_sched_test: PASS`.

`regression-omega.log` also runs this same new test against the default Omega
builder in the integration snapshot. It exits 0, with zero differing rows in
both orders and `gpu_tern_sched_test: PASS`.

The unchanged original test CLI was separately linked with main's library and
the external algebra object. `cli-stale.log` runs `--sim` and exits 1 with
34 checks and 14 failures. `cli-current.log` runs `--sim` and exits 0 with
34 checks and no failures. The algebra object is compiled separately, without
the simulator pre-include. Function/data sections and linker GC omit unused
external algebra backends. See the corresponding `cli-*-build.log` files.

`independent-rows-{stale,current}.log` records the original smallest case row
by row. The Wolfram values, unpacked host dot product and unchanged algebra
oracle agree for every row. The current simulator agrees too; the stale one
gets every row wrong and repeats each adjacent row value. The archived
`independent-rows.c` is that probe, with only archival comments and the stated
HOME-prefix substitution added.

## Focused make targets

The main-based branch's codegen target is `test-codegen-freestanding` in
`mk/omega-gpu-engine.mk`. The stacked tern generator's focused targets in
`mk/codegen-tern.mk` are `test-codegen-tern-diff`,
`test-codegen-tern-sim` and `test-codegen-tern-mutants`. The local scheduler
target is defined in `Makefile`. These are the targets exercised here, not a
claim that unrelated attention, tensor-core or MoE codegen suites were rerun.

| Tree and target | rc | Observed final result |
| --- | --- | --- |
| main-based source, `test-gpu-tern-sched` | 0 | `gpu_tern_sched_test: PASS` |
| main-based source, `test-codegen-freestanding` | 0 | `PASS: freestanding codegen contract (14 fixtures, 0 failing checks)` |
| integration copy, `test-codegen-tern-sim` | 0 | `CODEGEN TERN SIM: 34 checks, 0 failed: PASS` |
| integration copy, `test-codegen-tern-mutants` | 0 | `CODEGEN TERN OMEGA MUTANTS PASS (68 source-mutant/config controls)` |
| integration copy, `test-codegen-tern-diff` | 0 | `CODEGEN TERN DIFF PASS (213120 programs, 14258 refusals)` |

The freestanding gate also reported byte-identical dumps for 14 fixtures,
24448 code bytes, all 2^32 bf16 inputs equal to the hosted cast, and its
malloc allowlist canary refused. The tern differential caught its record
negative control before printing PASS. These are MEASURED values copied
from the filed logs, not extrapolations.

The integration snapshot's standalone tern target source list was not yet
wired to newer API-shell and register-allocation bindings. The first direct
link attempt failed with missing `omega_apishp` and `omega_ra_{ir,run,live}`.
The final focused target runs used the existing `BW_CODEGEN_BINDING` plus
the API-shell generated bindings through a command-line source-list override.
No source in the integration tree was edited. Integrator4 was given the exact
missing interface and owns the canonical make dependency repair. The successful
commands below therefore prove the target behaviors with completed linkage,
not that the unmodified snapshot's build wiring is repaired.

Run these from the integration copy, with the pinned OMEGA_OMC,
OMEGA_BRIDGE and OMEGA_MV set as above:

```sh
tern_sources='src/omega_blackwell_codegen_tern.c src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c $(CG_TERN_SRC) $(CG_TERN_SCHED_SRC) src/cudapp_om_cg_tern_policy.c src/cudapp_om_bw_enc_sm121.c src/omega_gpu_fmt.c src/sha256.c $(BW_CODEGEN_BINDING) src/cudapp_om_apish_blk.c src/cudapp_om_apish_shp.c src/cudapp_om_apish_trit.c src/cudapp_om_apish_slot.c src/cudapp_om_apish_pack.c'
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 "CG_TERN_HOST_SRCS=$tern_sources" "OMEGA_MV=$OMEGA_MV" test-codegen-tern-sim
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 "CG_TERN_HOST_SRCS=$tern_sources" "OMEGA_MV=$OMEGA_MV" test-codegen-tern-mutants
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 "CG_TERN_HOST_SRCS=$tern_sources" "OMEGA_MV=$OMEGA_MV" test-codegen-tern-diff
```

Each `test-codegen-*.log` contains the command arguments and complete output.
The command header is a plain argument rendering; the shell snippet above
preserves the source-list argument's quoting. An earlier differential execution
hit an Eval deadline before completing and was stopped by exact owned PID;
`interrupted-diff.log` labels it INCONCLUSIVE. The final file-backed execution
had no deadline and completed with the PASS shown above.

The API-rest consumer correction is in that lane's
`tests/api_rest_families.sh`: compile the external oracle separately and
pre-include the local simulator only for the external test translation unit.
It was reported uncommitted by ApiViewsRest2 while this receipt was collected;
this branch does not claim to contain that separate lane's runner repair.


## Hardware tier

NOT_RUN. No GPU window is authorized. This simulator regression does not claim
chip qualification. A future operator-authorized chip receipt should use the
pinned original `gpu_tern_test` chip battery on the selected integrated source,
retain its existing admission/refusal checks and record that exact revision.
