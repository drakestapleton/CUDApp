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
