# AMD Developer Cloud: MI300X access packet (DRA-204, for DRA-193)

State on 2026-11-07: **PACKAGE_READY**. Not submitted, no account, no credit, no instance, no spend.
Every AMD hardware row is NOT_RUN. CUDA++ has no AMD execution path yet: `backend/targets.om` lists
`amd-gfx9` (gfx900/gfx90a/gfx942, MI300) as PLANNED, hardware NOT_RUN, and the only AMD object in
the tree is the LLVM oracle fixture `backend/amd/vadd-gfx1151.s`.

## The program (read 2026-11-07)

Sources: [AMD Developer Cloud](https://www.amd.com/en/developer/resources/cloud-access/amd-developer-cloud.html)
(page and FAQ), its [terms of use](https://www.amd.com/content/dam/amd/en/documents/developer/cloud/developer-cloud-terms-of-use.pdf),
the [getting-started article](https://www.amd.com/en/developer/resources/technical-articles/2025/how-to-get-started-on-the-amd-developer-cloud-.html)
(30 June 2025) and [DigitalOcean GPU Droplet pricing](https://www.digitalocean.com/pricing/gpu-droplets).

- **Who:** "independent developers and open-source project contributors involved with ML, low-level
  programming, and AI applications development". CUDA++ and Omega are Apache-2.0 public repositories
  by one independent developer, so the stated audience fits. Approval is "in [AMD's] sole discretion,
  based on the intended use case ... and a detailed description of how you plan to use the GPU credit."
- **What:** $100 of complimentary credit to members of the AMD AI Developer Program, usable only on
  Instinct MI300X GPUs. Not for attached volumes, object storage or backups.
- **Clock:** "Complimentary credit hours expire thirty (30) days from the date of deposit ... unless
  stated otherwise in your credit confirmation email." Deposit is within three business days of
  approval. The same FAQ says credits "are activated once cloud account is created or logged into via
  $100 credit link". The Linear text said 30 days from activation; the FAQ says from deposit. Treat
  the confirmation email as the authority and record its date. (The older 25-hour offer in the 2025
  article expired 10 days from deposit.)
- **Machine:** a VM on a third-party cloud (DigitalOcean). 1x MI300X: 192 GB GPU memory, 20 vCPUs,
  240 GB RAM, 720 GB boot NVMe, 5 TB scratch NVMe. 8x MI300X: 1536 GB GPU memory, 160 vCPUs. Images:
  bare Ubuntu, or "Quick Start" with ROCm preinstalled (vanilla, or with framework containers). Access
  is SSH as root, a web console or JupyterLab.
- **Billing traps:** a powered-off VM is still billed until it is destroyed. When the credit runs out
  or expires with no payment method on file, the VM is destroyed and its data is lost. An 8x VM uses
  credit 8 times as fast.
- **Price if paid:** DigitalOcean on-demand MI300X $2.59 per GPU-hour, billed per second with a
  5-minute minimum (pricing page, effective 1 August 2026, read 2026-11-07). A secondary source
  (spheron.network) reports $1.99 per GPU-hour billed on AMD Developer Cloud on 17 September 2026;
  unverified.
- **Terms to read before accepting:** the terms forbid using the cloud to develop inventions derived
  from the AMD Developer Cloud or its hardware and software in order to seek patent protection, or to
  analyse or modify patents. Texas law. You keep the rights to your work.

## Cost plan (no spend authorized by this packet)

All arithmetic in Wolfram (`WolframLanguageEvaluator`, 2026-11-07):
`credit=100; r1=259/100; r2=199/100; plan={3/2,6,2}` gives

| Quantity | Value |
|---|---|
| GPU-hours the $100 credit buys at $2.59 | 38.61 |
| GPU-hours at $1.99 (unverified rate) | 50.25 |
| Wall hours on an 8x VM at $2.59 | 4.83 |
| Planned GPU-hours, W0 + W1 + W2 = 1.5 + 6 + 2 | 9.5 |
| Planned cost at $2.59 / at $1.99 | $24.61 / $18.91 |
| Credit left after the plan at $2.59 | $75.40 |
| Hard stop: 12 GPU-hours at $2.59 | $31.08 |

Only the 1x configuration is used. The hard stop is enforced by the operator watching the console;
budget alerts are not a cap. Paid overage is not authorized.

## When to apply

The 30-day clock starts at deposit, and the windows that need the credit most (W1, W2) need host
work that does not exist yet (below). Recommendation: **apply when DRA-193's host gate for gfx942 is
green**, so W0, W1 and W2 all fall inside one 30-day credit. If Drake wants an access receipt sooner,
W0 alone on pay-as-you-go costs at most 1.5 x $2.59 = $3.885 (Wolfram: `3/2*259/100` = 777/200) and
needs his card on DigitalOcean. Both options are in `QUESTIONS.md`.

## Sign-up steps (Drake, under his own identity)

1. Read the terms PDF above, including the patent clause.
2. Join the AMD AI Developer Program: https://developer.amd.com/ai-developer-program (AMD single
   sign-on account, email verification, program terms).
3. Sign in to https://devcloud.amd.com with the same AMD sign-on.
4. Open the credit request and paste the answers below. The form sits behind the login, so its exact
   fields are NOT_READ; an earlier internal draft, citing an AMD credit article, listed affiliation,
   job function, intended use and public profile.
5. Wait for the decision email. On approval, record: amount, deposit date, expiry date, and whether a
   payment method was required. Put these in DRA-204.
6. Only when W0 is scheduled: create a new SSH key pair for the window, add the public key, create
   one **1x MI300X** GPU Droplet with the vanilla ROCm Quick Start image (or bare Ubuntu plus a pinned
   ROCm), nothing else attached.

## Application answers (draft for Drake)

- **Affiliation:** Independent (Aien). [DRAKE: confirm wording]
- **Job function:** Software engineer / researcher. [DRAKE: pick the closest option]
- **Public profile:** https://github.com/aien-dev
- **Intended use:**

> I develop CUDA++ (github.com/aien-dev/CUDApp) and the Omega compiler (github.com/aien-dev/omega-mv),
> an open-source (Apache-2.0) GPU toolchain that writes GPU machine code itself, without a vendor
> compiler or user-space GPU libraries. Today it runs only on one NVIDIA GPU, and AMD Instinct is the
> second target on its roadmap. I would use the credit on one MI300X, in three short windows: first a
> device inventory and a rocBLAS reference run; then my own gfx942 code objects for vector add,
> reduction and matrix multiply, compared bit for bit with a host reference, with planted-wrong
> controls that must fail, and timed against rocBLAS on the same GPU; then an independent replay.
> Each window has a written plan and leaves a public receipt with exact versions and logs, including
> the failures. I do not claim an AMD port exists yet; this credit is for the first real hardware test.

Keep every sentence true on the day it is sent: if the gfx942 path is not on main by then, say it is
in progress and send the link to its pull request.

## Bounded validation plan

| Window | Rows | Ready when | GPU-hours |
|---|---|---|---|
| W0 access test | `tools/access/plans/amd-mi300x.tsv`: inventory (rocminfo, amd-smi, /dev/kfd, ROCm version), reference (rocBLAS SGEMM with CPU verification, SGEMM 4096 timing, bandwidth) | now | at most 1.5 |
| W1 candidate | gfx942 vadd, fsum16 and SGEMM from CUDA++, bit compare with host references (`backend/spirv/conform.om` for vadd and fsum16; a GEMM reference with a written tolerance still has to be added), planted-wrong controls, paired timing against rocBLAS on the same device; then the selected shared-model forward (DRA-193) | the DRA-193 host gate below is green | at most 6 |
| W2 replay | W1's plan file rerun by a different agent from the receipt alone | W1 accepted | at most 2 |

W1 and W2 rows are already in the plan file as `MISSING_IMPLEMENTATION` with their blockers, so a
W0 receipt shows them as not run instead of hiding them. A row changes only in a reviewed change,
after its host gate passes.

What DRA-193 host work must deliver before W1 (none of this exists today):

1. A first-party gfx942 code-object writer in `.om` under `backend/amd/`, admitted by the existing
   `backend/targets.om` loader checks (the ones the multitarget gate already runs on gfx1151).
2. A launcher: either HSA runtime calls through Omega's checked shared-library calls (extension E2,
   omega-mv#888, open) or direct KFD ioctls with AQL packets (E1, ADR 0043). Until one exists, a
   migration-pending C launcher would need Drake's authorization (new C).
3. A native x86-64 build of the compare step, or copying outputs back so `conform.om` compares them
   on the Spark. The VM is x86-64 and the pinned omc writes AArch64 binaries.

## Teardown (every window)

1. `collect.sh run` finishes and prints `RECORDED`.
2. `scp -r` the run directory to the Spark. Check that `SHA256SUMS` verifies there (`close` does it).
3. Destroy the Droplet in the console. Powering off is not enough.
4. In the console, confirm no Droplet, volume, snapshot or reserved IP remains; read the credit
   balance and the window's charge.
5. `collect.sh close RUN ACCESS_TESTED|NONE <charged_usd> yes <who> <UTC time>` with `RCHECK` set.
   Commit the run directory under `evidence/ACCESS-amd-devcloud-<window>/`.

Owner of teardown: whoever opened the window, named in the TEARDOWN line. If the window is opened
by an agent with Drake's console access delegated, Drake still confirms step 4.

## If access is refused

Record the refusal (date, text) on DRA-204 and keep W0 rows as `BLOCKED_ACCESS`. DRA-201 needs one
non-NVIDIA device, not AMD in particular: the Intel packet (`INTEL-CLOUD-SERVICES.md`) and the Apple
M-series lane (DRA-195) are the alternatives. No capacity is bought without Drake's written approval.
