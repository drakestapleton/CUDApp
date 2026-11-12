# Elementwise chip gate at 5783a549: which call failed (2026-11-12)

A second run of `build/gpu_elementwise_test` built at commit 5783a549 (binary sha256
`f65d3afcd00180c101a8d4f922dd7b300f0c7c48fb9c1b7e375e0980660db2d2`), on the GB10, shared chip,
values only. It exists because the first receipt (`evidence/chip-gates-shared-20261112/`) named the
wrong call; see `evidence/chip-gates-shared-20261112/README.md.correction.json`.

- `elementwise-unbuffered.txt`: `stdbuf -o0 -e0 ./build/gpu_elementwise_test --out FILE`. Twelve
  cases pass, through `swiglu_1000`. The first failure is `swiglu_131073`: `rc=UNWRITTEN`,
  `calls=2`, `unwritten=131071`. Then the process dies, exit 139.
- `gdb.txt`: `gdb -batch -ex run -ex bt`. SIGSEGV in `memcpy` called from `run_abc`, from
  `run_1d`. In `run_abc` the byte count is 0x80000 (524288). In `run_1d` the call is n = 0x20001
  (131073) and the chunk is 1 element.

What this shows: a one-dimensional call longer than one chunk (131072 elements) is split, and the
last chunk of `swiglu_131073` has one element. At 5783a549 `run_1d` gave that chunk the plan's
`p[10]`, which for n >= 131072 is the full chunk, 524288 bytes. The launch therefore checked 131072
output words of which the kernel wrote one (131071 unwritten), and copying 524288 bytes into the
4 bytes left of the caller's buffer ran past the mapping.

A call of one chunk is not affected: for n = 4096 the plan's `p[10]` is 16384, the chunk's own size
(measured by the independent reviewer of head b1818e36 on the hosted plan; the first twelve cases
here pass).

Fix: commit 97fc9eb3, `bytes = ni * 4`. With it `swiglu_131073` passes with `calls=2 unwritten=0`
(`evidence/chip-gates-shared-20261112/after-97fc9eb3/elementwise.txt`).
