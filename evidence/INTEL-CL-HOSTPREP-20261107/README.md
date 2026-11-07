# INTEL-CL-HOSTPREP-20261107: Intel GPU W1 host preparation (DRA-194), Spark CPU only

Kanban t_3bbd5f48, CUDApp branch `hive/t_3bbd5f48-intel-cl-gpu`. Tested commit
`56ffb3a06ca0c0c5827420358e26570e059be6f7` (clean tree), stacked on `hive/t_ebf8bda5-portable-nv-apple`
(`bec13796`, which carries apple195's `33f15580`) with `hive/t_e308e9c5-access-packets` (`309e6d98`) merged in
at `9b594462`.

**Labels.** IMPLEMENTED and EXERCISED on the Spark host CPU (Linux AArch64):
- the `MT_TARGET=3` OpenCL GPU mode of `tools/multitarget/spirv-host.c` (refusal paths only);
- `tools/access/intel-cl.sh` (bundle, the row commands up to the ICD refusal, the Spark-side compare);
- the new rows of `tools/access/plans/intel-gpu.tsv`.

TESTED: both host gates are green. **Intel GPU: NOT_RUN.** There is no Intel Cloud Services instance;
sign-up is Drake's (DRA-206). No row in this directory is a GPU result. Native x86 execution is NOT_RUN:
no x86 build of conform.om was made, because the compare runs on the Spark.

## Results

| Gate | Base tree `9b594462` (the two parent branches merged, before this change) | This branch `56ffb3a0` |
|---|---|---|
| `tests/multitarget/run.sh` (PoCL 7.2 + lavapipe, cores 10-11) | `MULTITARGET PASS pass=61 fail=0 not_run=2` | `MULTITARGET PASS pass=78 fail=0 not_run=2` |
| `tests/access/run.sh` | `ACCESS GATE PASS pass=52 fail=0` | `ACCESS GATE PASS pass=86 fail=0` |

The 17 new multitarget checks:
- `MT_TARGET=3` builds without Vulkan.
- No opt-in, `MT_CL_GPU=0` and `MT_CL_GPU=yes` each exit 2 and write no output.
- `vk` is refused (2).
- No ICD and a relative ICD are refused (3).
- An ICD path naming `nvidia`, and the PoCL library, are refused before loading (9).
- An ICD that is a directory, an `.icd` descriptor or a non-ELF file is refused (3). strace shows that
  the mock driver these name is never looked for. Without the check (mutant m4) the loader does look
  for it.
- A copy that allows PoCL reaches the pinned PoCL platform and is refused because it has no GPU device
  (5, "no CPU fallback"). strace of that run shows PoCL opened and no `libnvidia`, `libcuda` or
  `/dev/nvidia` path.
- The default build with `MT_CL_GPU=1` still runs on the PoCL CPU device, and its bytes equal `vadd.ref`.

All earlier checks pass as before: vadd, fsum16 and gemm16 bit-exact on PoCL and lavapipe, mutants
caught, isolation controls. The 2 NOT_RUN are the existing `hello_gpu --sim` cross-check and the HSACO
oracle; their tools are not on this host.

The 34 new access checks:
- The 11 W1 rows exist with their kind and want, and run `intel-cl.sh`. No `MISSING_IMPLEMENTATION`
  row is left.
- The collector exports `ACCESS_OUT` and hashes files that rows keep there.
- The bundle holds 16 files plus `SHA256SUMS`.
- The launcher arguments keep a bundle path with spaces and `[x]*` whole, for vadd, fsum16 and a
  gemm16 mutant (`argv-vadd-spaced-bundle.txt`).
- On this GPU-less host, the real plan's candidate and control rows, with the spaced bundle, record
  11 rows, none PASS, and no Intel GPU device (`real-plan-on-spark.*`). The compare fails them
  (`compare-real-plan.txt`).
- The row helper refuses an `.icd` descriptor.
- The compare on SYNTHETIC outputs passes 3 agreeing lines (`compare-good.txt`; all `dev=d7`,
  `pass_on_d0=0`).
- The compare fails a tampered copy, a VM PASS with wrong bytes, a control whose mutant output equals
  the reference, and correct bytes from a VM FAIL (`compare-lie.txt`).
- With nothing to compare it reports NOT_RUN (exit 3).
- rcheck still refuses that synthetic receipt (a candidate PASS on the host CPU).

## Negative controls on the tests themselves (`mutants/`, each one change of `56ffb3a0`)

| Mutant | Gate | Result |
|---|---|---|
| m1: `main()` opt-in check disabled | multitarget | `FAIL pass=75 fail=3`: the three opt-in refusals |
| m2: GPU enumeration widened to `CL_DEVICE_TYPE_ALL`, device-type check removed | multitarget | `FAIL pass=77 fail=1`: `cl3-neg-cpu-only-platform`. The exit is still 5, but for the wrong reason; the text check catches it |
| m3: compare trusts the VM verdict and ignores conform.om's exit | access | `FAIL pass=84 fail=2`: the two compare-lie lines |
| m4: regular-file, `.icd` and ELF checks on the ICD removed | multitarget | `FAIL pass=74 fail=4`: the three indirection refusals, plus strace showing the loader looked for the mock driver |
| m5: one bundle path in the vadd arguments unquoted | access | `FAIL pass=85 fail=1`: `argv vadd` (the path split into three arguments) |

## Independent review (`review/`)

Pre-PR desk-mode review by the Spark `reviewer` profile (Codex gpt-6-astra, reasoning medium) of head
`b7369f93`:
`VERDICT aien-dev/CUDApp#0 … verdict=MERGE_AFTER_FIX … session=local-8722acb4… findings=2`.

It found two blocking issues, both fixed in `56ffb3a0`:
- P1: ocl-icd reads a directory or an `.icd` file as an indirection. The launcher now requires a regular
  ELF library not named `*.icd`.
- P2: bundle paths were word-split. Arguments are now built directly.

Non-blocking points, also addressed:
- a cmp error now exits 72;
- the opt-in wording is fixed;
- refusal and mutant controls are distinguished;
- the compare's exit 0 is not a window verdict.

A `#0` review does not count for a merge. The desk needs the reviewer's verdict on the PR at its head.

## Lean model (`lean/`)

`IntelClGate.lean` models the `MT_TARGET=3` exit decision, including the ELF-library check, and one
compare line. It is hand-copied from the C and the script. PROVED with no axioms:
- an output is written only with the opt-in, a GPU device, a clean ICD path, an ELF library ICD and
  both isolation scans;
- with no opt-in the exit is always 2;
- a compare PASS needs conform.om to agree and a VM PASS.

Both negative controls (no GPU check; a compare that trusts the VM alone) are proved to violate the
property. The proof covers the model only. The compiled launcher is what the gates above exercise.

## Environment

`spark-final/identity.txt`:
- Linux 7.0.0-1019-nvidia aarch64, gcc 13.3.0;
- omc `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`;
- PoCL 7.2 (`libpocl.so.2.16.0` `38c77fcc…`, LLVM 18.1.3), an existing Spark build copied into the lane;
- spirv-val `4e102aff…`, Mesa lavapipe, strace 6.8.

Cores 10-11 under `pool-10-11.lock`. No GPU was opened: the launcher's own isolation scan and the
strace checks show no NVIDIA path.

## Not run

- Every Intel GPU row (W0 and W1): NOT_RUN, no instance.
- `MT_TARGET=3` past device selection: NOT_RUN. That covers the context, the build on the GPU driver,
  profiling, isolation with the ICD mapped, and `dri_fds`. It needs a GPU-typed OpenCL device; the
  dispatch code is shared with the CPU path.
- `MT_TARGET=1` built on macOS: NOT_RUN here. A Linux `-fsyntax-only` check with a stub
  `mach-o/dyld.h` passes for targets 0 to 3.
- `MT_TARGET=2` on the GB10: NOT_RUN (syntax only, no window).
- `make test`: NOT_RUN (no `HOST_TESTS` input changed).
- Not changed: `MT_TARGET=0` pins PoCL through the same `OCL_ICD_VENDORS` mechanism, with a compile-time
  path. It checks the platform name only after loading. That is pre-existing, and left as it was to keep
  target 0's behaviour.

## Replay

```sh
# Spark, cores 10-11 under the pool lock; tools as in identity.txt
SPIRV_VAL=… LD_LIBRARY_PATH=… CUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 POCL_LIBRARY=<abs libpocl.so> \
  MT_CFLAGS=-I<CL+vulkan headers> LVP_ICD=/usr/share/vulkan/icd.d/lvp_icd.json MT_OUT=<dir> \
  taskset -c 10-11 nice -n 10 sh tests/multitarget/run.sh <abs>/omc     # MULTITARGET PASS pass=78 fail=0 not_run=2
taskset -c 10-11 nice -n 10 sh tests/access/run.sh <abs>/omc            # ACCESS GATE PASS pass=86 fail=0
```

Every file here is listed in `SHA256SUMS`. The gate logs have the home directory replaced by `$HOME`.

Signed: t_3bbd5f48 (Hermes worker, claude-opus-5-5, anthropic)
