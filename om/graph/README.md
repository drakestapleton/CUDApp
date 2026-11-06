# Seeded model graph recording

The default graph recorders use seeded Omega plans for scalar decode, Qwen3.6, Qwen3.8, batched Q36 heads, prefill mixer/expert ordering, MoE grouping/window bindings, output heads, allocation region offsets/sizes, standalone residual/SwiGLU glue, debug-copy launch records, and prefill launch/copy planning. `CUDAPP_MIGRATION_PENDING_C_GRAPHS=1` selects the retained C differential oracle in the four migrated translation units. It is not enabled by default.

`om/units.sh` composes the units and commits their portable C ABI bindings. The compiler is omega-mv e5b32567, omc SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. The final entry function of each composition is the bridge export. Plans use fixed-size ulong descriptor views; the shell copies packed uint argument words and calls existing CUDApp recorder APIs. It does not add an execution backend or GPU launcher.

The C shell retains tensor object observation, allocation/lifetime, recorder dispatch, statistics, errors, queue execution, and existing closure/admission API calls. Slice 8 moves the remaining Q38 dimensional defaults and derived widths, model scalar configuration/shape checks, layer classification and counters, rotary/FP8 gather tables, decode barrier normalization and timeline admission, prefill register-budget admission and allocation rounding into seeded Omega. Pure allocation layout and debug-copy/standalone glue descriptors are Omega units. Pointer/null/lifetime checks around host objects remain C; this is not a claim that every line in the four C files has been replaced.

## Host checks

Run inside the device-free namespace, never against live devices:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 CUDAPP_OMC_HOST=1 OMEGA_OMC=../omega-mv/seed/out/omc OMEGA_BRIDGE=../omega-bridge test-model-graphs-om
```

The wire test compares seven argument-bank formats with an independent C oracle. The command-list differential compiles the actual graph translation units with old/new selection, compares complete command-list argument banks, QMD slots, launch code, waits, independence and graph state. Nested GDN, attention, GEMM and router recorders are deterministic ABI adapters: their complete input arguments become launches in the real command list. This isolates the migrated graph decisions, but is **not** end-to-end parity of actual nested kernel recordings or real model weights.

Fixtures cover boundaries and generated sequence positions/configurations, scalar/batched heads, GDN/attention, Q38 BF16/split-down shapes and exact-head records, prefill multi-token paths, native MoE windows, combine, grouping and standalone glue. Independent layout/copy/transfer/launch checks compare generated descriptors with C expressions. Record flips are rejected. `mutants.py` recompiles five semantic wire changes; compilation errors are not mutant kills. Fake-platform submission probes exercise one complete token's layer/final plans without device access.

`make test-model-graphs-warp` interprets the actual GATHER glue program with Omega-packed arguments in both warp orders, and rejects its source-index mutant. This is simulator evidence, not hardware qualification.

`make test-model-graphs-nested` independently compiles old/new graph objects and invokes unwrapped production GDN, attention, FP8/BF16 GEMV, native GEMM and router recorders with synthetic resource observations and fake platform/RM. It compares every bank/QMD and submitted word, requires one submission per complete word, and kills a changed real plan descriptor. Q36 native MoE windows use actual seeded staged artifacts, temporary cubin export/snapshot admission and real leases; these device-free host paths are exercised. Weight/state contents are not kernel-executed. This complements, rather than replaces, the larger ABI-adapter randomized differential. Hardware qualification and the integrated full suite remain NOT_RUN. See `CHIP.md` and the dated receipt.

## Slice 8: graph residue and hello

`residue.om` exports scalar metadata decisions over fixed descriptors;
`table.om` produces rotary/gather permutation words. The independent scalar
oracle is selected only with default-OFF
`CUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE=1`. Existing #310 C graph oracles remain
separately selectable. `hello.om` and `hello-plan.om` cover hello input/output
checking and host plans; see [the hello runbook](../../examples/hello_gpu/README.md).

`test-graph-residue` compares deterministic output against the C oracle, with a
deliberately corrupted result as a red check. Cases follow constructor branches:
dimension defaults, divisibility and capacity limits; vocabulary and layer/head
limits; mask bits and both layer classes; all legal gather layouts; barrier
values; timeline empty/full/overflow; explicit/default register budgets; full,
short, poisoned and bit-flipped output; all launch chunks and both bug modes.
Dimension and scalar rejection cases include 0, UINT32_MAX and adjacent source
boundaries. Non-NaN float output is compared bit-exact with contraction disabled.

`test-graph-residue-mutants` compiles seven changed Omega units and requires
behavioral disagreement: poison, input period, checker add/sub, dimension cap,
timeline capacity, gather stride and load scoreboard. A compiler refusal is a
failed gate, never a mutant kill.

`test-graph-residue-coverage` instruments the C oracle at O0 and runs the actual
hello oracle simulator plus both fault controls. The scalar header has every
executable line and branch reached. Its untaken `!in` branch is unreachable
after zero selects the nonzero default. ABI-status/abort branches in the glue
are deliberately not invoked with forged pointers or lengths. The hello oracle's
remaining uncovered branches are the device-only main path and device error
cleanup (NOT_RUN: no GPU window), allocation/IR-append failure handling (not
injected), CLI usage/invalid-option I/O, alternate diagnostic opcode strings and
nearest-buffer ties/ranges not generated by the vecadd fault. These are retained
host/device glue, not untested alternate C computation selected in production.
The differential separately exercises bandwidth target/positive-time branches
and wrong-answer checking that the normal/past-end/short simulator does not.
Warp-simulator opcode/MUFU branches unused by vecadd belong to the simulator's
own tests; coverage here does not qualify them.

Q36 formerly silently truncated a full timeline; Q38 could copy past its fixed
stamp array. Both now reject a batch that cannot fit before copying any stamps.
The shared capacity regression checks the last fitting batch and the first
overflow, including UINT32_MAX count. Failure leaves the output index unchanged.

### Runtime table view pin (pending upstream)

The converter/frontend pins in `om/units.sh` select omega-bridge #73
`79e8c3613a6f`, stacked on unmerged #71. This CUDApp change is pending both PRs.
Only `model_graph_table` selects `bridge-seeded --runtime-views`. Its numeric
`edit uint 24576` is a capacity: the actual allocation length is passed to the
checked entry, so Omega writes packed uint32 values directly. The old fixed-512
scratch buffer, widening/narrowing copy and C chunk loop are removed. All legal
Q/KV layouts are compared up to the maximum extent, including q=kv=24; admission
controls cover empty/null, over-capacity, nonempty-null and result/view overlap.
WolframLanguageEvaluator: `(24+24)*512` -> `24576`.

This is the hosted checked converter profile, not native/freestanding. The
existing freestanding units and pins retain their original path. Build the
bridge generator with its existing pinned bootstrap seed using `tools/build.sh`;
set `OMEGA_SEED` to that baseline executable when a separate bridge worktree
does not contain `build/bootstrap/omc`. The seeded source validator remains
e5b32567 through `OMEGA_OMC`. Re-run `om/units.sh --check` with the same bridge,
seed and validator. Every hosted generated file/identity is refreshed because
the converter pin changes; integration must regenerate the union of lane units.
