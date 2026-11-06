# Seeded Omega MoE codegen migration

Stack: `feat/omega-codegen-moe` is based on CodegenA's `feat/omega-codegen-a` at `89ee4db` (generic replay introduced at `22ebf8f`), not directly on main. Registration is commit `9300208`.

## Implementation

Both public `.h` APIs remain unchanged. The handwritten C implementations remain solely behind `CUDAPP_MIGRATION_PENDING_C_CODEGEN_MOE` and `CUDAPP_MIGRATION_PENDING_C_CODEGEN_MOE2`, each defaulting to zero. The default C entry points serialize the original specification, invoke seeded Omega, and replay records using the existing `omega_gpu_om_ir_replay`. C also serializes physical allocation metadata for the Omega scheduling pass; it does not choose instructions, allocations, loop extents, branch targets, decode arithmetic or stall values.

Units:

- `cg_moe_emit.om`: exact record/control packing, allocator requests, branch patching.
- `cg_moe_core.om`: validation, allocations, common prologue, deep staging, decoder and row-pipeline operations.
- `cg_moe.om`: step/deep/software-pipeline/coalesced-output builder.
- `cg_moe2.om`: row-pipelined builder.
- `cg_moe_sched.om`: physical-register dependency scheduling for variant 1's optional scheduling mode.

Compiler pin: omega-mv `e5b32567`, omc SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. Generated portable C is checked in. The previous untracked emitter was treated as unproved input; branch origins, allocator metadata and loop handling were corrected against the C oracle. No prior lane receipt is reused.

## Structural domain from the C oracle

Common: lane bytes 8 or 16; rows 1, 2, 4 or 8; warps every integer 1 through 32; positive K at most 32768, divisible by 128 and by `32 * lane_bytes`; signed row-load immediate bound. The harness iterates K in steps of 256 and lets the original C guard reject the wrong lane-width multiples.

Variant 1 additionally enumerates depth 0 through 16, scheduling/coalescing/weight-first each 0 or 1, and every mutant selector 0 through 5. Selectors 4 and 5 require coalescing; coalescing requires `warps * rows <= 32`. Nonzero depth divides the K-step count, requires `rows * depth * lane_bytes <= 128`, and K at most `512 * warps`. The C staging code supports a clamped partial final round; no stronger divisibility gate is introduced.

Variant 2 enumerates mutant selectors 0 through 3 and the common structural axes. It requires at most 16 K steps, `rows * steps * lane_bytes <= 128`, and K at most `512 * warps`. Its original C ignores the entire uint32 depth, scheduling, coalescing and weight-first fields. None lands in any immediate or other output field: the C oracle never reads them. The test therefore exercises every single-bit value and all-ones independently for each of those four fields at every accepted structural tuple. There is no claim to have enumerated the entire uint32 Cartesian product; structural exhaustion plus ignored-scalar bit basis is explicit. The source mutant leaking the high depth bit into a descriptor immediate is required to fail.

The host differential compares exact return codes, every IR instruction, allocation state, encoded SASS bytes and packed cubin bytes in memory. No output binaries, SASS, cubins, PTX or NVIDIA listings are committed. All scalar axes also receive boundary and bit-basis rejection checks. Refused programs have no specified scratch contents and only their exact return codes are compared.

## Verification commands

Use the pinned environment:

```
export OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc
export OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge
taskset -c 10-14 nice -n 10 sh om/units.sh --check
taskset -c 10-14 nice -n 10 make -j3 test-codegen-moe-diff test-codegen-moe-sim
taskset -c 10-14 nice -n 10 python3 om/codegen/moe_mutants.py
taskset -c 10-14 nice -n 10 make -j3 test
```

The source-mutant runner requires baseline success, successful compilation of each altered `.om` input, and the differential harness's ordinary mismatch exit (not a crash). The independent functional simulator fixture uses `bw_warp_sim_k.h`, both builders, both fragment orders and ascending/descending/shuffled warp visits. The scale-drop negative control must change every output. The simulator does not model scoreboard timing.

## Arithmetic provenance

WolframLanguageEvaluator: `FromDigits[#,16]& /@ {"358","380","388","390","398","39c","3a0","3a4","07F00000","80","000fc000","ffffffff"}` -> `{856,896,904,912,920,924,928,932,133169152,128,1032192,4294967295}`.

WolframLanguageEvaluator: `{2^64,2^56,512*1*1*1,512/128}` -> `{18446744073709551616,72057594037927936,512,4}`.

WolframLanguageEvaluator: `{(1+Mod[56,8]/8)*2^(Quotient[56,8]-7),512*(1+Mod[56,8]/8)*2^(Quotient[56,8]-7)}` -> `{1,512}`. This supplies the independent all-one e4m3 simulation reference.

## Chip tier: NOT_RUN

No GPU window was authorized. No command in the host acceptance paths opens a device. No ROADMAP qualification status is upgraded.

In a separately authorized exclusive GB10 window, first build `taskset -c 10-14 nice -n 10 make -j3 build/gpu_fp8_api_test`. Capture the current revision, compiler/bridge pins and current window authorization in a new receipt directory. Run `./build/gpu_fp8_api_test --host-only` and `./build/gpu_fp8_api_test --sim`; then use the existing FP8 chip runner/authorization procedure to execute the chip parity and shipped negative-control battery with `./build/gpu_fp8_api_test --variant 1 --out <new-receipt>/variant1.json` and `./build/gpu_fp8_api_test --variant 2 --out <new-receipt>/variant2.json`. The checked CLI also supports `--sched`, `--depth`, `--coal` and `--wfirst`; exercise the default and scheduling/deep/coalesced/weight-first arms in separately named receipts. Record exit codes and output digests and preserve the shipped arithmetic/parity negative controls. Stop on a failed gate; do not weaken it. Chip qualification remains separate from host differential and simulator evidence.
