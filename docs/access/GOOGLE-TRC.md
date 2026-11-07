# Google TPU Research Cloud: proposal and feasibility packet (DRA-205)

State on 2026-11-07: **PACKAGE_READY**. Not submitted, no Google Cloud project, no quota, no spend.
Every TPU row is NOT_RUN. TPU is an exploratory target after AMD, Intel and Apple (DRA-192); this
packet prepares an application with a long lead time and does not admit a TPU implementation lane.

## The program (read 2026-11-07)

Sources: [TRC about](https://sites.research.google/trc/), [TRC FAQ](https://sites.research.google/trc/faq/),
[Cloud TPU pricing](https://cloud.google.com/tpu/pricing).

- **Who:** anyone may express interest; invitations go to approved applicants on a rolling basis.
  An earlier read of the form (2026-10-26) recorded: "TRC is currently oversubscribed", only one
  submission per person, and no status updates; it records the email of the signed-in Google account.
- **What:** free Cloud TPU quota, granted to the participant's own Google Cloud project on a
  temporary basis, ready within minutes of accepting the invitation. TRC quota cannot be used through
  Vertex AI; Google points participants to Compute Engine TPU VMs.
- **Not free:** "other GCP services are not" (VMs, Cloud Storage). In practice the project needs a
  billing account, so a card is on file even if the TPUs cost nothing [INFERENCE from the FAQ wording].
- **Obligations:** share the TRC-supported research (publications, open source, blog posts), give
  Google detailed feedback, follow the Google AI Principles, accept Google's terms and privacy policy.
  Suggested acknowledgement: "Research supported with Cloud TPUs from Google's TPU Research Cloud (TRC)".
- **Price if paid** (per chip-hour, on demand, read 2026-11-07): v5e $1.20 (us-central1), Trillium
  (v6e) $2.70 (us-east1). An earlier read (2026-10-26) also recorded that the Google Cloud free trial
  excludes TPUs.

## What we would run, and what it would and would not show

The route already exists on the host, in omega-mv `seed/stablehlo/` (README at main):
`irstablehlo.om` turns an Omega IR unit into StableHLO text; `gate-irstablehlo.sh` checks every row of
`cases.tsv` on two CPU oracles (IREE and the XLA CPU PJRT client) with exact comparison and mutants;
`pjrt-oracle.py` (existing interop driver, not Omega) compiles a module with a PJRT client and compares
the result bit for bit. Its `PJRT_ORACLE_PLATFORM=tpu` mode is the TPU receipt path; it has never run.

On a TPU VM, the plan `tools/access/plans/google-tpu.tsv` runs the modules built on the Spark (never
regenerated on the VM) through Google's TPU PJRT client:

- inventory: accelerator type from the metadata server, device nodes, jax/jaxlib/libtpu versions;
- reference: `jax.devices("tpu")` lists the chips;
- candidates: `vaddi32`, `vaddu32`, `vaddf32` and `vaddf64` (the f64 row records whether libtpu
  refuses or emulates it; f64 is not native on TPUs);
- controls: a wrong-op module and a wrong expected vector must mismatch (exit 1); a malformed module
  must be refused (exit 2).

Label for any pass: **EXERCISED on TPU via closed libtpu**. Below StableHLO, compilation and the
kernel are Google's (libtpu is a closed binary; no public TPU ISA). This is never an Omega TPU
backend, a TPU qualification or a firmware/driver result.

## Feasibility verdict (what DRA-205 decides after one window)

GO for a later TPU lane only if all hold: every integer and f32 candidate row PASS on the TPU with
its controls caught; the f64 behaviour recorded; the supported-operation list (`cases.tsv` plus the
translator's refusals) written down; and the dependency boundary stated (StableHLO text in, libtpu
below). Otherwise NO-GO with the failing rows. Either way the next step goes through DRA-162
admission after the priority backends.

## Cost and time plan

One window, one v5e or v6e TPU VM with one chip, at most 4 hours, artifacts copied back by scp (no
Cloud Storage bucket). With TRC quota the TPU costs $0; the project's other charges (disk, network)
are recorded from the billing console after teardown. If Drake ever prefers paid time instead of
TRC: 4 chip-hours of v5e on demand is $4.80 and of v6e $10.80 (Wolfram: `4*12/10`, `4*27/10`).
No paid spend is authorized by this packet.

## Sign-up steps (Drake, under his own identity; one submission per person)

1. Sign in to Google with the account that should own the Cloud project.
2. Open https://sites.research.google/trc/ and its sign-up form. Paste the answers below; fill the
   bracketed fields truthfully. Read the AI Principles, terms and privacy policy before ticking them.
3. Submit once. Record the date in DRA-205.
4. On invitation: create (or pick) a Google Cloud project with a billing account, accept the
   invitation for that project, and record the granted quota: TPU generation, zone, chip count and
   expiry.
5. Only when the window is scheduled: create one TPU VM in the granted zone with one chip, using the
   runtime version Google's TPU documentation names for that generation, for example
   `gcloud compute tpus tpu-vm create omega-trc-w0 --zone=<zone> --accelerator-type=<type> --version=<runtime>`.
   Install JAX with TPU support on the VM as Google's quickstart says, copy the bundle, run the plan.
6. Delete the VM (`gcloud compute tpus tpu-vm delete omega-trc-w0 --zone=<zone>`), confirm in the
   console that no TPU VM, disk or IP remains, and close the receipt.

## Form answers (draft for Drake)

- **First / last name:** Drake Stapleton
- **Affiliation:** Independent (Aien). [DRAKE: confirm; do not name an institution you do not belong to]
- **Job title / role:** [DRAKE: e.g. independent researcher, only if true]
- **Country:** [DRAKE]
- **How did you hear about TRC:** [DRAKE]
- **Project description:**

> Omega (github.com/aien-dev/omega-mv) is an open-source (Apache-2.0) programming language and
> compiler whose seeded compiler is written in Omega. I am investigating portable ML compilation: the
> same Omega programs lowered to several accelerators, each result checked against an independent
> reference with planted-wrong controls. Omega already translates a bounded set of tensor operations
> to StableHLO, checked exactly on two CPU implementations (IREE and the XLA CPU PJRT client). I have
> never run it on a TPU. With TRC quota I would (1) run those StableHLO modules through the supported
> Cloud TPU PJRT runtime and compare every element with the CPU results, recording which operations
> and element types the TPU accepts (for example f64); (2) extend the operation set to matrix
> multiply and reductions with the same checks; (3) evaluate one small training or inference step.
> Timeline: (1) in the first month, (2) and (3) over the following two months. Every gate script,
> receipt and negative result will be published on GitHub, with written feedback to the TRC team.
> This proposal does not assume an existing TPU backend.

## Teardown owner

Whoever creates the TPU VM deletes it in the same sitting and names themselves in the TEARDOWN line.
Drake confirms in the billing console that the project shows no running TPU VM afterwards.
