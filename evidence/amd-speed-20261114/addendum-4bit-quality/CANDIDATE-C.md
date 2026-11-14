# Candidate C: Q4D8-1 weights with calibration-based error compensation (pre-registered)

Registered 2026-11-14 ~08:45Z, before any C measurement. Lane uxl-brief/FourBit. Main approved preparing it as design and code only; it uses no extra cores until it is run.

## What changes, and what does not

- **Unchanged:**
  - the container layout and arithmetic contract (`CONTAINER.md`, Q4D8-1);
  - the kernels (`q4gemv_d8*`);
  - bytes per token;
  - the activation format.
- **Changed:** only the choice of `q` (and optionally the scales) inside each group, which is `CONTAINER.md` section 4.
- **Method:** GPTQ-style error compensation (Frantar et al., 2022, arXiv:2210.17323), adapted to Q4D8's fixed groups.

### Steps for each linear W (M×K)
1. **Calibration activations.** X holds this linear's input vectors, N rows, taken from the FP8 model (`fp8` arm) on the calibration text. Each row is passed through the Q4D8 activation format, exactly as the kernel sees it. Then:
   - `Hm = 2·Xᵀ X / N + λ·mean(diag)·I`, with λ = 0.01;
   - columns where diag(Hm) = 0 get Hm[c,c] = 1 and W[:,c] = 0;
   - columns are sorted by descending diag(Hm), because GPTQ uses act-order.
2. **Scales.** Fixed before the sweep, by the existing section-4 clip search on W. The group of column c is unchanged: (row, seg(c), lane(c)).
3. **Sweep.** Go over the columns in the act order:
   - `q = clamp(round(w_c / s_group), -8, 7)`;
   - `err = (w_c − q·s) / Hinv[c,c]`;
   - `W[:, later] −= err ⊗ Hinv[c, later]`, using the upper Cholesky factor of `Hm⁻¹`, processed in blocks of 128 columns.
4. **Output.** The same tensors in the same format. Only the nibble values differ. The container metadata gains `q4_method = "gptq-actorder-v1"`, plus the sha256 of the calibration text and of the calibration settings.
5. **Classes.** Every class that candidate A converts. The GDN input projections stay at seg 4, as in A. If C fails, a C-keep variant (GDN input projections in FP8) follows the same rule as B.

## Calibration data, disjoint from every evaluation input
- **Text:** WikiText-2 raw **train** file (`wiki.train.raw` from the same `wikitext-2-raw-v1.zip`, sha256 `ef7edb56…`).
- **Selection:** the first 128 windows of 512 tokens, with no overlap with the test file used by bar input 1.
- **Rows:** N = 65,536 per linear.
- **Fixed seed for subsampling:** if memory forces fewer rows, take every k-th row (k = ceil(65536/N_max)) and record N.

## Bar: the same thresholds and inputs as RESULT.md section 1, unchanged

| Bar item | Pass when |
|---|---|
| B1 | PPL ratio ≤ 1.03 |
| B2 | top-1 agreement ≥ 94 % |
| B3 | generated agreement ≥ 95 % |
| B4 | mean KL ≤ 0.05 |
| B5 | ARC lost−won ≤ 3 |

- Data: `evidence/data.json`, body sha256 `6b26f641…`.
- Harness: the same `tools/q4eval.py`. The fp8 final state is reused (`--reuse-fp8-final`).
- **Decision rule:** C ships if it passes B1–B5. Otherwise C-keep is run, and it ships if it passes. Otherwise the result is FAIL.
- The Q4 lm_head is adopted only if `<arm>+lmq4` also passes.
- Thresholds, inputs and arms never change after the first C arm starts.

## Implementation plan (code to write; nothing measured yet)
1. **`tools/q4calib.py`.** A layer-streaming FP8 forward over the calibration windows, reusing q4eval's `build_layer` and its hooks.
   - For each converted linear, it accumulates `XᵀX` in float64 (K×K: 5120² or 17408², at most 2.4 GB) over Q4D8-formatted inputs.
   - It saves `H_<tensor>.npy` under `calib/`, resumable per layer.
   - lm_head: H from the final-norm outputs.
2. **`tools/q4gptq.py`.** For each tensor, it reads the source FP8 weights and H, computes the scales by `q4format.encode_matrix`'s clip search, runs the sweep, and writes the shards in the same container format into `container-c/`.
   - Determinism: float64 throughout, fixed order, and two runs give identical bytes.
3. **Gates.**
   - `test_q4gptq.py`, on random small cases:
     - with H = I, the output must equal `encode_matrix` exactly, because GPTQ with an identity Hessian reduces to round-to-nearest;
     - the output error `‖(W−Ŵ)X‖` must fall below round-to-nearest on correlated synthetic X;
     - nibble and scale layout pass `check_tensor`.
   - The existing kernel gate already covers any Q4D8 tensor, so no kernel change is needed. Run `gate.sh` with the C container.
4. **Run on the Spark CPU** under the pool locks, nice 10, pause-safe, with a BOARD line: q4calib (about 64 layers × 65,536 rows), then q4gptq, then `q4eval --streams q4 --reuse-fp8-final fp8-final-L64.npy` into `eval-c/`.

## Amendment 1 (2026-11-14 ~08:55Z, before any C measurement): calibration size
The 65,536-row figure assumed a GPU. On the Spark CPU, accumulating XᵀX for down_proj (K=17408) alone would cost
about 40 TFLOP per layer. Calibration is therefore **16 windows × 512 tokens = 8192 rows**: the first 8192 tokens
of `wiki.train.raw`, from its first 4,000,000 bytes, tokenised as the evaluation text is.

Shared inputs are stored once:
- qkv and z share one H;
- q, k and v share one H;
- gate and up share one H.

Implementation: `tools/q4calib.py` (`--windows 16 --window-len 512`). Thresholds, inputs, arms and the decision rule
are unchanged.
