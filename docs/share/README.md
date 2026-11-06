<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# Zero-copy buffer sharing between processes

`native/share/share.om` lets one process give another process its GPU buffer
without copying it. Both processes then read and write the same physical pages
through the CPU, and the GPU sees the buffer at the same virtual address in the
importer as the CPU does. It is the CUDA++ form of ROADMAP section 3
("Zero-copy shared memory"). It is written in Omega and composes after
`native/nvrm/context.om`, reusing that file's descriptors, handles and mapping
slots. `context.om` is unchanged.

Status: IMPLEMENTED and EXERCISED against a fake RM on the host. It has not
run on a GB10. The chip tier is NOT_RUN; the runbook is below.

## Calls

| Function | Does | Returns |
|---|---|---|
| `nvexport m` | Grants `DUP_OBJECT` on the memory behind mapping `m` to any client of the same user (`RM_SHARE`, NVOS57) and fills the 24-byte record `nvshrec`. | `true`, or `false` with `nverr` set |
| `nvimport want ro` | Reads `nvshrec`, duplicates the owner's memory into this process's client (`RM_DUP_OBJECT`, NVOS55), proves the size, maps it for the CPU, reserves a UVM external range at the CPU address, maps it for the GPU there (`RM_MAP_MEMORY_DMA`, NVOS46) and adds the UVM external mapping (`UVM_MAP_EXTERNAL_ALLOCATION`). | CPU mapping handle; `mapaddr` of it is also the GPU VA. `-1` with `nverr` on refusal |

An import is released with `nvdrop` like any `nvalloc` mapping. The duplicate
holds its own reference to the pages, so an import keeps working after the
owner frees its buffer or exits.

With `ro` true every access flag is read-only: CPU map `NVOS33` access field
`READ_ONLY` (1), CPU mapping bound with access 1, DMA flags 33041 (`33040`
plus `NVOS46` access `READ_ONLY`), UVM mapping type 3 (`READ_ONLY`). A CPU
write through a read-only import traps with exit 103.

An imported buffer cannot be exported again (`EPERM`). Only the owner exports.

## Record

Little endian, 24 bytes, produced by `nvexport`, consumed by `nvimport`. The
transport between processes (pipe, socket, file) is the caller's.

| Offset | Width | Field | Accepted |
|---|---|---|---|
| 0 | u32 | magic `"OMZC"` = 1129991503 | exactly that value |
| 4 | u32 | version | 1 |
| 8 | u32 | owner RM client handle | nonzero |
| 12 | u32 | owner memory handle | nonzero |
| 16 | u64 | size in bytes | 1 .. 68719476736 (2^36), multiple of 4096, equal to `want` |

## Refusals

Before any RM call (no slot, no RM object, context not faulted):

| `nverr` | Cause |
|---|---|
| -5 | context faulted, closed, or UVM not registered |
| -22 | malformed record: magic, version, zero handle, size 0, over 2^36, not page aligned |
| -34 | record size differs from `want` |
| -24 | no free mapping slot |
| -75 | handle sequence exhausted |

From RM, after the duplicate exists (the duplicate is freed, no fault):

| `nverr` | Cause |
|---|---|
| RM status (positive) | `RM_DUP_OBJECT` refused: wrong handle or client, not shared, other user |
| -34 | the object is not `size` bytes (size proof below) |

GPU stage (UVM range, DMA map, UVM mapping): the slot stays quarantined and
the context is faulted, exactly as in `nvalloc`. `nvmclose` or `nvmforce`
releases it.

### Size proof

`RM_DUP_OBJECT` does not report the object's size. Before mapping, the
importer asks RM to map one page at offset `size`. RM must refuse it with
`INVALID_BASE` (0x20), which means the object is not larger than `size`. The
full map of `size` bytes must then succeed; `INVALID_LIMIT` (0x2e) means the
object is smaller. Either mismatch returns -34 and the duplicate is freed,
which also retires any RM mapping record the probe created. The RM behavior
is read from `src/nvidia/src/kernel/rmapi/mapping_cpu.c` lines 245-269 of
NVIDIA open-gpu-kernel-modules 580.173.02 (not vendored here; the vendored
tree holds headers only). It is not yet confirmed on the chip.

## Constants

Request numbers, parameter sizes, field offsets and statuses are not typed by
hand in the tests. `tests/share/abi-oracle.c` prints them from the pinned
headers in `third_party/nvidia-open-580.173.02/` as Omega functions, and the
fake RM checks every call against them. The literals in `share.om` were
computed with Wolfram:

