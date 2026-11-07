# NVML seeded-driver migration evidence

Baseline: `4bf58894d1985d41240736145de7b5fe03e377ee`, branch `feat/omega-nvml-replacement`. Source identities are in `sources.sha256`. Tests were CPU-only on core 2, nice 10, with bwrap replacing /dev. No GPU lease or hardware access.

## Exercised commands

Old baseline:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- env TMPDIR=/tmp/nvml365 OMEGA_OMC=/home/drakestapleton/work/finish/nvml365-tools/omc OMEGA_BRIDGE=/home/drakestapleton/work/finish/nvml365-bridge taskset -c 2 nice -n 10 sh tests/nvml/run.sh
```

New integrated target:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 2 nice -n 10 make test-nvml CUDAPP_OMC_HOST=1 OMEGA_OMC=/home/drakestapleton/work/finish/nvml365-tools/omc OMEGA_BRIDGE=/home/drakestapleton/work/finish/nvml365-bridge
```

Both exited 0. `old-run.log` and `new-run.log` contain the observed outputs. `comparison.json` records exact equality of all 22 mutant summary lines, including each failure/check count, and equality of healthy summary lines ignoring ordering. The new gate adds the explicitly labelled seeded-Omega-model line. Both models pass 476 checks; the altered error-string oracle fails 1 of 476; the CUDA-header comparison covers 55 facts. The thin-shell guard and its negative control pass.

The gate was bootstrapped from `tests/components/proc.om`, `tests/nvml/gate.om`, `tests/nvml/mutants.om` using the pinned compiler. The comparison/refusal commands are enumerated in `comparison.json`. Control inputs were private copies mounted over the target files, never changes to the tracked sources:

- Empty OMEGA_OMC/OMEGA_BRIDGE: old/new exit 77.
- OMEGA_OMC=/bin/true: old/new exit 69.
- Removed FRONTEND_PIN declaration: old/new exit 74.
- FRONTEND_PIN replaced by 64 zeroes: old/new exit 69.
- Appended one newline to generated step C: old/new exit 1.
- New gate compiled with NmT0 identical to NmF0: gate exit 1 after the child returned 0 instead of required 1. This is a genuine surviving-mutant refusal, not a compile/exec failure.

Raw control logs are in `controls/`. The permanent test driver is Omega; no scratch Python or shell orchestration is shipped. Versions and dependency roles are documented in `host-tools.txt` and `tests/nvml/README.md`.

## Exact migration boundary

| Path | Physical lines | Role |
|---|---:|---|
| tests/nvml/host.om | 942 | Pure Omega test/model, fake transport only |
| tests/nvml/gate.om | 725 | Seeded gate conductor, replacing run.sh |
| tests/nvml/mutants.om | 71 | Exact source-mutant declarations/calls |
| compat/nvml/cudapp_nvml.c | 170 | Remaining handwritten production protocol/ABI, including 3 migration-comment lines |
| compat/nvml/nvml.h | 116 | Remaining handwritten ABI declarations, including 2 migration-comment lines |
| tests/nvml/nvml_test.c | 344 | Legacy C ABI ORACLE, not shipped |
| tests/nvml/transport.c | 77 | Legacy fake-transport ORACLE, not shipped |
| tests/nvml/transport.h | 14 | Legacy fake-transport ORACLE declarations |
| tests/nvml/abi.c | 69 | CUDA-header ABI ORACLE, not shipped |

The pure-Omega protocol/wrappers/cache are `host.om:225–444`; fake transport is `host.om:144–223`. They have not been integrated into a production .so. The C file still contains the production protocol, not only pointer glue. Its replacement requires the generated shared-library ABI and production Omega protocol integration described by `BRIDGE-EXTENSION.md` and https://github.com/aien-dev/omega-bridge/issues/83.

## Not exercised / external gates

- Generated production .so, concurrent ABI calls, live RM and GB10 qualification: NOT_RUN.
- Devcall utilization-size facility: unchanged omega-mv#1048 limitation.
- Final-head CI: NOT_RUN. Main reported aien-dev Actions blocked by GitHub owner flagging; no reruns requested.
- Push: NOT_RUN by instruction after Main reported the push account suspended. Local commit/patch delivery only.
- Independent signed review, desk merge, deployment: NOT_RUN and not authorized.

#365 remains HOLD for omega-bridge#83 as well as push/CI. No READY line was posted. No compiler feature or accepted bridge syntax changed here, so existing harness language/reference pins are unchanged; the proposed extension must update the harness when implemented.
