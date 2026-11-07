<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# NVML subset host tier, refresh against main and #280, 2026-11-07

Branch `feat/omega-nvml-replacement` (CUDApp #365) at 27e0b93d, base `main`
b774723f (main has not moved since the branch was cut, so no rebase).
27e0b93d takes #280's `tests/nvml/run.sh` byte for byte: the bridge-seeded
frontend pin is now read from `om/units.sh` instead of a second copy.
Compiler: omc at omega-mv e5b32567, sha256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Bridges built from the fingerprinted seed (sha256 191b776e…) with
`tools/build.sh`: bc11b30b (bridge-seeded 23ff7a23…, the main pin) and
d90433a9 (bridge-seeded a3a455bd…, the #280 pin). Every command ran in a
device-free namespace on cores 10-11 under the pool-10-11 lock:

```sh
B="bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10,11 nice -n 10"
$B make test-nvml CUDAPP_OMC_HOST=1 OMEGA_OMC=$OMC OMEGA_BRIDGE=$BRIDGE
$B env OMEGA_OMC=$OMC OMEGA_BRIDGE=$BRIDGE sh om/units.sh --check
```

The full command list is `gates.sh`/`run3.sh` in the card workspace; the rc
and last line of each command are in `head/commands.txt` and
`merge-280/commands.txt`, with the logs beside them (home directory written as `~`).
In both files the first `om/units.sh --check` line (rc 3, `NOT_RUN: set
OMEGA_SEED`) is a setup miss: the bridge's `build/bootstrap/omc` seed was not in
place. The last two lines are the rerun with the seed in place.

## `head/`: #365 at 27e0b93d (bridge bc11b30b)

| Check | rc | Verdict |
| --- | --- | --- |
| `make test-nvml` (476 checks, 22/22 mutants caught, ABI 55 facts, thin-shell guard) | 0 | PASS |
| `om/units.sh --check` | 0 | PASS |
| `crumb verify .` | 0 | PASS (42 current) |
| NEG `make test-nvml`, env unset | 2 | red as required |
| NEG `run.sh`, env unset | 77 | red as required (NOT_RUN) |
| NEG `run.sh`, non-pin omc | 69 | red as required (REFUSED) |
| NEG `run.sh`, `om/units.sh` without `FRONTEND_PIN` (new with 27e0b93d) | 74 | red as required |
| NEG `run.sh`, `om/units.sh` naming another frontend pin (new) | 69 | red as required |
| NEG `om/units.sh --check`, one byte appended to `cudapp_om_nvml_step.c` | 1 | red as required |

## `merge-280/`: #365 merged into #280 (integ/omega-cuda-stack 630e119b, bridge d90433a9)

#280 already carries the NVML slice (from #305, regenerated at the a3a455bd
frontend). Merging #365 into 630e119b gives 16 add/add or content conflicts.
Before 27e0b93d there were 17; `tests/nvml/run.sh` no longer conflicts.
Resolve every conflict to #280's side and the merged tree is #280's tree
exactly (both `3ba84627c9810c45276189012ab6c6a8c605d903`). So once #280 lands,
#365 adds nothing but this evidence directory. Conflicted paths: `ROADMAP.md`,
`compat/nvml/.crumb`, `evidence/NVML-2026-11-06/{commands.txt,test-nvml.log,wrong-omc.log}`,
`native/nvml/.crumb`, `om/units.sh`, `src/cudapp_native.h`,
`src/cudapp_om_nvml_{errstr,step}.{c,ident}`, `src/cudapp_om_units.ident`,
`tests/nvml/.crumb`, `tools/extract/{.crumb,drift-baseline.tsv}`.

| Check | rc | Verdict |
| --- | --- | --- |
| `make test-nvml` (476 checks, 22/22 mutants caught) | 0 | PASS |
| `om/units.sh --check`: per-unit byte comparison (NVML included) | — | PASS: every unit compared equal; the script reached its last step |
| `om/units.sh --check`: #280's `om/no-posix.sh --check` | 128 | NOT_RUN: it reads oracle compilers with `git show` from the omega-mv checkout holding `OMEGA_OMC`; here omc is a bare copy |
| `crumb verify .` | 1 | FAIL on `tools/prof` only. This is #280's own head 630e119b (same tree), reported to the #280 owner; not NVML |
| Negative controls (env unset, non-pin omc, no/other `FRONTEND_PIN`, tampered unit) | 2, 77, 69, 74, 69, 1 | all red as required |

## Not run

- Live resource manager (`/dev/nvidiactl`, `/dev/nvidia0`) and GB10 chip tier:
  NOT_RUN. main has no `native/nvrm/transport.om` (#281 is in #280 only), and
  there is no live driver for `compat/nvml` yet.
- Utilization over the #281 transport: NOT_RUN (55888-byte control vs the
  e5b32567 devcall bound, aien-dev/omega-mv#1048).
- CI does not run `make test-nvml`. `host-tests` runs `make test` without
  `CUDAPP_OMC_HOST=1`, so `test-nvml` is not in `CARRIED_HOST_TESTS` there.
  These host results come from the Spark only.
