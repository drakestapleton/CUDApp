# PORTABLE-APP-20261107: preregistration (written before any GPU run of this card)

Card cuda201 (kanban t_ebf8bda5), Linear DRA-201 under DRA-192. Parent result: apple195 (DRA-195,
`evidence/APPLE-M2-20261107/`, branch `hive/t_e1d7686d-apple-m2` @ `33f15580`). This file is committed and
pushed before the first GB10 run and before the Apple M2 replay, so its criteria cannot be tuned to results.

## Selection

- Non-NVIDIA backend: Apple M2 GPU (MacBook Air Mac14,2), the first physically available non-NVIDIA GPU.
  AMD (DRA-204) and Intel (DRA-206) are access preparation only on 2026-11-07: no runner exists. Qualification
  issue for the Apple runner: DRA-195 (In Review; CUDApp branch `hive/t_e1d7686d-apple-m2` READY on the desk).
- NVIDIA backend: the DGX Spark GB10 (sm_121), inside one exclusive `spark-window` lease.
- Application: the ADR 0043 contract workload of `tests/multitarget/run.sh`, three FP32 kernels
  built by seeded Omega: `vadd` (1048576 elements), `fsum16` (65536 rows x 16, frozen left-to-right order),
  `gemm16` (256 batches of 16x16x16, frozen k order, no contraction). Application source =
  `backend/spirv/emit.om` (kernels), `backend/spirv/conform.om` (inputs, reference, compare),
  `backend/targets.om` (loader admission). These three files are not changed by this card.
- Not selected, with reason: the own Omega LLM forward/train on a GPU. Its kernels exist for the CPU and
  for the GB10 only; nothing lowers them to SPIR-V or MSL, so no unchanged GPU source exists on a second
  vendor (NOT_RUN, concrete gap: an Omega SPIR-V or MSL lowering of the ollm kernels, ADR 0043 E3). The
  model's CPU path is already identical on Grace (Spark) and Apple M2 (guest) in apple195's receipts; that
  row is a CPU row and is not counted as a GPU backend. TensorFold (DRA-197..199) has no verdict yet and is
  not a consumer here.

## Same artifacts on both GPUs

The SPIR-V modules, inputs and references are produced once, on the Spark, by `run.sh` with the pinned
omc (`2c824805…029f`), and the same bytes go to both GPUs (`SHA256SUMS` checked on each side). Expected
digests (equal to apple195's `outputs.sha256`): `vadd.vk.spv b56654c9…0444`, `fsum16.vk.spv 1c139c14…5805`,
`gemm16.vk.spv 191e4f42…c9e`, inputs `a.bin 6f5b3e0f…a2c1`, `b.bin 6b2796ec…b316`, `x.bin 2c9a66be…de01`,
`ga.bin fa4636b8…6c3c`, `gb.bin e79a069f…dcee`.

Build/configuration differences, enumerated (no other difference is allowed):

| Item | GB10 | Apple M2 |
|---|---|---|
| Launcher source | `tools/multitarget/spirv-host.c` (one file, same commit) | same |
| Launcher build | `-DMT_TARGET=2`, gcc, Linux, `-l:libvulkan.so.1` (Khronos loader pinned to `nvidia_icd.json`, layers off) | `-DMT_TARGET=1`, Apple clang, MoltenVK linked directly (no loader) |
| Vendor runtime | NVIDIA Vulkan driver 580.173.02 (proprietary user space, SPIR-V compiler in the driver) | MoltenVK 1.4.2 (SPIRV-Cross to MSL, Metal compiler) |
| Floating-point mode pins | inherited `__GL_*`/`__NV_*`/`VK_*` cleared; shader disk cache off | inherited `MVK_*` cleared; `MVK_CONFIG_FAST_MATH_ENABLED=0` |
| Dispatch | 64 invocations per workgroup, `ceil(n/64)` groups, host-visible coherent buffers | same |
| Weights / repacking | none (no model weights in this workload) | none |

## Pass criteria (frozen)

1. Correctness tolerance: **0 ULP**. Every output byte of every real kernel equals the Omega reference
   (`cmp` and `conform check` exit 0), on each GPU, for the first run, a repeat run and every timing
   repetition.
2. Every planted-wrong mutant module differs from the reference on each GPU (`conform check` exit 1).
3. Cross-target: each GB10 output file equals the Apple M2 output file byte for byte (real and mutant).
4. No hidden fallback: the GB10 launcher refuses a CPU device (lavapipe manifest, exit 5), a wrong device
   (exit 5) and a wrong driver; it fails isolation (exit 9) when told to forbid the NVIDIA driver core or to
   require a library that is not loaded; no `libcuda`, PoCL or lavapipe mapping is present during the run.
   The Apple launcher keeps apple195's controls (forbid MoltenVK 9, wrong device 5, cl refused 2).
5. Cache/lifecycle (GB10): a cache-off build writes nothing to a private cache path; a cache-on build
   writes cache files there and a warm run gives the same bytes; corrupted cache entries give the same
   bytes or a fail-closed refusal, never wrong bytes; the driver's default cache is untouched.
6. Unsupported operations: the Omega loader refuses MSL_SOURCE/AARCH64 (11), CUBIN (12), PTX (10) and a
   missing entry (23) before any run (run.sh); given a missing entry anyway, each driver must fail closed
   (no output). apple195 found MoltenVK aborts (134); the GB10 driver's behaviour is recorded as observed.
7. Native baseline on the GB10: CUDA++'s own sm_121 `hello_gpu` vadd (no CUDA, no NVIDIA user-space library)
   prints output sha256 = `vadd.ref` (`72d0c1b2…b6a6`) on every repetition; `hello_gpu_control --bug=short`
   goes red. Apple native (Metal/MPS) and NVIDIA vendor-native (CUDA/cuBLAS) baselines are NOT_RUN: they
   need foreign harness code that is not authorized here.

## Timing (observation only; never a pass condition)

- 10 fresh processes per kernel per GPU (`MT_REPS=10`), conformance build. Recorded per run: setup,
  pipeline (driver compile), run (record+submit+wait), device span (timestamp queries around the dispatch),
  total process time to output, peak RSS. Reported: median, minimum and maximum per kernel per target.
- GB10 run inside `--gb10 exclusive --quiet exclusive`. The Mac is an unmanaged laptop on battery; its
  numbers carry that condition.
- Same-device comparison on the GB10 only: CUDA++ native vadd device time (hello_gpu "the GPU took") versus
  the NVIDIA Vulkan route's device span for the same vadd. Their launch shapes differ (hello_gpu: 8
  launches of 131072 elements at 128 threads; Vulkan: one dispatch of 16384 x 64), so this is a reported
  observation, not a speedup claim. GB10 versus M2 numbers are cross-system cost observations, never a
  software speedup.

Signed: cuda201 (t_ebf8bda5) (Hermes worker, claude-opus-5-5, anthropic)
