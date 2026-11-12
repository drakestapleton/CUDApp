# Elementwise chip gate at 5783a549: which call failed (2026-11-12)

Two more runs of `build/gpu_elementwise_test` built at commit 5783a549 (binary sha256
`f65d3afcd00180c101a8d4f922dd7b300f0c7c48fb9c1b7e375e0980660db2d2`), on the GB10, shared chip,
values only. They exist because the first receipt (`evidence/chip-gates-shared-20261112/`) named
the wrong call; see `evidence/chip-gates-shared-20261112/README.md.correction.json`.

The two runs are separate processes and are reported separately.

## Run 1: unbuffered output (`elementwise-unbuffered.txt`)

`stdbuf -o0 -e0 ./build/gpu_elementwise_test --out FILE`.

- Twelve cases pass, through `swiglu_1000`.
- `swiglu_131073` is the first failure: `rc=UNWRITTEN`, `calls=2`, `unwritten=131071`, and its
  mutant call also returns `UNWRITTEN`. Both calls returned: the FAIL line is printed after them.
- The process then dies with exit 139. This run does not show in which later call.

## Run 2: under gdb (`gdb.txt`, command in `gdb-commands.txt`)

- SIGSEGV in `memcpy`, called from `run_abc`, called from `run_1d`.
- Frame 0 (`memcpy`): x0, the destination, is 0xfffff4bff010; x5 is 0xfffff4c7f010, which is
  x0 + 0x80000 (524288); x2, the bytes still to copy at the fault, is 0x7efb0 (520112).
- Frame 2 (`run_1d`): registers hold 0x20001 (131073) and 1.

Under gdb the fault came in a call with n = 131073 and a destination range of 524288 bytes. Whether
it is the same call as the first `UNWRITTEN` one of run 1 is not shown: the two processes have
different memory layouts, and in run 1 those calls returned.

## What the two runs support

A one-dimensional call longer than one chunk (131072 elements) is split, and the last chunk of
`swiglu_131073` has one element. At 5783a549 that chunk was launched and copied as 524288 bytes:
run 1 shows 131071 of its 131072 checked words unwritten, and run 2 shows a 524288-byte copy
faulting in a call with n = 131073. That is the plan's `p[10]` for n >= 131072 (the full chunk),
used where the chunk's own 4 bytes belong.

A call of one chunk is not affected: for n = 4096 the plan's `p[10]` is 16384, the chunk's own
size (measured by the independent reviewer of head b1818e36 on the hosted plan; the first twelve
cases of run 1 pass).

Fix: commit 97fc9eb3, `bytes = ni * 4`. With it `swiglu_131073` passes with `calls=2 unwritten=0`
(`evidence/chip-gates-shared-20261112/after-97fc9eb3/elementwise.txt`).
