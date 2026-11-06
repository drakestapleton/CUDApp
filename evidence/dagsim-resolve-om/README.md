# Dagsim resolution and diff: seeded Omega host evidence

## Revision and scope

Branch: `feat/dagsim-resolve-om`. Draft PR: [CUDApp #341](https://github.com/aien-dev/CUDApp/pull/341), based on `feat/dagsim-parse-om` / #331 at `79dac21d2fccae9a86239c2ab13cb2bcd8ffffb5`.

Registration was committed and pushed first as `58e7b49cbe150a08a716c3a65464204a388a98d8`: `om/units.sh`, the native declarations, generated bindings and unit identities. `pins.txt` records the compiler, bridge, source and dependency pins. The seeded compiler is omega-mv `e5b32567a7aff3ce594e3254a622a846a6cd190b`, executable SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

The resolver implementation and initial full host envelope are committed as `2cf2c484300db4014c65c03945f52f7fd5889f0f`. The scheduler fail-closed repair is a separate subsequent commit. `source-sha256.txt` pins the final exercised source files independently of receipt/commit metadata.

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

`oracle.gcov` and `resolve.gcov` capture the initial resolver cutover; `scheduler-oracle.gcov` and `scheduler-resolve.gcov` capture the subsequent scheduler-boundary repair. All are captured immediately after the unmutated differential, before negative controls/mutants. They are branch receipts, not a claim of complete coverage of unrelated parser/formatter code.

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

The initial full envelope above is recorded at the resolver cutover. After the scheduler-boundary repair, the resolver, dagsim unit/CLI/mutants, parser differential, 1,200-case corpus and byte-for-byte regeneration gates were all rerun successfully under the same wrapper. `scheduler-commands.json` records these final focused commands, each rc 0, with the same final lines as the corresponding rows above. Their logs are `scheduler-resolve.log`, `scheduler-dagsim.log`, `scheduler-parse.log`, `scheduler-corpus.log` and `scheduler-regenerate.log`. The full tools and cost-model gates were not rerun after this scheduler-only guard repair; their initial full-envelope results remain separately recorded.

## Scheduler integration boundary

The inherited scheduler compiled `ds_eval_c` unconditionally and used a weak `omega_dag_eval` fallback. `scheduler-fallback.log` preserves the pre-repair reproduction: a default build omitting `src/cudapp_om_dag_eval.c` linked and evaluated successfully, with output identical to the linked scheduler. The parent explicitly assigned this defect back to this lane.

The separate repair puts `ds_eval_c`, Kahn ordering, lifetime computation and scheduler state helpers behind the existing default-OFF `CUDAPP_MIGRATION_PENDING_C_TOOLS` oracle guard. The endpoint-name formatting helper remains shared. The default scheduler declaration is strong, and the weak-symbol runtime fallback is deleted. The Omega-to-C numeric adapter used only by the C oracle is also guarded. The flag-ON C scheduler remains exercised by the focused lifetime/prediction mutants and full-C corpus oracle.

The new regression in `resolve_check.py` builds a default CLI while deliberately omitting only the Omega scheduler binding. It must fail linkage with an undefined `omega_dag_eval` reference; the default object must contain a strong undefined scheduler reference and no `ds_eval_c` definition. `scheduler-boundary.log` records the expected linker rejection and symbol inventory. This is a **PASS negative control**, not an implementation build failure. The normal linked CLI, differential and corpus gates pass. No silent host scheduler remains in the default build.

Final metadata compilation and verification also passed, rc 0: `crumb verify: OK: 44 crumbs current` (`scheduler-crumb.log`). Generated crumbs are sealed again after staging the final receipts.

## Tier verdicts

- IMPLEMENTED: resolver/diff/lookup/counterfactual computation and packing decisions in seeded Omega; legacy resolver C is default-OFF. The inherited default C scheduler fallback is removed, with the C scheduler retained only behind its oracle guard.
- EXERCISED: host differential **PASS**, runtime negative control **PASS**, nine compiled behavioural mutants killed **PASS**, requested focused/full host gates **PASS**, byte-for-byte regeneration **PASS**, crumb compilation/verification **PASS**.
- QUALIFIED: chip **NOT_RUN**. Host and synthetic results are not hardware qualification.
- MERGED: **NOT_RUN**. Draft PR only.
- DEPLOYED: **NOT_RUN**. No live services changed.
