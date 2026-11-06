# DEC kernels from seeded Omega: chip status

`om/codegen/cg_dec.om` (with `cg_dec_emit.om`) builds the eight decode glue kernels of
`src/omega_blackwell_codegen_dec.h` (AXPY, RES, SWIGLU, ARGMAX, GATHER, GATE, GROUP, XBOUND).
`omega_blackwell_codegen_dec_prog` replays the unit's IR records through `omega_gpu_om_ir_replay`.
The handwritten C builder remains in the source behind the default-off switch
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_DEC` and serves as the differential oracle.

| Tier | Target | Status |
|---|---|---|
| IR, allocation and encoded bytes equal the C builder | `make test-codegen-dec-diff` | EXERCISED on the host |
| Recompiled Omega mutants killed | `make test-dec-mutants` | EXERCISED on the host |
| SIMT warp simulator against host references | `make test-codegen-dec-sim` | EXERCISED on the host |
| GB10 chip run of the Omega-built kernels | `make test-gpu-decode-graph` with the chip runner, in an authorized exclusive window | NOT_RUN |

The encoded bytes are identical to the C builder's for every accepted spec in the differential's partition, so
existing chip receipts for the C builder apply to the same bytes. That is an inference, not a chip
qualification. A chip tier for this unit is NOT_RUN until it runs in an authorized GB10 window.
