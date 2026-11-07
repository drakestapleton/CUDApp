# PORTABLE-APP-20261107: one unchanged application on an NVIDIA GPU (GB10) and a non-NVIDIA GPU (Apple M2)

Card cuda201 (kanban t_ebf8bda5), Linear DRA-201 (parent DRA-192). Builds on apple195 (DRA-195,
`evidence/APPLE-M2-20261107/`). Criteria were frozen in `PREREG.md`, committed and pushed as `61b9a1ac`
before either GPU run below. Runs: 2026-11-07, 01:28Z to 01:32Z. Everything in the results table was
executed on the named hardware; anything that did not run says NOT_RUN.

## Outcome

The same Omega application source, the same SPIR-V code objects and the same inputs ran on the NVIDIA GB10
and on the Apple M2 GPU. Every output matched the Omega reference bit for bit (0 ULP) on both GPUs, and the
two GPUs' output files are byte-identical to each other, real kernels and planted-wrong mutants alike. The
GB10 also ran CUDA++'s own sm_121 vadd (no CUDA, no NVIDIA user-space library), which gives the same output
digest. Scope: the three-kernel contract workload only (vadd, fsum16, gemm16). The model, arbitrary CUDA
programs and an AIEN-owned execution path for SPIR-V are not covered (see "What this does not show").

## Application, identity and the one shared artifact set

| Item | Identity |
|---|---|
| Application source (unchanged by this card) | `backend/spirv/emit.om` sha256 `9b622253…a0db4`, `backend/spirv/conform.om` `557061f0…a3b1`, `backend/targets.om` `8e5349ec…9994` |
| Compiler | seeded omc `2c824805…029f` (omega-mv `369dbc96`) |
| Code objects (built once on the Spark, copied to both GPUs, `SHA256SUMS` checked on each side) | `vadd.vk.spv b56654c9…0444`, `fsum16.vk.spv 1c139c14…5805`, `gemm16.vk.spv 191e4f42…c9e`; mutants `10284242…`, `951a53c3…`, `fabf98eb…` |
| Inputs | `a.bin 6f5b3e0f…a2c1`, `b.bin 6b2796ec…b316`, `x.bin 2c9a66be…de01`, `ga.bin fa4636b8…6c3c`, `gb.bin e79a069f…dcee` |
| References (Omega `conform ref`) | `vadd 72d0c1b2…b6a6`, `fsum16 6a6de937…6290`, `gemm16 a006702d…6195` (also equal to `hello_gpu --sim`, `spark-host/run.log`) |
| Launcher source | `tools/multitarget/spirv-host.c` sha256 `b66d63ec…7dc8` on both machines (one file, `MT_TARGET` selects the build) |
| Model / weights | none in this workload; no repacking |

These are byte-identical to apple195's run on the same branch line (`evidence/APPLE-M2-20261107/outputs.sha256`).
Build and configuration differences are exactly the table in `PREREG.md` ("Build/configuration differences").

## Results

