# Resident small-model kernel

IMPLEMENTED in seeded Omega at compiler source `omega-mv@e5b32567`.
Chip: EXERCISED on the GB10 on 2026-11-12, on a shared chip (values only), receipt
`evidence/chip-gates-shared-20261112/`: `gpu_resident_chip --chip` passes both witnesses in both
modes after the `S2R` scheduling fix described there. It is not QUALIFIED: the exclusive window of
the runbook below is still NOT_RUN. The ROADMAP row is PARTIAL.
This branch stacks on the generic replay in CodegenA commit `22ebf8f`.
No replay implementation is forked or changed here.

`resident.om` emits one descriptor interpreter through the canonical eight-u64 IR
records and `omega_gpu_om_ir_replay`. `check.om` exports the Omega admission entry.
`src/omega_gpu_resident.c` widens descriptor words and calls those entries. It has
no operation selection, scheduling, layer arithmetic or barrier decisions.
The internal builder is not an addition to the public runtime API.

## Contract

A descriptor is eight little-endian u32 words:
`op, input_a_offset, input_b_offset, output_offset, rows, cols, 0, 0`.
Offsets count f32 elements in one arena. Operations are GEMV (1, row-major),
ADD (2), MUL (3), SUM (4, one output row), COPY (5). Elementwise operations and
COPY require cols=1. Dimensions are bounded by 32; the table has at most 64
entries and the arena at most 65536 elements. Outputs cannot overlap their own
inputs. Later descriptors may consume earlier outputs. Admission rejects invalid
opcodes, bounds, reserved words, dimensions, overlap and unsupported launch grids.
The table and pointers must not change after admission until completion.

The launch has one or two CTAs, each with 32 threads. Thread zero of each CTA
works; other threads exit. CTA c owns rows c, c+ctas, ... . The device loops over
the descriptor table; no host return occurs between layers. The included model
has GEMV -> ADD -> MUL -> SUM -> COPY, with dependencies at every boundary.

The grid barrier uses an arrival counter and monotonically increasing generation
word, initially both zero. Every CTA fences its data stores, then atomically adds
one to the counter. The last arrival atomically resets the counter, fences, and
atomically exchanges the new generation. Others poll with STRONG.SYS loads.
There is a fence on exit. Arena traffic uses STRONG.SYS loads/stores too.
The initial count/generation must be reset for every model invocation.
All CTAs must fit simultaneously; this bounded grid is not a general oversubscribed
cooperative launch. Hardware occupancy and memory-order behavior still require
the chip tier. Never run a dropped-barrier control on the chip.

The fixed vreg plan uses pairs 0..7 and scalars 8..30. Initialization and terminal
self-reads hold all values across every generated loop and branch arm, without
assuming that the straight-line register allocator understands a CFG.

## Host acceptance

Run with the pinned `OMEGA_OMC` and `OMEGA_BRIDGE` from the repository:

```sh
taskset -c 10-14 nice -n 10 make -j3 test-gpu-resident
```

The plain C evaluator is independent of the generator. f32 order is fixed:
GEMV multiplies, rounds, then accumulates left-to-right with a separate FADD;
SUM accumulates left-to-right. The compiler uses `-ffp-contract=off`.
The exact-f32 witness includes a cancellation that distinguishes an FMA from
separate multiply/add. WolframLanguageEvaluator query:
`{Expand[(1+2^-23)(1-2^-23)-1], N[1+2^-23,25], N[1-2^-23,25]}`
returned `{-1/70368744177664, 1.00000011920928955078125,
0.99999988079071044921875}`. The rounding check
`Round[(1+2^-23)(1-2^-23),2^-24]-1` returned `0` through
WolframLanguageEvaluator: separately rounded f32 multiplication is 1, so adding
-1 is zero; an FMA retains the nonzero product residual.

`bw_warp_sim_k.h` previously completed each CTA before starting the next. It now
has a default-off instruction quantum and the existing integer atomic-exchange
operation. Existing `ws_run` callers keep quantum=0 and retain their scheduling.
`tests/gpu_resident_grid.h` interleaves one-warp CTAs in both CTA orders and
several instruction quanta; it executes the emitted polling loops, not a host
replacement barrier. This SC/coherent memory model cannot qualify GPU caches,
fences, progress or occupancy. It also has no instruction latency: a
variable-latency producer read without its wait passes the simulator. `gpu_resident_test`
therefore runs the static hazard checker (`src/omega_bw_hazard.c`) on the encoded kernels
(1 and 5 layers, 1 and 2 CTAs) before the simulator, and the mutant
`fixed-latency-thread-id` must fail there. The dropped-barrier control uses an intentionally
unfair long quantum so the reduction observes unfinished producer rows.
Corrupted descriptors must be refused before replay. Source mutations must
compile successfully, then fail the oracle. Missing tools are NOT_RUN.

## Authorized chip runbook (exclusive window: NOT_RUN)

Only after the operator opens a current exclusive window, runs the platform's
normal admission/doctor checks, and authorizes this exact candidate:

```sh
taskset -c 10-14 nice -n 10 make -j3 build/gpu_resident_chip
SPARK_WINDOW="$AUTHORIZED_WINDOW_ID" build/gpu_resident_chip --chip
```

The executable refuses missing `--chip` or an empty window label before any
device call. The label is a guard, not authorization. Capture the command, rc,
stdout/stderr, source/compiler hashes, machine and window in a new evidence
directory. A timeout is uncertain completion: preserve the runtime latch and
parked buffers; do not retry, free through another path, reset the GPU or unload
modules. Escalate to the operator.

Both paths use PCAS command lists with WFI ordering and the same model, arena,
CTA geometry and f32 comparison. The resident path records one launch; the
per-layer baseline records five. Require exact output, the expected generation
words, matching semaphore counts and observed launch counts on both ordinary and
cancellation fixtures. Record raw elapsed_ns independently; do not claim a speedup
from launch count alone. The historical receipt
`evidence/OWN30B-LAUNCH-COST-20261014T0120Z/` measured 24–31 us per extra launch.
That is context, not a current resident-kernel measurement.
