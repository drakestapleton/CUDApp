# API views rest — WIP receipt, 2026-11-06

This is an implementation checkpoint, not a completed migration or behavior qualification. Pending omega-bridge#71/#73. The source pin is omega-mv e5b32567 and omc SHA-256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f. The private bridge checkout is #73 at 79e8c3613a6f.

## Exercised checks

All compilation checks below used the assigned device-isolation prefix:
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.

| Command | rc | Final line / observation |
|---|---:|---|
| `env OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_SEED=~/work/cudastack/omega-bridge/build/bootstrap/omc OMEGA_BRIDGE=~/work/cudastack/lanes/ApiViewsRestBridge sh om/units.sh` | 0 | wrote checked portable Omega units to the worktree src directory |
| Same environment, isolated `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` |
| Isolated `gcc -std=gnu11 -Wall -Wextra -Werror -D_GNU_SOURCE -O2 -ffp-contract=off -Isrc -fsyntax-only` followed by all ten assigned C paths | 0 | no output |
| Same syntax command with `-DCUDAPP_MIGRATION_PENDING_API_VIEWS_REST=1` | 0 after repair | no output; initial run found unused `x_rows`, then the oracle-only explicit unused cast repaired it |

The exact ten paths are listed in RESUME.md. Syntax success is not link success or behavior evidence.

## Separate `part` rename

Commit f138336 renames the forbidden local `part` to `pp` in `om/api/apish_shp.om`. The generated C bytes did not change. At that commit, isolated serial API-shell checks returned:

| Command | rc | Final line |
|---|---:|---|
| `make test-api-shells-diff` | 0 | `PASS api-shells checks=753157` |
| `make test-api-shells-mutants` | 0 | `PASS mutants killed=8` |

An initial parallel invocation raced on the shared output filename and failed the differential. The mutant runner now gives each invocation a private temporary OUT path. The serial checks above were run after that repair. These checks have NOT been rerun against the final host-shell edits; do not treat them as final-branch behavior evidence.

## Tier verdicts

- IMPLEMENTED: registered runtime-extent transport, packing, reduction, cache, slot, tensor-core and FP8 host units; substantial default-Omega C cutovers.
- EXERCISED: source compilation and generated-unit reproducibility only, plus the isolated rename checks above.
- Final host differential, source partitions, red check, coverage and this slice's mutant suite: NOT_RUN.
- Host/simulator family targets on final branch: NOT_RUN.
- Chip, live-RM, VFIO: NOT_RUN.
- QUALIFIED, MERGED, DEPLOYED: no.

## Required benchmark table

No benchmark samples were collected. Do not infer a speedup from the implementation.

| Moved loop group | C samples | Omega samples | Wolfram median of three | Ratio |
|---|---|---|---|---|
| FP8 oracle row accumulator and magnitude | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN |
| FP8/Tensor-core permutation and row gather | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN |
| Tern packing, prefill copy/ratios, slot transitions, descriptor/argument fills | NOT_RUN | NOT_RUN | NOT_RUN | NOT_RUN |

The FP8 host reference uses two passes: accumulator and magnitude are each f64, so both cannot be packed into a uint64 result. Main approved two passes and prohibited a seven-slot extension. Measure the duplicated product work under the required isolation and compute all medians and ratios through Wolfram before completing this PR.

## Completed continuation — ApiViewsRest2, 2026-11-06

This section supersedes the WIP verdicts above; their historical receipts are
unchanged. Implementation base: `c1d68bb9854c3bc995a111f2cd7a366eb15da265`.
Source/compiler pin: omega-mv `e5b32567a7aff3ce594e3254a622a846a6cd190b`;
compiler SHA-256 as above. Converter: `/tmp/bridge-73-apiviews` at
`79e8c3613a6f6f9e8eaaa9c013fc25bcb39e0538`.
Canonical providers were adopted unchanged from CheckersRest2 `343a061`:
`cg_pack_len/cg_pklen`, `cg_pack_h/cg_pkh`, `cg_mrow/mrow`.
No duplicate provider implementation was introduced.

### Implemented

- MoE cache scans, pinned-victim selection, upload chunk/window/row iteration,
  canonical fragment packing, row compaction and launch/reservation arithmetic.
- Remaining prefill GEMM and host descriptor/allocation/launch arithmetic.
- Standalone linkage includes every API-rest unit and the three providers;
  the original API-shell standalone runner now links slot, idle and pad.
- Fixed checked narrowing of packed pointer/statistic halves, SWIGLU source
  argument order, four-segment endpoint/scale view sizes and slot pin wrap.
  The slot boundary mutant reproduces the missing-wrap failure.
- The retained C configuration now builds with strict unused-function warnings;
  its unused default-only FP8 GEMM argument writer is correctly guarded.

### Final exercised commands

