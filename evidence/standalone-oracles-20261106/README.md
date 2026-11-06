# Standalone oracle migration — 2026-11-06

## Scope and verdicts

IMPLEMENTED: the eight Lane 23 checker programs use the seeded
`om/standalone-oracles/oracles.om` unit by default for integer/float fixture
construction, shape/budget policy, reference BF16 accumulation, error thresholds,
finite/class checks, comparisons and verdict aggregation. The former C scalar
implementation is available only with the default-OFF
`CUDAPP_MIGRATION_PENDING_C_STANDALONE_ORACLES=1` verification flag.
`omega_so_oracles` is a strong reference; removing its implementation fails linking.
Driver calls, byte transport, filesystem/clock effects and independent numerical
oracles remain C. No public API changed.

EXERCISED: host scalar differential PASS; all eight host checker before/after
comparisons PASS; eight red controls rejected; nine compiled Omega source mutants
killed; focused attention, prefill and q38 host/simulator gates PASS; C0 static
admission and fixture reproducibility PASS.

QUALIFIED: chip/live-RM/VFIO/hardware **NOT_RUN**, no authorized GPU window.
MERGED: no. DEPLOYED: no. This receipt does not qualify a driver, CUDA loader,
physical kernel or replacement GPU runtime.

Base source revision: `b774723f537cec16a359fe082b8100317a5b5490`.
The first branch commit, `b6ddd225`, registers the unit before the caller cutover.
See the PR head for the final implementation revision; this receipt is committed
with that implementation rather than claiming a self-referential commit hash.

**binary128 oracle stays C: seeded Omega has no binary128 type; proposal = f128 soft type or u64-pair arithmetic in om/numeric**

The preserved `long double` computations in q38 `check.c` and `norm-check.c`
are explicitly marked independent binary128 reference oracles. This host reports
`__LDBL_MANT_DIG__=113` (see `identities.log`). They are not silently lowered to
binary64. Their high-precision reference arithmetic/comparison remains C; the
representable f32/f64/integer policy and boolean aggregation is Omega. Existing
`ldexpf`, `sqrtl`, `fabsl`, `fmaxl`, `isfinite` and related libm reference operations
remain host-library oracles, not a new Omega production dependency. Historical
`om/q38-head/envelope-validation.json` is untouched and is not a qualification
receipt for this change.

## Pins and tools

- omc source: omega-mv `e5b32567`; executable SHA-256
  `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Existing bridge converter SHA-256:
  `69d629df170a24ba0c2cb6d41dbfaed520e248e0d723236dd32abf0581524d84`.
- Existing bridge frontend SHA-256:
  `23ff7a2356bb5d980060a4685a27ac46587013069b7d24b37c2873d7ee659b37`.
- Final Omega source SHA-256:
  `07b7193434753f3a0ee54f9aaa1c9a3c3bdebc298fbb5dfdf3ead34604e905a3`.
- Final generated C SHA-256:
  `f5310a8a0e6d9ed9a875c09d1894f7430190d2556e1ddee0d35d09852fdea2c8`.
- Host verification tools measured in `identities.log`: GCC 13.3.0,
  aarch64-linux-gnu, Python 3.12.3. CUDA header identity is CUDA_VERSION=13000;
  headers and link-only fixture stubs do not authorize or execute a GPU call.
- Generated bridge C is committed with the normal `om/units.sh` identity records.
  Native/reference float compilation uses `-ffp-contract=off`.

## Commands actually run

Commands below ran from the lane worktree. Every test, simulator and mutant was
inside `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.
No full `make test`, GPU probe, live-RM test or service operation ran.

```sh
export OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc"
export OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge"
# Generation/build only:
taskset -c 10-14 env OMEGA_OMC="$OMEGA_OMC" OMEGA_BRIDGE="$OMEGA_BRIDGE" sh om/units.sh
taskset -c 10-14 sh compat/cuda/build.sh /tmp/standalone-v2-final

# Final documented focused targets (focused.log):
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  env OMEGA_OMC="$OMEGA_OMC" OMEGA_BRIDGE="$OMEGA_BRIDGE" \
  make -s -j3 test-standalone-oracles test-standalone-oracles-family \
  test-standalone-oracles-mutants test-attention-mirror-dispatch test-prefill-scoped

# Original source, default Omega and C snapshot (family.log):
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  python3 om/standalone-oracles/family-check.py build/libomega_gpu.a \
  --frame build/q38-head/bounds.frame --base b774723f537cec16a359fe082b8100317a5b5490

# Broader unchanged production prefill host/simulator gates (prefill.log):
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  env OMEGA_OMC="$OMEGA_OMC" OMEGA_BRIDGE="$OMEGA_BRIDGE" \
  make -s test-prefill-shape test-prefill-scoped

# Final additive v2 identity and import proof (c0.log):
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  env OMEGA_OMC="$OMEGA_OMC" sh compat/cuda/test.sh /tmp/standalone-c0-v2-proof
```

