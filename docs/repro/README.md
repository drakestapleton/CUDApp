# Reproducing the portable application (DRA-202 package)

Linear DRA-202 (parent DRA-192). Card cuda202 (kanban t_c2fbba86). This package lets someone outside
AIEN reproduce, from public sources only, the DRA-201 result: one unchanged seeded-Omega application ran
on an NVIDIA GPU (GB10) and on a non-NVIDIA GPU (Apple M2), and the two GPUs wrote byte-identical
outputs that equal Omega's bit-exact reference (`evidence/PORTABLE-APP-20261107/README.md`).

| Claim about this package | Status |
|---|---|
| Package (pinned sources, tool fetch, producer, two GPU replays, published digests, cost meter) | IMPLEMENTED on this branch |
| Author-side clean-room reproduction from a fresh clone of the pushed branch | see `evidence/REPRO-COST-20261107/README.md` |
| Independent reproduction by someone outside AIEN | **NOT_RUN**: nobody outside was contacted. DRA-202 grants no outreach authorization; the package is prepared first, contact is Drake's decision |

## What you reproduce

- The application: three FP32 compute kernels written in seeded Omega, `backend/spirv/emit.om` (SPIR-V
  builder), `backend/spirv/conform.om` (host reference and bit compare), `backend/targets.om` (target
  table and code-object loader checks): `vadd` (elementwise add, 1048576 elements), `fsum16` (frozen
  left-to-right 16-wide row sum, 65536 rows), `gemm16` (256 batched 16x16x16 products, frozen k order, no
  fused multiply-add). ADR 0043 portable-backend contract.
- The claim: the SPIR-V modules built once on Linux AArch64 give, on each GPU, output files whose
  sha256 equal `docs/repro/expected.sha256` (the `out/` lines). The real kernels' outputs equal the
  references; the planted-wrong mutants' outputs are the fixed wrong bytes (so a GPU that "passes" by
  returning the reference regardless is caught).
