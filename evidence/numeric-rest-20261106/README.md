# Numeric remainder: seeded Omega default

Date: 2026-11-06. Scope: census lane 1 NUMERIC_REST. This is stacked on
CUDApp #288, `feat/numeric-om`, base
`1b78bfe99aba346e68274e4922f60791456295ab`. The scalar wrappers from that
base remain in place. No public signature changed.

IMPLEMENTED: the remainder defaults to seeded Omega. EXERCISED: host and
warp simulator, as recorded below. QUALIFIED on GB10: NOT_RUN. MERGED:
NOT_RUN. DEPLOYED: NOT_RUN. No GPU window or live-service change was used.

## Source and dependency pins

`om/units.sh` validates the composed Omega source before converting it and
records its source/generated-C hashes in `src/cudapp_om_numeric*.ident`.
Registration was committed first, in `ceb00a1b6617`.

- Seeded compiler: omega-mv `e5b32567`, SHA-256
  `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Existing bridge converter SHA-256:
  `69d629df170a24ba0c2cb6d41dbfaed520e248e0d723236dd32abf0581524d84`.
- Existing seeded frontend SHA-256:
  `23ff7a2356bb5d980060a4685a27ac46587013069b7d24b37c2873d7ee659b37`.

These are the base branch's pins, not a second toolchain. The pinned compiler
and bridge remain verification/build dependencies; the production units are
checked generated C from first-party `.om`. No host math implementation was
substituted for the new Omega source. Existing scalar math units are reused.

## Cutover and retained shells

| Omega source | Migrated behavior |
| --- | --- |
| `om/numeric/rest.om` | IEEE predicates, predicate masks, element/reference/CPU dispatch, FTZ, launch/bulk/reduction admission, source-lane mapping, identities/padding/MEAN finish |
| `om/numeric/patch.om` | Patch lengths and all instruction words |
| `om/numeric/check.om` | Structural producer/register/barrier/tail/QMD and E1 obligations |
| `om/numeric/trace.om` | Parity and seed admission, finite-normal domains, comparison and trace accumulation |
| `om/numeric/reduce.om` | Frozen tree tile reduction and deliberately distinct sequential left fold |

The five remainder units are strong dependencies. C adapters marshal ABI
scalars/views and float bits, allocate/free hierarchy buffers, transfer
rows/tiles, query the existing FP environment/CPU feature effect sites, and
render diagnostic strings. Patch annotations and the operation registry are
static descriptive data. Diagnostics preserve refusal categories, not their
previous exact prose. No weak-symbol or missing-unit fallback exists.

Old computational bodies compile only with
`CUDAPP_MIGRATION_PENDING_C_NUMERIC_REST` or the explicit differential flag
`CUDAPP_MIGRATION_PENDING_C_NUMERIC_REST_DIFF`. The latter retains independent
C IEEE predicates in oracle translation units; the driver deliberately uses
the default Omega-backed public inlines. `CUDAPP_NUMERIC=c` opts into both
the preserved scalar C oracle and this remainder oracle. No default flag is
changed to C.

AArch64 BF16 CPU-tier admission still consults the existing capability effect
site only when Omega requests it, and preserves the unavailable-feature
refusal and empty-input behavior. Unused third operands are not read; a
non-dereferenceable unused `c` pointer is an explicit negative control.

## Partitions and comparison contract

`tests/numeric_rest_om_diff_test.c` derives cases from the source decisions:

- All 42 numeric operations and all four reduction operations.
- Counts 0/1, 31/32/33, 63/64/65, 1023/1024/1025; launch counts
  65535/65536/65537 and UINT32_MAX. MEAN admission checks its maximum and
  both neighbors without allocating a huge input. SIZE_MAX and row-extent
  overflow refusals are separate cases.
- Both signs of zero, smallest/largest subnormal, smallest normal, finite
  neighbors around one, finite extrema, infinities, quiet/signaling NaNs,
  integer exactness/conversion boundaries and narrowing boundaries. Each
  source pattern is rotated through the operands.
- Missing-buffer combinations, invalid op/name, refused variants, empty
  rows, short row stride, the FP-environment refusal, BF16 feature refusal,
  and uniform versus varying FFMA constants.
- Every patch word and provenance key; every bit of each patch word and
  every bit of all 96 QMD words is toggled independently and checked against
  the old structural oracle.
- All parity/seed trace fields, checked-lane masks, CPU/reference/FTZ outputs,
  row reference and CPU results, identities, every hierarchy level and the
  non-contract sequential fold.

Non-NaN floating results must match by bits. NaNs compare by class. F16/BF16
parity preserves the original class rules. There is no tolerance relaxation
for finite reduction outputs, no reordered accumulation, and no implicit
FMA contraction (`-ffp-contract=off`).

WolframLanguageEvaluator:
`Table[{i,(1+i 2^-23)-1,Abs[(1+i 2^-23)-1]<=2^-20},{i,{7,8,9}}]`
returned `{{7,7/8388608,True},{8,1/1048576,True},{9,9/8388608,False}}`.
The driver checks those seed-bound neighbors against both implementations.

The frozen tree has an explicit red control. WolframLanguageEvaluator:
`f[z_]:=First[Normal[NumericArray[{z},"Real32"]]]; v=Join[{16777216,1,-16777216},ConstantArray[0,29]]; seq=Fold[f[#1+#2]&,0,v]; Do[v=Table[If[i+d<=32,f[v[[i]]+v[[i+d]]],v[[i]]],{i,32}],{d,{16,8,4,2,1}}]; {seq,First[v]}`
returned `{0.,1.}`. The runtime must produce the tree's `0x3f800000`, while
its explicitly named sequential non-contract path produces positive zero.
This demonstrates that a left-fold substitution is not equivalent.

## Reproduction envelope

Every host test, mutant, differential, simulator and coverage run used:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 COMMAND
```

Build parallelism was at most `-j3`. Set:

```
OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc
OMEGA_BRIDGE=~/work/cudastack/omega-bridge
```

The command logs are captured combined stdout/stderr. The lane's home
prefix in the top-level captured logs is normalized to `~`; execution used
absolute expanded paths. Mutation diagnostics retain their temporary paths.

| Command inside envelope | rc | Observed result / log |
| --- | --- | --- |
| `sh om/units.sh` | 0 | checked portable units written; [generation.log](generation.log) |
| `make -j3 test-numeric-rest-om-diff` | 0 | `NUMERIC REST OM DIFF PASS`, `checks=8418438`; [remainder-diff.log](remainder-diff.log) |
| `make -j3 test-numeric-om-diff` | 0 | `NUMERIC OM DIFF PASS 152415164`; [scalar-diff.log](scalar-diff.log) |
| `make -j3 test-ampere-numeric-host test-ampere-f16-host` | 0 | numeric host PASS; F16 boundary/native/negative controls PASS; [numeric-host.log](numeric-host.log) |
| `make -j3 test-gpu-scan` | 0 | C3 scan simulator PASS, 139 checks, both scheduling orders; [scan-sim.log](scan-sim.log) |
| `build/gpu_scan_test --host-only` | 0 | C3 scan host PASS, 133 checks; [scan-host.log](scan-host.log) |
| `make -j3 test-numeric-om-mutants` | 0 | preserved scalar mutants PASS 6/6; [scalar-mutants.log](scalar-mutants.log) |
| `make -j3 test-numeric-rest-om-mutants` | 0 | remainder mutants PASS 6/6; [remainder-mutants.log](remainder-mutants.log) |
| `make -j3 AMPERE_CUBIN_DIR=build/numeric-rest-c-oracle CUDAPP_NUMERIC=c test-ampere-numeric-host` | 0 | independent opt-in C host tier; [c-oracle-host.log](c-oracle-host.log) |
| `sh tools/numeric_rest_om_coverage.sh /tmp/integ/numeric-rest-coverage-final-20261106` | 0 | `-O0 --coverage` oracle entry/branch report; [coverage.log](coverage.log) |

## Negative controls and killed Omega mutants

The disposable mutant runner first requires a green regenerated control.
It then removes each remainder generated unit in turn: rest, patch, check,
trace and reduce. All five must fail to link with the corresponding undefined
strong Omega entry, not silently take a C path. Diagnostics are in `mutants/`.

| Mutant | Changed source decision | Required rejection |
| --- | --- | --- |
| `source_lane` | LDS XOR lane 63 to 62 | reference result bits differ |
| `ftz_dropped` | remove FTZ input/output handling | FTZ result bits differ |
| `parity_compare` | reverse the equality branch | trace differs |
| `minmax_padding` | remove MIN/MAX identity admission | identity differs |
| `patch_exit` | change EXIT opcode word | patch words differ |
| `seed_bound` | replace bound denominator | seed trace differs |

All six are changed `.om` sources, regenerated through the pinned converter,
not hand-edited generated C. Compilation failure is not counted as a kill:
the mutated differential binary must run and report `FAIL`.
Every killed binary exited `1`; the control exited `0`. The exact first
failure and per-unit link diagnostics are preserved in the logs.

## Coverage interpretation

The coverage runner compiles the old C oracle and default adapters together
at `-O0 --coverage`. `coverage.log` records each function's line coverage and
each file's executed/taken branch counts. Coverage is evidence of exercised
entries and branches, not a proof that every possible floating input was
enumerated. Native x86-only branches were not exercised on this AArch64 host;
allocation failure is not injected. The preserved scalar gate supplies its
separate scalar differential coverage. Metadata/effect-only helpers outside
the census remainder are not claimed fully covered.

The measured file summaries are:

| Source | Lines executed | Branches executed | Branches taken at least once |
| --- | --- | --- | --- |
| `omega_numeric.c` | 93.88% | 96.22% | 85.16% |
| `omega_numeric_reduce.c` | 95.00% | 98.41% | 76.98% |
| `omega_numeric_rest.inc` | 100.00% | 98.46% | 81.54% |
| `omega_numeric_reduce_rest.inc` | 100.00% | 100.00% | 86.67% |

All listed remainder oracle entries were exercised. The log separately
reports incomplete lines for native CPU realization, bulk default/error
branches, patch defaults and launch refusal paths; it does not silently
classify them as covered. Native chip execution is not part of this report.

## GB10 runbook: NOT_RUN

Do not execute this runbook without a fresh, explicitly authorized exclusive
GPU window using the current operator procedure. A free-looking lock or a
historical receipt is not authorization. This lane did not acquire or modify
the chip lock, services, GPU modules, VFIO binding or live model.

After the operator authorizes the window and safely handles any live serving
work under that procedure:

1. Record the exact integrated head, compiler/bridge identity and passive
   device state. Re-run the focused device-hidden host/simulator commands
   above on that head.
2. Build `build/gpu_scan_test` with `make -j3 build/gpu_scan_test` in the
   device-hidden build envelope. Inspect its source's unchanged runtime
   limits before the chip run.
3. Under the authorized operator GPU wrapper, run `build/gpu_scan_test` with
   no tier flag. Its existing chip tier checks every n from 1 through 2048,
   power-of-two neighbors through the default maximum, random lengths and
   the existing three scan mutants. Do not substitute `--sim` or reduce the
   maximum and call that full chip qualification.
4. Save command, rc, exact final line, chip identity, raw comparisons and
   negative-control results in a new immutable receipt. Stop on any failed
   control or mismatch. Restore/release the window only through the current
   operator procedure.

Until that procedure is actually executed, chip/live-RM/VFIO/HW results remain
NOT_RUN. Full `make test` belongs to the integrator and was not run by this
lane. No merge or deployment is implied by these host results.
