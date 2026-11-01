# Reproduce the GB10 latency table

This measures fixed instruction latencies on an NVIDIA GB10 (DGX Spark, sm_121) and compares them with Mesa NAK. Nouveau does not run on the Spark yet. The kernels are CUDA++ machine code. Nothing in the launch links `libcuda`, `libcudart`, or `libnvidia-ml`.

## Command

From a CUDA++ checkout, on the Spark, with `/dev/nvidia0` and `/dev/nvidia-uvm` present and the open kernel module loaded:

```sh
sh tools/latmeas/run_spark.sh latmeas-out
```

The script builds and encodes first. It then runs `make doctor` and stops if the machine is not ready. Only the launch is inside `flock /tmp/aien-gb10.lock`, and that launch is wrapped in `timeout 600`. The binary prints an upper bound before it opens the device: about 1400 launches, 90 seconds if each one takes 50 ms, usually shorter. The hard stop is 600 seconds.

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

NAK's table is the stall the next instruction must wait, not a scoreboard. These chips do not interlock a fixed-latency GPR write (`src/omega_numeric.c`). The runner therefore tries stall 1 through 15 (1 through 36 for HMMA, because the field is only four bits and the rest is `NOP`s). The reported stall is the smallest S such that S and every larger S matched the oracle on all 11 runs. The cycle median is the 32-bit clock delta at that S. An empty loop with the same trip count is the overhead control; its median is in `raw.json`.

The oracle for ALU, FMA, mixed ALU/FMA, the predicate chain, FMNMX, and IMAD.WIDE is a host formula (exact integer or exact f32). HMMA's oracle is the same kernel at stall 36: lane 0's `d0` must be non-zero there, and shorter stalls must match it. A zero reference leaves the HMMA cell blank.

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
