# CHIP.md — tools-om dagsim scheduler

Date: 2026-11-06. Lane: ToolsOm. Branch: feat/tools-om.

No GPU window is authorized for this lane. Do not take `/tmp/aien-gb10.lock`. Do not touch :18091, :18173–18178, PID 68451, the omega-tool services, the nvidia modules, or vfio.

## What this lane emits

`om/tools/dag_eval.om` is a host scheduler. It does not emit a kernel, a cubin, or PTX. A warp-sim run is not applicable. Chip, live-RM, and VFIO tiers are NOT_RUN because there is no authorized window and because this unit has no device path.

## If a later window is opened

Do not run this unit on the chip. The acceptance path is the host differential:

```
taskset -c 10-14 nice -n 10 make -j3 test-dagsim
```

The C oracle used for the generated corpus must be compiled with `-ffp-contract=off` and `-DCUDAPP_MIGRATION_PENDING_C_TOOLS`. Default C contraction is not the canonical result. Canonical arithmetic is non-contracting IEEE, one rounding per Omega operation.

`tools/interop/cubin_probe.c` and `compat/cuda/fixture/vadd_drv.c` link the CUDA driver ABI. They stay C. They are not a chip qualification of this lane. Do not launch them without a separate authorized window and SPARK_WINDOW.
