<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# Final bridge #73 adoption receipt

The earlier receipt preserves the pre-adoption observations. This additive
receipt covers the final runtime-extent table implementation and final pins.

- Compiler: frozen omega-mv e5b32567; omc SHA256
  2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f.
- Bridge: draft #73, 79e8c3613a6f, pending #71 and #73 merge.
- toc.om SHA256 d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538.
- bridge-seeded SHA256 a3a455bd77381e132215924295763e8728889d7d578278ae376de78324be8629.
- Unchanged baseline bridge bootstrap seed SHA256
  191b776ef98821597c3e30655da6d680a9b66edbc9ceaca3c39c61ea6df098e9;
  this builds the converter, not the admitted CUDApp source validator.
- Pin-only commit: 756452c. Separate timeline-repair commit: 7eb4b77.

`model_graph_table` now uses a bounded runtime `uint` view with the actual live
allocation count. It writes directly: no C chunk dispatch, scratch conversion
loop or widening allocation. The fixed declaration is the source-derived maximum,
not a requirement to pass the capacity as the length. Empty/null is admitted;
over-capacity, nonempty-null and result/view overlap refuse status 32 without
changing the result. Every legal gather layout is checked against independent C
formulas, including the maximum q=kv=24 case.

WolframLanguageEvaluator: `(24+24)*512` -> `24576` elements.

All executions below used this complete prefix:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

Environment (home prefix normalized):

```
OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc
OMEGA_BRIDGE=~/work/cudastack/lanes/GraphsHello-bridge73
OMEGA_SEED=~/work/cudastack/omega-bridge/build/bootstrap/omc
```

Commands and observed results:

1. `make -j3 test-graph-residue-coverage test-graph-residue-mutants test-hello-sim test-gpu-decode-graph test-model-graphs-om test-model-graphs-warp test-model-graphs-nested test-gpu-moe-window`: rc0; final line `MODEL-GRAPHS mutants PASS killed=5 compiled=5`. See runtime-final.log. The expected mutant FAIL lines are red controls, not unhandled failures.
2. `sh om/units.sh --check`: rc0; final line `PASS portable Omega units regenerate byte for byte`. See runtime-boundaries.log.
3. `make -j3 test-graph-residue-coverage test-graph-residue-mutants`: rc0 after adding maximum-view and refusal cases; final line `PASS graph residue: 7 compiled semantic mutants killed`. See runtime-boundaries.log. gcov replaced an older checksum's accumulated profile during this run; no failure was suppressed.
4. Remove only the two owned generated .gcda files, then `make -j3 test-graph-residue-coverage`: rc0, final line `Lines executed:62.01% of 437`. See runtime-fresh-coverage.log. This fresh run has no stale profile merge. Scalar oracle and hello simulator annotations are filed separately; coverage percentages are gcov measurements, not modeled values.

PASS: host graph/decode admission, model list/plan differential and negative
controls, real nested recorder differential, warp simulation, hello simulation,
C-vs-Omega differential/red check, runtime-view refusals, seven compiled Omega
mutants, byte-identical regeneration, gcov collection.

NOT_RUN: full integrated make test (Integrator3 owns it), chip, live RM, VFIO.
No GPU/device qualification, merge or deployment is claimed. Hosted runtime views
do not change any freestanding unit's profile. The chip runbook remains
examples/hello_gpu/README.md.
