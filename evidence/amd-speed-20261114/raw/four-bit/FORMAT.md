# Choosing the 4-bit format for Qwen3.8-27B on gfx942

Lane uxl-brief/FourBit, 2026-11-13. Result: **Q4D8-1** (CONTAINER.md):
- signed int4 weights, symmetric (no zero point);
- one BF16 scale per group of at most 128 weights (32 for the two GatedDeltaNet input projections);
- the activation handed to the kernel as a 16-bit fixed-point vector split into four signed base-16 digits, so
  the inner product is pure integer `V_DOT8C_I32_I4` work.

Measured quality is in RESULT.md. This file holds the reasons. Every number here is a source quotation or
labelled MODELED.

## 1. What gfx942 can decode cheaply

Source: AMD, *"AMD Instinct MI300" Instruction Set Architecture* (CDNA3).
- Revision dated 5 August 2025, 553 numbered pages.
- Local copy on the Spark: `~/docs/amd-gpu/AMD_Instinct_MI300_CDNA3_ISA.pdf`, sha256 `0cec4237cd93ce7dd76ee8502771429eb2308ab47dfa389f5bcc34f5903a6e2a`.
- Fetched from amd.com on 2026-11-07; the URL is in `~/docs/amd-gpu/SOURCES.md`.
- AMD's licence allows review only, so the ISA is paraphrased and cited here, never copied.

