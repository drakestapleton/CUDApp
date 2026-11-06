# Graph head scaling migration

IMPLEMENTED on base b459d248e20f (#322). No graph construction changes.
Compiler: omega-mv e5b32567, sha256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f.
Converter: the unchanged om/units.sh pin, matching /tmp/bridge-73-apiviews.

Both read APIs strongly call omega_ghscale over the complete hidden vector.
The C multiplication is retained only behind default-OFF
CUDAPP_MIGRATION_PENDING_C_GRAPH_HEAD_SCALE. Public ABI is unchanged.

## Host evidence

All executions below used
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.

- `make -j3 test-graph-headscale OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/tmp/bridge-73-apiviews`: rc 0. Final line `PASS graph head scaling red control and six mutants`.
- Float differential: 4096 exponent/sign/mantissa-class cases, a complete 4096-element vector, empty vector. Non-NaNs compare bit-exact; NaNs compare class. Includes signed zero, both infinities, subnormals, signaling/quiet NaNs and rounding-boundary mantissas. `-ffp-contract=off`.
- Strong binding removal: linker refused missing omega_ghscale.
- Killed compiled Omega mutants: identity, zero, negative, half, skip, add. Identity is the explicit red control.
- `make -j3 test-gpu-q38-graph-host test-model-graphs-warp test-model-graphs-om` with explicit compiler/bridge/seed paths: rc 2. The q38 host test reports four gather-table failures (9 checks). This is outside head-scaling scope: the unchanged #322 grgslot table has repeated/out-of-range indices. Reported to integration owner with artifact://9246. Warp, wire, plans, command-list differential and five existing model-graph mutants PASS in the same run. Graph family acceptance is FAIL until its owner repairs the base.
- An earlier command omitted compiler paths: rc 2, existing mutant setup NOT_RUN. This is not counted as behavior evidence.

WolframLanguageEvaluator: `{127-64,(127-64)*2^23,2^-64}` -> `{63,528482304,1/18446744073709551616}`. The source multiplier is the original C power-of-two literal.

## Chip runbook — NOT_RUN

No GPU window exists. After explicit exclusive-window authorization, build the graph chip tests at the integrated revision, run their normal q36/q38 model fixtures with FP8 head enabled, compare returned hidden vectors with the default-OFF C oracle, and exercise hidden=NULL and non-FP8 head paths. Record exact source/compiler/model pins and commands in a new receipt directory. Do not infer hardware qualification from host floats or the warp simulator.

MERGED: no. DEPLOYED: no. Integration/full suite and harness guide refresh belong to the parent.
