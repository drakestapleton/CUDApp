<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# Final cubin differential follow-up

The original receipt proves raw IR and encoded instruction text equality. It did not compare final cubin containers. This follow-up closes that acceptance gap without rewriting the original logs.

`tests/api_kernel_gen_attention.c` now compares the complete solved `OmegaRegAlloc` structure, including uniform allocation and peak UGPR usage. Every successfully allocated spec must encode successfully on both paths and produce identical text. Both texts are then independently passed to `omega_cubin_pack`, with the four attention pointer arguments and eight scalar arguments at their source-defined constant-bank offsets, the existing full constant-bank capacity, the corresponding allocator's `omega_bw_gpr_declared(peak_gpr_usage)`, and the shape's existing shared-memory reservation. Full cubin lengths and every byte are compared. The cubin API has no separate UGPR metadata input; uniform allocation is compared in the full solved allocator and encoded text.

The existing perturbed-record red control remains. A second red control perturbs the final byte of the Omega-path cubin after both packs succeed; the target requires rc 1. All cubins remain in memory and are freed; no cubin binary or NVIDIA disassembly is committed.

The source partition is unchanged. Generator units, production cutover, shared registration, compiler pin and allocator/encoding/cubin implementations are unchanged.

Commands are run from the worktree under:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 \
  env OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc" \
      OMEGA_BRIDGE=/tmp/integ/bridge-73-79e8c36 \
      OMEGA_SEED="$HOME/work/cudastack/omega-bridge/build/bootstrap/omc" \
  make -j3 -f Makefile -f tests/attention_kgen.mk TARGET
```

`TARGET` is `test-api-kgen-attention` and then `test-api-kgen-attention-mutants`. Final full outputs are `differential-final.log` and `mutants.log` in this directory, captured verbatim through `tee` with shell `pipefail`. The parent's integrated gate remains separate. Physical chip qualification is NOT_RUN; the original README's authorization-gated chip runbook still applies.

The first extended differential returned rc 0 and reported `cases=39423 cubins=39392 records+allocation+encoded-bytes+cubins identical`, with both red controls detected. Its `differential.log` is preserved, including a libgcov warning about the prior fixture's profile checksum. The fixture object recipe now deletes its disposable `.gcda` when recompiling, and depends on its make fragment so changed fixture build settings regenerate that object.

## Observed final differential

The rebuilt fixture returned rc **0** with no stale-profile warning:

```text
ATTENTION-IR DIFFERENTIAL PASS cases=39423 cubins=39392 records+allocation+encoded-bytes+cubins identical
PASS attention red control detected perturbed IR record
PASS attention red control detected perturbed final cubin
```

Thus all **39,392** successfully allocated specs have full solved-allocation,
encoded-text and complete cubin equality; invalid input dispositions remain in
the **39,423** total cases. Each negative binary invocation returns **1**;
the enclosing make target returns **0** only when both are detected.

The standalone mutation target returned rc **0** with final line
`PASS attention seeded semantic mutants: 5 killed`. Its cubin-aware baseline
reported the same case/container counts. Descriptor offset, query offset,
thread special register, scalar argument offset and stall-control field
mutations each regenerated and compiled successfully, then returned rc **1**
with an IR record mismatch.

The parent now includes this make fragment from shared build wiring, while the
standalone receipt runner supplies it with `-f`. The fragment has an include
guard so those two routes can coexist without overriding recipes. The final
integration-shape command deliberately supplies the fragment twice, and uses
`-j1` to serialize its two goals:

```sh
# Under the identical bwrap/taskset/nice/env wrapper above:
make -j1 -f Makefile -f tests/attention_kgen.mk -f tests/attention_kgen.mk \
  test-api-kgen-attention test-api-kgen-attention-mutants
```

Its output is retained separately as `integration-include.log`.

## Final outcomes

`integration-include.log` returned rc **0**, without recipe-redefinition or
stale-profile warnings. Its differential baseline again reported
`cases=39423 cubins=39392 records+allocation+encoded-bytes+cubins identical`;
both independent red controls were detected, and its final line is
`PASS attention seeded semantic mutants: 5 killed`.
`rc.tsv` records commands, statuses and final lines for each acceptance receipt.

The following coverage command was run under the same device-hidden CPU
wrapper, with this evidence directory as cwd:

```sh
gcov --json-format -b -c -f \
  ~/work/cudastack/lanes/KernGenAttnOm/build/api-kgen/attention_diff.gcno
```

It returned rc **0**. `coverage.log` and `attention_diff.gcov.json.gz` retain
the output and machine-readable profile. Gcov reports `gen_attention` at
`100.00% of 312` lines and `gen_attention_general` at `100.00% of 176`;
`cubin_same` is at `100.00% of 11`. This query on that profile returned `[]`,
saved verbatim in `generator-uncovered-edges.json`:

```sh
gzip -dc attention_diff.gcov.json.gz |
  jq '[.files[].lines[] |
       select(.function_name=="gen_attention" or
              .function_name=="gen_attention_general") |
       select(any(.branches[]?; .count==0)) |
       {line:.line_number,function:.function_name}]'
```

The original README's defensive-helper reachability explanation remains
applicable. These percentages are gcov's measured output, not an estimate.
The final crumb command, under the same wrapper, was
`sh -c 'crumb compile . && crumb verify .'`; rc **0**, final lines:

```text
crumb compile: 46 crumbs, 0 rewritten, HEAD caad63b
crumb verify: OK: 46 crumbs current
```

| Tier | Verdict |
|---|---|
| Seeded generation and C-oracle differential, including full allocation/text/cubin | PASS |
| Deliberately perturbed IR record and final cubin | PASS: both refused with rc 1 |
| Seeded behavioral mutants | PASS: 5 compiled mutants killed |
| Standalone/shared-include build coexistence | PASS |
| Crumb compile/verify | PASS |
| Integrated host/simulator gate rerun after this follow-up | NOT_RUN by this lane; parent owns the integrated gate |
| Physical GB10 qualification | NOT_RUN: no authorized chip window |
| Merge/deployment | NOT_RUN |

This follow-up changes only the owned attention fixture, its make fragment and
evidence. The production cutover, seeded units, shared build wiring and replay
ABI are unchanged.
