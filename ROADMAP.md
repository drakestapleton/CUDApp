# CUDA++ roadmap: every NVIDIA piece on the Spark, and what does its job

The goal is a DGX Spark where CUDA++ does the jobs of NVIDIA's user-space software and my own GB10 driver does the job of NVIDIA's kernel modules, on top of nothing of NVIDIA's but the GPU's own signed firmware, and a GPU stack that does things CUDA does not. GB10 facts measured on the way (chip id, boot sequence) still go upstream to the Linux drivers Nova and nouveau as findings. Each row names its receipt; a row without a receipt is not done. Status words: PASS (receipt on a named commit), PARTIAL, MISSING, BUILDING (work exists, no receipt on the GB10 or in CUDA++ yet), stays (NVIDIA's part remains in the path).

Sources: Omega `docs/OMEGA.md` sections 2 to 5 (branch `docs/what-omega-is`, 2026-10-21). Driver policy changed on 2026-10-30: my own driver replaces `nvidia.ko` and `nvidia-uvm.ko`, as aien-dev/omega `docs/OMEGA.md` on main (f56a372a) already says; the earlier line here, that the L332 work was a research harness and GB10 support belonged only upstream, is withdrawn. The route analysis (user-space Omega over `vfio-pci`, nouveau and nova, or a kernel module) is `docs/nvidia-out/DESIGN.md`, layer L4.

2026-11-06 codegen-rest migration note: the attn256, pattn256, router, FP8
GEMM, BF16 GEMV, pq, and qg builders now have seeded Omega implementations
under `om/codegen/`, replayed through the shared CUDApp IR path. The original C
builders remain default-off differential oracles. The replay wire adds opt-in
metadata word 6 for the existing extended GPR limit; zero preserves the old
wire. The FP8 GEMM admission check now rejects overflowing `mt` before
multiplication. CPU differential, simulator, and mutation receipts belong in
`evidence/codegen-rest/2026-11-06/`; the hardware tiers remain NOT_RUN and no
roadmap bar is upgraded by this migration. See `om/codegen/CHIP.md`.

## 1. NVIDIA software on the Spark today

| NVIDIA piece | What it does | CUDA++ replacement | Status | Receipt or where the work is |
| --- | --- | --- | --- | --- |
| `nvcc`, NVVM, PTX, `ptxas` | Compile kernels to machine code | sm_121 encoder + register allocator (`src/omega_blackwell_*`) | PASS | every GPU kernel this stack runs is encoded by it (Omega `docs/OMEGA.md` s2) |
| `libcuda` | User-space driver API | `nvrm/` + `m16/` driver client | PARTIAL | Implemented; imported M16 summary lacks its referenced raw logs. Current native qualification NOT_RUN; see README evidence limitation. |
| `libcudart` | Runtime API: device, memory, launch, streams | session, command lists, job engine | PARTIAL | one device and one process; streams and events are bar C4 |
| cuBLAS | Matrix multiply | matmul, tensor-core GEMM, FP8/BF16 libraries | PARTIAL | model shapes only; any-shape GEMM is bar C3 |
| cuDNN, FlashAttention | Attention, network layers | decode and prefill attention | PARTIAL | model-shaped |
| CUB, Thrust | Reductions, scans, sorts | frozen-order reductions; f32 and u32 prefix sums and a stable radix sort (keys, key-value pairs) whose kernels and launch plans are seeded Omega (`om/scan/scan_ir.om`, `om/scan/scan_plan.om`, `src/omega_gpu_scan.h`) | PARTIAL | `evidence/E1-CLOSURE/`; general scan and sort are bar C3. Note 2026-11-06 (branch `feat/c3-scan-sort-om`): host and warp-simulator tiers bit-exact for every op at n = 1 to 70, the tile, CTA and grid edges up to 32769, and 40000 and 270000 (`make test-gpu-scan`), C-vs-Omega differential and six `.om` mutants caught (`mk/scan-sort.mk`), all 22 scan kernels decode identically in nvdisasm and our disassembler; chip tier NOT_RUN, so no receipt for 2^28 yet |
| cuFFT, cuRAND | FFT, random numbers | xoshiro256** generator (64-bit, uniform and normal f32), Philox4x32-10 (cuRAND's `curandStatePhilox4_32_10_t` stream), batched complex f32 FFT, real R2C/C2R FFT (cuFFT layout) and 2-D/3-D complex FFT, kernels and pass plan built by the seeded unit `om/mathlibs/mathlibs_ir.om` (`src/omega_gpu_mathlibs_api.h`, runtime API 0.14.0 section 9.9) | PARTIAL | `evidence/MATHLIBS-RNG-FFT-2026-10-27/chip-receipt.txt`: one GB10 run, 268 checks, streams bit-exact with aien-math; complex-to-complex FFT from 4 to 2048 points. 2026-11-06 note (`evidence/MATHLIBS-PHILOX-RFFT-ND-2026-11-06/`): Philox (Random123 KAT, 1024 cuRAND 10.4.0 host-oracle rows), R2C/C2R for 8 to 4096 points and 2-D/3-D transforms (dims 4 to 2048) pass the host and warp-simulator tiers with every mutant caught; their chip tier is NOT_RUN. No Philox uniform/normal output, no non-power-of-two sizes |
| `cuobjdump`, `nvdisasm` | Read machine code back | my own sm_121 disassembler | BUILDING | bar C6: on Omega branch `c6/disasm` (19eebe0e, `evidence/C6-DISASM/` there) it agrees with `nvdisasm` on 331 kernels; not carried into CUDA++ yet, and some host tests here still call `nvdisasm` |
| Nsight | Profiling | ceiling probes | PARTIAL | `evidence/OWN30B-T3-TC-PEAK-BW-5faa0f70-20261013T2236Z/` (115.14 TFLOPS tensor-core peak, 195.1 GB/s reads), `evidence/OWN30B-LAUNCH-COST-20261014T0120Z/`. 2026-11-05: `tools/prof` host and simulator report against those ceilings and Omega docs/OMEGA.md section 5 (`make test-prof`). Chip tier NOT_RUN (`tools/prof/chip.sh`). Status stays PARTIAL until a chip receipt. |
| CUDA samples and docs | First kernel, API reference | `examples/hello_gpu`, `docs/runtime/API.md` | PARTIAL | bar C2: hello-GPU from an empty folder on the GB10 + the zero-CUDA gate |
| `nvidia.ko` | Kernel driver: GPU boot, memory, channels | my own GB10 driver, in three repositories: aien-dev/aien-gpu (Omega units inside a Linux module), aien-dev/omega-gpu-driver (seeded Omega over `vfio-pci`), aienux#441 (L332: C reference driver and GB10 simulator) | BUILDING | no track has run on the GB10. aien-gpu `docs/STATUS.md` (ec9d6d76): host and QEMU EXERCISED, hardware NOT_RUN. omega-gpu-driver README (07781415): W0 EXERCISED in QEMU only. aienux#441: merged into aienux main on 2026-10-30 (merge 4c25c6ce); L332 simulator gates PASS at cd5eef7a (`evidence/own-gpu-driver/rung1/gates-20261021T2158Z.txt` there). Open before the chip: the chip id is unread (`PMC_BOOT_42`, aienux `docs/GB10_FIRMWARE_INVENTORY.md` on main since #441), the GPU windows wait for my go, and the `nvrm/` client still speaks NVIDIA's ioctls (moving it is PROPOSED) |
| `nvidia-uvm.ko` | Unified memory, page tables, faults | my own GB10 driver (same three repositories; aien-gpu's README names `nvidia_uvm` as replaced) | BUILDING | host simulator, 225 checks PASS (aienux `feat/gb10-gpu-mmu-sim` bcc56506); no chip run |
| `nvidia-modeset.ko`, `nvidia-drm.ko` | Display | not needed for compute | not used by CUDA++ | |
| `nvidia-smi`, NVML | Status, clocks, power | `cudapp-smi`, a seeded Omega program (`native/smi/`); NVML subset `compat/nvml/` over pure Omega units (`native/nvml/`) | MISSING | 2026-11-05 note, no receipt on the GB10 yet: host fixture and fake resource-manager tiers PASS (`make test-smi`); the live read-only procfs/sysfs run matches `nvidia-smi` on 20 fields (`evidence/SMI-2026-11-05/`). Name, temperature, pstate and per-process memory need the `--rm` path, whose live run is NOT_RUN (needs a spark-window). Utilization is blocked by omc's 16383-byte `devcall` bound (55888-byte control; aien-dev/omega-mv#1042). Clocks and power have no public control in the 580.173.02 headers. 2026-11-06 note: an NVML C ABI subset (init/shutdown, count, handle, name, UUID, temperature, memory, utilization, compute processes, error strings) passes its host tier against a fake resource manager, 22/22 mutants caught, ABI equal to CUDA 13.0 `nvml.h` on 55 facts (`make test-nvml`, `evidence/NVML-2026-11-06/`); live resource-manager and chip runs NOT_RUN. |
| GSP firmware (`gsp_*.bin`) | Runs inside the GPU | stays NVIDIA's: the chip checks its signature | not replaceable | the kernel module loads NVIDIA's signed file from the system's firmware folder; CUDA++ never ships it |

2026-11-06 attention note: `om/attn-any/` adds seeded-Omega arbitrary-shape
f32/bf16/f16 online-softmax attention (dimensions 1..256, rectangular sequences,
GQA and top-left causal/non-causal masks), with independent host reference,
warp simulation, source-mutant controls and `om/attn-any/CHIP.md`.
Chip qualification is NOT_RUN; the §1 row remains PARTIAL and existing
model-tuned attention generators are unchanged.

2026-11-06 DNN note: `om/dnn-rows/`, `om/dnn-activations/` and `om/conv2d/`
add seeded row softmax/layernorm/rmsnorm, ReLU/erf-GELU/tanh-GELU/SiLU and
strict implicit-GEMM conv2d. Shared f32/bf16/f16 input formats accumulate
to f32; convolution supports NCHW/NHWC, groups, stride, asymmetric padding
and dilation without a global im2col buffer. Shape admission, tail/address
logic and arithmetic are Omega. Independent double references, warp
simulation and compiled source mutants exercise the host path; see
`om/dnn-layers/README.md` for scope and device-free commands.
Chip qualification and performance are NOT_RUN. This is a dated implementation
note, not a status upgrade or complete cuDNN feature-coverage claim.


## 2. Bars before I say "replaces CUDA"

From Omega `docs/OMEGA.md` section 3. Each needs a receipt on the exact commit; none is waived.

- **C1** Omega's languages compile kernels through checker2 to sm_121 and run on the GB10 (vector add, tiled GEMM, decode attention; a broken kernel is refused; one live 35B kernel replaced with identical golden tokens). The language part lives in Omega; the code generator it targets is here.
- **C2** a stranger runs hello-GPU from an empty folder with only the README commands, and the zero-CUDA gate passes on the binary; the runtime API is written down and versioned.
- **C3** any-shape GEMM in BF16 and FP8 (200+ shapes, 1 to 8192, each checked against a host reference, speed as a fraction of the measured tensor-core ceiling); reductions and prefix sums from 1 to 2^28 elements, bit-exact.
- **C4** streams and events: two queues finish in under 0.75 of the sum of their solo times; an event wait orders two jobs, checked by data.
  2026-11-05 (CUDApp#115): the pure part (method words of a submission incl. wait_for ACQUIRE and barrier choice, submit_pair markers, AIEN_OMEGA_GPU_CHANNELS, extra queue admission) is seeded Omega, `om/streams/streams.om`; C writes and submits. Host and sim tiers PASS, C vs Omega identical on 102400 swept chunks (`make test-om-streams`), 4 mutants red. Chip NOT_RUN; status unchanged.
- **C5** two processes share the GPU safely, and a kernel that never ends is timed out with the process still usable.
- **C6** my disassembler agrees with `nvdisasm` on every library kernel, and nothing calls `nvdisasm` any more.

## 3. More than CUDA

Things a CUDA stack does not do, because it does not own the whole path. Omega `docs/OMEGA.md` bar P4: at least one measured win from each before it is claimed.

| Item | Why CUDA cannot | Status |
| --- | --- | --- |
| One submission per generated word | CUDA launches go through the runtime and driver per kernel or per graph; I write the command list myself | PARTIAL: command lists exist (`src/omega_gpu_cmdlist.*`); extra launch in a list costs 24 to 31 us today with wait-for-idle (`evidence/OWN30B-LAUNCH-COST-20261014T0120Z/`). 2026-11-06: seeded record/sizing and whole-program aliased RAW/WAR/WAW planning added (`om/cmdlist/`), retaining one submission or refusing the ring budget; independent launches need no inter-launch WFI. Host differential/simulator and count-model gates are separate from chip ordering/latency, which remain NOT_RUN. Complete model-graph effect annotations are still required; status is not upgraded. |
| Resident whole-model kernel | needs control of scheduling and memory below the CUDA API | MISSING (AIEN L272). 2026-11-06: seeded descriptor-driven implementation in `om/resident/` and five-layer reference/multi-CTA simulation tests; chip tier NOT_RUN. [Contract and chip runbook](om/resident/README.md). |
| Zero-copy shared memory | the GB10's CPU and GPU share one memory; CUDA still stages through its own allocators | PARTIAL: weights shared between processes through the driver client (physics `feat/shared-mem` 82467f19, 1 GiB import in 171 ms; not yet in CUDA++ main). 2026-11-05: Omega export and import, read-only included, in `native/share/share.om`; host only (fake RM, read-only write trap, 20 mutants caught; `make test-zero-copy-share`, `docs/share/README.md`); chip NOT_RUN. The same calls for the hosted native library in `native/share/share_native.om` (OM-2 function mode); host only (seccomp fake RM, 20 mutants caught; `make test-zero-copy-share-native`); chip NOT_RUN |

2026-11-06: ModelGraphsOm adds seeded graph plans and an old-C/default-Omega differential (`om/graph/README.md`). The independent nested gate also compares real production GDN/attention/FP8/BF16/GEMM/router command lists and submitted words, including device-free native artifact admission; old/new each submit once per complete word. This does not upgrade the row: chip qualification and complete model-graph effect annotations remain separate. See `om/graph/CHIP.md`.

## 4. Speed

Speed is reported against the physical ceiling of this machine, not only against yesterday. The table and the ceilings are in Omega `docs/OMEGA.md` section 5; the head-to-head measurement against llama.cpp and vLLM on this Spark, same model and prompts, end to end, is bar P0 (AIEN L394; in progress, not finished, not published). Until it is published, no speed comparison with CUDA, llama.cpp, vLLM or TensorRT is claimed. Chip-only time per token and end-to-end time through the server are reported separately and never set beside each other or beside other projects' published end-to-end figures.

## 5. The developer bar: goals for being easier to use than CUDA

The research in `docs/PAIN_POINTS.md` ranks what developers hit with CUDA and its libraries, and why they stay. Each pain point becomes a row with a done-when anyone can run. The full table, with every row's mechanism and the counts behind its rank, is `docs/PAIN_POINTS.md` section 3; open rows have issue drafts in `docs/issues/`. The rows are developer-experience goals, not measured comparisons with CUDA, and none of them is a speed claim.

| Row | Aims to improve on CUDA by | Status |
| --- | --- | --- |
| D1 | Memory you can see, and a refusal before the machine hangs | DONE on the host; chip run open |
| D2 | One command, one version: `make doctor`, then `make hello-gpu` | DONE in doctor wrapper; no direct library driver-version guard. 2026-11-05 note: the library's PCI-id admission at device open (D6) is host-tested only (`make test-chip-admission`) |
| D3 | Faults named at the launch that caused them | DONE on the simulator; chip half open |
| D4 | The simulator first, from the same program: `hello_gpu --sim` | DONE |
| D5 | Builds in seconds, ships small | DONE (2.0 s, 1.06 MB) |
| D6 | A new chip works on day one or is refused with a sentence | DONE in doctor wrapper; table row is not current-commit qualification. 2026-11-05 note: the library now refuses an unknown or unqualified PCI id before opening any `/dev/nvidia` node (`om/admission/chip_admit.om`); host and mutant tests only, chip run NOT_RUN |
| D7 | Same inputs, same bits, at no cost | simulator DONE; chip DONE for hello-gpu, bf16 GEMV, rmsnorm, decode attention (10 separate processes each, `evidence/d7-same-bits-chip/2026-10-27/`); other kernels not hashed; the "no cost" half is not measured |
| D8 | No per-process CUDA tax | PLANNED (chip window) |
| D9 | No licence that stops you | DONE (`evidence/dx/pain-points/licence.txt`) |
| D10 | No compiling at run time | DONE for hello-gpu |
| D11 | Open libraries with an easy default path | PLANNED (L386, Omega bar C3) |
| D12 | Work per answer, against the machine's own ceiling | simulator DONE; chip open |
| D13 | Our own disassembler, in plain text | PLANNED (L386, Omega bar C6) |
| D14 | Recover from a fault without restarting | PLANNED (L386, Omega bar C5) |

## 2026-11-06: core utility Omega migration

Core hashing, canonical encoding, object/effect/evaluation utilities, vectors,
formatting, host math and cubin refusal rendering use local `om/core/*.om` by
default. The existing cubin ELF, Ampere encoder, binary16, revision and epoch
Omega implementations are reused. The handwritten C oracle is opt-in through
`CUDAPP_MIGRATION_PENDING_C_CORE`; no hardware status is upgraded.
See `om/core/README.md` and `evidence/core-util/2026-11-06/` for host evidence,
negative controls, recompiled mutants and the authorization-gated chip runbook.

## 2026-11-06: core allocator and matmul Omega lane (pending integration)

The seeded core allocator, IR helpers, five matmul builders, scalar register/layout
decisions and vecadd writer have host differential, ten recompiled Omega mutants,
existing encoder fixtures and BF16 warp-simulator receipts in
`evidence/regalloc-om/2026-11-06/README.md`. The full assigned file-set migration
is not complete: realization, hazard analysis and remaining wrapper/packing/spec
policy are still C. Native aggregate adoption and freestanding OM2 array support
are pending integration; chip qualification is NOT_RUN. No roadmap row status is
upgraded by this note.
