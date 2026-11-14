# fp8gemv_b128f v3r2 (two rows per wave) and v3r4 (four rows per wave)

Lane uxl-brief/AmdKernelSpeed.Fp8TwoRow, Spark (Linux AArch64, no /dev/kfd), 2026-11-14. CPU only. Every step ran
under `flock -o ~/handoffs/hive/queen/pool-15.lock taskset -c 15 nice -n 10`. No GPU work was done, and nothing was
landed or published. The final run is `run-all.log`: START 2026-11-14T03:05:46Z, `RUNALL PASS 2026-11-14T03:09:24Z`,
rc=0.

## Outcome

| Candidate | Object sha256 | Kernel gate (seeded gfx942 executor + LLVM 18) | Repository acceptance, adapted to the grid | Launcher, rewritten real2.plan |
|---|---|---|---|---|
| v3r2 | `9377954c28eae8597273a6bf99ed2964905a8a27f8512cb7cc1c140d1dc91e9c` | PASS: 62/62 cases; 22/22 mutants and 2/2 grid controls CAUGHT; waitcheck control CAUGHT | PASS: cases=27 controls=8 | exit 77; 6/6 checks PASS |
| v3r4 | `5620081b0d2cfa5d6bcc35daa5d1093663424f3721350748357d0b375028b4b3` | PASS: 62/62 cases; 27/27 mutants and 2/2 grid controls CAUGHT; waitcheck control CAUGHT | PASS: cases=27 controls=8 | exit 77; 6/6 checks PASS |

Status: EXERCISED on the host (seeded CPU executor, LLVM 18 encoding oracle, seeded launcher up to the device
boundary). Hardware and performance are NOT_RUN. No MI300X timing exists for either candidate, so this does not
show whether either one is faster than v2b2.

For comparison, v1 built from the repository table in the same gate has object sha256
`72049eeefffe5431c0f8972d1d9f3e4f93e39841312aeb492ac24f455d23adbd` (GpuBenchSync recorded the same value).

## What the kernels do

`gen_r2.py` generates both candidates (default `--rows=2`; `--rows=4` gives v3r4).

**Generator**
- It runs GpuBenchSync's `gen.py` unchanged (batch 2 = v2b2), including all of its self-checks against v1.
- It reuses that generator's emitters, and asserts that its own emitters reproduce v1's and v2b2's lines exactly for the first row.

**Row mapping**
- Workgroup g computes rows R·g .. R·g+R-1.
- A wave whose first row is ≥ M leaves at once. The kernel is therefore also correct at v1's grid of M workgroups; the gate checks this on every case.

**What is shared and what is per row**
- Shared:
  - the x loads (16 dwords per lane per 1024-column chunk);
  - the BF16 scale loads (R divides 128, so all rows of a wave are in one scale block row);
  - the column counters and the x/scale pointers.
- Per row:
  - W pointer, W dwords, accumulator and decode temporaries.
  - Rows after the first use the first row's instructions under a register renaming. The generator asserts that renaming is exact and that the registers are disjoint.

**Bit identity with v1**
- Per row, the following are v1's: the lane-to-column split, the per-lane FMA order, the scale application, the XOR reduction and the store.
- v2b2's choice between fast decode and v1 decode is made per row.
- As a result, every output word equals v1's bit for bit (gate: whole memory image).

**M not a multiple of R**
- A missing row's W pointer stays equal to the previous row's, so it reads nothing past W.
- That row recomputes the previous row, and its store is masked by `M > row`.
- The gate covers M = 1, 2, 3, 5, 6 and 7. The acceptance's access-instrumented executor measures exact buffer spans (M=257 case plus the one-byte-short controls).

**ABI and resources**
- ABI: v1's kernarg (W, x, scale_inv, y, M, K), unchanged. Grid: ceil(M/R) workgroups of 64.

| | VGPRs | Instructions (v1: 422) | Loads in flight per pair of chunks | Code bytes |
|---|---|---|---|---|
| v3r2 | 96 | 2272 | 32 | 11948 |
| v3r4 | 128 | 4385 | 48 | 23064 |

## Files (fp8r2/)

