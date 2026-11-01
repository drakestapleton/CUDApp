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

The runner tries stall 1 through 15 (1 through 36 for HMMA; the stall field is four bits, and anything past 15 is `NOP`s). `stall_required` is the smallest S above 1 such that S and every larger S matched on all 11 runs. If stall 1 already matches, `stall_required` is null: that is not a measured latency of 1. `cycle_median_stall1` is the clock delta at programmed stall 1 whether or not the oracle matched. `cycle_median` is the delta at `stall_required` when that exists. `run0_result` is lane 0's result word at each stall of the first run. For HMMA, `reference_result` is `d0` at stall 36; a zero reference leaves `stall_required` null.

The fixed-latency oracle arms the destination to `0x7f800000` and the producer does not read it. The next instruction does. A stale read adds that sentinel. The host self-test checks the fresh and stale totals differ, and that the stall-1 SASS really is that pair with no scoreboard and no yield. HMMA is unchanged: `d` starts at 0, the consumer is `FADD` of `d0`, and stall 19 is HMMA stall 15 plus a `NOP` of stall 4.

## NAK pin

Mesa commit `01bab9302a6bf0a082c7ba8c49d74427cf10b0cf`.

- `src/nouveau/compiler/nak/sm120_instr_latencies.rs` — SM100 tables plus one cycle, and +9 / +7 for HMMA / IMMA.
- `src/nouveau/compiler/latencies/sm100/reg_raw.csv` — rows are readers, columns are writers.
- `src/nouveau/compiler/latencies/sm100/pred_raw.csv` — predicate write to a guard is row `redirected_fp64`, column `dualalu`, 13, plus the SM120 one.

`python3 tools/latmeas/diff_nak.py --verify-mesa` re-downloads those files and checks the cells. It is not part of `make test`.

## What this does not measure

Loads, stores, and other scoreboarded ops. BSSY, BSYNC, BRA, EXIT, and BAR: NAK calls them decoupled, and the diff lists the encoder's fixed stall, but there is no GPR chain for them. QMMA: the encoder waits 30 and NAK's IMMA-to-ALU total is 26; the opcode is a patched placeholder, so this kit does not launch it.

Priority 1 (ALU, FMA, mixed, predicate) matches NAK on paper. That match is circular until this table is filled on the chip. The attention emitter still programs predicate stall 13; the predicate kernel is the measurement for that row too.

## Host-only check

`make test-gpu-latmeas` encodes every chain, checks it with `sm121dis`, checks the oracles, and dry-runs the template on synthetic input that is deleted afterwards. It does not open a GPU and it does not write a result into the tree.
