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
C operation-clamp mutant. This exercised the migrated model arithmetic and
ranking on the existing tests; it is **not** a C-vs-Omega decision identity proof.

`cm_blob_unit` passed after blob/digest cutover. It compares initialization,
serialization bytes, digest bytes and deserialization against the retained C
oracle, checks digest tampering, all header-bit perturbations, both header byte
orders, SHA block/padding boundaries, lengths 0 through 255 and larger inputs.

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

The original assignment still needs full artifact manifest/admission/JSON and
remaining owned API plan migration; all-input C-vs-Omega decision differentials
and dedicated source mutants for the newly migrated model arithmetic/ranking;
and a correct-rounding argument beyond the finite libm sweep. Do not describe
this draft as qualified or as a completed CUDA-stack replacement. No ROADMAP
status has been upgraded.

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

## Artifact format and admission

The artifact public API is unchanged. The admitted input remains the canonical
`.manifest`, not JSON. The `.json` file is an output-only mirror; no JSON admission
endpoint or parser has been invented. Adversarial JSON-looking bytes are tested
as malformed manifest input. Python's independent JSON parser checks emitted
mirrors, rejects duplicate keys and invalid escapes, and distinguishes legacy
raw invalid UTF-8 from valid JSON rather than changing its encoding contract.

`artifact_format.om` owns ordered manifest layout, identity/export record
construction, JSON quoting and key order, numeric spelling, and checksum-list
emission. `artifact_admit.om` owns hash syntax and equality, field limits, launch,
contract, constant-bank and driver refusals and the admission sequence.
`artifact_elf.om` owns ELF section selection, bounds and exact text matching.
The default C shell owns pointers, retained allocations, file reads/writes and
SHA-256 calls. `CUDAPP_MIGRATION_PENDING_C_ARTIFACT=1` selects the original C
oracle. No public declaration or runtime API manifest entry changes.

The format wire has a 65,536-byte buffer (input below 20,000, QMD snapshots at
24,576 and 24,832, driver at 25,088, digest text at 26,000, output at 32,768)
and 128 scalar slots. Field offsets/lengths are at `8+k`/`48+k`; slots 88–98
hold export numeric values; slots 100–105 hold retained-file sizes. Admission
stages request an identity digest, five file digests, then ELF/text comparison.
The ELF pull protocol uses 512 bytes and 32 scalar slots; return 2 requests a
bounded byte copy, 1 accepts, and 0 refuses. Policy is never inferred from a
host-side pointer or a re-opened file. Existing read-once snapshot tests remain
the acceptance path.

Regenerate with the existing pinned `om/units.sh` tools. The ELF magic literal
comes from `WolframLanguageEvaluator:
FromDigits[Reverse[{127,69,76,70}],256] -> 1179403647`; this is a byte encoding,
not measured hardware evidence.

Focused host checks (no full suite or GPU):

```sh
taskset -c 10-14 nice -n 10 make -j3 test-artifact-format
taskset -c 10-14 nice -n 10 make -j3 test-artifact-format-pending
taskset -c 10-14 nice -n 10 env OMEGA_OMC=/path/to/pinned/omc \
  OMEGA_BRIDGE=/path/to/omega-bridge make -j3 test-artifact-format-mutants
```

The fixed-seed corpus is `0xA17FAC7`: 6,000 admission cases plus export/escape
cases, comparing exact refusal codes, artifact identities and all eight export
files. It includes duplicate/reordered records, malformed JSON-looking bytes,
oversize fields, invalid hashes, strict driver strings, and escape cases.
The differential's flipped expected byte must fail; source mutants must compile
and then be rejected, never count a compiler failure as a kill.

### ArtifactOm host receipt

IMPLEMENTED and EXERCISED on `feat/omega-artifact-format`, stacked on
RuntimeHost `16da758961c165dac9aef585137172175651c372` (#291). This receipt
supersedes the artifact-only host work listed under Remaining acceptance work;
it does not upgrade the cost-model, full-stack, or hardware claims.

The checked generator used omega-mv
`e5b32567a7aff3ce594e3254a622a846a6cd190b` and omc SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
`om/units.sh` completed with rc 0. The focused commands above ran serially,
under cores 10–14 and nice 10, with these observed results:

| Tier | rc | Observed verdict |
| --- | --- | --- |
| Default existing manifest fixtures | 0 | `native_artifact_manifest: 70 checks, 0 failed` |
| Default existing export fixtures | 0 | `test_gpu_artifact_export: 47 checks, 0 failed` |
| Existing CLI shape fixtures | 0 | `ARTIFACT EXPORT SHAPES PASS` |
| C/Omega differential of all three existing fixture sets | 0 | Same three verdicts; exact return codes, retained bytes, IDs and export files |
| Fixed-seed generated corpus | 0 | `ARTIFACT-FORMAT PASS export=256 admission=6000 seed=0xa17fac7 byte-exact strict-json` |
| Independent JSON codec/parser | 0 | `ARTIFACT-FORMAT codec PASS byte-domain=256 valid-utf8 strict-escapes duplicate-keys invalid-utf8-classified` |
| Flipped expected manifest byte | 0 | `ARTIFACT-FORMAT negative-control PASS expected-byte-flip rc=1` |
| PENDING C existing fixtures | 0 | 70 manifest checks and 47 export checks, zero failed; CLI shapes PASS |
| Compiled Omega source mutants | 0 | `ARTIFACT-FORMAT mutants PASS killed=5 compiled=5` |

The five compiled mutants remove backslash escaping, invert identity-digest
acceptance, change the ELF machine constant, reject equality at the current-driver
admission boundary, and swap the cubin/text hashes in the checksum list.
Each is rejected by a runtime differential assertion, not a compile failure.
The expanded focused mutant command above completed rc 0. The driver-boundary
mutant is caught at admission case 48 (Omega rc 8 versus C rc 0); the checksum
ordering mutant is caught at export case 0's `.sha256` byte comparison.
Logs remain in `build/artifact-format/mutant-artifact_{format,admit,elf}.log`,
`build/artifact-format/mutant-artifact_admit-driver-boundary.log`,
`build/artifact-format/mutant-artifact_format-checksum-order.log`, and
`build/artifact-format/negative-control.log`.

The differential target clears only its generated export/shape fixture
directories before each run, preserving EXCL refusal behavior while making the
host gate repeatable. The direct existing test recipes also track the new C
shell include, so changing it cannot silently reuse an old test binary.

PASS applies to the focused host tiers only. Full `make test` and chip/device
qualification are NOT_RUN. No service, GPU device, driver, gate, or compiler pin
was changed. This work is not MERGED or DEPLOYED.