| File | sha256 |
|---|---|
| `fp8gemv_b128f-v3r2.hsaco` | `9377954c28eae8597273a6bf99ed2964905a8a27f8512cb7cc1c140d1dc91e9c` |
| `fp8gemv_b128f-v3r4.hsaco` | `5620081b0d2cfa5d6bcc35daa5d1093663424f3721350748357d0b375028b4b3` |
| `kernels/fp8gemv_b128f/v3r2/fp8gemv_b128f.isa` (table for seeded hsaco.om) | `82a44f25f0b78fb469186642815757e30355051eb2d461c18d0c115873795c7f` |
| `kernels/fp8gemv_b128f/v3r2/fp8gemv_b128f-gfx942.s` (LLVM oracle listing) | `51f0978cbc69ad6b2bfd326b32d0fdd4a738cf8ba8b1e015ac7522654e6b4e8a` |
| `kernels/fp8gemv_b128f/v3r4/fp8gemv_b128f.isa` | `016c1085814f9507109840ebc8e577099b89cdc148d3c1ff3a3b091f52594cca` |
| `kernels/fp8gemv_b128f/v3r4/fp8gemv_b128f-gfx942.s` | `7fd518329da90a6a6cb2250191946f4c2fb3ad591dad6d64b9a138ebd3cfc43a` |
| `gen_r2.py` (generator) | `8e783484955c74d080d1cc57984b5e71ac9b07eb3249aa746a4f62524b663b7b` |
| `halve-grid.py` (plan rewriter) | `e03577aa4c10bc5f59b818a3cfb8d9f0ad64d5e3f3173b1cb6000aebc93db18f` |
| `gates/kernel-gate-r2.sh` | `78afcf49a7edbc9b00e83a30220eeef02bade3c67f4f3bc610b184f5e3bec3b2` |
| `gates/fp8gemv_b128f_compare_r2.py` | `dc783782ae728880c84532e228ff5df9525e73f860fb9c56aca82d14197d426e` |
| `gates/waitcheck.py` (GpuBenchSync's, copied unchanged) | `2b75cfab33d5d77e8adcce02ca3648d84f99d5b0f047f2d3ac32da04d35de6ae` |
| `make-acc-r2.py` (writes the adapted acceptance) | `92cc65f78ccd5a620dbec184e19da0f40f0e077acaaabd526395e729c5743d24` |
| `acc-r2.sh` | `a4e81cead4090e3902ec5bb05d4b0f252fe87a86befad871eca13ba15eab7189` |
| `launch-check.sh` | `679d9e829217011f837440caff5875de6475a5fa7ce8ebd616fc82516e4c4f8c` |
| `run-all.sh` | `0bac41230b9cebfe1715fb793c642e57f4f69a1649e1397e74e7fb19eb970fd7` |

**Inputs**
- Read from `../takeover-gpubenchsync/fp8v2/`:
  - `gen.py` `cf1cde45…db25db`;
  - `v1.isa` `4e805df0…4ecd1`, the same bytes as the repository's `fp8gemv_b128f.isa`;
  - `v1.s` `6184039f…b93e`;
  - `ckpt/w` with its `w.sha256` manifest.
- Copied into this folder:
  - `omc-2c82` `2c824805…5029f` (the gate refuses any other compiler);
  - `src/CUDApp` (`diff -r` against GpuBenchSync's copy: identical; the source has no `.git`, so the dc536882 attribution is GpuBenchSync's).
- Built in the gate: hsaco producer `eca3f3b0…ab95b`, executor `0005b41b…404c3`.

**Logs**
- `run-all.log`
- `gate-v3r2.log`, `gate-v3r4.log`
- `acc-*.log`
- `launch-check-v3r2.log`, `launch-check-v3r4.log`
- `acc-harness-rows2.diff`, `acc-harness-rows4.diff`
- `run-all.attempt1.log`: an earlier, superseded failing run (see *What failed*).

## 1. Kernel gate (seeded gfx942 executor, LLVM 18 parity)

`gates/kernel-gate-r2.sh` is GpuBenchSync's `kernel-gate.sh`, adapted to R rows per wave.

**Steps**
1. Check the omc pin.
2. Build v1 and the candidate with the seeded hsaco.om; two candidate builds must be byte-identical.
3. Check LLVM 18 parity of `.text` and of the 64-byte descriptor (all bytes except the entry offset 16..23), the same check as `kernel-gate.sh`.
4. Run `waitcheck.py`. The table has no partial waits, so it runs on the executor unchanged (checked).
5. Run `fp8gemv_b128f_compare_r2.py`.

**Cases and controls in `fp8gemv_b128f_compare_r2.py`**
- GpuBenchSync's 39 cases unchanged (M=3), plus 23 added cases:
  - special bytes 0x00/0x80/0x7f/0xff in the *first* row of a wave, in the pair, single and residual loops (the original cases put them only in row 1);
  - rows 3/4 at M=5;
  - clean M = 1, 2, 4, 6, 7, 8;
  - M=130 across the 128-row scale block.
- Per case, all of the following must hold:
  - v1 at M workgroups, the candidate at ceil(M/R), and the candidate at M workgroups all exit 0 with no output;
  - their whole memory images are byte-identical;
  - the images equal the repository's exact rational oracle.
- Grid controls:
  - `v1-at-halved-grid`;
  - `cand-at-floor-grid` (floor instead of ceil for the grid).

```
v3r2 object sha256 9377954c28eae8597273a6bf99ed2964905a8a27f8512cb7cc1c140d1dc91e9c
PASS v3r2 two builds byte-identical
PASS v3r2 code bytes equal independent LLVM 18 oracle (11948 bytes)
PASS v3r2 descriptor bytes (except entry offset 16..23) equal LLVM oracle
WAITCHECK PASS peak_outstanding=32 errors=0
CONTROL ctl-nowait CAUGHT by waitcheck: WAITCHECK FAIL peak_outstanding=32 errors=115
MUTANTS listed=22 built=22
FP8GEMV-B128F-R2-GATE PASS cases=62 mutants=22 controls=24 fail=0

v3r4 object sha256 5620081b0d2cfa5d6bcc35daa5d1093663424f3721350748357d0b375028b4b3
PASS v3r4 two builds byte-identical
PASS v3r4 code bytes equal independent LLVM 18 oracle (23064 bytes)
PASS v3r4 descriptor bytes (except entry offset 16..23) equal LLVM oracle
WAITCHECK PASS peak_outstanding=48 errors=0
CONTROL ctl-nowait CAUGHT by waitcheck: WAITCHECK FAIL peak_outstanding=48 errors=225
MUTANTS listed=27 built=27
FP8GEMV-B128F-R4-GATE PASS cases=62 mutants=27 controls=29 fail=0
```

**Mutants**
- The generator lists them (`gen_r2.py --list`) and the gate builds every listed one. The gate checks the built count against the listed count, so the `v2-[a-z-]*` silent-skip pattern of the original gate does not apply.
- All were CAUGHT. Counts are cases with a differing image (v3r2 / v3r4):

| Mutant | What it breaks | Caught by |
|---|---|---|
| `rowswap` (requested) | the two rows' outputs swapped | 61 / 61 |
| `sharedacc` (requested) | row B adds its scaled group sums into row A's accumulator | 62 / 62 |
| `sharedgroup` | row B uses row A's group partial register (bites in the interleaved residual) | 21 / 21 |
| `oddstore` | stores of later rows not masked by M | 45 / 48 |
| `brow` | row B's W pointer not advanced by K | 61 / 61 |
| `bshortadvance` | row B's W pointer advances 1024 instead of 2048 per pair | 41 / 41 |
| `wrongw-b` | row B's second chunk of a pair loads the first chunk's W | 40 / 40 |
| `residb` | row B's residual bytes read from row A | 20 / 20 |
| `pairwrongx` | shared x pointer not advanced between the two chunks of a pair | 48 / 48 |
| `pairdropscale` | one shared scale load dropped | 48 / 48 |

  Detection-path mutants, per row (v3r2 / v3r4):

| Mutant | Row A | Row B |
|---|---|---|
| `neverslow-*-pair` | 47 / 6 | 12 / 9 |
| `neverslow-*-single` | 42 / 3 | 6 / 6 |
| `missnan-*` | 6 / 6 | 12 / 10 |
| `misszero-*` | 53 / 3 | 6 / 5 |
| `fastnodouble-*` | 49 / 49 | 38 / 38 |
| `fastwrongbyte-*` | 49 / 49 | 38 / 38 |

  v3r4 only:
  - `rowswap-cd` 21;
  - `sharedacc-dc` 60;
  - `brow-d` 21;
  - `neverslow-d-pair` 3;
  - `missnan-d` 2.

  Grid controls (v3r2 / v3r4):
  - `v1-at-halved-grid` 61 / 61;
  - `cand-at-floor-grid` 45 / 48.
- `missnan-a` is caught only by the added cases (first hit `m4-k5120-7f-rowA`). With GpuBenchSync's 39 cases alone it would have escaped, because their row-A rows never hold a 0x7f byte.

## 2. Repository acceptance (`fp8block_fast.py`, real checkpoint rows)

**Why the harness is adapted**
- `fp8block_fast.py` drives the row-probe executor with workgroup ids equal to row ids, so it cannot run a multi-row kernel unchanged.
- `make-acc-r2.py` writes `fp8block_fast_r2.py` into a tree copy (`acc/LABEL/CUDApp`). Every edit is an exact single-occurrence replacement; the full diffs are `acc-harness-rows2.diff` and `acc-harness-rows4.diff`.

**What changes**
- Workgroup ids are row//R, and the overdispatch workgroup is ceil(M/R).
- Sampled rows, including the checkpoint rows read from the shards, are closed under the wave.
- The per-row-scale source-mutant control samples row R+1 instead of row 1. With R rows per wave, row 1's wave starts at row 0, where that mutant equals the original. For R=4 it uses an M = 769 fixture, so that block row 4 exists.
- The report records grid `ceil(M/R)`.
- The oracle, the v11 tolerance, the fixtures, the shards (`layers-0`, `layers-3`, pinned by `w.sha256`) and the LLVM checks are unchanged.

**Results**

| Run | Expect | Result |
|---|---|---|
| v3r2, R=2 | pass | `PASS fp8gemv_b128f cases=27 controls=8 hardware=NOT_RUN performance=NOT_RUN`; ACCEPT PASS rc=0 |
| v3r4, R=4 | pass | same line; ACCEPT PASS rc=0 |
| v1, R=1 (adapter control: must still pass v1) | pass | same line; ACCEPT PASS rc=0 |
| v1, R=2 | fail | CAUGHT rc=1 (all-ocp-bytes byte mismatch) |
| v3r2, R=4 | fail | CAUGHT rc=1 |
| v3r2 `rowswap`, `sharedacc`, R=2 | fail | both CAUGHT rc=1 |
| v3r4 `rowswap`, `sharedacc`, R=4 | fail | both CAUGHT rc=1 |

- Checkpoint cases: 17408x5120, 5120x17408, 12288x5120, 1024x5120, 10240x5120, 6144x5120 and 5120x6144. Each case checks every memory byte.
- Controls caught in each passing run: `per-row-scale`, `transposed-scale`, `no-tail-block`, and `short-{kernarg,W,x,scale_inv,y}` (exit 68).

## 3. Plan rewriter and launcher check

`halve-grid.py IN.plan OUT.plan [ROWS]`:
- Uses `measure/amplify-plan.py`'s OMPLAN01 parser (copied), plus each record's offset.
- Sets grid x of every `fp8gemv_b128f` dispatch to ceil(M/ROWS) and changes nothing else.
- Self-checks, with the same parser, that only those grid words differ.
- Refuses (exit 2, writes nothing):
  - a dispatch whose grid x is not its M (an already rewritten plan);
  - a non-OMPLAN01 file.

`launch-check.sh VAR ROWS`:
- Uses the seeded launcher `../omega-launch` (sha256 `2d6bfba8…9a47`, equal to `parse-gate.log`).
- Uses the sparse weights in `../w/` (sizes checked against `SIZES.tsv`) and the arguments from `real2.plan.cmd`, built as `parse-gate.sh` builds them.
- Reads `measure/stub-plan/real2.plan` (sha256 `2a292dad…c012`).

```
HALVED rows=2 dispatches=1126 rewritten=400 bytes_differing=400  out sha256 3f9492cad219b8c86871f82c8fce40d1a849f72c25dc24fa405d985a7d3c189c
CLASS fp8gemv_b128f:10240x5120 count 48 gx 10240 -> 5120     CLASS fp8gemv_b128f:1024x5120 count 32 gx 1024 -> 512
CLASS fp8gemv_b128f:12288x5120 count 16 gx 12288 -> 6144     CLASS fp8gemv_b128f:17408x5120 count 128 gx 17408 -> 8704
CLASS fp8gemv_b128f:5120x17408 count 64 gx 5120 -> 2560      CLASS fp8gemv_b128f:5120x6144 count 64 gx 5120 -> 2560
CLASS fp8gemv_b128f:6144x5120 count 48 gx 6144 -> 3072       CLASSES dispatches=1126 unchanged-non-fp8 OK
PASS original exit=77            (OMEGA device=absent errno=2)
PASS halved-v1code exit=77
PASS halved-v3r2code exit=77     (launch-v3r2/code/fp8gemv_b128f.hsaco = the v3r2 object)
PASS refuse-rewrite-twice exit=2 REFUSED dispatch 2: fp8gemv_b128f grid (5120, 1) is not (M=10240, 1); already rewritten?
PASS refuse-not-a-plan exit=2 REFUSED not an OMPLAN01 plan
PASS halved-neg-count exit=64    (OMEGA refuse plan field=span-size)
LAUNCHCHECK pass=6 fail=0
```

ROWS=4 gives the same result: grid 10240→2560 … 6144→1536; output sha256 `e4c91a462ac2075467a90310f47de4635e36cc86afa26b8f3b4f8901f1ccdf31`; `LAUNCHCHECK pass=6 fail=0`.

Exit 77 means the launcher accepted the plan, code objects and weights up to the device boundary. It is not an execution.

## Exact commands

From `spark:~/work/hive/uxl-brief-20261113/AmdKernelSpeed/fp8r2`:

```
sh run-all.sh > run-all.log 2>&1        # everything below, each step under the pool-15 lock
# which runs, with F=../takeover-gpubenchsync/fp8v2 and L="flock -o ~/handoffs/hive/queen/pool-15.lock taskset -c 15 nice -n 10":
$L python3 -I gen_r2.py --rows=2 $F/gen.py $F/v1.isa $F/v1.s kernels/fp8gemv_b128f/v3r2        # --rows=4 ... v3r4
$L sh gates/kernel-gate-r2.sh $PWD/omc-2c82 $PWD fp8gemv_b128f v3r2 2 python3 -I gen_r2.py --rows=2 $F/gen.py $F/v1.isa $F/v1.s
$L sh acc-r2.sh v3r2 kernels/fp8gemv_b128f/v3r2/fp8gemv_b128f.isa kernels/fp8gemv_b128f/v3r2/fp8gemv_b128f-gfx942.s 2 pass
$L sh launch-check.sh v3r2 2
python3 -I halve-grid.py ../measure/stub-plan/real2.plan OUT.plan 2      # the rewriter alone
```

## What failed, and limits

**Superseded first run**
- `run-all.attempt1.log` ended `RUNALL FAIL`.
- In the R=4 acceptance, the per-row-scale control used the M=257 fixture. Its scale grid has no block row 4, so the mutant read past `scale_inv` and the access-instrumented executor refused it instead of producing a wrong value. The harness requires exit 0 for that control.
- Fix: that control alone uses M = 128·(R+2)+1 for R > 2 (`make-acc-r2.py` item 3).
- The rerun (`run-all.log`) passes. Every other result was the same in both runs.

**Earlier aborted attempts during development**
- Fixed and not used as evidence:
  - checkpoint rows not closed under the wave;
  - a log name collision.

**Adapted gates**
- The repository acceptance ran only through the adapted harness (diffs above). The unmodified `fp8block_fast.py` does not apply to a multi-row kernel, because it passes row ids as workgroup ids.
- GpuBenchSync's v2 mutant script (`fp8gemv_b128f_mutants.py`) was not run. Its text patterns assume one row per loop. Its ten mutants have row-A/row-B equivalents in `gen_r2.py`.

**Pairing plan and object**
- The launcher accepts a halved plan with the v1 object (exit 77) as well as with the candidate. It does not bind the grid to the object's rows per wave.
- So a halved plan must ship only with v3r2/v3r4, and v1/v2b2 must keep the full grid.
- The reverse pairing (v3rR at the full grid) is correct but wastes waves.

**Not run**
- Hardware, timing and full-model token check on the MI300X: NOT_RUN. No Omega or AIEN repository was changed.
