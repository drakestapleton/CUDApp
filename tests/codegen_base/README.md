# Base codegen and common IR replay migration

Source baseline: CUDApp #309, `63d18a2cc92cb05f800d17e46b35f0cd6237b3b0`.
The seeded compiler is omega-mv `e5b32567`, SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Registration was committed before caller cutover (`79de6f9`).

The production entry remains `omega_gpu_om_ir_replay`. No opcode was added.
The baseline ends at `BW_IR_ATOMG_ADD_F32_STRONG_GPU`; #312's separately
stacked f64 extension ends at 78, leaving 79 unused. When combining #312,
the replay admission call must receive its `BW_IR_LAST` rather than the
baseline enum maximum. This branch does not silently import that other PR.

## Every function in the two target files

Names in each row are exhaustive, including guarded alternate definitions.
“Already ported” refers to the default hosted path, not a claim that the
existing freestanding migration oracle has been removed. Verification
oracles remain independent C; they are not production implementations.

| Functions | Classification | Implementation / reason |
|---|---|---|
| `omega_bw_ir_alloc_vreg`, `omega_bw_ir_alloc_vreg64`, `omega_bw_ir_alloc_vreg128`, `omega_bw_ir_alloc_uvreg`, `omega_bw_ir_alloc_uvreg64`, `omega_bw_ir_append`, `bw_ra_decide`, `bw_ra_alloc`, `bw_ra_touch` | Already ported, #309 | `regalloc_ir.om`; C selects ABI fields, copies instructions and writes intervals. Guarded C alternatives remain the #309 oracle. |
| `omega_bw_regalloc_c_oracle`, `omega_bw_regalloc_solve` | Already ported, #309 | `regalloc.om`; default solve serializes/deserializes the fixed wire arrays. |
| `omega_bw_gpr_declared`, `omega_bw_gpr_declared_ok`, `omega_bw_ir_set_gpr_limit` | Already ported, #309 | `regalloc_ir.om`; existing oracle alternatives are unchanged. |
| `bw_c_encode_insn` | Already ported, #79 and encoder extensions #301/#307 | `bw_enc_sm121.om` owns instruction packing, control/scheduling arithmetic, operand checks, branch deltas and padding. C is the independently selectable encoder oracle. |
| `bw_om_encode` | Computation moved here | `cg_fields.om` packs instruction fields; C routes physical-register arrays, invokes the checked binding and copies output. |
| `encode_single_insn` | Computation moved here | `cg_fields.om` assembles the four little-endian words. Its C-primary alternative remains the existing encoder oracle. |
| `omega_bw_encode_program` | Computation moved here / existing #309 binding | `cg_bind.om` kind 6 now admits capacity and computes required bytes together; C checks pointers and dispatches the encoder. |
| `bw_encode_program_bytes` | Already ported, #79/#309 | Omega encoder produces bytes; C copies the result or runs the explicitly enabled differential oracle. |
| `bw_om_matprog`, `omega_blackwell_codegen_matmul_i32_prog`, `omega_blackwell_codegen_matmul_tensor_prog`, `omega_blackwell_codegen_matmul_tensor_loop_prog`, `omega_blackwell_codegen_matmul_frag_prog`, `omega_blackwell_codegen_matmul_frag_block_prog` | Already ported, #309; matmul excluded from this lane | `cg_matmul_params.om`, `cg_matmul_i32.om`, `cg_matmul_tensor.om`, `cg_matmul_loop.om`, `cg_matmul_frag.om`, `cg_matmul_block.om`; C packs scalar ABI arguments and calls the common replay entry. |
| `bw_keep_alive`, `bw_live_until`, `frag_block_store_fits` | Already ported, #309 | Only in the existing guarded matmul oracle; corresponding lifetime/store-offset policies are Omega. |
| `bw_mat_decide`, `omega_matmul_frag_block_shape`, `omega_matmul_frag_block_unroll`, `omega_matmul_frag_unroll` | Already ported, #309 | `cg_matmul.om`; pointer-output unpacking remains ABI glue. |
| `frag_a_tab_init`, `frag_f32_to_bf16`, `omega_matmul_frag_pack_a`, `omega_matmul_frag_pack_a_f32`, `omega_matmul_frag_pack_a_row0`, `omega_matmul_frag_pack_a_row0_f32`, `omega_matmul_frag_pack_b`, `omega_matmul_result_sentinels` | Outside owned scope, #323 | ApiViewsElem owns these A/B packers and sentinel traversal. No competing edits. |
| `omega_bw_ir_init`, `omega_blackwell_encode_one` | ARCH-0042 ABI shell | Object clearing, identity-map initialization, pointer checks and invocation. No encoding policy. |
| `omega_bw_program_code_bytes`, `omega_blackwell_kernel_encode_into` | Already ported, #309 / ARCH-0042 shell | `cg_bind.om` makes size/count decisions; C routes caller-owned buffers, hashes bytes and populates the public descriptor. |
| `bw_kernel_alloc_encode`, `omega_blackwell_codegen_matmul_i32`, `omega_blackwell_codegen_matmul_tensor`, `omega_blackwell_codegen_matmul_tensor_loop`, `omega_blackwell_codegen_matmul_frag`, `omega_blackwell_codegen_matmul_frag_block` | ARCH-0042 allocation shell | Pointer checks, stack object selection, malloc/free and existing program-generator dispatch. |
| `omega_kernel_dump` | ARCH-0042 file shell | Optional environment-selected file writing and content hashing; no instruction computation. |
| `bw_enc_diff_note` | Verification shell | Writes differential receipts and aborts on a disagreement; production encoder selection is unchanged. |
| `omega_blackwell_verify_codegen_fixtures`, `omega_blackwell_verify_codegen_fixtures_intops`, `omega_blackwell_verify_codegen_fixtures_fragmm`, `omega_blackwell_verify_codegen_fixtures_reconv`, `omega_blackwell_verify_codegen_fixtures_bfly_f2fp`, `omega_blackwell_verify_codegen_fixtures_atomf32`, `omega_blackwell_verify_codegen_fixtures_fb1cut4`, `omega_blackwell_test_regalloc_bounds`, `omega_blackwell_test_codegen_variation` | Independent verification oracles | Golden-word comparisons, fixture construction, assertions and reporting intentionally remain C, not an adoption of a production C implementation. |
| `field` (`omega_gpu_om_ir.c`) | Computation moved here | `cg_replay.om` admits each register field; `cg_fields.om` decodes it. Old helper is default-OFF `CUDAPP_MIGRATION_PENDING_C_CODEGEN_BASE`. |
| `extend_loop` | Computation moved here | `cg_life.om` extends signed live-in intervals to loop closure. |
| `loop_invariant_ok` | Computation moved here | `cg_invar.om` checks each source, nonuniform redefinition and post-loop use. |
| `omega_gpu_om_ir_replay` | Computation moved here; entry retained as ARCH-0042 shell | `cg_replay.om` checks record/metadata/width/operand/closure admission; `cg_fields.om` decodes; `cg_life.om` extends loops and validates exact hints; `cg_invar.om` checks invariants. C invokes the emitter, marshals objects and dispatches already-ported allocation. |
| `omega_gpu_om_ir_build` | Computation moved here; buffer shell retained | `cg_budget.om` owns the declaration floor, declaration/peak constraint and CTA register budget. C retains pointer/zero-thread entry guards and encode dispatch. |
| `call_unit`, `omega_gpu_om_ir_data`, `omega_gpu_om_ir_rc_name` | ARCH-0042 ABI/replay dispatch shell | Calls the selected bridge entry, checks its status/result, clears caller memory and returns the stable diagnostic string. |

