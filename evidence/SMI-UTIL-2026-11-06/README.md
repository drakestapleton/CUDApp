<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# SMI-UTIL-2026-11-06: cudapp-smi utilization host receipts

Machine: DGX Spark GB10 (sm_121), driver 580.173.02, Linux aarch64.
Paths under the user's home are written as `~`.

Compilers:

- candidate: aien-dev/omega-mv#1048 (`fix/devcall-param-cap-1042` at
  `e9b635875e6d43314aedc9d85f25c8f65636ac3d`), omc sha256
  `390629fd22590aaefca852045f8b0ace86c62cefb3b1bc203ce400773c8abc2f`, devcall
  bound 1048576 bytes. PROPOSED: not merged, needs Drake's approval.
- base: omega-mv `e5b32567`, omc sha256
  `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

The runs below used the working tree committed as `2767411` on
`feat/cudapp-smi-util` (no source change between the runs and the commit).

## Host tier (`host-run.log`)

```sh
taskset -c 10-14 nice -n 10 sh tests/smi/run.sh <candidate omc> <base omc>
```

rc 0, last line `CUDAPP-SMI HOST PASS`. Mutants caught 22/22 (8 new for
utilization). The devcall bound probe made a 55888-byte devcall on `/dev/null`
(not a GPU device) and got ENOTTY under the candidate; under the base omc it
trapped 3 (exit 103). Fake RM: default report `utilization.gpu=42`,
`utilization.memory=13`; faults w (NOT_SUPPORTED), e (count 0) and z (zero
timestamp) print `[N/A]`; y (0x1b) exits 31; r (tracker 72) and x (util
10001) exit 32.

Controls on the same tree: `run.sh <base omc>` rc 69 (REFUSED, wrong pin);
`run.sh` with no argument and no `CUDAPP_SMI_OMC` rc 77 (NOT_RUN);
`make test-smi` without `CUDAPP_SMI_OMC` rc 2 (NOT_RUN);
`make test-smi CUDAPP_OMC_HOST=1 CUDAPP_SMI_OMC=<candidate> OMEGA_OMC=<base>`
rc 0, last line `CUDAPP-SMI HOST PASS`.

## Live read-only tier (`live-*`)

```sh
taskset -c 10-14 nice -n 10 sh tests/smi/live.sh <candidate omc> OUTDIR
```

rc 0, last line `CUDAPP-SMI LIVE PASS (20 fields)`. `cudapp-smi` ran without
`--rm` and opened no device; `utilization.gpu` and `utilization.memory` are
`[N/A]` there and RM_NOT_RUN in `live-compare.tsv` (nvidia-smi: `0 %`, `0 %`).
`live.sh <base omc>` rc 69 (REFUSED).

## Not run

Live `--rm` (opens `/dev/nvidiactl` and `/dev/nvidia0`): NOT_RUN, needs an
authorized spark-window. Chip qualification: NOT_RUN.
