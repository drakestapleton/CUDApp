# Qwen3.8-Flash-Next FP8: where the bytes are, and what can be removed exactly

Date 2026-11-18. CUDApp commit `ce03c88fb13b7eec798efa9a2953f6e3ed5c72fd`. Machine: x86-64 cloud container, 4 cores,
15 GB RAM, no GPU, gcc 13.3.0, numpy 2.5.3. Nothing here ran on a DGX Spark. Chip status: NOT_RUN.

Source: `Qwen/Qwen3.8-Flash-Next-FP8` on Hugging Face, revision `7fb48d708ca6cd8eeed8f4db0433be18af96984f`.
No weights are committed. The scripts fetch byte ranges and record SHA-256 of what they read.

## Question

The official FP8 checkpoint is 172.76 GiB. A DGX Spark has 128 GB of unified memory. Is there waste in the weights
that can be removed without changing the model's output?

## Measured

1. **Inventory** (`inventory.py`, `inventory.log`): all 131 safetensors headers, 152089 tensors. Headers SHA-256
   `0c1a0ab6e4a93e85f4ad7226e4a60a3f106c2be10214f4be99585ebdac6b1584`.

   | Part | GiB | Type | Text serving |
   |---|---|---|---|
   | Routed experts (48 x 512 x 3 matrices) | 112.500 | E4M3 | resident |
   | N-gram table | 47.684 | E4M3 | 16 rows of 160 bytes per token: read from disk |
   | GDN | 3.886 | BF16 | resident |
   | MTP head | 2.513 | mixed | not needed for plain decoding |
   | Gated residual | 1.193 | BF16 | resident |
   | Token embedding, lm_head | 1.184 each | BF16 | resident |
   | QSA attention and indexer | 1.150 | BF16 | resident |
   | Vision tower | 0.836 | BF16 | not needed for text |
   | Router and shared expert | 0.557 | BF16 | resident |
   | PLE projections, expert scales | 0.075 | BF16 | resident |

2. **Structure of the expert weights** (`fetch_layer0.py`, `analyze.py`, `analyze-layer0.log`): 12 experts of
   layer 0, 58,982,400 weight bytes, sample file SHA-256
   `7579317a9c4d236b08ac0be7bd379040b9cbaa57b113d9c693a8cbd53a2a8991`.
   - Order-0 entropy 6.56 to 6.60 bits per weight byte; 254 of 256 codes used; no zero codes, no NaN codes.
   - zlib -9 and xz reach 6.64 to 6.67 bits: there is no sequence structure beyond the code histogram.
   - No dead neurons: the weakest of 640 intermediate neurons carries 0.69 to 0.88 of the median importance.
   - Full rank: 610 to 617 of 640 singular values are needed for 99% of the energy.
   - Experts are unrelated: cosine between down projections of different experts has mean 0.0001, max 0.0023.

   Layers 11, 24 and 47, 3 experts each (`fetch_layers.py`, sample SHA-256
   `ffb4b7f41736b2c7ec76b6ab334dc85e564240a752da943bc6ae4de1f3832c90`): order-0 entropy 6.503, 6.487, 6.603.

3. **A real lossless coder** (`rowrans.c`, SHA-256
   `777d3612db721f0dae1dcbf00558d038511f34ef80948661132e60201662ba80`; `run.py`; `rowrans.log`). One static 14-bit
   rANS model per matrix, one independent stream per matrix row, so rows can be decoded in parallel. The coded size
   counts the frequency table, the row offsets and the streams.

   ```
   gcc -O2 -Wall -Wextra -Werror -o rowrans rowrans.c
   python3 -I run.py l0.npz lx.npz
   tensors=63 raw=103219200 coded=85069628 bits/weight=6.5933 ratio=0.8242 all_roundtrip_bit_exact=True flipped_bit_detected=63/63
   ```

   All 63 matrices decode to the identical bytes. Negative control: one flipped bit in the coded stream breaks the
   round trip in 63 of 63 matrices.

## Derived (arithmetic on the numbers above, not measured)

- Coded experts: 112.500 x 0.8242 = 92.72 GiB, if the 63-matrix sample ratio holds for all 73,728 matrices.
- Text-serving resident set: 92.72 + 9.23 (all other resident parts, as shipped) = 101.95 GiB, against 121.73 GiB
  uncoded. 128 GB is 119.2 GiB.

## What this does and does not show

- The expert weights have no structural waste in this sample. Pruning neurons, low-rank factoring or sharing between
  experts would each remove information. They are not free.
- The removable waste is coding redundancy: E4M3 spends 8 bits where the weights carry about 6.5. Removing it is
  bit-exact, so the model's output cannot change.
- Not shown: decode speed. The GEMV kernels read FP8 bytes directly today. A coded store needs either a decoded cache
  of hot experts or a decoder inside the kernel. Neither exists yet. Status: PROPOSED.
- Not shown: the true free memory on the Spark, the routing skew of the experts, or any token-per-second number.
- `rowrans.c` is a host measurement tool, not runtime code. The runtime decoder belongs in `.om`.
