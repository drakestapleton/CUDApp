# Direct gfx942 KFD launcher

Intent: exercise our seeded gfx942 code objects through Omega on MI300X, with a CPU-checkable host path and honest hardware receipts. The executable uses Linux/KFD syscalls directly; neither ROCr nor HIP is in its production path. Python and C under `tests/amd-kfd-launch/` are external test instruments and notebook measurements only.

## Build and CPU acceptance

`COMPILER` records the local omega-mv source head and the seeded compiler digest. Concatenate `om2`, `common.om`, `elf.om`, `device.om`, `dispatch.om`, `plan.om`, `main.om`, then `../cli.om`, and pipe into that compiler. Prefix `target x86linux` before `om2` for x86. The gate performs this composition, validates the compiler digest before execution/output, and builds both targets:

```
python3 -I tests/amd-kfd-launch/check.py /absolute/path/to/omega-mv/seed/out/omc
```

On Spark run CPU jobs through the assigned pool lock, nice and CPU affinity. `AMD_KFD_LAUNCH_OUT` chooses a new evidence directory; do not overwrite an old receipt. The gate emits `results.json`, `summary.json`, binaries, malformed objects and calculation queries/results. It exercises native and qemu-x86_64 ELF parsing and absent-device errno; a native ptrace instrument replaces device opens with memfds and all KFD ioctls with checked replies. It checks GPU identity, UAPI layout, same-VA render mappings, separate KFD doorbell mapping, queue fields, generic CLI ring wrap, output bytes and cleanup. A second instrument checks each native instruction's packet publication order. Wrong GPU records, early valid headers and missing completion must fail. These instruments are not a GPU emulator, shader executor or hardware qualification.

## Commands

```
omega-launch plan PLAN.bin CODE_OBJECT... WEIGHT_FILE...
omega-launch inspect OBJECT.hsaco KERNEL
omega-launch vadd VADD.hsaco vadd
omega-launch run OBJECT.hsaco KERNEL GROUPS_X GROUPS_Y ARGS.bin INPUT... OUTPUT.bin
```

The generic grid is groups X/Y with workgroup dimensions 64/1/1 (wave64). The vadd smoke uses our vadd object and checks every output bit. `inspect` validates without opening a device. Objects are trusted first-party kernels, not an untrusted-code sandbox.

The loader accepts ELF64 little-endian AMDHSA code-object V5, gfx942, bounded nonoverlapping load segments and a defined `.kd` descriptor. It loads segments at their virtual offsets and zeroes BSS; no relocations, negative entry offsets, wave32, scratch/private-segment requirement, an ENABLE_SGPR_QUEUE_PTR requirement, or oversize LDS. No LLVM-generated object substitutes for our seeded writer. Malformed input is refused before GPU allocation.

### Argument manifest

All integers are little-endian. The header is `OMKFD001`, followed by five u64 fields: raw kernarg byte length, output byte length, repeat count, timeout nanoseconds and relocation count. Each relocation is two u64 values: raw-kernarg byte offset and input-buffer index. Raw kernarg bytes follow the relocation pairs. There must be exactly that many bytes and the raw size must equal the descriptor's kernarg size. Pointer slots must be aligned, in bounds, unique and initially zero. Indices refer to the positional input files; the index equal to their count means the output buffer. The launcher writes only these slots with addresses of its own mapped allocations. Other scalar fields remain as supplied.

Exit classes: 64 CLI/argument contract; 65 ELF/span contract; 66 topology; 67 insufficient KFD ABI; 74 syscall or I/O failure; 75 completion timeout; 77 `/dev/kfd` absent (prints `errno=2`). Compiler handle/span refusals remain documented traps. Normal completion destroys the queue, releases mappings/allocations and closes fds. On an error exit Linux releases the process's descriptors and KFD context; no persistent service or queue is created.

## Memory and dispatch

Select a gfx942 topology node and its DRM render minor; use that node's GPU ID, never an NVIDIA identifier. GET_VERSION requires KFD 1.14 or later, then ACQUIRE_VM binds the render fd. Each buffer has one 2 MiB-aligned reservation and 64-bit sizes. ALLOC receives its compiler-bound address, MAP_MEMORY its checked GPU-ID span, and the returned mmap offset is mapped through the render fd at that same reserved address. Public VRAM holds code, inputs and output; In the generic command, GTT holds kernargs and queue state. The persistent plan places static kernargs in VRAM; its token cell and queue state stay in coherent GTT. The doorbell is instead mapped through `/dev/kfd`. No raw caller-provided ioctl address is trusted.

