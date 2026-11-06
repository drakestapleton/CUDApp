# DRA-197: unmodified TensorFold baseline on one GB10, decode profile and operator choice

Linear: DRA-196 (workstream), DRA-197 (this deliverable). Lane TensorFoldSpark, Spark hive.
Status: **EXERCISED on one GB10, single run, not yet independently reproduced.** Not a qualification.
No TensorFold code was changed. No Omega kernel exists yet; nothing here measures one.

## Pins

| Input | Value |
|---|---|
| TensorFold | github.com/ashhart/TensorFold `5a45539af031726f68f6dacdf713dbdff8f55b60` (main, 2026-11-06T22:17Z; the DRA-196 plan pinned `c70d67bb`, 4 commits earlier). Clean tree (`dirty=0` in env.txt) |
| Zig | 0.17.0, `zig-aarch64-linux-0.17.0.tar.xz` sha256 `9e8d11661d4ae3bd57702a3832781e23ad151dde5798e16a5ccd503f65234ff8` (ziglang.org index) |
| nvcc | CUDA 13.0 V13.0.88 (upstream's GB10 figures used 13.4) |
| Build | `zig build -j3 -Dgpu=spark -Dnvcc=/usr/local/cuda/bin/nvcc`, cores 0-2, 9 min 20 s, exit 0 (build.txt) |
| Binary | `zig-out/native/bin/tensorfold-native` sha256 `9c90827c70879c8300fff4bdf2c38d607262c5e6e69c181e14b4d926e03bd850` |
| Model | `mlx-community/Qwen3.5-2B-MLX-4bit@93760be4f1f69842a46bc13dbdc0f19e291392a3`, mlx-q4g64, tied head; `model.safetensors` sha256 `713fe7e5…bf07d5` (model.sha256). Benchmark workload for TensorFold only |
| Device | NVIDIA GB10 sm_121, driver 580.173.02, kernel 7.0.0-1019-nvidia |
| Window | spark-window `20261106T223312Z-f7bdfd`, chip and quiet locks exclusive; GPU part ran 22:33:21Z-22:34:31Z |

## Commands

```sh
TF_BIN=…/tensorfold-native TF_SRC=<TensorFold 5a45539a> TF_MODEL=<checkpoint dir> \
  spark-window run sh run/baseline.sh OUTDIR          # inside an open exclusive window
nsys stats --report cuda_gpu_kern_sum …               # done by baseline.sh
python3 run/decode_fractions.py prof.sqlite > decode-kernels.tsv
```

Server flags: `serve MODEL --context 8192 --no-thinking --no-update-check` (defaults otherwise: greedy,
copy drafts on, no prompt cache). Clients: TensorFold's own `tools/bench_openai.py --tokens 256 --reps 5
--temperatures 0` and `tools/bench_concurrent.py --levels 1,4,8 --tokens 256 --reps 3 --temperatures 0
--alone --serial`.

## Results (raw files in this directory)

Load: server ready 3.06 s after start (load.txt; the server's own line says 2.6 s).

| Workload | Streams | Decode tok/s per stream | Aggregate tok/s | TTFT | Token equality |
|---|---|---|---|---|---|
| chat, plain in practice (1.008 tokens/round) | 1 | 169.72 median of 5 (169.72, 169.21, 170.01, 168.66, 169.99) | — | 15.1 ms | sha `b453f81251cc` all 5 reps |
| code completion, copy drafts (2.37 tokens/round) | 1 | 393.27 median of 5 | — | 8.4 ms | sha `bc1513a7f13c` all 5 reps |
| code | 4 / 8 | 282.1 / 212.6 | 1113.6 / 1667.5 | ≤30 / ≤60 ms | alone 12/12, 24/24 equal |
| chat | 4 / 8 | 134.0 / 111.9 | 530.2 / 877.1 | ≤60 / ≤150 ms | alone 12/12, 24/24 equal |

Per-rep coefficient of variation (Wolfram): chat 0.341 %, code 0.444 %.
Cold versus warm: every request missed the (disabled) prompt cache, so these are cold-prompt figures; no
warm-cache cell was run. Peak memory: the server reports model 1.57 GiB and 0.19 GiB per stream at 8192
tokens (server-default.log); no separate peak-RSS sampler was run.

### Decode-window kernel profile (nsys 2025.3.2, `--cuda-graph-trace=node`)

A second server ran under nsys while `bench_openai.py --reps 3` ran (openai-prof.*; the profile slows
nothing visible: chat 169.2-170.0 tok/s in server-prof.log). The decode window is 9.801 s wall with the
GPU busy 9.227 s (94.14 %), 1436 rounds, 6.43 ms of kernel time per round. decode-kernels.tsv has every row.

| Kernel (grid) | Share of window wall | Avg per call | Note |
|---|---|---|---|
| `tf_lane_gemv_many` (192, 130, 80 CTAs) | 41.85 % together | 70.4 / 53.4 / 36.4 µs | fused multi-projection affine4 GEMV |
| `tf_lane_gemv::gemv_kernel<64,64,8>` (32 CTAs) | **20.62 %** | 29.4 µs, 48 calls/round | single-projection affine4 GEMV, n = 2048 |
| `tf_qmm_group::group_kernel<64,16,…>` (3880 CTAs) | 19.46 % | 1328 µs, 1/round | lm_head, n = 248320, 286.06 MB per call = 215.4 GB/s |
| all affine4 GEMV together | 81.93 % | | layer projections 772.15 MB/round = 181.1 GB/s |
| `tf_q35_norm` (1 CTA) | 2.20 % | 3.12 µs | bf16 RMSNorm + residual + group sums |
| `tf_q35_mlp_act` | 0.72 % | 2.10 µs | |

All fractions, bandwidths and ceilings below were computed with WolframLanguageEvaluator from the integer
nanosecond totals in decode-kernels.tsv and the byte counts in the checkpoint's safetensors header
(layer projections 772,153,344 B; tied embedding/head 286,064,640 B).

## Operator choice and Amdahl bounds

Amdahl: S = 1/((1-f) + f/s), f measured above, s = hypothetical kernel speedup (not a measurement).

| Candidate | f | s = 1.05 | s = 1.25 | s = 1.5 | s → ∞ | Can Omega/CUDA++ produce it today? |
|---|---|---|---|---|---|---|
| `tf_q35_norm` | 0.02196 | 1.0010 | 1.0044 | 1.0074 | 1.0224 | Nearly: f32 rmsnorm has chip receipts; bf16 I/O, residual and group-sum outputs are missing |
| `gemv_kernel<64,64,8>` | 0.20623 | 1.0099 | 1.0430 | 1.0738 | 1.2598 | No: needs int4 affine g64 |
| lm_head `group_kernel` | 0.19460 | 1.0094 | 1.0405 | 1.0694 | 1.2416 | No: needs int4 affine g64 |

**Selected operator (one): `tf_lane_gemv::gemv_kernel<64,64,8>`, the single-projection MLX affine4
group-64 decode GEMV at n = 2048, 1 row.** Its ceiling (1.2598) is the largest of the single
kernels, and it has the clearest headroom: 32 CTAs per call and an estimated 160.5 GB/s [INFERENCE: from
the inferred shapes, 4,718,592 B average per call over 29.406 µs], against 215.4 GB/s measured for the
lm_head kernel on the same chip in the same window. The norm is compatible sooner but its ceiling is 1.0224. The two projections it serves are [INFERENCE: 48 calls per round = 2 per layer, 32 tiles =
2048/64] the attention/GDN output projection (k = 2048) and the MLP down projection (k = 6144); DRA-198
must log the actual (n, k, sk) per call before any kernel work.

TensorFold's kernel contract requires a replacement to be bit-equal to the kernel it replaces
(docs/kernel-contract.md:47-49, :123 at 5a45539a). So the opt-in path keeps model behavior identical by
construction, and any gain must come from the same arithmetic in the same order, scheduled better.

**Missing capability (precise).** CUDApp main `b774723f` / omega-mv main `3e3dc205` have no int4 path:
1. int4 unpack with bf16 scale and bias per 64-element group (MLX affine layout after TensorFold's
   `tf_pack_dense` repack);
2. by-value struct kernel parameters (TensorFold passes `tf_lane_gemv::Part`, 4 pointers + 4 ints; the
   CUDApp cubin writer has 1/2/4/8-byte parameters only);
3. launch with programmatic dependent launch (TensorFold sets PDL on GB10) or an equivalent ordering proof;
4. fp32 accumulation that reproduces TensorFold's per-thread order, warp reduction and split-K (`sk`)
   slice sum bit for bit.
The sm_121 encoder that would emit it is C in CUDApp (`src/omega_blackwell_codegen*.c`), a migration-pending
host implementation; any claim will say "Omega/CUDA++ cubin", not "written in .om", until that changes.

## Preregistration

PREREG.md fixes the comparison, metrics, thresholds and statistics before any candidate exists.

## Not run

Independent reproduction (TfVerify); warm-cache cells; long-context prefill; a peak-memory sampler;
TensorFold's own `zig build test-cuda-host`, `kernel-contract` and golden tests on this build; any
upstream contact (TensorFold requires an ATC claim, which opens a public session: needs Drake's go).
NVIDIA tools: nsys only, on TensorFold's own Apache-2.0 kernels; timings only, no code listing
(PROVENANCE.md section 6).

## Rollback

Nothing installed system-wide. Remove `~/work/hive/TensorFold` (source, toolchain, 1.7 GB checkpoint) and
`/dev/shm/hive/TensorFold`. The raw profile `prof.nsys-rep` (sha256
`d2d0780b2752dea77771b5a71b0e3eecfe68641454616b0704a7d95bc52b42a3`) is kept outside git at
`~/handoffs/hive/tensorfold/DRA197-20261106T2233Z-prof.nsys-rep` on the Spark.
