# Standalone checker computation

`oracles.om` owns the reference arithmetic and verdict predicates shared by the
standalone CUDA fixture, interop probe, attention dispatcher, prefill cache and
scoped checks, and exact-head checks. `om/units.sh` generates the committed
`src/cudapp_om_standalone_oracles.c` and its identity record. The generated entry
is `omega_so_oracles`; `oracle.h` makes a strong reference and aborts if the
checked bridge reports a trap. There is no weak-symbol or runtime C fallback.

`CUDAPP_MIGRATION_PENDING_C_STANDALONE_ORACLES` defaults to zero. Setting it to
one selects the independent C snapshot in `oracle.h`, solely for migration
comparison. `oracle-c.c` exposes that snapshot to the differential checker.
The test builds compare the original target sources from the specified base,
the default Omega path, and this C snapshot separately.

## Private operation interface

The entry takes five `ulong` arguments: operation, a, b, c, d. Float values
cross the interface as their IEEE bit encodings. Unused arguments are zero.
This is a private checker interface, not a public CUDA++ API.

| Operation | Result |
|---|---|
| 0, 1, 2 | vadd a input, b input, and uint32 sum with C-compatible wrapping |
| 3, 4 | integer equality and three-way nonzero conjunction |
| 5, 6, 7 | ulong sum, `(a+c)*b` extent, and 256-thread grid rounding |
| 8, 9, 10 | low/high pointer assembly, mirror extent, expected copy time |
| 11, 12, 13, 14 | cache fixture tile, mutation index, expected allocation count, paired equality |
| 15, 16, 17 | reading tile/group boundary and exact-head admission reserve |
| 18, 19 | exactly representable f32 inputs for the BF16 matmul fixture |
| 20, 21 | f64 absolute difference between f32 values and strict `> 0.0001` test |
| 22, 23 | wrapped RNG transition and its f32 sample |
| 24, 25 | finite f32 predicate and absolute-weight maximum |
| 26, 27 | interval and dense/sparse/alternating normalizer fixture values |
| 28 | increment the failure count if its predicate is zero |
| 29, 30 | f32 greater-or-equal and ordered maximum, preserving operand order |
| 31, 32 | interval fixture exponents |
| 33, 34, 35, 36 | unsigned greater/less/subtract and matching success/failure classes |
| 37, 38 | f32 greater-than and ordered f64 maximum |
| 39, 40 | adversarial weight sign and negative normalizer exponent |
| 41 | select b when a is zero, otherwise c |
| 42 | exact BF16 fixture bit narrowing; not a general quantizer |
| 43 | one separately rounded f32 BF16 product and accumulator addition |
| 44 | three-way nonzero disjunction |

Explicit masks precede Omega's checked numeric narrowing where the original C
operation truncates to uint32. Negative exponents cross the ulong ABI as
modular two's-complement encodings. Float arithmetic and C comparison builds
use `-ffp-contract=off`; there is no contracted multiply-add substitution.

## Independent higher-precision reference

binary128 oracle stays C: seeded Omega has no binary128 type; proposal = f128 soft type or u64-pair arithmetic in om/numeric

The existing long-double interval reference, slack calculations, square/sum,
square root, and their higher-precision comparisons remain independent C
oracles. Their Boolean combinations, finite f32 checks, maxima on f32 results,
shape/budget calculations and failure aggregation use Omega. The Spark compiler
reports `__LDBL_MANT_DIG__ = 113`; no reference is silently reduced to f64.
Existing `ldexpf`, `nextafterl` and `sqrtl` calls remain recorded libm sites.

## Focused host checks

Every test, simulation and mutant must run with devices hidden:

```sh
export OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc"
export OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge"
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make test-standalone-oracles
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make test-standalone-oracles-family
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make test-standalone-oracles-mutants
```

The family target needs the CUDA 13 header but not a CUDA library or device.
`host-cuda.c` is a verification-only CPU transport linked instead of libcuda;
it runs the two CUDA-calling sources without loading a driver. It is not a
replacement CUDA library or an implementation fallback. `host-norm.c` supplies
CPU memory and executes the real emitted normalizer IR with the existing warp
simulator. The `--chip` argument in this host-only variant does not make its
result chip qualification. The real `make cubin-probe` remains outside host
acceptance targets and still requires its existing operator window.

`family-check.py --base REV ...` extracts only the eight original checker
sources from REV and compares their output with the migrated versions against
the same unchanged host dependencies. Without `--base`, it compares the C
snapshot with Omega. It rejects a corrupted transport result or Omega predicate
for every check, and also requires a missing-unit link to fail.

`scalar-check.c` crosses source-derived boundaries at v-1/v/v+1 and integer
endpoints, including uint32 truncation, tile/row thresholds, sparse tail, and
both IEEE float classes. NaNs compare by class, not payload. Tolerance bit
neighbors come from the Wolfram query recorded in the evidence README.
`mutants.py` compiles each changed `.om` source through the pinned bridge and
requires a behavioral mismatch; compile failures do not count as killed mutants.

See `evidence/standalone-oracles-20261106/README.md` for exact runs, outputs,
fixture v2 identities, limitations and the NOT_RUN chip runbook.
