# Runtime host policy migration

## Scope and status

IMPLEMENTED, host-only; not merged, deployed or chip-qualified. This is a pending
migration, not completion of the original all-host-logic assignment.

The generated defaults come from these first-party Omega units:

- `artifact_policy.om`: QMD template/launch admission, including the original
  uint32 shared-memory rounding wraparound. Byte views do not require aligned QMDs.
- `artifact_parse.om`: printable fields, lowercase SHA text, decimal/list and
  semantic-version parsing, and hexadecimal byte formatting.
- `api_policy.om`: shared matmul, RMSNorm, RoPE, attention-head, cache-range,
  bf16-weight and router-expert shape predicates. Allocation, device transport,
  pointer admission and error strings remain in the existing C adapters.
- `cm_features.om`, `cm_stats.om`, `cm_fit.om`, `cm_quantile.om`,
  `cm_prediction.om`, `cm_decision.om`, `cm_calibration.om`: cost-model basis,
  cells/defaults, statistics, Cholesky/prediction, quantiles, ranking/exploration,
  calibration and ledger arithmetic. Floating values cross the ABI as binary64
  bits, not as silently narrowed integers.
- `cm_sort.om`: 256-element sorted runs and bounded two-window merges. C retains
  allocation, copying and pointer advancement; ordering runs in Omega. The C
  comparator is total: numerical order, then unsigned bits for ties, NaNs last.
  This removes qsort's unspecified ordering of equal values, signed zeros and
  distinct NaNs. Tests compare the complete byte order against glibc qsort using
  that same total comparator, including ties and run-boundary sizes.
- `cm_math.om`, `cm_math_extra.om`, `cm_math_erfc.om`: first-party binary64
  log/log2/ceil/exp/exp2/sqrt/erfc/fabs. Integer bit manipulation, fma-free
  double-double arithmetic and an integer restoring square root are used. No
  production libm call substitutes for these implementations.
- `cm_blob.om`, `cm_digest.om`: blob header construction/validation, scale and
  length admission, SHA-256 and digest comparison. SHA-256 reuses the existing
  `om/cache/digest.om` rounds, constants and padding. C streams bounded blocks;
  the existing native blob layout, including ignored reserved bits, is retained.
  Little- and big-endian header packing are compared against a C reference.

Retained C oracles are selected by `CUDAPP_MIGRATION_PENDING_C_ARTIFACT`,
`CUDAPP_MIGRATION_PENDING_C_API_POLICY`, and `CUDAPP_MIGRATION_PENDING_C_CM_`
with suffix `SORT`, `MATH`, `FIT`, `STATS`, `FEATURES`, `QUANTILE`, `PREDICTION`,
`DECISION`, `CALIBRATION`, `BLOB`, or `DIGEST`. Every switch defaults to zero.
No public API or manifest changed. Generate with `sh om/units.sh` using the
existing pinned compiler and bridge; explicit `OMEGA_UNITS` overrides are honored.

## Exercised evidence

`make test-host-policy` covers shape Cartesian boundaries and deterministic
random triples, QMD bit perturbations and launch inputs, decimal/list/text
parsing, total-order sorting, binary64 functions, blob headers and SHA-256.
Source mutants of API envelopes, QMD relocation words, decimal digit limits and
sort order must compile and then fail the differential executable.

Before the final integration run, `make -j3 test-costmodel test-dagsim` passed:
`checks 127 failures 0`, `dagsim_unit: 203 checks, 0 failed`, pinned CLI fixtures,
unsupported/refused prediction mutants, inverted-lifetime mutant and the retained
C operation-clamp mutant. This exercises the default Omega model arithmetic,
prediction, calibration and ranking on the existing tests. The additional
direct differential below checks same-state decisions rather than assuming
parity from those standalone passes.

`cm_blob_unit` passed after blob/digest cutover. It compares initialization,
serialization bytes, digest bytes and deserialization against the retained C
oracle, checks digest tampering, all header-bit perturbations, both header byte
orders, SHA block/padding boundaries, lengths 0 through 255 and larger inputs.

