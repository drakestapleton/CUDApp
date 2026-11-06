# Engine fixture residue and clock scalars — 2026-11-06

## Scope and verdicts

IMPLEMENTED / EXERCISED on `feat/engine-fixture-rest-om`, based on
`feat/engine-submit-om` at `1ecaf8c40802dee63c9534944fae492005076033`
(PR #321). The four hardware fixture callers are `m16/m16_concurrent.c`,
`m16/m16_requalify.c`, `nvrm/lifecycle_gates.c` and
`nvrm/submission_visibility_gates.c`. Clock conversions are extracted from
`m16/m16_gpu_wait.c`, `m16/m16_native.c`, `src/omega_blackwell_engine.c`,
`src/omega_gpu_wait.c`, `src/omega_blackwell_submit.c` and
`src/omega_gpu_engine.c`. Elementwise's clock is not owned by this lane.

`om/engine/fixture_rest.om` owns scalar arithmetic/predicates, qualification
result conversion, bounded fixture-text checks, churn sizes/patterns, submission
sizing, flush-tail words and integer-to-f64 reporting scales. The existing
`omega_eng` export remains strong, with private operations 28..30 dispatching
these helpers; `omega_fr_text` is a separate strong bounded-text entry.
C keeps ioctl/mmap, clock_gettime, proc/filesystem reads, effects, iteration and
byte-moving shells. Effectful short-circuit order is preserved. The public API
is unchanged. The default-OFF references are
`CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST` and
`CUDAPP_MIGRATION_PENDING_C_CLOCK_SCALARS`; each flag is also independently
compared against the default Omega transcript.

| Tier | Verdict |
| --- | --- |
| Host C/Omega differential for every extracted helper | PASS |
| Independent oracle-flag selection | PASS |
| Missing strong entries link control | PASS: missing symbols fail to link |
| Red answer mutation | PASS: changed ns scale detected |
| Fresh recompiled Omega mutants | PASS: 7/7 killed |
| All four hardware fixture binaries | BUILD_ONLY PASS; never executed |
| Hardware / live RM / VFIO / chip qualification | NOT_RUN |
| MERGED / DEPLOYED | NO |

The first lane commit registers the units. Generated C is checked bridge output;
it is not a second hand-written implementation. No compiler/bridge pin, external
dependency, header API contract, or hardware admission gate changes.

## Reproduction and command receipt

Compiler: omega-mv `e5b32567`, `seed/out/omc` SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Use the base's existing `OMEGA_BRIDGE=~/work/cudastack/omega-bridge` and
`OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc`.
Regeneration: `env OMEGA_OMC=... OMEGA_BRIDGE=... taskset -c 10-14 nice -n 10 sh om/units.sh`
returned rc 0, final line `wrote checked portable Omega units to .../src`.

Every test, differential, mutant and simulation command runs inside:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 COMMAND
```

`COMMAND=python3 tests/om_fixture_rest_test.py` returned rc 0, final line:
`PASS fixtures BUILD_ONLY; HW NOT_RUN; mutants 7/7 KILLED`.
It builds both reference and Omega variants with `-ffp-contract=off`, compares
complete helper transcripts, exercises both individual flags, records gcov,
recompiles each mutant and finally **links without executing** the four fixture
binaries. `make test-engine-fixture-rest` registers the same focused acceptance.
The missing-unit negative returns a nonzero linker result, as required.
Text receipts accompany this file; compiled fixtures and mutant binaries are
not committed. Filed receipts replace the local home directory with `~`.

## Source-derived partitions

`tests/om_fixture_rest_test.c` enumerates all scalar operations 0..19 and each
pair of the source boundary values. It tests every flush-word index including
the invalid sentinel, both reporting scales, and each bounded-text pattern.

Existing focused gates also ran with the same no-device prefix and environment:

| COMMAND | rc | Observed result |
| --- | --- | --- |
| `make -j3 test-om-eng-diff test-om-eng-red` | 0 | `PASS om eng diff: 44584 records identical (om vs C)`; flipped answer detected |
| `make -j3 test-om-eng-mutants` | 0 | `PASS om eng mutants: 16/16 killed` |
| `make -j3 test-gpu-wait test-gpu-wait-mutants host-tests-m16` | 0 | `OMEGA_GPU_WAIT_HOST PASS`, `OMEGA_GPU_WAIT_MUTANTS PASS`, `M16_GPU_WAIT_HOST PASS`, `M16_GPU_WAIT_MUTANTS PASS (3/3 killed)` |
| `make -j3 test-gpu-engine-sim` | 0 | engine `SUMMARY passed=346 failed=0`; matmul bounds `SUMMARY passed=6 failed=0` |

These are additional host/simulator PASS receipts, not executions of the four
hardware fixtures. Full `make test` is intentionally NOT_RUN; the integrator
owns the aggregate gate.

| Source/helper | Partition contract |
| --- | --- |
| Six clock conversions | seconds and nanoseconds at 0/1, unit scale boundaries ±1, UINT32_MAX, INT64_MAX, UINT64_MAX; ns multiplication wrap at seconds 18446744073 ±1 and remainder 709551615 ±1; ms multiplication wrap at seconds 18446744073709551 ±1 |
| Concurrent qualification | unsigned subtraction/wrap, ns-to-ms quotient, all eleven flush tail words and invalid index, 32-bit VA wrap boundaries |
| Requalification proc/payload checks | equality and inequality at byte/word/64-bit extrema; exact libcuda/libcudart match, mismatch, NUL termination and end-of-512-byte buffer |
| Lifecycle churn and invariants | size-cycle indices 7/8/9; byte pattern 254/255/256; index 0/1/max; sum/add/sub/max and result predicates at zero/nonzero and both Boolean classes; log period 1999/2000/2001 |
| Visibility submission | distinct/reuse count, uint32 truncation of `2*entries+3`, payload offsets around 0/1/127/128/129 and UINT32/UINT64 extrema; byte count and combined verdict |
| Reporting scales | each integer source edge converted to f64 and divided by MiB/GiB with exact `%a` comparison and contraction disabled |
| Error/proc bounded text | all five actual patterns at every start position 0..511, missing/partial patterns, early NUL, pattern length 0 and >32 |

There are no floating-point inputs to the extracted functions: the reporting
function accepts ulong values, so negative zero, NaN, infinity and subnormal
input classes are not reachable API inputs. Integer zero and finite large values
are tested. This is not a claim about arbitrary floating-point conversion.
The existing clock behavior is unsigned modular arithmetic, including the
casts from `timespec`; no saturating/validation policy was added.

`gcov.log` records the C oracle header at 95.24% lines, 100.00% branches
executed and 98.11% branches taken. The unexecuted control is the invalid private
opcode abort, outside the source-derived operation domain. Coverage does not
imply a hardware fixture ran.

| Fresh `.om` mutant | Fault | Result |
| --- | --- | --- |
| ns_scale | multiply seconds by wrong ns scale | KILLED; red control |
| ms_divisor | divide nsec by wrong ms divisor | KILLED |
| size_cycle | use seven rather than eight churn sizes | KILLED |
| count_wrap | omit required uint32 submission-count wrap | KILLED |
| flush_offset | change flush address offset by one | KILLED |
| float_scale | change GiB divisor | KILLED |
| text_equal | invert bounded pattern equality | KILLED |

## Wolfram calculations used in extraction

WolframLanguageEvaluator:
`{Quotient[2^64-1,10^9],Quotient[2^64-1,1000],Mod[2^64-1,10^9]}`
-> `{18446744073,18446744073709551,709551615}` (seconds/seconds/nanoseconds
for unsigned wrap partitions).

WolframLanguageEvaluator:
`{BitOr[2*2^28,4*2^16,40/4],BitOr[2*2^28,5*2^16,92/4],2^31,FromDigits["46464646",16],1+2^20}`
-> `{537133066,537198615,2147483648,1179010630,1048577}`
(encoded words and fixture sentinel/address constants).
These are arithmetic facts, not hardware measurements.

## Chip runbook — NOT_RUN

These fixtures open hardware and **must not be executed during host acceptance,
even under bwrap**. Their successful link is only BUILD_ONLY evidence.
A future operator must grant the current exclusive GPU window, confirm exact
revision/compiler/generated unit pins, and approve the family qualification
commands before running anything below. Never infer permission to stop services,
open a device, alter modules, clear a lock or run VFIO from this receipt.

Inside that separately authorized window, use the four binaries built by the
focused runner in `build/fixture-rest/`: `m16_requalify`, `m16_concurrent`,
`lifecycle_gates` and `submission_visibility_gates`. Follow their existing family
runbooks/arguments, capture each exit code and complete gate transcript, repeat
the concurrency/lifecycle scenarios, and include the existing intentionally
refused submission/overflow/quarantine cases as negative controls. If comparison
against the default-OFF C reference is authorized, rebuild the same revision with
both flags set, run the same approved cases, and compare invariant outcomes
(timings are measurements, not expected byte-identical logs). Record a new
receipt with exact hardware, revision and commands. Until then hardware,
live-RM and VFIO are NOT_RUN, not PASS, and no roadmap row is upgraded.
