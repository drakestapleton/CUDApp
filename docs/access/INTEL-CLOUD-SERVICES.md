# Intel Cloud Services: Intel GPU evaluation packet (DRA-206, for DRA-194)

State on 2026-11-07: **PACKAGE_READY**. Not registered, nothing requested, no instance, no spend.
Every Intel GPU row is NOT_RUN. CUDA++ lists `intel-xe-lp`, `intel-xe-hpg` and `intel-xe2`
(Lunar Lake / Battlemage) in `backend/targets.om` as PLANNED, hardware NOT_RUN. Its SPIR-V path
(`backend/spirv/emit.om`, checked by `conform.om`) has run only on CPU drivers (PoCL, lavapipe).

## What the provider is today (checked in a browser, 2026-11-07)

DRA-206 named Intel Tiber AI Cloud. That service is not what the Intel URLs serve today:

| URL | Result on 2026-11-07 |
|---|---|
| https://console.cloud.intel.com/ (Tiber console, still linked from Intel's Gaudi setup page) | DNS name does not resolve (`DNS_PROBE_FINISHED_NXDOMAIN`) |
| intel.com `.../developer/tools/tiber/ai-cloud.html` | redirects to the Intel Trust Authority page |
| https://cloud.intel.com/ | **Intel Cloud Services**: "Sign in with your corporate or personal account to request access to Intel hardware and accelerators" |

Intel Cloud Services, from its public pages and docs (https://cloud.intel.com/docs/index.html, release
notes GA 1.0.4 dated 4 September 2026):

- **Catalog on the public page:** AI PC (Core Ultra), **GPU Accelerators (Arc Pro B-Series)**, CPU
  servers (Xeon 6). The full hardware catalog is behind sign-in (NOT_READ). Data Center GPU Max and
  Gaudi are not on the public page; whether they can still be requested is unknown.
- **Request flow:** sign in; pick an instance in the Hardware Catalog; fill the request (instance
  name, intended use, use case, duration, deployment details; optional SSH key); wait for an email in
  2 to 3 business days saying approved, rejected or waitlisted. Pre-approved requests are available
  at once; the landing page says "Test Drive access is self-service".
- **Limits:** every reservation has a fixed end date; extensions may be requested, but the total
  cannot exceed 30 days. Data left on the instance may be lost after expiry. Up to 18 additional
  users with the same email domain.
- **Access:** browser (JupyterLab on Linux instances, RDP on Windows instances) or SSH with a public
  key uploaded to the account and attached to the instance.
- **Cost:** no price and no payment step appear in the public docs. The sign-in guide asks for a
  "corporate email address", while the landing page says corporate or personal. Both are unverified
  until Drake signs in.

What a request must get for DRA-194 to mean anything: a **Linux** instance with a **programmable
Intel GPU** (Arc Pro B-Series is the one named publicly), SSH, permission to install packages and
build software, and OpenCL or Level Zero exposed to the user. A CPU-only, Windows-only or
notebook-only instance cannot satisfy the Intel GPU gate. Gaudi is a different target with its own
runtime and cannot satisfy it either.

## Cost and time plan

| Window | Rows | Hours on the instance |
|---|---|---|
| W0 access test | `tools/access/plans/intel-gpu.tsv`: PCI identity and driver, clinfo, sycl-ls, xpu-smi, Level Zero `ze_peak` / `ze_bandwidth` | at most 1 |
| W1 candidate | CUDA++ SPIR-V vadd and fsum16 through Intel's OpenCL driver, bit compare with `conform.om`, planted-wrong control, timing; then the shared-model rows of DRA-194 | at most 6 |
| W2 replay | W1's plan rerun by a different agent | at most 2 |

All three must fit in one reservation of at most 30 days. Expected charge: $0 if the evaluation is
free as the public pages suggest; any price shown at request time is recorded in the request and
needs Drake's written approval before the instance is used.

## Sign-up and request steps (Drake, under his own identity)

1. Open https://cloud.intel.com/ and choose Sign In / Sign Up. Register with name, email, language,
   region and password; enter the emailed verification code; read and accept the Intel Cloud
   Services licence agreement (record its date or version).
