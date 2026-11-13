# Direct gfx942 KFD launcher

Intent: exercise our seeded gfx942 code objects through Omega on MI300X, with a CPU-checkable host path and honest hardware receipts. The executable uses Linux/KFD syscalls directly; neither ROCr nor HIP is in its production path. Python and C under `tests/amd-kfd-launch/` are external test instruments and notebook measurements only.

## Build and CPU acceptance

`COMPILER` records the local omega-mv source head and the seeded compiler digest. Concatenate `om2`, `common.om`, `elf.om`, `device.om`, `dispatch.om`, `main.om`, then `../cli.om`, and pipe into that compiler. Prefix `target x86linux` before `om2` for x86. The gate performs this composition, validates the compiler digest before execution/output, and builds both targets:

```
python3 -I tests/amd-kfd-launch/check.py /absolute/path/to/omega-mv/seed/out/omc
```

On Spark run CPU jobs through the assigned pool lock, nice and CPU affinity. `AMD_KFD_LAUNCH_OUT` chooses a new evidence directory; do not overwrite an old receipt. The gate emits `results.json`, `summary.json`, binaries, malformed objects and calculation queries/results. It exercises native and qemu-x86_64 ELF parsing and absent-device errno; a native ptrace instrument replaces device opens with memfds and all KFD ioctls with checked replies. It checks GPU identity, UAPI layout, same-VA render mappings, separate KFD doorbell mapping, queue fields, generic CLI ring wrap, output bytes and cleanup. A second instrument checks each native instruction's packet publication order. Wrong GPU records, early valid headers and missing completion must fail. These instruments are not a GPU emulator, shader executor or hardware qualification.

## Commands

```
omega-launch inspect OBJECT.hsaco KERNEL
omega-launch vadd VADD.hsaco vadd
omega-launch run OBJECT.hsaco KERNEL GROUPS_X GROUPS_Y ARGS.bin INPUT... OUTPUT.bin
```

The generic grid is groups X/Y with workgroup dimensions 64/1/1 (wave64). The vadd smoke uses our vadd object and checks every output bit. `inspect` validates without opening a device. Objects are trusted first-party kernels, not an untrusted-code sandbox.

The loader accepts ELF64 little-endian AMDHSA code-object V5, gfx942, bounded nonoverlapping load segments and a defined `.kd` descriptor. It loads segments at their virtual offsets and zeroes BSS; no relocations, negative entry offsets, wave32, scratch/private-segment requirement, or oversize LDS. No LLVM-generated object substitutes for our seeded writer. Malformed input is refused before GPU allocation.

### Argument manifest

All integers are little-endian. The header is `OMKFD001`, followed by five u64 fields: raw kernarg byte length, output byte length, repeat count, timeout nanoseconds and relocation count. Each relocation is two u64 values: raw-kernarg byte offset and input-buffer index. Raw kernarg bytes follow the relocation pairs. There must be exactly that many bytes and the raw size must equal the descriptor's kernarg size. Pointer slots must be aligned, in bounds, unique and initially zero. Indices refer to the positional input files; the index equal to their count means the output buffer. The launcher writes only these slots with addresses of its own mapped allocations. Other scalar fields remain as supplied.

Exit classes: 64 CLI/argument contract; 65 ELF/span contract; 66 topology; 67 insufficient KFD ABI; 74 syscall or I/O failure; 75 completion timeout; 77 `/dev/kfd` absent (prints `errno=2`). Compiler handle/span refusals remain documented traps. Normal completion destroys the queue, releases mappings/allocations and closes fds. On an error exit Linux releases the process's descriptors and KFD context; no persistent service or queue is created.

## Memory and dispatch

Select a gfx942 topology node and its DRM render minor; use that node's GPU ID, never an NVIDIA identifier. GET_VERSION requires KFD 1.14 or later, then ACQUIRE_VM binds the render fd. Each buffer has one 2 MiB-aligned reservation and 64-bit sizes. ALLOC receives its compiler-bound address, MAP_MEMORY its checked GPU-ID span, and the returned mmap offset is mapped through the render fd at that same reserved address. Public VRAM holds code, inputs and output; GTT holds kernargs and queue state. The doorbell is instead mapped through `/dev/kfd`. No raw caller-provided ioctl address is trusted.

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

The packet body is written while its header is INVALID. A CPU fence precedes the final aligned header write; further fences precede write-pointer and 8-byte doorbell publication. The dispatch has system acquire/release fence scopes and barrier semantics. The launcher bounds host polling by the requested monotonic timeout, waits for signal zero, fences, and copies output. It reports `OMEGA dispatch_ns=... packets=...`; this is host-monotonic dispatch-and-wait time, not GPU event time and not model tokens/s.

No SET_MEMORY_POLICY or debugger RUNTIME_ENABLE is needed by this path. Kernel registration configures SH_MEM bases/config; the gfx9.4.3/9.4.4 path sets F8_MODE. The default/alternate cache-policy ioctl is not the initializer. This is a source-based prerequisite, not a measurement of an installed remote driver's register state. See root `PROVENANCE.md` for the named kernel tag and manual requirement.

## One-cell notebook bundle

`tests/amd-kfd-launch/bundle.py --gate GATE_DIR --gemv-tree GEMV_TREE --attention-tree ATTN_TREE --out NEW_DIR` packages the verified x86 executable and exact committed seeded objects; `cell.py` is the one cell. `notebook-check.py NEW_DIR GATE_DIR` checks every object's parser path on both targets, executes the real cell's absent-device path through qemu, compares its reference calculations to rational scalar sums, and kills a changed-asset-digest control.

The integration bundle is named `notebook-probe/launch2/`. It includes the correct attention merge object selected by digest, never the earlier silicon-wrong merge. Object source commits and full hashes are generated into `bundle.json`; no peer branch is merged into this launcher. The builder generates README.md and hashes all delivered files except the manifest itself.

The cell persists `amdserve-launch2.txt` after every line, streams each launcher stdout line while the process is running, and retains inputs, outputs and raw stdout/stderr. CONTROLS is first, the observed topology table is included, and RESULT is always last, including asset-unpack or execution failure. It runs vadd, then the FP8/BF16 GEMV2 layer shapes and a tail shape with an independent fmaf/reduction reference. FP8 uses E4M3FNUZ row scales after reduction, with a discriminating OCP-bias control. Optional HIP measurements use the same object bytes and are labelled `OMEGA objects via AMD HIP loader`; no torch or NVIDIA rows are emitted. Host dispatch time includes diagnostic writes between dispatch and wait boundaries. Process wall time and HIP GPU-event time remain different quantities. There is no receipt truncation or line cap.

**NOT_RUN until the authorized MI300X run:** actual KFD mappings/queue creation, device completion, GPU output parity, FP8 mode, timings, attention dispatch, full 27B serving and any Spark comparison. Merely packing an object or passing the CPU instruments does not qualify it on silicon.
