# Seeded core allocator and matmul builders

The default core allocator and five matmul IR builders are seeded Omega units.
The retained C differential oracles are selected only with
`CUDAPP_MIGRATION_PENDING_C_REGALLOC` and
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_MATMUL`. The vecadd instruction writer also
uses the former switch. The existing `bw_enc_sm121.om` encoder is reused.

The register allocator preserves the old allocator's interval ordering,
strict last-use release rule, scalar/pair/quad alignment, uniform register
reservation, pressure refusals, partial refusal state, and allocation maps. The
C shell owns program memory and marshals the `ulong[4096]` wire documented in
`regalloc.om`; allocation choices belong to Omega. `regalloc_ir.om` owns the
capacity, interval, declared-GPR and limit decisions.

The matmul units emit the existing IR replay wire, not machine code. The sole
replay implementation handles allocation and encoding. Builder-specific
`keep_alive`, carried loop values, and accumulator lifetimes use extension-only
exact liveness in meta[7] and meta[352..511], documented next to meta[6] in
`src/omega_gpu_om_ir.h`. No instruction enum entries are renumbered.

## Portable registration

All eleven units are registered in the default `UNITS` list of `om/units.sh`
and lowered by the pinned portable bridge (OM-1, no `om2` header) into checked
generated C. The declarations are in `src/cudapp_native.h`. The default build
needs only the pinned omc; `sh om/units.sh --check` proves the committed C
regenerates byte for byte.

| Source under om/codegen | Generated C | Export |
| --- | --- | --- |
| regalloc.om | src/cudapp_freest_regalloc.c | omega_ra_run |
| regalloc_ir.om | src/cudapp_om_regalloc_ir.c | omega_ra_ir |
| regalloc_live.om | src/cudapp_om_regalloc_live.c | omega_ra_live |
| cg_matmul.om | src/cudapp_om_cg_matmul.c | omega_cg_matmul |
| cg_matmul_params.om | src/cudapp_om_cg_matmul_params.c | omega_cg_par |
| cg_matmul_i32.om | src/cudapp_om_cg_matmul_i32.c | omega_cg_i32 |
| cg_matmul_tensor.om | src/cudapp_om_cg_matmul_tensor.c | omega_cg_tens |
| cg_matmul_loop.om | src/cudapp_om_cg_matmul_loop.c | omega_cg_loop |
| cg_matmul_frag.om | src/cudapp_om_cg_matmul_frag.c | omega_cg_frag |
| cg_matmul_block.om | src/cudapp_om_cg_matmul_block.c | omega_cg_block |
| cg_vecadd.om | src/cudapp_om_cg_vecadd.c | omega_cg_vec |
| bw_hazard.om | src/cudapp_om_bw_hazard.c | omega_hz_chk |
| bw_hazard.om + bw_hazard_dec.om | src/cudapp_om_bw_hazard_dec.c | omega_hz_dec |
| cg_bind.om | src/cudapp_om_cg_bind.c | omega_cg_bind |
| cg_fields.om | src/cudapp_freest_cg_fields.c | omega_cg_fields |
| cg_replay.om | src/cudapp_freest_cg_replay.c | omega_cg_replay |
| cg_life.om | src/cudapp_freest_cg_life.c | omega_cg_life |
| cg_invar.om | src/cudapp_freest_cg_invar.c | omega_cg_invar |
| cg_budget.om | src/cudapp_om_cg_budget.c | omega_cg_budget |

`bw_hazard.om` is the SM121 hazard decoder and checker behind
`src/omega_bw_hazard.c` (C oracle: `CUDAPP_MIGRATION_PENDING_C_HAZARD`). The
portable bridge needs fixed-size views, so the checker takes a 32768-byte code
view (2048 instructions) and a 3203584-word state; longer code is refused,
where the C accepted it. `cg_bind.om` owns the matmul spec bytes, the
realization binding message, the machine descriptor, matmul dispatch, vector
launch geometry, program padding, buffer fit and the code-digest admission
used by `omega_blackwell_matmul.c`, `omega_blackwell_realize.c`,
`omega_blackwell_codegen.c` and `omega_blackwell_encoder.c` through
`src/omega_bw_bind.h` (C oracle: `CUDAPP_MIGRATION_PENDING_C_BIND`). The C
keeps memory, structs and buffer routing. Under NO_POSIX, `cg_bind.om` and
the shared `bw_id_native` wrapper over `cache/digest.om` and
`blackwell/identity.om` produce the canonical binding and identity, without
a C fallback. Kernel code-digest calls still route through `src/sha256.c`.

`cg_sentin.om` exposes a uint-bit view for the native integer ABI; its private
binary32 equality is unchanged, including signed zero and NaN behavior.

The portable bridge has no integer-to-integer bitcast. `regalloc.om` stores
signed words through `ra_u2l`/`ra_l2u`, a portable two's-complement
reinterpretation. Freestanding builds (`OMEGA_GPU_NO_POSIX`) use the existing
seeded allocator, five core matmul builders, scalar policies, fragment packers,
sentinels, vecadd writer and binding decisions through checked caller-stack
native units registered in `om/no-posix.sh`. They no longer select C computation
merely because POSIX is unavailable. C remains an explicit default-OFF oracle.
`make test-codegen-freestanding` compares hosted and native Omega instruction dumps.
`make test-om-encoder-primary` compares 1024 register/control/predicate samples
for each of eight operand forms of every opcode 0..78 against C, hosted Omega and native Omega; the conductor is
`om/encoder/primary_parity.om`, including a flipped-byte refusal control.
The operand-mode sweep also caught the seeded LDG.E encoder omitting its
existing C signed byte-offset field. Both seeded encoder and spec table now
use the same field as the existing wide-load forms. The dedicated LDG gate
checks signed extrema, range refusal and dropped-offset/bound mutants.

`tests/regalloc_om/run.sh` checks regeneration, then links the generated units.
It compiles the C oracle under renamed exports, compares the complete program
including maps and refusal state, compares successful encodings, exercises
exact-liveness boundaries, and runs existing encoder fixtures and BF16 warp
simulation. `mutants.py` lowers altered Omega source with the frontend,
converter and flags named by that unit's checked `src/cudapp_om_<unit>.ident`
(`bridge-freest` for `regalloc`, `cg_life` and `cg_replay`; `bridge-seeded`
otherwise), verifies those pins and the checked C digest, relinks only that
unit's generated C and requires semantic failures; compile or link failure is
not counted as detection. `bridge-freest` also needs `OMEGA_SEED` (the
bridge's baseline compiler).

Pinned seeded compiler: omega-mv e5b32567, omc SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

See `evidence/regalloc-om/2026-11-06/README.md` and
`evidence/regalloc-om/2026-11-06-rest/README.md` for measured results and the
remaining unported assigned files. This is not a claim that all CUDApp CUDA
replacement work is complete.

The base/common-replay residue is now separate from the family emitters:
`cg_fields.om` owns instruction wire packing/decoding and little-endian words;
`cg_replay.om` owns metadata, width, operand and closure admission;
`cg_life.om` owns loop extension and exact hints; `cg_invar.om` owns
post-loop-use/redefinition policy; `cg_budget.om` owns the CTA register budget.
The single `omega_gpu_om_ir_replay` C entry remains the ABI/dispatch shell.
The old base computation is default-OFF
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_BASE`, without a C retry on Omega refusal.
See [the exhaustive function classification and host gate](../../tests/codegen_base/README.md),
[chip runbook](../../tests/codegen_base/CHIP.md), and
[dated host evidence](../../evidence/2026-11-06-codegen-base/README.md).
