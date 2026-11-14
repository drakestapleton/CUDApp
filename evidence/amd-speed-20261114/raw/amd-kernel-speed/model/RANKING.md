# Rung 2, step 1: bytes, floors and ranking per dispatch class (MODELED)

Subject: CUDApp `dc536882c06a692991be97dc65cd2d037d82b129`, the native plan `real2.plan`
(sha256 `2a292dadc236f224005ed4e2d8ac2ba8b33dc71d79dabcc8497d177561f9c012`, 1,126 dispatches per step),
Qwen/Qwen3.8-27B-FP8 revision `017b9c7af6b5689d5dd426a76e0bc077eb5ca20a`, one MI300X, 5.3 TB/s (AMD's figure).

## How the numbers were made

- `classes.py` rebuilds the step's dispatch list from `oracle/driver-real2.py` `Card.emit` and the checkpoint's own
  safetensors headers (fetched by HTTP range, `headers/FETCHED.json`; no weight bytes read). It asserts the per-kernel
  dispatch counts equal the published `real2.plan.json` `by_kernel` and the total equals 1,126. Output: `classes.tsv`.
- The same plan was re-emitted on the Mac from header-only stub files with the repository's own `emit-plan.py`;
  its sha256 equals the plan the card ran (`2a292dad…`). So the measurement plans below derive from the exact plan.
- Every division below is Wolfram (`WolframLanguageEvaluator`, queries in `wolfram-queries.md`).

## Correction to the stated floor

The task gives the floor as 24.38 GB per step and 4.6 ms. That number omits the output head. Wolfram on `classes.tsv`:

| What is read per step | Bytes | Floor at 5.3 TB/s |
|---|---|---|
| all FP8 projections (with block scales) + BF16 `in_proj_a/b` + one 10,240 B embedding row | 24,377,128,960 | 4.5995 ms |
| plus the BF16 output head `lm_head` 248320x5120 (read in full every step) | 26,919,925,760 | 5.0792 ms |
| plus delta-net recurrent state (305,154,240 B) and key/value rows at 286 positions | 27,243,823,296 | 5.1403 ms |

The checkpoint's `lm_head.weight` is BF16 (2,542,796,800 B, header of `outside.safetensors`). Reading it as FP8 would
change logits, so it is not output-preserving. **The output-identical floor for this plan is 5.08 ms (MODELED); 4.6 ms
needs a smaller output head and is a different, non-identical rung.** The 26.721 ms baseline moves 1.007 TB/s,
19.0 percent of 5.3 TB/s (MODELED from MEASURED 26.721 ms).

## Weight bytes and floor per class

| Class | Dispatches | Bytes per dispatch | Floor per dispatch | Floor per step |
|---|---|---|---|---|
| fp8 mlp gate/up 17408x5120 | 128 | 89,139,840 | 16.82 us | 2.153 ms |
| fp8 mlp down 5120x17408 | 64 | 89,139,840 | 16.82 us | 1.076 ms |
| bf16 lm_head 248320x5120 | 1 | 2,542,796,800 | 479.8 us | 0.480 ms |
| fp8 gdn in_proj_qkv 10240x5120 | 48 | 52,435,200 | 9.89 us | 0.475 ms |
| fp8 gdn in_proj_z 6144x5120 | 48 | 31,461,120 | 5.94 us | 0.285 ms |
| fp8 out_proj / o_proj 5120x6144 | 48+16 | 31,461,120 | 5.94 us | 0.380 ms |
| fp8 attn q_proj 12288x5120 | 16 | 62,922,240 | 11.87 us | 0.190 ms |
| fp8 attn k/v 1024x5120 | 32 | 5,243,520 | 0.99 us | 0.032 ms |
| bf16 gdn in_proj_a/b 48x5120 | 96 | 491,520 | 0.093 us | 0.009 ms |
| gdnstate_step | 48 | 6.36 MB state in+out | 1.20 us | 0.058 ms |
| 15 other small classes (norms, residual, SiLU, conv, attention, argmax) | 581 | under 1 MB, except attention part: 1.17 MB at 286 positions | under 0.25 us | under 0.01 ms |

## Gap ranking from earlier card measurements (apportioned, not a model run)

Kernel times from `~/handoffs/amd-omega-numbers/LIVE.md` (MEASURED earlier on MI300X through AMD's HIP launch, cold
wall clock over 200 launches; the kernel versions of those sessions are not re-confirmed against the dc536882 objects,
so this ranking only orders the work; the step-2 card measurement replaces it):

| Class | Earlier ms per dispatch | Percent of bandwidth | ms per step | Gap to floor per step |
|---|---|---|---|---|
| rmsnorm5120 (one workgroup, 129 per step) | 0.0179 | 0.06 | 2.309 | **2.308** |
| fp8 17408x5120 | 0.0324 | 51.9 | 4.147 | **1.994** |
| fp8 5120x17408 | 0.0367 | 45.8 | 2.349 | **1.272** |
| mlpsilu | 0.0034 | 1.2 | 0.218 | 0.215 |
| bf16 lm_head | 0.6866 | 69.9 | 0.687 | 0.207 |
| gdnconv4 | 0.0030 | 2.1 | 0.144 | 0.141 |
| sum of these six | | | 9.853 | |
| **not yet apportioned** (other 22 classes, and the cost of 1,126 serialized dispatches) | | | **16.868** | |

[INFERENCE] The unapportioned 16.9 ms is spread across 1,126 barrier-ordered dispatches (about 15 us each). Per-dispatch
fixed cost, small one-workgroup kernels (rmsnorm, gdnparams, argmax_final, merge) and the attention part grid
(2048x24 = 49,152 workgroups per layer, sized for 131,072 positions while 286 are used) are the candidates. The step-2
measurement separates them: an empty-dispatch control gives the fixed cost, and per-class insertion gives each class.

## Order of work (superseded 2026-11-14 by the card table in ../RESULT.md)

The card measurement in the real plan puts FP8 17408x5120 at 49.6 us (33.9 percent of bandwidth), 1.53x the earlier
isolated 32.4 us (Wolfram). [INFERENCE] The most direct candidate: the earlier figure was measured on the session-v5 "tuned" FP8 kernel, which is
not confirmed to be the dc536882 `fp8gemv_b128f` object. A second candidate: it came from a HIP benchmark that launched the same shape back
to back. There, each launch overlapped the previous one and the x vector stayed cached. In the plan, every dispatch
waits on its predecessor (barrier bit) and reads a freshly written x. The list below is the pre-measurement order.

1. Per-dispatch fixed cost and small kernels: fuse rmsnorm into its neighbours, multi-workgroup norm, fewer dispatches.
2. FP8 matrix-vector at 46 to 52 percent of bandwidth: the 192 MLP dispatches per step carry 3.27 ms of gap.
3. Attention part grid sized from the live position, not the capacity.
4. BF16 output head (70 percent), delta-net state and convolution.