| Row | Device / route | Result | Receipt |
|---|---|---|---|
| Host gate (producer) | Spark CPU, lavapipe | `MULTITARGET PASS pass=52 fail=0 not_run=2` (hello_gpu `--sim` cross-check now runs: PASS) | `spark-host/run.log` |
| vadd, fsum16, gemm16 | **GB10**, NVIDIA Vulkan driver 580.173.02 | EXERCISED: bit-exact, `conform check` wrong=0; repeat identical; mutants wrong=1048576 / 44123 / 47715 | `gb10-gpu/nvidia-gpu.log` |
| same three | **Apple M2 GPU**, MoltenVK 1.4.2 over Metal | EXERCISED: bit-exact, repeat identical, mutants differ (apple-gpu.sh) | `m2-gpu/apple-gpu.log` |
| Cross-target | GB10 vs M2 output files | EXERCISED: all 6 files (3 real, 3 mutant) byte-identical | `gb10-gpu/nvidia-gpu.log` (cross-target lines) |
| 10 timing repetitions per kernel | both GPUs | 30/30 outputs = reference on each GPU | `*/timing.tsv` |
| Native baseline, same device | **GB10**, CUDA++ own sm_121 `hello_gpu` (own encoder, own launcher, nvidia.ko ioctls; no libcuda) | EXERCISED: 10/10 PASS, output sha256 = `vadd.ref` `72d0c1b2…b6a6`; machine code `9e28bd9c…f075`; `hello_gpu_control --bug=short` red (exit 1, 1 never written) | `gb10-gpu/hello_gpu-*.txt`, `hello_gpu_control.log` |
| Native baseline, same device | Apple M2: Metal / MPS | NOT_RUN: needs a foreign (Swift/ObjC/Metal) harness, not authorized | — |
| Native baseline, same device | GB10: CUDA / cuBLAS | NOT_RUN: foreign vendor stack, not used by AIEN | — |
| Overall gates | | `NVIDIA-GPU PASS pass=46 fail=0 not_run=0`; `APPLE-GPU PASS pass=20 fail=0` | logs |

GB10 window: `20261107T013053Z-901cc9`, `--gb10 exclusive --quiet exclusive`, open 01:31:02Z, close
01:31:41Z, verdict "quiet machine". A resident service (`omega-tool-model round40-fast server`, 335 MiB of
GPU memory, 0 % utilisation at open and close) stayed loaded; it was not touched (`window-*.json`).

## Contract, supported subset, capability queries, unsupported operations

- Versioned contract: ADR 0043 portable-backend contract as implemented by `tests/multitarget/run.sh` at
  this branch. Code object format SPIRV_VK = SPIR-V 1.3, Vulkan 1.1 environment, `Shader` capability,
  `GLCompute` entry, 64 invocations per workgroup, storage buffers only (Omega loader:
  `LOAD ACCEPT SPIRV_VK version=1.3`). Precision: binary32, round to nearest even, no FMA contraction
  (`NoContraction` on every gemm16 product and sum), frozen operation order.
- Supported subset (demonstrated): elementwise add, 16-wide left-to-right reduction, 16x16x16 batched GEMM,
  FP32 only. Nothing else is claimed.
- Capability queries (both GPUs, printed by every run): device name, type, vendor, device id, driver id and
  version, Vulkan conformance version, max workgroup count/invocations/shared memory, timestamp period,
  queue timestamp bits; Apple also `VK_KHR_portability_subset`.
  - GB10: `NVIDIA GB10`, integrated, vendor 0x10de, device 0x2e12, driver NVIDIA 580.173.02, Vulkan 1.4.312,
    conformance 1.4.1.3, max invocations 1024, shared 49152 B, timestamp period 1 ns.
  - M2: `Apple M2`, integrated, vendor 0x106b, device 0x1a060208, driver MoltenVK 1.4.2, Vulkan 1.2.357 (as
    requested), conformance 1.4.4.0, max invocations 1024, shared 32768 B, timestamp period 1 ns.
- Unsupported operations, explicit behaviour: the Omega loader refuses MSL_SOURCE / AARCH64 (11, PLANNED),
  CUBIN (12, reserved), PTX (10, unknown), wrong environment (22) and a missing entry (23) before any run
  (`spark-host/run.log`). Given a missing entry anyway, the GB10 driver refuses cleanly
  (`vkCreateComputePipelines` error, exit 7, no output); MoltenVK aborts the process (SIGABRT, 134, no
  output), as apple195 found. Both fail closed.
- No hidden CPU or remote fallback (GB10): a manifest selecting lavapipe is refused before any run (exit 5,
  "device is not a GPU"); a wrong device name is refused (5); the isolation scan finds the NVIDIA driver core
  mapped and no lavapipe, PoCL or libcuda mapping; told to forbid the driver core it fails (9); told to
  require an absent library it fails (9). The M2 controls are apple195's (MoltenVK forbid 9, wrong device 5,
  cl refused 2). The work is local on both machines: the inputs are files, the launcher has no network code.

