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

## 14. gfx942 BF16 GEMV

`backend/amd/gfx942/bf16gemv.isa` defines `bf16gemv(W,x,y,M,K)`: three
64-bit global pointers at offsets0/8/16, unsigned32 M/K at24/28, a32-byte
kernarg. W is row-major little-endian BF16, two bytes per element; x[K] and
y[M] are FP32. Each row starts with acc=positive zero, then, for ascending k,
`acc = RN32(BF16(W[m*K+k]) * x[k] + acc)` with **one fused rounding**.
The output is acc; K=0 writes positive zero and M=0 writes nothing.
The grid is `(M,1,1)` with local size `(64,1,1)`, one row per lane. EXEC
excludes over-dispatched rows. The row product is multiplied and doubled
with both32-bit halves and carry propagation; valid arrays must fit the
machine's64-bit address space. No input is rounded to BF16.

### Exact widening and the instruction choice

BF16 is S1E8M7, bias127. LLVM18.1.3
[APFloat.cpp lines127–129](https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Support/APFloat.cpp#L127-L129)
gives BF16 and FP32 the same exponent range, with precisions8 and24.
**BF16-to-FP32 widening is exactly `uint32(bf16_bits) << 16`**: no finite
rounding, zero flushing, or reinterpretation as IEEE FP16. Signed zeros,
infinities and NaN sign/payload bits are carried by the widening itself.
Subsequent FMA uses the existing explicit canonical-NaN host policy, which
does not establish device NaN payload parity.

The CDNA3 manual physical PDF p507 specifies `GLOBAL_LOAD_SHORT_D16_HI`:
`VDATA[31 : 16].b16 = MEM[addr].b16; // VDATA[15:0] is preserved.`
Initialize v7 once with `V_MOV_B32 v7,0` (p186), then use this high-half load
for each weight. Nothing in the loop changes v7's low16 bits. This performs
load and exact widening in **one instruction per weight**, rather than a
ushort load followed by a shift. This is an instruction-count choice, not
a measured throughput claim. Each iteration waits for VM_CNT before FMA
and before the next partial-register load. BF16 has no FP8 format-select
precondition: this path is a bitwise load followed by FP32 FMA; descriptor
FP32 round mode0 and denorm mode3 retain input/output denormals (p45).

### Semantics and encoding authority

The manual is the same physical-page-numbered CDNA3 source as section12.1.
All instructions used by this kernel are listed below, including reused
ones. Encoding links name the pinned LLVM18.1.3 format/opcode definitions;
the independent `bf16gemv-gfx942.s` is assembled by `llvm-mc-18 -mcpu=gfx942`.
The gate compares **complete .text**, metadata note, descriptor bytes other
than the ET_REL entry relocation, and checks the resolved entry separately.

| Instruction | CDNA3 physical PDF page | Semantics | LLVM18.1.3 encoding |
| --- | --- | --- | --- |
| S_LOAD_DWORDX4 | 68,70,147 | Four dwords; component alignment, quad SGPR destination; flags unchanged. | [SMInstructions.td][bf16-sm], SMEM and `_vi` opcode |
| S_LSHL_B32 | 103 | Low32 shift, count masked5bits; SCC=result nonzero. | [SOPInstructions.td][bf16-sop], SOP2 |
| S_SUB_U32 | 97 | Low32 difference; SCC=unsigned borrow. | [SOPInstructions.td][bf16-sop], SOP2 |
| S_AND_SAVEEXEC_B64 | 124–125 | Save old EXEC, AND source into EXEC; SCC=new EXEC nonzero. | [SOPInstructions.td][bf16-sop], SOP1 |
| S_CBRANCH_EXECZ | 140 | Branch iff whole EXEC zero; signed16 DWORD displacement from next PC. | [SOPInstructions.td][bf16-sop], SOPP |
| S_CBRANCH_SCC0 / S_CBRANCH_SCC1 | 139 | Branch iff SCC=0 / SCC=1; same displacement; flags unchanged. | [SOPInstructions.td][bf16-sop], SOPP |
| S_WAITCNT | 141 | Wait for lgkmcnt/vmcnt0; no flag writes. | [SOPInstructions.td][bf16-sop], SOPP |
| S_ENDPGM | 138 | End wave; implicit memory wait. | [SOPInstructions.td][bf16-sop], SOPP |
| V_AND_B32 | 174 | Bitwise AND, flags unchanged. | [VOP2Instructions.td][bf16-vop2], VOP2e |
| V_ADD_U32 | 183 | Low32 sum, no carry flags. | [VOP2Instructions.td][bf16-vop2], gfx9 |
| V_CMP_GT_U32 | 22,251 | VCC=EXEC AND unsigned greater-than mask. | [VOPCInstructions.td][bf16-vopc], VOPCe |
| V_LSHLREV_B32 / V_LSHRREV_B32 | 173 | Left / unsigned right shift, count masked5bits; flags unchanged. | [VOP2Instructions.td][bf16-vop2], VOP2e |
| V_MUL_LO_U32 / V_MUL_HI_U32 | 361 | Low / high32 unsigned product; flags unchanged. | [VOP3Instructions.td][bf16-vop3], opcodes0x285/0x286 |
| V_MOV_B32 | 186 | Bitwise copy to active lanes; flags unchanged. | [VOP1Instructions.td][bf16-vop1], VOP1e |
| V_ADD_CO_U32 | 22,175 | Low32 sum; replace VCC with active-lane carry mask. | [VOP2Instructions.td][bf16-vop2], VOP2e |
| V_ADDC_CO_U32 | 22,176 | Include incoming lane carry; replace VCC. | [VOP2Instructions.td][bf16-vop2], VOP2e |
| GLOBAL_LOAD_SHORT_D16_HI | 87,90,507 | Read exactly two bytes into high16; preserve low16, flags, inactive lanes. | [FLATInstructions.td][bf16-flat], `FLAT_Real_AllAddr_vi<0x25>` |
| GLOBAL_LOAD_DWORD | 87,90,503–504 | Active-lane32-bit load, VGPR64 address plus signed13 offset. | [FLATInstructions.td][bf16-flat], opcode0x14 |
| GLOBAL_STORE_DWORD | 87,90,505 | Corresponding active-lane32-bit store. | [FLATInstructions.td][bf16-flat], opcode0x1c |
| V_FMAC_F32 | 185 | Exact product plus destination, one nearest-even FP32 rounding. | [VOP2Instructions.td][bf16-vop2], gfx940 opcode0x3b |

[bf16-sm]: https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/SMInstructions.td
[bf16-sop]: https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/SOPInstructions.td
[bf16-vop1]: https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOP1Instructions.td
[bf16-vop2]: https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOP2Instructions.td
[bf16-vopc]: https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOPCInstructions.td
[bf16-vop3]: https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOP3Instructions.td#L1604-L1611
[bf16-flat]: https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/FLATInstructions.td#L1969-L1993

The only new executor instruction is `GLOBAL_LOAD_SHORT_D16_HI`, with the
existing restricted GLOBAL VGPR64/off addressing, no cache modifiers.
It reads exactly two little-endian bytes and preserves destination low16.
Other D16 forms or modifiers refuse66; insufficient memory refuses67,
I/O failure74. The single-step cases derive expected high/low halves from
p507, including lane63, inactive lanes, unchanged flags, negative offsets,
the final two bytes of memory, and truncated-memory refusals. A low-half
clearing mutant must fail its manual-derived case.

### Acceptance and hardware boundary

`tests/amd-exec/bf16gemv.py` independently reconstructs BF16 finite values
as exact Fractions and applies the existing rational single-rounding FMA
oracle, not host floating arithmetic. The gate runs the Cartesian shape
table M={0,1,63,64,65}, K={0,1,3,64,65,130} for finite and special inputs.
The weight base is two-byte aligned but not dword aligned, so odd K tests
both row-end alignments. Every byte of memory is checked: output elements,
guard words, inputs, kernarg, and deliberately over-dispatched rows.
An additional isolated-weight matrix checks each special class without an
earlier NaN masking later operations. Controls execute a big-endian weight
loader and an instruction-table mutant that truncates x's low16 bits before
FMA; both must produce output-bit mismatches, not compilation/refusal errors.
Previous kernel objects remain byte-identical and all prior gate cases
remain. Final PASS counts come from the actual case tables.

**Hardware NOT_RUN:** no AMD device, dispatch, memory-ordering/timing,
throughput, ROCr loading, model weights, or Qwen end-to-end inference was
exercised. The binary/text and host semantic gates are not GPU qualification.

## 15. gfx942 wave-parallel FP8/BF16 GEMV

`fp8gemv2.isa` and `bf16gemv2.isa` are separate kernel symbols and objects;
the previous five kernels and producer remain unchanged. Their argument
layouts and datatypes are the same as sections13–14. **Launch workgroup
size64, GRID_X=64*M, GRID_Y=1.** Workgroup X is the output row; lane0 alone
stores. An extra row group safely returns before touching vector memory.
The descriptor does not request workgroup Y. The new kernels are not
drop-in replacements for the old kernels' launch geometry.

### Hardware observation and design

The operator's MI300X notebook record
`~/handoffs/hive/amdserve/evidence/AMD-MI300X-NOTEBOOK-20261112T224000Z`
reports bit-exact old-kernel results. FP8 times for MxK
17408x5120, 5120x17408, 5120x5120 were0.7472,2.5073,0.7295ms;
BF16 times were0.7402,2.5001,0.7399ms. Time followed K rather than weight
bytes. That observation motivates removing the dependent load/wait for
every scalar k; it does not establish a tuned-kernel speedup.

The CDNA3 manual physical PDF p12 defines a wavefront as64 work-items.
[AMD's MI300 architecture description][gemv2-arch] specifies304 active CUs.
One wave per row supplies M independently schedulable workgroups rather
than the old ceil(M/64) groups. The [ROCm Compute Profiler7.2.2 vector-L1
description][gemv2-cache] specifies64B cache lines and explains address
coalescing and backpressure from outstanding requests. Each lane owns four
consecutive columns. Across the wave, a chunk accesses contiguous weight
and x ranges, instead of loading one scalar from widely separated rows.
FP8 packs those four weights into one DWORD; BF16 uses DWORDX2; x uses
DWORDX4. These are consecutive-byte accesses, not a claim that a memory
instruction is a single cache transaction.

The main loop prefetches four chunks into disjoint VGPRs: four weight loads
and four x loads before one `s_waitcnt vmcnt(0)`. The CDNA3 manual p141
defines this wait in terms of outstanding vector-memory operations; it
does not require a wait after each issue. The load group processes1024
columns before advancing pointers. The remaining columns use guarded
scalar loads with EXEC saved/restored for every element. The batch size is
a tuning choice, not an imposed input-size limit. No input is truncated.
Resource use and the complete instruction stream are compared against
LLVM18 gfx942, including all wide loads, FMAC, EXEC moves and DS operations.

No LDS storage or barrier is used. `DS_BPERMUTE_B32` (p437) says:
“This does not access LDS memory and may be called even if no LDS memory
is allocated to the wave. It uses LDS hardware to implement an arbitrary
swizzle across threads in a wavefront.” Its LLVM18 instruction declaration
is under `let Uses = [EXEC]`; no M0/LDS-range setup is required here.
Each permutation is followed by `s_waitcnt lgkmcnt(0)` before the add.

### Exact accumulation order

For lane L, start P[L]=+0. Visit chunks q=0,1,... in ascending order and
j=0,1,2,3 within each chunk. When k=256*q+4*L+j is less than K, set
P[L]=RN32_FMA(decode(W[row,k]),x[k],P[L]). The unrolled load grouping does
not change this order. Then perform six simultaneous butterfly stages
with distance32,16,8,4,2,1, in that exact order:
`next[L] = RN32_ADD(P[L], P[L XOR distance])`. Each stage reads a snapshot
of the previous stage. Store lane0, applying one final RN32 scale
multiplication for FP8 only. Missing tail elements cause no arithmetic,
not artificial zero products that might alter signed zero or NaN results.
K=0 still follows the defined zero reduction and FP8 scaling.

`tests/amd-exec/gemv2.py` records this algorithm using exact `Fraction`
arithmetic and the independent binary32 rounding/FMA reference. Its add
is the exact rational equivalent FMA(a,1,b), not host float arithmetic.
The Omega executor's integer FP32 add/FMA paths retain denormals and use
round-to-nearest-even; the explicit canonical-NaN policy from section10
remains. Arithmetic order differs deliberately from the old serial GEMV.
The tree-order control places +2^24,+1,-2^24 in lanes0,16,32 respectively;
swapping the first two stages must change the output, not merely the code.

### Added instruction semantics and refusal boundary

All reused instructions retain the manual audit in sections12–14.
New operations below have manual-derived single-step input/output/flag
cases, including inactive lanes and refusals. The bounded source/destination
subset is explicit: this is an executor, not a claim of complete gfx942.

| Instruction | CDNA3 physical PDF page | Implemented semantics and boundary | LLVM18.1.3 source |
| --- | --- | --- | --- |
| S_MOV_B64 | 20,115 | Even SGPR pair to EXEC only; exact64-bit copy, SCC/VCC unchanged. Other forms refuse66. | [SOPInstructions.td][bf16-sop], SOP1 opcode1 |
| V_XOR_B32 | 174 | Active-lane bitwise XOR; no flag writes or modifiers. | [VOP2Instructions.td][bf16-vop2], opcode21 |
| GLOBAL_LOAD_DWORDX2 / DWORDX4 | 87,90,504 | Sequential2/4 little-endian dwords from the existing VGPR64/off address; flags and inactive lanes preserved. Full range checked before writes. | [FLATInstructions.td][bf16-flat], opcodes21/23 |
| DS_BPERMUTE_B32 | 437 | Source lane=((address+offset)/4)%64, disabled source returns0, disabled destination unchanged; all source reads precede all writes. GDS/accumulator/DATA1 modifiers refuse66. | [DSInstructions.td][gemv2-ds], `DS_Real_vi`, opcode63 |

Instruction words alone drive execution; the executor does not recognize
kernel names, ISA tables, metadata, or expected answers. Unknown encodings
refuse66. Truncated/bad text refuses65, flat-memory bounds67, I/O74, CLI64;
the gate rejects a wrong compiler digest69 before invoking any compiler.
The64-entry permutation temporary is the physical wave width, not an
arbitrary software capacity. Per-instruction controls deliberately lose
SCC on MOV64, substitute OR for XOR, omit a wide-load dword, and read an
aliased permutation source live; all must fail the manual expectations.

### Host acceptance and hardware handoff

The gate retains all prior shapes and cases. New shape tables are the
union of both old GEMV tables plus chunk-boundary/tail cases; the final
PASS line derives its counts from the actual tables. Finite and special
inputs check every output bit and every unchanged memory byte, including
past-M guards. Additional controls change the reduction order, drop the
tail, or change the row stride and must produce output mismatches.
For each of the three notebook shapes, a test-only row entry executes
first, middle and last rows with the real full M/K and full matrix address
layout. All other output bytes must stay untouched. The production
executor CLI is unchanged. The sampled row results are recorded in a TSV.
Both compiler builds and emitted objects must be byte-identical.

**Tuned hardware NOT_RUN:** performance, cache traffic, memory scheduling,
device NaN payloads and device results require the parent's MI300X run.
Host execution is not a memory-bandwidth measurement. BF16 uses exact
widening; FP8 assumes E4M3FNUZ (bias8, sole NaN0x80). Beside that hardware
row, the exact manual p60 precondition remains:
“The register SH_MEM_CONFIG, bit[8] must be set to 1 to produce the correct
results for BF8 and FP8 operations.”

[gemv2-arch]: https://rocm.docs.amd.com/en/docs-6.3.1/conceptual/gpu-arch/mi300.html
[gemv2-cache]: https://rocm.docs.amd.com/projects/rocprofiler-compute/en/docs-7.2.2/conceptual/vector-l1-cache.html
[gemv2-ds]: https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/DSInstructions.td

## 16. gfx942 tiled full-attention core (27B)

### Model and boundary

The exact [Qwen3.8-27B-FP8 config at revision
017b9c7af6b5689d5dd426a76e0bc077eb5ca20a][attn-config] is filed unchanged at
`tests/amd-exec/model/qwen38-27b-fp8-config.json`. Its digest is checked
against the historical Q27 manifest, rather than a mutable model name.
Its `text_config` specifies `num_attention_heads=24`,
`num_key_value_heads=4`, `head_dim=256`, `attn_output_gate=true`,
`output_gate_type="swish"`, and full attention every fourth layer.
The same configuration specifies `rms_norm_eps=1e-6`,
`partial_rotary_factor=0.25`, and `rope_theta=10000000`.

This commit implements **only the attention core**: Q is FP32, already
Q-normalized and rotated; the BF16 K/V cache already contains the newest
token. It writes FP32 attention values before the output gate. Q/K
normalization, RoPE, cache append, and the output swish/sigmoid gate belong
to separate kernels, not an implicit assumption that they are unnecessary.
The mathematical reference is `tests/gpu_decode_attn256_test.c`: lines
50–52 establish24/4 heads and6 queries per KV head, lines61–66 give the
cache layout, and lines125–143 specify GQA mapping, the scaled dot product,
max-subtracted softmax, and weighted V. Lines100–111 describe the preceding
zero-centered `(1+w)` Q/K normalization and partial RoPE; lines118–123
describe K/V BF16 cache append. These are model-math references, not
NVIDIA tool or instruction facts.

Both kernels share a48-byte kernarg:
Q pointer at0, K at8, V at16, partials at24, output at32, token count T
at40, cache capacity at44; the last two fields are unsigned32-bit.
Q and output are `[24][256]` FP32. K and V are `[4][capacity][256]` BF16.
T means **number of valid tokens**, not the newest zero-based position;
the caller requires T<=capacity. A prep launch appending position P feeds
T=P+1 to this core. Nonfinite model inputs are outside the numerical
qualification fixtures; unsupported instructions and memory violations
still have the executor's defined refusals.

`attn256_part`: launch `(ceil(T/64),24)` workgroups of64 lanes, with
workgroup X selecting a64-token tile and workgroup Y selecting the Q head.
Both dimensions may be overdispatched: invalid tiles/heads return without
accessing arrays. `attn256_merge`: launch `(1,24)` workgroups of64 lanes.
The executor CLI takes **work-item grid X**, so its X argument is64 times
the workgroup count, not the count itself. Both descriptors enable
workgroup-id Y; the gate compares their descriptor bytes with LLVM18 and
checks that bit explicitly.

Partials contain `[24][ceil(T/64)]` records of1040 bytes: FP32 maximum
at0, denominator at4, eight untouched padding bytes at8, and256 FP32
unnormalized weighted sums at16. T=0 reads no Q/K/V/partial records and
writes zero output. Partial buffers are scratch, not a persistent KV
cache or a host computation dependency.

### Scheduling and exact arithmetic order

The wave64,304-CU and64-byte cache-line references in section15 apply here.
One wave per head alone would offer only24 workgroups. Tiling the context
adds independently schedulable workgroups without atomics or cross-wave
barriers. Each lane owns four adjacent dimensions. Q loads use DWORDX4;
the K and V loads for a token are DWORDX2, both issued before a single
`vmcnt(0)` wait. Thus all lanes cover contiguous512-byte BF16 rows, not
one strided scalar each. DS_BPERMUTE uses no allocated LDS; its lgkm wait
and snapshot semantics are unchanged from section15. Hardware bandwidth,
occupancy and performance remain NOT_RUN.

Within each tile, visit tokens in ascending order. Each lane starts its
dot accumulator at +0 and executes four RN32 FMA operations for dimensions
`4*lane+j`, j=0,1,2,3. Six simultaneous XOR butterfly adds at distances
32,16,8,4,2,1 follow. Multiply the resulting score by exact FP32
`1/sqrt(256)=1/16=0.0625`. Every active lane obtains the same score.
Initialize maximum m=-infinity, denominator l=+0, and each sum a=+0.
For score s, set:

* n=MAX32(m,s).
* alpha=EXP2_32(RN32_MUL(RN32_SUB(m,n),LOG2E32)).
* beta=EXP2_32(RN32_MUL(RN32_SUB(s,n),LOG2E32)).
* l=RN32_ADD(RN32_MUL(l,alpha),beta).
* a[d]=RN32_FMA(beta,V[d],RN32_MUL(a[d],alpha)).
* m=n.

LOG2E32 is the binary32 constant0x3fb8aa3b. The merge visits tile records
in ascending tile order, reweights old and tile statistics by their
maxima in the same manner, and uses FMA for both the merged denominator
and sums. Finally it computes RCP32(l) once and multiplies each sum by it.
It does not claim the same rounding order as a monolithic softmax or the
Spark reduction.

### Transcendental model and manual audit

Ordinary add/subtract/multiply/FMA retain the executor's exact RN-even
integer algorithms and denormal preservation. EXP and RCP are different:
the ISA promises1ULP, not device-identical answer bits. The executor
selects one representative within that allowance. EXP uses binary64
range reduction, an18-term Taylor polynomial of exp(r*ln2), and exact
binary64 powers of two before RN32 conversion. RCP uses binary64 division
then RN32 conversion. Both implement the manual's input/output FTZ and
special-value behavior. This binary64 use is in seeded Omega, not a host
library or a production Python dependency.

Recorded Wolfram query:
`{N[Log[2],22], N[2 Log[2]^19/19!,22], N[1/Log[2],22], 1/Sqrt[256]}`
returned ln2=0.69314718055994530941723212145817656807,
the Taylor remainder bound1.55460168577147128381779943180201880963e-20,
log2(e)=1.44269504088896340735992468100189213743, and1/16.
The finite-precision implementation is additionally checked against
independent Decimal120-digit enclosures, not justified by the polynomial
remainder alone.

| Instruction | CDNA3 physical PDF page | Semantics, flags, refusal boundary | LLVM18.1.3 source |
| --- | --- | --- | --- |
| S_LSHR_B32 | 103 | Unsigned source shifted by low5 shift-count bits; SCC iff result nonzero. Other flags unchanged. | [SOPInstructions.td][bf16-sop], opcode30 |
| V_SUB_F32 | 169 | RN32 subtract, denormals preserved; active lanes only, EXEC/VCC/SCC unchanged. | [VOP2Instructions.td][bf16-vop2], opcode2 |
| V_MAX_F32 | 172 | IEEE signaling-NaN quieting first, quiet NaN selects numeric operand, +0 preferred to -0; active lanes only, flags unchanged. | [VOP2Instructions.td][bf16-vop2], opcode11 |
| V_EXP_F32 | 195 | pow(2,S0),1ULP, input/output denormals flushed; -infinity maps0, signed zero maps1, +infinity maps+infinity. | [VOP1Instructions.td][attn-vop1], opcode32 |
| V_RCP_F32 | 196 | Reciprocal,1ULP, input/output denormals flushed; signed zero/infinity map to signed infinity/zero. | [VOP1Instructions.td][attn-vop1], opcode34 |

Every new opcode has manual-derived single-step cases checking results
and flags; the shared vector-lane dispatcher retains its inactive-lane cases. The
gate retains all earlier cases. Literal/source/modifier refusal boundaries
remain those in sections10–15; no kernel-name recognition is introduced.

`attention.py` independently uses Fraction arithmetic with RN-even at
each ordinary instruction boundary and Decimal120-digit EXP reference
values. It encloses the manual's1ULP allowance and rounds interval
endpoints outward; an interval is a conservative enclosure, not a claim
that every value inside is an allowed device result. These intervals are
propagated through every reweighting, accumulation and final reciprocal.
Every partial statistic and output must lie inside its own interval.
All inputs, cache tails, padding, and guard bytes must stay unchanged.
Per-output exact rational interval bounds and widths are emitted as JSON;
each case prints the widest interval. No fixed arbitrary output tolerance
replaces the per-instruction bound. Controls remove max subtraction,
mis-map GQA, and load V from K; each must produce a numerical mismatch.

**Hardware NOT_RUN:** MI300X execution of these attention objects, timing,
the actual transcendental choices, and end-to-end model output require
the parent's hardware bundle. Host byte parity and bounded numerical
checks are not hardware qualification or a measured speedup.

[attn-config]: https://huggingface.co/Qwen/Qwen3.8-27B-FP8/resolve/017b9c7af6b5689d5dd426a76e0bc077eb5ca20a/config.json
[attn-vop1]: https://github.com/llvm/llvm-project/blob/llvmorg-18.1.3/llvm/lib/Target/AMDGPU/VOP1Instructions.td

## 17. gfx942 fused full-attention preparation and output gate

This follows the same pinned [Q27 config][attn-config] as section16.
The gate verifies its digest and the compiled specialization:24 Q heads,
4 KV heads,256 dimensions, epsilon1e-6, theta10000000, and64 rotary
dimensions (partial factor0.25). This is the text decode path: the three
MRoPE position axes all equal P, so interleaving sections[11,11,10]
does not change the32 inverse-frequency indices. Multimodal unequal-axis
position construction is not implemented by this text-position producer.

### Reference and layouts

Reference: [Transformers v5.8.0 modeling_qwen3_5.py][prep-reference].
Lines128–140 define inverse frequencies; lines147–175 form/interleave
the position embeddings. Lines541–545 define `rotate_half(x)` as
`[-x_second_half, x_first_half]`; lines570–585 apply it only to the
rotary prefix. Lines659–665 split each512-element Q projection head into
Q256 and gate256, then apply Q/K norms. Lines668–672 apply RoPE and append
K/V. Lines710–723 define zero-centered RMSNorm with `(1 + weight)`.
Line690 multiplies attention output by **sigmoid(gate)**; this is not
`silu(gate)`. The latter belongs to the gated-delta norm at lines177–190.
The config's `output_gate_type: swish` must not override the full-attention
reference. The existing mathematical reference
`tests/gpu_decode_attn256_test.c:100–123` agrees on norm/rotation/append
and line19 explicitly places sigmoid outside the attention core.

This implementation's explicit ABI uses FP32 projection/norm values and
position embeddings, then rounds only K/V cache storage to BF16.
It follows the reference formulas, not a claim of PyTorch BF16-intermediate
bit identity. Norm weights are FP32 views of model weights, shared across
heads. No weight values, epsilon, or gate data are invented by the kernel.

`attn256_prep` has80-byte kernarg, alignment8:

| Byte offset | Argument | Layout/type |
| --- | --- | --- |
| 0 | Q_gate | FP32 `[24][512]`, Q then gate in each head |
| 8 | K_raw | FP32 `[4][256]` |
| 16 | V_raw | FP32 `[4][256]` |
| 24 | Q_weight | FP32 `[256]`, zero-centered |
| 32 | K_weight | FP32 `[256]`, zero-centered |
| 40 | cos_sin | FP32 cos32 followed by sin32, shared token position |
| 48 | Q_out | FP32 `[24][256]` |
| 56 | K_cache | BF16 `[4][capacity][256]` |
| 64 | V_cache | BF16 `[4][capacity][256]` |
| 72 | P | u32 append position |
| 76 | capacity | u32 allocated cache capacity |

Launch24 by1 workgroups of64 lanes. Head=`s2`, Y=`s3` must be0.
Overdispatched heads/Y and P>=capacity write nothing. This is the real
allocated-memory boundary, not an arbitrary token cap. After this launch,
the core's token count is `T=P+1`. Each wave handles one Q head; the first
four waves also process the corresponding K/V head. Inputs must not alias
outputs or the cache append slices. Historical cache positions, packed
gate halves, raw projections, norm weights and guard bytes remain unchanged.

Norm order: lane i visits dimensions4i+j, j0..3, with FP32 FMA of x*x
into+0. All64 lanes use snapshot XOR32,16,8,4,2,1 ADD reductions.
Multiply the sum by1/256, add epsilon (separate RN32 operations), then
V_RSQ_F32. Multiply x by this reciprocal norm, then by RN32(1+weight).
For each rotary pair d,d+32, d0..31, output is separately rounded
`x[d]*cos[d] + (-x[d+32])*sin[d]` and
`x[d+32]*cos[d] + x[d]*sin[d]`. This is **half pairing**, not adjacent
even/odd pairing. The DS_BPERMUTE address XOR is32 bytes (lane XOR8),
with all16 participating lanes active. Dimensions64..255 are unchanged
after norm. No FMA contraction is substituted across the two rotation
products/addition.

K and V use integer RN-even BF16 conversion: add0x7fff and retained
low-bit parity, then shift16. Infinities and signed zero survive;
any NaN becomes quiet0x7fc0, preventing signaling/low-payload NaNs from
turning into infinity. Two BF16 values are packed into each existing
GLOBAL_STORE_DWORD, so no new store opcode or misaligned write is needed.

`attn256_gate` has24-byte kernarg: Q_gate pointer0, core attention FP32
`[24][256]` pointer8, output of that shape pointer16. Launch96 by1 groups
of64; overdispatch is guarded. It reads the packed gate half at
`head*512+256+dimension`. It computes t=exp2(RN32(-abs(g)*LOG2E32)),
r=rcp(RN32(1+t)), then sigma=r for nonnegative g and sigma=RN32(t*r)
for negative g. Output is RN32(attention*sigma), not multiplied by g.
This stable form avoids positive exponent overflow. The gate is a separate
tiny kernel before o_proj; the existing GEMV2 objects remain unchanged.
The gate declares32 VGPRs (highest referenced v30), not the prep kernel's48.

### Seeded host position-embedding producer

`backend/amd/gfx942/rope.om` accepts exactly one little-endian u32 P
on stdin and emits256 bytes: cos32 then sin32 as little-endian FP32.
Missing/trailing input refuses64 with no output; I/O failure is74.
It is compiled with the same seeded pin, uses only Omega native arithmetic
and syscalls, and has no Python/libm/runtime dependency.

Inverse frequency is RN32(1 / RN32(theta^(i/32))), i0..31; position
is converted to FP32 and multiplied at a FP32 boundary. Power uses
binary64 range reduction and18 exponential terms. The test-only
`ropefreq-probe.om` compares all32 frequencies bit-for-bit with independent
Decimal120-digit power followed by the two required FP32 roundings.
For sin/cos the producer reduces the FP32 angle with a split pi/2 in
binary64, computes21st/20th-degree sine/cosine series, selects the quadrant,
and rounds once to FP32. The64 generated values are shared across every
attention layer for this token; no head recomputes them on the GPU.

Recorded calculations:

* Wolfram `N[Log[10000000],22]` ->16.11809565095831978812594;
  `N[2/Pi,22]` ->0.6366197723675813430755351.
* Wolfram `N[Pi/2-157079601287841796875/10^20,30]`
  ->3.13916478650481321691639751442e-7.
  Python byte encoding, `struct.unpack("<Q",struct.pack("<d",
  3.139164786504813216916397514420985846996875529075603e-7))[0]`
  ->0x3e95110b4611a626, used as a binary64 bitcast to avoid the pinned
  compiler's long fractional-literal parser limit.
* Wolfram `N[Pi,125]` supplied the independent oracle's recorded pi
  constant; it is not read from the producer.
* Wolfram `{N[(786/1000)^23/23!,24],N[(786/1000)^22/22!,24],
  N[2^-42+2^-53+40*2^-52,24],N[2^-39,24]}`
  ->{1.52133245558152738690949e-25,4.45173619317749744261046e-24,
  2.36366481942695827456191e-13,1.81898940354585647583008e-12}.

MODELED binary64 allowance, not a formal Lean proof: the21-significant-bit
high pi/2 part times the at-most32-bit quadrant index is exact in binary64;
its first subtraction is exact. The low-part representation/product
errors contribute at most2^-42 over u32 positions. The residual stays
within0.786 despite quotient rounding; series truncation is bounded by
the recorded terms, with conservative binary64 roundoff accounting
2^-53+40*2^-52. The gate's absolute check allows2^-39 plus half a final
FP32 ULP against independent Decimal120 sine/cosine, including P=2^32-1.
This allowance is only for generated embeddings; the GPU applies their
actual bits and gets no added arbitrary output tolerance.

### New opcode audit and gates

| Instruction | CDNA3 physical PDF page | Semantics and side effects | LLVM18.1.3 source |
| --- | --- | --- | --- |
| V_RSQ_F32 | 197 | Reciprocal square root,1ULP, input/output FTZ; -0 -> -inf, +0 -> +inf,4 ->0.5,+inf ->+0,-inf -> negative quiet NaN. Active lanes only; EXEC/VCC/SCC and inactive registers unchanged. | [VOP1Instructions.td][attn-vop1], opcode36 |
| Floating inline selectors240..248 | 42, Table23 | FP32 bits for ±0.5, ±1, ±2, ±4 and the manual's exact0x3e22f983 for1/(2*pi). Scalar/vector MOV preserve flags; inactive vector lanes remain untouched. | Operand encodings, not new opcodes |
| S_NOP | 138 | Issue delay1..16 clocks from low4 immediate bits; no register, flag or memory changes. The functional executor advances PC only and does not model elapsed cycles. | SOPP opcode0 |

The executor uses native binary64 sqrt/divide followed by FP32/FTZ as
a bounded representative, not the actual MI300X approximation. Manual
single-step cases include both end lanes, inactive-lane sentinels, signed
zeros, subnormals, negative input, infinities and4. Replacing RSQ by
RCP must be CAUGHT. The bulk probe checks4096 independent positive normal
inputs against Decimal120 and the manual1ULP enclosure. S_NOP is the only
other new opcode; earlier audit tables and refusal boundaries apply.
Every added inline selector has scalar and vector single-step cases, plus
V_ADD_F32 with inline1.0. Restoring the old refusal must be CAUGHT.
LLVM selects inline1.0 instead of a literal DWORD; the production tables
use selector242 so their bytes, lengths and branch displacements match.

CDNA3 pp28–29, Table11: “The hardware does not check for the following
dependencies; they must be resolved by inserting NOPs or independent
instructions.” A VALU transcendental followed by a consuming non-trans
VALU needs one wait state. The merge's RCP and gate's EXP therefore have
`s_nop 0`; other EXP/RSQ results already have independent intervening
instructions. `tests/amd/scheduling.py` checks LLVM-decoded bytes from
every emitted object for this rule, including all earlier GEMV versions.
It emits real mutant objects with those NOPs removed and requires
both scheduling failures. This is a static check for this specific hazard,
not a cycle model or a claim that all other hardware hazards are covered.
S_NOP single-step cases check low/high immediate bits and unchanged flags;
a mutant clearing SCC must be CAUGHT.

The prep oracle propagates RSQ intervals through every norm and rotation
operation, checks every Q output and every appended K/V element, and
requires all input/history/guard bytes unchanged. BF16 V conversion is
bit-for-bit including denormals, ties, signed zero, infinities and NaNs.
For K it reports any BF16 result made ambiguous by the allowed RSQ interval
rather than hiding it. Exact per-Q-output intervals/widths are filed as
JSON; each case prints its widest interval. Controls omit the norm's1,
use the wrong half-pair, write the wrong cache position, truncate BF16,
multiply the sigmoid result by g, and use the wrong packed-head stride.
Host-producer controls corrupt rotary dimension and malformed input.
All earlier gate counts and controls remain. Both new code objects and
the producer/executor get two byte-identical builds, with LLVM instruction
and complete descriptor/note parity as before.

**Hardware NOT_RUN:** preparation/gate objects, RSQ/EXP/RCP choices,
generated-position end-to-end model quality and timing require the
parent's MI300X bundle. This does not claim complete serving.

[prep-reference]: https://github.com/huggingface/transformers/blob/v5.8.0/src/transformers/models/qwen3_5/modeling_qwen3_5.py

## 18. Token MLP activation and mandatory scheduling check

The first MI300X attention-core exercise is filed separately in
`evidence/AMD-GFX942-ATTENTION-DEVICE-20261113T000755Z/`. The corrected
RCP-consumer delay is exercised on device; the earlier raw diagnostic and
its BF16 reference-input defect remain visible. This is not qualification
of the complete token path. The Table11 static scheduling gate stays
mandatory for **every** future object; functional executor parity alone
does not test issue timing.

`mlpsilu` implements [Transformers v5.8.0 Qwen3_5MLP lines695–707][prep-reference]:
`down_proj(silu(gate_proj(x)) * up_proj(x))`. The pinned config revision
017b9c7a has hidden5120, intermediate17408 and SiLU activation. This
kernel consumes the FP32 gate/up outputs of the existing GEMV2 kernels,
and produces the FP32 vector consumed by GEMV2 down. It is not the
attention-output sigmoid-only gate.

ABI: gate pointer0, up pointer8, output pointer16, N:u32 at24;28bytes.
Launch ceil(N/64) workgroups in X, one in Y,64threads. N=0 and extra X/Y
groups write nothing; the final partial wave masks its inactive lanes.
Inputs and output must not overlap. Each active lane computes:

1. t = EXP2(RN32(-abs(g) * LOG2E32)), LOG2E32=0x3fb8aa3b.
2. r = RCP(RN32(1+t)); if g has its sign bit set, r = RN32(r*t).
3. s = RN32(g*r); output = RN32(s*up).

EXP/RCP use the same CDNA3 pp195–1961ULP/FTZ contract as §16/17, not an
assumption of device-identical transcendental bits. The reference uses
independent Fraction round-to-nearest-even arithmetic and Decimal120
transcendental enclosures at those exact instruction boundaries.
The finite-input contract includes signed zeros, subnormals and large
finite magnitudes. The explicit EXP-consumer NOP and independent
instructions after RCP satisfy Table11. No instruction is newly added to
the executor: the manual-derived instruction cases in earlier sections
remain applicable, including carry, EXEC restoration and SCC.

The gate retains every earlier case and adds counts0,1,63,64,65,17408,
checks every output interval and every untouched input/guard byte, and
requires actual wrong-object controls omitting the SiLU input multiply
and reading gate instead of up to be CAUGHT. Host and LLVM gates build
twice and compare bytes; the latter checks the full descriptor and
instruction encodings and runs the scheduling check over all objects.

`tests/amd-exec/token.py` and `section.py` generate code-only notebook
sections from the actual fixture functions and their arithmetic dependency
closure. Inputs and all exact results/intervals are computed in the cell:
no full memory image or interval list is embedded. Generation executes
that same code and checks every reconstructed case against the host
oracle, with a missing-case drift control. An emitted-source byte-size
alarm checks the operator's target below15KB per section, to fit the
external relay's approximately60KB cell limit. Other kernels' torch
branches are removed by AST specialization, not by a second handwritten
implementation. The generated section's `run_section(ctx)` tests small
cases, measures event/warm-wall/cold-wall timings at17408, checks every
timed output and guard (all cold copies, not sampled), records wrong-object
controls, and labels the separate torch measurement
“(AMD library, not Omega)”. Cold inputs occupy distinct addresses exceeding
the notebook's COLD_BYTES. No notebook timing is inferred from host runs.
Each exported source is executed in the loader's fresh empty namespace;
all generated/decorated/nested function code is also checked for unresolved
global lookups before export. A control removes the math import and must
be CAUGHT in the deferred interval checker. Factory execution alone does
not exercise that checker; hardware notebook execution stays NOT_RUN.

## 19. Standalone zero-centered RMSNorm5120

`rmsnorm5120` follows [Transformers v5.8.0 lines710–723][prep-reference]:
FP32 x*rsqrt(mean(x²)+epsilon), then multiply by (1+weight). This is the
input, post-attention and final norm, not the gated delta net's norm
(whose weight is not zero-centered). The pinned config in §17 supplies
hidden5120 and epsilon1e-6. All input, weight and output elements are FP32;
the decoder's BF16 weight can be widened exactly before this interface.

ABI: input pointer0, weight pointer8, output pointer16, rows:u32 at24.
Input/output layout [rows][5120], one shared weight[5120]. The buffers
must not overlap. Launch X=rows, Y=1, block64; zero rows and additional
X/Y groups do not write. No arbitrary row cap is introduced.

The instruction order is specified: each lane accumulates80 squares
using RN-even FMA in ascending chunk/j order, at index256*chunk+4*lane+j
(chunk0..19, j0..3). All lanes reduce by snapshot XOR32,16,8,4,2,1 using
RN-even ADD. Multiply by RN32(1/5120), add RN32(epsilon), RSQ, then each
output uses separately rounded x*inv and result*RN32(1+weight).
RSQ has the manual pp1971ULP/FTZ enclosure; no guessed device bit pattern.
The kernel reloads inputs after reduction rather than keeping all rows in
registers. The host executor's already-audited dwordx4, bpermute, carry
and loop semantics are reused; no new opcode or gate exception is added.
The RSQ-consumer delay is explicit and the all-object scheduling gate
remains mandatory.

Recorded independent calculation:
`tests/amd-exec/sgemm.py round32(Fraction(1,5120)) -> 0x394ccccd`;
`tests/amd-exec/norm.py` asserts it at every invocation. The Fraction
oracle reproduces the declared reduction order, propagates the Decimal120
RSQ enclosure through each later rounding, and reports every output's
interval plus the widest observed interval. Cases include zero rows,
zero vectors, several rows, subnormal inputs and large finite magnitudes.
Wrong-object controls omit the zero-centered1, use the wrong mean divisor,
and omit the last accumulation chunk. Every original case is retained.

The generated `rmsnorm5120.py` notebook section uses the same instrument
as §18: all outputs/guards, genuine wrong-object controls, event and
warm/cold wall timing with every timed copy checked, and a separate torch
line labeled “(AMD library, not Omega)”. Kernel and notebook hardware
execution remain NOT_RUN until the parent runs that section on MI300X.

## 20. Gated delta net BF16 causal convolution

`gdnconv4` implements the single-token cached convolution in
[Transformers v5.8.0 lines211–225 and455–463][prep-reference].
The pinned config revision017b9c7a (§17) has kernel4, key heads16,
value heads48, head dimensions128 and BF16 model weights. The channel
layout is Q[16][128], K[16][128], V[48][128], totaling10240.

The dtype boundaries follow the reference, not a new FP32-only variant:
line220 concatenates the existing cache and projected token and converts
to the weight dtype; line221 copies the last state_len entries back;
line222 runs the grouped BF16 convolution, whose result is BF16;
line223 applies SiLU to that BF16 result, producing BF16; line224 converts
back to the incoming hidden-state dtype. This interface takes the incoming
projection as FP32, explicitly rounds it to BF16 for the append, uses BF16
weights/cache and FP32 accumulating FMA, rounds the convolution result to
BF16 before SiLU, and rounds SiLU to BF16 before widening its output to
FP32. Subsequent Q/K normalization therefore receives those widened values.

ABI (40 bytes): input pointer0, weight pointer8, old_cache pointer16,
new_cache pointer24, output pointer32. Input/output are [10240] FP32;
weight/old_cache/new_cache are [10240][4] BF16, tap-major inside each
channel. All buffers are disjoint. The new state is old[1],old[2],old[3],
RN_BF16(input). Old state is immutable; the caller swaps old/new pointers
after the launch. Grid X=160, Y=1, block64; additional X/Y groups return.

For each channel, start the accumulator at +0 and FMA the four new-cache
samples and weights in ascending tap order. BF16 conversion is RN-even
with signed-zero preservation, infinity preservation and canonical NaN
0x7fc0. SiLU uses the stable sigmoid order in §18 on the widened BF16
convolution result. The manual's EXP/RCP error enclosures are propagated
through FP32 operations and the final BF16 rounding. Every output must
also lie on the BF16 bit lattice; interval membership alone is insufficient.
Every new-cache halfword is checked exactly. Existing audited instructions
are reused, and the all-object scheduling gate is mandatory.

`tests/amd-exec/gdnconv.py` independently constructs the updated cache,
performs Fraction FMA and BF16 rounding at each stated boundary, and reports
every output interval and its maximum width. Its cases include zero cache,
different histories, BF16 ties, subnormals, large values, infinities/NaNs,
partial dispatch and full10240-channel overdispatch. Separate controls
remove each of the three BF16 conversions or shift the cache incorrectly.
The generated notebook section checks all state/output/guard bytes,
every timed cold copy, genuine wrong objects, and a separately labeled
BF16 torch grouped-convolution reference. Hardware remains NOT_RUN.
