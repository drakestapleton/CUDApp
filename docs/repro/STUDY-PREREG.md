# Pre-registered migration study: automated route versus manual port (DRA-202)

Card cuda202 (kanban t_c2fbba86). Frozen when committed; any change after the first trial starts is a
new, dated amendment below the original text, never an edit of it. **Status: PROPOSED. No trial has
run. No participant has been contacted** (DRA-202 grants no outreach authorization; recruiting is
Drake's decision).

## Question

For one stated scope, what is R = automated-route cost / manual cost, with its uncertainty?

## Task (identical for both arms)

Starting point: CUDApp at `b774723f` (before S1): the three kernels exist as CUDA-free seeded Omega
SPIR-V builders and run on CPU drivers only; CUDA++ has its own sm_121 `vadd` (`examples/hello_gpu`).

Goal: the three kernels (vadd, fsum16, gemm16 as specified in `backend/spirv/conform.om` at `bec13796`,
including the frozen orders and no contraction) run on an NVIDIA GB10 and on an Apple M2 GPU, and pass
the quality target.

Quality target (both arms, checked by the same script, not by the participant): on each GPU, the six
output files (three real kernels, three planted-wrong mutants supplied by the study) have the sha256 in
`docs/repro/expected.sha256` `out/` lines, 10 fresh processes per kernel all equal; the no-CPU-fallback
control (a CPU driver selected must be refused) and the wrong-device control pass. Nothing else counts
toward the target (no performance requirement).

## Arms

| Arm | Route |
|---|---|
| A (automated) | an AI coding agent (named model and version, fixed system prompt and tool set) with the Omega toolchain and the CUDApp repository, starting from the starting point, no human code edits |
| M (manual) | a human engineer with the same starting point and the same hardware, any language and toolkit (CUDA, Metal, Vulkan, OpenCL, ...), no AI code generation |

Learning-order control: each participant (and each agent configuration) does two equivalent task sets in
counterbalanced order, set 1 = (vadd, fsum16, gemm16), set 2 = (vmul, fmax16 frozen-order row max,
gemm16 with B transposed), so a learning effect shows up as an order difference instead of an arm
difference. Arm A runs every order from a fresh session; nothing from one run is visible to the next.
Set 2's references, mutants and expected digests do not exist yet: they are added to `conform.om` and to a
frozen digest list, and committed, before any participant starts (study preparation, not a trial).

## Participants

Arm M: at least 3 engineers with at least 2 years of professional GPU programming, at least one GPU API
beyond CUDA (Metal or Vulkan), no prior contact with CUDApp or Omega. Expertise recorded by a short
questionnaire before the task. Arm A: 3 independent runs per order.

## Cost basis (recorded separately, never merged)

| Field | Unit | Arm A source | Arm M source |
|---|---|---|---|
| conversion, debugging, validation, deployment time | s | run wall-clock and per-phase timestamps | participant time log, 15-minute granularity, plus screen-session timestamps |
| human assistance | s | every human message or action during the run | help requested from anyone |
| compute | GPU-seconds, CPU core-seconds | lease records | the same tooling on the same machines |
| model cost | tokens and USD at a fixed, stated price table | provider counters | — |
| labour cost | USD at one stated loaded hourly rate applied to both arms | — | — |
| failures | count and time of runs that did not reach the target | included in the arm total | included |
| ongoing support | s over 90 days after delivery | logged separately, not in R | logged separately |

R = (A conversion + debugging + validation + deployment, time and USD) / (M same). Report R per task set
and order, the median over participants and runs, and the full range; with 3 per arm, report the exact
values, not a confidence interval that the sample cannot support. A single trial is reported as a single
trial. Compiler or emitter seconds alone are never the numerator.

## Analysis rules fixed now

- A run that does not reach the quality target within 40 working hours (M) or 8 hours (A) is a failure
  and enters with its full spent cost and a flag; it is never dropped.
- The below-1% hypothesis is supported for this scope only if every reported R value is below 0.01.
- No result generalizes beyond these three kernels, these two GPUs and these participants.

## Not decided here (needs Drake)

Recruiting the participants (external contact), their compensation, and the price table to use. Until
then the study is PROPOSED and R is NOT_RUN.