| Command / log | rc | Observed final line or result |
|---|---:|---|
| `om/units.sh` | 0 | wrote checked portable Omega units to the lane `src` directory |
| `compat/cuda/build.sh` | 0 | `built ptx 3f104fbfb9f2a98c633b9fab39cf1f67caae009d3785f42238b1c5ae32739971` |
| focused targets / `focused.log` | 0 | `PASS standalone family: eight host checks, identical before/after verdicts, eight red controls` |
| original-source comparison / `family.log` | 0 | same eight-check family PASS line |
| prefill host/sim / `prefill.log` | 0 | `wrong_tile_height: caught` |
| fixture admission / `c0.log` | 0 | `CUDA-C0-PROFILE-TEST PASS` |

`prefill.log` was collected before the final checker-only policy/absolute-error
refinements; production prefill code was unchanged. Final `focused.log` and
`family.log` re-exercise the affected checkers after those refinements, including
scoped isolation and its source mutants. Focused targets also exercise q38 bounds,
1309 envelope cases and envelope mutants, descriptor/layout refusals and budget
admission. The `bad row...` diagnostics in `family.log` are intentional existing
negative controls and match the original source byte-for-byte.

## Before / after and a red control for every checker

`family.log` compiles the actual original files from the base Git revision, the
Omega-default current files and the current default-OFF C snapshot. It asserts
both stdout and stderr are byte-identical. All normal variants return 0.
`focused.log` separately labels its available baseline `C-snapshot`; it does not
claim that snapshot is the original Git source.

| Checker | Identical original/default/C-snapshot observed result | Red rc / cause |
|---|---|---|
| `compat/cuda/fixture/vadd_drv.c` | `VADD_U32 PASS n=4099 grid=17 block=256 checksum=8806203976955` | 1, poisoned CPU transport result |
| `tools/interop/cubin_probe.c` | M=17 K=7 N=19, `worst_abs=0 bad=0 untouched=0` | 5, poisoned CPU transport result |
| `om/attention-mirror/dispatch-check.c` | actual adapter scoped order/reuse/bypass/isolation/geometry/operation failures PASS | -6, corrupted Omega equality result |
| `om/prefill-shape/cache-check.c` | `prefill cache admission: PASS` | 1, corrupted paired equality |
| `om/prefill-shape/scoped-check.c` | `scoped qgemm isolation: PASS` | 1, corrupted Omega equality result |
| `om/q38-head/budget-check.c` | exact-head reserve/null-handle refusal PASS | -6, corrupted Omega equality result |
| `om/q38-head/check.c` | bounds/slack/tails/orders/repetitions PASS; existing arithmetic and infinite-radius mutants caught | 1, corrupted Omega conjunction |
| `om/q38-head/norm-check.c` | `RMSNorm envelope: 90 cases, 0 failures, largest norm 71.5541835982, bound 100: PASS` | 1, poisoned CPU simulator transport result |

Negative return -6 is Python's observed signal return for SIGABRT, not a claimed
process exit code. Core dumps are disabled by the runner. A separate missing-unit
link control fails with an undefined `omega_so_oracles` symbol.

`host-cuda.c` is a verification-only CPU CUDA transport: it never loads a GPU
module. Its independent CPU vector-add/matmul supplies transport results, while
the changed probe's reference calculation uses Omega. `host-norm.c` captures real
emitted norm IR and executes it in WsSim. Its invocation uses the existing
`--chip` CLI to reach that checker path, but all GPU/session/allocation/launch
transports are wrapped and devices hidden. This is simulator evidence, **not chip
evidence**. `HOST-STUB-NOT-A-GPU-WINDOW` is a deliberately synthetic transport
marker, not an authorization token. The vadd fixture was never run on a GPU.

## Partitions and Omega source mutation controls

The scalar C/Omega differential reports **1270080 cases**, including source-derived
v-1/v/v+1 boundaries, 0/UINT32_MAX/UINT64_MAX, tile/dispatch/cache/reading/budget
boundaries, fixture moduli and sparse-tail indices. Float inputs include signed
zero, subnormals, ordinary finite values, infinities and NaNs (NaNs compared by
class). Tolerance neighbors are tested in f32 and f64. Checked Omega narrowing is
preceded by explicit unsigned masks to preserve C wrapping; absolute error clears
the binary64 sign bit, including negative zero.

Every mutant below modifies `.om`, is recompiled through the pinned seeded bridge,
and is killed by the scalar differential. A compiler rejection does not count as
a killed mutant. Final results are recorded in `focused.log`.

