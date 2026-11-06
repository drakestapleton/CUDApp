<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# NO_POSIX native cutover receipt (2026-11-06)

## Scope and custody

IMPLEMENTED and host EXERCISED. Chip QUALIFIED: NOT_RUN. MERGED: NOT_RUN.
DEPLOYED: NOT_RUN. No device was opened and no live service or GPU lock was touched.
This is a freestanding computation/binding qualification, not device qualification.

Base: `eceb8c542fa6` (CUDApp #330). First registration commit: `56c0fea`.
The complete base-revision macro inventory is [tests/NO_POSIX.md](../../tests/NO_POSIX.md).
Only the explicit encoder, ELF writer and bf16 C fallback computations are cut over.
Session command-list computation, fragment packers/sentinels and #327's base
functions remain outside this lane. Their migration status is not claimed here.

The encoder and ELF writer use the same pre-existing `.om` sources as hosted
portable builds, prefixed with `om2`, and compiled by the seeded compiler.
The new `om/numeric/no_bf16.om` implements the integer rounding path.
`om/units.sh --native-no-posix` builds all three through `tools/native --freestanding`.
`CUDAPP_MIGRATION_PENDING_C_NO_POSIX` is default OFF and selects the C primary
oracles. Existing hosted per-unit differential macros remain default OFF.
No public ABI, compiler syntax, gate policy or host word changes were made.
The matching build/registration guide is `tests/NO_POSIX.md`; no omega-harness
language/compiler guide or compiler pin changes are required by this cutover.

Final native bridge: combined #76, `741c1ebb2b0fbae311a77ad2118b13d5ec7a17ee`,
selected by Main through Integrator4. Earlier #75 receipts are retained as
historical attempts, not as final stage-5 custody. Registry pins:

- `tools/native`: `14b2b51ed062ebaa7ea99e83752cc075452465b9ddba4939eb171692358ecb6b`
- `src/native.om`: `e2077722aaeffb1121b70160e22357ea47f369e0039ab1ea06dc700f6f4e45d7`
- Unit compiler: omega-mv `e5b32567`, SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`
- Generator bootstrap compiler required by the bridge: SHA-256 `734f0a39319fbfa930bcc62cb30dbf06d47d78e126372b74f5baf478e6126b61`
- GCC: Ubuntu `13.3.0-6ubuntu2~24.04.1`, version `13.3.0`

The two compilers have distinct jobs: `OMEGA_NATIVE_OMC` bootstraps the bridge
generator; `OMEGA_NATIVE_OMC2` is forced to the assigned `OMEGA_OMC` for all
computation units. Generated C is bounded ABI/memory binding, not handwritten
computation. Existing scalar f16 already uses its import-free seeded binding.

## Commands and observed results

Native environment used:

```sh
export OMEGA_NATIVE_OMC=/tmp/no-posix-baseline-omc
export OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc
export OMEGA_BRIDGE=/tmp/bridge-pin-merge
```

Every test, red control, mutant and child test binary was inside:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 COMMAND
```

| Command (inside wrapper) | RC | Observed result | Receipt |
|---|---|---|---|
| `make -j3 gpu-engine-freestanding test-gpu-engine-freestanding test-codegen-freestanding` | 0 | Engine 50/0; platform 7/0; malloc canary refused; 14 fixture dumps, 24448 code bytes identical; exhaustive bf16 sweep passed; final `PASS: freestanding codegen contract (14 fixtures, 0 failing checks)` | `required-final.log` |
| `make -j3 test-no-posix-native` | 0 | Import-free static ELF; same hosted/native result bytes; red exit 90; six source mutants killed; C encoder dump and goldens equal native; cubin image/refusal differential passed | `target-final-native.log` records every child command/RC |
| `make -j3 test-no-posix-native test-gpu-native-runtime` | 0 | Focused native gate passed; runtime consumer reported `test_omega_gpu_session_platform: 27 passed, 0 failed`, explicitly HOST_ONLY/synthetic | supplementary consumer run, same device-free wrapper |
| `sh om/units.sh --check` with portable bridge checkout `~/work/cudastack/omega-bridge` matching `om/omega-bridge.lock` | 0 | `PASS portable Omega units regenerate byte for byte` | `units.log`; rerun after dispatcher addition with same result |
| `make -j3 build/no_posix/native/omega_api_defs.o` after removing only that generated object | 0 | Regenerated both native objects; grouped-output rule recovers partial output | `rebuild.log` (initial native composition) |

The partial link retains exactly the existing allowed undefined symbols:
`memcmp memcpy memset strcmp strlen`. Its malloc canary is refused.
The new static bare AArch64 ELF has **no undefined symbols**, no interpreter,
no `.tdata` and no `.tbss`. No new libc dependency is admitted.

The cubin positive sweep reports 854 calls, 740 images, 111037984 image bytes;
ARG/TEXT/PARAMS/CBANK/GPR/SHARED refusal classes are all reached. Its separate
explicit-target fixture covers sm_86 and sm_121 successes plus target
`0,85,87,120,122,UINT32_MAX` refusals and a null parameter-array refusal.
The C-vs-Omega differential compares complete images and diagnostic strings,
not a checksum approximation. A flipped image byte aborts with signal 6.

The bare fixture covers encoder empty/padded output, no-padding output,
invalid opcode, short-view status and untouched result sentinel; native bf16
covers zero, negative zero, exact values, both tie parities, infinity and NaNs.
The required codegen gate additionally compares **every f32 bit pattern** to
the hosted `__bf16` cast. The source's quiet-NaN payload convention is preserved.

Killed `.om` mutants: NOP word (exit 2), NOP control (2), padding floor (1),
byte-size multiplier (1), NaN quiet bit (7), RNE tie parity (7).
The generated wrong-result red ELF exits 90 instead of success.

## Coverage and unsuccessful attempts

C oracles ran under bwrap with `--coverage -O0` and `-ffp-contract=off`.
The final gcov summaries in `target-final-native.log` report:

- `omega_blackwell_codegen.c`: 88.40% lines, 90.84% branches executed,
  53.86% branches taken at least once; this file includes other lanes' functions.
- `omega_cubin.c`: 93.85% lines, 96.35% branches executed,
  78.47% branches taken at least once.

[coverage-exceptions.md](coverage-exceptions.md) enumerates every uncovered
branch's source-line set with a justification. No full branch-coverage claim
is made. Allocator exhaustion and corrupted native status/detail shapes are
not synthesized by adding production hooks.

An initial supplemental oracle invocation used hosted `--golden` on a
NO_POSIX binary. That binary's existing argument contract instead runs the
exhaustive bf16 sweep unless passed `--no-sweep`; at `-O0` this timed out.
That attempt is INCONCLUSIVE, not PASS (`combined-gates.log`, composite RC 2
after terminating the owned oracle). A second overlapping Eval was interrupted
by the first Eval's timeout. Only this lane's exact orphan PIDs were stopped.
The focused harness now uses the existing `--no-sweep` mode for supplementary
buffer/golden coverage. The required `test-codegen-freestanding` target is
unchanged and its exhaustive sweep still runs and passes in `required-final.log`.
All corrected gates were rerun after the process cleanup. No process is retained.

Earlier `gates.log`, `native.log`, `native-final.log` if present, and
`combined-corrected-native.log` document intermediate compositions/cases.
`required-final.log` and `target-final-native.log` are the final acceptance
receipts. Home prefixes in exported textual logs are normalized to `~`; no
result values are changed. Binary test return records are hex-framed in logs.

## Chip runbook (NOT_RUN)

1. Obtain the current operator-approved exclusive chip window through the
   existing Spark authorization procedure. A free advisory lock is not authority.
   Do not stop or alter any live model service under this lane's authority.
2. Pin the integrated CUDApp head, combined bridge revision and both compiler
   digests above. Repeat these host gates inside the device-free wrapper first.
   Preserve the native `work/` identities and generated object digests with the
   operator's chip receipt; do not commit cubins or NVIDIA binaries.
3. In the approved hardware harness, link `build/gpu_fs/omega_gpu_freestanding.o`
   with its real ABI-1.1 `OmegaGpuPlatform`. Require explicit platform/clock
   installation; never treat the synthetic runtime test as a real platform.
   Encode the same 14 fixture descriptors through
   `omega_blackwell_kernel_encode_into` into caller-owned buffers. Compare their
   bytes and declared-register metadata to the frozen hosted fixture dumps
   before any submission. Package through the same native `omega2_cubin_elf`
   unit using platform-owned scratch memory, not the hosted malloc shell.
4. Launch the admitted canary/matmul fixture through the approved platform's
   existing launch path. Compare output bits and completion markers to the
   hosted reference; retain code digest, input/output digests, return status,
   platform identity and device/window identity in a new chip receipt.
   The invalid-opcode and invalid-target controls must refuse before submit;
   mutated output must be rejected before a GPU launch, not executed as SASS.
5. A host match, successful ELF packing or synthetic session is not hardware
   success. Missing real platform installation, missing authorization or any
   skipped launch stays NOT_RUN. This lane performs no hardware launch.

## Remaining C and integration boundary

C remains for native entry/ABI marshalling, buffers, pointer ownership,
allocation/diagnostic shells and syscall/platform guards from the inventory.
The inline bulk bf16 helper remains for its existing other callers and the
explicit C oracle; default freestanding scalar conversion no longer calls it.
The allocating cubin shell is not an allocator-free firmware API and is not
part of the GPU partial link. The native unit itself is import-free.

No compiler/bridge extension is required. The new native roots are registered
through a separate mode because the normal portable registry emits committed
portable C, whereas native custody emits caller-stack machine-code bindings.
Shared integration files: `om/units.sh`, `mk/omega-gpu-engine.mk`, encoder and
cubin selection blocks, matmul conversion and the matching comments/docs.
SessionCmdlist2 was notified that its mutant harness must include
`$(GPU_NO_POSIX_OBJS)` alongside `$(GPU_FS_OBJS)`; it confirmed that change.
Full `make test`, chip, live RM/VFIO, firmware integration and deployment are
NOT_RUN here; the integrator owns the combined full gate.

## Wolfram queries

- `Integrate[x^2,{x,0,1}] -> 1/3` (evaluator verification).
- WolframAlpha `sqrt(27)/2 -> (3 sqrt(3))/2` (second tool verification).
- `FromDigits[#,16]& /@ {"7f800000","007fffff","0040","7fff"}`
  `-> {2139095040,8388607,64,32767}`.
- `v=FromDigits[#,16]& /@ {"00000000","80000000","3f800000","3f808000","3f818000","7f800001","ffffffff","7f800000"}; {v,(If[BitAnd[#,2139095040]==2139095040 && BitAnd[#,8388607]!=0,BitOr[BitShiftRight[#,16],64],BitShiftRight[Mod[#+32767+BitAnd[BitShiftRight[#,16],1],2^32],16]]& /@ v)}`
  `-> {{0,2147483648,1065353216,1065385984,1065451520,2139095041,4294967295,2139095040},{0,32768,16256,16256,16258,32704,65535,32640}}`.
- `{86+{-1,0,1},121+{-1,0,1},2^32-1}`
  `-> {{85,86,87},{120,121,122},4294967295}`.

These are reference calculations. Test counts/coverage/byte counts quoted
above are measured outputs, not modeled hardware performance.

## Addendum: inherited session-mutant consumer

The stacked base's legacy session-mutant link also consumes `GPU_FS_OBJS`.
It now links `GPU_NO_POSIX_OBJS`, keeping this draft self-contained before the
session lane's replacement recipe lands. SessionCmdlist2 agreed to this
one-line shared-file change; its #334 recipe already carries the same objects.
No session source or mutant semantics changed.

`make -j3 test-gpu-session-mutants` under the same wrapper and combined bridge
environment returned 0: **11 of 11 mutants killed**. Exact output is recorded
in [session-mutants.log](session-mutants.log). On integration retain #334's
replacement `SESSION_FS_OBJECTS='$(GPU_FS_OBJS) $(GPU_NO_POSIX_OBJS)'` recipe
rather than the superseded base recipe.
