# Core register allocation and matmul Omega port receipt

Branch: `feat/regalloc-om`, based on CUDApp `22ebf8fc281faabe8df241b04eaff62e6a91d2f6`.
Status: **IMPLEMENTED and EXERCISED for the core allocator, IR helpers, five
matmul builders, scalar register/layout decisions and vecadd writer. The whole
assigned file set is NOT COMPLETE.** Nothing is merged, deployed, or chip-qualified.

## Source and compiler identities

- Seeded Omega compiler: omega-mv e5b32567; omc SHA-256
  `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Frozen bridge checkout used by the hosted private gate: bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4.
- Native function-mode compiler binary at `lanes/bh-tools/omc-native`: SHA-256
  `734f0a39319fbfa930bcc62cb30dbf06d47d78e126372b74f5baf478e6126b61`.
- Source and wire contract: `om/codegen/REGALLOC.md` and
  `src/omega_gpu_om_ir.h`. Native aggregate registration belongs to Integrator;
  this lane generates a private test aggregate, not a second production library.

The C allocator and matmul builders remain only behind the explicit migration
switches. No op enum values changed. The last-use override wire is extension-only
and applied before one solve; invalid flags, shortened lifetimes, past-end hints,
v159 and v160 are exercised. The simulator repair is isolated as commit
`6a855fc`; its separate receipt is `signed-ge/README.md`.

## Commands and observed results

All commands below ran inside the device-free namespace. Expected legacy-oracle
refusal diagnostics are emitted for invalid generated shapes; they are not
unexpected test errors.

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh tests/regalloc_om/run.sh
```

Exit 0. Final result lines:

```text
PASS exact-liveness flag-zero flag-invalid shortened past-end v159 v160
PASS scalar register and matmul policies cases=10000 including uint32 wrap boundaries
PASS regalloc differential cases=181614 refused=179089 encoded=2525 record-flip=caught
PASS signed GE host-reference cases=98 register and immediate
PASS existing encoder golden fixtures and vecadd SHA-256
PASS matmul warp-sim cases=15 CPU oracle bit-identical
```

The differential includes 10000 deterministic random IR programs (mixed scalar,
pair, quad and uniform registers, capacity exhaustion, register pressure and
partial refusal state), all five generated builder families, and the canonical
fixture corpus reused directly from `test_omega_codegen_freestanding.c`.
It compares the entire program, allocation maps, refusals and successful encoded
bytes. A flipped instruction record is detected.

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 python3 tests/regalloc_om/mutants.py
```

Exit 0; final line `PASS recompiled Omega semantic mutants=10`.
Each mutant compiles with rc0 and is rejected semantically with rc1:

| Mutant | Observed detection |
| --- | --- |
| pair-alignment | program/map/partial-state case 1 |
| reserved-gpr | program/map/partial-state case 1 |
| equal-endpoint-release | program/map/partial-state case 7 |
| sort-order | program/map/partial-state case 1 |
| reserved-ugpr | program/map/partial-state case 9 |
| quad-alignment | program/map/partial-state case 1 |
| shortened-override-check | liveness case 3 rc0, expected10 |
| v159-boundary | liveness case 1 rc10, expected0 |
| override-ignored | program/map/partial-state case 53004 |
| override-shortened | refusal case 53004 |

Per-mutant compile/exercise logs and TSV are in `build/regalloc-om/`.
The separately recompiled unsigned-GE simulator mutant is also killed; see the
isolated signed-GE receipt. Compile failure is never counted as a killed mutant.

## Verdicts

| Tier | Verdict | Scope |
| --- | --- | --- |
| Hosted allocator/builders | PASS | Differential, refusals, exact maps/bytes, scalar policies |
| Semantic mutants | PASS | Ten Omega mutants plus unsigned-GE simulator control |
| Warp simulation | PASS | Fifteen BF16 fragment/block cases against CPU oracle |
| Existing encoder fixtures | PASS | Existing fixture entrypoints and vecadd SHA-256 |
| Freestanding OM2 arrays | NOT_RUN | Integrated BridgeHost extension not adopted here |
| Full integrated make test | NOT_RUN | Integrator owns the one integrated gate |
| Chip/live RM/VFIO | NOT_RUN | No current authorization |
| Whole assigned file-set migration | NOT COMPLETE | Remaining C below |

## Remaining assigned C and exact external prerequisite

The following are not represented as completed Omega migrations:

- Program encoding dispatch/wrappers in `omega_blackwell_codegen.c` still use
  their existing C orchestration around the already-Omega instruction encoder.
- Packing/sentinel helpers in that file remain C.
- `src/omega_blackwell_matmul.c` spec, binding and dispatch policy remains C;
  the CPU arithmetic routines also remain as verification oracles.
- `src/omega_blackwell_realize.c` remains C.
- `src/omega_bw_hazard.c` remains C.
- `src/omega_blackwell_encoder.c` has its vecadd writer ported; existing digest
  orchestration and golden-fixture verification remain C.

No language impossibility has been established for the unimplemented policy,
packing or hazard logic. Native OM2 dynamic typed views are an available route;
there is no justified fixed-size cap or host fallback to add. Hashing should
reuse CoreUtilOm's unchanged public SHA-256 API rather than duplicate it.

The concrete external gate is the frozen bridge freestanding path: canonical
OM2 `regalloc.om` is refused with rc110, as documented in NATIVE-ABI.md:265-267.
The required extension is freestanding OM2 plus sized/dynamic array lowering
with the same admission, bounds and trap semantics and a no-libc link check.
Main assigned this to BridgeHost; it is not silently substituted by hosted code.
Integrator must register all eleven units in its single native aggregate and
qualify that extension before a production/freestanding completion claim.

No current request authorizes chip execution. Runbook:
`tests/regalloc_om/CHIP.md`.

## Numeric constant provenance

WolframLanguageEvaluator query:
`FromDigits[#,16]& /@ {"000ea800","000f2800","014fde00","000fdc00"}`
returned `{960512,993280,22011392,1039360}` for the existing LDG_A, LDG_B, HMMA
and NOP schedule constants. The vecadd word constants were likewise converted
from the existing first-party fixture using Wolfram, and its existing golden
SHA-256 test passes. These are encoder constants, not measured performance.
