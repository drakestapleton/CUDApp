# HW-MANIFEST-20261107T0602Z: hardware manifest gate after review fixes (CUDApp #367)

Status: CHIP_TESTED on the same two devices of one DGX Spark (`spark-b87b`, Linux 7.0.0-1019-nvidia
aarch64): the Cortex-X925 at processor 19 (`midr_el1=0x410fd851`) and the Nuvoton NPCT75x TPM 2.0 at
`/dev/tpmrm0` (manufacturer `0x4e544300`, firmware `0x00070002.0x00040001`). One run, one machine:
EXERCISED, not QUALIFIED. The GPU was not opened. Supersedes `HW-MANIFEST-20261107T0539Z` as the
evidence of the two admitted rows; that directory is unchanged.

Source: this commit (parent `06237840fe344e2b243d3aec91c16eb8ab50ce41`). Source, manifest and binary
digests are the `SRC` and `COMPILE` lines of `run.log`; all files here are in `outputs.sha256`.
Identity: `identity.txt` (hostname, `lscpu`, per-processor MIDR, `tpm2_getcap properties-fixed`,
tpm2-tools and strace versions).

## Commands

```
HW_OUT=/tmp/cudapp-hw-20261107T0602Z flock -o <cpu-pool lock> taskset -c 19 nice -n 10 \
  sh tests/hw/run.sh <abs>/omc                                        -> run.log, exit 0
flock -o <cpu-pool lock> taskset -c 19 nice -n 10 sh regress/regress.sh <hwman> . <dir>
  # once with hwman built from 06237840, once with this change      -> regress/, exit 0 both
```

## Result

`HW pass=121 fail=0 not_run=2` (`run.log`; per-check output in `logs/<check>.log`). NOT_RUN:
`neg-cpu-unpinned` (needs a CPU set wider than the one allotted processor) and `cpu-gb10-a725`
(no Cortex-A725 processor allotted).

## Review findings: each control fails on 06237840 and passes on this change

`regress/regress.sh` runs the same controls against both binaries (`regress/binaries.sha256`);
`regress/before-06237840.txt` and `regress/after.txt` give per control the exit code and last line.
Faults are injected with `strace -e inject` (6.8), an outside test tool.

1. State persistence. A fresh CPU handle at generation 1, then the run to generation 2 with a fault on
   the handle file. `w1`/`w2` is the injected call index (06237840 makes one `pwrite64` and one `fsync`
   in release; this change marks the handle in-flight first, so its release is call 2).

   | control | 06237840 | this change |
   |---|---|---|
   | failed write (`pwrite64` ENOSPC) | `w1`: exit 0 PASS, handle file empty, generations 0, 1, 2 all run again (exit 0) | `w1`: 74 before submission, handle kept at 1; `w2`: 74, handle `x…`, generations 0, 1, 2 refused (7) |
   | short write (`pwrite64` returns 3) | `w1`: exit 0 PASS, handle file empty, generations 0, 1, 2 run again | `w2`: 74, no PASS, generations 0, 1, 2 refused (7) |
   | failed sync (`fsync` EIO) | `w1`, `w2`: exit 0 PASS | `w1` (in-flight sync), `w2`: 74, no PASS, generations 0, 1, 2 refused (7) |

2. Runner binding. One closed runner table (`core.om` `TRUN`) is checked by `check`, which `admit` and
   `run` load through.

   | mutated manifest | 06237840 | this change |
   |---|---|---|
   | CPU ops `+matmul` | check 0; run matmul 5 | check 3 `runner_op`; run 3, handle unchanged |
   | TPM ops `+seal` | check 0; **run seal exit 0, GetRandom sent, generation 1->2** | check 3 `runner_op`; run 3, handle unchanged |
   | CPU runner `hwman-gpu` | check 0; run 6 | check 3 `unknown_runner`; run 3 |
   | TPM row runner `hwman-cpu` | check 0; run 6 | check 3 `runner_family_route`; run 3 |
   | CPU route `DIRECT_DEVICE` | check 0 | check 3 `runner_family_route` |

3. Fault oracle. An unrelated failure injected on `/dev/tpmrm0` during the fault frame:

   | control | 06237840 | this change |
   |---|---|---|
   | badsize, write EIO | exit 0 `errno=-5 outcome=PASS` | 13 `unexpected_fault_result result=-5 want=-22` |
   | badcc, write EINVAL | exit 0 `errno=-22 outcome=PASS` | 13 `result=-22 want=721219` |
   | badcc, 4-byte response | exit 0 `errno=-1000 outcome=PASS` | 13 `result=-1000 want=721219` |

   Correction to `HW-MANIFEST-20261107T0539Z`: `badcc` is not answered by the TPM with `0x143`. The
   real response code is `0x000b0143` = TPM2_RC_COMMAND_CODE `0x143` | TSS2_RESMGR_TPM_RC_LAYER
   `11 << 16` (Wolfram Language `BitOr[16^^143, BitShiftLeft[11, 16]]` -> 721219), synthesized by the
   Linux resource manager (`drivers/char/tpm/tpm-dev-common.c`, `tpm2_prepare_space`) for a command code
   the chip does not list; the frame never reaches the chip. 06237840 printed only the low 3 hex digits.
   The gate now asserts `layer=tpmrm rc=0x000b0143` and `layer=kernel errno=-22` exactly.

`r1-script-bug` (outside the repository only): a first pass of `regress.sh` whose helper overwrote a loop
variable, so its persistence rows named the wrong files; discarded, rerun with the fix above.
