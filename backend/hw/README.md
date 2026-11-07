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

## Lifecycle

`hwman run MAN TARGET OP ARG GEN STATE [EXPECT]` performs, and prints a `STEP` line for each of:

1. **discover**: the runner reads the device identity (CPU: the pinned processor's `midr_el1` from sysfs;
   TPM: `TPM2_GetCapability(TPM_PROPERTIES, PT_FIXED)` on `/dev/tpmrm0`) and compares it with `identity`;
2. **claim**: `STATE` is opened and locked with `flock(LOCK_EX|LOCK_NB)`; it holds `"<generation> <target>"`.
   The caller's `GEN` must equal the stored generation;
3. **prepare**: `ARG` is validated for the operation;
4. **submit** and **complete**: the operation runs under `timeout_ms`;
5. **quiesce**: CPU re-reads the processor it ran on; TPM closes the descriptor;
6. **release**: after any submission the generation advances by one, then the lock is dropped. A refusal
   before submission releases without advancing.

`hwman admit MAN OP [TARGET]` is the read-only consumer query (proposed to the Omega Node scheduler,
DRA-407): exit 0 and an `ADMIT` line naming the target, or a refusal with the classes below.
`hwman fault MAN TARGET KIND GEN STATE` sends a fixed malformed TPM frame (`badsize`, `badcc`) and passes
only when the kernel or the TPM refuses it.

| Exit | Class |
| --- | --- |
| 0 | PASS |
| 2 | usage |
| 3 | manifest refused |
| 4 | unknown target |
| 5 | unsupported operation or fault kind |
| 6 | not admitted or wrong authority (runner does not own the route) |
| 7 | stale generation, wrong or unreadable handle |
| 8 | wrong device (identity, unpinned or moved processor) |
| 9 | busy (handle locked) |
| 10 | offline (node absent or unreadable) |
| 11 | timeout |
| 12 | oracle mismatch (`EXPECT`) |
| 13 | device error |
| 14 | invalid argument or generation |
| 74 | I/O error |
