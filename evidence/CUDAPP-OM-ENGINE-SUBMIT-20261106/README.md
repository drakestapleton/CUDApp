# Engine, submit, QMD and wait decisions in seeded Omega: host evidence, 2026-11-06

Branch `feat/engine-submit-om`, base `b774723` (origin/main when the branch was cut).
The compiler is the pinned seeded omc at `e5b32567`
(sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`).
Every test, mutant and differential ran as
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 <target>`.
The logs are in `logs/`, with the home directory replaced by `~`.

## Status per tier

| Tier | Verdict |
|---|---|
| Host: unit regeneration, differentials, red controls, mutants, focused targets | PASS (the tables below) |
| Host: compile and link of the four gate programs | PASS, compile only. Nothing was run |
| Live RM (real `/dev/nvidia*`) | NOT_RUN |
| Chip (GB10 sm_121) | NOT_RUN. See `om/engine/CHIP.md` |
| VFIO | NOT_RUN |

This is IMPLEMENTED and EXERCISED on the host. It is not QUALIFIED, MERGED or DEPLOYED.

## What changed

- `om/engine/eng.om` (unit `eng`, ops 0..27) and `om/qmd/bw_qmd.om` (unit `bw_qmd`) are first-party seeded Omega.
  `sh om/units.sh` regenerates the bindings `src/cudapp_om_eng.c` and `src/cudapp_om_bw_qmd.c`.
- The C files keep only their shells: ioctls, MMIO, doorbell, pushbuffer stores, clocks, and alloc/free/channel calls in their existing order.
- Each moved decision has its own default-OFF switch that restores the C oracle.
  The switch names are `CUDAPP_MIGRATION_PENDING_C_<PART>`, where PART is one of ENGINE, WAIT, MEMRESV, MEMCACHE, SERVING, M16NATIVE, M16WAIT, PLATFORM, SUBMIT, BWENGINE, REQUALIFY, CONCURRENT, VISIBILITY or LIFECYCLE.
  QMD uses `CUDAPP_MIGRATION_PENDING_QMD`, with a diff mode `CUDAPP_MIGRATION_PENDING_QMD_DIFF`.
- Builds with `OMEGA_GPU_NO_POSIX` (freestanding) or `ENG_C_PRIMARY` run the C oracle, because `eng_on()` is 0 there.
- The reference to `omega_eng` is strong. A hosted link that leaves out `src/cudapp_om_eng.c` fails at link time instead of falling back to C.
  The M16 programs and the NVRM gate programs now link the unit.
  `m16/m16_gpu_wait.h` used to say that physics code cannot include Omega code; that comment now describes the new state.

## Defects found and fixed

The differential exposed two defects in the predecessor's `eng.om`. Both are fixed.

1. `wrest` checked the new stall deadline after a progress reset, while the C oracle checks the deadline the previous poll left behind.
   It now tests `if ge now get w 2`. Before the fix, the differential failed at record 3009.
2. In `subm` k2, a value the C code casts to `uint32_t` was not masked.
   It is now `gx = and div c 8 4294967295`. Before the fix, the differential failed at record 17915.

Two more defects came from the work around it:

3. `tests/run_m16_requalification.sh` and `tests/run_nvrm_lifecycle_gates.sh` already failed to link at base `b774723` (undefined `omega_ampere_policy`).
   Their gcc lines now add `src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c` as well as `src/cudapp_om_eng.c`.
   They were compiled and linked only, never run.
4. `src/omega_gpu_engine.c` `apply_act` copied `w[3]` into `last_state`. Every action the unit returns carries state 255 (`phfail`, `phunc`), so the line was dead and has been removed.
   Mutant OM_STATE_IGNORED found it by surviving.
   The surviving mutant OM_COPY_IN_CHAIN showed a test gap: no case failed the first copy-in. Case `FAIL_PREPARE_COPY_IN_PROGRAM` now covers it (test-gpu-engine: 50 cases).

## Differentials and red controls

| Target | rc | Final line |
|---|---|---|
| test-om-eng-diff | 0 | `PASS om eng diff: 44584 records identical (om vs C)`. The om build made 110752 unit calls and reached all 28 ops |
| test-om-eng-red | 0 | `PASS red: flipped unit answer detected` (it first diverges at record 139) |
| test-om-qmd / test-om-qmd-diff | 0 | `om qmd sweep: 68003 checks PASS (partition 732 configs)` |
| test-om-qmd-red | 0 | `PASS red sweep aborted rc=134` |
| test-om-qmd-cov | 0 | Lines executed 96.79% of 218; branches executed 100.00% of 108, taken 78.70%; calls 86.49% of 37. The lines not reached are diff-failure paths |
| `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` |

The engine differential (`tests/om_eng_diff_test.c`) drives the same seeded random cases through two builds and requires identical records: the om build, and the C build with every `CUDAPP_MIGRATION_PENDING_C_*` switch set.
A deterministic pass (`lifee`) covers 17 lifecycle kinds × 3 bases × 6 positions × every edge value. Random draws never reach the exact 274877906944 B churn floor.

## Mutants

Recompiled `.om` mutants (`tests/om_eng_mutants.sh`, `tests/om_qmd_mutants.sh`) change one site in the unit, regenerate the binding with bridge-seeded, and rebuild.
An engine mutant is killed by the 44584-record differential (`diff=1`) or by the Blackwell backend host test (`be=1`).

| Target | rc | Result |
|---|---|---|
| test-om-eng-mutants | 0 | `PASS om eng mutants: 16/16 killed` |
| test-om-qmd-mutants | 0 | `PASS 6 mutants killed` (m_qmd0_w4, m_qmd1_w4, m_qmd1_w14, m_cbank_w0, m_cbank_w223, m_shared_cap) |
| test-gpu-engine-mutants | 0 | oracle run: `49 total, 49 killed`; om shell run: `21 total, 21 killed` |
| test-gpu-engine-backend-mutants | 0 | `109 total, 100 killed, 9 survived as redundant, 0 survived as test gaps` |
| mutants-m16-gpu-wait | 0 | `M16_GPU_WAIT_MUTANTS PASS (3/3 killed)` |
| test-gpu-wait-mutants | 0 | `OMEGA_GPU_WAIT_MUTANTS PASS` |

The 16 engine `.om` mutants are m_seqge_bit, m_wrest_stale_stall, m_mmshape_no_u32, m_cache_page, m_life_floor, m_reqf_sign, plus the ten rows that moved out of `tools/gpu_engine_backend_mutations.sh` together with their decisions: BE_ROUND_PAGES_ZERO_ACCEPTED, BE_ROUND_PAGES_NO_ROUNDING, BE_BUILD_COUNT_OVERFLOW, BE_THREADS_WRONG, BE_GRID_ROUNDS_DOWN, BE_QMD_RELEASE_CHECK, WRAP_MARKER_TIMEOUT, WRAP_SEMAPHORE_TIMEOUT, WRAP_MARKER2_TIMEOUT and WRAP_VISIBILITY_TIMEOUT.
BE_ROUND_PAGES_MINIMUM stays unmutated because it can never apply (the reason is in the backend script).

## Focused host targets

| Target | rc | Final line |
|---|---|---|
| test-gpu-engine | 0 | `SUMMARY passed=50 failed=0` |
| test-gpu-engine-compile | 0 | `SUMMARY failed=0` |
| test-gpu-engine-sim | 0 | `SUMMARY passed=6 failed=0` |
| test-gpu-engine-freestanding | 0 | `SUMMARY passed=7 failed=0` |
| test-gpu-wait | 0 | `OMEGA_GPU_WAIT_HOST PASS` |
| test-m16-gpu-wait | 0 | `M16_GPU_WAIT_HOST PASS` |
| test-gpu-serving-alloc | 0 | final control `reserve-skip (no reservation call): fails as required (5 failing checks)` |
| test-ampere-runtime | 0 | `17 passed, 0 failed` |
| test-gpu-session-platform | 0 | `SUMMARY passed=10 failed=0` |
| test-gpu-platform-aien | 0 | `test_omega_gpu_platform_aien: 10 cases, 0 failed` |

Compile only, run neither: `m16_requalify`, `m16_concurrent`, `lifecycle_gates` and `submission_visibility_gates` each build from `nvrm/nvrm.c m16/m16_native.c <program sources> src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c src/cudapp_om_eng.c` with gcc -O2 -Wall -Wextra -Werror, and each returns rc 0. `m16_concurrent` also takes `m16/m16_gpu_wait.c`. The exact commands are in `logs/gate-compile.log`.

Every row above was run (or rerun) after the last source edit: the `apply_act` dead-line removal, the new test case and the mutant-row fixes.

The full `make test` was not run; the integrator owns it.
