## 1. Acceptance bar (fixed 2026-11-13, before any 4-bit quality number existed)

### What is compared
Teacher-forced CPU forward passes (float32 arithmetic) of the Qwen3.8-27B text model (64 layers + final norm +
lm_head), checkpoint `Qwen/Qwen3.8-27B-FP8` rev `017b9c7af6b5689d5dd426a76e0bc077eb5ca20a`, run on the Spark CPU one
layer at a time by `tools/q4eval.py`. Every arm uses the same code, tokens and order; only weights differ:

| Arm | Linear weights | Activation into those linears |
|---|---|---|
| `fp8` (reference) | E4M3 x BF16 block scale, decoded exactly to float32 | float32 |
| `q4` (candidate A) | Q4D8-1 from the container for every FP8 linear of the 64 layers (GDN `in_proj_qkv`/`in_proj_z` at seg 4, all others seg 16); lm_head BF16 | Q4D8 activation format (CONTAINER.md section 2), exactly what the kernel consumes |
| `q4keep` (candidate B) | as `q4`, but GDN `in_proj_qkv`/`in_proj_z` stay FP8 | Q4D8 format only into Q4 linears |
| `q4+lmq4` (lm_head variant, final-hidden of `q4`) | as `q4`, plus lm_head Q4D8 seg 16 | as `q4` |

Norms, convolution, A_log, dt_bias, `in_proj_a`/`in_proj_b`, embeddings and everything else are the checkpoint's own
BF16 values in every arm. MTP and vision are not run.

### Fixed inputs (token ids recorded with sha256 in `evidence/data.json` before the run)
1. **Perplexity text:** WikiText-2 raw test file (`wiki.test.raw`), the first 2048 tokens as 2 windows of 1024,
   scored on positions 1..1023 of each window (2046 predictions).
2. **Lighthouse prompt:** `oracle/lighthouse256.json` from CUDApp `dc536882` (30 prompt + 256 generated ids).
3. **Coding prompt:** the GOLD prompt and its 24 generated ids from `oracle/driver-real2.py` at CUDApp `dc536882`.
4. **Task benchmark:** ARC-Challenge test (Hugging Face `allenai/ai2_arc`, revision
   `210d026faf9955653af8916fad021475a3f00453`, `ARC-Challenge/test-00000-of-00001.parquet`), the first 100 rows in
   file order, zero-shot: `Question: ...` then one line per choice `A. text`, then `Answer:`; the
   prediction is the choice letter whose token (` A`, ` B`, ...) has the largest logit at the last position.

### Thresholds, chosen from published 4-bit results before measuring
Sources (FORMAT.md section 2): paired AWQ int4 g128 vs FP8 on this model, 99.1-99.99 % recovery
([nicosuter](https://github.com/nicosuter/Qwen3.8-27B-AWQ/blob/master/results/2026-08-23-published-quants/README.md));
Q4_K_M top-1 agreement with BF16 about 96 % ([kingy.ai audit, via Umar Salim](https://umarsalim.com/blog/quantising-qwen3-8-27b-where-quality-plateaus));
NVIDIA NVFP4 within 1.54 points on six benchmarks ([aexol summary](https://aexol.ai/benchmarks/quant/qwen-3.8-27b)).

| # | Measure | Pass when |
|---|---|---|
| B1 | Perplexity ratio `PPL(arm) / PPL(fp8)` on input 1 | <= 1.03 |
| B2 | Top-1 agreement with `fp8` over every scored position of inputs 1-3 (2046 + 256 + 24 = 2326 positions) | >= 94 % |
| B3 | Top-1 agreement with `fp8` on the 280 generated positions of inputs 2 and 3 (teacher-forced on the fixture ids) | >= 95 % |
| B4 | Mean KL(fp8 ‖ arm) per position over inputs 1-3, nats | <= 0.05 |
| B5 | ARC-100: items `fp8` gets right and the arm gets wrong, minus the reverse | <= 3 |

Also reported, no threshold: ARC-100 accuracy of each arm, the free-running first-divergence index of each arm on
inputs 2 and 3 (exact when `fp8`'s own teacher-forced argmax equals the fixture), the per-linear relative output
error on real activations, and per-tensor weight error.

**Decision rule (fixed now):** candidate A ships if it passes B1-B5. Otherwise candidate B ships if it passes
B1-B5 (so the `q4keep` arm is run only when `q4` fails; the first run is `fp8` + `q4`). Otherwise the result is FAIL, with the numbers. The Q4 lm_head is adopted only if `q4+lmq4` also passes
B1-B5; otherwise lm_head stays BF16. No threshold, input or arm definition changes after the first arm starts; a
wrong bar gets a dated correction file beside this one, never an edit.

The bar is a check on agreement with the FP8 checkpoint at this scale (2.3k positions, 100 items), not a claim
about long-context, multilingual or agentic behaviour, which are NOT_RUN.
