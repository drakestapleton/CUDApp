# Two-device qualification plan and gate (DRA-203)

Card cuda202 (kanban t_c2fbba86). **Status: PROPOSED plan; the gate checker is IMPLEMENTED and host-tested;
every two-device execution is NOT_RUN.** The Spark has one GB10, so no second device exists to run on.
DRA-203 stays deferred until DRA-201 is accepted and a second physical accelerator is available without
purchase, or Drake approves the spend (re-entry condition on the issue). Nothing here claims multi-device,
cluster or datacenter capability; a two-device result, when it exists, will not claim one either.

## 1. Inventory of existing distributed work (reuse before building)

Checked 2026-11-07 with `gh search code --owner aien-dev` (allreduce, all-reduce, nccl, multi-gpu,
collective) and the capability register (aien-architecture-mv `docs/capability-register/register.tsv`).

| Item | Owner | What it is | Use here |
|---|---|---|---|
| register 35.1 NCCL, 34.1 distributed math, 37.1 GPUDirect | CUDApp | NONE_FOUND: no collectives, multi-GPU BLAS or peer access anywhere in aien-dev | confirms nothing to reuse for transport or reduction |
| register 35.2, aien-train `native/distributed7/admission.om` (#14, merged) | aien-train | collective metadata admission only: canonical 32-byte machine ids, membership generation, tensor descriptor agreement; no transport, no sum | reused as the step-0 admission before any transfer (§5) |
| aien-train `docs/omegallm/CONTRACT-V1.md` §5, §5.1 | aien-train | the step gradient is a fixed left-to-right fold over G blocks; K worker processes on one host give a byte-identical checkpoint for every K | the numerical contract of the two-device run (§3) |
| omega-mv Fabric (`src/fabric`, `src/runtime/aien_machine_id.h`) | omega-mv | machine identities and group limits (max 16 members) | identity format for `dev0`/`dev1` |
| DRA-15 sparse collective pilot (aien-train #27) | aien-train | a toy 2x3 shared-matrix difference scheme, source-only | **not** a network collective; not used |

## 2. Topology (homogeneous, one declared pair)

| Name | Devices | Link | Notes |
|---|---|---|---|
| **T1** (primary) | two NVIDIA DGX Spark, one GB10 each | one QSFP56 200 Gb/s passive DAC between the ConnectX-7 ports, direct, no switch | NVIDIA documents up to 200 Gb/s per port (DGX Spark User Guide, "ConnectX-7 Networking") |
| **T1b** (fallback) | the same two machines | the RTL8127 10GBASE-T ports, direct cable | slower; same protocol, same gate |

MEASURED on this Spark (spark-b87b, 2026-11-07, read-only): `lspci` lists the RTL8127 Ethernet
controller (`enP7s7`, 10000 Mb/s, link up) and no ConnectX-7 endpoint; the two root ports where it would
sit (`0000:00:00.0`, `0002:00:00.0`) report a 2.5 GT/s x4 link with no device behind them. So T1 also needs
the NIC to enumerate on this machine (firmware/BIOS setting or hardware; not investigated further, no root
action taken). `MemTotal` is 127598532 kB = 121.69 GiB (unified CPU/GPU memory).

Heterogeneous pairs (GB10 + Apple M2, or a cloud GPU) are a separate later extension.

## 3. Model and task

The own Omega LLM training step (aien-train, `CONTRACT-V3.md` maximum shape V 4096, T 256, D 384, H 16,
F 1536, L 16, B 64, G = B = 64; P = 29,995,392 parameters), data parallel over two ranks.

- Rank r (0 or 1) computes blocks j = 32r .. 32r + 31 (block j = sequence j, R = 1), each into its own
  buffer, exactly as a v1 worker (`CONTRACT-V1.md` §5.1).
- Single-device reference: the same build, same data and seed, one device, K = 1, the same number of
  steps. Tolerance: **zero** — the two-device checkpoint must be byte-identical to the reference, because
  the reduction below reproduces the reference's fold order exactly (§4). The f32 and BF16 GPU paths of
  aien-train keep their own contracts; the plan uses whichever gradient element width w (4 or 8 bytes) the
  trainer build stores, and the receipt records it.

## 4. Reduction: ordered, not tree or ring

The reference gradient is GRD = block 0, then GRD := GRD + block j for j = 1..63 in ascending j. IEEE
addition is not associative, so a ring or tree all-reduce (or adding two half sums) changes bits.
Protocol per step:

1. Both ranks compute their 32 blocks in parallel. Rank 0 folds its blocks as they finish into S0; rank 1
   keeps its 32 block buffers.
2. Rank 0 sends S0 (w·P bytes) and its 32 per-sequence losses to rank 1.
3. Rank 1 folds its blocks onto S0 in ascending j, giving GRD; it sends GRD (w·P bytes) and the combined
   loss to rank 0.
4. Both ranks apply clip and Adam to GRD identically; their checkpoints must be byte-identical to each other
   and to the reference.

PROVED in Lean 4 (`docs/multidevice/OrderedReduce.lean`, `lean_run_code`, Lean v4.35.0, core only), for an arbitrary binary operation f (no associativity or commutativity assumed):
`ordered2_eq_single` — rank 1 continuing rank 0's fold equals the single-device fold over all blocks
(axioms: propext, Quot.sound). Negative controls on a non-associative f(a,b) = 2a + b, both decided by
the kernel with no axioms: the naive half-sum combination differs (`naive2_differs`, 26 vs 18) and the
rank-swapped order differs (`swapped2_differs`, 26 vs 44). The proof is about this protocol model; it is
the reason a rank-swap mutant must be detectable, not evidence that any implementation is correct.

Bytes and time per step (Wolfram; MODELED from the protocol, line rate, no protocol overhead):

| Quantity | w = 4 (f32) | w = 8 (f64) |
|---|---|---|
| bytes moved per step, both directions, 2·w·P | 239,963,136 B | 479,926,272 B |
| line-rate transfer time at 10 Gb/s (T1b) | 0.192 s | 0.384 s |
| line-rate transfer time at 200 Gb/s (T1) | 0.00960 s | 0.0192 s |
| for comparison, shipping every rank-1 block, 32·w·P | 3,839,410,176 B (3.07 s at 10 Gb/s) | 7,678,820,352 B |
| rank-1 extra memory for its 32 blocks, 32·w·P | 3.58 GiB (2.9 % of 121.69 GiB) | 7.15 GiB (5.9 %) |

Steps 2–3 are serial (rank 1 cannot fold before S0 arrives), so with compute split evenly the scaling
efficiency is at most E = T1 / (2·T2) = T1 / (T1 + 2c), where c is the serial transfer plus fold time per
step (Wolfram `Simplify[t1/(2 (t1/2 + c))]`). Measured T1 decides whether that is useful; this plan
predicts no number for it.

## 5. Placement, synchronization, failure and recovery

- Placement: each rank holds the full weights and Adam state (checkpoint 120 + 24·P = 719,889,528 B) and
  its data shard; rank 1 also holds its 32 block buffers.
- Admission (step 0): both ranks run distributed7 admission with their canonical machine ids, membership
  generation and the gradient tensor descriptor; any disagreement stops before a byte is sent.
- Transport: rungs in order, each gated before the next.
  - R0 rehearsal: two processes on one GB10 over loopback. Exercises the protocol, framing and every
    negative control. It is single-device evidence and can never satisfy the gate (`devices=2` with two
    different device ids is required).
  - R1 (T1b): TCP over the 10 GbE link through the seeded omc socket host words (`socknew`, `bind`,
    `listen`, `accept`, `connect`, `send`, `recv`, `waitio`), host-staged buffers (GB10 memory is
    unified).
  - R2 (T1): the same over ConnectX-7.
  - R3: RDMA. Needs a verbs path; the dependency (vendor `libibverbs`/`mlx5` versus an own driver) is a
    decision for Drake before any work.
- Framing: every message carries step, membership generation, sender rank, tensor id, length and a
  sha256 of the payload. A digest mismatch, wrong generation, wrong rank or short read refuses the step.
  The digest detects corruption; it authenticates nobody.
- Deadline: every receive has a deadline (`waitio`). Timeout or peer close aborts the step on both ranks:
  no checkpoint is written for that step, exit nonzero.
- Checkpoint recovery: both ranks restart from the last checkpoint after verifying they hold the same
  checkpoint sha256; a run killed mid-step and resumed must end byte-identical to the uninterrupted
  reference.

## 6. Measurements the receipt must carry

Topology and link (negotiated speed from `ethtool`), device identities, all runtime dependencies (kernel
driver, firmware, NIC driver, libraries), bytes moved per step counted by the transport at both ends and
cross-checked with the NIC counters (`/sys/class/net/*/statistics`), compute time, communication time,
overlap (communication hidden under compute), single- and two-device step latency (median over the run),
tokens per second for both, and scaling efficiency against the same workload's single-device run.

## 7. The gate: MD2 receipt and `tools/multidevice/mdgate.om`

A receipt is one text file: `MD2 v1`, then `key=value` lines, then `END`. Keys: `fixture` (yes|no),
`claim` (TWO_DEVICE_QUALIFIED|NOT_RUN), `topology` (T1|T1b), `devices`, `dev0`, `dev1` (64-hex canonical
ids), `link`, `link_gbps`, `src` (aien-train commit), `omc` (sha256), `shape`, `steps`, `elem_bytes` (4|8),
`params`, `ref_ck`, `md_ck`, `md_ck1`, `ctl_rank_swap` (checkpoint sha256s), `ctl_drop_rank`,
`ctl_kill_rank`, `ctl_stale_gen`, `ctl_corrupt` (REFUSED|NOT_REFUSED), `bytes_step`, `t1_step_us`,
`t2_step_us`, `comm_us`, `overlap_us`, `eff_ppm`, `tput1_tok_s`, `tput2_tok_s`, `deps`.

`mdgate` (seeded Omega, pinned omc `2c824805…`) accepts `claim=TWO_DEVICE_QUALIFIED` only when every key is
present once and:

| Rule | Refusal reason |
|---|---|
| exactly 2 devices with different ids | `devices-must-be-2`, `dev0-equals-dev1` |
| two-device checkpoint = single-device reference, rank 1 = rank 0 | `two-device-checkpoint-differs-from-single-device-reference`, `rank-1-checkpoint-differs-from-rank-0` |
| the rank-swap mutant's checkpoint differs from the reference | `rank-swap-control-equals-reference` |
| drop, kill, stale-generation and corrupt-payload controls all REFUSED | `a-negative-control-was-not-refused` |
| bytes_step ≥ 2·elem_bytes·params (the protocol minimum; overhead is reported in ppm) | `bytes_step-below-ordered-reduction-minimum` |
| overlap ≤ comm ≤ t2, t2 > 0 | `times-inconsistent-overlap-comm-t2` |
| eff_ppm = ⌊t1·10^6 / (2·t2)⌋ | `eff_ppm-differs-from-t1-over-2-t2` |
| steps, link speed and both throughputs nonzero | `steps-link-or-throughput-zero` |
| any other claim (datacenter, N > 2, ...), unknown or repeated key, malformed value | `unknown-claim`, `unknown-key`, `key-repeated`, `value-malformed-for-key` |

Exit 0 `MDGATE PASS` (fixture=no), 1 FAIL, 3 NOT_RUN (no receipt, or `claim=NOT_RUN`), 5 `MDGATE
PASS-FIXTURE` (fixture=yes: a test fixture can never read as hardware). `tests/repro/run.sh` checks the
committed fixture `tests/repro/md2/good.md2` (5), a fixture=no copy made only in the test's scratch
directory (0), empty input and claim=NOT_RUN (3), and 16 single-rule mutants (1, each with its reason).

Current verdict, no receipt exists: `MDGATE NOT_RUN reason=no-receipt` (exit 3).

## 8. Prerequisites (not available; none is an agent action)

1. A second GB10 machine (purchase or loan) — Drake.
2. A QSFP56 200 Gb/s DAC (T1) or a Cat6A cable (T1b) — Drake, physically.
3. ConnectX-7 enumerating on this Spark (T1 only) — needs a look at firmware/BIOS settings with root.
4. Ownership: the trainer's data-parallel step is aien-train work (lead ownmodel owns the GPU training
   path); the transport, protocol and gate are CUDApp work (multi-device execution, ADR 0042). Both start
   at R0 only after this plan is accepted.

## 9. Next-scale gate

None is pinned. A gate for more than two devices is written only after a two-device receipt passes and
its measured bottleneck (serial transfer, fold, framing, NIC) says what the next scale must test.
