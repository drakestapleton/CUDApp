# GEMM fault detection and schedule perturbation: evaluator contract

Linear DRA-420 (the numerical/GEMM slice only) and DRA-387 (the evaluator and concurrency slice).
CUDApp#55 (schedule perturbation) and CUDApp#56 (reduced-concurrency witness) are covered on the
host only. Weight integrity at load time belongs to the ownmodel loader and is not in this slice.

Status labels: everything here is **SIMULATED/HOST_TESTED**. The candidate is a first-party `.om`
tiled GEMM run by simulated workers on the host CPU. No GB10 (sm_121) kernel, device output buffer or
device scheduler has been exercised. Every device edge in section 10 is **NOT_RUN**.

## 1. Files and build

| Path | Role |
|---|---|
| `tests/abft/frozen.om` | Frozen plan: 51 rows plus config. Its hash is in `tests/abft/FROZEN.sha256`, committed in `9a16df99` before any checker existed. |
| `native/abft/lib.om` | Buffered output, splitmix64 streams, FNV-1a, bf16 and f32 decoding. |
| `native/abft/check.om` | Independent checkers: full recompute, two-sided checksum, sampled recompute, input digest, structural check. |
| `native/abft/cand.om` | Candidate tiled GEMM (W workers, two barriers per K tile) and the labelled fault injectors. |
| `native/abft/sched.om` | CUDApp#55 admission. A refused row never forks. |
| `native/abft/run.om` | Evaluator: fork, deadline, reap, collect, detect, admit, retry, tallies, witnesses, gates. |
| `native/abft/cul1.om` | `kabft`: the same two-sided checksum on the merged learn53 CUL1 GEMM records. |
| `tests/abft/cul1-unit.om` | CUL1 checksum unit cases. Uses `tests/learn53/fixtures.om`. |
| `tests/abft/run.sh` | Pin check, frozen-hash check, build, unit run and full evaluator run. |

```sh
OMEGA_OMC=/abs/omega-mv/seed/out/omc sh tests/abft/run.sh build/abft   # all 51 rows plus gates
build/abft/abft FIRST LAST                                             # a row range; gates are skipped
```

