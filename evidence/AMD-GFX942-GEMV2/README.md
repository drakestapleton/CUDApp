# Wave-parallel gfx942 FP8/BF16 GEMV receipt

Intent: remove the observed serial-load dependency and insufficient wave count in the first GEMV kernels, preserving an explicitly defined, independently checked floating-point order.

Implementation and instruction/manual citations: [PROVENANCE section15](../../PROVENANCE.md#15-gfx942-wave-parallel-fp8bf16-gemv). This is a host receipt, not a GPU timing or bandwidth claim. The parent's MI300X run owns hardware qualification. No AMD GPU, live service or port18091 was touched; no push, merge or deployment was performed.

## Reproduce

Run in this repository on Linux AArch64, with the pinned compiler:

```sh
flock /home/drakestapleton/handoffs/hive/queen/pool-14.lock nice -n 10 \
  env AMD_OUT=/absolute/fresh/host sh tests/amd/run.sh \
  /home/drakestapleton/work/hive/amdserve/AmdGfx942/pin/omc
flock /home/drakestapleton/handoffs/hive/queen/pool-14.lock nice -n 10 \
  env AMD_EXEC_OUT=/absolute/fresh/exec sh tests/amd-exec/run.sh \
  /home/drakestapleton/work/hive/amdserve/AmdGfx942/pin/omc
```

The exact exercised paths are in `commands.txt`; exits, gate logs and their checksums are recorded beside this file. `SHA256SUMS` is generated from the receipt's files, not hand-maintained. Full temporary artifacts are retained at `/home/drakestapleton/handoffs/hive/amdserve/evidence/GFX942-GEMV2-WORK/`.

The new host tests check the union of the old GEMV shape tables and chunk/tail boundaries, both finite and special inputs, actual full-M/K sampled notebook rows, every output bit and all other memory bytes. Neither old object nor old gate coverage is replaced. New instruction cases come from the CDNA3 manual, not the executor; deliberate incorrect semantics must be caught. Whole `.text`, note and descriptor-resource bytes are checked against LLVM18.1.3. Two compiler builds and two code-object builds are byte-identical when the gates pass. Final PASS counts are generated from the case tables.

## Hardware bundle contract

- Symbols: `fp8gemv2` and `bf16gemv2`; same argument layouts as their older counterparts. Objects in `objects/`.
- Workgroup size64; dispatch M workgroups on X, one on Y. An HSA packet's work-item grid is `GRID_X=64*M`, `GRID_Y=1`.
- Row is workgroup X. Each lane visits `k=256*q+4*lane+j` in ascending q then j, starting its FP32 partial sum at +0. Use fused multiply-add with nearest-even rounding and gradual underflow.
- All64 lanes execute each simultaneous tree step `v[i]=RN32(v[i]+v[i XOR s])`, with s=32,16,8,4,2,1. Lane0 stores; FP8 multiplies by the row scale once after reduction.
- FP8 weights are E4M3FNUZ. CDNA3 manual physical p60: “The register SH_MEM_CONFIG, bit[8] must be set to 1 to produce the correct results for BF8 and FP8 operations.”
- The `*-tail.hsaco` control skips the whole nonempty remainder path. Exercise it with K not divisible by1024; the host control uses K=3. A no-tail control is not expected to differ when the remainder is empty. The `*-order.hsaco` and `*-stride.hsaco` controls are also retained.
- The order-control witness uses unit weights, x[0]=+2^24, x[64]=+1, x[128]=-2^24 and all other x=0, M=1,K=129, FP8 scale1. It swaps the first two tree stages. The Fraction oracle records the expected results, without trusting kernel output.
- Device NaN payloads, instruction scheduling and actual bandwidth remain hardware questions. The host canonical-NaN policy is explicit and is not a claim about device NaN payload selection.

## Diagnostics encountered and repairs

1. Pinned omc error12 at concatenated executor line977 (`if or lt get st 3 mul n 4 gt p sub get st 3 mul n 4`). The new wide-load bounds logic exceeded the compiler's temporary-register pool inside `globalop`. Extracted `wide(p,lane)` and split the lower/upper checks. No semantics or bounds were removed.
2. Pinned omc error7 at concatenated row-probe line1152 (`for i ulong sub argcnt 3`). `argcnt` is an int, while the loop count is ulong. Added the explicit `ulong argcnt` conversion after the positive argument-count check.
3. Initial host descriptor comparison failed: the table used four-register VGPR resource granularity while gfx942 LLVM emitted eight-register granularity. The instruction streams and metadata already matched. Corrected only the two new kernels' `COMPUTE_PGM_RSRC1` low field to the observed LLVM value; kept the independent descriptor comparison intact.

The initial failed gate and compiler diagnostic logs are preserved, rather than replacing them with a later PASS. The targeted manual suite and tuned-kernel suite were exercised after their specific repairs. Final full gates are separate logs.

## Parked work

Attention was parked before implementation, as requested, on `amdserve/gfx942-attn` at local WIP commit `98d54221`; its stop note is `evidence/AMD-GFX942-ATTN-WIP/README.md`. No attention kernel or attention object was produced. The tuned GEMV branch does not include that WIP note.