```
WolframLanguageEvaluator: {3*2^30+28*2^16+70*2^8+52, 3*2^30+24*2^16+70*2^8+53,
  2^15+2^8+2^4, BitOr[2^15+2^8+2^4,1], FromDigits[Reverse[ToCharacterCode["OMZC"]],256],
  2^36, FromDigits["1b",16], FromDigits["20",16], FromDigits["2e",16], FromDigits["57",16],
  2^40, 2^49-2^26}
-> {3223078452, 3222816309, 33040, 33041, 1129991503, 68719476736, 27, 32, 46, 87,
    1099511627776, 562949886312448}
WolframLanguageEvaluator: {2^36 + 2^12, 2^37, 2^14, 2^13, FromDigits[{67,90,77,79},256]}
-> {68719480832, 137438953472, 16384, 8192, 1129991503}
WolframLanguageEvaluator: {2^16, 2^16/4096, 10*10^9} -> {65536, 16, 10000000000}
WolframLanguageEvaluator: {FactorInteger[2^49 - 562949869535232], 2^49 - 5*2^24 == 562949869535232}
-> {{{2, 24}, {5, 1}}, True}
```

The GPU VA bound `562949869535232` in `nvshgpu` is `2^49 - 5*2^24`, the
`vaSize` that `nvfull` in `context.om` gives the VA space
(`NV_VASPACE_ALLOCATION_PARAMETERS` byte 8). `nvshgpu` checks an import's CPU
address against it exactly as `nvalloc` does, with the same literal. (The
`2^49-2^26` in the first query above is not that bound.)

## Host test

```
OMEGA_OMC=<pinned omc> make test-zero-copy-share
```

`tests/share/run-share-host.sh` runs four tiers. None opens a device file.

1. Host reference: build and run `abi-oracle.c` against the pinned NVIDIA
   headers, producing `abi.om`.
2. Simulation: rewrite a private copy of `context.om` + `share.om` so every
   device word (`devcall`, `devctl`, `uvmctl`, device `fopen`, `fclose`,
   `mapbind`, `mapnew`) calls `tests/share/fake-rm.om`, and refuse the run if
   any raw device word survives. The fake keeps an RM object table per client
   and backs each memory object with a real memfd, so an import really aliases
   the owner's bytes. `tests/share/share-test.om` forges records byte by byte
   and checks:
   - owner export and record bytes; export refusals;
   - same-process and foreign-client imports, read-write and read-only, by
     reading and writing the shared bytes;
   - an import cannot be re-exported;
   - the owner frees first and both imports still read the bytes;
   - negative controls: wrong handle, wrong client, unshared object, size
     mismatch in both directions, `want` mismatch, seven malformed records,
     faulted context, and a UVM failure that must quarantine the slot;
   - a separate binary writes through a read-only import and must exit 103.
3. Mutants: `tests/share/mutants.sh` applies 20 one-site changes to
   `share.om` (each read-only flag, both bind accesses, the size proof, the
   share policy, the record checks, the release on refusal, the GPU stage).
   Each mutant must compile and the suite must fail on it.
4. Chip gate: `tests/share/chip-share.om` is compiled from the unrewritten
   sources and never executed.

## Hosted native library unit

`native/share/share_native.om` is the same export and import in OM-2
function mode, for the one combined hosted native library (omega-bridge ABI v2,
`tools/native --host`; Integration generates and links that library). Function
mode has no globals, so it does not compose with `context.om`. All state lives
in caller buffers, and the CPU mapping token lives in the explicit native
context the caller passes.

| Export (C, `omega_native.h`) | Arguments after `ctx, uint64_t *result` |
|---|---|
| `omega2_ctx_nvshare_export` | `uint8_t *s` (`zcshare`), `uint8_t *rec` (`zcrec`), `uint8_t *p, uint64_t p_len` |
| `omega2_ctx_nvshare_import` | `uint8_t *s`, `const uint8_t *rec`, `uint8_t *p, uint64_t p_len`, `const uint8_t *path, uint64_t path_len` |
| `omega2_ctx_nvshare_release` | `uint8_t *s`, `uint8_t *p, uint64_t p_len` |

`*result` is a signed 64-bit value, also stored in `zcshare.err`: 0, a negative
errno (the table above, plus -16 for a block that already holds an import and
-1 for re-exporting an import), or the positive RM status of a refused request.
`p` is a 9264-byte scratch (the size of `UVM_MAP_EXTERNAL_ALLOCATION_PARAMS`);
any other `p_len` is `OMEGA_BAD_ARG` before any request. `path` names the
control device (`/dev/nvidiactl` on the chip) without a trailing NUL; import
opens it for the descriptor `RM_MAP_MEMORY` binds and closes it afterwards.

