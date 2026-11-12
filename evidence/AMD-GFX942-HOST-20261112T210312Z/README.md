# gfx942 HSA code object: host-only receipt

This receipt exercises the seeded-Omega FP32 `vadd` writer, not AMD hardware or Qwen serving.
Source base: CUDApp Radicle `rad:z4RxD8vXigmWg61nM8miQ8DVPAXEP`, main
`a6dee65e00c3d4bbda32a7d63c9e5a0126a92a9a`. Branch: `amdserve/gfx942-hsaco`.
The commit containing this receipt is local only. Nothing was pushed, published, installed or landed.

## Inputs and replay

`inputs.sha256` is generated from the producer, table, oracle, gate, unchanged loader and
unchanged multitarget gate. The compiler is the repository's existing pin:

- omega-mv `e5b32567a7aff3ce594e3254a622a846a6cd190b`, `seed/out/omc`;
- SHA256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`;
- recovered from that commit in the local Radicle storage, not rebuilt or replaced with a newer pin.

Commands below ran from the lane's CUDApp root on Linux AArch64. `$HOME` is the operator's home;
`E=evidence/AMD-GFX942-HOST-20261112T210312Z` and
`OMC=$HOME/work/hive/amdserve/AmdGfx942/pin/omc` expand to absolute paths. The CPU pool lock was
held only for each command, with a 600-second deadline and `nice` on cores 10,11.

Before changing source (exit **0**):

```sh
timeout 600 flock -o "$HOME/handoffs/hive/queen/pool-10-11.lock" taskset -c 10,11 nice -n 10 \
  env MT_OUT="$HOME/work/hive/amdserve/AmdGfx942/runs/mt-before" \
  sh tests/multitarget/run.sh "$OMC"
```

Final AMD gate (exit **0**; complete module, not selected assertions):

```sh
timeout 600 flock -o "$HOME/handoffs/hive/queen/pool-10-11.lock" taskset -c 10,11 nice -n 10 \
  env AMD_OUT="$PWD/$E/gate" sh tests/amd/run.sh "$OMC" > "$E/gate.log" 2>&1
```

After changing source (exit **0**, same compiler and arguments as before):

```sh
timeout 600 flock -o "$HOME/handoffs/hive/queen/pool-10-11.lock" taskset -c 10,11 nice -n 10 \
  env MT_OUT="$HOME/work/hive/amdserve/AmdGfx942/runs/mt-after" \
  sh tests/multitarget/run.sh "$OMC"
