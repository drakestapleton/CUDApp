# Migration cost of the portable application (DRA-202)

Card cuda202 (kanban t_c2fbba86). Everything in the measured tables is printed by
`sh tools/repro/cost.sh <pinned omc>` from public commits plus the committed effort file
`docs/repro/effort.tsv`; the exact output is `tests/repro/cost.expect`, and `tests/repro/run.sh` checks it
against `git diff --shortstat` and the published module digests. Arithmetic in this file was checked with
Wolfram (queries in `evidence/REPRO-COST-20261107/wolfram.txt`).

## Result in one paragraph

Moving the application from CPU drivers to two GPU vendors (scope S1) changed 120 files, +5959/−38 lines,
of which 112 files and 4968 added lines are evidence (logs and receipts); the 8 non-evidence files carry
+991/−38. The four modules that already existed (vadd and fsum16, OpenCL and Vulkan flavours) are byte
identical before and after: the port needed no change to the emitted kernels. The application source did
grow by one kernel (gemm16, +218/−15 in `backend/spirv/`; the 15 removed lines are usage notes and the
kernel dispatch rewritten to include it). Two agent runs did the work in 4787 s of wall-clock (1.33 h),
281 model calls and an estimated USD 27.39 at list price, with 48 s of exclusive GB10 time. Human
hands-on time and independent review were not measured. The cost ratio R against a manual migration is
**NOT_RUN**: no manual baseline exists yet (`docs/repro/STUDY-PREREG.md`), so nothing here supports or
refutes a below-1% claim.

## Scopes

| Scope | Commits (BASE..HEAD) | What it is |
|---|---|---|
| S0 | `c30c0be6..d89ec37e`, 3 commits (CUDApp #138) | the portable contract itself: target table, SPIR-V emitter, CPU-driver host gate |
| S1 | `b774723f..bec13796`, 3 commits (DRA-195 `33f15580`, DRA-201 `61b9a1ac`, `bec13796`) | the port to Apple M2 (MoltenVK) and NVIDIA GB10 (NVIDIA Vulkan) |

The metered change is the whole-tree diff BASE..HEAD (`git diff --numstat --no-renames`). Every path must
fall in one category or `cost.om` refuses the input: `backend/` = app, `tools/multitarget/` = launcher,
`tests/multitarget/` = test, `mk/` and `Makefile` = build, `evidence/` = evidence, `docs/` = docs.

## Measured: source change (MEASURED, from git)

| Category | S0 files | S0 +/− | S1 files | S1 +/− |
|---|---|---|---|---|
| app (`.om` application) | 4 | +2096 / −0 | 2 | +218 / −15 |
| launcher (interim C, migration-pending) | 2 | +346 / −0 | 1 | +326 / −12 |
| test | 1 | +241 / −0 | 3 | +430 / −6 |
| build | 3 | +23 / −2 | 1 | +10 / −0 |
| evidence | 27 (18 binary) | +319 / −0 | 112 | +4968 / −0 |
| docs | 1 | +29 / −3 | 1 | +7 / −5 |
| **total** | **38** | **+3054 / −5** | **120** | **+5959 / −38** |
| commits, span first author time to last commit time | 3, 33771 s (9.38 h) | | 3, 1387 s (23.1 min) | |

Commit span is not effort: it starts at the first commit, after the work behind it.

Emitted kernel modules (sha256 of `emit` output at BASE and at HEAD, compiled with the pinned omc):

| Module | S0 | S1 |
|---|---|---|
| vadd.cl, vadd.vk, fsum16.cl, fsum16.vk | new | **same** (byte-identical) |
| gemm16.cl, gemm16.vk | — | new |

## Measured: effort (S1 only)

From local Spark records copied into `docs/repro/effort.tsv` with their sources (kanban `task_runs`, Hermes
`sessions` / `session_model_usage`, spark-window `close.json`). The values are MEASURED counters; the dollar
figure is Hermes' ESTIMATE from the provider's published price table (`cost_status estimated`), not an
invoice.

| Kind | DRA-195 run (t_e1d7686d) | DRA-201 run (t_ebf8bda5) | S1 total |
|---|---|---|---|
| agent wall-clock | 3530 s | 1257 s | 4787 s = 1.33 h |
| model API calls | 185 | 96 | 281 |
| input tokens (uncached) | 372 | 194 | 566 |
| output tokens | 146351 | 111453 | 257804 |
| cache-read tokens | 59267382 | 28162608 | 87429990 |
| cache-write tokens | 551948 | 397438 | 949386 |
| estimated cost (list price) | USD 17.5417244 | USD 9.8495476 | USD 27.391272 |
| exclusive GB10 time | — | 48 s (window `20261107T013053Z-901cc9`) | 48 s |
| Apple M2 GPU time | not metered (no lease on the Mac; per-run timings in `m2-gpu/timing.tsv`) | | not metered |
| human hands-on | NOT_MEASURED (no intervention recorded in either run; earlier MacBook ssh/Lima setup not counted) | | NOT_MEASURED |
| independent review | NOT_MEASURED (reviewer replay NOT_RUN) | | NOT_MEASURED |

S0 effort: NOT_MEASURED (the #138 work predates the run records that exist on the Spark).

## DRA-202 cost categories against what is measured

| Category (DRA-202) | Measured here | Status |
|---|---|---|
| conversion | app +218/−15 (one new kernel; 0 bytes of change to the existing kernels), launcher +326/−12, build +10 | MEASURED (lines); time not separable from the agent runs |
| debugging | inside agent wall-clock | not separately measured |
| validation | tests +430/−6, evidence 112 files, GPU 48 s | MEASURED |
| deployment / packaging | this card (DRA-202): 3067 s, 117 calls, est. USD 13.66 at a 03:10Z snapshot (`evidence/REPRO-COST-20261107/README.md`) | MEASURED snapshot; final figure on the issue |
| tuning | none was done (no performance work in S1) | — |
| human assistance | — | NOT_MEASURED |
| compute | GB10 48 s exclusive; CPU time not metered | partial |
| ongoing support | — | NOT_MEASURED (no maintenance period has passed) |
| failures | both runs completed; refused/aborted attempts inside them are in their session logs, not counted separately | not separately measured |

## The ratio R

R = (conversion + debugging + validation + deployment via the automated route) / (the same for an
equivalent manual migration). The numerator above is partly measured; the denominator does not exist.
**R: NOT_RUN.** The study that would measure the denominator is pre-registered in
`docs/repro/STUDY-PREREG.md`. Until it runs, the "below 1%" figure is a hypothesis with no measurement
behind it, and a single trial would not generalize beyond its stated scope.

## Limits

- One trial per scope; the agent runs are single samples (no repeat, no variance).
- The automated route here is an AI agent plus a seeded-Omega contract that already existed (S0). S1's
  low cost depends on S0, whose effort is not measured.
- Line counts include generated and copied evidence; the evidence category is reported separately so it
  can be excluded.
- The token counters and estimated cost come from the Hermes records of the two sessions; they are not
  public. They are copied, with sources, into `docs/repro/effort.tsv`.
