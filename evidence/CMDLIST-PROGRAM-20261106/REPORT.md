# Seeded whole-program command-list planning — 2026-11-06

## Revision and disposition

- Branch: `feat/cmdlist-program-om`.
- Code revision exercised: `4136a7d8f68f51480f24729a310601e4d593b311` (implementation `e94651855b92d7ac7a21a456e0aa229878bf6800`, followed by device-isolation documentation and rejecting stale/invalid lists before allocating planner storage).
- Stacked on Streams `daee8b6fc054df8fe40830a04127f10e21ba9184`, branch `feat/c4-streams-om`; its method words, pair markers and queue selection are reused, not reimplemented.
- Draft PR: https://github.com/aien-dev/CUDApp/pull/304, base `feat/c4-streams-om`.
- **IMPLEMENTED / EXERCISED: host PASS.** **QUALIFIED on chip: NOT_RUN. MERGED / DEPLOYED: NOT_RUN.** No GPU window was authorized.
- ROADMAP §3's “One submission per generated word” row remains **PARTIAL**, with a dated note. This is a planner and bounded submission implementation, not a claim that every model producer has already supplied complete access metadata.

## Implementation and interface

`om/cmdlist/record.om`, `layout.om`, `plan.om` and the composed `access.om` unit own record encoding, list/storage sizing, capacity and ring bounds, conservative legacy chunk bounds, access validation, dependency classification and whole-program submission planning. Their generated C and provenance are committed via `om/units.sh`. Registration and `mk/om-cmdlist.mk` with `CARRIED_HOST_TESTS` are present in the first implementation commit. The seeded implementation is the default; the previous C record/layout policy is retained behind `CUDAPP_MIGRATION_PENDING_CMDLIST`, default OFF. C remains the allocation, copying and GPU-effect shell.

The additive runtime API is **0.14.0**. Header, manifest and API guide change together:

- `OmegaGpuCmdListAccess`: complete VA/byte intervals with READ/WRITE flags, at most 16 per launch.
- `OmegaGpuCmdListProgramPlan`: exact launch/submission/wait/word counts.
- `omega_gpu_cmdlist_add_accesses`, `omega_gpu_cmdlist_set_accesses`, `omega_gpu_cmdlist_plan_program`, `omega_gpu_cmdlist_submit_program`.

The setter supports existing `_record` helpers, changes no bank/QMD bytes and preserves the previous descriptor if a replacement is rejected. Legacy recordings without annotations are explicitly unknown and retain conservative drains; an explicitly empty access set is different. Invalid or overflowing intervals are refused. The planner considers every outstanding dispatch since the last drain, not just adjacent launches, and classifies RAW, WAR and WAW hazards. It emits zero submissions for an empty program and exactly one for a nonempty program that fits. A capacity or pushbuffer-budget failure is refused, never silently split. Admission and optional final completion waits remain. Manual barrier/skip/cross-wait options that would invalidate this contract are rejected by the whole-program API.

Alias normalization is part of that completeness contract: multiple virtual mappings of the same physical storage must use common canonical intervals, or remain unknown. Distinct raw VAs alone do not prove physical independence; the planner has no hidden physical-mapping oracle. Header/API/README comments explicitly document this precondition. These comment-only clarifications were added after the final host gate; executable policy and API declarations are unchanged.

The launch-cost producer has a `--program` path with complete input/table/weight/scale/output ranges. Actual model-graph producer annotation belongs to the coordinated ModelGraphsOm lane; no fabricated full-token access trace is used here. Existing legacy submit behavior remains available and uses seeded layout policy.

## Why independent waits can be dropped

