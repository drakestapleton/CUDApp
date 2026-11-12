# gfx942 executor chain on the reviewed KFD layer: integration receipt

Source: the files in `inputs.sha256` as committed at `55a438ec2fad558adb7d7ff9cb29705a5f474bd6` (CUDApp branch `amdserve/gfx942-exec-chain`): the
executor, fsum16, SGEMM, the CDNA3 semantics fix, the KFD refusal cases, FP8 GEMV and the loop-bound fix, rebased onto
the reviewed KFD head `52f3bc35`. This receipt replaces `AMD-GFX942-CHAIN-20261112T223225Z`, which was taken before the loop-bound
fix. The per-slice receipts (`AMD-GFX942-EXEC-*`, `-SGEMM-*`, `-SGEMM-SEMANTICS-*`, `AMD-KFD-REVIEW-*`,
`-FP8GEMV-*`) record each slice on its own branch before the rebase; this receipt is the one that matches this tree.
Compiler: omega-mv `e5b32567` `seed/out/omc`, sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Host: spark-b87b, Linux AArch64, 20261112T230144Z. No device was opened.

Commands, from the CUDApp root, under `flock ~/handoffs/hive/queen/pool-8-9.lock nice -n 10 taskset -c 8,9`:

```sh
AMD_EXEC_OUT=OUT/e sh tests/amd-exec/run.sh "$OMC"  # amd-exec.log
AMD_OUT=OUT/h sh tests/amd/run.sh "$OMC"            # amd-host.log
AMD_KFD_OUT=OUT/k sh tests/amd-kfd/run.sh "$OMC"    # amd-kfd.log
MT_OUT=OUT/m sh tests/multitarget/run.sh "$OMC"     # multitarget.log
```

All four exited 0. Final lines:
- `AMD-GFX942-HOST PASS pass=50 fail=0 kernels=vadd,fsum16,sgemm,fp8gemv hardware=NOT_RUN`
- `AMD-GFX942-EXEC PASS kernels=vadd,fsum16,sgemm,fp8gemv counts=0,1,63,64,65,200 sgemm_shapes=14 sgemm_cases=29 semantics_cases=350 fp8gemv_shapes=21 fp8gemv_cases=43 fp8_conversion_bytes=256 hardware=NOT_RUN`
- `AMD-KFD-HOST PASS pass=23 fail=0 not_run=2`
- `MULTITARGET PASS pass=22 fail=0 not_run=4`

Hardware, separately and not from this tree: the vadd, fsum16, sgemm and fp8gemv objects this tree produces
(digests in amd-exec.log) ran bit-exact on an AMD Instinct MI300X VF through libamdhip64 (AMD runtime as loader)
in the notebook receipts AMD-MI300X-NOTEBOOK-20261112T223000Z, -223800Z and -224000Z, filed in
~/handoffs/hive/amdserve/evidence on the Spark. Not run from this tree: the Omega launcher, ROCr loading from
Omega, NaN payloads, Qwen 27B.
