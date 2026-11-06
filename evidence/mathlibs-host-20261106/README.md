# Mathlibs host policy and planner migration

Date: 2026-11-06. Branch: `feat/mathlibs-host-om`.

## Scope and provenance

Base: `feat/mathlibs-gaps` (#317), `3d28358d48814779106588b19c94f62fc363d960`.
`git merge-base --is-ancestor origin/feat/mathlibs-rfft-2d-philox HEAD`
returned 0: this base already contains #303's RFFT, 2D and Philox additions.
Stack order is #303, then #317, then this draft; do not merge this directly
onto a main revision that omits either prerequisite.

`664f21c3` registers the seeded unit first. `27919d0a` adds its typed ABI
entries, generated bindings and differential gate before the C cutover.
The cutover and this receipt follow those commits. `sources.sha256` records
the exercised implementation and test source identities.

- **IMPLEMENTED:** default seeded Omega RNG admission/seed/count/poison
  arithmetic, clock arithmetic, legacy FFT admission/walk expansion, and
  PlanMany Stockham/Bluestein/axis/copy/layout/overlap planning.
- **EXERCISED:** device-masked host differential and existing host/sim gates PASS.
- **QUALIFIED:** chip, live RM, VFIO and other hardware tiers NOT_RUN.
- **MERGED / DEPLOYED:** no; this is a draft stacked change.

No public API or capability changes: public headers, manifest and API.md are
intentionally unchanged. The README describes the default path and focused
gate; ROADMAP has one dated note and no status-row upgrades. No language,
compiler, bridge pin, host word or harness protocol changes are introduced.

## Implementation boundary

`om/mathlibs/host.om` owns the computation. Its six small entry units are
`host_scalar.om`, `host_walk.om`, `host_step.om`, `host_many.om`,
`host_index.om` and `host_fft.om`. Separate typed entries follow the pinned
bridge's existing single-export/fixed-view ABI; there is no new converter or
second pin. `om/units.sh` registers them and generates six committed C units,
private declarations and identity receipts. Every entry is a strong reference.

`CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST` defaults to 0. The old C computation
is retained only behind that flag as the differential oracle. The default
adapter serializes inputs, calls Omega and performs effects: clock_gettime,
session locking, codegen/cache calls, allocation/free, byte loads/stores/copies,
callbacks, device launch, scratch transfers and synchronization. It does not
fall back to C when an Omega entry is absent or refuses an input. Clock reads
remain C effects; seconds/nanoseconds and elapsed-time arithmetic use Omega.
No new production dependency or GPU launcher is introduced.

The fixed PlanMany output workspace has 8192 ulong cells. Like the existing
`g_words` effect workspace, it is protected by the session lock; completed
launches and table bytes are materialized into owned storage before public
callbacks. This avoids both a per-plan workspace allocation and a large
per-thread stack frame. No plan record survives by referencing the workspace.
The unit initializes all consumed fields and records; stale unused cells are
not read. The descriptor bound is 204 records; the source-derived conservative
upper bound for admitted rank-three transforms is 96, including table records.
At most nine distinct table keys are needed, below the existing 16-key cache.
These are modeled bounds, not hardware measurements; queries appear below.

## Reproduce the exercised commands

All commands ran in this private worktree. The wrapper hides NVIDIA/VFIO
nodes, pins CPUs 10-14 and lowers priority. Builds use at most three jobs.
The runner refuses visible NVIDIA/VFIO nodes and executes its compilation,
coverage, differential, red, link-negative and mutant children inside the same
wrapper. No full `make test`, chip command or live service action was run.
Paths in retained logs replace `/home/drakestapleton` with `~`.

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  env OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc" \
      OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge" \
  make test-mathlibs-host-om

bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  make -j3 test-gpu-mathlibs

bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  env OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc" \
      OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge" \
  sh om/units.sh --check
```

| Command / tier | rc | Observed final result |
| --- | ---: | --- |
| `make test-mathlibs-host-om` inside wrapper | 0 | `MATHLIBS-HOST-OM GATE PASS: differential, red, strong-link negative, gcov, 8 killed .om mutants` |
| Differential child, `-O0 --coverage -ffp-contract=off` | 0 | `MATHLIBS-HOST-OM PASS cases=2477` |
| Intentional wrong-index red child | 1, expected | `FAIL line 232 case 2477: omega_mh_index(l,e,1,1)==2` |
| Missing-six-units link child | 1, expected | `collect2: error: ld returned 1 exit status` |
| `make -j3 test-gpu-mathlibs` inside wrapper | 0 | `mathlibs sim: PASS (6265 checks)` |
| `sh om/units.sh --check` inside wrapper | 0 | `PASS portable Omega units regenerate byte for byte` |
| `crumb compile . && crumb verify .` before commits | 0 | `crumb verify: OK: 39 crumbs current` |

The missing-unit log contains unresolved `omega_native_mhval`, `mhwalk`,
`mhstep`, `mhmany`, `mhidx` and `mhfft` references, including references from
the production API object. No weak-symbol or C fallback passes the control.
`acceptance.log` retains the complete combined command output;
`differential.log`, `red.log`, `missing-unit.log` and `mutants.tsv` retain the
focused results.

The unchanged family gate also reports host RNG fixture 792 rows / zero
mismatches, Philox fixture 1024 rows / zero mismatches and the existing
Philox distribution negative controls. Its normal-double fixture rows are
explicitly not compared because that existing IR has no f64 normal path;
this receipt does not turn that inherited limitation into a PASS.

## Source-derived partitions

| Surface | Exercised values / comparison |
| --- | --- |
| Legacy FFT | Every power of two 4..2048, both directions, all legacy modes; invalid dimensions and nonpowers; rank 1..3, summed-log bound rejection; batches 0, 1, 1023, 1024, 1025. Compare rc, byte requirements, every launch field and twiddle bytes. |
| PlanMany shapes | 2, 3, 5, 6, 7, 8, 11, 16, 30, 64, 127, 1024, 2047, 2048, 2049, 4095, 4096; rank 1..3 and C2C/R2C/C2R, both directions, in/out of place. Multi-Bluestein axis pairs (13,11), (11,31), (31,11) exercise workspace growth/reuse and distinct table keys. |
| Layout fields | For stride, distance, embed, rank and each dimension axis: 0, 1, 2, 3, 4, 5, 15, 16, 17, 4095, 4096, 4097, UINT32_MAX. Include zero stride, overlapping rows/batches and explicit collision bitmaps. Compare admission codes; successful plans compare all launches, bytes and table bytes. |
| Global and scratch products | n=4 batch 33554431/33554432/33554433; Bluestein n=11 batch 8388607/8388608/8388609; layout distance 268435451/268435452/268435453. These surround the source's point, scratch and byte-range bounds without allocating those extents in rejected cases. |
| RNG admission / seed | Every RNG kind and invalid kinds; stream bound 131071/131072/131073, zero and UINT32_MAX; per-stream limits and adjacent values; seeds 0, 1, UINT64_MAX and wrap-boundary values. Compare unsigned seed increment, poison predicates and count arithmetic against C. |
| Public effectful API admission | Default and C-oracle RNG, Philox and FFT calls with null/invalid inputs, bad sizes/types and overlapping valid host buffers. These return before GPU effects. Valid device execution remains NOT_RUN. |
| Failure controls | Null plan/bytes/dimensions, invalid mode/type/rank/inverse/in-place values; callback abort propagation; intentional wrong index; omitted mandatory unit symbols. |
| Float classes | Poison-scan words include positive/negative zero, subnormal, infinity and NaN encodings. These are raw sentinel classification/count tests, not a new floating arithmetic path: different NaN payloads are not asserted numerically equal. Plan/twiddle payloads compare exactly; existing simulator numeric checks remain unchanged. Compilation disables FP contraction. |

### Coverage limits and branch dispositions

`coverage.log` is gcov from the separately compiled C-oracle/test translation
unit, **not** a claim of Omega branch coverage or whole-library chip coverage.
It reports 68.22% of 1010 lines overall; the API oracle alone reports 60.28%
of 798 lines. `many_build`, all many layout/index helpers, `many_axis`,
`many_copy`, `many_scan`, `lg_in` and `rng_args_ok` have 100% line execution;
`walk_locked` has 98.53% of 68 lines. Line coverage is not branch coverage.
`omega_gpu_mathlibs_api.c.gcov` retains the annotated oracle source.
`coverage-disposition.tsv` enumerates every uncovered oracle branch and maps
it to a disposition below; none is silently counted as covered.

| Disposition | Why not exercised |
| --- | --- |
| `ALLOC` | Allocation-failure injection is not part of this gate; normal/reallocation paths and callback effects are exercised. Internal collector capacity is bounded by admitted IR steps. |
| `TABLE_INTERNAL` | Cache overflow requires more than 16 distinct keys, but admitted rank-three plans need at most nine. Other untaken legs require allocation failure or refusal of a validated internal table request. |
| `FACTOR_INTERNAL` | Invalid factor/radix/divisor responses require corrupted internal IR; validated dimensions supply nonzero divisors and valid factors. Remaining legs propagate allocation/IR failure. The real workspace-size rejection is exercised. |
| `COPY_INTERNAL` | Zero extents cannot reach copy construction from admitted nonempty dimensions; reciprocal-generation refusal requires invalid internal divisors. |
| `BASIC_BOUND` | The basic dense-layout embed bound is looser than the preceding global point bound, including the real in-place padding. Thus its overflow refusal is unreachable for admitted default layouts. |
| `PLAN_FAILURE` | Allocation/internal-IR failure propagation remains unexercised. The loop's rc-failure condition is bypassed by its explicit break. Both real length-versus-half-length outcomes are exercised. |
| `WALK_INTERNAL` | Untaken internal-codegen legs require malformed/refused validated IR or table allocation failure. Callback abort is injected at the first launch; later-stage callback-abort return legs are not separately injected. |
| `EFFECT_NOT_RUN` | Outside the pure-planner oracle comparison: device/session/launch/scratch/sync and cache/codegen effect management or their failure legs. Production host/sim codegen checks run separately; real device effects remain NOT_RUN. |

## Omega source mutants

Each mutation is unique in `host.om`, regenerated through the pinned bridge
and compiled successfully. A regeneration or compilation failure is not
counted as a kill. Every executable below returned 1 instead of PASS.

| Mutant | Generate rc | Compile rc | Run rc | Result |
| --- | ---: | ---: | ---: | --- |
| RNG stream limit 131072 -> 131073 | 0 | 0 | 1 | KILLED: admission mismatch |
| Seed increment i+1 -> i+2 | 0 | 0 | 1 | KILLED: seed mismatch |
| 64-bit poison constant last digit | 0 | 0 | 1 | KILLED: sentinel mismatch |
| Chunk grid bound 1024 -> 1023 | 0 | 0 | 1 | KILLED: launch mismatch |
| Zero-stride predicate 0 -> 1 | 0 | 0 | 1 | KILLED: admission mismatch |
| Real half-length +1 -> +0 | 0 | 0 | 1 | KILLED: plan byte mismatch |
| Axis rotation chooses wrong first axis | 0 | 0 | 1 | KILLED: plan collection mismatch |
| Overlap bitmap bit mask 7 -> 6 | 0 | 0 | 1 | KILLED: fill/index mismatch |

## Pins and arithmetic receipts

- Seeded omc revision `e5b32567`; binary SHA-256
  `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Existing toc pin: `69d629df170a24ba0c2cb6d41dbfaed520e248e0d723236dd32abf0581524d84`.
- Existing bridge-seeded pin: `23ff7a2356bb5d980060a4685a27ac46587013069b7d24b37c2873d7ee659b37`.
- Existing freest pin: `836cbf9ef8a3c4ee82b0458070884405bc8296f83b10f933556bf27750bb3661`.
- Existing bridge-freest pin: `51b41794e7001c5606b715fd7de8a34ec935a6e4e4bc1da36ad08102c009537d`.

WolframLanguageEvaluator queries (integer bounds/counts, not measurements):

```text
{FromDigits["ffbadbad",16],FromDigits["00badbad",16],FromDigits["ffbadbadffbadbad",16],2^28,2^27,1024*128,2^32,2^32-1,FromDigits["9e3779b97f4a7c15",16]}
-> {4290436013,12245933,18427282365706066861,268435456,134217728,131072,4294967296,4294967295,11400714819323198485}

{2^28-4,2^28-4+{-1,0,1},Floor[(8192-32)/40],3*(4+3*Ceiling[13/2])+12+9,2^32-1,2^28-1}
-> {268435452,{268435451,268435452,268435453},204,96,4294967295,268435455}

{2^27/4,2^27/4+{-1,0,1},2^28/32,2^28/32+{-1,0,1}}
-> {33554432,{33554431,33554432,33554433},8388608,{8388607,8388608,8388609}}

{3*3,8192*8,Floor[(8192-32)/40],3*(4+3*Ceiling[13/2])+12+3*3}
-> {9,65536,204,96}
```

The last query gives table-key count, workspace bytes, output capacity and
conservative maximum records. The maximum Stockham pass count uses M<=8192;
each admitted axis contributes at most four other Bluestein records and three
Stockham sequences. Twelve copy/rotation records and nine table records are
reserved conservatively.

## Chip runbook — NOT_RUN

There is no authorized GPU window for this lane. The mandatory wrapper hides
devices and cannot produce hardware qualification. Do not remove that masking,
stop a serving process, touch the chip lock or load/unload a driver to run this
receipt. No historical authorization is reused.

A future operator must first obtain a current exclusive hardware window and
follow the then-current GPU/session authorization procedure, including service
ownership and permitted device nodes. Build the same committed revision and
retain its source/binding identities in a **new** evidence directory. With the
operator-approved device-visible execution wrapper, the existing executable's
chip tier is selected by omitting `--sim` and `--host-only`:

```text
./build/gpu_mathlibs_test --out <new-chip-receipt-directory>
```

This is a runbook command, not a command run by this lane. There is no `--chip`
flag. Preserve the executable's RNG, FFT, Philox, real/ND, poison, repeat and
negative-control receipts; unexpected refusal, mismatch or untouched output
is FAIL/INCONCLUSIVE, never PASS. Record device/driver identity and restore
services only under the operator's current instructions.

That existing chip entry point does not cover the complete PlanMany matrix.
Before qualifying PlanMany, an authorized device run must additionally exercise
C2C/R2C/C2R, Stockham and multi-Bluestein ranks, explicit layouts, overlap
refusals and batch/stride boundaries through the public API against the host
reference, including repeat/poison controls. The current host plans and
simulator results are not substitutes for that hardware check. All hardware
results, including PlanMany, remain NOT_RUN here; the integrator also owns the
full combined repository gate, which this focused lane did not run.
