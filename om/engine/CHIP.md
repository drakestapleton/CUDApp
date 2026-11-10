# om/engine and om/qmd on the chip

Status: chip tier NOT_RUN. Every result for these units so far is host-only
(fake backends, the host simulator and the C-oracle differential). Host results
do not qualify the GB10.

## What moved

`om/engine/eng.om` (unit `eng`, ABI `omega_eng(result, m[24], 24, w[16], 16)`,
`m[0]` = op 0..27) owns the branches of the engine phase machine, wait step,
job validation, memory reserve, cache rounding, serving bounds, poison match,
cleanup, lifecycle, requalify and visibility predicates, submit timeouts, the
M16 release header, the M16 wait failure class and the platform checks.
`om/qmd/bw_qmd.om` (unit `bw_qmd`) packs the Blackwell QMD and constant bank.

The C shells keep every side effect and its order: ioctls, MMIO reads,
doorbell writes, pushbuffer stores, clocks, allocation, free and channel calls.
They ask the unit what to do next and do it.

Each moved decision has a default-OFF switch that restores the C oracle:
`CUDAPP_MIGRATION_PENDING_C_<PART>` with PART one of ENGINE, WAIT, MEMRESV,
MEMCACHE, SERVING, M16NATIVE, M16WAIT, PLATFORM, SUBMIT, BWENGINE, REQUALIFY,
CONCURRENT, VISIBILITY, LIFECYCLE, plus `CUDAPP_MIGRATION_PENDING_QMD` for the
QMD packer. QMD now uses the same `bw_qmd.om` source in hosted and
`OMEGA_GPU_NO_POSIX` builds. The latter calls the checked caller-stack native
root `omega2_bw_qmd`; it no longer selects the C QMD body automatically.
Other engine parts retain their existing freestanding boundaries.

## Why the host differential is not chip evidence

The differential (`make test-om-eng-diff`, 44584 records) proves the unit
returns the same decision as the C oracle for the same inputs. On the chip the
inputs come from real MMIO reads, real semaphore values and real clocks. The
timing interleavings a GPU produces (late progress, a marker that changes
between two polls, a release written after the visibility barrier) are only
modelled by the fake backends.

The M16 wait header (`m16/m16_gpu_wait.h`) used to say that physics code
cannot include Omega code. The M16 programs now link `src/cudapp_om_eng.c`.
A chip run is the first place that change meets the real driver.

## Chip runs needed to qualify

Each needs an authorized exclusive GB10 window under the current procedure.
None of these was run for this change.

| Gate | Command | Shows |
|---|---|---|
| M16 requalification | `tests/run_m16_requalification.sh` | release header, withheld doorbell and requalify predicates against the real channel |
| NVRM lifecycle | `tests/run_nvrm_lifecycle_gates.sh` | churn floor 274877906944 B, high-water cap 2147483648 B, cleanup predicates |
| Submission visibility | `nvrm/submission_visibility_gates.c` (no run script) | visibility predicates |
| M16 concurrent | `m16/m16_concurrent` (built by the requalification script) | withheld, parity, cross-talk, final-page and root-collision decisions |
| Blackwell vector | `omega_blackwell_execute_vector` through the engine | phase machine, QMD packing and release words on sm_121 |

For each gate, the qualifying evidence is a pass in the default build and the
same pass with the matching `CUDAPP_MIGRATION_PENDING_C_<PART>` switch, plus a
red control (a deliberately wrong unit answer) that the gate rejects.

The four gate programs compile and link on the host with the default build
(compile only, nothing run).
