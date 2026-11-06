# Dagsim resolution and diff: seeded Omega host evidence

## Revision and scope

Branch: `feat/dagsim-resolve-om`. Draft PR: [CUDApp #341](https://github.com/aien-dev/CUDApp/pull/341), based on `feat/dagsim-parse-om` / #331 at `79dac21d2fccae9a86239c2ab13cb2bcd8ffffb5`.

Registration was committed and pushed first as `58e7b49cbe150a08a716c3a65464204a388a98d8`: `om/units.sh`, the native declarations, generated bindings and unit identities. `pins.txt` records the compiler, bridge, source and dependency pins. The seeded compiler is omega-mv `e5b32567a7aff3ce594e3254a622a846a6cd190b`, executable SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

`dag_res.om` owns provenance refusals; mode-dependent capacity, launch and duration selection; cost-model request features and prediction acceptance; confidence interval decisions; calibration presence and expiry; resolved header/resource/op packing decisions; resource counterfactual changes; lookup scans; and streaming changed-list membership. `dag_diff.om`, composed with the resolver helpers, owns added/kept/removed counts, removal evidence, shared-reference double counting, fabricated savings and removed-time accounting. The original implementations are available only with `CUDAPP_MIGRATION_PENDING_C_DAGSIM_RESOLVE=1`, which defaults to zero.

This is a host cutover receipt, not a model-performance or hardware claim.

## Host boundary and dependency inventory

There was no Omega cost-model unit at the pinned base: the `rx_cm_predict` references and `om/` units were inspected before implementation. As expressly allowed by the assignment, `src/runtime/rx_costmodel.c` remains the existing first-party C predictor/Student-t quantile implementation. Its exact C/header/lock digests are in `pins.txt`; it is not represented as seeded Omega. Replacing this exception requires a seeded Omega cost-model unit. `rx_cm_predict`, `rx_cm_t_quantile` and libm `pow` are recorded calls: the host copies requests/results while Omega decides the requests, validity, mode and intervals. No new foreign runtime or library was introduced.

The existing host runtime is glibc/libm `2.39-0ubuntu8.9`; `pow` and the clock query remain host calls. C returns raw `time(NULL)` seconds; Omega performs the day conversion and staleness comparison. C retains formatted refusal strings selected by Omega status/index, allocation/free, and ABI field copies. GCC `13.3.0-6ubuntu2~24.04.1`, the pinned bridge/compiler, gcov and bubblewrap `0.9.0` are verification/build tools, not first-party computation replacements.

`ds_resolved_pack` delegates header/resource/op packing decisions to Omega. Its dependency and lifetime-buffer loops are retained as a pure layout shell: fixed-offset copies into the existing scheduler ABI, no arithmetic model or selection rules. C similarly marshals typed fields to the fixed-view resolver ABI. Numeric storage is two model views in 16,384 words; scheduler output is 8,192 words; text storage is 131,072 bytes. The streaming changed-list API keeps arbitrary list length without introducing a C tokenizer. The six native parameter slots are respected by four fixed views plus the mode argument; bridge result/trap handling is checked. Generated C is bridge output, not handwritten replacement computation.

No language, compiler pin, host word, bridge API, repository ownership or gate policy changes were introduced. Existing tools documentation was updated in `tests/tools-om/README.md`; no omega-harness skill change is needed for this consumer-only use of the existing language and ABI.

## Source-derived differential partition

`tests/dagsim/resolve_diff.c` links the default implementation and a separately compiled, renamed flag-ON C oracle. It compares complete `ds_result` refusal state, status, successful 8,192-word resolved packs, cost-model request fields, lookup results, counterfactual return bits and complete `ds_diff` results. The predictor is fault-injected at its permitted host-call boundary; the real predictor is independently exercised by the cost-model and corpus gates.

Final differential result: **4,498 checks, zero failures** (`resolve.log`). The partition includes:

- Each resource/op provenance value and missing source; all resolve refusal returns; nominal, pessimistic and optimistic modes; all op kinds and present/absent launcher/engine paths.
- Calibration present/absent and both calibrated resource and op paths; `asof` immediately before, at and after `taken + valid`, and nonpositive `asof` through the raw-clock path.
- Predictor unsupported/refused/unknown/zero-sample/other-status outcomes, absent cost model, quantile boundaries and NaN, and feature integer bounds.
- Zero, one, maximum-minus-one and `DS_MAX_OPS` op counts; full resource/dependency/buffer packing capacities; successful and missing lookup names and empty/long queries.
- Each resource doubling rule, unmatched launchers, unsupported indices, default resource kinds, refused counterfactual evaluation and injected allocation failure.
- Every `DS_RM_*` reason, empty/missing/present/self references, proof absent/present, retained/removed/unrelated consumers, NULL options and changed lists, missing/duplicate/fabricated records, shared double counting, changed kept demand, added ops and time-vs-bandwidth removed accounting.
- Changed-list tokens crossing every tested stream boundary through 1,024-byte prefixes; exact token matching rather than substring matching; zero and maximum op-count diffs.

`oracle.gcov` and `resolve.gcov` are captured immediately after the unmutated differential, before negative controls/mutants. They are branch receipts, not a claim of complete coverage of unrelated parser/formatter/scheduler code.

The runtime negative control deliberately corrupts the candidate resolved pack: **caught, rc 1** (`red.log`). The pre-cutover test source also failed compilation because the migration guard did not exist (rc 1); this prerequisite failure is not counted as a behavioural mutant.

Nine independently regenerated, compiled Omega mutants were killed by behavioural assertions, each **rc 1**, not by compilation failure (`resolve.log` and `mutant-*.log`):

1. Unknown-resource provenance acceptance.
2. Calibration expiry `>` changed to `>=`.
3. Pessimistic duration selecting the low endpoint.
4. Zero-sample prediction acceptance.
5. Bandwidth counterfactual multiplying by three instead of two.
6. Eliminated-op consumer proof bypass.
7. Deferred-op consumer acceptance.
8. Removed-time accounting dropping launch time.
9. Changed-reference membership erased.

## Commands and observed results

Every test, mutant, simulator and regeneration check ran inside:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 <command>
```

The wrapper hides NVIDIA, CUSE and VFIO devices. No GPU window or live service was touched. Build parallelism never exceeded three; the tools suite ran serially. `commands.json` contains exact wrapped commands, return codes and final lines. Logs replace the home directory prefix with `~`; no results were altered.

| Wrapped command | rc | Observed final line |
| --- | --- | --- |
| `make -j3 test-dagsim-resolve-om` | 0 | `dagsim resolve Omega differential: PASS` |
| `make -j3 test-dagsim` | 0 | `omega mutants: 5 killed` |
| `make -j3 test-dagsim-parse-om` | 0 | `gcov: oracle and unpacker branch receipts written` |
| `make -j3 test-dagsim-corpus-om` | 0 | `dagsim corpus: PASS (1200 cases, driver and flipped-record red controls)` |
| `make -j1 test-tools-om` | 0 | `tools-om artexp: PASS (differential 116 cases, 266 files, negative control, 10 mutants killed)` |
| `make -j3 test-costmodel` | 0 | `mutant restore-op-clamp: killed by unsupported-op-not-aliased-to-matvec (exit 1)` |
| `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` |
| `crumb compile . && crumb verify .` | 0 | See `crumb.log` for the current crumb count and verdict. |

The focused resolver gate was rerun after extending its source partition and moving coverage capture before mutants. The other full-suite results use the same implementation; subsequent changes at that point affected only the resolver test partition.

## Scheduler integration boundary

The inherited scheduler still compiled `ds_eval_c` unconditionally and used a weak `omega_dag_eval` fallback. `scheduler-fallback.log` reproduces it: a default build omitting `src/cudapp_om_dag_eval.c` linked and evaluated successfully, with output identical to the linked scheduler. This was handed to the parent and then explicitly assigned back for a separate scheduler-boundary commit. It is not evidence that the default binary is fail-closed without its Omega scheduler.

## Tier verdicts

- IMPLEMENTED: resolver/diff/lookup/counterfactual computation and packing decisions in seeded Omega; legacy resolver C is default-OFF.
- EXERCISED: host differential **PASS**, runtime negative control **PASS**, nine compiled behavioural mutants killed **PASS**, requested focused/full host gates **PASS**, byte-for-byte regeneration **PASS**, crumb compilation/verification **PASS**.
- QUALIFIED: chip **NOT_RUN**. Host and synthetic results are not hardware qualification.
- MERGED: **NOT_RUN**. Draft PR only.
- DEPLOYED: **NOT_RUN**. No live services changed.
