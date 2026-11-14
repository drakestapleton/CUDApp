# Rung 3 design: several tokens per 27B pass, output identical to plain greedy

Lane `uxl-brief/SpecDecode`, 2026-11-13. Status of this file: PROPOSED design. What is already built and gated
is listed in section 8 and in `RESULT.md`. Target: `Qwen/Qwen3.8-27B-FP8` rev `017b9c7a`, launcher and kernels
at CUDApp `dc536882`, compiler omega-mv `2ee0a70d` (omc sha256 `26540e6d…8991`).

## 1. What the checkpoint already contains

`mtp.safetensors` (477,202,224 bytes, 22 tensors) is a multi-token-prediction head trained with the model
(`mtp_num_hidden_layers: 1`, `mtp_use_dedicated_embeddings: false`):

| Part | Tensor | Shape | Type |
|---|---|---|---|
| mix | `mtp.fc.weight` | 5120 x 10240 | BF16 |
| norms | `mtp.pre_fc_norm_embedding`, `mtp.pre_fc_norm_hidden`, `mtp.norm` | 5120 each | BF16 |
| one full-attention layer | `mtp.layers.0.self_attn.{q,k,v,o}_proj`, `mlp.{gate,up,down}_proj` | same shapes as layer 3 | FP8 E4M3 + 128x128 block scales |
| layer norms | `input_layernorm`, `post_attention_layernorm`, `q_norm`, `k_norm` | 5120, 5120, 256, 256 | BF16 |

It shares the 27B's embedding table and output head. Every kernel it needs is already among the 18 model
kernels (`fp8gemv_b128f`, `bf16gemv2`, `rmsnorm5120`, `attn256_*`, `mlpsilu`, `residual5120`, `argmax_*`,
`embed5120_step`). No new arithmetic is needed for the drafter, only new dispatches and one concatenation.