## Cache and lifecycle (GB10)

- Conformance mode keeps NVIDIA's on-disk shader cache off; a cache-off build pointed at a private path
  wrote nothing there, and the driver's default cache directory was unchanged across the whole script.
- A cache-on build wrote 2 files to its private path. Pipeline creation (driver compile), cache off / cold /
  warm, in µs: vadd 10405 / 15888 / 108, fsum16 15542 / 20108 / 109, gemm16 22667 / 26996 / 123 (one run
  each, MEASURED, observation). Warm outputs equal the reference.
- Every cache file corrupted mid-file: all three kernels still produced the reference bytes (the driver
  did not use a corrupted entry to produce wrong output).
- Every run creates and destroys its own instance, device, buffers and pipeline; 10 fresh processes per
  kernel per GPU all returned the reference bytes.

## Memory and end-to-end latency (MEASURED, 10 fresh processes per kernel; median [min, max])

Columns from `spirv-host: time …`: setup = instance, device, buffers, upload; pipeline = shader module +
compute pipeline; run = record + submit + wait; device = timestamp-query span around the dispatch; total =
process start to output ready. Statistics computed with Wolfram (`Median`, `Min`, `Max`).

| Target | Kernel | device | run | pipeline | setup | total | peak RSS | buffers |
|---|---|---|---|---|---|---|---|---|
| GB10 NVIDIA vk | vadd | 43.3 µs [41.5, 54.0] | 1186 µs [1163, 1394] | 10.5 ms [10.2, 11.3] | 325 ms [322, 973] | 341 ms [338, 989] | 195 MiB | 12 MiB |
| GB10 NVIDIA vk | fsum16 | 37.6 µs [33.0, 52.2] | 1192 µs [1152, 1287] | 15.7 ms [15.5, 16.0] | 324 ms [321, 347] | 343 ms [339, 365] | 180 MiB | 4.25 MiB |
| GB10 NVIDIA vk | gemm16 | 8.75 µs [8.26, 10.8] | 1202 µs [1125, 1231] | 22.8 ms [22.6, 23.1] | 328 ms [323, 348] | 352 ms [347, 372] | 174 MiB | 0.75 MiB |
| M2 MoltenVK | vadd | 382 µs [353, 429] | 1298 µs [1204, 1490] | 0.81 ms [0.76, 0.97] | 10.6 ms [9.9, 18.1] | 14.3 ms [13.6, 21.7] | 43 MiB | 12 MiB |
| M2 MoltenVK | fsum16 | 394 µs [342, 425] | 1153 µs [1114, 1194] | 0.93 ms [0.88, 0.97] | 9.25 ms [9.03, 9.94] | 11.9 ms [11.6, 12.6] | 27 MiB | 4.25 MiB |
| M2 MoltenVK | gemm16 | 833 µs [796, 878] | 1576 µs [1542, 1622] | 1.08 ms [1.03, 1.14] | 8.73 ms [8.56, 9.31] | 11.5 ms [11.3, 12.2] | 21 MiB | 0.75 MiB |
| GB10 CUDA++ native (hello_gpu) | vadd | host submit-to-completion, 8 launches summed: 854 µs [821, 859] | — | kernel build 60 µs (no driver compile) | — | — | — | 12 MiB |

How to read these (do not compare across rows without the caveats):

- Setup dominates the GB10 Vulkan route: 95 % of the median total for vadd (325 of 341 ms), mostly NVIDIA
  driver/instance start-up per process. On the M2 it is 74 % (10.6 of 14.3 ms).
