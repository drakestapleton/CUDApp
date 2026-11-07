# Hardware capability manifest and device lifecycle (DRA-400, DRA-401)

`manifest.tsv` registers every processor family Omega is asked to reach. `hwman` (seeded Omega:
`core.om`, `cpu.om`, `tpm.om`, `main.om`, concatenated in that order and compiled by the pinned omc)
checks the register, answers admission queries and runs the admitted targets through one typed lifecycle.
GPU execution stays in `backend/targets.om` and the CUDApp runtime; the GPU row here only records its
status and why `hwman` will not run it.

Build: `cat backend/hw/core.om backend/hw/cpu.om backend/hw/tpm.om backend/hw/main.om | omc > hwman`.
Gate: `make test-hw OMC=/abs/omc` (`tests/hw/run.sh`).

## Manifest

Line 1 is `hwman<TAB>1`; line 2 is the header below. Every row has exactly 25 tab-separated, non-empty
fields (`-` means none).

| Field | Meaning |
| --- | --- |
| `id` | lowercase `[a-z0-9-]`, unique |
| `family` | CPU GPU APU PPU SPU NPU TPU LPU MCU ISP TPM SECURE_ENCLAVE ASIC FPGA DPU WSE NEUROMORPHIC PHOTONIC QPU TSU BPU; all 21 must appear |
| `vendor`, `part`, `revision` | exact device, or `unresolved` |
| `route` | NATIVE_ISA, DIRECT_DEVICE, VENDOR_RUNTIME, REMOTE_SERVICE, FIRMWARE_OWNED, UNRESOLVED |
| `platform`, `identity`, `node` | where it lives, the identity discovery must read back (`midr_el1=0x…`, `manufacturer=0x…`, `uuid=…`), and the device node |
| `isa_cmds`, `discovery`, `ops` | instruction set or command format, how it is found, comma-separated operations |
| `addr_spaces`, `dma_coherence`, `queue_irq`, `reset_power`, `firmware` | the device's memory, ordering, submission, reset and firmware ownership |
| `compiler_artifact`, `refs` | what produces its code; primary sources |
| `runner`, `timeout_ms` | the executor (`hwman-cpu`, `hwman-tpm`, or an outside authority such as `spark-window`) and the wall-clock budget |
| `status` | CHIP_TESTED, PRIOR_RECEIPT, EMULATED, HOST_TESTED, PROPOSED, BLOCKED_ACCESS, BLOCKED_NAME |
| `admit` | YES or NO |
| `evidence`, `blocker` | receipt path (or `NOT_RUN`) and the exact reason a row is not admitted |

`hwman check` refuses (exit 3) a row that admits without CHIP_TESTED evidence, a `hwman-*` runner and
operations; a BLOCKED row without a blocker or with `admit=YES`; UNRESOLVED routes that are not
BLOCKED_NAME (and the reverse); PROPOSED rows that admit; tested statuses whose evidence is `NOT_RUN`;
non-decimal timeouts; duplicate ids; unknown enums; and a register missing any family.

Every `hwman-` runner is bound by one closed table in `core.om` (`TRUN`), used by `check` and therefore
by `admit` and `run`, which load the register through `check`:

| Runner | Family | Route | Operations |
|---|---|---|---|
| `hwman-cpu` | CPU | NATIVE_ISA | `sumsq64`, `fnv1a64` |
| `hwman-tpm` | TPM | DIRECT_DEVICE | `props`, `getrandom` |

A `hwman-` runner outside the table is `manifest unknown_runner`, a runner on another family or route is
`manifest runner_family_route`, and an operation outside the runner's list is `manifest runner_op`; all
exit 3. `run` also refuses (exit 5) an operation its runner does not execute before discovery or claim.

## Lifecycle

`hwman run MAN TARGET OP ARG GEN STATE [EXPECT]` performs, and prints a `STEP` line for each of:

1. **discover**: the runner reads the device identity (CPU: the pinned processor's `midr_el1` from sysfs;
   TPM: `TPM2_GetCapability(TPM_PROPERTIES, PT_FIXED)` on `/dev/tpmrm0`) and compares it with `identity`;
2. **claim**: `STATE` is opened and locked with `flock(LOCK_EX|LOCK_NB)`; it holds `"<generation> <target>\n"`
   (an empty new file is generation 0). The caller's `GEN` must equal the stored generation;
3. **prepare**: `ARG` is validated for the operation;
4. **submit** and **complete** (both CPU and TPM print `STEP complete`): before submission byte 0 of
   `STATE` is set to `x` and synced, so a crash or failed release from here on leaves the handle
   unreadable. `timeout_ms` is overrun detection, not cancellation: the CPU compares elapsed time after the
   operation returns; the TPM bounds its wait for the response and refuses, but a sent command is not
   cancelled in the chip;
5. **quiesce**: CPU re-reads the processor it ran on; TPM closes the descriptor;
6. **release**: after any submission the generation advances by one: bytes 1.. of the new record are
   written and synced, then byte 0, then synced again. The record never shrinks (the generation only
   grows), and nothing is truncated. A failed or short write or a failed sync refuses with 74
   (`state_write_failed`) and never prints PASS; the handle stays unreadable (exit 7 for every
   generation), or after only the final sync failed holds the advanced record. It never returns to an
   earlier generation. Recovery of an unreadable handle is an operator decision: inspect the device, then
   create a new `STATE` file. A refusal before submission releases without advancing.

`hwman admit MAN OP [TARGET]` is the read-only consumer query (proposed to the Omega Node scheduler,
DRA-407): exit 0 and an `ADMIT` line naming the target, or a refusal with the classes below.
`hwman fault MAN TARGET KIND GEN STATE` sends a fixed malformed TPM frame and passes only on the one
exact expected result: `badsize` (commandSize 13, 12 bytes written) on kernel errno -22 (`layer=kernel
errno=-22`); `badcc` (commandCode 0x11e) on rc 0x000b0143 (`layer=tpmrm rc=0x000b0143`), which is
TPM2_RC_COMMAND_CODE 0x143 with TSS2_RESMGR_TPM_RC_LAYER (11 << 16): the Linux resource manager
(`drivers/char/tpm/tpm-dev-common.c`, `tpm2_prepare_space`) answers an unlisted command code itself, so
the frame never reaches the chip. Any other result is a device error (exit 13,
`unexpected_fault_result kind= result= want=`).

| Exit | Class |
| --- | --- |
| 0 | PASS |
| 2 | usage |
| 3 | manifest refused |
| 4 | unknown target |
| 5 | unsupported operation or fault kind |
| 6 | not admitted (`admit=NO`, outside lease) |
| 7 | stale generation, wrong or unreadable handle |
| 8 | wrong device (identity, unpinned or moved processor) |
| 9 | busy (handle locked) |
| 10 | offline (node absent or unreadable) |
| 11 | timeout |
| 12 | oracle mismatch (`EXPECT`) |
| 13 | device error |
| 14 | invalid argument or generation |
| 74 | I/O error, including `state_write_failed` (handle persistence failed after submission) |
