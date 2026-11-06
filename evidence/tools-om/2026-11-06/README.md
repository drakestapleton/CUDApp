# tools-om receipt, 2026-11-06

Lane ToolsOm. Branch `feat/tools-om`. Host only. Chip NOT_RUN. Warp-sim NOT_RUN: this unit emits no kernel.

## What moved

The dagsim post-resolve scheduler moved to `om/tools/dag_eval.om` and is the default. `ds_eval` packs the resolved model, calls `omega_dag_eval`, and unpacks. Parse, resolve, `rx_costmodel`, and printf stay in C. The old scheduler remains behind `CUDAPP_MIGRATION_PENDING_C_TOOLS` (default off) and is the differential oracle.

Generated binding: `src/cudapp_om_dag_eval.c`, declared in `src/cudapp_native.h`. Identity:

```
omega-ident 2 unit=dag_eval abi=bridge-seeded src=3b3f4530c365cb33a12c629bc76763921779eb96cda5bf57587b7866baeccf08 c=5864d515d6333e3d4592cf0cd27ca1c14bb60de2cff6098480552616a96449ce omc=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f converter=69d629df170a24ba0c2cb6d41dbfaed520e248e0d723236dd32abf0581524d84 frontend=23ff7a2356bb5d980060a4685a27ac46587013069b7d24b37c2873d7ee659b37 path=om/tools/dag_eval.om
```

`om/units.sh --check` with that omc pin: `PASS portable Omega units regenerate byte for byte`.

## Host evidence

`make -j3 test-dagsim` (taskset 10-14, nice 10):

- `dagsim_unit: 203 checks, 0 failed`
- `tests/dagsim/run_cli.sh`: every existing invocation ok, including the second-run byte-identical check
- `mutant_lifetime.sh`: killed `inverted_lifetime_masks_overflow`
- `mutant_predict.sh`: killed `predict_unsupported_despite_known` and `predict_refused_despite_known`
- `mutant_omega.sh`: five `.om` mutants killed (cycle check, capacity precheck, lifetime check, ledger check, makespan update). A one-byte flip of a passing makespan record failed `cmp`.

Generated corpus, seed 20261106, 1200 OEV files (200 each of chain, bandwidth, buffer, cycle, launch, barrier), Omega CLI against the C oracle built with `-O2 -ffp-contract=off -DCUDAPP_MIGRATION_PENDING_C_TOOLS`:

```
PASS n=1200 bad=0 kinds={'case_chain': 200, 'case_bw': 200, 'case_buf': 200, 'case_cycle': 200, 'case_launch': 200, 'case_barrier': 200}
```

The same corpus against a default-contraction C build mismatched on bandwidth counterfactuals. Rebuilding that oracle with `-ffp-contract=off` matched byte for byte, including the doubled-bandwidth fixture whose contracting build printed `1051000000` where the non-contracting build printed `1.051e+09`. Canonical semantics are the non-contracting result. The dagsim link line passes `-ffp-contract=off`.

## Tiers

| tier | verdict |
|---|---|
| host unit + existing CLI | PASS |
| host corpus, 1200, non-contracting oracle | PASS |
| omega mutants, 5, plus record flip | PASS |
| warp-sim | NOT_RUN (no kernel) |
| chip / live-RM / VFIO | NOT_RUN (no window; no device path) |
| full `make test` | NOT_RUN (integrator gate) |

## C that stayed C

- `tools/dagsim/dagsim.c` parse, resolve, and `rx_costmodel`. Resolve calls `rx_cm_predict`. Omega has no checked call into that C function. Extension: a named foreign-call host word with a pinned prototype, rejected unless the callee is in an allowlist. Until then the C wrapper packs resolved numbers and Omega schedules.
- `tools/dagsim/dagsim_cli.c` printf. The JSON formatter (`%.0f` / `%.6g`) stays in C so the CLI text is the C formatter applied to Omega's doubles.
- `tools/doctor/cudapp_doctor.c`. Program mode has `fopen`/`stat`/`read` (omega-mv@e5b32567 host_words). The tool was not ported. Its output is a formatted checklist over `/proc`, `/sys`, and `/dev`. A faithful port has to match that text byte for byte, including `uname`. Not done in this change.
- `tools/artifact_export.c`. Calls artifact and codegen APIs owned by other lanes. ArtifactOm confirmed no source overlap. Omega cannot call those C functions. Same foreign-call extension as resolve.
- `tools/ampere_families.c`, `tools/ampere_corpus.c`. Bodies call other lanes' generators and write corpus files. Same foreign-call limit. The corpus refusal predicate (`op > BW_IR_MUFU_LG2`, explicit FP8 marker) is a few comparisons and was not split out.
- `tools/own30b_decode_profile.c`. `--wrap=omega_gpu_session_launch` recorder with static token tables. Function mode has no globals. Profiler confirmed no overlap with `tools/prof/`. This file is the thin C ABI shell the convention allows.
- `tools/interop/cubin_probe.c`, `compat/cuda/fixture/vadd_drv.c`. Link `libcuda` (`cuModuleLoadDataEx`, `cuLaunchKernel`, and the driver API fixture). ARCH-0042: CUDA++ owns GPU execution. These stay C. They are not an Omega launcher.
- `tools/multitarget/spirv-host.c`. Already behind `CUDAPP_MIGRATION_PENDING_SPIRV_HOST` (default off). Limit already recorded in the file: no checked shared-library call (extension E2, ADR 0043, omega-mv#888).