2. Upload an SSH public key (console menu SSH Keys, Upload key). Use a new key pair made for this
   evaluation.
3. Open the Hardware Catalog. Record every GPU instance type listed (name, GPU model, OS, duration
   choices) in DRA-206; that is the "available hardware tier" the issue asks to verify.
4. Request one **Linux** instance with an **Arc Pro B-Series** GPU (or a newer programmable Intel GPU
   if listed). Attach the SSH key in the request form. Answers below.
5. Wait for the approval email. Record decision, date and the reservation end date in DRA-206.
6. Do not start the instance's work before W0 is scheduled.

## Request answers (draft for Drake)

- **Instance name:** cudapp-intel-eval
- **Use case:** AI / software development (pick the closest)
- **Duration:** the shortest choice that covers W0 to W2; extend only if W1 is ready
- **Intended use / deployment details:**

> I develop CUDA++ (github.com/aien-dev/CUDApp) and the Omega compiler (github.com/aien-dev/omega-mv),
> an open-source (Apache-2.0) GPU toolchain. Omega already emits SPIR-V compute kernels that pass bit
> for bit on CPU OpenCL and Vulkan drivers; I have not yet run them on an Intel GPU. On this instance
> I will record the GPU and driver, run Intel's Level Zero reference tests, then run the same SPIR-V
> kernels (vector add, row reduction, then matrix multiply) through Intel's OpenCL driver, compare
> every output with a host reference, include planted-wrong kernels that must be caught, and time
> them against Intel's own stack on the same GPU. Every run leaves a public receipt with exact
> versions and logs. Linux, SSH and permission to build software are needed.

## Host work before W1 (done on the Spark CPU, DRA-194; Intel GPU NOT_RUN)

