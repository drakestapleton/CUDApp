# gfx942 executor chain on the landed KFD layer: integration receipt

Source: the files in `inputs.sha256` as committed at `9c166cc8e77961582927bedea1670679981381c6` (CUDApp branch `amdserve/gfx942-exec-chain`): the
executor, fsum16, SGEMM, the CDNA3 semantics fix, the KFD refusal cases and FP8 GEMV, rebased onto the reviewed KFD
head `52f3bc35`. The per-slice receipts in `evidence/AMD-GFX942-EXEC-*`, `-SGEMM-*`, `-SGEMM-SEMANTICS-*`,
`AMD-KFD-REVIEW-*` and `-FP8GEMV-*` record each slice on its own branch before this rebase; the rebase merged
`tests/amd/run.sh` and `tests/amd-kfd/oracle.py` with the S1/S2 review fixes, so this receipt is the one that
matches this tree.
Compiler: omega-mv `e5b32567` `seed/out/omc`, sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Host: spark-b87b, Linux AArch64, 20261112T223225Z. No device was opened.

Commands, from the CUDApp root, under `flock ~/handoffs/hive/queen/pool-14.lock nice -n 10`:

```sh
AMD_OUT=OUT/h sh tests/amd/run.sh "$OMC"            # amd-host.log
AMD_EXEC_OUT=OUT/e sh tests/amd-exec/run.sh "$OMC"  # amd-exec.log
AMD_KFD_OUT=OUT/k sh tests/amd-kfd/run.sh "$OMC"    # amd-kfd.log
MT_OUT=OUT/m sh tests/multitarget/run.sh "$OMC"     # multitarget.log
```

All four exited 0. Final lines:
- `AMD-GFX942-HOST PASS pass=48 fail=0 kernels=vadd,fsum16,sgemm,fp8gemv hardware=NOT_RUN`
- `AMD-GFX942-EXEC PASS kernels=vadd,fsum16,sgemm,fp8gemv counts=0,1,63,64,65,200 sgemm_shapes=14 sgemm_cases=29 semantics_cases=350 fp8gemv_shapes=21 fp8gemv_cases=43 fp8_conversion_bytes=256 hardware=NOT_RUN`
- `AMD-KFD-HOST PASS pass=21 fail=0 not_run=2`
- `MULTITARGET PASS pass=22 fail=0 not_run=4`

Not run: every AMD hardware row, ROCr loading, dispatch, SH_MEM_CONFIG FP8 mode, NaN payloads, speed, Qwen 27B.
