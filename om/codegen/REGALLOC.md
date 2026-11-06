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

## Aggregate integration

Integrator owns the single hosted/freestanding aggregate and its generated public
headers. Register these eleven units there, rather than creating a second
production aggregate:

| Source under om/codegen | Native export |
| --- | --- |
| regalloc.om | omega2_ra_run |
| regalloc_ir.om | omega2_ra_ir |
| regalloc_live.om | omega2_ra_live |
| cg_matmul.om | omega2_cg_matmul |
| cg_matmul_params.om | omega2_cg_par |
| cg_matmul_i32.om | omega2_cg_i32 |
| cg_matmul_tensor.om | omega2_cg_tens |
| cg_matmul_loop.om | omega2_cg_loop |
| cg_matmul_frag.om | omega2_cg_frag |
| cg_matmul_block.om | omega2_cg_block |
| cg_vecadd.om | omega2_cg_vec |

`tests/regalloc_om/run.sh` creates only a private test aggregate. It compiles the
C oracle under renamed exports, compares the complete program including maps
and refusal state, compares successful encodings, exercises exact-liveness
boundaries, and runs existing encoder fixtures and BF16 warp simulation.
`mutants.py` recompiles altered Omega source and requires semantic failures;
compile failure is not counted as detection.

Pinned seeded compiler: omega-mv e5b32567, omc SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
The frozen bridge bc11b30b supports the hosted OM2 route. Its freestanding route
refuses OM2 arrays with rc110. BridgeHost owns the explicit freestanding OM2
array extension; this branch does not bypass that refusal or silently use a
hosted runtime for freestanding execution.

See `evidence/regalloc-om/2026-11-06/README.md` for measured results and the
remaining unported assigned files. This is not a claim that all CUDApp CUDA
replacement work is complete.
