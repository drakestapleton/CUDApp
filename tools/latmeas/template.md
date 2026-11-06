# GB10 (sm_121) measured latencies vs NAK

Chip: NVIDIA GB10 (DGX Spark), sm_121
Driver: NVIDIA open kernel module <version>, no NVIDIA user-space libraries
Method: dependent-chain timing kernel. Each instruction consumes the previous result, chain length N = <n>, cycles read with <clock source>, median of <k> runs, <warps/occupancy>.
Source: <link to the kernel(s) in aien-dev/CUDApp>
How to rerun: <one-line command>

NAK reference: `nak/sm120_instr_latencies.rs` plus `latencies/sm100/*.csv` at Mesa commit <sha>

## Priority 1: values CUDA++ currently borrows from NAK (needs independent measurement)

| Writer -> reader | NAK value | NAK source line | GB10 measured | Delta | Runs / spread | Kernel |
|---|---|---|---|---|---|---|
| ALU -> ALU (GPR RAW) | 4 + 1 | | | | | |
| FMA -> FMA (GPR RAW) | 4 + 1 | | | | | |
| Mixed ALU <-> FMA (GPR RAW) | 5 + 1 | | | | | |
| Predicate write -> guard | 13 + 1 = 14 | `latencies/sm100/pred_raw.csv` | | | | |

## Priority 2: instructions where CUDA++ (nvcc-derived) and NAK disagree

| Instruction / pair | NAK value | nvcc sm_121 stall | GB10 measured | Agrees with | Kernel |
|---|---|---|---|---|---|
| | | | | NAK / nvcc / neither | |

## Notes
- Anything not fixed-latency (LDG/LDS, scoreboard waits) is out of scope for this table.
- Confidence notes and anomalies go here.

The filled copy is written by `tools/latmeas/fill_template.py` into the output directory. This file stays blank: it is the form, not a result.
