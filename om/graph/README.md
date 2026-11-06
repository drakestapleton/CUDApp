# Seeded model graph recording

The default graph recorders use seeded Omega plans for scalar decode, Qwen3.6, Qwen3.8, batched Q36 heads, prefill mixer/expert ordering, MoE grouping/window bindings, output heads, allocation region offsets/sizes, standalone residual/SwiGLU glue, debug-copy launch records, and prefill launch/copy planning. `CUDAPP_MIGRATION_PENDING_C_GRAPHS=1` selects the retained C differential oracle in the four migrated translation units. It is not enabled by default.

`om/units.sh` composes the units and commits their portable C ABI bindings. The compiler is omega-mv e5b32567, omc SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. The final entry function of each composition is the bridge export. Plans use fixed-size ulong descriptor views; the shell copies packed uint argument words and calls existing CUDApp recorder APIs. It does not add an execution backend or GPU launcher.

The C shell retains tensor object observation, allocation/lifetime, model setup and shape metadata, environment/configuration admission, recorder dispatch, statistics, errors, queue execution, and existing closure/admission checks. Pure allocation layout and debug-copy/standalone glue descriptors are now Omega units. This is not a claim that every line in the four C files has been replaced.

## Host checks

Run inside the device-free namespace, never against live devices:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 CUDAPP_OMC_HOST=1 OMEGA_OMC=../omega-mv/seed/out/omc OMEGA_BRIDGE=../omega-bridge test-model-graphs-om
```

The wire test compares seven argument-bank formats with an independent C oracle. The command-list differential compiles the actual graph translation units with old/new selection, compares complete command-list argument banks, QMD slots, launch code, waits, independence and graph state. Nested GDN, attention, GEMM and router recorders are deterministic ABI adapters: their complete input arguments become launches in the real command list. This isolates the migrated graph decisions, but is **not** end-to-end parity of actual nested kernel recordings or real model weights.

Fixtures cover boundaries and generated sequence positions/configurations, scalar/batched heads, GDN/attention, Q38 BF16/split-down shapes and exact-head records, prefill multi-token paths, native MoE windows, combine, grouping and standalone glue. Independent layout/copy/transfer/launch checks compare generated descriptors with C expressions. Record flips are rejected. `mutants.py` recompiles five semantic wire changes; compilation errors are not mutant kills. Fake-platform submission probes exercise one complete token's layer/final plans without device access.

`make test-model-graphs-warp` interprets the actual GATHER glue program with Omega-packed arguments in both warp orders, and rejects its source-index mutant. This is simulator evidence, not hardware qualification.

`make test-model-graphs-nested` independently compiles old/new graph objects and invokes unwrapped production GDN, attention, FP8/BF16 GEMV, native GEMM and router recorders with synthetic resource observations and fake platform/RM. It compares every bank/QMD and submitted word, requires one submission per complete word, and kills a changed real plan descriptor. Q36 native MoE windows use actual seeded staged artifacts, temporary cubin export/snapshot admission and real leases; these device-free host paths are exercised. Weight/state contents are not kernel-executed. This complements, rather than replaces, the larger ABI-adapter randomized differential. Hardware qualification and the integrated full suite remain NOT_RUN. See `CHIP.md` and the dated receipt.