Method definitions are pinned to [NVIDIA/open-gpu-doc@9fdf5c4062007929d9f4e6cbad9c9771fe61b880, classes/compute/clcec0.h](https://github.com/NVIDIA/open-gpu-doc/blob/9fdf5c4062007929d9f4e6cbad9c9771fe61b880/classes/compute/clcec0.h):

- lines 49–50: `WAIT_FOR_IDLE` is a separate method, not the launch schedule operation;
- lines 262–277: explicit shader cache invalidation/flush fields;
- lines 363–364 and 378–383: each launch still sends its own QMD address and the `INVALIDATE_COPY_SCHEDULE` PCAS action;
- lines 666–680: NO_WFI cache invalidation is distinct; it is not substituted for a required dependency drain.

Each launch retains its immutable, distinct bank/QMD record and all scheduling methods. The initial WFI/cache invalidation admits the program. Only complete nonconflicting intervals justify omitting an intervening WFI/cache-invalidation pair; read/read overlap is harmless, while any RAW/WAR/WAW or unknown effect retains the full pair. Completion release/flush operations are unchanged. The host differential checks emitted method bytes against Streams' C oracle. These definitions and interval proofs are **not** a chip-ordering qualification. Full details and the operator chip runbook are in [om/cmdlist/README.md](https://github.com/aien-dev/CUDApp/blob/4136a7d8f68f51480f24729a310601e4d593b311/om/cmdlist/README.md).

## Final device-isolated acceptance

All final host tests, mutants, simulator runs and API controls execute under Main's device-free namespace rule. The wrapper exposes a minimal `/dev`, not NVIDIA, CUSE or VFIO devices. CPU affinity and make parallelism are retained.

From the lane root:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge \
  make -j3 test-om-cmdlist test-om-cmdlist-mutants test-runtime-api build/gpu_launch_cost_test
```

**PASS, rc 0**, observed final lines in `isolated-focused-api.log`:

```text
PASS cmdlist independent differential: 12000 randomized programs, edges, 1000 launch records
PASS sim data: seven dispatches, independent overlap and nonadjacent RAW/WAR/WAW, two warp orders
PASS real command-list shell: 2000 record/plan/emit programs, PCAS/INLINE/stamps, exact C method differential; no device opened
PASS five compiled Omega mutants rejected by behavior
RESULT: PASS (runtime API 0.14.0: 309 functions, 49 structs, 464 fields, 235 constants match the headers; surface 741 names, 197 INTERNAL)
RESULT: PASS (every negative control was caught)
```

The chip executable is **built only**, not executed. The independent C planner covers empty, one-launch, full-capacity and capacity+1 programs, exact ring bounds, unknown effects, aliases, each hazard class, nonadjacent dependencies and invalid/wrapping ranges. The record oracle compares 1,000 encodings with the existing C bank/QMD builders, including extreme register-count inputs. The public shell differential includes both recording APIs, valid and refused access replacement, PCAS/INLINE modes and stamps.

The simulator uses the existing warp simulator to execute seven real copy/add kernels, with adversarial reverse dispatch order inside each barrier-delimited queue epoch and both warp orders. Every buffer is compared with serialized execution. It is a host queue/warp data check, not GPU execution.

Five separately compiled `.om` mutants are behavior-red; each test process returns **rc 1**, while the mutant harness returns **rc 0**:

| Mutation | Observed rejection |
|---|---|
| Drop required barrier | `FAIL sim data: queue epochs differ from serialized execution (warp order 0)` |
| Off-by-one capacity | Independent planner/edge differential FAIL |
| Wrong RAW class | Independent planner/edge differential FAIL |
| Adjacent-only hazard search | Independent planner/edge differential FAIL |
| Undercount pushbuffer words | Independent planner/edge differential FAIL |

Their final isolated run output is in `isolated-mutants/*.log`; build failure does not count as a caught mutant.

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge \
  sh om/units.sh --check
```

**PASS, rc 0**: `PASS portable Omega units regenerate byte for byte` (`isolated-units-check.log`). Compiler SHA256 is `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f` (seeded omega-mv@e5b32567); committed `.ident` files record converter/frontend/source/generated-C hashes.

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 crumb compile .
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 crumb verify .
```

**PASS, rc 0**: `crumb compile: 40 crumbs, 0 rewritten, HEAD 4136a7d`; `crumb verify: OK: 40 crumbs current`.

## Count model — no latency inference

Observed integer outputs from the existing decode split/glue shapes and historical unknown-token count model:

| Program | Launches | Old per-kernel submissions | Old list submissions | New submissions | Old list waits | New waits |
|---|---:|---:|---:|---:|---:|---:|
| FP8 GEMV split 1 | 1 | 1 | 1 | 1 | 2 | 2 |
| FP8 GEMV split 2 | 2 | 2 | 1 | 1 | 3 | 2 |
| FP8 GEMV split 4 | 4 | 4 | 1 | 1 | 5 | 2 |
| FP8 GEMV split 8 | 8 | 8 | 1 | 1 | 9 | 2 |
| FP8 GEMV split 16 | 16 | 16 | 1 | 1 | 17 | 2 |
| Decode glue | 5 | 5 | 1 | 1 | 5 | 3 |
| Qwen3-30B legacy unknown model | 289 | 289 | 1 | 1 | 289 | 289 |

The existing command list already submits once when it fits; this work does not pretend otherwise. The model reports no wait reduction for the unknown full-token program. The split benchmark includes its completion stamp wait. These are source-derived host model counts, not a captured fully annotated model trace and not chip timings. Full CSV, including emitted word counts, is in `isolated-focused-api.log`.

## Chip runbook and exclusions

**Chip latency, data parity, cache ordering, timestamp interpretation and drift: NOT_RUN.** **Full `make test`: NOT_RUN**, reserved for Integrator's final full gate. No NVIDIA device, shared GPU lock, modules or service on :18091 was touched by this lane.

The README contains the build and exact baseline/program/baseline sequence using `build/gpu_launch_cost_test`, fixed `n=16384`, `k=2048`, splits `1,2,4,8,16`, 20 repetitions, poisoned outputs, bitwise serialized reference checks, separate GPU stamp span/host elapsed, and drift controls, following the immutable `OWN30B-LAUNCH-COST-20261014T0120Z` receipt. It requires a current exclusive window, the operator's existing hardware gate, and explicit permission to expose devices rather than implicitly bypassing the host namespace rule. The candidate's intermediate stamps are labeled front-end stamps, not kernel completions. Nothing was run on chip.

## Earlier attempts and receipt handling

- `focused.log` and `mutants/*.log` preserve the pre-isolation focused run, which passed before Main introduced the namespace requirement. They are superseded by the final isolated run, not silently relabeled.
- `earlier-combined-timeout.log` preserves a focused/API invocation stopped by its 600-second deadline: **INCONCLUSIVE aggregate**, despite the focused tests and API positive control having passed. It was not the full `make test`.
- `pre-isolation-cancelled.log` preserves the subsequent unit/API invocation explicitly cancelled when the device-free rule arrived: **INCONCLUSIVE aggregate**; its unit byte-check and API positive control had passed. The final isolated invocation completed all controls.
- Receipt text only substitutes `~` for `/home/drakestapleton`; no measurements or failure lines were removed. Historical OWN30B receipts were not changed.
- `crumb close CmdList . src/cmdlist modify ...` returned `CLOSED ... [no lock was held]`; there is no remaining CmdList claim or child process. The pre-existing coordination-store `STORE_UNREGISTERED (dry run)` warning was not bypassed or reconfigured.
- Repository-local API, migration guide and ROADMAP documentation are updated. No compiler, language, bridge, pin, ownership or hardware-gate change requires a separate omega-harness skill update.
