# Any-shape tensor GEMM

`gemm_ir.om` owns the default BF16, E4M3 and F16 PACK_A, PACK_B and MAIN builders. The public GEMM API, planner, existing encoder/cubin path and runtime launch path are unchanged. F32 retains its existing scalar FFMA reference builder. `src/omega_gpu_gemm.c` only replays the checked records into the existing register allocator. `om/units.sh` regenerates the committed checked C binding with the repository's pinned seeded compiler and bridge.

The old tensor builder is an oracle behind `CUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER=1`; the default is zero. All existing rt/ct/wm/wn, band, layout and stall configurations remain accepted; the planner's derived strides are unchanged. The Omega MAIN now double-buffers operand registers. Loads on held data barriers 3 and 4 overlap tensor work on the other set; address-read barrier 1 is retained, and the planner reserves the terminal prefetch slack. Tensor steps still accumulate in increasing K order. The builder differential contract remains **sim-equal output**, not byte-identical instructions: register naming, packing and built-in mutants differ from the retained C builder. Encoder parity is separate: build with `CUDAPP_ENCODER=diff` to encode every generated program through Omega and the retained C encoder. No tensor-core performance claim is made without a chip receipt.

## Edges and accumulation

The existing planner pads M/N to the configured CTA tile and K to four tensor instructions. PACK computes row and K validity independently, redirects invalid reads to a readable source word, then masks invalid values to zero. E4M3 byte reads use the existing aligned-u16 extraction convention. The MAIN loop consumes each packed pair in increasing K order. Stores independently predicate row and column and reconverge before continuing. BF16 steps contain 16 products; E4M3 steps contain 32. Accumulation starts at positive zero.

The independent host reference is `tests/gpu_gemm_test.c:ref_elem`, using `bw_hmma_model.h` and `bw_qmma_model.h`. It computes exact double products, aligns the products plus the accumulator at the group's nominal maximum exponent (nominal mode 2), truncates alignment at 26 significant bits, sums, then converts to float toward zero after each group. BF16 uses group 16; E4M3 uses group 32. This is a specified model, not a vendor arithmetic guarantee. Simulator comparison requires **identical float bits at every output**, including every tail output. Historical chip model receipts do not qualify the new builder.

A second independent reference sums decoded products in double. For the bounded finite sweep inputs (no overflow, underflow or NaN), a conservative group error coefficient is `u=2^-18`, including term alignment and float conversion. It overbounds `(33*4/2^26 + 1/2^23)`, allowing the largest group plus accumulator and nominal-exponent slack. With `n=Ceiling[K/16]`, the per-K absolute tolerance is `n/(2^18-n) * Sum[Abs[a_k b_k]]`. This supplemental check never replaces the stricter bitwise model check.

WolframLanguageEvaluator queries and results:

```
(33*4/2^26+1/2^23)<1/2^18 -> True
Table[{k,With[{n=Ceiling[k/16]},n/(2^18-n)]},{k,{1,16,17,32,33,64,8192}}]
-> {{1,1/262143},{16,1/262143},{17,1/131071},{32,1/131071},
    {33,3/262141},{64,1/65535},{8192,1/511}}
```

The tolerance applies to this bounded test corpus, not all possible BF16 bit patterns.

## Deterministic sweep and negative controls

The shape generator is independent of kernel tile sizes:

```
d=Join[Range[64],{127,128,129,255,256,257,511,512,513,
  1023,1024,1025,2047,2048,2049,4095,4096,4097,8191,8192}];
axis=DeleteDuplicates[Flatten[Table[{{v,3,5},{3,v,5},{3,5,v}},{v,d}],1]];
s=DeleteDuplicates[Join[axis,Table[{i,2 i+1,3 i+7},{i,30}],
  {{17,19,33},{31,33,65},{63,65,17},{65,67,33},{127,129,3},{129,127,5}}]];
{Length[s],Min[Flatten[s]],Max[Flatten[s]],Max[Times@@@s],
 Total[Times@@@s],Max[Times@@@s[[All,{1,2}]]],
 Max[Times@@@s[[All,{1,3}]]],Max[Times@@@s[[All,{2,3}]]]}
-> {286,1,8192,177510,3802243,24576,40960,40960}
```

