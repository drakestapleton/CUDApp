# Addendum: 4-bit weight quality for Qwen3.8-27B, 2026-11-14

**Result: FAIL for both pre-registered 4-bit candidates.** The 4-bit weights are not adopted. The 4-bit kernels and their speed results are unaffected. The next candidate changes only how the 4-bit values are chosen.

## The bar, fixed before any measurement
- **File:** `BAR-section1.md` in this folder, sha256 `3ac61cf962539c19652fdc2640b40317d46843690e63562ca0e5e07254fbc095`.
- **Reference:** the FP8 checkpoint `Qwen/Qwen3.8-27B-FP8`, revision `017b9c7af6b5689d5dd426a76e0bc077eb5ca20a`.
- **Inputs:**
  - WikiText-2 raw test, 2 windows of 1024 tokens;
  - the lighthouse prompt with its 256 generated tokens;
  - the coding prompt with its 24 generated tokens;
  - the first 100 ARC-Challenge test items.
- **Positions:** 2326 scored positions in total, plus the 100 ARC answers.

| Measure | Pass when |
|---|---|
| B1: perplexity ratio against FP8 | at most 1.03 |
| B2: top-1 agreement with FP8 on all scored positions | at least 94 % |
| B3: top-1 agreement on the 280 generated positions | at least 95 % |
| B4: mean KL divergence from FP8 | at most 0.05 nats |
| B5: ARC items FP8 gets right and the arm gets wrong, minus the reverse | at most 3 |

**Decision rule:** candidate A ships if it passes B1 to B5. Otherwise candidate B ships if it passes. Otherwise the result is FAIL.

## How it was measured (MEASURED on the Spark CPU, float32)
- **Harness:** each run is a teacher-forced forward pass, one layer at a time, using the transformers Qwen3.5 decoder layers.
  - On a small test model the harness matches the transformers whole-model forward exactly.
  - An independent reference with the activation hooks removed does not match, so the check can fail.
- **Weights:** the 4-bit arms read the converted Q4D8 container: signed int4 weights with BF16 group scales, groups of up to 128 weights (up to 32 for the gated delta net input projections).
- **Activations:** the same 16-bit activation format the GPU kernel consumes.
- **Raw files:** `metrics.json`, `positions.npz` and `bar.txt` for each candidate are in `candidate-a/` and `candidate-b/`, copied byte for byte.

## Results

| Arm | B1 PPL ratio | B2 top-1 | B3 generated | B4 KL | B5 ARC | ARC correct |
|---|---|---|---|---|---|---|
| FP8 reference | 1 (PPL 6.2126) | 100 % | 100 % | 0 | 0 | 97 of 100 |
| A: every linear 4-bit, BF16 head | 1.0465 FAIL | 90.41 % FAIL | 92.50 % FAIL | 0.0481 pass | 1 pass | 96 of 100 |
| A with a 4-bit head | 1.0658 FAIL | 88.65 % FAIL | 90.36 % FAIL | 0.0598 FAIL | 1 pass | 96 of 100 |
| B: as A, gated delta net input projections kept FP8 | 1.0599 FAIL | 90.97 % FAIL | 92.86 % FAIL | 0.0464 pass | 1 pass | 96 of 100 |
| B with a 4-bit head (not pre-registered; reported for completeness) | 1.0804 FAIL | 88.99 % FAIL | 91.43 % FAIL | 0.0585 FAIL | 1 pass | 96 of 100 |

**Decision under the frozen rule: FAIL.**

## Cause (MEASURED per linear layer on real activations)
- Rounding the weights to 4 bits causes 6 to 11 % relative output error in every class of linear layer.
- The 16-bit activation format adds about 0.1 %.
- Keeping the gated delta net input projections in FP8 (candidate B) did not change the outcome: B1 got worse (1.0599 against 1.0465), and B2 to B4 moved by at most 0.6 points.

The loss comes from round-to-nearest 4-bit weights spread across all layers. It does not come from the activation format or from one sensitive tensor class. The group layout was not varied, so it is not ruled out.

## Next: candidate C, registered before any measurement
`CANDIDATE-C.md` in this folder; sha256 values in `SHA256SUMS`.

- **What changes:** the 4-bit values are chosen with calibration-based error compensation (GPTQ style, act order), using WikiText-2 train text. The calibration text does not overlap the evaluation inputs.
- **What stays the same:** the container layout, the kernels, the bytes per token, the bar and the decision rule.
- **Status:** C is a follow-up and will be reported separately when it finishes.