How it is driven (definition read from vLLM 0.33.0 `qwen3_5_mtp.py` and `llm_base_proposer.py`, used here only
as the interoperability definition of the checkpoint's own head):

1. Slot q holds `fc(concat(norm_e(embed(token q)), norm_h(hidden q-1)))`, where `hidden q-1` is the 27B's
   post-final-norm hidden state at position q-1 (the vector the output head reads).
2. The head's layer attends causally over its own slots, then `mtp.norm`, then the shared output head. The
   output at slot q is a guess for token q+1.
3. Further drafts chain: the next slot takes the drafted token and the head's own output vector in place of the
   27B hidden state, one position later.

## 2. One pass

A pass feeds R = k+1 rows to the 27B. Row 0 is the last token already emitted; row j (1..k) is draft `d[j-1]`.
`g[j]` is the 27B's greedy token after row j.

```
pass:
  embed rows 0..k                                  (R rows)
  64 layers: GEMVs and batched maps once for all R rows (weights read once), row-ordered kernels once per row (section 3)
  final norm, output head, argmax for R rows       -> g[0..k]
  accept:   a = longest prefix with d[j] == g[j]   (spec.om specacc)
  emit:     n = tokens g[0..n-1], n <= a+1, cut at the request's room and after a stop token (spec.om specemit)
  commit:   position += n; convolution and recurrent states are rings indexed by position (section 4); nothing is copied
  draft:    multi-token head from hidden row n-1 and token g[n-1] -> d[0..k-1] for the next pass (section 5)
```

Why the output is the plain greedy output: `g[0]` depends only on committed state and the last emitted token,
so it is the token plain greedy emits next. `g[j]` is computed with rows 1..j in context; it is the plain
greedy token exactly when those rows equal `g[0..j-1]`, which is what `specacc` requires. R is fixed per plan
(R = k+1, a compile-time constant of each R-row kernel). The first pass after the prompt therefore also runs R
rows, with placeholder drafts that are not trusted: `specacc` accepts none unless they happen to equal the
target, and either way the emitted tokens are the plain greedy ones (the S1 gate's always-wrong drafter is
exactly this case).

The whole pass is one fixed dispatch list, replayed every pass like today's step. Acceptance runs on the card
(a new small kernel `spec_accept`, section 6), so there is still one doorbell and one completion signal per
pass and no host round trip between verification and drafting.

## 3. Rows in the kernels (kernel owner AmdKernelSpeed; R-row variants written in this lane, as agreed)

AmdKernelSpeed, in a message to this lane on 2026-11-13: "write the R-row variants yourself, on a local branch in your own folder, as new kernel
names. I will review them against my v2 tables." Their v2 work changes load scheduling only, not reduction or
product order; until their gdnstate_step v2 gate passes, the R-row work matches v1 (`dc536882`).

- **Shape.** Each R-row kernel is a new kernel `NAME_rR` with R a compile-time constant (one object per R). The
  kernarg ABI is the one-row kernel's; activations are R x K f32 (row stride 4K bytes) and outputs R x M f32
  (row stride 4M bytes). Packet templates stay constant.
- **Weights once.** For the matrix kernels (`fp8gemv_b128f`, `bf16gemv2`) each wave loads its weight words (and,
  for FP8, decodes each byte and loads its block scale) once and applies them to all R activation rows. Input
  rows are R x 5120 x 4 B (164 KB at R = 8) or R x 17408 x 4 B (557 KB). VGPRs grow by 24 per added row plus
  two pointer-stride registers (allocated, from the S2 report: fp8 48 to 80 / 104 / 224 at R = 2 / 3 / 8; bf16
  40 to 72 / 96 / 216), which lowers occupancy: whether that costs time is a card measurement, NOT_RUN.
- **Row identity (the gate, built: section 8, S2).** Row r of an R-row dispatch is bit-identical to the
  one-row kernel run on row r alone. The generator (`kernels/gen_rows.py`) gets this by construction: every
  instruction touching row 0's activation, partial, accumulator, reduction temp or output is repeated for each
  further row on its own registers, right after the original, with the same other operands.
  This is stricter than equal tokens on purpose: with near-ties of 0.01 to 0.19 logit already on record
  (brief, lines 97 to 100), only bit-identical rows make "identical to plain greedy" a property of the
  construction and not of the prompt.
- **Rows that depend on earlier rows.** These keep their one-row kernels and run once per row, in row order,
  each row's dispatches reading its own position cell `pos_r = p + r` (R cells written by `spec_accept`):
  - `attn256_prep_step`, `attn256_part_step`, `attn256_merge_step`: row r writes K/V at `p + r` and attends to
    `0 .. p + r`. Position-indexed already; unchanged.
  - `gdnconv4_step` and `gdnstate_step`: two parity banks cannot keep a rejected row's predecessor (row r+2
    overwrites the bank row r wrote). `gdnconv4_step_ring16` and `gdnstate_step_ring16` (built, S2b) replace only
    the prologue: input slot `position mod 16`, output slot `position+1 mod 16`, in one buffer of 16 slots
    (81,920 B and 3,145,728 B per slot). The arithmetic after the prologue is the reviewed kernel's.
- **Row-independent maps.** Measured on the seeded executor (S2c), with the reviewed objects unchanged:
  `rmsnorm5120` (its `rows` argument) and `mlpsilu` (its `count`) already process R contiguous rows in one dispatch,
  byte-identical to R one-row dispatches. `residual5120`, `gdnrmsgate` and `attn256_gate` do not (one dispatch over
  a larger grid differs), nor does `gdnqknorm` (the measurement's control); `gdnparams` (48 values per row in one
  64-lane group) is [INFERENCE] not row-contiguous, not measured. Those run once per row with row-offset kernargs
  (identical by construction) until they get R-row variants. For `residual5120`, `attn256_gate` and `gdnrmsgate`
  those exist now (S2d, `kernels/gen_ymaps.py`): the y guard becomes y < R and the per-row pointers get
  group_y x row stride in the prologue, so one dispatch covers R rows.

