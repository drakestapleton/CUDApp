# AMD KFD and AQL host data layer: host-only receipt

Source: CUDApp branch `amdserve/kfd-aql` at `2fd6b851d1276f924579698f2fa040b90db62b39` (sources in `inputs.sha256`).
Compiler: omega-mv `e5b32567a7aff3ce594e3254a622a846a6cd190b` `seed/out/omc`, sha256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f` (the repository pin).
Host: spark-b87b, Linux AArch64, 20261112T211639Z. No device was opened.

Commands, from the CUDApp root, under `flock ~/handoffs/hive/queen/pool-6-7.lock taskset -c 6,7 nice -n 10`:

```sh
env AMD_KFD_OUT=OUT sh tests/amd-kfd/run.sh "$OMC"     # exit 0, gate.log
env MT_OUT=OUT2 sh tests/multitarget/run.sh "$OMC"      # exit 0, multitarget-after.log
```

Final lines: `AMD-KFD-HOST PASS pass=21 fail=0 not_run=2` and `MULTITARGET PASS pass=22 fail=0 not_run=4`
(the same multitarget line as before this work, recorded by the authoring lane at base `a6dee65e`).
The logs here are portable copies: only the home directory prefix was replaced with literal `$HOME`.
`byte-oracle.log` is the Python byte-oracle output the gate also prints.

Not run: AMD hardware, KFD calls, queue creation, GPU memory, AQL dispatch, completion, vadd
execution, Qwen, speed and memory-ordering or coherency proof.
