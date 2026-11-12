# gfx942 host executor: host receipt

Source: the files in `inputs.sha256`, as committed at `04c1b812cfb6a5d478fc33393226c611e386128e` (CUDApp branch `amdserve/gfx942-exec`).
Compiler: omega-mv `e5b32567` `seed/out/omc`, sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Host: spark-b87b, Linux AArch64, 20261112T213006Z. No device was opened.

Command, from the CUDApp root:

```sh
flock ~/handoffs/hive/queen/pool-14.lock nice -n 10 env AMD_EXEC_OUT=OUT sh tests/amd-exec/run.sh "$OMC"
```

Exit 0. Final line: `AMD-GFX942-EXEC PASS counts=0,1,63,64,65,200 hardware=NOT_RUN`. `gate.log` is a portable copy (home prefix replaced by literal `$HOME`).
Not run: AMD hardware, timing, memory-model and race behaviour, hardware NaN payloads.