| Mutant | Changed policy | Observed killing input |
|---|---|---|
| vadd_multiplier | reference multiplier off by one | op=0 a=1 |
| grid_round_down | omit launch-grid round-up | op=7 a=1 |
| mirror_threshold | move mirror size boundary | op=10 a=2 b=0x400 |
| cache_admit_extra | admit an extra cache case | op=13 a=2 |
| reading_boundary | exclude the reading boundary | op=15 a=0x40 |
| budget_short | reserve one byte less | op=17 a=0 |
| accept_infinity | classify infinity as finite | op=24 a=0x7f800000 |
| rng_multiplier | change reference RNG recurrence | op=22 a=1 |
| norm_sparse_tail | omit last sparse coefficient | op=27 a=0x13ff b=1 |

The relocated dispatch mutant compiler also needed the original checker include
path after the new helper include; this was fixed rather than suppressing those
existing controls. The final focused gate kills all four existing dispatch
mutants and all three existing scoped mutants.

## Frozen v1 identities and additive v2 fixture identity

This is a **fixture identity change**, not a mutation of a pinned v1 profile.
`profile-c0-aarch64-v1.tsv` and `profile-c0-x86_64-v1.tsv` remain byte-identical to
the base revision; `identities.log` records the comparison and their SHA-256:

- aarch64 v1: `fba4fd4f91e93dd9543038998494b2a5263e042f8394e373647b2321a7278fe8`.
- x86_64 v1: `28d405fe1ffe05cf08125a2f110df1948fbba7acfade814c7bb8733036d13a11`.

The build and test entry points select the new v2 profiles. `c0.log` records two
identical rebuilds, successful static admission on both ISAs, and an **empty diff
of CUDA imported symbols between v1 and v2** for each ISA. New non-CUDA trap
runtime imports are explicitly listed, not silently ignored. The x86_64 result
is static-only; no x86_64 device execution is claimed. All 14 existing profile
refusal controls reject.

Final v2 host executable SHA-256:

- aarch64: `c2e71d889a01f0ab63c1252ea2b62182b9e994c2d4c028056309745caebbf0e5`.
- x86_64: `89e471cd1198e9688ad080cdd7f43aa897fe36f426283ec4036f99556c7231cb`.
- PTX identity is unchanged:
  `3f104fbfb9f2a98c633b9fab39cf1f67caae009d3785f42238b1c5ae32739971`.

## Wolfram calculation record

WolframLanguageEvaluator queries used for newly written constants/partitions:

```text
{2^31,2^32-1,2^32,2^30*2,FromDigits["A5A5A5A5",16],2^23,2^24,Ceiling[4099/256],5120*4,5120*8}
-> {2147483648,4294967295,4294967296,2147483648,2779096485,8388608,16777216,17,20480,40960}

{Ceiling[1024/2],24*1024,Ceiling[1024/2]*24*1024}
-> {512,24576,12582912}  (scratch bytes in the last result)

{2^63-1,IntegerString[2^63-1,16]}
-> {9223372036854775807,"7fffffffffffffff"}

Table[With[{p=pair[[1]],bias=pair[[2]],e=Last[MantissaExponent[1/10000,2]]-1},
 With[{u=(e+bias) 2^(p-1)+Round[(1/10000) 2^(p-1-e)]-2^(p-1)},
 IntegerString[{u-1,u,u+1},16]]],{pair,{{24,127},{53,1023}}}]
-> {{"38d1b716","38d1b717","38d1b718"},
    {"3f1a36e2eb1c432c","3f1a36e2eb1c432d","3f1a36e2eb1c432e"}}
```

These are calculated encoding/boundary values, not measured device performance.
Numeric checker output above is quoted from the measured host logs.

## Chip runbook — NOT_RUN

No chip command is authorized by this receipt. The parent/integrator owns any
future hardware qualification. Before a future run, obtain the current explicit
exclusive-window authorization, inspect current gate/claim/service ownership,
verify the source/compiler pins and follow the applicable CUDApp hardware runbook.
Do not infer availability from an unlocked file or this historical host receipt.
Do not touch live services, driver bindings, VFIO, modules or GPU locks here.

Within a separately authorized hardware task, the existing q38 runbook/runner
provides physical bounds and norm entry paths; a real norm invocation requires
real operator-selected weights/envelope inputs, and a real interop probe requires
a qualified cubin and function/dimensions. Record exact approved inputs, observed
outputs, device/session identities and negative controls in a new hardware receipt.
Keep the pure host dispatch/cache/scoped/budget/scalar checks device-hidden.
Do **not** execute the libcuda-compat vadd fixture on a GPU as part of this lane.
No host transport result may be relabeled QUALIFIED. Until that separate task,
chip/live-RM/VFIO/hardware remain NOT_RUN.

## Integration notes

The lane changes no compiler syntax, compiler pin, encoder, bridge contract,
public API, repository ownership or qualification gate. The new private unit and
focused commands are documented in `om/standalone-oracles/README.md`; active C0
fixture identities are documented in `compat/cuda/README.md`. ROADMAP receives
one dated note and no status-row upgrade. No omega-harness skill contract changes.
Full-repository integration/CI remains the parent integrator's responsibility.
