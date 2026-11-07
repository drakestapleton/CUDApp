# Access packets for non-NVIDIA cloud hardware

Status on 2026-11-07: **PACKAGE_READY** for all three providers below. Nothing has been submitted,
no account was created, no credit was activated, no instance exists and no money was spent. Every
hardware row for AMD, Intel and Google TPU is NOT_RUN. These packets prepare the work; they are not
evidence that any of it ran.

CUDA++ runs only on the NVIDIA GB10 today (`backend/targets.om`: one SUPPORTED row, every other GPU
PLANNED, hardware NOT_RUN). These packets prepare the first real device time on three other vendors,
so that a time-limited credit is not spent on setup.

| Provider | Packet | Hardware | Linear | Cost to us | State |
|---|---|---|---|---|---|
| AMD Developer Cloud | [AMD-DEVELOPER-CLOUD.md](AMD-DEVELOPER-CLOUD.md) | 1x Instinct MI300X (gfx942) VM | DRA-204, then DRA-193 | $100 credit if approved; whether a card must be on file is not verified | PACKAGE_READY |
| Intel Cloud Services | [INTEL-CLOUD-SERVICES.md](INTEL-CLOUD-SERVICES.md) | Arc Pro B-Series (Xe2) instance, as listed on the public portal | DRA-206, then DRA-194 | evaluation access; no price is published | PACKAGE_READY |
| Google TPU Research Cloud | [GOOGLE-TRC.md](GOOGLE-TRC.md) | Cloud TPU VM (v5e or v6e), one chip | DRA-205 | free TPU quota if accepted; VMs, disks and storage billed | PACKAGE_READY |

## States

Each provider moves through these states, and each needs its own evidence. One never implies the next.

| State | Evidence that moves it |
|---|---|
| PACKAGE_READY | this packet, merged |
| SUBMITTED | Drake submitted the application under his own identity; date and confirmation recorded |
| APPROVED | the provider's written approval: amount, hardware, expiry |
| ACTIVATED | the credit or quota is visible in the account; the expiry clock is running |
| ACCESS_TESTED | an ACCESS1 receipt for window W0 that `tools/access/rcheck.om` accepts with `claim=ACCESS_TESTED` |

ACCESS_TESTED means the device answered and the vendor's own stack ran on it. It is not a CUDA++
backend result and not a qualification. Backend qualification stays with DRA-193 (AMD) and DRA-194
(Intel); the TPU go/no-go stays with DRA-205.

## Shared rules for every window

1. **No spend without a written number.** A window runs only against a credit or quota already
   granted, or a spend Drake has approved in writing with a dollar cap. Budget alerts are not caps.
2. **Prepare first, activate last.** Credits expire (AMD: 30 days from deposit; Intel reservations:
   30 days at most). Apply when the rows the credit is for are ready on the host, not before.
3. **One plan file per window.** The window runs the rows of a `tools/access/plans/*.tsv` file and
   nothing else. A new experiment is a new plan file in a reviewed change.
4. **Receipts leave before teardown.** Copy the run directory back to the Spark, then destroy the
   instance, then check in the provider console that it is gone and that no disk, IP or bucket is
   left. Powered-off AMD instances are still billed.
5. **The checker decides the claim.** `collect.sh close` writes the receipt; `rcheck.om` accepts or
   refuses it. A refused receipt is kept, with its refusal, never edited.
6. **No outside model weights.** No window downloads or runs any outside model. The own Omega LLM
   is the only model; a model row needs its frozen manifest first (DRA-193, DRA-194).
7. **No custom driver or firmware assumption.** A cloud VM gives the vendor's kernel driver. Rows
   that need Aien's own driver are out of scope for these windows.

## Files

| Path | What it is |
|---|---|
| `docs/access/RECEIPT.md` | the ACCESS1 receipt format and the rules rcheck enforces |
| `tools/access/rcheck.om` | the receipt checker, seeded Omega, pinned omc `2c824805…` |
| `tools/access/collect.sh` | records one window on the VM (`run`) and closes it on the Spark (`close`) |
| `tools/access/plans/amd-mi300x.tsv` | AMD W0 rows |
| `tools/access/plans/intel-gpu.tsv` | Intel W0 rows |
| `tools/access/plans/google-tpu.tsv` | TPU W0 rows, including the first StableHLO candidate rows |
| `tests/access/run.sh` | host gate: fixtures, one-line mutants, a checker mutant and a collector self-test |

`collect.sh` is POSIX shell because it has to run on the provider's x86-64 VM, where the pinned
omc's AArch64 binaries do not run. It only runs commands and records exit codes. Every rule about
what a receipt may claim is in `rcheck.om`, which runs on the Spark.

Run the host gate (Linux AArch64, pinned omc):

```
sh tests/access/run.sh /absolute/path/to/omc    # omc sha256 2c82480596bbdc50…5029f
```

It ends `ACCESS GATE PASS pass=<n> fail=0` and exits 0. It opens no GPU and uses no network.
Host receipt: `evidence/ACCESS-GATE-20261107T0127Z/` (52 of 52 at `cce2886e`, host CPU only).