| Instruction | Where | What it gives a 4-bit kernel |
|---|---|---|
| `V_DOT8C_I32_I4` | section 12.7 (VOP2), opcode 58, p. 176 | Eight signed 4-bit × signed 4-bit products, summed into a 32-bit accumulator, in **one** instruction. |
| `V_DOT8_I32_I4` | section 12.10 (VOP3P), opcode 42, p. 262 | The same with a separate addend register. |
| `V_DOT4C_I32_I8` | section 12.7, opcode 57, p. 176 | The 8-bit version, for comparison. |
| `V_CVT_F32_I32` | section 12.8 (VOP1), opcode 5, p. 180 | Integer group sum to float32: one instruction per group, not per weight. |
| `V_FMAC_F32` | section 12.7, opcode 59, p. 177 | One FMA per group to apply the group scale. |
| `V_CVT_F32_FP8` | section 12.8, p. 203 | FP8 (the current kernels' format) has a hardware convert. |
| FP4 / E2M1 | none | A text search of the ISA finds no FP4/E2M1 conversion and no FP4 matrix-core input. The only 4-bit convert is `V_CVT_OFF_F32_I4` (VOP1 opcode 14), an offset table, not E2M1. |

Consequences:
- **MXFP4, NVFP4 and NF4 have no hardware decode on CDNA3.** Every weight would need a bit-field extract plus a
  table lookup or exponent assembly before its FMA.
- **Signed int4 multiplies eight weights per instruction.** That is possible only when the other operand is also
  4-bit integers, which is why the activation is split into base-16 digits.
- Four digits carry a 16-bit signed integer per element, so one 8-weight dword needs four `V_DOT8C_I32_I4`.
  The group sum stays exact: a group has at most 128 weights, `|q| <= 8` and `|xq| <= 2^14`, so `|I| <= 2^24`.

**MODELED, how much ALU time a weight may cost before memory stops being the limit.**
Wolfram: `bw = 5.3*10^12; cus = 304; clk = 2.1*10^9; bw/(cus clk) -> 8.30 bytes/clock/CU`.

Inputs and sources:
- MI300X peak memory bandwidth 5.3 TB/s and 2100 MHz peak engine clock: the
  [AMD data sheet](https://www.amd.com/content/dam/amd/en/documents/instinct-tech-docs/data-sheets/amd-instinct-mi300x-data-sheet.pdf).
- 304 CUs: the data sheet. The notebook card's rocminfo also reports `simd_count 1216`, i.e. 4 SIMDs per CU
  (`card-evidence/uxl-env.log`).

Result: 8.30 bytes/clock/CU is 16.6 four-bit weights per clock per CU. A CU issues 64 lane-operations per clock
(4 SIMDs × 16 lanes, wave64 in 4 clocks), which leaves **3.85 lane-operations per weight**
(Wolfram `64/(2*8.302) -> 3.854`).

Cost of each decode path against that budget:

| Decode path | Lane-operations per weight | Fits the 3.85 budget? |
|---|---|---|
| Integer dot (this format) | 4 `V_DOT8C` per 8 weights = 0.5, plus loads and loop overhead | Yes, about 7× headroom |
| Float decode of int4 (shift/mask, convert, FMA) | about 3 | Barely |
| FP4 decode via lookup | more than that | No |

**MODELED, activation traffic (review finding 12).** Besides 4 weight bytes, each lane reads 16 bytes of digit
planes per step. Per row that is 2·Kp bytes of activation against Kp/2 bytes of weights: 4× the weight bytes
(Wolfram `(2*5120)/(5120/2) -> 4`). In the two-row kernel `q4gemv_d8r2` it is 2×.

The activation buffer is the same for every row, so it is served from the caches, not HBM:
- 10 KB at K=5120 fits the 32 KB vector L1;
- 34 KB at K=17408 does not, and comes from L2.

The existing FP8 kernel has the same 4:1 ratio (a 4-byte float `x` per FP8 weight byte). Cache bandwidth can
therefore limit both kernels before HBM does. The card run reports achieved weight bytes/s per shape so that this
shows up, rather than being assumed away.

[INFERENCE] The ISA has no throughput tables. That `V_DOT8C_I32_I4` issues at the full 32-bit VALU rate is the
card measurement's job (RESULT.md section 5).

## 2. Published quality of 4-bit Qwen3.8-27B

All sources were read 2026-11-13. They are web pages written by third parties: leads, not receipts.

1. **[nicosuter, Qwen3.8-27B-AWQ, results 2026-08-23](https://github.com/nicosuter/Qwen3.8-27B-AWQ/blob/master/results/2026-08-23-published-quants/README.md).**
   - Five W4A16 AWQ checkpoints were scored against the same `Qwen/Qwen3.8-27B-FP8` baseline on five suites
     (GPQA-Diamond, LiveCodeBench v6, BFCL v4, MMMU-Pro, multimodal).
   - Macro recovery was 99.10 % to 99.99 %.
   - Where they differ: the two that left the GatedDeltaNet `in_proj_qkv` / `in_proj_z` in BF16 ranked top. Int8
     on those two projections matched them. Int4 g32 came next (99.81 %). Int4 g128 was last (99.10 %) and the only
     one that separated from the rest.
   - All five keep `lm_head`, `in_proj_a`/`in_proj_b`, vision and MTP in source precision.
   - Symmetric int4 (soyrsoyr) was the best of the five, so a zero point is not needed.
2. **[Umar Salim, "Quantising Qwen3.8-27B: Where Quality Plateaus"](https://umarsalim.com/blog/quantising-qwen3-8-27b-where-quality-plateaus).**
   - llama.cpp GGUF ladder, executed-code benchmarks: Q4_K_M MBPP+ 55.0 against 56.9-59.3 for Q5 to Q8, within
     about 2.5 standard errors.
   - Quotes a [kingy.ai audit](https://kingy.ai/blog/qwen3-8-27b-best-quantization-gguf/) of top-1 agreement with
     BF16: Q4_K_XL 96.0 %, Q5_K_M 97.3 %, Q6_K 97.9 %, Q8_0 98.9 %.
3. **[Quesma, "Benchmarking Qwen3.8 27B quantizations"](https://quesma.com/blog/qwen38-27b-quantizations-benchmarked).**
   Unsloth Q4_K_M matches BF16 on GPQA-Diamond, IFBench and Terminal-Bench 2.1; 2-bit loses a little; 1-bit collapses.
4. **[Aexol quant benchmark page](https://aexol.ai/benchmarks/quant/qwen-3.8-27b)**, summarising model cards:
   - NVIDIA NVFP4: within -1.54 to +0.75 points on six benchmarks;
   - AMD Quark MXFP4-AWQ: GSM8K +1.67;
   - ISTA 3.5-bit GSQ: near lossless.

Takeaways used here:
- 4-bit weights with group size of 128 or less are near-lossless on this exact model.
- The GatedDeltaNet input projections are the sensitive tensors (smaller groups or 8-bit).
- `lm_head` is never quantized in the published 4-bit checkpoints.

## 3. Which layers are sensitive, and what the plan does with them

| Tensor class | Published evidence | Plan |
|---|---|---|
| MLP gate/up/down, full-attention q/k/v/o, GDN `out_proj` | int4 g128 near-lossless (source 1) | Q4D8 seg 16: groups of ≤128 (80 at K=5120, 96 at K=6144, 128/128/16 at K=17408) |
| GDN `in_proj_qkv`, `in_proj_z` | int4 g128 worst, g32 fine, int8/BF16 best (source 1) | Candidate A: Q4D8 seg 4 (groups ≤32). Candidate B: keep FP8. RESULT.md bar picks one. |
| `lm_head` (BF16 today) | kept in source precision by all (source 1) | BF16 unless the `q4+lmq4` arm passes the same bar |
| `in_proj_a`/`in_proj_b`, norms, conv, A_log, dt_bias | kept by all | unchanged BF16 (47 MB per token in total) |
| `embed_tokens` | one row read per token | unchanged |

Measured sensitivity:
- per-tensor weight error: `MANIFEST.json` `rel_frob_err`;
- per-linear output error on real activations: q4eval `sens`, the first 256 rows of each sequence.

Both are in RESULT.md.

## 4. Alternatives considered

| Option | Why not chosen |
|---|---|
| NVFP4 / MXFP4 (E2M1 + FP8/E8M0 block scale) | No FP4 hardware on CDNA3 (section 1). Quality is similar to int4 g≤32 (source 4). |
| NF4 (16-entry codebook) | A table lookup per weight; no integer dot product exists for it. |
| Asymmetric int4 (zero point) | Needs a per-group sum of `x` correction; the published symmetric checkpoint was as good (source 1). |
| int4 with a per-row scale | Coarser than every published near-lossless recipe; groups of ≤128 cost 0.125-0.2 bit/weight. |
| Contiguous 128-column groups | Lanes would change group every step. The strided layout (8 columns × SEG steps, 512 apart) gives each lane one group per segment: one convert and one FMA per segment instead of per step. Expected quality is the same; it is measured, not assumed. |
| float16/BF16 activations with float decode of int4 | About 3 lane-ops per weight against a 3.85 budget (section 1). The 16-bit fixed-point activation keeps the integer path. |

One thing in Q4D8 is **not** in the published recipes: the activation's fixed-point form. Each token's vector
gets one power-of-two scale and 14-15 significant bits relative to its largest element. A token with a huge
outlier loses relative precision on its small elements.
- The quality arms `q4` and `q4keep` apply exactly this format to every input of every 4-bit linear, so its cost
  is inside the measured numbers.
- The per-linear output error separates it per tensor: q4eval reports `weight_only` (Q4 against FP8 weights,
  same Q4D8 activation), `activation_only` (FP8 weights, Q4D8 activation against raw) and `total`.

## 5. Bytes per token (decode, batch 1, weights only)

Integer counts come from the tensor shapes (`tools/q4format.geometry`). Ratios and times are from Wolfram, with
the MODELED floor at the data sheet's 5.3 TB/s.

| Configuration | Weight bytes per token | vs FP8 | MODELED floor at 5.3 TB/s |
|---|---|---|---|
| FP8 today (AmdKernelSpeed `model/classes.tsv`) | 26,919,925,760 | 1 | 5.08 ms |
| A: Q4 everything, `lm_head` BF16 | 15,537,809,408 | 0.577 | 2.93 ms |
| B: Q4, GDN input projections FP8, `lm_head` BF16 | 17,249,576,960 | 0.641 | 3.25 ms |
| A + Q4 `lm_head` | 13,662,496,768 | 0.508 | 2.58 ms |
| B + Q4 `lm_head` | 15,374,264,320 | 0.571 | 2.90 ms |

Wolfram query:
```
fp8 = 26919925760; a = 15537809408; b = 17249576960; alm = 13662496768; blm = 15374264320;
bw = Quantity[5.3, "Terabytes"/"Seconds"]; f[x_] := UnitConvert[Quantity[x, "Bytes"]/bw, "Milliseconds"];
-> ratios 0.5772, 0.6408, 0.5075, 0.5711; floors 5.079, 2.932, 3.255, 2.578, 2.901 ms
```

Bits per weight including scales (Wolfram, from the scale bytes per row):
- 4.2 for seg 16 at K=5120 (one segment of 10 steps, groups of 80);
- 4.6 for seg 4 at K=5120 (GDN input projections: segments of 4, 4 and 2 steps, groups of 32/32/16; corrected after
  review finding 14, which caught an earlier "4.5").

The byte table above was always computed from the exact shapes.
