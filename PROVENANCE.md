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
