# Provenance: where CUDA++'s hardware knowledge comes from

This file is the rule for adding hardware facts (instruction encodings, container layouts, launch
descriptors, latencies) to CUDA++, and the record of how the existing ones were obtained. It is an
engineering record, not legal advice. Questions that need a lawyer are listed at the end and are open.

`THIRD_PARTY.md` lists third-party material and its licences. `docs/HOW_I_DID_IT.md` describes the method.
This file says which inputs are allowed, which are not, and what every new fact must record.

## 1. The licence terms this follows

Checked 2026-10-27 against the primary texts (copies and SHA-256 values are kept with the review that
produced this file):

- **CUDA Toolkit EULA** (https://docs.nvidia.com/cuda/eula/index.html; online copy "Last updated: January
  7, 2025", `/usr/local/cuda/EULA.txt` of CUDA 13.0 "Last updated: January 12, 2025"). It covers nvcc,
  ptxas, cuobjdump, nvdisasm and the CUDA libraries.
  - §1.2(1): "You may not reverse engineer, decompile or disassemble, or remove copyright or other
    proprietary notices from any portion of the SDK or copies of the SDK."
  - §1.2(8): "You may not reverse engineer, decompile or disassemble any portion of the output generated
    using SDK elements for the purpose of translating such output artifacts to target a non-NVIDIA
    platform."
  - §1.1.2(4): "Unless a developer tool is identified in this Agreement as distributable, it is delivered
    for your internal use only." nvcc, ptxas, cuobjdump and nvdisasm are not in the distributable list
    (Attachment A).
  - §2.1 (CUDA supplement): "The SDK is licensed for you to develop applications only for use in systems
    with NVIDIA GPUs."
- **NVIDIA Driver License Agreement** (driver 580, `/usr/share/doc/libnvidia-compute-580/copyright`):
  §2.1 firmware only on NVIDIA platforms and never translated; §2.2 "You may not reverse engineer,
  decompile, or disassemble the SOFTWARE provided in binary form".
- **NVIDIA open GPU kernel modules**: MIT per file (`third_party/nvidia-open-580.173.02/COPYING`).
- **Mesa NAK/NVK**: MIT. **LLVM**: Apache-2.0 WITH LLVM-exception. **CUTLASS**: BSD-3-Clause.

## 2. Allowed sources

| Class | Source | How it may be used |
|---|---|---|
| A. Public documentation | PTX ISA, CUDA Binary Utilities (including its SASS instruction lists), CUDA Driver API reference, NVIDIA `open-gpu-doc` | Read and cite. |
| A. Open source under a permissive licence | NVIDIA open kernel module headers (MIT), Mesa NAK/NVK (MIT), LLVM `BinaryFormat/ELF.h` (Apache-2.0 WITH LLVM-exception), CUTLASS (BSD-3-Clause) | Cite the file and commit for each fact. Code is copied only with its notice, and only into `third_party/`. |
| A. The chip | Running my own bytes on my own NVIDIA GPU | Any fact; the receipt is the evidence. |
| B. NVIDIA SDK tools, oracle only | `nvcc`, `ptxas`, `cuobjdump`, `nvdisasm` installed on my own machine under the CUDA EULA | Only on inputs I wrote: my own probe kernels (CUDA C or PTX I authored) and bytes my own encoder emitted. Only for NVIDIA-GPU targets. Record the tool version. The tools are never shipped, wrapped or run by CUDA++ itself. |

## 3. Not allowed

1. Running any tool on, extracting from, or reading the machine code or container bytes of an NVIDIA
   binary: `libcuda`, cuBLAS, cuDNN, cuFFT, TensorRT, NCCL, the compiler binaries, the firmware or any
   other driver or SDK file (CUDA EULA §1.2(1); driver licence §2.2). This includes the cubins embedded in
   NVIDIA libraries.
2. Using anything learned from NVIDIA tool output (class B) for a non-NVIDIA target (CUDA EULA §1.2(8)).
   Non-NVIDIA backends take their facts from class A sources for that vendor only.
3. Committing SDK files, NVIDIA headers other than the MIT open-module headers, NVIDIA binaries, or
   firmware.
4. Giving MAX (Modular) source or binaries to an AI system in order to produce software that reimplements
   or substitutes for MAX (MAX License §1.4, "Last Modified: October 21, 2026"). Reading MAX to
   interoperate with it is allowed by the same clause.
5. Describing the work as a reverse-engineered copy of NVIDIA's software. It is not: see section 5.

## 4. What every new hardware fact records

In the source comment next to the fact, or in the test that pins it:

- the class (A or B) and the exact source: document and section, file and commit, or tool and version;
- for class B, the probe input (path of the `.cu`/`.ptx` I wrote) and the command line;
- the chip receipt that confirmed it, or NOT_RUN;
- for golden words: whether they were written independently (from class A plus the chip) or copied from
  class B output. Copied words are a regression pin, not independent evidence
  (see the `independent-sass-opcode-goldens` practice in the omega-harness skills).

Stored NVIDIA tool listings are kept only when a test needs them, under `tests/` or `evidence/`, and each
is listed in section 6.

## 5. How the existing knowledge was obtained (inventory, 2026-10-27)

| Area | Files | Class | Notes |
|---|---|---|---|
| Kernel-module interface (ioctls, classes) | `nvrm/`, `m16/`, `third_party/nvidia-open-580.173.02/` | A | 67 MIT headers copied unchanged. |
| SASS encodings | `src/omega_blackwell_encoder.c`, `src/omega_blackwell_codegen*.c/.h` | A + B | Mesa NAK facts cited; words matched against `nvcc 13.0.88 -arch=sm_121 -cubin` + `cuobjdump -sass` of probe kernels I wrote (`docs/HOW_I_DID_IT.md` step 2). Chip receipts in `evidence/`. |
| Instruction latencies, QMD, dispatch order | `src/omega_gpu_elementwise_api.c`, `src/omega_gpu_cmdlist.c` | A | Mesa `sm120_instr_latencies.rs`, `qmd.rs`, `nvk_cmd_dispatch.c` (MIT). |
| Tensor-core arithmetic model | `tests/gpu_tc_probe_test.c`, `tests/bw_hmma_model.h` | A (chip) | Fitted on chip outputs. |
| cubin container writer | `src/omega_cubin.c`, `src/omega_cubin.h` | B | Layout read from a `ptxas 13.0.88 -arch=sm_121` reference output with `readelf`/`cuobjdump -elf` (the source does not name the probe input; the next change to the file records it). Header constants are also public in LLVM `ELF.h` (class A). No bytes copied. |
| Decode cross-checks | `tests/gpu_reconv_test.c`, `tests/device-trace/run-emitter-checks.sh`, `tools/artifact_smoke.sh` | B | `nvdisasm`/`cuobjdump` read my own bytes. Since 2026-11-05 `gpu_reconv_test.c` reads them with Omega's own decoder (`om/disasm/sm121_dis.om`), and NVIDIA's `nvdisasm` runs only in the named oracle targets (`make test-sm121dis`, `tools/sm121dis/README.md`). `artifact_smoke.sh` skips unavailable tools; `run-emitter-checks.sh` requires `nvdisasm` on `PATH`. |

No source comment or record in this repository says a fact was obtained by disassembling an NVIDIA
library, driver or firmware. A fact found to have such an origin is removed and re-derived from class A
or B, and the removal is recorded here.

## 6. NVIDIA tool output stored in this repository

#84 stored `tests/device-trace/expected.sass`, an `nvdisasm 13.0 --binary SM121` listing of the
trace emitter's own words, as a regression pin. #93 (2026-10-27) removed it:
`tests/device-trace/run-emitter-checks.sh` now keeps the listing in a temporary directory and checks
hand-written expectations (opcode counts and the address-path instructions).

Stored now: each is `nvdisasm` V13.0.85 `--binary SM121` reading words my own encoder emitted (class B on
my own bytes); the input bytes sit beside the listing; no NVIDIA-generated code is involved. They are
kept as review evidence that the independent decoder agrees with the encoder. No test reads them: the
gates run `nvdisasm` live and check fixed expectations (`make test-om-encoder-ldg`,
`make test-om-encoder-atomg`, `make test-om-encoder-guard`, `make test-sm121dis-opmove`).

| Listing | Input (my bytes) | Why kept |
|---|---|---|
| `evidence/ldg-offset-20261107/nvdisasm.txt` | `ldg_words.bin`: four LDG.E rows of #373's encoder fix | Shows the offsets 0, +0x40, -0x100000 and +0x7fffff decode as encoded. |
| `evidence/atomg-min-20261107/nvdisasm.txt` | `min_words.bin`: the three ATOMG.E.MIN.64.STRONG.SYS rows of `om/encoder/atomg_min_test.om` | Shows op 64's words decode as the intended instruction and registers. |
| `evidence/atomg-min-20261107/atomg_field_probe_nvdisasm.txt` | `atomg_field_probe.bin`: ATOMG_ADD_STRONG_SYS's words with the operation field (w2 bits 23..26) set to 0..15 and the type field (bits 9..11) to 0..7 | Records how operation 1 = MIN and type 2 = .64 were identified. |
| `evidence/guard-pred-20261107/nvdisasm-after.txt` | `guard_words.bin`: the 135 instructions of `om/encoder/guard_pred_test.om bin` at the DRA-483 fix (@P0 / @!P0 forms of 63 ops, 4 LDCU guard probes, 5 omega-mv #1044 bool/trap rows) | Shows every guard decodes as encoded, and that LDCU's guard bits name UP0. |
| `evidence/guard-pred-20261107/nvdisasm-before.txt` | `guard_words_before.bin`: the same rows from the encoder before the fix | Shows the @P0 / @!P0 forms of 58 ops (MOV and STG.E among them) decoded with no guard (always), and @!P0 EXIT as @P0. |
| `evidence/INTEG-280-20261110/opmove-nvdisasm.txt` | `opmove_words.bin`: the sixteen rows of `om/disasm/opmove_test.om bin`, one per BlackwellIR op 64..79 after the #280 numbering move | Shows each moved op's words decode as the intended instruction, and gave the exact text of the decoder rows added for them. |

No listing of nvcc-generated code and no NVIDIA binary is stored.
The C1b guard gate (`make test-om-encoder-guard`) uses nvdisasm 13.0.85
`-b SM121 -hex` only on bytes emitted by our seeded encoder and its own
uniform-predicate probe words. Listings stay in ignored `build/om-encoder/`.
No new instruction form is introduced by this guard correction.
`make test-om-encoder-ldg` uses the same pinned oracle only on our emitted
LDG.E words. It checks the existing C #373 signed byte-offset field, restored
in the seeded encoder and its spec table after operand parity exposed the
omitted field. The listing remains ignored; no new instruction form is added.

### LDG.E signed byte offset, ham/encoder-ldg-offset

Class A decision source: Mesa NAK (MIT), commit
`8190c11866dfb23e4dfacaea2f7cb795596d1fb1`,
[sm70_encode.rs, OpLd::encode](https://gitlab.freedesktop.org/mesa/mesa/-/blob/8190c11866dfb23e4dfacaea2f7cb795596d1fb1/src/nouveau/compiler/nak/sm70_encode.rs#L3437-3512):
global load selects opcode `0x981` with UGPR addressing, then unconditionally
encodes `self.offset` in `40..64`. Its signed type is `i32` in
[ir.rs, OpLd](https://gitlab.freedesktop.org/mesa/mesa/-/blob/8190c11866dfb23e4dfacaea2f7cb795596d1fb1/src/nouveau/compiler/nak/ir.rs#L6501-6516).
Thus the existing C offset field is right; the seeded encoder and spec omitted it.
The nvdisasm 13.0.85 LDG gate is secondary verification on our bytes only, not
the decision source. Chip execution for this correction: NOT_RUN.

## 7. Open questions for a lawyer

1. Is compiling my own probe kernels with nvcc and reading the result with cuobjdump, to learn the
   encoding of NVIDIA instructions for an NVIDIA-GPU toolchain, within the CUDA EULA? §1.2(8) is limited to
   translation to non-NVIDIA platforms; does any other clause reach it?
2. Is a toolchain that replaces nvcc on NVIDIA GPUs an "application" developed "only for use in systems
   with NVIDIA GPUs" (§2.1)?
3. Can tool listings of my own code (section 6) be published, given §1.1.2(4)?

## 8. gfx942 host-only code-object writer (2026-11-12)

`backend/amd/gfx942/hsaco.om` is a first-party seeded-Omega writer for the fixed FP32 `vadd`
kernel, not a general AMD assembler or a qualified AMD runtime. It reads the instruction-table
data in `vadd.isa`, packs the words, resolves its forward branch, and serializes the ELF64 HSA v5
object, kernel descriptor, dynamic/static symbols, SysV hash and MessagePack metadata note.
The producing path uses no LLVM, linker, shell serializer, C, Rust or Python.
`tests/amd/run.sh` coordinates builds and compares against public LLVM tools; it opens no GPU.

All AMD facts are class A, independently sourced from **LLVM 18.1.3**, Apache-2.0 WITH
LLVM-exception. These are the exact public sources used (tag `llvmorg-18.1.3`):

| Fact | Source |
|---|---|
| HSA ELF64 header/flags, metadata keys, kernel descriptor/entry state and wave64 ABI | [AMDGPUUsage](https://releases.llvm.org/18.1.3/docs/AMDGPUUsage.html), sections “ELF Code Object”, “Kernel Descriptor”, “Initial Kernel Execution State”, “Code Object V5 Metadata”, and assembler `.amdhsa_kernel` directives |
| `EM_AMDGPU=224`, HSA OSABI=64, ABI v5=3, gfx942 mach=0x4c, XNACK/SRAMECC ANY flags, note type 32 | [llvm/include/llvm/BinaryFormat/ELF.h](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/include/llvm/BinaryFormat/ELF.h) |
| Descriptor offsets and RSRC1/2/3/kernel-property bit fields | [llvm/include/llvm/Support/AMDHSAKernelDescriptor.h](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/include/llvm/Support/AMDHSAKernelDescriptor.h) |
| SMEM field layout and scalar-load opcodes | [SMInstructions.td](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/SMInstructions.td), `SMEM_Real_vi`, `SM_Real_Loads_vi` |
| SOP1/SOP2/SOPP layouts and shifts, saveexec, branch, wait/end opcodes | [SOPInstructions.td](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/SOPInstructions.td), `SOP1_Real`, `SOP2_Real32`, `SOPP_Real_32`, `_vi` and gfx6–gfx10 definitions |
| VOP1 move layout/opcode | [VOP1Instructions.td](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOP1Instructions.td), `VOP1e`, `V_MOV_B32` `_vi` |
| VOP2 FP32 add, integer add/carry, shifts and mask layout/opcodes | [VOP2Instructions.td](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOP2Instructions.td), `VOP2e`, `_vi` and gfx9 definitions |
| VOPC unsigned bounds compare | [VOPCInstructions.td](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOPCInstructions.td), `VOPCe`, `V_CMP_GT_U32` `_vi` |
| GLOBAL load/store and 64-bit VGPR address form | [FLATInstructions.td](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/FLATInstructions.td), `FLAT_Real`, `FLAT_Real_gfx940`, `GLOBAL_LOAD_DWORD`, `GLOBAL_STORE_DWORD` |
| VGPR/SGPR resource granules and extra special SGPRs | [AMDGPUBaseInfo.cpp](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/Utils/AMDGPUBaseInfo.cpp), `getVGPREncodingGranule`, `getSGPREncodingGranule`, `getNumExtraSGPRs` |

The ORACLE assembly `vadd-gfx942.s` was copied from the ended `t_bd34e75b` lane's untracked
`backend/amd/gfx942/vadd-gfx942.s`; the original was not modified. Its kernel-argument layout
was checked against `backend/amd/vadd-gfx1151.s` (a/b/c pointers at 0/8/16, count at 24,
28 bytes). The new oracle fixes the copied single-VGPR `gid << 2` address path by preserving
the high address bits and propagating addition carries; it also explicitly preserves FP32
denormals (the assembler default flushes them). Resource declarations match the changed kernel.
Only public AMD LLVM sources/tools informed these changes; no NVIDIA-tool fact was used.

Read-only prior art inspected: CUDApp unmerged `feat/native-amd-agx-lab` at
`268a2e9d`, `native/gpulab/{amd.om,test-amd.om,amd-oracle-input.om,README.md}`.
That is a different gfx1100 integer lab/ABI; no code, instruction words or encoding constants
from it were reused. The new table packs public LLVM field definitions and is checked against
`llvm-mc-18 -triple=amdgcn-amd-amdhsa -mcpu=gfx942 -filetype=obj`.

Verification-only installed oracles: Ubuntu LLVM 18.1.3 `llvm-mc-18`, `llvm-objdump-18`,
`llvm-readelf-18` and `llvm-objcopy-18`. They are not dependencies of the producer binary.
`llvm-objcopy --dump-section` receives an explicit disposable output so it never rewrites
the emitted input. The oracle is relocatable: no `ld.lld` was installed and none was installed
for this work. The gate compares `.text` bytes, rebased disassembly, metadata, and descriptor
bytes excluding its relocated entry displacement; it does **not** claim linked-file identity.
New logs/listings and tool/source digests are filed under `evidence/AMD-GFX942-HOST-*/`.
`AMD-GFX942-HOST-20261112T210312Z` records the first commit `6ebd47b0`; `AMD-GFX942-HOST-20261112T212828Z`
records the reviewed source (MessagePack values and lengths in their full spec encodings, with a gate case
comparing every width boundary to hand-written spec bytes and a single-byte mutant that the comparison catches).
The emitted vadd object is the same in both.

Status: host byte/format checks only. AMD loader interoperability, queue dispatch, numeric FP32
outputs, boundary vectors, performance, Qwen execution and every AMD hardware row are **NOT_RUN**.

## 9. AMD KFD and AQL host data layer (2026-11-12)

`backend/amd/kfd/` is first-party seeded Omega: the 64-byte AQL kernel dispatch packet
(`aql.om`), the vadd kernel-argument block (`kernarg.om`), table-driven byte records for twelve
KFD requests plus aperture, event-wait and GPU-ID entries (`records.om` reading `requests.tsv`),
and a sequential model of the order in which a host publishes a packet into a user-mode queue
(`ring.om`). It opens no device and issues no request: the pinned compiler refuses every KFD
`devctl` request (error 9), and the gate keeps one compile-only reproduction per request.

Sources of the facts, each read for the fact named and nothing copied:

| Fact | Source |
|---|---|
| KFD request numbers, record sizes, field offsets and widths (interface 1.14) | installed `/usr/include/linux/kfd_ioctl.h`, Ubuntu `linux-libc-dev:arm64` 6.8.0-139.139, sha256 `4123a021f00f85235e3e8dda4e0b4c4ec8ed4f7ce30e8882529f68652f765247`, AMD MIT-style permission notice in the header; public upstream [include/uapi/linux/kfd_ioctl.h at v6.8](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/kfd_ioctl.h?h=v6.8) (the installed file is digest-pinned, not assumed identical) |
| The same layouts for x86-64 | installed `/usr/x86_64-linux-gnu/include/linux/kfd_ioctl.h`, `linux-libc-dev-amd64-cross` 6.8.0-25.25cross1, sha256 `d9bf51d2cde26ea18e2ec287674bb022d7b23a0858a14cdd55c55d3eb3ec339e` |
| AQL packet layout, header bit fields, packet types, fence scopes, doorbell packet-ID rule | ROCR-Runtime tag `rocm-6.2.0` (commit `1d1b8456c3530e7b464f6d74ed0baa2b28345126`) [src/inc/hsa.h](https://raw.githubusercontent.com/ROCm/ROCR-Runtime/rocm-6.2.0/src/inc/hsa.h), sha256 `2b23a43eef5b869b1a0c2245aa59e2b3da0f0c074dea5dae8d049ea4bf3a8e3b`, University of Illinois/NCSA licence; no runtime code linked |
| Code object v5 and its metadata | LLVM 18.1.3 [AMDGPUUsage.rst](https://raw.githubusercontent.com/llvm/llvm-project/llvmorg-18.1.3/llvm/docs/AMDGPUUsage.rst), sha256 `4222ab6cb3f1c73f5cdbdaedf8690a638528a9f6ba6ea3cca35ed325b8242c4c`, Apache-2.0 WITH LLVM-exception |
| vadd kernel-argument layout (a, b, c at 0, 8, 16; count at 24; 28 bytes) | `backend/amd/vadd-gfx1151.s` in this repository, sha256 `cb653f777801f65199f5243bb3db0c67f966cafc13bc99f8632c90dc0fcc1f2e`; the gfx942 kernel of section 8 uses the same layout |

`requests.tsv` was generated once by the test-only C program `tests/amd-kfd/layout-oracle.c`
from the installed header, and the gate regenerates it from the live header every run and
compares the whole table. `tests/amd-kfd/oracle.py` packs the expected bytes independently with
the Python standard `struct` module. Both are verification oracles, never on the producer path.
Test tools: GCC 13.3.0-6ubuntu2~24.04.1 (native and `x86_64-linux-gnu-gcc`), LLVM 18.1.3
`llvm-mc-18`, `llvm-readelf-18`, `llvm-objcopy-18`, Python 3.12.3 run with `-I`. No NVIDIA tool
and no fact learned from one was used.

Status: host byte and order checks only. The ring model is an execution model of publication
order, not a proof of memory ordering or doorbell coherency. KFD calls, queue creation, GPU
memory, dispatch, completion and every AMD hardware row are **NOT_RUN**.

## 10. gfx942 host instruction executor (2026-11-12)

`backend/amd/gfx942/exec/` (`fp.om`, `fma.om`, `main.om`, concatenated as one unit) is a first-party
seeded-Omega executor for the gfx942 instructions used by the kernels in sections 8, 11 and 12. It decodes the raw
`.text` words of the emitted code object (the gate extracts `.text` through the ELF section headers)
and runs wave64 workgroups over a flat byte file holding the kernel arguments and arrays.
Every instruction and branch target is checked before memory is opened; anything outside the subset is
a refusal with a defined exit (65 malformed, 66 unsupported encoding, 67 memory range). Binary32
addition is integer arithmetic (guard, round and sticky bits, ties to even, subnormals kept, no host
floating point); NaN results are the canonical quiet NaN `0x7fc00000`, a policy of this model and not
a claim about the hardware's NaN payloads.

Facts: instruction formats and opcodes from the LLVM 18.1.3 sources already listed in section 8;
IEEE 754-2019 for binary32 addition. `tests/amd-exec/oracle.py` (test only, Python standard library,
run with `-I`) computes every expected output with binary64 addition rounded to binary32 by `struct`,
which is exact for binary32 addition, and checks all output bits plus untouched words past `count`.

Status: host execution model only. Waits complete at once and memory is sequential, so this checks
what the kernel computes, not timing, races or the GPU's memory model. Every AMD hardware row is
**NOT_RUN**.

## 11. gfx942 fsum16 and table-defined kernel metadata (2026-11-12)

`backend/amd/gfx942/fsum16.isa` specifies the frozen left-to-right binary32 reduction already
defined by `backend/spirv/emit.om`: each active row loads its first input and adds the remaining
inputs in order, rounding after every addition. It does not start from zero or reassociate.
The ABI is global pointers `x` and `o` followed by uint `n`; one wave64 workgroup covers consecutive
rows. Both address paths compute the high and low halves before adding the pointers.

The producer `backend/amd/gfx942/hsaco.om` reads its kernel name, arguments, sizes, register counts
and descriptor resource words from `K`/`A` table rows (schema in `vadd.isa`). It no longer selects
metadata by a hardcoded kernel name. Identifier bytes are streamed from the seekable table, and
MessagePack and ELF sizes are derived from those bytes. This schema supports contiguous global-pointer
and uint arguments; absent/duplicate kernel declarations, unsupported argument kinds and layout gaps
are refusals with exit 4 before object output. The complete vadd object retains its previous digest,
checked by both gates.

`backend/amd/gfx942/fsum16-gfx942.s` is an independently written LLVM 18.1.3 oracle. The host gate
compares its `.text`, metadata note and descriptor fields with the Omega object and checks the
resolved kernel-entry displacement. Its metadata uses LLVM's explicit `!str n` tag: plain `n`,
quoted `"n"` and YAML's `!!str n` all gave LLVM's `invalid HSA metadata` error in this run.
Instruction formats and opcodes use only the public sources in section 8; this kernel introduces
no executor opcodes or new production dependency.

The executor interface is `exec RAW_TEXT MEMORY GRID_X GRID_Y`, with explicit decimal work-item
extents and local size `(64,1,1)`. The one-dimensional vadd/fsum16 callers pass their count and
one; tests deliberately overdispatch zero-size inputs to exercise masking. The decoder does not
infer a kernel from its filename, metadata or kernarg layout. Addition and canonical NaNs are unchanged.

`tests/amd-exec/oracle.py` computes each reduction independently by applying its rounded binary32
reference after every addition. It checks all output bits, inputs, arguments and guard bytes, and
reverses the load offsets in a test-only mutant table. The recorded order-sensitive row is
`[0x4c000000, 0xcc000000, 0x3f800000, thirteen zero words]`: the Python reference measured
left-to-right bits `0x3f800000`, reverse-order bits `0x00000000`; the executed mutant was caught.
The earlier attempted row with `0x4b800000`/`0xcb800000` was not order-sensitive and was replaced
after the oracle's explicit inequality assertion failed. No numerical tolerance is used.

Status: IMPLEMENTED and EXERCISED on the Linux AArch64 host. Receipts under
`evidence/AMD-GFX942-FSUM16-*` record the compiler pin, inputs, commands, output digests, failures
and passing gates. AMD dispatch, timing, race behaviour, hardware NaN payloads and Qwen inference
remain **NOT_RUN**. This is host code-object and instruction-model evidence, not hardware qualification.

## 12. gfx942 ordered-FMA SGEMM (2026-11-12)

`backend/amd/gfx942/sgemm.isa` implements row-major `C[M,N] = A[M,K] * B[K,N]`.
Its ABI is three global pointers followed by uint `M`, `N`, `K` (36 bytes). One wave computes a
64-column tile of one row: `s2` is group x, `s3` group y, and `v0` the lane. The descriptor enables
both workgroup-ID SGPRs. Each output starts at positive zero and executes
`acc = fma(A[m,k], B[k,n], acc)` in increasing `k`; `K=0` writes positive zero without input loads.
The code uses `v_fmac_f32`, whose destination is its third source, not a rounded multiply plus add.
Address construction uses full high/low 32-bit products and carry propagation.

Instruction authority is LLVM **llvmorg-18.1.3**:
- [VOP2Instructions.td](https://raw.githubusercontent.com/llvm/llvm-project/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOP2Instructions.td),
  `V_FMAC_F32` tied destination/source2 declaration and gfx9/VI opcode `0x3b`; `V_MUL_F32` opcode
  `0x05` is implemented for the executed separate-multiply/add negative control.
- [SOPInstructions.td](https://raw.githubusercontent.com/llvm/llvm-project/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/SOPInstructions.td),
  `S_MOV_B32_vi`, `S_ADD_U32_vi`, `S_SUB_U32_vi`, `S_ADDC_U32_vi`, `S_LSHL_B64_vi`,
  `S_MUL_I32_vi`, `S_MUL_HI_U32_vi`, and `S_CBRANCH_SCC0`/`S_CBRANCH_SCC1`.
  Existing format fields remain those listed in section 8. SCC is explicit executor state.

The table writer stores numeric labels in a growable anonymous file. References resolve to checked
signed 16-bit word offsets relative to the next instruction; duplicate/undefined labels and either
overflow direction refuse with exit 4 before object output. There is no fixed label-count capacity.
The gate checks both signed boundaries and both overflow controls; SGEMM's actual backward branch
and complete instruction text independently match `llvm-mc-18 -mcpu=gfx942` assembly.

`exec/fma.om` represents the exact finite product plus addend as an integer scaled by `2^298`.
Binary32's exponent and significand limits require at most 554 bits, held in nine unsigned 64-bit
limbs. It adds or subtracts exact magnitudes, then rounds once using a halfway bit, sticky bits and
even-significand tie-breaking, retaining denormals. Overflow is handled after cancellation and
rounding. Signed zeros and infinities follow the stated IEEE nearest-even model; NaNs remain the
model's canonical `0x7fc00000` policy, not a promise of hardware payload parity.

`tests/amd-exec/sgemm.py` is an independent **test-only** Python `fractions.Fraction` oracle.
It rounds arbitrary rational results directly to binary32, with no host float FMA or double
rounding. A streaming probe covers special-value combinations, seeded random triples, cancellation,
subnormal halfway cases and overflow canceled before rounding. Matrix cases cover dimensions
1, 3, 64, 65, empty dimensions and `K=0`; extra groups in both X and Y must preserve every guard.
The mutant replaces the actual fused instruction by separately executed `v_mul_f32`/`v_add_f32`
and must fail bit-for-bit comparison. Host gates retain all vadd/fsum16 controls and exact object
digests, compare SGEMM metadata/descriptor bytes with independent assembly (including the group-Y
enable bit), and build both executables and objects twice for byte identity.

Receipts live under `evidence/AMD-GFX942-SGEMM-*`. AMD dispatch, performance, GPU memory ordering,
hardware NaN payloads and Qwen inference remain **NOT_RUN**. This host model is not hardware qualification.

### 12.1. CDNA3 semantics correction and complete subset audit

The initial SGEMM host receipt at `df9c14a74c549adb029d243eaed5cb10fceecc84` established
encoding/model agreement, **not correct ISA semantics**: `S_SUB_U32` incorrectly set SCC for
no-borrow, and the kernel used branches matching that error. The correction uses SCC = borrow
(`S1 > S0`), exits a row on SCC0, stores zero for empty K on SCC1, and continues K on SCC0.
The historical receipt is retained unchanged; the correction receipt is a separate record.

Semantic authority is the AMD *Instinct MI300 Instruction Set Architecture (CDNA3)* manual,
local source `~/docs/amd-gpu/AMD_Instinct_MI300_CDNA3_ISA.txt`. Page numbers below are **physical
PDF pages** (`doc-page amd-gpu/AMD_Instinct_MI300_CDNA3_ISA PAGE`), not the printed footer.
LLVM 18 remains the independent encoding oracle, not an instruction-semantics oracle.

| Implemented instruction | PDF page | Audited operation and side effects |
|---|---:|---|
| S_LOAD_DWORD | 68, 146 | One dword; each address component ignores its low two bits; flags unchanged. |
| S_LOAD_DWORDX2 | 68, 70, 146 | Two consecutive dwords; even destination SGPR pair. |
| S_LOAD_DWORDX4 | 68, 70, 147 | Four consecutive dwords; quad-aligned destination SGPRs. |
| S_ADD_U32 | 97 | Low32 sum; SCC is unsigned carry-out. |
| S_SUB_U32 | 97 | Low32 difference; SCC is unsigned **borrow** (`S1 > S0`). |
| S_ADDC_U32 | 98 | Sum includes incoming SCC; replaces SCC with carry-out. |
| S_LSHL_B32 | 103 | Shift count masked to five bits; SCC is nonzero result. |
| S_LSHL_B64 | 103 | Shift count masked to six bits; even pairs, low word first; SCC is nonzero full result. |
| S_MUL_I32 | 104 | Low32 product, signed/unsigned low bits identical; preserves SCC. |
| S_MUL_HI_U32 | 106–107 | High32 unsigned product; preserves SCC. |
| S_MOV_B32 | 115 | Bitwise copy; preserves SCC. |
| S_AND_SAVEEXEC_B64 | 124–125 | Read source and old EXEC, AND into EXEC, save old EXEC to destination, SCC = new EXEC nonzero. Source/destination alias follows the pseudocode on p125. |
| S_ENDPGM | 138 | Terminates the wave; implicit wait is satisfied by synchronous memory. |
| S_CBRANCH_SCC0 | 139 | Branch iff SCC=0; signed16 DWORD displacement relative to next instruction; preserves flags. |
| S_CBRANCH_SCC1 | 139 | Branch iff SCC=1; same displacement; preserves flags. |
| S_CBRANCH_EXECZ | 140 | Branch iff whole EXEC=0; same displacement; preserves flags. |
| S_WAITCNT | 141 | Supported vmcnt/lgkmcnt waits complete immediately in the synchronous model; no flag writes. |
| V_ADD_F32 | 169 | FP32 add, nearest-even, input/output denormals retained. |
| V_MUL_F32 | 170 | FP32 multiply with one rounding; implemented by exact FMA with a product-signed zero. |
| V_LSHRREV_B32 | 173 | Unsigned right shift of S1 by S0[4:0]; no flags. |
| V_LSHLREV_B32 | 173 | Low32 left shift of S1 by S0[4:0]; no flags. |
| V_AND_B32 | 174 | Bitwise AND; no flags. |
| V_ADD_CO_U32 | 175, 22 | Low32 sum; VCC receives carry-out for active lanes and zero for inactive lanes. |
| V_ADDC_CO_U32 | 176, 22 | Includes incoming per-lane VCC, then replaces the whole VCC mask. |
| V_ADD_U32 | 183 | Low32 sum; no carry-in or carry-out, preserves VCC. |
| V_FMAC_F32 | 185 | Destination is third input; exact multiply-add followed by one rounding. |
| V_MOV_B32 | 186 | Bitwise copy to active lanes only; no flags. |
| V_CMP_GT_U32 | 251, 22 | Unsigned greater-than, VCC = EXEC AND comparison mask; EXEC unchanged. |
| GLOBAL_LOAD_DWORD | 87, 90, 503–504 | 64-bit even VGPR address pair plus signed13 byte offset; active lanes load a dword. |
| GLOBAL_STORE_DWORD | 87, 90, 505 | Same address calculation; active lanes store a dword. |

All vector source operands observe the incoming VCC, including scalar VCC halves used as sources.
All VCC-writing instructions replace the whole mask (p22), including when EXEC is zero.
These rules fix inactive carry bits and read-after-write contamination in the sequential host lane loop.
The scalar-memory fix masks each address component separately (p68), not their sum.
Misaligned multi-dword SGPR destinations and 64-bit VGPR address pairs refuse with exit 66 (pp20–21,
70). Negative immediate SMEM offsets with SOE=0 are undefined on p68 and refuse with exit 66:
the general format table calls the field unsigned (p67), while individual load descriptions call
it signed (pp146–147); the executor does not guess at this undefined case.

General FP controls are p45: round mode 0 is nearest-even and denorm mode 3 retains inputs and
outputs, matching the descriptors. The exact integer FP implementations and independent Fraction
checks remain as described above. The canonical-NaN host policy is explicit; NaN payload propagation,
exceptions, XNACK replay/clause hazards, memory races, timing, and hardware dispatch are not modeled.
Only the kernels' synchronous flat-memory contract is exercised; this audit does not qualify hardware.

`tests/amd-exec/semantics.py` contains manual-derived register/flag input/output cases, not expected
values generated from the executor. A test-only sparse-state adapter calls the same production
`step` routine used by wave execution, with actual encoded instruction words. Cases check inputs
and untouched registers, flags, lane 63, inactive lanes, carry/borrow edges, masked shifts, signed
branches, literals, SMEM component alignment, and GLOBAL offsets and memory guards. Executable
mutants independently restore each corrected old behavior, including the inverted SCC, and each
must fail its corresponding case. The final PASS derives the case count from the actual case table;
all previous vadd/fsum16/SGEMM counts and controls remain. Hardware remains **NOT_RUN**.

## 13. gfx942 E4M3FNUZ GEMV

`backend/amd/gfx942/fp8gemv.isa` defines `fp8gemv(W,x,s,y,M,K)`: four 64-bit
global pointers followed by two unsigned 32-bit dimensions, a 40-byte kernarg.
W is row-major M-by-K **E4M3FNUZ** bytes; x[K], s[M], and y[M] are FP32.
For each row m, acc starts at positive zero. In ascending k order, compute
`acc = RN32(dequant(W[m*K+k]) * x[k] + acc)` with a single fused rounding.
After the loop, compute `y[m] = RN32(s[m] * acc)` **once**, not a scale on each
product or each accumulation step. K=0 still applies the scale to positive zero.
The grid is `(M,1,1)`, local size `(64,1,1)`; each lane owns one row. The EXEC
guard excludes every row at or beyond M, including deliberately extra groups.
Pointer products and additions retain both 32-bit halves.

### Format and encoding authority

AMD [HIP 6.2.4 FP8 Numbers](https://rocm.docs.amd.com/projects/HIP/en/docs-6.2.4/reference/fp8_numbers.html),
“Supported Devices”, explicitly lists **gfx940/gfx941/gfx942: FNUZ FP8 Yes,
OCP FP8 No**. Its text says: “if you use this on gfx94x GPU, it will be fnuz
number”, and defines NaN as sign set with all exponent and mantissa bits zero.
LLVM [18.1.3 APFloat.h lines 162–179](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/include/llvm/ADT/APFloat.h#L162-L179)
defines E5M2FNUZ bias16 and E4M3FNUZ bias8, with no infinity or signed zero;
NaN occupies the negative-zero encoding. This kernel uses E4M3FNUZ only.
It does not interpret weights as OCP E4M3FN (bias7, signed zero, other NaN codes).

The AMD Instinct MI300 CDNA3 ISA, physical PDF pp58–60 (same locally hashed
manual as section12.1), gives the FP8 format table and conversion rules.
FP8 is E4M3, bias8, sole NaN0x80, sole zero0x00; exponent15 values are finite.
The BF8 table's INF entry conflicts with its NaN entry; no BF8 operation is
implemented or inferred from that cell. The explicit LLVM/HIP FNUZ definitions
above establish the BF8 format name, not hardware qualification.

LLVM18 [VOP1Instructions.td lines 589–621](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOP1Instructions.td#L589-L621)
selects SDWA for byte0 under `HasCvtFP8VOP1Bug`.
[Lines 1418–1444](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOP1Instructions.td#L1418-L1444)
assign CVT_F32_FP8 opcode0x54 and fix SDWA destination selector bits42–40 to6.
The kernel uses that SDWA BYTE0 form even where a non-SDWA encoding might exist.
[VOP3Instructions.td lines1604–1611](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOP3Instructions.td#L1604-L1611)
assign MUL_LO_U32=0x285 and MUL_HI_U32=0x286. The format definitions cited in
section8 supply VOP3A and SDWA bit positions. FLATInstructions.td's gfx9 GLOBAL
table assigns unsigned-byte load opcode0x10. The host gate assembles the
independent `fp8gemv-gfx942.s` with `llvm-mc-18 -mcpu=gfx942` and compares
the complete instruction section, metadata note, and non-relocated descriptor
bytes, rather than treating an accepted mnemonic as encoding proof.

| Added instruction | Physical PDF page | Semantics and state effects checked by single-step cases |
| --- | --- | --- |
| V_CVT_F32_FP8 SDWA BYTE0 | 58–60, 211–212 | Convert low byte from E4M3FNUZ; active lanes only; preserve EXEC, VCC, SCC. All byte values checked, upper source bytes ignored. |
| V_MUL_LO_U32 | 361 | Low32 of unsigned 32-by-32 multiplication; preserve flags and inactive destinations. |
| V_MUL_HI_U32 | 361 | High32 of the same unsigned 64-bit product; preserve flags and inactive destinations. |
| GLOBAL_LOAD_UBYTE | 87, 90, 503 | Same global address rule as dword load; exactly one byte read and zero-extended, including final byte of memory. |

The executor accepts only conversion BYTE0 with LLVM's fixed destination selector
and zero modifiers. Other selectors, non-SDWA conversion, other conversion opcodes,
and unsupported modifiers refuse66. Its VOP3 multiply subset accepts a VGPR src0
and an SGPR or VGPR src1, no src2 or modifiers. Byte loads retain address/range
refusals67 and I/O refusals74. Validation still decodes every word before opening
memory for execution. The conversion is integer-only in seeded Omega: extract
the sign/exponent/fraction, normalize an FP8 subnormal, and construct exact FP32
bits. No finite value requires rounding. The host maps the sole FP8 NaN to
canonical FP32 0x7fc00000; this does **not** claim hardware NaN payload parity.
FP32 multiplication and accumulation retain the exact integer FMA implementation
and one nearest-even rounding, with denormals preserved.

The producer's former eight-format array is replaced with an anonymous-file
registry, growing with declared formats. Each record's field capacity derives
from the maximum64 instruction bits; it does not bound the number of formats.
Format IDs are keys, not array offsets. Branch references retain reserved SOPP
format ID4. Registry checks cover sparse high IDs, extra unused formats with an
unchanged object, and undeclared-format refusal before output. All earlier code
objects must remain byte-identical to the corrected SGEMM chain.

### Acceptance and hardware boundary

`tests/amd-exec/fp8gemv.py` independently decodes the format into exact Python
Fractions, then uses the rational FP32 rounding oracle. The single-step table
checks every byte's conversion without GEMV or scaling hiding an error. The
kernel gate also runs an all-byte matrix, the Cartesian M/K shape table, empty
rows, finite/special FP32 inputs and scales, and whole-memory guards. Executable
controls substitute a complete OCP E4M3FN converter and replace the scale
multiply with a move; each must produce a bit mismatch. Earlier gates, shapes,
SCC/VCC/addressing controls, and wrong-compiler preflight refusal remain.
Final PASS counts are derived from the test tables. Gate receipts record actual
commands, logs, compiler errors, outputs, hashes and observed exits.

**Hardware NOT_RUN — required notebook check:** the manual's exact text on
physical PDF p60 is: “The register SH_MEM_CONFIG, bit[8] must be set to 1 to
produce the correct results for BF8 and FP8 operations.” When that bit is set,
this kernel assumes **E4M3FNUZ** weight bytes (bias8, unsigned zero0x00, NaN0x80),
not OCP E4M3FN. This host executor neither reads nor writes SH_MEM_CONFIG.
The notebook must establish that register precondition through an authorized
hardware path before dispatch. No register state, GPU dispatch, timing,
throughput, ROCr loading, or device-output parity has been measured here.
