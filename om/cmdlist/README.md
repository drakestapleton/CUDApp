# Seeded whole-program command lists

This component stacks on Streams `daee8b6fc054df8fe40830a04127f10e21ba9184`
(draft #272). It reuses Streams' method encoder, queue selection and pair markers;
it does not introduce another GPU launcher. Omega owns the policy, CUDApp owns
allocation, copying, synchronization, queue submission and observation.

## Default implementation

- `record.om`: bank template, arguments, launch admission and QMD1/QMD0 encoding.
- `layout.om`: allocation sizes, legacy chunk capacity, chunk bounds and inline patches.
- `plan.om`: whole-program validation, aliased byte-range hazards, exact ring budget
  and one-submission plan. `access.om` exports its single-descriptor validator.
- `src/omega_gpu_cmdlist.c`: memory/effect shell; the former C record and sizing
  implementation is an opt-in oracle, `CUDAPP_MIGRATION_PENDING_CMDLIST=1`.
- Generated portable C is checked in and regenerated only through `om/units.sh`.
  Compiler pin: `omega-mv@e5b32567`, omc SHA-256
  `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

A program describes every user-data access as `{va, bytes, READ|WRITE}`. Address
intervals, not buffer identities, determine aliasing. All outstanding launches
since the most recent drain are compared; comparing only adjacent launches is
incorrect. RAW, WAR and WAW require a drain; read/read overlap does not. Empty
access sets mean **known empty**. Legacy records mean **unknown**, conservatively
conflicting with every other record. Descriptors are copied and validated.

The first launch retains an admission WFI and full shader-cache invalidation.
Hazards retain that same pair. Independent launches omit both. Completion release
and its flush remain unchanged. Optional timestamps add a final explicit WFI;
independent-launch timestamps are front-end positions, not completion times.
`ProgramPlan.waits` counts explicit WFI methods, including admission and that
optional final timestamp WFI; it does not count the unchanged completion release.

A program either fits one submission including the platform's published prefix,
or is refused before queue commit. It is never silently split. Empty programs
produce zero submissions. Experimental barrier suppression, skipped launches,
first-barrier suppression and cross-list waits are refused on this path.

## Why the omitted waits are not dispatches or required cache publication

Pinned method source:
[NVIDIA/open-gpu-doc@9fdf5c4062007929d9f4e6cbad9c9771fe61b880,
classes/compute/clcec0.h](https://github.com/NVIDIA/open-gpu-doc/blob/9fdf5c4062007929d9f4e6cbad9c9771fe61b880/classes/compute/clcec0.h).

- `clcec0.h:49-50`: `WAIT_FOR_IDLE` is a separate method. Omitting a redundant
  wait does not omit the launch.
- `clcec0.h:262-277`: shader-cache invalidation has distinct instruction, data,
  constant and flush-data fields. Every CPU-written bank/QMD is unique per launch
  and immutable until completion; the retained leading invalidate admits these
  records. For an independent launch no earlier dispatch writes anything it
  reads or writes, or reads anything it overwrites. There is no inter-dispatch
  data publication requiring another shader-cache invalidation.
- `clcec0.h:363-364,378-383`: every PCAS launch still sends its QMD address and
  `INVALIDATE_COPY_SCHEDULE` action. QMD copy/invalidation and scheduling are not
  dropped along with the idle wait. INLINE retains both complete QMD payloads.
- `clcec0.h:666-680`: NVIDIA also defines a distinct `NO_WFI` invalidate method.
  This planner does **not** replace dependency drains with that weaker operation.

These definitions justify which methods are preserved and the range proof
justifies each omitted drain. They are not a chip ordering qualification. Host
simulation cannot prove GPU cache behavior; chip ordering and latency are NOT_RUN.
The C encoding oracle is retained for differential checking, not as a new trusted
implementation. Numeric conversion used in the QMD port was checked with Wolfram:
`WolframLanguageEvaluator: FromDigits["04b44808",16] -> 78923784`.

## Host acceptance

Every host test, simulator, mutant and differential runs in a device-free
namespace, as required by Main's 2026-11-06 safety rule. It exposes a minimal
`/dev`, without NVIDIA, CUSE or VFIO nodes, plus the assigned CPU affinity:

```sh
export OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc
export OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge
export OMEGA_BRIDGE_DIR=$OMEGA_BRIDGE
export OMEGA_MV=$HOME/work/cudastack/omega-mv
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-om-cmdlist test-runtime-api
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-om-cmdlist-mutants
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/units.sh --check
crumb compile .
crumb verify .
```

The independent C oracle checks 12,000 randomized programs, aliased ranges,
RAW/WAR/WAW/read-read, nonadjacent dependencies, empty/one/full/capacity+1 programs,
exact ring bounds and 1,000 launch records against the pre-existing C bank/QMD
builders. The shell differential additionally exercises public record/plan APIs
and checks complete emitted pushbuffers against Streams' C method oracle.

`cmdlist_sim_test.c` executes real copy-plus-one instructions in the existing
`bw_warp_sim.h` simulator. A queue harness reverses dispatch order inside each
barrier-delimited epoch, then checks every buffer against serialized execution
under both warp orders. This adversarial schedule makes a missing required drain
observable as wrong **data**, not merely a different barrier counter.

Five separately compiled Omega mutants must fail behavior: omitted hazard drain
(simulator data mismatch), capacity off-by-one, wrong RAW classification,
adjacent-only dependency tracking and incorrect pushbuffer sizing. A compile
failure is not a killed mutant. Build/run receipts are retained under
`build/om-cmdlist/mutants/` (or the configured `OUT_DIR`).

## Count model and chip runbook

`build/om-cmdlist/cost` reports integer counts only, for the split FP8 decode
workload from `tests/gpu_launch_cost_test.c` and the five-kernel decode glue sequence
from `tests/gpu_cmdlist_test.c`. The historical 289-launch token is also shown
with **unknown** effects: it deliberately receives no invented wait reduction.
These are source-derived count models, not captured production traces or latency
measurements. Complete graph-producer annotations are required before claiming a
whole-token reduction. ROADMAP remains PARTIAL.

Observed host count output (PCAS; split FP8 uses timestamps, glue/token do not):

| Program | Launches | Old per-kernel submissions | Old list submissions / WFIs | New submissions / WFIs | New words |
| --- | --- | --- | --- | --- | --- |
| FP8 split 1 | 1 | 1 | 1 / 2 | 1 / 2 | 57 |
| FP8 split 2 | 2 | 2 | 1 / 3 | 1 / 2 | 67 |
| FP8 split 4 | 4 | 4 | 1 / 5 | 1 / 2 | 87 |
| FP8 split 8 | 8 | 8 | 1 / 9 | 1 / 2 | 127 |
| FP8 split 16 | 16 | 16 | 1 / 17 | 1 / 2 | 207 |
| Decode glue | 5 | 5 | 1 / 5 | 1 / 3 | 67 |
| Historical token, unknown effects | 289 | 289 | 1 / 289 | 1 / 289 | 2347 |

The old command-list implementation already used one submission for these
programs. The change removes only provably redundant inter-launch WFIs and
refuses oversized whole programs instead of splitting. The per-kernel column
is a different execution strategy, not a claimed reduction over old lists.

Chip qualification requires Drake's current exclusive-window authorization and
the existing operator gate; a free lock is not authorization. No chip command was
run in this lane. Preserve the historical
`evidence/OWN30B-LAUNCH-COST-20261014T0120Z/` receipt; create a new dated receipt.
Keep the OWN30B method: fixed `n=16384`, `k=2048`, splits `1,2,4,8,16`, same weights
and bytes, one selected FP8 kernel, 20 repetitions, poisoned outputs checked
bit-for-bit against serialized one-launch references on every repetition, GPU
stamp span and host elapsed reported separately. Run WFI baseline before and
after the candidate as drift controls. Do not infer candidate latency from host
counts. Required negatives: dropped launch must name the missing semaphore;
missing hazard drain must be data-red in the host simulator before any chip run.

Build the existing chip executable on the host (this does not open a GPU):

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 build/gpu_launch_cost_test
```

Only inside the authorized exclusive window, through the operator's current
hardware gate, and after explicit authorization to expose the device for chip
testing (the host namespace rule is not implicitly waived), run these commands
in this order and preserve stdout plus receipts:

```sh
build/gpu_launch_cost_test --reps 20 --n 16384 --k 2048 --out baseline-before.json
build/gpu_launch_cost_test --program --reps 20 --n 16384 --k 2048 --out program.json
build/gpu_launch_cost_test --reps 20 --n 16384 --k 2048 --out baseline-after.json
```

`--program` attaches the complete table/input/weight/scale/output ranges to each
recorded launch and calls the new one-submission API. Shared input is read-only;
weight, scale and output slices do not alias other launches' writes. Its first
intermediate stamp can precede completion, so only endpoint span and host elapsed
are comparable to the old cost method. Receipt metadata labels these front-end
stamps. The old dropped-launch negative remains a legacy-oracle check. Candidate
chip data parity, cache ordering, drift and timing are all NOT_RUN here.
