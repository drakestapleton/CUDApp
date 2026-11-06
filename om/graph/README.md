# Seeded model graph recording

The default graph recorders use seeded Omega plans for scalar decode, Qwen3.6, Qwen3.8, batched Q36 heads, prefill mixer/expert ordering, MoE grouping/window bindings, output heads, and prefill launch/copy planning. `CUDAPP_MIGRATION_PENDING_C_GRAPHS=1` selects the retained C differential oracle in the four migrated translation units. It is not enabled by default.

`om/units.sh` composes the units and commits their portable C ABI bindings. The compiler is omega-mv e5b32567, omc SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. The final entry function of each composition is the bridge export. Plans use fixed-size ulong descriptor views; the shell copies packed uint argument words and calls existing CUDApp recorder APIs. It does not add an execution backend or GPU launcher.

The C shell retains tensor object observation, allocation/lifetime, model setup, environment/configuration admission, recorder dispatch, statistics, errors, queue execution, and existing closure/admission checks. Layout construction, debug readback copies and public standalone glue helpers have not all moved to Omega. Those remaining decisions require further migration; this is not a claim that every line in the four C files has been replaced.

## Host checks

Run inside the device-free namespace, never against live devices:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 CUDAPP_OMC_HOST=1 OMEGA_OMC=../omega-mv/seed/out/omc OMEGA_BRIDGE=../omega-bridge test-model-graphs-om
```

The wire test compares seven argument-bank formats with an independent C oracle. The command-list differential compiles the actual graph translation units with old/new selection, compares complete command-list argument banks, QMD slots, launch code, waits, independence and graph state. Nested GDN, attention, GEMM and router recorders are deterministic ABI adapters: their complete input arguments become launches in the real command list. This isolates the migrated graph decisions, but is **not** end-to-end parity of actual nested kernel recordings or real model weights.

Fixtures cover boundaries and generated sequence positions/configurations, scalar/batched heads, GDN/attention, Q38 BF16/split-down shapes, prefill multi-token paths, native MoE windows, combine and grouping. Record flips are rejected. `mutants.py` recompiles five semantic wire changes; compilation errors are not mutant kills. Fake-platform submission probes exercise one complete token's layer/final plans without device access.

Remaining acceptance includes actual nested-kernel end-to-end parity across supported production configurations, full prefill launch/copy planning differential, Q38 exact-head fixture coverage, a focused warp simulation receipt, and hardware qualification. See `CHIP.md`. These are NOT_RUN until a receipt says otherwise.
