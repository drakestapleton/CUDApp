# Seeded decode and tensor-prefill attention builders

## Scope and source

Branch: `feat/omega-codegen-attn`, stacked on CodegenB `f219a04eff8ff28e2d8d0b8cd65f210e552bbd54`.
Implementation plus final controls: `2dd498f`; draft PR: <https://github.com/aien-dev/CUDApp/pull/277>.
Registration-only first commit: `4b15a9b`. The separately authorized Makefile comment correction is `860404e`.

`cg_attn{,_emit}.om` and `cg_pattn{,_emit}.om` own the variable loops, instruction and operand records, scoreboard state, branch patches, loop metadata, and RMSNorm/RoPE prologue. The existing public APIs and headers are unchanged. The default C adapters replay those units through `omega_gpu_om_ir_replay`. The original builders remain separately compiled differential oracles behind default-off `CUDAPP_MIGRATION_PENDING_C_CODEGEN_ATTN` and `CUDAPP_MIGRATION_PENDING_C_CODEGEN_PATTN`.

Compiler: omega-mv `e5b32567`, omc SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Bridge converter SHA-256: `69d629df170a24ba0c2cb6d41dbfaed520e248e0d723236dd32abf0581524d84`.
Bridge frontend SHA-256: `23ff7a2356bb5d980060a4685a27ac46587013069b7d24b37c2873d7ee659b37`.
The committed `.ident` files bind the exact sources and generated C. No device words or newer context compiler are used by these units.

## Complete accepted-spec differential: PASS

The public guards define a finite accepted set. The test enumerates decode depth 1–4 and mutant 0–4; prefill warps 4 or 8 and mutant 0–4, with FUSED/APPEND/ATTEND and mirror permitted only for APPEND. Runtime sequence lengths and addresses are launch parameters, not builder configurations. Every accepted spec is covered, not a sampled boundary set. Invalid boundaries, extreme integer inputs, null spec, and null output are also exercised.

For every accepted spec, `tests/codegen_attn_diff.c` compares the complete IR records, register allocation, and full padded encoded bytes. It then changes one record and requires rejection, and invokes an Omega scheduling mutant that changes the first load's stall and requires rejection. The production adapters always select the unmutated root kind.

Command (all build/run commands below use `taskset -c 10-14 nice -n 10`):

```sh
OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc \
OMEGA_BRIDGE=~/work/cudastack/omega-bridge sh om/units.sh
make -j3 test-codegen-attn build/gpu_decode_attn_test build/gpu_prefill_attn_test
```

Both commands returned 0. Generation log: `final-units.log`. Focused build/test log: `final-focused.log`.
Final differential line:

```text
PASS attention differential cases=378 accepted=60 record-negative-controls=60 Omega-mutants-killed=60 IR+allocation+encoded-bytes
```

The earlier `focused.log` is a retained failed development attempt: the changed K_VAR scheduling incorrectly preserved pending bits for a first variable-latency load. It failed at decode record 2. The corrected emitter preserves pending bits only for K_VAR_MORE, matching the C scheduling distinction. The final exhaustive differential above passed after regeneration. `units.log` belongs to that earlier attempt, not the final result.

## Existing host and simulator paths: PASS

These four commands each returned 0, in this order:

| Command | Log | Final line |
| --- | --- | --- |
| `build/gpu_decode_attn_test --host-only` | `decode-host.log` | `gpu_decode_attn_test: 43 checks, 0 failed: PASS` |
| `build/gpu_decode_attn_test --sim` | `decode-sim.log` | `gpu_decode_attn_test: 12 checks, 0 failed: PASS` |
| `build/gpu_prefill_attn_test --host-only` | `prefill-host.log` | `gpu_prefill_attn_test: 69 checks, 0 failed: PASS` |
| `build/gpu_prefill_attn_test --sim` | `prefill-sim.log` | `gpu_prefill_attn_test: 97 checks, 0 failed: PASS` |

The simulator logs include caught numerical mutants: decode mutants 1–4; prefill mutants 1–4 on fused, split, and mirror paths. These are host simulator results, not hardware qualification. Numeric values in these logs are measured outputs from the existing simulator tests. Encoding-constant conversions through Wolfram are recorded separately in `wolfram.txt`.

## Regeneration check and complete host suite: PASS

The serialized final acceptance command returned 0:

```sh
taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge \
  sh om/units.sh --check
taskset -c 10-14 nice -n 10 env \
  OMEGA_OMC=/home/drakestapleton/work/cudastack/omega-mv/seed/out/omc \
  OMEGA_BRIDGE=/home/drakestapleton/work/cudastack/omega-bridge \
  OMEGA_MV=/home/drakestapleton/work/cudastack/omega-mv \
  OMEGA_BRIDGE_DIR=/home/drakestapleton/work/cudastack/omega-bridge \
  make -j3 test
```

The commands were chained with `&&` under `set -o pipefail`, each piped through `tee` into its named log. `units-check.log` ends with `PASS portable Omega units regenerate byte for byte`. `full-host.log` ends with:

```text
CUDA++ HOST TESTS PASS (90 targets)
```

The tool recorded wall time 5935.68 seconds for the combined command. This run used no deadline, was not retried, and was not interrupted by the cross-lane `make -j5 test` process-pattern kill. Complete tool output is also retained as `artifact://2162`. This is a private-branch host gate, not a gate for an integrated branch or a hardware receipt.

The committed `full-host.log` has five explicitly marked redactions of nvdisasm instruction listings, which repository policy prohibits committing. No result line is changed. The unredacted output remains in the tool artifact above, not in Git.

## Chip tier: NOT_RUN

No GPU window is authorized. No chip launch, service transition, module change, lock manipulation, or hardware qualification is part of this receipt. MERGED and DEPLOYED are also NOT_RUN; the PR remains draft.

For a future explicitly authorized exclusive GB10 window, first follow the current service-safety, authorization, and hardware runbook. The actual binaries (not the stale nonexistent shell-script names) are:

```sh
build/gpu_decode_attn_test --out decode.json
build/gpu_prefill_attn_test --out prefill.json
```

Both commands open the GPU when neither `--host-only` nor `--sim` is specified. These commands are a future chip runbook, NOT commands exercised by this migration. Preserve their new receipts separately and complete the window's required closeout before reporting hardware qualification.

## Ownership and guide impact

Open PRs were checked before source work; no open PR owned the assigned attn/pattn files. Before the authorized Makefile comment-only change, all open Makefile PR diffs were inspected; none changed those exact comments. CodegenA owns the shared `om/codegen/.crumb`; it approved the disjoint attention unit files and retains that shared claim. CodegenTc keeps its public helper API unchanged; no shared Omega emitter interface was required. The separate `om/attn-any` lane is untouched.

`docs/OMEGA_HOST_TESTS.md` documents the migration, accepted space, commands, pins, and chip boundary. No Omega language, compiler, bridge ABI, harness pin, gate, repository ownership, or hardware status was changed; no omega-harness skill update is required for these builder implementations.
