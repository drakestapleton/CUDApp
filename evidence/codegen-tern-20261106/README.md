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
| Omega source mutants | `make -j3 test-codegen-tern-mutants` | 0 | `CODEGEN TERN OMEGA MUTANTS PASS (24 source-mutant/config controls)` |
| Current warp simulator | `make -j3 test-codegen-tern-sim` | 0 | `CODEGEN TERN SIM: 34 checks, 0 failed: PASS` |

The simulator uses CUDApp's existing `tests/bw_warp_sim_k.h` and the pinned
Omega ternary/algebra battery. The pinned Omega simulator itself ignores the
scalar LDG immediate; using that older simulator produced the same 14 failing
row-major cases for both C and Omega builders. CUDApp already contains the
correct immediate-aware load semantics. The adapter pre-includes that current
simulator, without editing either simulator or weakening the battery. The
simulator's shipped mutants 1, 3 and 2 were detected.

`mutants.log`, `units-check.log`, and `simulator.log` retain the focused output.
The source-mutation log substitutes environment-variable names for workspace
prefixes; the other logs retain commands as executed.
`simulator-pinned-omega-failed.log` preserves the initial failed run against the
superseded simulator rather than presenting it as a passing gate.

## Numeric encoding checks
WolframLanguageEvaluator:
`{BitAnd[2^32-1,BitNot[BitShiftLeft[15,9]]],BitShiftLeft[2^32-1,32]}`
→ `{4294959615,18446744069414584320}` (stall-field clear mask and high-word mask).

WolframLanguageEvaluator: `BitXor[2147516416,1]` → `2147516417`
(the builder-bias source mutation).

WolframLanguageEvaluator:
`{FromDigits["80008000",16],FromDigits["fffffffe",16],FromDigits["00010001",16],FromDigits["00020002",16],FromDigits["358",16],Table[FromDigits[x,16],{x,{"380","388","390","398","3a0","3a8","3ac","3b0","3b4"}}]}`
→ `{2147516416,4294967294,65537,131074,856,{896,904,912,920,928,936,940,944,948}}`.