Every command below used
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.
Builds used at most `-j3`; no full `make test` or GPU path was run.
Environment: `OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc`,
`OMEGA_SEED=~/work/cudastack/omega-bridge/build/bootstrap/omc`,
`OMEGA_BRIDGE=/tmp/bridge-73-apiviews`; family runner additionally uses
`OMEGA_ROOT=~/work/cudastack/omega-mv`.
Receipts are in `continuation/`; absolute home paths in those new receipts
were normalized to `~`. Earlier failed/intermediate receipts are retained;
the filenames below identify the final results.

| Command | rc | Final result | Receipt |
|---|---:|---|---|
| `make -j3 build/libomega_gpu.a` and family runner relink | 0 | default library and family executables linked | `default-link.txt`, `families-final.txt` |
| `make -j3 OUT_DIR=build/api-rest-c-oracle CUDAPP_ENCODER=c 'CC=cc -DCUDAPP_MIGRATION_PENDING_API_VIEWS_REST=1' build/api-rest-c-oracle/libomega_gpu.a` | 0 | C-oracle configuration archive created | `c-oracle-link.txt` |
| `python3 tests/api_rest_mutants.py` | 0 | baseline `PASS api-rest checks=1204450 red=low-half-corruption`; `PASS api-rest mutants killed=7` | `mutants-final.txt` |
| `sh tests/api_rest_families.sh` | 0 | `PASS api-rest family host/sim; chip NOT_RUN` | `families-final.txt` |
| `OUT=build/api-rest-coverage-complete REST_CFLAGS='--coverage -O0' sh tests/api_rest_run.sh` | 0 | `PASS api-rest checks=1204450 red=low-half-corruption` | `oracle-coverage-run.txt` |
| `gcov -b -c -n build/api-rest-coverage-complete/api_rest_diff.gcno` | 0 | `Lines executed:99.06% of 320` | `oracle-coverage-final.txt` |
| `bash tests/api_shells_run.sh` | 0 | `PASS api-shells checks=753157` | `standalone-link.txt` |
| `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` | `units-check.txt` |
| `cc -std=gnu11 -O2 -ffp-contract=off -Wall -Wextra -Werror -Isrc tests/api_rest_bench.c build/libomega_gpu.a -lm -lpthread -o build/api-rest-bench` | 0 | linked, no diagnostics | `benchmark-build.txt` |
| `build/api-rest-bench` | 0 | final eight-group samples recorded | `benchmark-final.txt`, `benchmark-samples.csv` |

Own partitions cover the migrated functions independently of the old shell
suite: source-loop expected values, view refusals, pointer-half and byte-offset
boundaries, tile/chunk thresholds at v-1/v/v+1, 0 and UINT32_MAX.
The seven killed source mutants are pointer low-half truncation, invalid cache
admission, pinned-victim selection, upload byte count, both SWIGLU argument
orders and slot pin wrap. Both own test translation units have 100% executed
source lines and branches; branch outcomes taken are 66.33% and 67.21%.
The retained scalar-oracle header has 96.84% executed lines and 93.24% executed
branches; this is not a claim of exhaustive branch-outcome coverage.

Family host/sim PASS: tensor-core GEMM, FP8 GEMM, prefill GEMM, prefill host,
tgemm, tern and FP8 API. Tern host/sim: 56/34 checks; FP8 API: 4997/214
checks; slots host: 24 checks; MoE window host: 23 checks. The MoE-window
test has no simulator mode; its changed iteration/arithmetic paths are
exercised by the own differential, not claimed as device execution.
The external tern algebra test must use CUDApp's current simulator, as the
family runner does; its stale quoted simulator otherwise gives false
row-major failures, independently reproduced with the retained C oracle.

### Measured host benchmark

Each value is the median elapsed ns for 1000 iterations, from three samples.
Wolfram query and exact result: `continuation/benchmark-final-wolfram.txt`.
These checked portable Omega bindings are slower, not a speedup.

| Moved work | C median ns | Omega median ns | Omega/C |
|---|---:|---:|---:|
| A fragment packing | 1073351 | 147642147 | 137.55253128 |
| Row compaction | 17920 | 2617584 | 146.07053571 |
| Launch descriptor | 3728 | 179121 | 48.04747854 |
| FP8 sum and magnitude | 327474 | 18127103 | 55.35432737 |
| FP8/TC permutation and gather | 947302 | 56150134 | 59.27374164 |
| Tern packing | 206081 | 12183818 | 59.12150077 |
| Slot pin transition | 4416 | 239298 | 54.18885870 |
| Prefill copy and ratios | 1073111 | 905787055 | 844.07582720 |

FP8 Omega sum and magnitude retain the mandated separate f64 passes.
There is no chip timing or hardware-performance inference.

### Final tiers

- IMPLEMENTED: API-rest completion and the repairs above.
- EXERCISED: host linking, source differential/red controls, seven killed
  mutants, available family host/simulator gates, coverage and regeneration PASS.
- QUALIFIED: host behavior only as scoped above; hardware qualification NOT_RUN.
- Chip, live-RM, VFIO: NOT_RUN. No service, GPU, driver or boot state changed.
- MERGED, DEPLOYED: no. PR remains draft.
