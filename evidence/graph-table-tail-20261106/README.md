# Graph rotary-tail repair

Base: #322 b459d248e20f. The existing q38 host gate was run before editing in a fresh worktree with an empty build directory. It failed four of nine checks: the 24/4 and 16/4 gather tables had incorrect q/k tails and repeated indices. This establishes the defect independently of head-scale work.

Root cause: `grperm` returned `r + i - r/2` in the nonrotary second-half tail. The source C oracle, `omega_dattn256_perm_slot` in src/omega_gpu_decode_attn256_api.h, returns i. The repair restores that branch; no interface change.

## Commands and results

All tests used `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.

- Before edit: `make -j3 test-gpu-q38-graph-host OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/tmp/bridge-73-apiviews`: rc 2, `gpu_q38_graph_test (host-only): 9 checks, 4 failed: FAIL`. No unit regeneration occurs in this target; it used the committed generated C from #322. Artifact: artifact://9265.
- Regeneration: `OMEGA_SEED=~/work/cudastack/omega-bridge/build/bootstrap/omc OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/tmp/bridge-73-apiviews sh om/units.sh`: rc 0. Unchanged compiler and converter pins from #322.
- After edit: `make -j3 test-gpu-q38-graph-host`: rc 0, `gpu_q38_graph_test (host-only): 9 checks, 0 failed: PASS`. Artifact: artifact://9274.
- `OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/tmp/bridge-73-apiviews python3 om/graph/table-mutants.py`: rc 0, `PASS graph table five compiled mutants`. Killed original-tail, tail-zero, tail-plus-one, pair-boundary, rotary-half. The original-tail mutant reinstates the exact defect.

The regression checks every table entry for both model shapes against an independently written C permutation oracle, including the rotary/nonrotary boundary.

## Reachability

q38 IS reachable from `make test`: Makefile HOST_TESTS includes CARRIED_HOST_TESTS, mk/carried-tests.mk line 69 registers test-gpu-q38-graph-host and its recipe uses --host-only. The q36 graph path is covered by test-model-graphs-om and test-model-graphs-nested in mk/model-graphs.mk; there is no separately named q36 host target in this base. This repair does not explain a previous integrated full-gate PASS; inspect that gate's exact integrated inputs rather than infer omission.

## Hardware runbook

Chip NOT_RUN, no authorized GPU window. With a future explicit window, execute the q38 chip graph test on both supported shapes with pinned model/weight inputs; compare baseline and command-list paths and record in a new receipt. Host permutation repair is not chip qualification.

IMPLEMENTED and EXERCISED host-only. Not MERGED or DEPLOYED. No historical receipt is changed.
