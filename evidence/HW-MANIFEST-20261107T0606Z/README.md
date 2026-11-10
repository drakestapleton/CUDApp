# HW-MANIFEST-20261107T0606Z: hardware manifest gate, final run of the CUDApp #367 review fixes

Status: CHIP_TESTED on the Cortex-X925 at processor 19 (`midr_el1=0x410fd851`) and the Nuvoton NPCT75x
TPM 2.0 at `/dev/tpmrm0` of one DGX Spark (`spark-b87b`, Linux 7.0.0-1019-nvidia aarch64). One run, one
machine: EXERCISED, not QUALIFIED. The GPU was not opened. Evidence of the two admitted rows.

Same sources and the same hwman binary (sha256 `50062b0f0a77e4e2cf5d38bb49fc2bffe51294b22d15be3062492cde376df1b8`)
as `HW-MANIFEST-20261107T0602Z`; the manifest differs only in the GPU row's blocker text (the GPU row is
refused as `not_admitted`, no longer "wrong authority") and in the evidence field of the admitted rows.
Digests: the `SRC` and `COMPILE` lines of `run.log`; files here in `outputs.sha256`. Identity:
`identity.txt`.

```
HW_OUT=/tmp/cudapp-hw-20261107T0606Z flock -o <cpu-pool lock> taskset -c 19 nice -n 10 \
  sh tests/hw/run.sh <abs>/omc                                        -> run.log, exit 0
```

`HW pass=121 fail=0 not_run=2`. NOT_RUN: `neg-cpu-unpinned` (needs a CPU set wider than the one allotted
processor) and `cpu-gb10-a725` (no Cortex-A725 processor allotted).

The before/after regression of every review control against `06237840` (persistence, runner binding,
fault oracle) and the `badcc` correction (`0x000b0143`, kernel resource-manager layer, not the chip) are
in `../HW-MANIFEST-20261107T0602Z/README.md`.
