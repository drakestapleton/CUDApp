# NO_POSIX API-view BF16 — 2026-11-06

IMPLEMENTED and EXERCISED on host CPU. Chip QUALIFIED: NOT_RUN. Not merged or deployed.

Base: #323 `64fd424e757c`; dependency #335 `ee886e852b490e5cd44ef3a09e91b83c67942dab`, merged here as `3cec7e21`. The separate session/packet migration is #348, not part of this branch.

## Boundary

The existing `src/omega_gpu_apiviews.h:av_bf16_n` is the only definition site. Hosted callers retain canonical `om/apiviews/bf16.om`. Under `OMEGA_GPU_NO_POSIX`, view transport reads each float's raw bits and calls the strong `omega2_no_avbf` export, compiled by `om/no-posix.sh --freestanding`. No float arithmetic, rounding, NaN policy or packet computation is delegated to hand-written C. Conversion and both mode policies are implemented in `om/apiviews/no_avbf.om`.

The canonical hosted public ABI has floating views, outside the pinned native frontend's public subset (direct native compilation refuses it with rc 110). The freestanding variant copies its integer `av_rne`/`av_stbf` computation with a scalar integer-public ABI. The gate compares the canonical generated hosted view binding, native-hosted scalar binding, and freestanding scalar binding, not two copies of the C oracle.

`CUDAPP_MIGRATION_PENDING_C_NO_POSIX_SESSION=1` explicitly selects the old C BF16 oracle. Default is 0, including GPU_FS_CFLAGS. The oracle's mode argument now retains the canonical ulong mode width, rather than truncating a nonzero mode to zero. Nonzero mode values above UINT32_MAX are exercised. Unexpected native refusal traps without importing abort/stdio.

## Pins and execution isolation

- omc: omega-mv `e5b32567`, SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- native bridge: `741c1ebb`; frontend SHA-256 `14b2b51ed062ebaa7ea99e83752cc075452465b9ddba4939eb171692358ecb6b`; generator SHA-256 `e2077722aaeffb1121b70160e22357ea47f369e0039ab1ea06dc700f6f4e45d7`.
- Baseline bootstrap compiler: `/tmp/no-posix-baseline-omc`; native bridge tree `/tmp/bridge-pin-merge`.
- Every gate below ran under `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`. No real NVIDIA, VFIO or CUSE device is visible. No live GPU services, driver modules or GPU locks were touched.

## Commands actually exercised

From this checkout, prefix the first command with the isolation command above:

```sh
env OMEGA_NATIVE_OMC=/tmp/no-posix-baseline-omc \
  OMEGA_BRIDGE=/tmp/bridge-pin-merge \
  OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc \
  make -j3 test-no-posix-bf16 test-no-posix-native
```

rc 0. BF16 final line:

```text
PASS no-posix BF16: canonical/native hosted/freestanding differential; symbol OFF/ON red; 6 killed mutants
```

Existing native gate final line:

```text
PASS C_NO_POSIX encoder oracle dump equals native dump; gcov recorded
```

That gate also passed bare-ELF no-import/interpreter/TLS checks, hosted bytes, explicit red exit 90, six existing encoder/BF16 mutants, cubin shell differential and injected-byte red control. The production freestanding GPU aggregate linked with only the preexisting permitted string/memory imports. New native objects combined have zero undefined imports.

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  make -j3 -f mk/apiviews.mk test-apiviews apiviews-red
```

rc 0. Observed `PASS apiviews host 93 checks` and expected `RED CHECK: selftest rc 134` following the deliberate byte mismatch.

## Partitions, controls and mutants

`tests/no_posix_bf16.c` sweeps every upper 16-bit prefix (all signs/exponents/payload prefixes), crossed with lower halves 0, 1, 32767, 32768, 32769, 65534, 65535. This covers zero/MAX and neighboring values, subnormal/normal/infinity/NaN boundaries, payload/sign preservation, both sides of every rounding tie and both parity arms. Modes: 0, 1, 2, UINT32_MAX, 4294967296, UINT64_MAX. Empty null views are included; output canaries check exact view extent. This is a source-derived partition sweep, not an exhaustive enumeration of all f32 bit patterns.

Production `omega_gpu_matmul_api.c` is compiled at O0 under NO_POSIX twice. `nm` plus `objdump -dr` prove flag OFF references `omega2_no_avbf`, with no `av_bf16_c` or hosted `omega_av_bf16`; flag ON contains/calls `av_bf16_c` and does not reference `omega2_no_avbf`. Removing native objects causes an undefined `omega2_no_avbf` link failure. Gate-generated dumps, nm/disassembly, and refusal stderr live under `build/no_posix_bf16/` and are reproducible, not committed bulk artifacts.

All six new mutants compiled to freestanding native objects and were killed by output disagreement with canonical hosted behavior (not compile failure): round-bias, tie-parity, quiet-bit, nan-payload, canonical-nan, mode-select. The C ON control agrees with canonical behavior.

## Remaining hardware runbook

Chip tier NOT_RUN: no current exclusive GPU authorization. In an explicitly authorized window, integrate the #323 and #335 dependencies plus this branch; rebuild production NO_POSIX objects with the flag at its default 0; rerun both host gates above inside the device-hidden wrapper; inspect the production object symbol evidence; then use the established platform/session upload-and-matmul chip runbook with finite, tie, subnormal, infinity and NaN payload BF16 inputs. Compare uploaded BF16 bytes with the canonical hosted fixture and run an authorized incorrect-conversion negative control. Record device/window/source pins and chip receipts separately. Host/simulator PASS is not hardware qualification. Do not stop or replace live services outside the current operator authorization procedure.
