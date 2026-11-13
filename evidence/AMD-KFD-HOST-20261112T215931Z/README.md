# AMD KFD and AQL host data layer: host-only receipt

Source: CUDApp branch `amdserve/kfd-aql`, the files in `inputs.sha256` as committed at `c6875409c46a1443064dfef34cd1146f4b22d863`.
This replaces the receipt `AMD-KFD-HOST-20261112T211639Z` (still in history) because the review fixes changed
`records.om` and `tests/amd-kfd/oracle.py` after it.
Compiler: omega-mv `e5b32567a7aff3ce594e3254a622a846a6cd190b` `seed/out/omc`, sha256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f` (the repository pin).
Host: spark-b87b, Linux AArch64, 20261112T215931Z. No device was opened.

Commands, from the CUDApp root, under `flock ~/handoffs/hive/queen/pool-14.lock nice -n 10`:

```sh
env AMD_KFD_OUT=OUT sh tests/amd-kfd/run.sh "$OMC"     # exit 0, gate.log
env MT_OUT=OUT2 sh tests/multitarget/run.sh "$OMC"      # exit 0, multitarget-after.log
```

Final lines: `AMD-KFD-HOST PASS pass=21 fail=0 not_run=2` and `MULTITARGET PASS pass=22 fail=0 not_run=4`.
The logs here are portable copies: only the home directory prefix was replaced with literal `$HOME`.
`byte-oracle.log` is the Python byte-oracle output the gate also prints. A copy of this tree with the old
`records.om` fails the same gate (`AMD-KFD-HOST FAIL pass=20 fail=1`, `record-index-max` exit 0 where 3 is required).

Not run: AMD hardware, KFD calls, queue creation, GPU memory, AQL dispatch, completion, vadd
execution, Qwen, speed and memory-ordering or coherency proof.
