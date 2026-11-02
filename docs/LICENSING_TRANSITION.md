# CUDA++: why Apache-2.0 WITH LLVM-exception, and how to use the MIT version

**First-party CUDA++ material in this revision is offered under Apache-2.0 WITH LLVM-exception**, except where a file carries a different explicit notice. The license applies to this revision whether obtained from a branch or a release. Earlier MIT grants remain valid; this change does not revoke them.

## Why I built it, and why the license changed

I built CUDA++ to give people an independently implemented path to their NVIDIA GPU without depending on NVIDIA's proprietary user-space compiler, runtime, driver library or math libraries. The goal was to remove that software dependency, not to distribute NVIDIA's proprietary implementation under my name.

The foundation came from my own implementation and interoperability research. I wrote probe kernels, inspected their outputs using vendor tools offline, compared one-variable changes, consulted public technical references and permissively licensed projects, and tested my own encodings and runtime on hardware. I acknowledge that reverse engineering openly. The method and source references are documented in [HOW_I_DID_IT.md](HOW_I_DID_IT.md); the scoped provenance review is in [LICENSE_PROVENANCE.md](LICENSE_PROVENANCE.md).

Understanding hardware behavior and writing my own implementation is different from taking proprietary source code. The documented provenance identifies my implementation work separately from the third-party material I actually include. I do not claim NVIDIA's open-source headers, Mesa's contributions, NVIDIA's firmware, or anyone else's work as mine. The audit records evidence and its limits, not a universal legal guarantee.

The MIT foundation remains available to everyone at the exact revision below. Going forward, my first-party improvements build on that foundation under Apache-2.0 WITH LLVM-exception. The LLVM exception lets a developer compile proprietary models or GPU kernels with CUDA++ and ship the resulting object code under terms of their choosing; it also resolves the GPLv2 patent/indemnity conflict at the kernel boundary. It does not license NVIDIA's firmware, the vendor driver, or third-party notices.

This is not a claim that every NVIDIA component has been replaced: the documented working path still uses NVIDIA's open kernel modules and signed firmware. Omega is relicensed under the same terms in its own repository.

## What remains under its original terms