`zcshare` (96 bytes): inputs `ctlfd`@0, `uvmfd`@4, `client`@8, `device`@12,
`vaspace`@16 and `uuid`@72 (16 bytes) from the NVRM client; export reads
`obj`@20 and `size`@32; import reads `newh`@24 (handle for the duplicate),
`mode`@28 (0 read/write, 1 read-only), `size` and `addr`@40 (CPU and GPU
address, 4 KiB aligned, inside [65536, 2^40 - size]). Outputs: `obj` (the live
duplicate), `map`@48 (CPU mapping token), `live`@56 (bits: 1 duplicate, 2 CPU
mapping, 4 UVM range, 8 DMA mapping, 16 completed import), `stage`@60
(51..57), `err`@64 and `fault`@88. `zcrec` is the 24-byte record above.

Rules the callers keep:

- Import and release use the same native context. A release through another
  context runs the UVM, DMA and RM frees, then traps in `mapdrop` (bridge
  status 3, result unwritten); the block still shows `live` 18, and the right
  context finishes the release.
- Each live import holds one slot of that context; a full context refuses the
  import with -24 before the duplicate survives (it is freed).
  `omega_native_context_destroy` returns `OMEGA_CONTEXT_BUSY` while an import
  is live.
- Release frees in order UVM range, DMA mapping, RM duplicate, CPU mapping.
  Each `live` bit clears only when its request succeeded, so a refused step is
  retried by calling release again.
- `fault` is sticky. A faulted block refuses export and import with -5; release
  it and discard it.

`tests/share/run-share-native.sh` (`make test-zero-copy-share-native`) builds
a single-unit test library from this file into the build directory, never
linked into the product, and runs `tests/share/native-share-test.c`. That test
traps `ioctl` on two unopened descriptors with seccomp and answers as RM and
UVM, decoding each request with the pinned NVIDIA headers. Memory objects are
memfds, so imports alias the owner's bytes in both directions. It checks
export, read/write and read-only imports (a child's write to a read-only import
gets `SIGSEGV`), every refusal, size lies in both directions, the faults of a
zero duplicate handle, a refused DMA or UVM mapping and a moved DMA address, a
refused RM free, context slots, and the wrong-context trap. Then it rebuilds
the library from each of the 20 mutants in `tests/share/native-mutants.txt`;
each must build and the test must fail. Pins (checked by sha256):
`OMEGA_NATIVE_OMC2` b8f8f137… (omega-mv dbedb2ca), `OMEGA_NATIVE_OMC`
734f0a39…, and `ZERO_COPY_NATIVE_BRIDGE` an omega-bridge checkout with the
explicit native context (omega-bridge#69 at 248a1984). Without them the target
reports NOT_RUN and exits 2.

The chip tier of this unit is NOT_RUN. It needs the combined library with an
NVRM client bring-up export (client, device, VA space, UVM registration, GPU
UUID) to fill `zcshare`; the program-mode chip runbook below sends the same
requests.

## Chip runbook (NOT_RUN)

Only in a Drake-authorized exclusive GPU window, following the current
window procedure. It opens `/dev/nvidiactl`, `/dev/nvidia0` and
`/dev/nvidia-uvm` from two processes.

```
cd <CUDApp checkout at the PR head>
OMEGA_OMC=<pinned omc> ZERO_COPY_OUT=build/zero-copy sh tests/share/run-share-host.sh --no-mutants
sha256sum build/zero-copy/chip-share       # record next to the receipt
chmod 0755 build/zero-copy/chip-share
taskset -c 10-14 build/zero-copy/chip-share; echo rc=$?
```

Pass is rc 0 and stdout `ZERO_COPY_SHARE_CHIP_PASS`. The program forks before
any device is opened. The parent allocates 65536 bytes, fills them, exports
and sends the record over a pipe. The child imports the buffer read-write and
read-only, checks the owner's bytes through both, writes a marker byte through
the read-write import, and checks that the read-only import sees it. The parent
checks that its own mapping sees the child's marker, frees its buffer, and
tells the child. The child checks both imports again, drops them, and stops its
context. A failure prints `ZERO_COPY_SHARE_CHIP_FAIL step N`; steps 20..32 are
the owner's, 40..53 the importer's, 60..64 pipe, fork and reaping (see the source).

Negative control for the window: run it once with the record's handle
corrupted (`nvshput 12 add nvshget 12 1` before the send, in a scratch copy),
which must fail at step 42 with an RM status in `nverr`.