`test-cm-decision-differential` links the unchanged direct model unit through
decision/prediction interceptors and the retained C implementation. It compares
actions, selected arms, measurement pairs, reasons, refusal/known/borrowed flags
and sample counts exactly, and records floating-output ULP distances separately.
Additional deterministic inputs cover unsupported operations, saturated
core/pressure classes, empty/partial eligibility, budgets and fallback indices.
Poisoned-output invalid-argument probes check the cleared prediction contract.
Assertion-disabled verifier builds are refused, not reported as passing.

The exercised run reported 21,058 decisions and 8,617 predictions, with all
discrete comparisons equal. This is parity over those exercised inputs, not a
universal decision-threshold proof and not independently trained-state parity.
The largest measured floating distance was 5,611,147 ULP in `mean_log2_ps`:
candidate `0x1.c1fea4ee44ep-6`, C `0x1.c1fea4e8eaf75p-6`. The harness records
the exact model/features/policy in ignored `build/cm_decision_max.case` so this
near-zero value can be replayed without regenerating its training fixture.

WolframLanguageEvaluator:
`a=FromDigits["1c1fea4ee44e",16]2^(-6-4 StringLength["c1fea4ee44e"]);`
`b=FromDigits["1c1fea4e8eaf75",16]2^(-6-4 StringLength["c1fea4e8eaf75"]);`
`N[{a,b,a-b,Abs[(a-b)/b]},30]`
returned absolute difference `1.9467576856113666 * 10^-11` and relative
difference `7.0880118476913915 * 10^-10`. These describe that measured case;
they are not a global model-error bound. The C oracle retains normal compiler
flags; a separately compiled `-ffp-contract=off` oracle is diagnostic only.
Replaying the captured case with **only** `cm_oracle.c` rebuilt using that flag
reported zero ULP difference across the complete decision output. This isolates
contraction as the cause for this case, not for every possible model state.

`test-cm-source-mutants` passed ten attributed source controls: unsupported-op
aliasing, doubled sample counts, incorrect variance division, reversed quantile,
incorrect spread scaling, changed measurement action, inverted calibration
clamp, changed blob magic, lost digest bits and the wrong logarithm base.
Every mutant compiled and failed a named numerical/decision/blob assertion.
The decision interceptor independently caught the action mutant, and its
assertion-disabled build was refused.

The binary64 sweep uses 65,536 deterministic random bit patterns per tested
function, signed special values, subnormal/binary-exponent boundaries and
neighboring values; exp/exp2/erfc also have targeted dyadic grids. Measured maximum
ULP distance from this host's glibc was respectively `0,0,0,1,0,0,2` for
`log,log2,ceil,exp,exp2,sqrt,erfc`. This finite sweep is not a proof of correct
rounding for all binary64 inputs. `fabs` is implemented but not included in that
reported seven-function sweep. Floating exception flags and errno compatibility
are not claimed.

Wolfram's high-precision checks disagree with glibc at the measured worst exp and
erfc samples and select the Omega result in both cases:

- `Exp[2^-26]` at 150 digits begins
  `1.000000014901161304869959263969`; rounded bits `3ff0000004000001`.
  glibc returned `3ff0000004000000`.
- For bits `402570dc08794a6a`, `Erfc[x]` at 150 digits begins
  `6.41098283195702340057758 * 10^-52`; rounded bits `354eb3d792b02a8a`.
  glibc returned `354eb3d792b02a8c`.

The final `make test` result is recorded in the draft PR, not inferred from the
focused runs above. Chip remains NOT_RUN.

## Wolfram derivations

The mounted WolframLanguageEvaluator supplied constants and reference values.
For positive/negative normal values, the bit conversion query used:

