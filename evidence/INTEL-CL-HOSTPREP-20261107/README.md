# INTEL-CL-HOSTPREP-20261107: Intel GPU W1 host preparation (DRA-194), Spark CPU only

Kanban t_3bbd5f48, CUDApp branch `hive/t_3bbd5f48-intel-cl-gpu`. Tested commit
`e3561855610578be9da9d3f0f98560b253ce3a1a` (clean tree), stacked on `hive/t_ebf8bda5-portable-nv-apple`
(`bec13796`, which carries apple195's `33f15580`) with `hive/t_e308e9c5-access-packets` (`309e6d98`) merged in.

**Labels.** IMPLEMENTED and EXERCISED on the Spark host CPU (Linux AArch64): the `MT_TARGET=3` OpenCL GPU mode
of `tools/multitarget/spirv-host.c` (refusal paths only), `tools/access/intel-cl.sh` (bundle, row commands up
to the ICD refusal, Spark-side compare) and the new rows of `tools/access/plans/intel-gpu.tsv`. TESTED: both
host gates green. **Intel GPU: NOT_RUN** (no Intel Cloud Services instance; sign-up is Drake's, DRA-206). No
row of this directory is a GPU result. Native x86 execution: NOT_RUN (no x86 build of conform.om was made; the
compare runs on the Spark).

## Results

| Gate | Base tree `9b594462` (the two parent branches merged, before this change) | This branch `e3561855` |
|---|---|---|
| `tests/multitarget/run.sh` (PoCL 7.2 + lavapipe, cores 10-11) | `MULTITARGET PASS pass=61 fail=0 not_run=2` | `MULTITARGET PASS pass=74 fail=0 not_run=2` |
| `tests/access/run.sh` | `ACCESS GATE PASS pass=52 fail=0` | `ACCESS GATE PASS pass=82 fail=0` |

The 13 new multitarget checks: `MT_TARGET=3` builds without Vulkan; no opt-in, `MT_CL_GPU=0` and
`MT_CL_GPU=yes` exit 2 with no output; `vk` refused (2); no ICD and a relative ICD refused (3); an ICD path
naming `nvidia` and the PoCL library refused before loading (9); a copy that allows PoCL reaches the pinned
PoCL platform and is refused for having no GPU device (5, "no CPU fallback"); strace of that run shows PoCL
opened and no `libnvidia`, `libcuda` or `/dev/nvidia` path; the default build with `MT_CL_GPU=1` still runs
on the PoCL CPU device and its bytes equal `vadd.ref`. Every earlier check (vadd/fsum16/gemm16 on PoCL and
lavapipe bit-exact, mutants caught, isolation controls) passes as before. The 2 NOT_RUN are the existing
`hello_gpu --sim` cross-check and the HSACO oracle (tools not on this host).

The 30 new access checks: the 11 W1 rows exist with kind and want and run `intel-cl.sh`; no
`MISSING_IMPLEMENTATION` row is left; the collector exports `ACCESS_OUT` and hashes files rows keep there;
the bundle holds 16 files with `SHA256SUMS`; the real plan's candidate and control rows on this GPU-less
host record 11 rows, none PASS, no Intel GPU device (`real-plan-on-spark.*`), and the compare fails them
(`compare-real-plan.txt`); the compare on SYNTHETIC outputs passes 3 agreeing lines (`compare-good.txt`,
all `dev=d7`, `pass_on_d0=0`) and fails a tampered copy, a VM PASS with wrong bytes, a control whose
mutant output equals the reference, and correct bytes from a VM FAIL (`compare-lie.txt`); nothing to
compare is NOT_RUN (exit 3); rcheck still refuses that synthetic receipt (candidate PASS on the host CPU).

## Negative controls on the tests themselves (`mutants/`)

| Mutant (one change) | Gate | Result |
|---|---|---|
| mut1: `main()` opt-in check disabled | multitarget | `FAIL pass=71 fail=3`: the three opt-in refusals |
| mut2: GPU enumeration widened to `CL_DEVICE_TYPE_ALL`, device-type check removed | multitarget | `FAIL pass=73 fail=1`: `cl3-neg-cpu-only-platform` (exit still 5, but for the wrong reason; the text check catches it) |
| mut3: compare trusts the VM verdict, ignores conform.om's exit | access | `FAIL pass=80 fail=2`: the two compare-lie lines |

## Lean model (`lean/`)

`IntelClGate.lean`: the `MT_TARGET=3` exit decision and one compare line, hand-copied from the C and the
script. PROVED with no axioms: output only with the opt-in, a GPU device, a clean ICD path and both
isolation scans; no opt-in always exits 2; a compare PASS needs conform.om's agreement and a VM PASS. The
two negative controls (no GPU check; VM-only compare) are proved to violate the property. Model only: the
compiled launcher is what the gates above exercise.

## Environment

`spark-final/identity.txt`: Linux 7.0.0-1019-nvidia aarch64, gcc 13.3.0, omc
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`, PoCL 7.2 (`libpocl.so.2.16.0`
`38c77fcc…`, an existing Spark build copied into the lane, LLVM 18.1.3), spirv-val `4e102aff…`, Mesa lavapipe,
strace 6.8. Cores 10-11 under `pool-10-11.lock`. No GPU was opened: the launcher's own isolation scan and the
strace check show no NVIDIA path.

## Not run

- Every Intel GPU row (W0 and W1): NOT_RUN, no instance.
- `MT_TARGET=3` past device selection (context, build on the GPU driver, profiling, isolation with the ICD
  mapped, `dri_fds`): NOT_RUN, needs a GPU-typed OpenCL device; the dispatch code is shared with the CPU path.
- `MT_TARGET=1` build on macOS: NOT_RUN here (Linux `-fsyntax-only` with a stub `mach-o/dyld.h` passes for
  targets 0 to 3); `MT_TARGET=2` on the GB10: NOT_RUN (compiled `-c` only, no window).
- `make test`: NOT_RUN (no `HOST_TESTS` input changed).
- Independent reviewer verdict: NOT_RUN (desk).

## Replay

```sh
# Spark, cores 10-11 under the pool lock; tools as in identity.txt
SPIRV_VAL=… LD_LIBRARY_PATH=… CUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 POCL_LIBRARY=<abs libpocl.so> \
  MT_CFLAGS=-I<CL+vulkan headers> LVP_ICD=/usr/share/vulkan/icd.d/lvp_icd.json MT_OUT=<dir> \
  taskset -c 10-11 nice -n 10 sh tests/multitarget/run.sh <abs>/omc     # MULTITARGET PASS pass=74 fail=0 not_run=2
taskset -c 10-11 nice -n 10 sh tests/access/run.sh <abs>/omc            # ACCESS GATE PASS pass=82 fail=0
```

Every file here is listed in `SHA256SUMS` (the gate logs have the home directory replaced by `$HOME`).

Signed: t_3bbd5f48 (Hermes worker, claude-opus-5-5, anthropic)