- NVIDIA's open-kernel-module interface headers retain their MIT notices in `third_party/nvidia-open-580.173.02/`.
- Mesa/NVK/NAK hardware facts and references retain their attribution in [THIRD_PARTY.md](../THIRD_PARTY.md). The historical audit reports no copied Mesa source code.
- Attributed quotations remain third-party text, not my newly authored work.
- The first-party AGPL-3.0-only transition is complete as of 2026-11-02. Drake Stapleton decided to relicense those files to Apache-2.0 WITH LLVM-exception. Before the edit, `git grep -l 'SPDX-License-Identifier: AGPL-3.0-only' -- ':!evidence' ':!third_party'` listed 57 files. CUDApp#89 had counted 48 on 2026-10-27 (`tests/learn53` 9, `native/learn53` 5, and the other directories below at the same counts). Nine files were added after that count: `tests/learn53` grew from 9 to 17 and `native/learn53` from 5 to 6. Each of the 57 SPDX lines changed from `AGPL-3.0-only` to `Apache-2.0 WITH LLVM-exception`. Those 57 files had no copyright line. The same edit normalized 26 first-party SPDX lines that said plain `Apache-2.0` to `Apache-2.0 WITH LLVM-exception` and left their copyright lines in place. The pull request URL is added on this branch after the pull request is opened. Directory counts for the 57: `tests/learn53` 17, `tests/speculation` 10, `tests/host-trace` 9, `native/learn53` 6, `native/speculation` 5, `native/blackwell` 3, `native/trace` 2, `native/gsp` 1, `native/nvrm` 1, `tests/device-trace` 1, `tests/gsp` 1, `tools/omtrace` 1.
- Receipts under `evidence/` keep the terms they were recorded with and are not edited. 33 receipt files still contain a historical `SPDX-License-Identifier: AGPL-3.0-only` line. `evidence/LEARN53-L777-CONTINUATION/{author,review}/omega-core.om` carry no licence header.
- `native/learn53/omega653.lock` still records `"license": "AGPL-3.0"` for the pinned omega-mv object `84d03c5523f39ca1045d1c313c5f62597779462b` at `src/learning/benchmark/core.om`. That field is the upstream object's licence at that revision, not a CUDA++ SPDX header. `docs/learn53/CATALOG.md` and `docs/learn53/CONTINUATION-PLAN.md` describe that same upstream LICENSE. Those three texts were not changed.
- `CITATION.cff` keeps `license: "Apache-2.0"`. The Citation File Format accepts SPDX license identifiers only, and the file's comment says `LICENSE` is authoritative for the LLVM exception.
- `tools/localrouter/setup.py`, `tests/localrouter/test_setup.py`, and `tests/localrouter/negative_control.py` keep `SPDX-License-Identifier: MIT`. They were neither AGPL nor plain Apache-2.0.
- The original first-party MIT license can be read in [LICENSE at the pinned MIT revision](https://github.com/aien-dev/CUDApp/blob/232332422bfdc31c8918085b50f13c1e18ab702e/LICENSE). That prior grant remains effective for material covered by it; it is not a blanket MIT option for later Apache-2.0 WITH LLVM-exception contributions.

## Files changed on 2026-11-02

AGPL-3.0-only SPDX headers, 57 files:

```text
native/blackwell/launch.om
native/blackwell/trace-emit.om
native/blackwell/vector.om
native/gsp/fwimage.om
native/learn53/catalog.om
native/learn53/moe-closure.om
native/learn53/omega-adapter.om
native/learn53/stream.om
native/learn53/task.om
native/learn53/wire.om
native/nvrm/context.om
native/speculation/costs.om
native/speculation/policy.om
native/speculation/replay.om
native/speculation/report.om
native/speculation/service.om
native/trace/htlaunch.om
native/trace/htrec.om
tests/device-trace/emitter-test.om
tests/gsp/run-fwimage-host.sh
tests/host-trace/badfrag.om
tests/host-trace/fakedev.om
tests/host-trace/fakenv.om
tests/host-trace/launch-test.om
tests/host-trace/lossy.om
tests/host-trace/overhead-absent.om
tests/host-trace/overhead-traced.om
tests/host-trace/overhead.om
tests/host-trace/recorder-test.om
tests/learn53/catalog-check.om
tests/learn53/catalog-export.om
tests/learn53/catalog-fixtures.om
tests/learn53/catalog-unit.om
tests/learn53/fixtures.om
tests/learn53/moe-export.om
tests/learn53/moe-fixtures.om
tests/learn53/moe-observe-export.om
tests/learn53/moe-observe-fixtures.om
tests/learn53/moe-observe-unit.om
tests/learn53/moe-series-export.om
tests/learn53/moe-series-unit.om
tests/learn53/moe-unit.om
tests/learn53/omega-unit.om
tests/learn53/stream-check.om
tests/learn53/stream-fixtures.om
tests/learn53/stream-input.om
tests/speculation/cost-test.om
tests/speculation/oracle-test.om
tests/speculation/packet-fixture.om
tests/speculation/policy-test.om
tests/speculation/reply-test.om
tests/speculation/report-test.om
tests/speculation/run-teardown-host.sh
tests/speculation/service-test.om
tests/speculation/support.om
tests/speculation/teardown-host-test.om
tools/omtrace/omtrace.om
```

Plain `Apache-2.0` SPDX lines normalized to `Apache-2.0 WITH LLVM-exception`, 26 files:

```text
src/omega_blackwell_codegen.h
src/omega_canonical.c
src/omega_canonical.h
src/omega_core.c
src/omega_core.h
src/omega_numeric.c
src/omega_numeric.h
src/omega_numeric_lifecycle.h
src/omega_numeric_reduce.c
src/omega_numeric_reduce.h
src/omega_path_check.h
src/omega_types.h
src/omega_vector.c
src/omega_vector.h
src/runtime/rx_costmodel.c
src/runtime/rx_costmodel.h
src/sha256.c
src/sha256.h
tests/bw_warp_sim.h
tests/bw_warp_sim_k.h
tests/bw_warp_sim_pa.h
tests/bw_warp_sim_tc.h
tests/omega/run.sh
tests/runtime/rx_costmodel_unit.c
tests/test_omega_gpu_decode_attn_maxpos.c
tools/mutation_runner.sh
```

## Get the MIT foundation

Use this exact historical commit:

`232332422bfdc31c8918085b50f13c1e18ab702e`

These checkout commands were tested against the public repository:

```sh
git clone --no-checkout https://github.com/aien-dev/CUDApp.git CUDApp-mit-snapshot
cd CUDApp-mit-snapshot
git fetch origin main
git checkout --detach 232332422bfdc31c8918085b50f13c1e18ab702e
git rev-parse HEAD
head -1 LICENSE
```

The final commands show that exact commit and `MIT License`. To develop your own fork from this foundation, create a branch there:

```sh
git switch -c my-mit-development 232332422bfdc31c8918085b50f13c1e18ab702e
```

You can push that branch to a repository you control. GitHub's fork button alone does not select the historical version: explicitly check out the pinned commit. This is an existing MIT snapshot, not a claim that I created a separate hosted fork or that it was the final MIT commit.

Keep the MIT license and third-party notices when redistributing that version. Later Apache-2.0 WITH LLVM-exception contributions do not become MIT merely because you copy them onto this branch. The terms attached to the material you actually use continue to apply.