These are 286 distinct dimension triples, not repeated layouts counted as shapes. Each axis reaches 1 and 8192. Every triple runs both types in all four layouts and every output is checked, not sampled. Long skinny shapes cover large axes without an impractical dense simulation; coupled dimensions and asymmetric multi-tile cases exercise simultaneous edges and K tails. This does not claim all triples in the dimension cube have been exhaustively enumerated. The established general GEMM simulator target additionally covers square/irregular shapes and general tile/warp/band configurations.

Mutant 1 rounds valid K down to a tensor step, dropping a nonempty K tail. Mutant 2 changes the edge predicate from row < R to row < R-1. Each mutant must successfully execute and produce a numerical mismatch on 17x9x33 for both types. A launch refusal is not credited as catching a mutant. Each sweep starts with poisoned output, and the simulator bounds every input, output and packed-buffer access. Both `--sim` and `--chip` run the numerical negative controls.

Host-only acceptance (opens no GPU device):

```
taskset -c 10-14 nice -n 10 make -j3 test-gpu-gemm-any test-gpu-gemm-any-differential test-gpu-gemm
```

The differential target builds a separate executable with the migration switch enabled, runs the same 286 shapes and compares all per-shape output digests. It also independently checks both paths against the host reference. Generated bindings are not a new host implementation: they are the checked bridge output from `gemm_ir.om`.

## Own Omega LLM policy and pipeline gate

`ollm.om` composes with `host.om` and `gemm_ir.om`. `gemm_ollm_cfg` takes the existing 32-word host planning wire, with M/N/K in words 8/9/10. Mode 0 preserves the f32 FFMA path; mode 1 selects BF16 input with FP32 accumulate. Success is status 0. Logical one-row decode uses rt=ct=wm=wn=1 (16x8 outputs, 32 threads). Prefill M>=64 uses the measured existing 64x128 default (rt2 ct4 wm2 wn4, band8). Shared memory is zero. This is a geometry choice, not a measured optimum. V32768 logits need two N16384 calls because this API's maximum dimension remains 16384.

Before PACK, use existing `om/apiviews/bf16.om` RNE conversion. Do not use grouped-FP8 tcgemm's scaled/permuted X format. Master weights and exact f32 training/evaluation remain unchanged. The existing model already has BF16 tensor kernels and fused ReLU epilogues in aien-train `gpu/gk.om`; these kernels are not automatically selected by changing the CUDApp session API. Its own .om driver must load these raw encoded kernels and use this argument/packing ABI. No model or live service is changed here.

The standalone gate uses the pinned compiler above and opens no device:

```
cat om/gemm-any/host.om om/gemm-any/ollm.om om/gemm-any/gemm_ir.om tests/gemm_pipeline.om | "$OMEGA_OMC" > build/gemm_pipeline
chmod +x build/gemm_pipeline
build/gemm_pipeline
```

Run under the operator's CPU pool lock. It covers 27 tensor geometries, three kernels per geometry, 72 own-model plans, four-word FP32 accumulator bundles and held barriers 3/4. `tests/bw_hazard_test.c` also checks every encoded geometry. Mutating the decode row tile bound or narrowing an accumulator must fail this gate. The full numeric file is `make test-gpu-gemm-any test-gpu-gemm-any-differential test-gpu-gemm`, with the predeclared bitwise model and per-K tolerances above. Checker results and simulation are host evidence, never a chip receipt.

The existing `gpu_gemm_test` additionally accepts `--shape M,N,K` for one exact BF16 timing/parity call, without silently clipping dimensions to 8192. Example inside an authorized window: `build/gpu_gemm_test --shape 1,384,384`. It retains the original comparator: every output for M*N*K<=2^27, otherwise the declared sample plus tail rows/columns. The original no-argument 208-shape sweep is unchanged.

## Resident own-model decode attention