- Not covered: a model (this workload has none), an AIEN-owned GPU execution path for SPIR-V (the GPU
  routes are vendor runtimes: NVIDIA's Vulkan driver, MoltenVK), arbitrary CUDA programs, performance.

## Support matrix

| Stage | Platform | Status |
|---|---|---|
| Producer (`tools/repro/produce.sh`) | Linux AArch64. Exercised on NVIDIA DGX Spark, Ubuntu 24.04 (DGX OS), kernel 7.0.0-1019-nvidia, gcc 13.3.0, Mesa lavapipe 25.2.8, Vulkan loader 1.4.328.1 | see evidence |
| NVIDIA GPU (`tools/repro/replay-nvidia.sh`) | NVIDIA GB10 (DGX Spark), NVIDIA driver 580.173.02 | see evidence |
| NVIDIA GPU, any other model or x86-64 | — | NOT_RUN: `tests/multitarget/nvidia-gpu.sh` refuses a host that is not Linux AArch64 |
| Apple GPU (`tools/repro/replay-apple.sh`) | Apple M2, macOS 26.6.2, MoltenVK 1.4.2 | see evidence |
| Apple M1/M3/M4 | — | NOT_RUN; set `MT_EXPECT_DEVICE` to the device name the launcher prints |
| AMD, Intel, Qualcomm, other GPUs | — | NOT_RUN (no runner; access packets only, `docs/access/` on a separate branch) |
| Producer on macOS or x86-64 | — | not supported: the seeded omc writes AArch64 Linux executables. On a Mac use an AArch64 Linux VM (DRA-195 used Lima 2.2.0 with Debian 13) |

## Dependencies, pins and licenses

| Dependency | Pin | How you get it | License | Role |
|---|---|---|---|---|
| aien-dev/CUDApp | the commit you were given (this branch) | `git clone` | Apache-2.0 (`LICENSE`; files carry `Apache-2.0 WITH LLVM-exception`) | the application, the harness, this package |
| seeded omc | aien-dev/omega-mv `369dbc96daa1684db2d7c2c7085bbf16b3fd77ee` `seed/out/omc`, sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f` | `git clone` (step 1) | Apache-2.0 | compiles the `.om` units; every script refuses another compiler (exit 69) |
| spirv-tools | 2025.4-1~ppa2 arm64, sha256 `45defd68…02f8` | `tools/repro/fetch-tools.sh` | Apache-2.0 | `spirv-val`, independent validator |
| libvulkan-dev | 1.4.328.1-1~1 arm64, sha256 `ad1170ca…431f` | `fetch-tools.sh` | Apache-2.0 | `vulkan/vulkan.h` for the launcher build |
| opencl-c-headers | 3.0~2023.12.14-1, sha256 `38f4696a…2f27b` | `fetch-tools.sh` | Apache-2.0 | `CL/` headers for the launcher build |
| Mesa lavapipe | 25.2.8 (`mesa-vulkan-drivers`) | system package (`apt install mesa-vulkan-drivers`) | MIT | CPU Vulkan driver the producer runs the modules on |
| Vulkan loader | `libvulkan1` 1.4.328.1 | system package | Apache-2.0 | loads lavapipe (producer) and NVIDIA's ICD (GB10) |
| C compiler | gcc 13.3.0 (Linux), Apple clang 17 (macOS) | system | GPL-3.0 with runtime exception / Apache-2.0 | builds the interim launcher `tools/multitarget/spirv-host.c` only |
| NVIDIA driver | 580.173.02 (open kernel module + proprietary user space incl. Vulkan) | already on the machine | MIT/GPL-2.0 kernel module; NVIDIA proprietary user space | the NVIDIA vendor-runtime route |
| MoltenVK | v1.4.2 `MoltenVK-macos.tar` sha256 `f95765a6…024e` | `tools/repro/replay-apple.sh` downloads it | Apache-2.0 | the Apple vendor-runtime route (Vulkan over Metal) |
| git, curl, sha256sum/shasum, dpkg-deb or ar+tar | any | system | various | fetching and checking |

The full digests are in `tools/repro/fetch-tools.sh` and `tools/repro/replay-apple.sh`.

## Steps

All paths absolute. Nothing below needs root except installing the two system packages, if they are
missing. Every step ends with one `PASS`/`FAIL`/`NOT_RUN` line; keep every output directory (it is the
receipt, below).

```sh
# 1. Sources (Linux AArch64)
git clone https://github.com/aien-dev/CUDApp && cd CUDApp && git checkout <commit>
git clone --filter=blob:none --no-checkout https://github.com/aien-dev/omega-mv ../omega-mv
(cd ../omega-mv && git sparse-checkout set --no-cone /seed/out/ && git checkout 369dbc96daa1684db2d7c2c7085bbf16b3fd77ee \
  && sha256sum seed/out/omc)            # must print 2c82480596bbdc50…029f
OMC=$(cd ../omega-mv && pwd)/seed/out/omc
W=$HOME/cudapp-repro                    # any empty directory

# 2. Pinned tools, unpacked under $W/tools (no root)
sh tools/repro/fetch-tools.sh $W/tools  # FETCH PASS root=...

# 3. Producer: build the SPIR-V modules, inputs and references from Omega; run them on lavapipe (CPU)
sh tools/repro/produce.sh $OMC $W/tools/root $W/out   # REPRO PRODUCE PASS files=14

# 4. NVIDIA GPU (DGX Spark GB10). Make sure nothing else uses the GPU; MT_GPU_WINDOW names that session.
MT_GPU_WINDOW=<your-session-id> TOOLROOT=$W/tools/root sh tools/repro/replay-nvidia.sh $W/out   # REPRO NVIDIA PASS files=6

# 5. Apple GPU (Mac with Apple silicon): copy $W/out/gpu-in to the Mac, clone CUDApp at the same commit there
sh tools/repro/replay-apple.sh $MACDIR  # $MACDIR/gpu-in present; REPRO APPLE PASS files=6
```

Diagnostics: `make doctor OMC=$OMC` prints the target table from `backend/targets.om` (which targets are
supported, planned or host-only). `sh tests/multitarget/run.sh $OMC` alone, without the launcher, checks
the Omega units, `spirv-val` and the loader refusals with no device. Each GPU replay prints the device's
capability queries (name, vendor, driver, Vulkan version, limits) into its log.

Optional native baseline on the GB10: build CUDA++'s own sm_121 `hello_gpu`
(`make OUT_DIR=$W/build $W/build/hello_gpu $W/build/hello_gpu_control`) and set `HELLO_GPU` and
`HELLO_GPU_CONTROL` for step 4; its output digest must equal `vadd.ref`. Without them those rows print
NOT_RUN.

## What to send back (the reproduction receipt)

- `$W/out/identity.txt`, `$W/out/run.log`, `$W/out/produce.check`, `$W/out/gpu-in/SHA256SUMS`
- `$W/out/nvidia/nvidia-gpu.log`, `identity.txt`, `replay.check` (step 4)
- `$MACDIR/apple/apple-gpu.log`, `identity.txt`, `replay.check` (step 5)
- the three final lines, the commit you checked out, and every place where you had to do something this
  file does not say (an intervention). An intervention is a finding, not a failure: it goes into the
  "Interventions" table below with your name.

A reproduction counts as independent only when someone without access to AIEN machines or accounts runs
it on their own hardware from these instructions and sends the receipt.

## Rollback

Everything the steps write is inside the clones, `$W` and `$MACDIR`. Delete them to undo the package.
`fetch-tools.sh` installs nothing system-wide. The NVIDIA replay runs in conformance mode with the driver's
shader disk cache off; its one cache-on control writes only under `$W/out/nvidia`, and the script checks
that the driver's default cache directory is unchanged. MoltenVK is unpacked under `$MACDIR/moltenvk`. If
you installed `mesa-vulkan-drivers` or `libvulkan1` for step 3, remove them with your package manager.

## Model preparation

None. This application has no model and no weights. The own Omega LLM is a separate workload: it runs on
the GB10 through aien-train, but not from this portable source on a second GPU vendor (NOT_RUN; it needs
the ADR 0043 E2/E3 lowering, omega-mv #888 / #883).

## Extension interface

How to add to the application, at this commit. This is the interface the harness actually uses, not a
versioned public API: there is no compatibility promise yet (status PROPOSED as a public interface).

- A kernel: a builder in `backend/spirv/emit.om` (SPIR-V 1.3, Vulkan 1.1 environment, `GLCompute`, 64
  invocations per workgroup, storage buffers only), its bit-exact reference and input generator in
  `backend/spirv/conform.om`, and its name in the kernel lists of `tests/multitarget/run.sh`,
  `nvidia-gpu.sh` and `apple-gpu.sh`. Contract rules (ADR 0043): binary32, round to nearest even, no
  contraction, a frozen operation order, a planted-wrong mutant that must change bytes.
- A target: a row in `backend/targets.om` (code-object format, environment, status), loader checks in
  `ob_code_load`, and a launcher build (`tools/multitarget/spirv-host.c` `MT_TARGET`, migration-pending C,
  default OFF; its successor is `backend/vulkan/host.om` over E2 checked shared-library calls).

## Interventions found while packaging (author side)

The DRA-201 run could not have been reproduced from the repository alone. These are the gaps found and
how this package closes them.

| # | Gap at `bec13796` | Closed by |
|---|---|---|
| 1 | The commands that ran the producer and the GB10 replay (cuda201's `stepA.sh`, `stepB.sh`) and the tool download were not committed | `tools/repro/produce.sh`, `replay-nvidia.sh`, `fetch-tools.sh` |
| 2 | The producer's `SHA256SUMS`, which both GPU scripts require, was written by the uncommitted driver script, not by `run.sh` | `produce.sh` writes it and checks it against `docs/repro/expected.sha256` |
| 3 | spirv-tools 2025.4 and libvulkan-dev 1.4.328 came from a PPA that DGX OS configures; stock Ubuntu noble resolves older versions | fixed URLs plus sha256 in `fetch-tools.sh` |
| 4 | The published GPU outputs were only in an evidence file, not a checkable list | `docs/repro/expected.sha256` and the `sha256sum -c` in each replay |
| 5 | MoltenVK's pinned download was a step in a Mac session, not a script | `replay-apple.sh` downloads and checks it |
| 6 | `nvidia-gpu.sh` needs `MT_GPU_WINDOW`, named after the Spark's lease tool | documented above: any id for an exclusive GPU session |

Still open (not closed here, recorded as limits): the producer needs Linux AArch64; `nvidia-gpu.sh`
refuses x86-64 and reads AArch64 library paths; `apple-gpu.sh` defaults to the device name "Apple M2";
lavapipe and the Vulkan loader are assumed to be system packages. Each is a reason an outside
reproduction on other hardware would print NOT_RUN rather than PASS.

## Cost of the migration

`docs/repro/MIGRATION-COST.md` (measured files, lines, commits, agent time, tokens, estimated cost) and the
pre-registered manual-baseline study `docs/repro/STUDY-PREREG.md`. Reproduce the cost table with
`sh tools/repro/cost.sh $OMC`; it must equal `tests/repro/cost.expect`.