```

The gate itself records compiler/source/output hashes, tool versions, both producer builds, both
emitted objects, oracle assembly output, extracted sections, disassembly, ELF/metadata inspection
and every negative control under `gate/`. Its final line is:

```text
AMD-GFX942-HOST PASS pass=14 fail=0 hardware=NOT_RUN
```

Both before and after multitarget final lines are:

```text
MULTITARGET PASS pass=22 fail=0 not_run=4
```

The two multitarget logs here are portable derivatives: only the exact home prefix was replaced
with literal `$HOME`. Unchanged original logs/outputs remain in the lane's sibling `runs/` directory;
`multitarget-original.sha256` records the original log hashes. No historical receipt was edited.

Missing-tool preflight was exercised separately with a PATH directory containing only symlinks to
`/usr/bin/uname` and `/usr/bin/sha256sum` (the temporary directory was then removed):

```sh
PATH="$PWD/../runs/missing-tools" /bin/sh tests/amd/run.sh "$OMC"
```

Exit **3**, `NOT_RUN missing tool llvm-mc-18`, saved as `missing-tool.log`. This preflight did not
compile or run a producer. Missing mandatory tools do not yield a successful gate.

## What the host gate established

- The producing path is seeded Omega only: `hsaco.om` reads `vadd.isa`, packs instruction fields,
  resolves its forward branch, and serializes ELF64/EM_AMDGPU/HSA v5, gfx942 flags, descriptor,
  symbols/dynamic table/SysV hash and an AMDGPU MessagePack metadata note. LLVM is never invoked
  by, or fed back into, the producer.
- `targets load AMDGPU_HSACO FILE gfx942` accepted the emitted object with exit 0 and `LOAD ACCEPT`.
  This is the existing offline loader validation, **not** a runtime or hardware loader.
- `.text` equals the independently assembled `llvm-mc-18 -triple=amdgcn-amd-amdhsa -mcpu=gfx942`
  oracle. Full `llvm-objdump-18 -d` listings also agree after rebasing only the ET_REL oracle to
  the emitted `.text` address and dropping the input filename heading. Raw words and operands stay.
- `llvm-readelf-18` parses the kernel `vadd`, descriptor symbol `vadd.kd`, gfx942 target and metadata.
  The note bytes match the oracle. Descriptor bytes match except the ET_REL entry relocation;
  the gate independently checks the resolved displacement to `vadd` in the emitted object.
- Both compiler invocations produce identical producer binaries and both object builds are identical.

Control lines in `gate.log`:

```text
PASS CONTROL instruction-table flipped bit CAUGHT code mismatch
PASS control-wrong-mach exit=33 LOAD REFUSE AMDGPU_HSACO code=33 reason=e_flags mach does not match the requested gfx target
PASS control-truncated exit=34 LOAD REFUSE AMDGPU_HSACO code=34 reason=no well-formed AMDGPU NT_AMDGPU_METADATA note in a PT_NOTE segment
PASS CONTROL wrong compiler digest CAUGHT exit=69 before compiler/producer/output setup
```

The flipped-table control changes the low bit of the `v_add_f32` source encoding (260 -> 261),
then runs the real producer. Wrong mach and truncation refuse with the loader's documented codes.
The wrong compiler is an executable control, not merely a changed expected checksum: it is refused
before execution, compilation or output-directory creation.

## Limits and surprises

- No `ld.lld`/`ld.lld-18` was installed. No package was installed. The oracle stays **ET_REL** and
  the Omega object is **ET_DYN**. Whole linked-oracle object identity is **NOT_RUN**; code, metadata
  and descriptor section parity are exercised. The relocation is visible in `oracle.readelf.txt`.
- `llvm-objcopy --dump-section` without an explicit output unexpectedly rewrote the scratch input,
  including ELF section-header/string-table layout. Final gate calls always supply a disposable
  output (`*.objcopy-rewrite`). Those files are oracle-tool byproducts, NOT producer artifacts and
  never loader input. The original `.hsaco` outputs remain untouched.
- The copied leftover oracle used `gid << 2` in a single VGPR, losing high address bits. The new
  oracle and table use full-width VGPR addresses with carry propagation. FP32 denormals are also
  explicitly preserved instead of accepting the assembler's flush default. These mechanisms are
  source/oracle checked only; numerical/boundary hardware behavior is **NOT_RUN**.
- Early scratch compiles exposed reserved-name `le` (exit 16), a function/local `sh` namespace
  collision (exit 16), register pressure (exit 12), and an untyped literal subexpression (exit 11).
  The final source uses `wrle`/`shdr`, splits operand parsing from packing, and types the expression.
  No missing compiler facility or host-language workaround was needed.
- The required old compiler has its existing loop-bound semantics (the streaming scans use bound
  999999); this is not an arbitrary table-buffer capacity. The parser has no line/file/code-size
  buffer, and the ELF code/note offsets are derived from emitted size. Format IDs identify exactly
  the eight used ISA layouts; fields are bounded by an actual 64-bit instruction. Flushing I/O
  buffers do not limit total input/output length. This remains a fixed-kernel writer, not an ISA
  compiler, general linker or hardened untrusted-input service.

All AMD hardware rows remain **NOT_RUN**, including MI300X dispatch, runtime/ROCr interoperability,
FP32 outputs, n=0/tail/boundary vectors, timing/throughput and Qwen execution. No GPU was opened,
no service changed, no sudo used and port 18091 was untouched. The existing target-status table was
not changed and AMD is not promoted to SUPPORTED.

## Independent constant checks (MODELED, not hardware measurements)

Mounted Wolfram setup checks: `Integrate[x^2,{x,0,1}] -> 1/3`, `sqrt(27)/2 -> (3 sqrt(3))/2`.

Wolfram Language query:

```wolfram
{IntegerString[BitOr[1,BitShiftLeft[2,6],BitShiftLeft[3,16],BitShiftLeft[3,18],BitShiftLeft[1,21],BitShiftLeft[1,23]],16],
 IntegerString[BitOr[BitShiftLeft[2,1],BitShiftLeft[1,7]],16],12/4-1,BitXor[260,261],
 Table[{g,Mod[4 g,2^32],Quotient[g,2^30],Mod[4 g,2^32]+2^32 Quotient[g,2^30],4 g},
       {g,{0,2^30-1,2^30,2^32-1}}]}
```

Result (resource bit fields dimensionless; address offsets in bytes):

```text
{"af0081", "84", 2, 1,
 {{0,0,0,0,0},
  {1073741823,4294967292,0,4294967292,4294967292},
  {1073741824,0,1,4294967296,4294967296},
  {4294967295,4294967292,3,17179869180,17179869180}}}
```

## Scope, provenance and guides

`PROVENANCE.md` records each public LLVM source and the copied leftover oracle. The unmerged gfx1100
lab at CUDApp `268a2e9d` was read-only prior art; none of its code/words/constants was reused.
`llvm-sources.sha256` records downloaded source-document identities (verification/navigation only,
not build inputs). `FILES.tsv` is the generated file-size inventory; `SHA256SUMS` covers the receipt
and every artifact except itself.

No omega-harness guide statement was made untrue: no language/pin/host-word/bridge change occurred.
The “CUDApp portable backend” section's hardware NOT_RUN/PLANNED target status and the gfx1151
LLVM-oracle statement about main remain true. This new gfx942 writer is a local, unmerged host slice.
The existing guide says not to call a vendor supported from host evidence; this receipt does not.
