# GB10 (sm_121) measured latencies vs NAK

Chip: NVIDIA GB10 (DGX Spark), sm_121
Driver: NVRM version: NVIDIA UNIX Open Kernel Module for aarch64  580.173.02  Release Build  (dvs-builder@U22-A24-5-4)  Tue Jun 23 08:34:19 UTC 2026. No NVIDIA user-space libraries.
Method: dependent-chain timing kernel. One warp (32 threads); every lane stores, and a stall matches only when every lane matches the oracle.
Each fixed-latency producer is issued at a programmed stall. The GB10 column is the smallest stall S such that S and every larger S in the sweep matched the oracle on every run. Cycles are the CS2R clock delta (NAK_SV_CLOCK_LO, 0x50) around that chain, median and max-min.
Source: tests/gpu_latmeas_test.c in aien-dev/CUDApp
How to rerun: sh tools/latmeas/run_spark.sh OUTPUT_DIR

NAK reference: `nak/sm120_instr_latencies.rs` plus `latencies/sm100/*.csv` at Mesa commit 01bab9302a6bf0a082c7ba8c49d74427cf10b0cf

## Priority 1: values CUDA++ currently borrows from NAK (needs independent measurement)

| Writer -> reader | NAK value | NAK source line | GB10 measured | Delta | Runs / spread | Kernel |
|---|---|---|---|---|---|---|
| ALU -> ALU (GPR RAW) | 4 + 1 = 5 | latencies/sm100/reg_raw.csv:2 row alu column alu = 4; sm120_instr_latencies.rs:457 GPR raw adds max(adjust, 1); IAdd3 is Alu at line 44 | 4 | -1 | 11 runs; stall spread 0; cycles at that stall median 2541 spread 0; stall-1 cycles median 2477 spread 0; loop overhead median 1261 | tests/gpu_latmeas_test.c (alu_alu) |
| FMA -> FMA (GPR RAW) | 4 + 1 = 5 | latencies/sm100/reg_raw.csv:5 row fma column fma = 4; sm120_instr_latencies.rs:59 FFma is Fma; line 457 adds 1 | 4 | -1 | 11 runs; stall spread 0; cycles at that stall median 2541 spread 0; stall-1 cycles median 2477 spread 0; loop overhead median 1261 | tests/gpu_latmeas_test.c (fma_fma) |
| Mixed ALU <-> FMA (GPR RAW) | 5 + 1 = 6 | latencies/sm100/reg_raw.csv:2 row alu column fma = 5 and line 5 row fma column alu = 5; sm120_instr_latencies.rs:457 adds 1 | 5 | -1 | 11 runs; stall spread 0; cycles at that stall median 4362 spread 0; stall-1 cycles median 4170 spread 0; loop overhead median 1261 | tests/gpu_latmeas_test.c (mixed) |
| Predicate write -> guard | 13 + 1 = 14 | latencies/sm100/pred_raw.csv:11 row redirected_fp64 column dualalu = 13; sm120_instr_latencies.rs:474 predicate raw(write, None) uses reader RedirectedFp64 and adds 1; ISetP is Dualalu at line 49 | 13 | -1 | 11 runs; stall spread 0; cycles at that stall median 2714 spread 0; stall-1 cycles median 70 spread 0; loop overhead median 1261 | tests/gpu_latmeas_test.c (pred) |

## Priority 2: instructions where CUDA++ (nvcc-derived) and NAK disagree

| Instruction / pair | NAK value | nvcc sm_121 stall | GB10 measured | Agrees with | Kernel |
|---|---|---|---|---|---|
| Predicate write -> guard, attention emitter | 13 + 1 = 14 | 13 | 13 | nvcc | tests/gpu_latmeas_test.c (pred) |
| HMMA -> non-HMMA reader (FADD of d0) | 19 + 9 = 28 | 15 + NOP 14 = 29 | 20 | neither | tests/gpu_latmeas_test.c (hmma_reader) |
| IMAD.WIDE low half -> IADD3 | 3 + 1 = 4 | 6 (blanket K_FIXED; IMAD.WIDE is not in fixed_pair_class) | 3 | neither | tests/gpu_latmeas_test.c (imad_wide_lo) |
| FMNMX (dualalu) -> IADD3 (alu) | 4 + 1 = 5 | 6 (blanket K_FIXED; FMNMX is not in fixed_pair_class) | 4 | neither | tests/gpu_latmeas_test.c (fmnmx_alu) |
| QMMA (IMMA) -> next reader | 19 + 7 = 26 | 15 + NOP 15 = 30 |  |  | not generated (patched QMMA placeholder; see REPRODUCE.md) |
| Fixed-producer minimum in the numeric checker | 5 + 1 = 6 for mixed ALU/FMA | 5 (any fixed producer; mixed pairs need 6) | 5 | nvcc | tests/gpu_latmeas_test.c (mixed) |
| BSSY stall | decoupled (1 & sb) | 1, no scoreboard |  |  | not generated (NAK class is decoupled, not a fixed GPR RAW) |
| BSYNC stall | decoupled (1 & sb) | 5, no scoreboard |  |  | not generated (NAK class is decoupled, not a fixed GPR RAW) |
| BRA stall | decoupled (1 & sb) | 5 |  |  | not generated (NAK class is decoupled, not a fixed GPR RAW) |
| EXIT stall | decoupled (1 & sb) | 5 |  |  | not generated (NAK class is decoupled, not a fixed GPR RAW) |
| BAR.SYNC stall | decoupled AGU (1 & sb) | 6 |  |  | not generated (NAK class is decoupled, not a fixed GPR RAW) |

## Notes
- Anything not fixed-latency (LDG/LDS, scoreboard waits) is out of scope for this table.
- Decoupled ops (BSSY, BSYNC, BRA, EXIT, BAR) are listed because the encoder gives them a fixed stall and NAK does not. They have no dependent-chain kernel.
- HMMA -> HMMA in NAK is 20 + 9 = 29, the same total the encoder uses. The HMMA row is the non-HMMA reader (28).
- QMMA is not generated here. Its encoder stall is 30 and NAK's IMMA -> ALU total is 26 (IMMA -> IMMA is 27).
- Confidence notes and anomalies go here.