1. **OpenCL GPU mode.** `tools/multitarget/spirv-host.c` (migration-pending C, built only with
   `-DCUDAPP_MIGRATION_PENDING_SPIRV_HOST=1`) has a build `MT_TARGET=3`: the OpenCL ICD loader is
   pinned to one vendor library `MT_CL_GPU_ICD` (Intel: the path in `/etc/OpenCL/vendors/intel.icd`),
   only `CL_DEVICE_TYPE_GPU` is enumerated (no CPU fallback: a platform without a GPU exits 5), the
   device must report vendor id `0x8086`, and the run prints platform, device name, `CL_DRIVER_VERSION`,
   device and IL versions, a timing line (setup, driver compile, run, profiled device ns) and the number
   of open `/dev/dri/` descriptors. It is opt-in twice: the build, and `MT_CL_GPU=1` at run time
   (anything else exits 2 before any input is read or any OpenCL call is made; `libOpenCL.so.1` itself
   is linked). The ICD's realpath must be a regular ELF file not named `*.icd`: ocl-icd would read a
   directory as a vendors search path and an `.icd` file as a descriptor naming another library. The
   `/proc/self/maps` and
   `/proc/self/fd` isolation scan stays, with its own default forbid list
   (`libpocl:nvidia:libcuda:libnvcuda`, checked against the ICD path before loading and against every
   mapping after the run) and a required mapping (the ICD's realpath). The default build `MT_TARGET=0`
   is unchanged: PoCL CPU only, whatever `MT_CL_GPU` says. Successor: `backend/opencl/host.om` over
   omega-mv#888 (E2).
2. **Compare path: copy back, compare on the Spark.** On the VM each candidate row compares its output
   with the conform.om reference in the bundle using `cmp` (exit 0 equal, 1 differ). The outputs stay in
   the run directory (`outputs/`, hashed into `SHA256SUMS`); after the copy back,
   `tools/access/intel-cl.sh compare` runs `conform.om check` on every output on the Spark and fails any
   output whose bytes conform.om rejects, any control whose mutant was not caught, and any row the VM
   recorded otherwise. An x86linux build of conform.om was not made: its native x86 execution is
   NOT_RUN so far, and the Spark compare needs none.
3. `ze_peak` and `ze_bandwidth` come from github.com/oneapi-src/level-zero-tests. Build them on the
   instance at a pinned tag and record the tag and binary sha256 in the receipt (TOOL lines). They
   are Intel's tests, used as the reference, not CUDA++ code.

Host evidence for 1 and 2 (Spark AArch64 CPU, no GPU): `evidence/INTEL-CL-HOSTPREP-20261107/`. Every
Intel GPU row is NOT_RUN until a window runs.

## W1 on the instance

```
# 1. Spark (CPU pool): produce the modules, inputs and references, then the bundle.
SPIRV_VAL=... CUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 POCL_LIBRARY=... MT_CFLAGS=-I<CL+vulkan headers> \
  MT_OUT=<dir>/mt sh tests/multitarget/run.sh <abs>/omc          # MULTITARGET PASS
sh tools/access/intel-cl.sh bundle <dir>/mt <dir>/intel-bundle      # 16 files + SHA256SUMS
git rev-parse HEAD                                                 # ACCESS_REV
# copy tools/access/collect.sh, tools/access/plans/intel-gpu.tsv and <dir>/intel-bundle to the VM

# 2. VM (Ubuntu with Intel's compute runtime: intel-opencl-icd, ocl-icd-libopencl1, opencl-c-headers, a C compiler)
ACCESS_WINDOW=intel-w1-<date> ACCESS_REPO=aien-dev/CUDApp ACCESS_REV=<40 hex> ACCESS_BUNDLE=$PWD/intel-bundle \
  sh collect.sh run intel-cloud intel-gpu.tsv run-intel-w1
# 3. copy run-intel-w1/ back, destroy the instance (Teardown below)

# 4. Spark: conform.om at the same rev with the pinned omc, then compare, then close.
<abs>/omc < backend/spirv/conform.om > <dir>/conform && chmod 700 <dir>/conform
sh tools/access/intel-cl.sh compare run-intel-w1 <dir>/conform run-intel-w1.compare.txt
RCHECK=<rcheck binary> sh tools/access/collect.sh close run-intel-w1 ACCESS_TESTED <usd> yes drake <time>
```

The rows: `cand-vadd-cl`, `cand-fsum16-cl`, `cand-gemm16-cl` (one module each, bytes = reference),
`cand-reps-cl` (every kernel 10 times, every output = reference, timing kept), and the controls
`ctl-{vadd,fsum16,gemm16}-mutant-cl` (planted-wrong kernels must differ, want 1), `ctl-optin-cl`
(no `MT_CL_GPU=1`, want 2), `ctl-isolation-cl` (forbid `/dev/dri/`, want 9), `ctl-require-cl` (a
required library never mapped, want 9), `ctl-vendor-cl` (expect AMD's vendor id, want 5). A refusal
control (`ctl-optin-cl`, `ctl-isolation-cl`, `ctl-require-cl`, `ctl-vendor-cl`) only passes with its exit,
the launcher's named refusal text and no output file; a mutant control passes when its output differs
from the reference (exit 1) and keeps that output for the Spark compare. Commit the run directory, the
compare file and the receipt under `evidence/ACCESS-intel-cloud-<window>/`. The compare's exit 0 is not a
window verdict (it needs neither every row nor a device). A candidate PASS counts only when rcheck
accepts the receipt and the compare file says `fail=0` with the same rows PASS on `d0`.

## Teardown

Copy the run directory back, check `SHA256SUMS` on the Spark, terminate the instance in the console
(turn off Termination Protection first if it was set), confirm it is gone, and close the receipt with
`collect.sh close`. Delete the uploaded SSH key from the account when the evaluation ends.

## If no suitable GPU is offered

Record the exact catalog (or the rejection) on DRA-206 and keep the rows `BLOCKED_ACCESS`. Do not
substitute a Xeon, Core Ultra CPU or Gaudi result for the GPU gate. Alternatives for DRA-201's one
non-NVIDIA device: the AMD packet (`AMD-DEVELOPER-CLOUD.md`) and the Apple M-series lane (DRA-195).
Buying hardware or paid cloud time needs Drake's written approval.
