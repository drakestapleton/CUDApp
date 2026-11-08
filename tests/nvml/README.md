# NVML host gate

```sh
make test-nvml CUDAPP_OMC_HOST=1 OMEGA_OMC=/absolute/pinned/omc \
     OMEGA_BRIDGE=/absolute/bridge OMEGA_CABI_BRIDGE=/absolute/cabi-bridge
```

Make bootstraps `tests/components/proc.om`, `gate.om`, `mutants.om` and `abi.om` into `build/nvml_gate`. All gate logic is seeded Omega. Run from the repository root, CPU-only, in a device-isolated sandbox, one gate per worktree (scratch is `build/nvml/`). The compiler SHA256 is `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

- `OMEGA_BRIDGE` is the omega-bridge checkout for the portable units in `om/units.sh` (52 rows in `src/cudapp_om_units.ident` at this head); its frontend digest is read from `om/units.sh` (`FRONTEND_PIN`).
- `OMEGA_CABI_BRIDGE` is the omega-bridge#83 checkout (cabi-1 emitter, built with `tools/build.sh`), used only for `native/nvml/api.om`. Its `src/toc.om` and `tools/bridge-seeded` must match `CABI_CONVERTER_PIN` and `CABI_FRONTEND_PIN` in `om/omega-bridge-cabi.lock`. This second pin is temporary (Manager(Aien) D2; unify under Linear DRA-154).

## Steps, in order

1. Configuration and pins: compiler, `OMEGA_BRIDGE` frontend pin, both CABI pins.
2. Committed generated bindings for `nvml_step`/`nvml_errstr` regenerate byte for byte; their prototypes compile.
3. `host.om` (476 checks over `rmfake.om`) builds and passes; an altered error-string oracle is rejected (negative control).
4. `lib-c`, `lib-h` and `lib-map` regenerate `compat/nvml/libnvidia-ml.c`, `nvml.h` and `libnvidia-ml.map` byte for byte from `native/nvml/api.om`.
5. The generated `build/nvml/libnvidia-ml.so.1` is built and the C caller `nvml_test.c` (fake RM in `transport.c`) links against it: 476 checks.
6. CUDA header ABI: 55 facts read by `abi.om` from the DWARF types and macros of `cc -g3 -include HEADER` on `/dev/null` (no C source), equal for the generated `nvml.h` and NVIDIA's; a copy of the generated header with `gpuInstanceId` widened to 64 bits must give different facts (negative control). The 11 generated prototypes compile after NVIDIA's `nvml.h`; a copy with `nvmlDeviceGetName`'s length changed to `unsigned long` must fail to compile (negative control; the expected compiler error appears in the log).
7. Exports and imports: `nm -D --defined-only` shows exactly the 11 api.om functions as `T`, `readelf -d` shows soname `libnvidia-ml.so.1`, and every undefined symbol is on the allowlist (C runtime, `pthread_mutex_*`, `omega2_nvrm_{open_file,close_file,ioctl}`). An object that calls `ioctl` directly must be rejected (negative control).
8. All 22 mutants of the Omega units are caught, with the original failure/check counts. Mutation compilation and exec failure never count as a caught mutant.

Without the CUDA header (`/usr/local/cuda/include/nvml.h` or `CUDA_NVML_INCLUDE`), step 6 prints NOT_RUN and the other steps still run, but the gate never prints `CUDAPP-NVML HOST PASS`: it ends `CUDAPP-NVML HOST INCOMPLETE (abi and prototype comparison NOT_RUN)` and exits 77.

## Exits

| Exit | Meaning |
| --- | --- |
| 0 | `CUDAPP-NVML HOST PASS` |
| 77 | compiler, `OMEGA_BRIDGE` or `OMEGA_CABI_BRIDGE` not configured or unreadable, or the CUDA `nvml.h` is absent (NOT_RUN; never PASS) |
| 69 | a pinned digest differs (compiler, bridge frontend, CABI converter or CABI frontend) |
| 74 | a pin row is missing or duplicated in `om/units.sh` or `om/omega-bridge-cabi.lock` |
| 1 | rejected behavior: stale generated file, failed check, uncaught mutant, failed ORACLE check |

Make without configuration prints NOT_RUN and exits 2. Failure diagnostics are fail-fast. A trap in the model exits with its trap status; the gate reports it as a failure.

## ORACLE tools

`nvml_test.c`, `transport.c` and `transport.h` are the foreign C caller of the generated ABI and its fake transport: ORACLE inputs, not shipped, on no build or production path (line 1 of each says so). Their replacement by Omega needs the omega-bridge cabi-2 shapes (Omega-shaped exports and a foreign-call probe). Host tools used only for verification: the pinned omc and bridges, `cc` (GCC), `nm` and `readelf` (GNU Binutils), `sha256sum` for pins, `env` and the bridges' `tools/bridge-seeded` wrapper. Make's one-time bootstrap uses cat/chmod/mv/mkdir as the other recipes do. Versions and evidence: `evidence/NVML-365-2026-11-07/`.

NOT_RUN: concurrent ABI callers, live RM/hardware qualification, and final-head CI while aien-dev Actions is externally blocked.