Dispatch count matters: with the row-native maps (S2c) and the y-grid maps (S2d) batched and the rest per row,
verification at R rows is 497 + 385 + 244 R dispatches (Wolfram, `model/wolfram.md` Q6: 1,126 at R = 1, 1,614 at
R = 3, 2,834 at R = 8), plus 23 per draft for the head (9 GEMVs and 14 small kernels, the terms of `model/wolfram.md` Q4). The 244 per-row dispatches are the row-ordered
kernels (attention steps, convolution, recurrent state), `gdnqknorm`, `gdnparams`, embedding, argmax and the step
counter; their measured one-row time is 4.66 ms, 3.10 ms of it `gdnstate_step`. That per-row time, not weight
traffic, decides what this rung buys at today's kernel speeds (`model/MODEL.md`).

## 4. State: keep n rows without copying

| State | Where | Rule |
|---|---|---|
| Attention K/V (16 layers) | by position, as today | rows write `position + r`; commit is `position += n`. Entries past the new position are stale and are overwritten before anything can read them, because row r reads only up to its own position. |
| Convolution window (48 layers) | ring of 16 slots per layer, 81,920 B each (`gdnconv4_step_ring16`) | same rule as the recurrent state. Gated (S2b). |
| Recurrent state (48 layers) | ring of 16 slots per layer, 3,145,728 B each (`gdnstate_step_ring16`) | row at position q reads slot `q mod 16`, writes slot `q+1 mod 16`; keeping n rows is `position := p + n`, nothing is copied. Gated (S2b): after 8 rows every prefix state n = 0..8 is still in its slot. |
| Step counter | `ctl.position` and R cells `pos_r` | `spec_accept_rR` adds n to `ctl.position` (built, S3a); a small map then writes `pos_r = position + r` for the row-ordered kernels (not built). |
| Head K/V (drafter) | by position, own buffers | slots at or below the committed position hold true-context entries; draft entries live above it and are rewritten by the next pass. |

Memory: recurrent ring 48 layers x 16 slots x 3,145,728 B = 2,415,919,104 B and convolution ring 48 x 16 x 81,920 B =
62,914,560 B (R up to 15), on a card that today uses 37.6 GB of 192 GB. No rollback pass exists: a rejected row costs nothing after the pass.

Edge cases (the first six are rows of the S0/S1 gates and of S3a; the last is not built):

