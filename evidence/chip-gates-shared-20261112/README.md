# Chip gates on the GB10, shared chip, 2026-11-12

First run on the chip of the chip gates listed in `AGENTS.md` ("GPU commands") since the #280
integration landed on main. Machine: DGX Spark (GB10, sm_121), NVIDIA open kernel module 580.173.02.

The chip was **shared**: the live 35B model and a training job used it at the same time, and no
window lock was held. These are value checks. No time printed in these logs is a claim, and this is
not the exclusive window that `om/resident/README.md` and `om/gemm-any/README.md` ask for. Status
for every row below is EXERCISED on the chip, not QUALIFIED.

`$HOME` in the logs replaces the home directory of the account that ran them. Nothing else was
edited.

## Before: commit 5783a549 (`before-5783a549/`)

| Step | Command | Result |
|---|---|---|
| doctor | `build/cudapp-doctor` | exit 0, READY |
| hello-gpu | `build/hello_gpu` | exit 0, PASS |
| hello-gpu red control | `build/hello_gpu_control --bug=short` | exit 1, FAIL, `checked 1048576 of 1048576 elements: 1 wrong, 1 never written` (as it must) |
| elementwise | `build/gpu_elementwise_test --out elementwise.json` | **exit 139 (SIGSEGV)** |
| mathlibs | `build/gpu_mathlibs_test --out mathlibs.txt` | exit 0, `mathlibs chip: PASS (938 checks)` |
| resident | `build/gpu_resident_chip --chip` | **exit 1**, `cmdlist: marker wait timed out (latched)` |
| zero-copy share | `build/zero-copy/chip-share` | exit 126: the host gate leaves the file mode 0644, so this step did not run |
| decode attention | `build/gpu_decode_attn_test --out decode.json` | exit 0, `38 checks, 0 failed: PASS` |

### elementwise

`gdb` backtrace: `__memcpy_sve` from `run_abc` from `run_1d`. `run_1d` in
`src/omega_gpu_elementwise_api.c` passed the plan slot `p[10]` as the byte count of each chunk.
Since the per-chunk record (`av_ew_rec`) replaced the per-chunk plan call, that slot is not per
chunk: a 4096-element EX2 copied 524288 bytes out of the device buffer into a 16384-byte result
buffer. The host tier of the test never reaches `run_abc`, so only the chip showed it.
Fix in 97fc9eb3: `bytes = ni * 4`, the expression the function had before commit 0b7c290e.

### resident

`resident-bisect/` holds the scratch copy of the chip test used to find the cause
(`resident_dbg.c`: it prints the two sync words and the arena after the submit, and takes the layer
count, CTA count, threads per CTA and model from the environment) and `runs.tsv`, one row per run:

- one CTA, five layers: passes;
- two CTAs, two layers: passes; three or more layers: marker timeout, arena correct, sync words
  (counter, generation) = (threads per CTA - 1, 2), or one less;
- two CTAs with one thread per CTA: passes.

So the threads that were meant to leave at `@P0 EXIT` were running. `om/resident/resident.om`
emitted `S2R` as a fixed-latency instruction. The compare two instructions later read the register
before `S2R` had written it and saw the zero the prologue had stored, so no thread left. All lanes
of both CTAs then ran the grid barrier; once the lanes of one warp split between the polling arm
and the last-arrival arm, the polling lanes spun and the others never ran.

The warp simulator has no instruction latency and passes this kernel. The static hazard checker
(`src/omega_bw_hazard.c`) reports it (`hazard-before.txt`: two RAW findings on R18 with release 0,
the two `S2R`), but no host test ran that checker on the resident kernel.
`kernel-5layers-2ctas-before.sass` is this repository's own disassembler on the failing kernel;
its two `??` lines are `ATOMG.E.EXCH.STRONG.SYS`, which the disassembler did not know.

Fix in 97fc9eb3: `S2R` is emitted as a variable-latency instruction (`KVAR`), as
`om/kgen/ew_emit.om` and `om/mathlibs/mathlibs_ir.om` already did; `cp` adds the zero register of
the fixed plan instead of an absent source, which the encoder writes as R0. `gpu_resident_test`
now runs the hazard checker on the encoded kernels (`hazard-after.txt`: 0 findings), and
`gpu_resident_mutants.py` has the mutant `fixed-latency-thread-id`, which that check kills.
The commit after it adds the resident kernels to the `bw_hazard_test` sweep and the
`ATOMG.E.EXCH.STRONG.SYS` row to the disassembler (`om/disasm/sm121_dis.om` and the C oracle).

## After: commit 97fc9eb3 (`after-97fc9eb3/`)

All eight steps, binaries rebuilt at that commit (hashes in `job.log`):

| Step | Result |
|---|---|
| doctor | exit 0, READY |
| hello-gpu | exit 0, PASS |
| hello-gpu red control | exit 1, FAIL with the expected line |
| elementwise | exit 0, `PASS: 35 checks, 0 failed`, every mutant caught |
| mathlibs | exit 0, `mathlibs chip: PASS (938 checks)` |
| resident | exit 0, four lines `exact_f32=PASS` (resident and cmdlist-layers, both witnesses), `GPU RESIDENT CHIP PASS` |
| zero-copy share | exit 0, `ZERO_COPY_SHARE_CHIP_PASS` (a copy of `build/zero-copy/chip-share` made executable for this run) |
| decode attention | exit 0, `38 checks, 0 failed: PASS` |

The live model answered `/health` with 200 after the run.

## Not run

- `build/gpu_gemm_any_test --chip` and the ceiling probes (`om/gemm-any/README.md`): they time
  8192 x 8192 x 8192 products and must not run beside a live model. NOT_RUN.
- Any exclusive, quiet window. NOT_RUN.
- A second machine. NOT_RUN.
