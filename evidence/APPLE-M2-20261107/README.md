# APPLE-M2-20261107: Apple M2 CPU and GPU through the portable-backend contract

Card apple195 (kanban t_e1d7686d), Linear DRA-195 under DRA-192, GitHub CUDApp#42. The shared
contract is ADR 0043 (aien-architecture-mv, Accepted 2026-10-30) as implemented by this repository's
host gate `tests/multitarget/run.sh`: kernels emitted by seeded Omega (`backend/spirv/emit.om`), the
Omega loader checks (`backend/targets.om`) and the bit-exact Omega reference (`backend/spirv/conform.om`).
Runs: 2026-11-07, 00:24Z to 01:12Z. Everything below was executed; anything that did not run says NOT_RUN.

## Machine (the physical runner)

| Item | Value (source) |
|---|---|
| Machine | MacBook Air, model identifier Mac14,2 (`sysctl hw.model`, `system_profiler`) |
| Chip | Apple M2: 8 CPU cores (4 performance, 4 efficiency), 10-core GPU, vendor 0x106b, Metal 4; 8 GiB unified memory |
| OS | macOS 26.6.2, build 25G83, Darwin 25.6.0 arm64; system firmware 18000.161.10 |
| Toolchain | Command Line Tools only (no Xcode; `xcrun -f metal` not found, so no offline Metal compiler): Apple clang 17.0.0 (clang-1700.6.4.2), macOS SDK 26.2, Swift 6.2.4 (unused) |
| GPU runtime | MoltenVK 1.4.2 (Vulkan 1.4.357, conformance 1.4.4.0) over Metal. `libMoltenVK.dylib` sha256 `aef00b13…4c5f` is the file inside the upstream release asset `MoltenVK-macos.tar` sha256 `f95765a6…024e`, equal to the digest GitHub publishes for KhronosGroup/MoltenVK v1.4.2 (`tools.txt`) |
| Metal profile (MoltenVK log) | MSL 4.0; GPU families Metal 4, Apple 8, Mac 1, Mac 2; 5461 MB GPU memory available; max invocations per group 1024; 32768 B shared; semaphores on MTLEvent; Metal 3 argument buffers |
| CPU guest | Lima 2.2.0 VM `omega-llm` (Apple Virtualization.framework, `vz`), 2 vCPU, 2 GiB; Debian 13, Linux 6.12.95+deb13-cloud-arm64; MIDR_EL1 0x610f0000 (implementer 0x61 = Apple). Used read-only: files only in guest `/tmp` (tmpfs), removed after |
| Producer and oracle host | spark-b87b (DGX Spark, Linux 7.0.0-1019-nvidia aarch64), CPU only: Mesa lavapipe 25.2.8, SPIRV-Tools v2025.4. No GPU window was opened |
| Compiler | seeded omc sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f` (omega-mv `369dbc96`, the pin in `run.sh`) |

## Results

Labels: EXERCISED = ran on the named hardware. Every bit-exact claim compares all output bytes with the
Omega reference; `conform check` (Omega) was also run on the returned Apple GPU outputs.

| Row | Device / route | Result | Receipt |
|---|---|---|---|
| Capability probe | M2 GPU via MoltenVK | EXERCISED: one physical device, integrated GPU, vendor 0x106b, device id 0x1a060208, driverID MOLTENVK, `VK_KHR_portability_subset` enabled; wrong expected device refused (exit 5) | `m2-gpu/apple-gpu.log`, `run-vadd.vk.log` |
| Allocation, transfer, queue, wait | M2 GPU | EXERCISED inside every dispatch: host-visible coherent buffers (shared storage), one command buffer, `vkQueueSubmit` + `vkQueueWaitIdle`. No separate timing or event test | `run-*.log` |
| vadd, 1048576 FP32 | M2 GPU | EXERCISED, bit-exact, sha256 `72d0c1b2…b6a6` (= `hello_gpu --sim` digest per run.sh); repeat identical; mutant (OpFSub) wrong=1048576 | `apple-gpu.log`, `conform-on-spark.txt` |
| fsum16 reduction, 65536 rows x 16 | M2 GPU | EXERCISED, bit-exact `6a6de937…6290`; repeat identical; mutant (right-to-left) wrong=44123 | same |
| gemm16 GEMM, 256 x (16x16x16) FP32 | M2 GPU | EXERCISED, bit-exact `a006702d…6195`; repeat identical; mutant (k reversed) wrong=47715 | same |
| Same three kernels | M2 CPU (Linux guest) | EXERCISED: `run.sh` PASS 41, FAIL 0, NOT_RUN 3. The three Omega units compile to the same binaries as on the Spark, all 12 modules and the 3 references are byte-identical to the Spark run | `m2-cpu-guest/run.log` |
| SPIR-V on a CPU driver | M2 CPU | NOT_RUN: the guest has no lavapipe, PoCL or C compiler (not installed, to leave Drake's VM unchanged) | `m2-cpu-guest/run.log` |
| Omega native code on macOS itself | M2 CPU, macOS | NOT_RUN: omc emits static Linux ELF only; Mach-O output and checked shared-library calls are ADR 0043 prerequisite E2 (omega-mv#888) | — |
| Model forward pass | M2 CPU (guest), own Omega LLM `ollm` (aien-train `0518c915`, binary `819664d5…`) | EXERCISED: `eval` (E 20 16 5512452 7952788) and an argmax `sample` identical to the Spark | `model/m2-job8.out`, `m2-extra.out` |
| Training step and checkpoint round trip | M2 CPU (guest) | EXERCISED: `init`, `gcheck`, `train` 20 steps (T 10 5554254, T 20 5527414); TR.BIN, HE.BIN and S.CK sha256 equal `evidence/OMEGALLM/efi/ref-files.txt` in aien-train; resume 0->10 (new process) ->20 gives the same checkpoint as 0->20; flipped checkpoint byte refused (rc 14); mutant b build changes eval | `model/` |
| mathck, adamck, prep, base | M2 CPU (guest) | EXERCISED: output identical to the committed AArch64 reference (aien-train `evidence/OMEGALLM/efi/ref.out` lines 1..94) | `model/m2-job8.out` |
| Model on the GPU | M2 GPU | NOT_RUN: the own model's kernels exist only for the CPU and the GB10; nothing lowers them to SPIRV_VK or MSL_SOURCE | — |
| `MSL_SOURCE` backend (the contract's Apple code object) | M2 GPU, Metal direct | NOT_RUN: no `.om` MSL emitter and no Metal host exist; `targets load MSL_SOURCE` refuses (exit 11). Needs E2 or a Drake-authorized interim Metal launcher | `spark/run.log` load-neg-msl |
| Apple Neural Engine | ANE | NOT_RUN, not investigated. GPU results say nothing about the ANE | — |
| Same-device native comparison (DRA-191) | M2 (Accelerate / MPS) | NOT_RUN: needs a native-stack harness (foreign code, not authorized here) | — |
| Performance | any | NOT_RUN: no timing was measured; the MoltenVK route is not a performance claim | — |

## Frozen precision and the controls

- Precision: binary32, round to nearest even, no fused multiply-add, frozen operation order. The launcher
  removes inherited `MVK_*` variables and sets `MVK_CONFIG_FAST_MATH_ENABLED=0` (MoltenVK's default is
  2). gemm16 marks every product and sum `NoContraction` (vk) or the entry `ContractionOff` (cl).
- Oracle independence: WolframLanguageEvaluator recomputed gemm16 batch 0 (256 outputs) from the input
  formulas with exact rationals and binary32 rounding; the SHA-256 of its 256 bit patterns
  (`da2f9d74…a27a`) equals that of the first 256 words of the Omega reference. The same computation
  with fused multiply-adds changes 105 of the 256 outputs, so the test detects contraction
  (`wolfram-gemm16.txt`).
- Observation, not a pass condition: with fast math forced on (`MVK_CONFIG_FAST_MATH_ENABLED=1`) the
  three kernels were also bit-exact on this M2. The conformance mode stays fast math off.
- Every planted-wrong mutant gave exactly the same output bytes on the M2 GPU as on lavapipe on the Spark.
- Refusals with no output written: `cl` on the MoltenVK build (2), a build forbidding the MoltenVK image
  it loads (isolation, 9), wrong expected device (5), missing module and a module that is not whole words
  (4), short input (4).
- Finding (failure cleanup): given a missing entry point, MoltenVK 1.4.2 aborts the process
  (`MVK_spirv_cross::CompilerError: Entry point does not exist`, SIGABRT, exit 134) instead of returning a
  Vulkan error. No output is written, but the process dies; the contract's gate for this case is the Omega
  loader (`targets load SPIRV_VK FILE ENTRY` exits 23 before any run).

## Commands

```sh
# Spark (producer and oracle; CPU pool lock, no GPU):
SPIRV_VAL=… CUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 MT_CFLAGS=-I<headers> MT_OUT=<dir> \
  sh tests/multitarget/run.sh <abs>/omc                                   # spark/run.log, exit 0
# Mac, Apple GPU (nice 10), DIR = the run.sh outputs listed in apple-gpu.sh with SHA256SUMS:
MVK_LIB=<MoltenVK>/dylib/macOS/libMoltenVK.dylib MVK_INCLUDE=<MoltenVK>/include MT_CC=clang \
  sh tests/multitarget/apple-gpu.sh DIR                                   # m2-gpu/apple-gpu.log, exit 0
conform check vadd|fsum16|gemm16 OUT                                      # m2-gpu/conform-on-spark.txt
# Mac, Apple CPU through the guest: m2-cpu-guest/guest-cpu.sh (run.sh with the same omc, then model-job.sh)
# Model job on each host: model/model-job.sh DIR CORE (DIR: ollm, ollm-mut-b, efi-ref.sh, efi-job.txt,
#   mathref.txt, STORIES.TXT = TinyStoriesV2-GPT4-valid.txt sha256 6874bae9…585d; model/inputs.sha256)
```

Exact output digests: `outputs.sha256`.
