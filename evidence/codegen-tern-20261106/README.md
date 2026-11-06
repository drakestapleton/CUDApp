# Ternary seeded Omega migration — 2026-11-06

This receipt belongs to branch `feat/omega-codegen-tern`, based on the shared
replay interface in `feat/omega-codegen-a` at `89ee4db` (draft #275).
The public ternary header is untouched. PR #229 changes only its QMD-cap comment;
it changes no builder-read field, so the exhaustive accepted domain covers both
merge orders.

## Verification inputs

- Omega source: `e5b32567a7aff3ce594e3254a622a846a6cd190b`.
- Seeded omc SHA-256: `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Omega bridge source: `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4`.
- Converter/frontend content pins are checked by `om/units.sh` before regeneration.
- The simulator adapter checks the Omega revision and rejects modified input
  fixture/algebra-oracle files. It invokes only `sim_battery`, not the original
  device-capable command-line entry point.

All build/run commands use `taskset -c 10-14 nice -n 10`, at most `make -j3`.
See `om/codegen/TERN.md` for environment variables and reproducible commands.
No chip authorization, device access, service changes, or deployment are part
of this receipt. Chip tier is **NOT_RUN**.


## Exercised focused gates

| Gate | Command after the stated affinity/nice prefix | rc | Observed final line |
|---|---|---|---|
| Reproducible units | `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` |
| Omega source mutants | `make -j3 test-codegen-tern-mutants` | 0 | `CODEGEN TERN OMEGA MUTANTS PASS (68 source-mutant/config controls)` |
| Current warp simulator | `make -j3 test-codegen-tern-sim` | 0 | `CODEGEN TERN SIM: 34 checks, 0 failed: PASS` |

The simulator uses CUDApp's existing `tests/bw_warp_sim_k.h` and the pinned
Omega ternary/algebra battery. The pinned Omega simulator itself ignores the
scalar LDG immediate; using that older simulator produced the same 14 failing
row-major cases for both C and Omega builders. CUDApp already contains the
correct immediate-aware load semantics. The adapter pre-includes that current
simulator, without editing either simulator or weakening the battery. The
simulator's shipped mutants 1, 3 and 2 were detected.

`mutants-all-shapes.log`, `units-check.log`, and `simulator.log` retain the current
focused output. The earlier `mutants.log` preserves the initial smaller point
set and substitutes environment-variable names for workspace prefixes; other
logs retain commands as executed. The expanded source-mutation gate crosses
every supported (layout, rows) shape with both output paths and epilogues,
then exercises multi-step unrolling, deep streaming, and chunked input.
`simulator-pinned-omega-failed.log` preserves the initial failed run against the
superseded simulator rather than presenting it as a passing gate.

## Exhaustive differential and full host attempt

The full host run exercised `./build/codegen_tern_diff_test` to completion:

```
CODEGEN TERN RECORD NEGATIVE CONTROL CAUGHT
CODEGEN TERN DIFF PASS (213120 programs, 14258 refusals)
```

The enclosing command was:

```
set -o pipefail
taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge \
  OMEGA_MV=/home/drakestapleton/work/cudastack/omega-mv \
  make -j3 test | tee evidence/codegen-tern-20261106/full-host.log
```

It finished with **rc 2**, not a timeout or interruption. Full-host tier is
**FAIL**: `tools/latmeas/catalog.json` still pointed `qmma_reader` at the
pre-migration C source line. The actual failure was:

```
FAIL: qmma_reader: src/omega_blackwell_codegen_qmma.c:42 does not contain 'nop(e, 15)'
make: *** [Makefile:174: test-gpu-latmeas] Error 1
```

`full-host.log` retains stdout, including the completed exhaustive differential;
`full-host-failure.log` retains the combined-output failure excerpt from the
tool transcript. No `CUDA++ HOST TESTS PASS` final line was observed.
The integration owner runs the final integrated full-host gate after the
source-contract repair; this receipt does not turn that unrun gate into PASS.

## Latency source-contract repair

The isolated repair updates all catalog anchors affected by the current lanes:

- QMMA reader: `om/codegen/cg_qmma.om:257`, exact `put x 7 15`.
- HMMA reader: `om/codegen/cg_tc.om:913`, exact `put x 7 ulong 14`.
- Numeric minimum: `src/omega_numeric.c:1258`,
  `CTRL_STALL(p[t].w[3]) < 5u`, rather than the moved comment.
- Elementwise and attention fixed-stall policies remain in their existing C
  emitters at the cataloged lines; their migration lanes did not move them.

The checker still executes `diff_nak.py --check`. Omega statements match
exactly, with the adjacent NOP initialization/emission and preceding tensor
stall also checked. Its self-check runs wrong-line, zero-stall and
digit-appended-stall source controls for both tensor readers. All are rejected;
the wrong-line and changed-source cases use the real checker, not a second
matching implementation. Temporary source mutants are cleaned automatically.

The requested red run on the unfixed lane tree returned **rc 1**, exactly the
stale QMMA assertion shown above (`latmeas-red.log`). The repaired green run
returned **rc 0** and ended `latmeas host self-test PASS`
(`latmeas-green.log`). The final green command used the new mandatory wrapper:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 sh tools/latmeas/selftest.sh \
  /home/drakestapleton/work/cudastack/lanes/CodegenTern/build/gpu_latmeas_test
```

The working directory was a disposable verification worktree, not the active
integration tree. Its source fixture was integration
`071ad44ed5628a065fe0efe298124df3a52d4eb0`, with only the cited default-source
files taken from:

- `om/codegen/cg_tc.om`:
  `57b19749567118e0400800ba9c10191ca627a499` (draft #278).
- `src/omega_numeric.c`:
  `1b78bfe99aba346e68274e4922f60791456295ab` (draft #288).

It used the existing lane host-test binary and the repaired checker/catalog.
This deliberately verifies the integration-only source contract without
merging unrelated lanes into #279. The isolated repair commit must be consumed
with those source revisions; the older #275-only base lacks `cg_tc.om`.

The full-host attempt, ternary gates and initial red run predate the device-free
namespace rule. One initial green also finished before that rule was received;
the green result quoted here is the subsequent device-free run. No old result
is relabeled as having used the wrapper. Future tests use the wrapper above.
This host-only source-contract gate does not qualify or launch hardware.

## Final device-free ternary acceptance

After documenting the namespace requirement, all focused ternary examples were
exercised again, serially inside one device-free namespace:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge \
  OMEGA_MV=/home/drakestapleton/work/cudastack/omega-mv \
  OMEGA_BRIDGE_DIR=/home/drakestapleton/work/cudastack/omega-bridge \
  sh -c 'sh om/units.sh --check && make -j3 test-codegen-tern-diff && python3 om/codegen/tern_mutants.py && make -j3 test-codegen-tern-sim'
```

Observed command **rc 0**. `device-free-focused.log` retains the full output:

```
PASS portable Omega units regenerate byte for byte
CODEGEN TERN RECORD NEGATIVE CONTROL CAUGHT
CODEGEN TERN DIFF PASS (213120 programs, 14258 refusals)
CODEGEN TERN OMEGA MUTANTS PASS (68 source-mutant/config controls)
CODEGEN TERN SIM: 34 checks, 0 failed: PASS
```

These are new device-free runs, not a relabeling of earlier evidence. The full
integrated suite remains the integration owner's gate; no second private full
suite was launched. The disposable latency verification worktree and temporary
generation environment have been removed. No owned acceptance process remains.

## Numeric encoding checks
WolframLanguageEvaluator:
`{BitAnd[2^32-1,BitNot[BitShiftLeft[15,9]]],BitShiftLeft[2^32-1,32]}`
→ `{4294959615,18446744069414584320}` (stall-field clear mask and high-word mask).

WolframLanguageEvaluator: `BitXor[2147516416,1]` → `2147516417`
(the builder-bias source mutation).

WolframLanguageEvaluator:
`{FromDigits["80008000",16],FromDigits["fffffffe",16],FromDigits["00010001",16],FromDigits["00020002",16],FromDigits["358",16],Table[FromDigits[x,16],{x,{"380","388","390","398","3a0","3a8","3ac","3b0","3b4"}}]}`
→ `{2147516416,4294967294,65537,131074,856,{896,904,912,920,928,936,940,944,948}}`.
