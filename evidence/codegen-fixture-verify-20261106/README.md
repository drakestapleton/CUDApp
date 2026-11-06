# Codegen fixture verification — 2026-11-06

## Scope and verdicts

IMPLEMENTED / EXERCISED on `feat/codegen-fixture-verify-om`, based on
`feat/checkers-rest-om` at `cd44785cc000` (PR #338). The seven exported
`omega_blackwell_verify_codegen_fixtures*` functions delegate comparison,
refusal/error-index arithmetic, optional-word policy, BF16 relations, BFLY bits,
control-word assembly and selection to `om/codegen/fixture_verify.om`.
C still invokes encoders, copies their four words, and controls fixture order.
The public API is unchanged. The C reference is default-OFF behind
`CUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST`.

| Tier | Verdict |
| --- | --- |
| Host source-partition C/Omega differential | PASS |
| All seven exported verifier invocations, C/Omega | PASS |
| Missing strong unit link | PASS: undefined-reference link failure required |
| Corrupted golden-data red control | PASS: verifier returned failure |
| Recompiled Omega mutants | PASS: 7/7 killed |
| Hosted/freestanding golden fixtures and buffer contract | PASS |
| Hosted/freestanding encoded fixture dump comparison | PASS: byte-identical |
| Freestanding undefined-symbol allowlist and malloc red | PASS |
| Existing replay-limit host gate | PASS |
| Unrelated exhaustive BF16 conversion sweep | NOT_RUN |
| Hardware / live RM / VFIO / chip qualification | NOT_RUN |
| MERGED / DEPLOYED | NO |

## Pins and reproduction

Compiler: omega-mv `e5b32567`, `seed/out/omc` SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Use this base's existing runtime-view bridge pin `/tmp/bridge-73-apiviews`
(toc `d84fd6eb`, frontend `a3a455bd`) and
`OMEGA_SEED=~/work/cudastack/omega-bridge/build/bootstrap/omc`.
The scalar-only freestanding profile produces
`src/cudapp_freest_fixture_verify.c`; it introduces no hosted runtime into the
freestanding build. Generated C is bridge output, not a second implementation.
The first lane commit registers the unit; subsequent changes finish its scalar
ABI and callers. No bridge/compiler pin, external dependency or API is changed.

Every command below ran under the device-free prefix:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 COMMAND
```

| COMMAND (inside that prefix) | rc | Observed final result |
| --- | --- | --- |
| `python3 tests/om_fixture_verify_test.py` | 0 | `PASS seven exported fixtures; red golden-data control; mutants 7/7 KILLED; HW NOT_RUN` |
| `make -j3 build/test_codegen_hosted build/test_codegen_freestanding test-gpu-fs-allowlist-control` | 0 | `allowlist control: a malloc caller is refused by the nm -u check (as required)` |
| `./build/test_codegen_hosted --golden` | 0 | `PASS: hosted golden-word fixtures through the .om encoder (0 failing checks)` |
| `./build/test_codegen_freestanding --no-sweep` | 0 | `PASS: freestanding codegen contract (14 fixtures, 0 failing checks)` |
| `./build/test_codegen_hosted --dump` and `./build/test_codegen_freestanding --dump` | 0 each | Captured outputs compared byte-for-byte equal by the acceptance driver |
| `make -j3 test-om-ir-limit` | 0 | `PASS replay-limit zero/default, boundaries, pre-mutation refusals, pressure` |

The `--no-sweep` argument excludes only the unrelated exhaustive conversion
sweep; all golden verifiers and the complete existing buffer contract ran.
The acceptance runner records exact compiler/generator commands and return
codes in `commands.tsv`. Selected text receipts accompany this file. Encoded
machine-code dumps, compiled binaries and mutant binaries are not committed.
Paths in filed receipts replace the local home directory with `~`.

## Source-derived partitions

`tests/om_fixture_verify_test.c` compares every extracted operation and wrapper
with the retained C reference. For the seven caller families (main, intops,
fragment MMA, reconvergence, BFLY/F2FP, atomf32, fb1cut4), the real exported
functions also run against all their existing golden encodings.

- All four word positions, each mask 0 through 15, equal words and mismatches;
  optional word 3 with zero and nonzero expected word.
- Error indices at zero, instruction/shift/word boundaries and UINT32/UINT64
  extremes; positive and negative status values; zero/nonzero refusal classes.
- BF16 changed-word XOR around the required `0x40000` bit, retained-word
  equality/mismatch; BFLY's required bit and independent second-pair mismatches.
- Word boundaries at 0/1, 7/8, 15/16/17, 31/32/33, 255/256,
  262143/262144/262145, 67108863/67108864/67108865, INT32_MAX,
  UINT32_MAX-1/UINT32_MAX/UINT32_MAX+1, INT64_MAX and UINT64_MAX.
- Source shifts are exercised at 0, 1, 15, 16, 17, 23, 24, 25, 31, 32 and 63;
  the shift-count domain is 0..63, matching defined C shifts. Arbitrary larger
  shift counts are not claimed as a differential contract.
- Equality selection takes both arms. Float fixtures compare encoded integer
  words, not arithmetic float values; no new floating-input computation exists.

`gcov.log` records the C reference header at 97.30% lines, 100.00% branches
executed, 96.88% branches taken. The unexecuted branch is the invalid-opcode
trap, outside the private operation domain. This is coverage evidence, not a
hardware qualification claim.

| Fresh `.om` mutant | Fault | Result |
| --- | --- | --- |
| word_equal | invert selected word equality | KILLED |
| error_index | change error-index arithmetic | KILLED |
| status_equal | invert expected status class | KILLED |
| bf16_bit | change BF16 relation bit | KILLED |
| bfly_bit | change required BFLY bit | KILLED |
| shift_count | change source shift | KILLED |
| optional_mask | require/ignore wrong optional word | KILLED |

Separately, corrupting golden `0x00027308U` to `0x00027309U` in a private caller
copy makes the actual exported verifier fail (rc 1). Omitting the strong unit
makes the synthetic caller fail to link (rc 1). Neither control is accepted as
a successful answer or a fallback.

## Wolfram calculations used in extraction

WolframLanguageEvaluator:
`{2^23,Mod[-2^23,2^32],Mod[-3,2^32]}` ->
`{8388608,4286578688,4294967293}`.

WolframLanguageEvaluator:
`{BitShiftLeft[16^^04000000,32],BitAnd[BitShiftLeft[16^^380,6],16^^00ffff00],BitAnd[BitShiftLeft[16^^358,5],16^^00ffff00]}`
-> `{288230376151711744,57344,27392}`.
These are encoded integer fields, not measured chip results.

## Chip runbook — NOT_RUN

No hardware fixture or GPU path was executed. Before any future hardware run,
the operator must grant the current exclusive chip window and approve the exact
revision and normal family runbook. First repeat this host acceptance under the
no-device wrapper, confirm the generated unit pin and freestanding allowlist,
then run the existing encoding/launch qualification for each affected family
inside that authorized window. Record exact revision, commands, positive golden
results and a deliberately refused/corrupted fixture in a new evidence receipt.
Do not infer permission to stop a service, open a device, alter modules, clear a
lock or run VFIO from this document. Until such a receipt exists, chip/live-RM/
VFIO qualification remains NOT_RUN; no roadmap status row is upgraded.