- no draft accepted: n = 1, the pass is a plain step;
- all accepted: n = k + 1;
- a draft that equals the target after an earlier mismatch is not accepted;
- stop token inside the accepted run: emit through it and stop, later rows discarded;
- fewer tokens left than the pass could yield: cut at the room, state kept for exactly n rows;
- first pass after the prompt: R rows with untrusted placeholder drafts (accepted only if equal to the target);
- draft ids are not trusted input: `embed5120_step` must refuse or clamp an id outside 0..248319 the same way
  for every row (the drafter's argmax cannot produce one, a wrong buffer could).

## 5. Drafter on the card

After `spec_accept` the pass runs the head as more dispatches of the same kernels:

1. Base slots for the n committed positions: `rmsnorm5120` twice (embedding and hidden), concatenate,
   `bf16gemv2` with `mtp.fc` (n rows), `rmsnorm5120`, `k_proj` and `v_proj`, prep into the head's K/V.
2. Draft 1: the full head layer on the last of those rows (q, attention over the head's K/V, gate, `o_proj`,
   MLP), `mtp.norm`, output head, argmax.
3. Drafts 2..k: the same with the previous draft token and the head's previous output vector.

Bytes read per draft: 477 MB of head weights plus the 2.54 GB BF16 output head, which is 11.2% of a full pass
(Wolfram, `model/wolfram.md`). The output head dominates, and drafts do not have to be exact, only often
right. Two cheaper heads are therefore in the model:

- **Shortlist head:** argmax over the first N rows of the output head only (N = 32768 is 335 MB at BF16).
  Acceptance measured (`acceptance/`, variant `ids<32768`).
- **FP8 output head for drafting only** (1.27 GB), built once at load from the BF16 head. Verification keeps
  the BF16 head, so emitted tokens do not change. Cost modeled only; its acceptance is NOT measured.

Alternative drafters with the same tokenizer are listed in `metadata/drafts.tsv`. The block drafters
(DFlash2, DSpark: 5 layers, 1.9 B parameters, 7 drafts in one pass) cost one read of 3.8 GB per pass instead of
k reads of the head; they need the 27B's hidden states at layers 5, 19, 33, 47, 61 and are third-party
weights. They are an option after the head, not part of this build.

## 6. Launcher and plan changes (owners: AmdKernelSpeed for the plan, AmdLaunchGap for the host path)

AmdLaunchGap, 2026-11-14: write the host loop on a local branch in this lane's folder, based on their head
`737aff5e` (branch `amd-launch-gap`, which adds `plan2`/`planscope`/`planstage` publication). Their constraint: in
`plan2` the last packet of a pass carries the system-scope release and the completion signal and `planheader`
picks it as `i == ps 6 - 1`. This design keeps one fixed dispatch list per pass (constant `ps 6`), with
`spec_accept` mid-pass and the drafter after it, so the host reads `emit_buf` only after that release.

Plan format (proposed OMPLAN02, a superset; OMPLAN01 plans stay valid):

- header gains `rows` (R), `emit_buf` (GTT buffer index) and `ctl_buf`; `n_gen` keeps its meaning;
- `emit_buf` layout, little-endian: u32 `n`, u32 `a`, then R x i32 token ids (`g[0..n-1]`), written by
  `spec_accept_rR`;
- `tok_buf` becomes R x i32 (row 0 = last emitted token, rows 1..k = drafts), written by the device: row 0 by
  `spec_accept_rR`, drafts by the head's `argmax_final` dispatches. The host writes it only for prompt steps;
- `ctl_buf`: u32 `room` (tokens the request may still take), i32 `stop` (token id or -1), u32 `position`;
  `spec_accept_rR` adds n to `position` and subtracts n from `room`. The R per-row position cells `pos_r` used by
  the row-ordered kernels are `position + r`, written by one small map after `spec_accept_rR` (not built).

`spec_accept_rR` is built and gated (S3a): kernarg {g, d, ctl, emit, tok}, one wave, integer only, lane 0 stores.
`om/spec.om` is its executable specification.

Host loop (`plan.om`, `planrun`): a prompt step is unchanged. A decode pass publishes the same packet list,
waits for the one completion signal, then reads `emit_buf`: `n` tokens are appended to the id list and
`OMEGA_STEP i tok=T us=U` is printed once per pass with `n=` and `a=` added. The loop ends when the id list
holds `n_gen` tokens or the last token is the stop token. `recordtoken` takes a count. Timing boundary is
unchanged (first packet write to completion signal), now per pass; ms per token is the sum of pass times over
tokens emitted, and the report must say so. Testing it needs the CPU gate's simulated device to write `emit_buf`
per pass (today it checks the queue image and fixed token bytes); that instrument change is part of S3b.

## 7. Gates on the way to the card

| Stage | What must hold | Where it runs | State |
|---|---|---|---|
| S0 | `specacc` / `specemit` equal two independent formulations on every case; 5 source mutants caught | Omega, CPU | PASS (section 8) |
| S1 | speculative decode equals plain greedy token for token and state for state on a model with all three state kinds, k = 1..8, six drafters; 8 mutants caught | Omega, CPU | PASS (section 8) |
| S2 | every R-row GEMV is bit-identical per row to its one-row form and to the exact oracle; mutants differ | seeded gfx942 CPU executor | PASS for `fp8gemv_b128f_r{2,3,8}`, `bf16gemv2_r{2,3,8}` |
| S2b | ring-state kernels equal the reviewed two-bank chain row for row and keep every prefix state | seeded gfx942 CPU executor | PASS for `gdnconv4_step_ring16` and `gdnstate_step_ring16` |
| S2c | which reviewed map kernels already take R rows in one dispatch | seeded gfx942 CPU executor | PASS (measurement): `rmsnorm5120`, `mlpsilu` yes; `residual5120`, `gdnrmsgate`, `attn256_gate` no |
| S2d | `residual5120`, `attn256_gate`, `gdnrmsgate` take R rows with grid y = R, byte-identical to R one-row dispatches | seeded gfx942 CPU executor | PASS for R = 2, 3, 8 |
| S3a | `spec_accept_rR` equals the S0-proved formulation of `om/spec.om` | seeded gfx942 CPU executor | PASS for R = 2, 3, 8 |
| S3b | OMPLAN02 parser, host loop, R-row plan emitter: CPU launcher gate with malformed-plan controls and the owner's existing checks unchanged | CPU instruments | NOT_RUN: not built |
| S4 | on the MI300X: 256 lighthouse ids and the 24 coding ids identical to the published plain run for k in {0, 1, 2, 4, 8}; wrong-weights control still diverges; a plan whose drafter is forced always wrong still gives the same ids at one token per pass; measured ms per token beside AMD's own software in the same session | card | NOT_RUN |

S4's always-wrong-drafter row is the control that separates "the verification is right" from "the drafter
happened to agree".

## 8. Built and gated so far

See RESULT.md section 5 for the files, commands, hashes and verdicts of S0, S1, S2, S2b, S2c, S2d and S3a.

## 9. What implementation changed (2026-11-14, lane SpecDecode2)

- **Plan emitter.** No new emitter: `plan/rdriver.py` is a driver module for the repository's unmodified
  `emit-plan.py`. It re-exports the published `driver-real2.py` and replaces `Card` by `RowCard` when `SPEC_ROWS` >= 2.
  With `SPEC_ROWS` unset the emitted lighthouse plan is the published `real2.plan` byte for byte (sha256 `2a292dad…`).
  `plan/to_omplan02.py` changes only the header, the emit/ctl buffer rows and the trailer.
- **Dispatch count.** Emitted: 884 + 243 R per pass (1,370 at R = 2, 1,613 at R = 3; plain 1,126). Section 3's
  497 + 385 + 244 R (Wolfram Q6) agrees at R = 2 only; the model must use the emitted counts.
- **Position cells.** A new integer kernel `spec_pos_rR` (first dispatch of a pass) writes pos[r] = ctl.position + r.
  The row-ordered kernels read pos[r]; `step_advance` is gone from R-row plans (`spec_accept_rR` advances the position).
- **Head drafter (k = 1 only).** The head runs on every row r, not only on the last kept row, so the dispatch list stays
  fixed: slot p+r+1 = fc([norm_e(embed(g[r])), norm_h(hidden_r)]) through the head layer (its own K/V by position, cells
  pos[r+1] from `spec_pos_r(R+1)`), `mtp.norm`, the shared output head and argmax per row into c[r]. After
  `spec_accept_rR`, a new integer kernel `spec_select_rR` writes token row 1 = c[n-1]. That costs 33 extra dispatches at
  R = 2 (8R + 17), not section 5's 23 per draft, and one extra pass over the 2.54 GB output head. Slots whose row was
  rejected are rewritten before any committed row reads them.
  Known difference from vLLM: in prompt passes the head's slot uses the model's own guess g[0] instead of the next
  prompt token, so the head's context over the prompt differs. Output is unaffected; acceptance may be lower than the
  CPU measurement. Drafts 2..k need a row gather (not built).
- **Host loop.** OMPLAN02 as proposed in section 6, implemented in `plan.om` (branch `spec-decode2`, commit f859c88f).
  Additions: `n_gen` must fit the u32 room cell (refused as `count`), and runtime refusal exit 76 when `n` breaks the
  contract.