The compiler must be omega-mv `e5b32567a7aff3ce594e3254a622a846a6cd190b` `seed/out/omc`, sha256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`; `run.sh` refuses any other.
`build/abft/abft` exits with the number of failed gates (0 when all pass), or 64 for bad arguments.

## 2. Roles and isolation

- **Evaluator.** Owns the frozen plan, the seeds, the input matrices, the nonce, the tolerances, the
  checkers, the deadline, the retry count, the trusted record and admission.
- **Candidate.** Runs only in a forked child. Its one channel back is the bytes it writes into an
  evaluator-created memfd, plus its exit status. Before any check runs, the evaluator reads those
  bytes into `gobs`.
- **Fault-free reference.** The evaluator also runs the candidate's own algorithm in-process, with
  one worker, round-robin and no fault (`cref`, into `gref`). It is used only to label the ground
  truth "corrupt" for the FP/FN table. No checker reads `gref`. The element order is k-ascending for
  every element under every schedule, so a fault-free run is bitwise schedule-independent.
- **Nonce.** The checksum multipliers and the sampled positions come from evaluator stream 1. That
  stream is seeded with a nonce from `random` after the candidate has exited, so the candidate cannot
  predict them.
- **Freeze.** The plan hash was committed before any checker, tolerance or candidate code. Gates 1–9
  (section 9) were written after the freeze but before the first run. After the first sandbox
  development runs, gate 8 was revised and gate 10 was added (section 9 says why). The frozen rows,
  seeds, S, deadline and retry count were never changed.

## 3. Fault model, precision and tolerance

**Fault model.** Errors that reach the output buffer:

- Bit flips in output elements: single and multi-element.
- Stale output: an uninitialised buffer, a skipped write, or a stale buffer that happens to be
  identical.
- Indexing errors: a dropped K tail, a transposed B, a row offset, or an out-of-bounds store.
- Synchronisation errors: a missing load barrier, a missing compute barrier, a barrier that counts
  only workers 0..1, or a hang.
- Corruption of the evaluator's A input after generation (common mode).

Not modelled: faults inside the checker itself, faults in the evaluator's generation code, and
faults that alter both the candidate and the checker's view of the output identically.

**Precision.**

- **dt 0 (exact).** u32 inputs; products and sums mod 2^32; tolerance 0.
- **dt 1, profile P1.** bf16 inputs (sign, exponent 120..127, 7-bit mantissa) and an f32 output. The
  products of two bf16 values are exact in f32. Accumulation is sequential in f32, round-to-nearest-even,
  with no FMA (OMEGESE float rules).

Bounds with $u = 2^{-24}$, $u' = 2^{-53}$ and $\gamma_n(u) = nu/(1-nu)$:

- **Per element.** Full and sampled modes compare against an f64 recomputation $s$:

  $$|\hat c_{ij} - s| \le \bigl(\gamma_K(u) + \gamma_K(u')\bigr)(1+2^{-20})\,(|A||B|)_{ij}$$

- **Checksum, one side.** $n$ is the summed extent: N for the right side, M for the left side. The
  bound is $t(n,K)\,g$, with

  $$t(n,K) = \bigl(\gamma_K(u) + \gamma_{n-1}(u')(2+\gamma_K(u)) + \gamma_K(u')(1+\gamma_{n-1}(u'))\bigr)(1+2^{-20})$$

  For the right side, $g_i = \sum_k |a_{ik}| \sum_j |b_{kj}|$, which is $(|A||B|e)_i$ with $|r| = e$.
  The left side is the mirror image. The multipliers are $\pm 1$ from the nonce stream.

**Wolfram checks** (WolframLanguageEvaluator; definitions $u = 2^{-24}$, $u' = 2^{-53}$,
$g[n] = nu/(1-nu)$, $g'[n] = nu'/(1-nu')$, $sl = 1 + 2^{-20}$):

- `And @@ Table[(1+up) <= (1-up) sl (1-gp[k]), {k,1,256}]` → `True`.
- `(1+up) <= (1-up) sl (1-gp[n+k])` for all $n, k \le 256$ → `True`.
- Values: `GF[80] = 4.768394328e-6`, `GR[48,80] = 4.768394339e-6`, `GF[256] = 1.525902193e-5` and
  `GR[256,256] = 1.525902198e-5`.
- The slack actually needed is $5.7065\times10^{-14}$, far below $2^{-20}$.
- Constants: `2^-24 == 59604644775390625/10^24` → `True`; `2^-20 == 95367431640625/10^20` →
  `True`; the decimal literal `1.1102230246251565e-16` lies within $2^{-106}$ of $2^{-53}$ → `True`,
  so it rounds to $2^{-53}$.

Every comparison is written `not le d t`, so a NaN is detected. The structural check also counts
outputs whose f32 exponent is all ones.

**Rounding and quantisation declaration.** These bounds are for profile P1 only. A device kernel
that uses FMA, a different accumulation order, split-K, tensor-core fragment accumulation, MUFU, or
denormal flushing (GB10 flushes denormals) needs its own profile and bound before these checks
apply. One example is gemm-any #283's bound $n/(2^{18}-n)\cdot\sum|ab|$ with $n = \lceil K/16\rceil$,
recorded as profile **P2, NOT_RUN**. Inputs avoid denormals by construction (exponent ≥ 120).

## 4. Exact mode (dt 0): checksum miss bound

The two-sided Freivalds check works mod $2^{32}$ with odd multipliers. Let $v$ be the 2-adic
valuation of the error pattern's combination. The miss probability is at most $2^{v-31}$ per side.

Wolfram enumeration over the scaled model mod $2^6$:

- Every single-element error is detected.
- The $2^5$ rectangle is always missed.
- The bound $2^{v-(w-1)}$ is attained.

At 32 bits, an error of $2^{31}$ on the four corners of a 2×2 rectangle is a **deterministic miss**
(row 40, gate 7). Full recompute catches it. On the merged learn53 CUL1 record, `kabft` misses the
same rectangle while `keval` (exact) catches it (`tests/abft/cul1-unit.om`, `abrect`). Wolfram:
`{2^31+21, 2^31+24, 2^31+47, 2^31+54}` → `{2147483669, 2147483672, 2147483695, 2147483702}`, and
`{{1,2},{3,4}}.{{5,6,7},{8,9,10}}` → `{{21,24,27},{47,54,61}}`.

## 5. Modes, admission and cost

| Mode | Work | In admission |
|---|---|---|
| Structural | output length, exit status, nonfinite count | yes |
| Input digest | FNV-1a over dims, A and B; detects common-mode input corruption | yes |
| Full recompute (thorough) | $2MNK$ multiply-adds in f64 or mod $2^{32}$ | yes |
| Checksum (ABFT, Huang–Abraham 1984, doi:10.1109/TC.1984.1676475; Freivalds) | $O(MK + KN + MN)$ per side | yes |
| Sampled recompute | $S = 64$ elements with replacement; about $SK$ | **no**, reported only |

- **Admission** is structural OR digest OR full OR checksum. Each detection is retried once, under a
  new schedule. A pass on retry is RECOVERED; detection on both attempts is REJECTED.
- **Sampled mode** is reported but does not affect admission. Its measured coverage and cost
  (evidence, REALITY.md) do not justify admission on their own: for a single corrupt element the
  detection probability is $1-(1-1/MN)^S$.
- **Performance comparisons.** Check time is never included in `t_cand`. Any performance comparison
  that uses these checks must charge equal check cost to both sides and disclose the mode.

## 6. Frozen plan

The row fields, fault codes and flags are documented in the header of `tests/abft/frozen.om`.

| Rows | Content |
|---|---|
| 1–5 | Fault-free FP rows (bf16 and exact; odd shape 33×17×41). |
| 6–14 | Single bit. |
| 15–18 | Multi-element. |
| 19–22 | Stale. |
| 23–27 | Index. 26 is the out-of-bounds store and must exit 103. |
| 28–37 | Sync. 34/35 is the W8/W2 pair; 36 is the hang; 37 is a wait removal without the control flag and must be refused. |
| 38–39 | Unsupported sites. Must be refused with 69. |
| 40–42 | Declared misses and common-mode corruption. |
| 43–44 | Transient and persistent recovery. |
| 45–49 | Host cost. |
| 50–51 | Witness pair with a correct barrier. |

Config: S = 64, deadline 20 s, 1 retry, plan version 1.

## 7. CUDApp#55 schedule perturbation

Policies:

| Policy | Behaviour |
|---|---|
| 0 | Round-robin. |
| 1 | Seeded random choice among ready workers. Stream 2, varied by seed, rep and attempt. |
| 2 | Run-ahead: worker `tgt` goes first whenever it is ready. |
| 3 | Delay: worker `tgt` is postponed while it sits at `site`, provided another worker is ready. |

Sites: 1 load, 2 compute, 3 barrier arrival. Site 4 (intra-unit atomic) and site 5 (device
instruction stream) are **refused with 69**, because there is no host atomic and no qualified delay
opcode. Nothing is guessed (`docs/learn53/DEPENDENCIES.md` on the integration branch: "#55 NOT_RUN …
no delay opcode guessed").

Each row is validated before the first fork:

| Code | Meaning | Status |
|---|---|---|
| 0 | ok | 0 |
| 1 | unknown policy | 65 |
| 2 | target out of range | 65 |
| 3 | atomic site | 69 |
| 4 | device instruction stream site | 69 |
| 5 | wait removal without the known-bad control flag | 65 |
| 6 | shape or worker bounds | 65 |
| 7 | transposed index on a non-square shape | 65 |
| 8 | delay policy without a site | 65 |
| 9 | hang control with W < 2 | 65 |

The SCHED record reports the number of distinct schedules, by an FNV hash of the worker-choice
sequence, across reps.

## 8. CUDApp#56 reduced-concurrency witness

A row with flag bit 3 is the reduced-concurrency partner of the previous row: same seed and shape,
fewer workers (W8 → W2). The evaluator compares the first-attempt output digests rep by rep and the
admission detections:

| Verdict | Meaning |
|---|---|
| 1 | Counterexample: the original detected a fault and the reduced run detected none. The fault depends on concurrency. |
| 2 | The digests differ rep by rep, and there is no counterexample. |
| 0 | Otherwise. |

A reduced run that is clean is not proof that the original is correct. It is only a witness.

## 9. Output records and gates

Output is numeric, space separated, after the `COLUMNS` header lines printed by the driver. The
records are PLAN, CASE, SCHED, TALLY, WITNESS, RESOURCES and GATE. In the CASE record, detector
columns read 0 pass, 1 detect, 2 not evaluated (no usable output). Verdict codes: 0 ACCEPT,
1 RECOVERED, 2 REJECTED, 4 retry follows. Times are in ns from `monons`.

| Gate | Condition |
|---|---|
| 1 | No false positive in any detector, admission included. |
| 2 | Admission FN = 0, except the declared rows 11 and 17 (low-mantissa flips below the P1 bound). |
| 3 | Schedule status: row 37 → 65, rows 38/39 → 69, every other row 0. |
| 4 | Row 43 all RECOVERED and row 44 all REJECTED. |
| 5 | Resources: forks = reaped, fd first = fd last, no fd or wait errors. |
| 6 | Row 36: timeouts = 2 × reps (both attempts killed at the deadline). |
| 7 | Row 40: checksum FN = full TP = corrupt cases (deterministic rectangle miss). |
| 8 | Row 42 (common mode): digest TP = corrupt cases, and full FN + structural TP = checksum FN + structural TP = corrupt cases. |
| 9 | Row 51 witness: same = reps and verdict 0. |
| 10 | Row 26: every attempt ends in Omega trap 3 (exit 103) before the store. |

**Gate revisions after the first sandbox run.**

- **Gate 8** first required full FN = checksum FN = corrupt cases. The frozen row flips bf16 bit 14,
  which is the exponent MSB. In 2 of 10 sandbox attempts the corrupted A element overflowed the f32
  product to inf or NaN. The structural check flags those outputs, and full recompute then also
  fails them, because $|\hat c - s| = \infty$. The revised gate keeps the claim the row exists for:
  the digest is the only checker that sees finite common-mode corruption.
- **Gate 10** was added so the out-of-bounds exit status, previously only visible in CASE records, is
  gated.

## 10. Known misses and pending device evidence

**Known misses (declared before the first run).**

- f32 low-mantissa flips whose magnitude is under the P1 bound (rows 11 and 17). The bound allows
  them; they are undetectable at this precision by design.
- The checksum is looser than per-element recompute by roughly the row or column extent: the bound
  sums $|A||B|$ over N or M elements.
- The exact $2^{31}$ rectangle: a deterministic checksum miss.
- The f32 ±e rectangle (row 41): a miss when the random signs cancel. Up to 1/4 per side pair is
  expected; the measured rate is in the evidence.
- Common-mode input corruption: only the digest catches it.

**NOT_RUN (needs producer interfaces).**

- Device output acquisition from a GB10 GEMM launch, i.e. a GEMM launch hook or an output dump from
  gemm-any #283.
- Device-side #55 instrumentation and a qualified delay opcode.
- Atomic-site perturbation.
- A GB10 profile P2 bound check.
- Hardware fault injection.