The prior family codegen PRs #274–#278 and #313–#316 implement separate
translation units and are not duplicated or modified here. #312's f64 IR
extension is likewise not overwritten. Cubin assembly remains in the existing
`omega_cubin` path; this lane compares its exact output rather than adopting
a different writer.

## Default and oracle selection

The newly removed C wire packing, word decoding, IR argument validation,
lifetime extension/invariant checking and build-budget arithmetic are behind
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_BASE`, undefined by default. The guard
preserves the original computation for the differential build. There is no
fallback from an Omega refusal to C.

This does not rename or enable #309's separate `CUDAPP_MIGRATION_PENDING_C_REGALLOC`,
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_MATMUL`, `CUDAPP_MIGRATION_PENDING_C_BIND`, or the
existing encoder oracle. The inherited `OMEGA_GPU_NO_POSIX` path selects those
older C oracles because the pinned scalar-only freestanding bridge cannot
convert their array views. This existing limitation is not evidence of a
freestanding Omega cutover. No new host binary/library/runtime dependency is
adopted by this lane; generated bindings use the repository's pinned bridge.

## Source-derived partition and negative controls

`differential.c` derives the partition from each admission predicate in the
original common replay and from the field widths in its record layout:

- Instruction count around 1, 8, 32 and 1024; vreg counts around 160/256;
  uniform counts around 32; loop counts around 24; GPR limits around 128/246.
- Each width tag, all opcode tags through the baseline maximum and one beyond,
  all five operand positions around their bounds, sentinel and `UINT32_MAX`.
- Every metadata flag value and emitter status/result class. Admission compares
  the return code and emitter diagnostic; successful replay compares the whole
  zero-initialized IR object. Error outputs do not promise partially mutated IR.
- Every control/flag wire bit through real program encoding; each field against
  the old source-derived pack/decode formula; little-endian byte-to-word checks.
- Cross-product loop start/end values, unsorted and shared closures, undefined,
  inside-loop and after-loop definitions, all three source positions,
  nonuniform and uniform redefinitions, reads before/inside/after a loop,
  and exact-live-hint bounds. Signed C `int` closure conversion and the -1
  interval sentinel are preserved explicitly without checked-cast traps.
- Peak/thread boundary cross product, including zero and `UINT32_MAX`, checks
  the source budget formula against `cg_budget.om`.

Each accepted encodable program compares the entire padded SASS byte stream
against the original C encoder and packs both streams with identical cubin
metadata, comparing the entire resulting cubin. No cubin, PTX or disassembly
is written to evidence or committed. The red build flips one SASS bit before
comparison and must fail. Five independent seeded-source mutants change field
shift, opcode bound, loop extension, loop rejection and the budget boundary;
all must be killed by the same executable.

## Reproduction and tiers

Run from the repository root. Every command below is host-only and must run
inside the device-hidden wrapper, even the mutant and simulator paths:

```sh
export OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc
export OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge
export OMEGA_BRIDGE_DIR=$OMEGA_BRIDGE
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  make -j1 test-codegen-base-om-mutants test-regalloc-om test-om-encoder test-codegen-freestanding
```

`tests/codegen_base/run.sh` emits source coverage of the preserved C oracle in
`build/codegen-base-om/`; generated mutant copies remain only in that ignored
build directory. `CODEGEN_BASE_OUT` selects a separate build directory.
See dated evidence for exercised results, coverage dispositions and exact
commands. The full `make test` belongs to the integration owner, not this lane.
Chip/live-RM/VFIO are **NOT_RUN**; see [CHIP.md](CHIP.md).

The change adds no language, compiler, pin, bridge profile, public API or
repository ownership rule. Existing omega-harness skill descriptions therefore
remain applicable; there is no skill example to refresh.
