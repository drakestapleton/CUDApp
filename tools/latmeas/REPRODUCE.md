# Reproduce the GB10 latency table

This measures fixed instruction latencies on an NVIDIA GB10 (DGX Spark, sm_121) and compares them with Mesa NAK. Nouveau does not run on the Spark yet. The kernels are CUDA++ machine code. Nothing in the launch links `libcuda`, `libcudart`, or `libnvidia-ml`.

## Command

From a CUDA++ checkout, on the Spark, with `/dev/nvidia0` and `/dev/nvidia-uvm` present and the open kernel module loaded.

This tree does not admit SM version `0xa04`. The GB10 driver (580.173.02) reports `NV2080_CTRL_GR_INFO_SM_VERSION_10_04`, and `nvrm_open` then fails with `unsupported SM 0xa04 or missing target classes`. Draft PR #251 (or the same admission) has to be merged, or applied locally, before the launch. The fix is not in this change.

```sh
sh tools/latmeas/run_spark.sh latmeas-out
```

The script builds and encodes first. It then runs `make doctor` and stops if the machine is not ready. Only the launch is inside `flock /tmp/aien-gb10.lock`, and that launch is wrapped in `timeout 600`. The binary prints an upper bound before it opens the device: 1408 launches, 71 seconds if each one takes 50 ms, usually shorter. The hard stop is 600 seconds.

Outputs, all under `latmeas-out/`:

- `raw.json` — clock deltas, the stall that matched, spread. No number in this file was typed in by hand.
- `sm121_vs_NAK.md` — the table. A cell stays blank when that chain did not produce a stall.
- `diff.md` — every fixed-latency claim that already differs from NAK, with file and line.
- `kernels/` — the machine code that was launched.
- `head.txt` — `git rev-parse HEAD` of the tree that produced the binaries.

Do not treat a blank cell as zero.

## What a row means

One warp, 32 threads, all of them executing the same chain. Lane 0 stores four words: the two clock reads, the chain result, and an auxiliary word.

The clock is `CS2R` with index `0x50`. NAK's `nak_private.h` names that `NAK_SV_CLOCK_LO`. On SM 50 and later, `from_nir.rs` emits `CS2R` rather than `S2R` (`sm70_encode.rs`: opcode `0x805`, index at bits 72..80). CUDA++'s encoder has `S2R` only, so the test emits `S2R` immediate `0x50` and rewrites the opcode to `0x805`.

The runner tries stall 1 through 15 (1 through 36 for HMMA; the stall field is four bits, and anything past 15 is `NOP`s). `stall_required` is the smallest S above 1 such that S and every larger S matched on all 11 runs (`reported_threshold`, `tests/gpu_latmeas_test.c:1223`, from `pass[][]` at line 1884). If stall 1 already matches, `stall_required` is null: that is not a measured latency of 1. `cycle_median_stall1` is the raw clock delta at programmed stall 1 (`cyc[][][0]`, lines 1965 and 1987), whether or not the oracle matched. It is not stall S minus one. `cycle_median` is the raw delta at `stall_required` (lines 1946 and 1981). Neither column subtracts `overhead_cycle_median` (line 1995). The clock is not the pass criterion. `run0_result` is lane 0's result word at each stall of the first run. For HMMA, `reference_result` is `d0` at stall 36; a zero reference leaves `stall_required` null.

The fixed-latency oracle arms the destination to `0x7f800000` and the producer does not read it. The next instruction does. A stale read adds that sentinel. The host self-test checks the fresh and stale totals differ, and that the stall-1 SASS really is that pair with no scoreboard and no yield. HMMA is unchanged: `d` starts at 0, the consumer is `FADD` of `d0`, and stall 19 is HMMA stall 15 plus a `NOP` of stall 4.

The two `CS2R` reads bracket the loop. Between them are the producer, its consumer, the advance of the live value, the sentinel move, and the loop-back. A programmed stall of 1 is only the producer's field. Dividing the clock delta by the op count counts that whole bracket. On the predicate chain the measured stall sits on the `ISETP` that feeds `@P0 BRA`. A stall shorter than the predicate leaves the previous `P0`, which that kernel sets true, so the poison move runs and the clocks cover one pass out of the loop.

