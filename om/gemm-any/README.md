# Any-shape tensor GEMM

`gemm_ir.om` owns the default BF16, E4M3 and F16 PACK_A, PACK_B and MAIN builders. The public GEMM API, planner, existing encoder/cubin path and runtime launch path are unchanged. F32 retains its existing scalar FFMA reference builder. `src/omega_gpu_gemm.c` only replays the checked records into the existing register allocator. `om/units.sh` regenerates the committed checked C binding with the repository's pinned seeded compiler and bridge.

The old tensor builder is an oracle behind `CUDAPP_MIGRATION_PENDING_C_GEMM_BUILDER=1`; the default is zero. All existing rt/ct/wm/wn, band, layout, leading-dimension and stall configurations remain accepted. The Omega kernel deliberately serializes operand loads rather than copying the C builder's double-buffering. Consequently the differential contract is **sim-equal output**, not byte-identical instructions or equivalent speed. No tensor-core performance claim is made without a chip receipt.

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
d=Join[Range[1,64],{127,128,129,255,256,257,511,512,513,
  1023,1024,1025,2047,2048,2049,4095,4096,4097,8191,8192}];
s=DeleteDuplicates[Flatten[Table[{{x,3,5},{3,x,5},{3,5,x}},{x,d}],1]];
{Length[d],Length[s],Min[Flatten[s]],Max[Flatten[s]],Max[Times@@@s]}
-> {84,250,1,8192,122880}
Total[Times@@@s] -> 1919355
```

These are 250 distinct dimension triples, not repeated layouts counted as shapes. Each axis reaches 1 and 8192. Every triple runs both types in all four layouts and every output is checked, not sampled. The skinny shapes make the full host sweep practical; this does not claim all triples in the dimension cube have been exhaustively enumerated. The established general GEMM simulator target additionally covers square/irregular shapes and general tile/warp/band configurations.

Mutant 1 rounds valid K down to a tensor step, dropping a nonempty K tail. Mutant 2 perturbs the edge row by one. Each mutant must successfully execute and produce a numerical mismatch on 17x9x33 for both types. A launch refusal is not credited as catching a mutant. Each run starts with poisoned output, and the simulator bounds every input, output and packed-buffer access.

Host-only acceptance (opens no GPU device):

```
taskset -c 10-14 nice -n 10 make -j3 test-gpu-gemm-any test-gpu-gemm-any-differential test-gpu-gemm
```

The differential target builds a separate executable with the migration switch enabled, runs the same 250 shapes and compares all per-shape output digests. It also independently checks both paths against the host reference. Generated bindings are not a new host implementation: they are the checked bridge output from `gemm_ir.om`.

## Chip sweep and speed runbook — NOT_RUN

No new chip receipt exists. Do not run the following outside a current Drake-authorized exclusive window. Follow AGENTS.md's chip custody, sentinel and lock procedure first. Do not infer authorization from this document or a free lock. Never run beside the live model.

Build the binary using the host command above. In the authorized window, first measure BF16 and E4M3 tensor-core ceilings using the existing `gpu_tc_peak_test` probe and its documented clocks/configuration. Save its raw receipt; do not substitute theoretical vendor rates. Set `BF16_CEILING_TFLOPS` and `E4M3_CEILING_TFLOPS` to those measured results. Then, inside the authorized chip wrapper:

```
build/gpu_gemm_any_test --chip --ceiling "$BF16_CEILING_TFLOPS,$E4M3_CEILING_TFLOPS"
```

The chip option refuses absent/nonpositive/nonfinite ceilings. It runs exactly the host sweep list with the exhaustive model and mathematical checks, then the existing timed 8192x8192x8192 probe for both types. The dense speed probe's existing reference samples outputs; it is explicitly separate from the exhaustive 250-shape acceptance list. Report each measured TFLOPS value divided by its same-type measured ceiling, preserving the probe's timing definition and clocks. The current implementation may trade speed for conservative scheduling; no target fraction is asserted.

Receipt requirements: commit and binary hashes, compiler/bridge pins, full shape list, all raw stdout/stderr, return code, model/tolerance versions, ceiling probe receipt, clock/thermal state, per-type timings and ceiling fractions, and a fresh negative-control run. Simulator PASS must never be relabeled chip PASS. No merge/deploy authority is conveyed.