Discovery reads `gpu_id` before properties, skips CPU nodes (`gpu_id=0`), and
skips nodes whose files cannot be opened or read. Each skipped failure records
its errno and node number. It accepts `gfx_target_version 90402`, logs the chosen
node/GPU ID/render minor, and opens that exact render device. If the container
does not expose it, `render_open` fails explicitly; another GPU's render device
is never silently substituted. The CPU gate injects EPERM at the syscall boundary
(including under root) for both `gpu_id` and `properties`, with CPU nodes and a
node-3-only GPU fixture. Mutants that abort on either skipped node must fail.

Each device step emits `OMEGA step NAME ok|fail errno=N phase=before|after detail=N`.
A `before` row means the step is about to be attempted, not that it succeeded.
Detail identifies the node, selected GPU, render minor, buffer index, doorbell
offset, byte count or packet count as applicable. Kernel errno is preserved;
contract failures use EINVAL, I/O/readback failures EIO, missing gfx942 ENODEV,
and polling timeout ETIMEDOUT. Raw `OMEGA error=N` and existing refusal exits
remain for syscall failures. All allocation/map buffers have separate rows.

Queue state includes ring, write/read pointers, context save and a coherent USER signal. The signal starts at the repeat count; each dispatch decrements it. Context sizes follow the cited ROCT gfx942 formula using actual node SIMD, shader-array and XCC properties, rather than a card-name constant. gfx942 AQL needs no EOP allocation. Context debug headers follow the ROCT ABI. The ring's configured size is not a limit on repeats: read-pointer backpressure permits wraparound.

The launcher owns a zero-initialized 4096-byte AMD queue object. Its write and
read dispatch IDs are at bytes 56 and 128 of the same live mapping. CREATE_QUEUE
receives the same opaque queue handle for both pointers, with zero address
bytes in the caller header. The compiler selects the fixed AMD AQL layout,
binds base+56 and base+128, and validates the complete 256-byte ABI object,
handle lifetime and writable kind. The launcher owns and clears a 4096-byte
allocation containing that object.

The HSA prefix names the ring, queue size and ID. The queue's read-pointer base
offset is 128, pointer mode at byte 180 is 2, and the inactive USER signal is a
separate live allocation. GET_PROCESS_APERTURES_NEW supplies LDS and scratch
aperture high words for the selected GPU, with exactly one matching record
required. Empty, truncated, duplicate, invalid or unmatched results are refused.
CU and wave limits come from the selected node's topology properties. The trusted
kernel subset still excludes private scratch and queue-pointer arguments.

The AQL ring is executable, readable, writable, coherent GTT memory (allocation
flags 0xe6000002). Both the full queue layout and executable ring were required
in the MI300X tests. A non-executable ring generated a GPU memory fault at the
ring address; setting executable alone with the old minimal queue still timed
out. No diagnostic observer, event injection, HIP or ROCr participates in the
production path. The CPU gate checks the entire queue image and zeroed tail,
ring permission, aperture selection and inactive signal, and rejects deliberately
broken versions.

The packet body is written while its header is INVALID. A CPU fence precedes the final aligned header write; further fences precede write-pointer and 8-byte doorbell publication. The dispatch has system acquire/release fence scopes and barrier semantics. The launcher bounds host polling by the requested monotonic timeout, waits for signal zero, fences, and copies output. It reports `OMEGA dispatch_ns=... packets=...`; this is host-monotonic dispatch-and-wait time, not GPU event time and not model tokens/s.

Success and timeout also emit `OMEGA diag NAME VALUE` decimal rows, read only
from existing owned mappings: queue read/write pointers, queue properties and read-pointer base offset, signal kind/value,
last published packet slot/header and actual packet kernel/kernarg addresses,
descriptor fields, and the first eight available output words. Vadd reports
counts matching its expected sum and NaN fill. `create_queue_input_qword_N`
reconstructs the input u64 at byte offset `8*N`, including addresses that the
pinned compiler binds privately; queue ID and full doorbell offset are outputs.
The CPU instrument compares all input qwords with the actual intercepted ioctl
block and tests untouched output, output without signal completion, and normal
completion. These rows are observations, not a timeout diagnosis.