`om/kgen/attention_ollm.om` provides `attention_ollm_ir` and `attention_ollm_wire` for E64, H6/8/16, f32 or explicitly selected BF16 KV. It reuses the existing fused general attention kernel, not the fixed-head grouped-Qwen builder. The layer cache is [K plane | V plane], each [head][capacity][64]; query and output are f32 [head][64] after RoPE. The shape is [heads,capacity,position,kv_bf16]. The helper fills the existing general-kernel table and constant-bank words, admitting only keys [0,position+1). It clears unused capacity without disturbing caller-supplied pointer words.

The caller supplies Q, layer-pool, output and table pointer pairs in argument words 0..7. Remaining consumed words 8..15 are MAXB=0, log2(e)/8 f32 bits, block stride=0, token stride, K-to-V plane bytes, head-map byte offset=16, heads, and zero. The table is [0,position+1,4+heads,0], head offsets in bytes, then block id zero. Launch grid is (heads,1,1), 64 threads, 1024 shared bytes. This is one sequence; no new batch ABI is implied. Keep the driver's existing cache-visibility and completion protocol.

Host-only exact wire/stream gate:

```
cat om/kgen/attention_emit.om om/kgen/attention_tuned.om om/kgen/attention_general.om om/kgen/attention_ir.om om/kgen/attention_ollm.om tests/attention_ollm.om | "$OMEGA_OMC" > build/attention_ollm
chmod +x build/attention_ollm
build/attention_ollm
```

Run under the CPU pool lock. It checks 36 resident KV wires, invalid shapes, and both kernel streams against the existing oracle-checked root. Changing hi to capacity must fail the same gate. Existing numeric acceptance remains `make test-api-kgen-attention test-gpu-attention`, with its unchanged f64-reference tolerances. No model integration or chip qualification is inferred from these host tests.

## Chip sweep and speed runbook — NOT_RUN

No new chip receipt exists. Do not run the following outside a current Drake-authorized exclusive window. Follow AGENTS.md's chip custody, sentinel and lock procedure first. Do not infer authorization from this document or a free lock. Never run beside the live model.

Build the binaries with `taskset -c 10-14 nice -n 10 make -j3 build/gpu_gemm_any_test build/gpu_tc_peak_test build/gpu_qmma_probe_test`. In the authorized window, measure the BF16 ceiling with `build/gpu_tc_peak_test` and the E4M3 ceiling with `build/gpu_qmma_probe_test` (its default includes the QMMA peak sweep). Preserve their clocks/configuration and raw receipts; do not substitute theoretical vendor rates. Set `BF16_CEILING_TFLOPS` and `E4M3_CEILING_TFLOPS` to those measured results. Then, inside the authorized chip wrapper:

```
build/gpu_gemm_any_test --chip --ceiling "$BF16_CEILING_TFLOPS,$E4M3_CEILING_TFLOPS"
```

The chip option refuses absent/nonpositive/nonfinite ceilings. It runs exactly the host sweep list with the exhaustive model and mathematical checks and both numerical mutants, then the existing timed 8192x8192x8192 probe for both types. The dense speed probe's existing reference samples outputs; it is explicitly separate from the exhaustive 286-shape acceptance list. Report each measured TFLOPS value divided by its same-type measured ceiling, preserving the probe's timing definition and clocks. The current implementation may trade speed for conservative scheduling; no target fraction is asserted.

Receipt requirements: commit and binary hashes, compiler/bridge pins, full shape list, all raw stdout/stderr, return code, model/tolerance versions, ceiling probe receipt, clock/thermal state, per-type timings and ceiling fractions, and a fresh negative-control run. Simulator PASS must never be relabeled chip PASS. No merge/deploy authority is conveyed.

## Current head evidence boundary

Host GEMM numerical tests and attention differential/simulation passed. The broader hazard sweep encountered an unresolved encoder parity abort, and the shared convolution simulator reported failures whose baseline causality is unresolved. The own-only checker and exact-own-shape timing/simulation options are implemented but not yet exercised. This head is HOLD, not hardware-qualified or ready to merge. No new fused epilogue or model integration is claimed. Do not replace the model's existing fused ReLU/residual path from this guide alone.
