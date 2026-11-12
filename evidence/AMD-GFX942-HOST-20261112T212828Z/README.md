# gfx942 HSA code object: host receipt for the reviewed source

Source: CUDApp branch `amdserve/gfx942-hsaco` at `62c4f0a200b37cc5bb3a05b367b296d49a07c2af` (the files in `inputs.sha256`), base Radicle main
`a6dee65e00c3d4bbda32a7d63c9e5a0126a92a9a`. This receipt supersedes nothing: `AMD-GFX942-HOST-20261112T210312Z`
remains the true record of the first commit `6ebd47b0` and its source digests; this one records the source after
the review fixes (`d8c94eb3` MessagePack encodings, then the gate case for them).
Compiler: omega-mv `e5b32567` `seed/out/omc`, sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Host: spark-b87b, Linux AArch64, 20261112T212828Z. No device was opened.

Command, from the CUDApp root:

```sh
flock ~/handoffs/hive/queen/pool-10-11.lock taskset -c 10,11 nice -n 10 env AMD_OUT=OUT sh tests/amd/run.sh "$OMC"
```

Exit 0. Final line: `AMD-GFX942-HOST PASS pass=16 fail=0 hardware=NOT_RUN`. `gate.log` is a portable copy (home prefix replaced by literal `$HOME`).
The emitted object is unchanged from the first receipt: sha256 `10eb12e019ff2117ddc0190650899d4254589375c62e5ebb970b1bbb7e67e55f`.

Not run: AMD hardware, ROCr loading, dispatch, FP32 outputs on a GPU, linked-oracle whole-object parity (no ld.lld).
