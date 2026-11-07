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

## Host work before W1 (none of it is done)

1. `tools/multitarget/spirv-host.c` enumerates only `CL_DEVICE_TYPE_CPU` and pins the OpenCL loader
   to one CPU library (PoCL). Running on an Intel GPU needs a GPU mode that accepts
   `CL_DEVICE_TYPE_GPU` from Intel's driver, records the device name and driver version, and keeps
   the current CPU-only mode and its isolation controls unchanged. This is a change to existing,
   migration-pending C; its successor stays `backend/opencl/host.om` over omega-mv#888 (E2). The
   Apple lane (DRA-195) is changing the same file for MoltenVK; the two changes must be coordinated.
2. `conform.om` compares outputs, but the instance is x86-64 and the pinned omc writes AArch64
   binaries. Either copy the output files back and compare on the Spark (simplest), or build the
   compare with `target x86linux`, whose native x86 execution is itself NOT_RUN so far.
3. `ze_peak` and `ze_bandwidth` come from github.com/oneapi-src/level-zero-tests. Build them on the
   instance at a pinned tag and record the tag and binary sha256 in the receipt (TOOL lines). They
   are Intel's tests, used as the reference, not CUDA++ code.

Until 1 and 2 land, the candidate rows stay `MISSING_IMPLEMENTATION` in the plan file and in every
W0 receipt.

## Teardown

Copy the run directory back, check `SHA256SUMS` on the Spark, terminate the instance in the console
(turn off Termination Protection first if it was set), confirm it is gone, and close the receipt with
`collect.sh close`. Delete the uploaded SSH key from the account when the evaluation ends.

## If no suitable GPU is offered

Record the exact catalog (or the rejection) on DRA-206 and keep the rows `BLOCKED_ACCESS`. Do not
substitute a Xeon, Core Ultra CPU or Gaudi result for the GPU gate. Alternatives for DRA-201's one
non-NVIDIA device: the AMD packet (`AMD-DEVELOPER-CLOUD.md`) and the Apple M-series lane (DRA-195).
Buying hardware or paid cloud time needs Drake's written approval.
