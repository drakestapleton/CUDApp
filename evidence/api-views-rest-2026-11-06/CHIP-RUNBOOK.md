# Chip runbook — NOT_RUN

This branch is not ready for chip qualification. Complete RESUME.md's host acceptance first. This runbook grants no authority to load a model, touch a device, take a lock, stop a service or run a chip test.

1. Integrator4 finishes the migration and records the exact CUDApp commit, pinned omc hash and bridge revision. Final isolated host differential, mutants, simulator targets and median-of-three benchmark must pass first.
2. Obtain Drake's current exclusive GPU-window authorization and follow the live gate/runbook for that window. Historical receipts, a free lock file and this document are not authorization. Do not disturb :18091, :18173–18178, PID 68451, /tmp/aien-gb10.lock, NVIDIA modules, VFIO or devices from this lane.
3. An authorized operator builds the existing test binaries with at most -j3 and records their exact paths and hashes. The source-backed existing modes are `gpu_tcgemm_test --chip`, `gpu_pfgemm_test --chip` and `gpu_tgemm_test --chip`. `gpu_fp8gemm_test` is a chip test by default; `--host-only` and `--sim` select the non-chip tiers. Do not accidentally run that binary without a tier selector during host acceptance.
4. The operator invokes those chip modes only through the currently authorized hardware runner and its mandatory quiet-window/device gates. This lane supplies no device-isolation bypass or replacement lock procedure. Host/simulator checks remain device-isolated with `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.
5. Collect full stdout/stderr, command, exit status, source/binary pins and final verdict for each family. Keep existing wrong-kernel/poison/mutant controls enabled; a mutant simulator error is not a killed semantic mutant. Compare default Omega and gated C only under the operator's authorized protocol, with -ffp-contract=off and NaN class handling.
6. Store fresh dated evidence without changing historical receipts. Record chip, live-RM and VFIO verdicts separately. A host or simulator PASS cannot qualify chip behavior. Restore/release only resources acquired by the authorized runner, following that runner's procedure.

Current verdicts: chip NOT_RUN; live-RM NOT_RUN; VFIO NOT_RUN.