- The GB10 vadd device span implies 12582912 bytes / 43.3 µs = 290 GB/s, above the GB10's DRAM bandwidth
  (NVIDIA publishes 273 GB/s; CUDApp's measured read ceiling is 195.1 GB/s). So that span does not bracket
  all memory traffic or the inputs were partly cache-resident after the host upload [INFERENCE]; it is not a
  bandwidth claim.
- Timestamp semantics differ by driver. MoltenVK maps Vulkan timestamps onto Metal counter sampling, which
  on Apple GPUs is coarser than a per-dispatch span [INFERENCE: not measured separately]; the M2 "device"
  column must not be read as kernel time.
- Same-device comparison on the GB10 (CUDA++ native versus the NVIDIA Vulkan route, same vadd semantics and
  same output bytes): hello_gpu's number is host-observed submit-to-completion summed over 8 launches of
  131072 elements (128 threads per block), including per-launch submission, L2 flush and marker waits; the
  closest Vulkan figure is "run" (one dispatch, record+submit+wait), 1186 µs, against 854 µs for hello_gpu.
  The launch shapes and what each clock brackets differ, so this is an observation, not a speedup claim.
- GB10 versus M2 numbers are cross-system cost observations (different vendors, memory systems, power
  and price; the Mac ran on battery as an unmanaged laptop). They say nothing about software speedup.

## Remaining dependencies (published)

| Target | Dependency | Owner / successor |
|---|---|---|
| GB10, portable route | NVIDIA proprietary Vulkan user space: `libGLX_nvidia.so.580.173.02` `42668cce…`, `libnvidia-glcore` `507a1811…`, `libnvidia-glvkspirv` (the SPIR-V compiler) `b6339617…`, `libnvidia-gpucomp` `d3cc3a9e…`; Khronos loader `libvulkan.so.1.4.328` `08297dd2…`; ICD manifest `7bdb6f27…` | vendor runtime; AIEN successor would be an SPIR-V (or Omega IR) to sm_121 lowering inside CUDA++ (not started) |
| GB10, both routes | `nvidia.ko` open kernel module 580.173.02 (`/proc/driver/nvidia/version`); GSP firmware `gsp_ga10x.bin` `32cb7cf3…`, `gsp_tu10x.bin` `6f3ccbd5…` | Aien's own driver (aien-gpu, omega-gpu-driver) is IMPLEMENTED only, hardware NOT_RUN; GSP firmware stays NVIDIA's |
| GB10, CUDA++ native | `nvidia.ko` + `nvidia-uvm.ko` ioctls only (no NVIDIA user-space library; `make zero-cuda-gate`) | CUDApp |
| M2 | MoltenVK 1.4.2 `libMoltenVK.dylib` `aef00b13…4c5f` (release tar `f95765a6…024e`), Apple Metal framework and GPU driver, macOS 26.6.2 (25G83), system firmware 18000.161.10 | ADR 0043: `.om` MSL_SOURCE emitter + Metal host via E2 (omega-mv#888) |
| Both | interim C launcher `spirv-host.c` (migration-pending, default OFF) | `backend/vulkan/host.om` over E2 checked shared-library calls |

Compatibility distinctions: **source** compatibility is shown for the `.om` application (one source, two
GPU vendors); **binary (code-object)** compatibility is shown for the SPIR-V modules (identical bytes
accepted by both vendor drivers); **API** compatibility is the Vulkan compute subset the launcher uses
(identical calls; the Apple build also enables `VK_KHR_portability_subset`). The vendor drivers' compiled
machine code differs and is not inspected (NVIDIA binaries are never disassembled, `PROVENANCE.md`).

## StableHLO / OpenXLA interchange decision (reasoned; REJECTED for this application, kept at the framework boundary)

- StableHLO portable artifacts carry 5 years of backward and 2 years of forward compatibility (VHLO,
  openxla/stablehlo `docs/compatibility.md`). That is the right property for a framework-boundary exchange
  (JAX/TF/PyTorch export), which belongs to Omega Bridge (ADR 0042), and Omega already emits StableHLO for
  elementwise units (`omega-mv` `seed/stablehlo/irstablehlo.om` at main `943d9555`, host-oracle-exercised
  with IREE and XLA CPU, hardware NOT_RUN).
- It cannot carry this application's contract: the StableHLO spec makes the order of `reduce`
  implementation-defined (a full binary tree chosen by the implementation) and defines `dot_general` through
  `reduce`, so fsum16's frozen left-to-right order and gemm16's frozen k order (the property that makes the
  outputs bit-identical across two vendors) are not expressible. Only vadd is order-free.
- Executing StableHLO means XLA/PJRT or IREE: third-party compilers and runtimes on the execution path,
  which would take execution away from CUDA++ and computation away from Omega (ADR 0042) and is outside
  "own engine". Decision: no StableHLO in the execution path or as the contract of this workload; keep the
  existing Omega-to-StableHLO export as an optional boundary format with tolerance-based contracts,
  revisit when a TPU runner exists (DRA-205). No rewrite is forced by this decision.

## What this does not show (NOT_RUN or out of scope)

- Execution through AIEN's own engine on both GPUs: the portable route is vendor runtimes on both (NVIDIA
  Vulkan, MoltenVK). CUDA++'s own path ran on the GB10 for vadd only, from CUDA++'s own kernel builder, not
  from these SPIR-V modules. No AIEN lowering of SPIR-V or of an Omega kernel IR to both sm_121 and Apple
  exists yet (ADR 0043 E3, omega-mv#883; MSL via E2, omega-mv#888).
- The own Omega LLM on either GPU through one source: NOT_RUN (its GPU kernels are GB10-only; nothing lowers
  them to SPIR-V/MSL). Its CPU path is identical on Grace and Apple M2 (apple195), a CPU row.
- "Zero rewrite" is limited to this three-kernel FP32 subset. No claim of arbitrary CUDA source or binary
  portability (CUDApp#74, #76 untouched).
- Native tuned baselines Metal/MPS (M2) and CUDA/cuBLAS (GB10): NOT_RUN. Independent reviewer replay of both
  runs: NOT_RUN (needs the reviewer profile and the Mac; commands below).
- AMD, Intel, other Apple chips, the ANE: NOT_RUN (no runner).

## Replay

```sh
# Spark, CPU pool lock (producer; writes MT_OUT with the modules, inputs and references):
SPIRV_VAL=… LD_LIBRARY_PATH=… CUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 MT_CFLAGS=-I<vulkan headers> \
  LVP_ICD=/usr/share/vulkan/icd.d/lvp_icd.json MT_OUT=<dir>/run HELLO_GPU=<build>/hello_gpu \
  sh tests/multitarget/run.sh <abs>/omc                         # MULTITARGET PASS pass=52 fail=0 not_run=2
# copy vadd/fsum16/gemm16 .vk(.mutant).spv, a/b/x/ga/gb.bin, *.ref with SHA256SUMS to DIR on each machine
# Mac (nice 10):
MT_REPS=10 MVK_LIB=<MoltenVK>/dynamic/dylib/macOS/libMoltenVK.dylib MVK_INCLUDE=<MoltenVK>/include MT_CC=clang \
  sh tests/multitarget/apple-gpu.sh DIR                         # APPLE-GPU PASS pass=20 fail=0
# Spark, inside spark-window open NAME --lease 20 --gb10 exclusive --quiet exclusive --wait:
MT_GPU_WINDOW=<id> CONFORM=<MT_OUT>/conform MT_COMPARE_DIR=<Mac DIR copied back> MT_REPS=10 \
  HELLO_GPU=<build>/hello_gpu HELLO_GPU_CONTROL=<build>/hello_gpu_control MT_CFLAGS=-I<vulkan headers> \
  taskset -c <cores> nice -n 10 sh tests/multitarget/nvidia-gpu.sh DIR   # NVIDIA-GPU PASS pass=46 fail=0 not_run=0
```

Build of hello_gpu: `make OUT_DIR=<build> <build>/hello_gpu <build>/hello_gpu_control` (gcc 13.3.0;
`hello_gpu` `de2aef09…`, `libomega_gpu.a` `18af54fd…`). Every binary digest: `outputs.sha256`.

Signed: cuda201 (t_ebf8bda5) (Hermes worker, claude-opus-5-5, anthropic)
