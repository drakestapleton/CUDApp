# Preregistration: Omega/CUDA++ GEMV under TensorFold (DRA-198 measured by DRA-199)

Frozen 2026-11-06 by TensorFoldSpark, before any candidate kernel exists. Changing anything below after
a candidate result is seen requires a new dated version that keeps this one and says why.

## Arms (same device, binary, checkpoint, flags, prompts, budgets)

- **A (control):** TensorFold `5a45539a` + the DRA-198 opt-in patch, switch **off**. Must reproduce the
  token hashes of the unmodified baseline (README) before B is measured.
- **B (candidate):** the same binary with the switch **on**: `tf_lane_gemv::gemv_kernel<64,64,8>` calls at
  the logged (n, k, sk) shapes go to the Omega/CUDA++ cubin; every other kernel, the CUDA context, streams,
  graphs and the scheduler stay TensorFold's.
- Unmodified-baseline binary `9c90827c…` is run once per session as a drift check against README.

## Correctness gates (any failure = FAIL; speed is not interpreted)

1. TensorFold kernel contract, replacement mode (docs/kernel-contract.md:39-79, :123): gates 1-5 and
   `replaced` bit-equality against `gemv_kernel<64,64,8>` on served shapes and tile edges.
2. Planted controls must turn red: the candidate's `_bad_nan`, `_bad_repeat`, `_bad_tail`, `_bad_sums`
   variants (pattern of zig/src/core/weight_only_cuda_impl.zig:5-45, loadable only from a controls image),
   plus a one-ulp perturbation of one output. A control that passes = FAIL of the gate.
3. Model behavior: per-request `token_sha256` of B equals A for every prompt and repetition; drafted equals
   plain (`--serial`); concurrent equals solo (`--alone`) at 1, 4, 8 streams.
4. Opt-in refusal paths: missing cubin, wrong sha256, missing symbol and a block-size limit below the launch
   each refuse at startup with the default path unchanged.

## Primary metric and decision

- Workload: `tools/bench_openai.py` prompt `gpu-chat-no-think` (plain decode, 1.008 tokens/round in the
  baseline), `--tokens 256 --temperatures 0`, 1 stream, `--context 8192 --no-thinking`.
- Unit: decode tok/s as that tool defines it.
- Design: 10 blocks. Each block runs A and B, each on a freshly started server, order AB in odd blocks and
  BA in even blocks; 1 discarded warm-up request, then 5 measured reps; block value = median of the 5.
- Statistic: d_i = ln(B_i / A_i); mean and two-sided 95 % Student t interval (df = 9), computed with
  WolframLanguageEvaluator; speedup = exp(mean), bounds = exp(interval).
- **PASS:** every correctness gate passes, lower bound > 1.000 and point estimate ≥ 1.010.
- **INCONCLUSIVE:** gates pass, the interval contains 1.000.
- **FAIL:** a gate fails, or the upper bound < 1.000 (a measured loss is published as such).

## Secondary metrics and regression limits (same blocks)

| Metric | Limit (lower 95 % bound of B/A unless noted) |
|---|---|
| Code prompt decode tok/s (copy drafts) | ≥ 0.990 |
| Aggregate tok/s at 4 and 8 streams (`bench_concurrent.py`, 3 reps per block) | ≥ 0.980 |
| TTFT median | upper bound of B/A ≤ 1.050 |
| Server-reported model and per-stream memory | identical |
| Replaced-kernel mean time per call, one profiled pair (nsys, descriptive) | reported, no threshold |

A secondary limit breach turns PASS into FAIL.

## Expected size (Amdahl, measured f = 0.20623)

Kernel speedup 1.05 → 1.0099 end to end; 1.25 → 1.0430; 1.5 → 1.0738; ceiling 1.2598 (Wolfram). The
1.010 practical threshold therefore needs a kernel speedup above about 1.05. Baseline per-rep CV is 0.341 %
(chat), so 10 blocks resolve a 1 % effect [INFERENCE: not a formal power calculation].

## Out of scope for this comparison

Other kernels, prefill, long context, warm cache, other models or chips, upstream submission.
GPU access: one exclusive spark-window lease per session, queued behind other hive leases on the BOARD.