The loop increment stalls 15 before `ISETP`. Stall 1 there is shorter than a fixed-latency add: the compare samples the old counter, the body runs one extra time, and the overhead kernel stores that counter. `sm121dis` of that kernel shows the `IADD`, the `ISETP`, and `@!P0 BRA` back to the `IADD`. The host simulator issues the same IR and keeps a GPR write hidden for 5 cycles and a predicate write hidden for 14 (the NAK floors). It requires the overhead counter to equal the trip count, the stall at the top of the sweep to equal the fresh oracle, and stall 1 to equal the stale oracle. Forcing the increment's stall back to 1 must fail the counter check.

The 2026-11-01 GB10 run of `03cdcc5` merged with PR #251 is `evidence/LATMEAS-GB10-20261101/`. `UNITS.md` there compares the stall field with NAK's `deps.delay`. That run stored lane 0 only, and its HMMA oracle was the stall-36 result.

The kernels after that receipt store eight words from every lane: the two clocks, four result words, and the lane id. HMMA's four result words are `FADD` of `d0` through `d3` against zero. The host value is sixteen products of bf16 1.0, not the stall-36 launch. A stall matches only when all 32 lanes match.

`evidence/LATMEAS-GB10-20261101-allanes/` is the GB10 run of that all-lane kernel (`c94c2c9`, merge of `4f3a4d0` and PR #251). Its `raw.json` still has one lane-0 word per stall and no host oracle, so the file alone cannot show that every lane and every HMMA destination matched. The source of that commit did require the full match before it recorded a stall. A later `raw.json` adds `oracle` (four host words), `reference_result` (the first of those), `lanes_matched_min`, `lane_fail_mask`, and `first_mismatch` (run, lane, word, got, want). `run_spark.sh` writes `RECEIPT.txt` after the launch: HEAD, parent SHAs, kernel, driver, device, gcc, UTC time, and the sha256 of `raw.json`. If proc `Model` is `Unknown` and the PCI id is `10de:2e12`, the model line is `NVIDIA GB10 (sm_121)`, the same name the doctor takes from `src/omega_gpu_chip.c`. The launch bound is the binary's estimated chip window: 50 ms per launch, rounded up. 1408 launches print 71 s. A run of about a second is inside that bound.

## NAK pin

Mesa commit `01bab9302a6bf0a082c7ba8c49d74427cf10b0cf`.

- `src/nouveau/compiler/nak/sm120_instr_latencies.rs` — SM100 tables plus one cycle, and +9 / +7 for HMMA / IMMA.
- `src/nouveau/compiler/latencies/sm100/reg_raw.csv` — rows are readers, columns are writers.
- `src/nouveau/compiler/latencies/sm100/pred_raw.csv` — predicate write to a guard is row `redirected_fp64`, column `dualalu`, 13, plus the SM120 one.

`python3 tools/latmeas/diff_nak.py --verify-mesa` re-downloads those files and checks the cells. It is not part of `make test`.

`--check` also anchors each latency claim to the default implementation source.
The QMMA and HMMA reader rows name their seeded Omega NOP/stall emissions, not
the migration-only C oracle. Omega anchors require an exact statement, its NOP
initialization and emission, and the preceding tensor instruction's stall.
The host self-test rejects a catalog pointed at a neighboring line and source
mutants that change a NOP stall to zero or append a digit to its value. These
controls use temporary source copies; they do not edit the builders.
The C-emitter citations track the integration header additions as well; moving
a source line requires refreshing its catalog line, not relaxing the match.

## What this does not measure

Loads, stores, and other scoreboarded ops. BSSY, BSYNC, BRA, EXIT, and BAR: NAK calls them decoupled, and the diff lists the encoder's fixed stall, but there is no GPR chain for them. QMMA: the encoder waits 30 and NAK's IMMA-to-ALU total is 26; the opcode is a patched placeholder, so this kit does not launch it.

Priority 1 (ALU, FMA, mixed, predicate) matches NAK on paper. That match is circular until this table is filled on the chip. The attention emitter still programs predicate stall 13; the predicate kernel is the measurement for that row too.

## Host-only check

`make test-gpu-latmeas` encodes every chain, checks it with `sm121dis`, checks the oracles, and dry-runs the template on synthetic input that is deleted afterwards. It does not open a GPU and it does not write a result into the tree.