No SET_MEMORY_POLICY or debugger RUNTIME_ENABLE is needed by this path. Kernel registration configures SH_MEM bases/config; the gfx9.4.3/9.4.4 path sets F8_MODE. The default/alternate cache-policy ioctl is not the initializer. This is a source-based prerequisite, not a measurement of an installed remote driver's register state. See root `PROVENANCE.md` for the named kernel tag and manual requirement.

## One-cell notebook bundle

`tests/amd-kfd-launch/bundle.py --gate GATE_DIR --out NEW_DIR` packages the verified
x86 executable and vadd object into `cell.py`, using xz then base85. Source commits
and hashes are generated into `bundle.json`; README and SHA256SUMS are generated.
The manifest covers every delivered file except itself.

The targeted diagnostic bundle is `notebook-probe/launch4/`. It runs the corrected
Omega vadd once, while the Python parent samples the child's KFD proc queue,
occupancy and VRAM files read-only. Missing files are explicit. After Omega exits,
even on timeout, it runs the identical object and inputs once through the installed
torch-loaded HIP runtime, labelled `REFERENCE (AMD runtime loading our object, not
the Omega launcher)`. Torch supplies the runtime, not kernels or a correctness
oracle. No other kernels, doorbell variants or added Omega packets are run.

The cell persists `amdserve-launch4.txt` after every line and retains raw child
stdout/stderr. CONTROLS is first; topology and every launcher stdout line are
included; RESULT is last even on exceptions. `notebook-check.py NEW_DIR GATE_DIR`
checks both parsers, real x86/qemu refusals and absent-device behavior, receipt
streaming, read-only proc sampling while a real child waits, and digest refusal.

The earlier launch4 bundle remains a vadd diagnostic harness. Hardware evidence
for the full queue fix is recorded separately: the uninstrumented x86 executable
`fd85139a4ea58a892a74b910c6432038feb5edbe41c6e4f118357cc57f30aa60`
passed vadd, 200 repeated vadd packets across ring wrap, fsum16, SGEMM and FP8 GEMV
with exact fixture outputs on MI300X. These kernel checks do not establish full
27B serving or a like-for-like Spark comparison.

## Persistent model plans

`plan` validates an OMPLAN01 file, all code descriptors, weight-file extents and
relocations before opening KFD. The format is specified in [PLAN.md](PLAN.md).
Plans and kernels are trusted first-party executable input. This is a command-line
inference executor; it does not expose an HTTP service or accept untrusted plans.

Code objects load once. Model buffers occupy checked, page-aligned slices of one
VRAM allocation and one coherent GTT allocation. A buffer's relocations may address
only its declared byte range, excluding alignment padding. This avoids consuming a
runtime map handle for each model tensor. Normal weight ranges copy directly from
files; BF16 vectors widen exactly into F32. Initialization and zero fill precede
timing. Kernargs and packet templates are built once. Each step replays the list,
with ring backpressure and one completion signal on the last dispatch. The plan
ring grows from the established 1024-byte minimum to a power of two large enough
for the complete step. Its size must fit the KFD UAPI byte-count field. The host
invalidates owned slots, copies packet bodies in contiguous chunks, fences,
publishes aligned headers, then fences before publishing the write pointer and
one doorbell per step. Static kernargs reside in VRAM. No performance claim
follows from those implementation choices without a matching card measurement. The host
writes prompt tokens, then reads generated IDs from the coherent token buffer.

`OMEGA_STEP i tok=T us=U` measures monotonic wall time from the first packet write
through the step completion signal. `OMEGA_PLAN ids=[...]` contains exactly the
requested generated IDs after successful cleanup. A timeout exits 75 and never
prints the completion marker. Prefill is token-by-token; decode statistics exclude
prompt steps. Allocation, weight loading, template construction and text decoding
are outside these per-step timings. Compare these boundaries explicitly with any
other runtime's numbers.

The CPU gate also runs `plan-check.py`: malformed plans on both target parsers,
independent code objects, raw file ranges, BF16 widening, data-pool initialization,
shared allocation offsets, ring wrap, output IDs, missing completion and cleanup.
`publication.c` observes a nonzero group-segment size and rejects a deliberate
swap of the packet's group/private fields. CPU simulation is not card proof.