```wolfram
bits[v_] := Module[{e=Floor[Log[2,Abs[v]]],sig},
  sig=Round[N[Abs[v] 2^(52-e),150]];
  If[v<0,2^63,0]+(e+1023)2^52+sig-2^52]
```

`{bits[Log[2]],bits[1/Log[2]],bits[1/Sqrt[Pi]]}` returned
`{4604418534313441775,4609176140021203710,4603256987541740397}`.
Splitting each high-precision value at its rounded binary64 high part supplied
low parts with bits `{4358002977218854975,4357082904033627428,4350950281585880448}`.
`bits[Sqrt[2]]` returned `4609047870845172685`.

The Acklam coefficients are the exact decimal rationals of the retained C
`a,b,c,d` arrays, not newly fitted approximations. Applying `Map[bits,{a,b,c,d},{2}]`
to those rationals returned:

```text
a: 13854155686354486565 4641977866302784411 13866933838737128345
   4639072047187312667 13852696627858410445 4612826843888178297
b: 13856235683484533956 4639889312772538999 13863054224260258002
   4634401141363829182 13847038013971134502
c: 13798997430714945504 13822851435045880248 13835960482696009560
   13836295942911834650 4616611452376731079 4613798575908835234
d: 4575625165243457492 4599480671287182180 4612688371394471446
   4615636595525398809
```

`2^16` returned `65536`, the deterministic random-sweep size. With
`t=(Sqrt[2]-1)/(Sqrt[2]+1)`,
`N[2 t^67/(67 (1-t^2)),30]` returned approximately
`1.5703257028259 * 10^-53` for the log series truncation bound.
`N[Erfc[6],30]` returned approximately `2.1519736712498913 * 10^-17`, below
`2^-53`, used only for rounding the reflected negative erfc tail to 2.
These are analytic bounds, not measured accuracy claims.

QMD: `{BitShiftRight[16^^300,6],16^^7ffff,16^^ffff,16^^1ff,16^^7ff}`
returned `{12,524287,65535,511,2047}`; `2^32-1` returned `4294967295`.
Blob packing queries and results are also quoted directly in `cm_blob.om`.

## Remaining acceptance work

Artifact manifest/admission/JSON and remaining owned API plans are being
completed on disjoint stacked branches before integration. The direct model
input corpus and dedicated arithmetic/ranking source mutants are exercised
above. A universal correct-rounding or arbitrary-threshold identity proof is
not claimed by the finite sweeps. Do not describe this draft as hardware
qualified or as a completed CUDA-stack replacement. No ROADMAP status has been
upgraded.

## Chip runbook — NOT_RUN

Do not execute these commands outside a newly authorized exclusive Drake window.
The current authorization/runbook must establish service shutdown, GPU ownership,
watchdog and lock handling first; an apparently free lock is not authorization.
This lane has opened no GPU device and has not touched any live service.

After authorization, from the exact reviewed checkout, build the existing test
binaries through the host-only targets, then run the actual chip batteries:

```sh
mkdir -p build/runtime-host-chip
taskset -c 10-14 nice -n 10 build/gpu_matmul_api_test --out build/runtime-host-chip/matmul.json
taskset -c 10-14 nice -n 10 build/gpu_elementwise_test --out build/runtime-host-chip/elementwise.json
taskset -c 10-14 nice -n 10 build/gpu_attention_test --out build/runtime-host-chip/attention.json
taskset -c 10-14 nice -n 10 build/gpu_router_test --out build/runtime-host-chip/router.json
taskset -c 10-14 nice -n 10 build/gpu_bf16_gemv_test --out build/runtime-host-chip/bf16.json
taskset -c 10-14 nice -n 10 build/gpu_decode_attn_test --out build/runtime-host-chip/decode.json
```

Require each battery's green parity checks and deliberately wrong kernel checks,
record exact SHA/toolchain and exit statuses, and compare with C-oracle builds
using the same shapes. Host references and simulators are not chip qualification.
Pure cost-model/blob tests themselves require no GPU window.
