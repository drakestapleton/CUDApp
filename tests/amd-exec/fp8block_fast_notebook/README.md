# MI300X comparison cell v13

Paste the complete generated `cell-v13.py` into the MI300X notebook. It embeds
seeded Omega objects and needs no checkpoint download. Hardware correctness and
performance are **NOT_RUN** until the relay executes it. CPU self-check timings
are explicitly `SIMULATED_NOT_MEASURED`; they are never GPU measurements.

The cell compares `fp8gemv_b128f`, the unchanged `fp8gemv_b128` baseline, and
stock AMD `torch.matmul` BF16. Real shapes are derived from the checkpoint
acceptance report, not maintained as a second list. For each shape, all three
paths use the same deterministic OCP E4M3 W, BF16 scale grid and BF16-representable
F32 input vector. Every output row is checked, including planted NaN rows. The
log prints `same_data_sha256` for cross-path matching. Correctness and timing
lines identify the actual Omega kernel. BF16's pre-dequantized weight matrix
and output are BF16, so its separate v11 BF16 tolerance remains unchanged.

Three rebuilt wrong-kernel controls run first: per-row scales, transposed scales,
and dropped final partial scale block. A missing, crashed or escaped control
cannot produce an overall PASS. Each group runs in a separate subprocess with
its exact object name/hash marker; a fault is reported without hiding later
groups. The control summary precedes correctness output. Output is saved to
`amdserve-probe-v13.txt`, with `.state`, `.mark` and per-group stderr companions.

Warm and cold tests retain v11's synchronized per-call wall-clock method. Cold
calls rotate distinct W/y allocations beyond the stated cache working set;
x/scales are shared. Torch uses its own BF16 matrix copies beyond that same
threshold. Allocation and dequantization are outside timing. The bandwidth
numerator is W bytes (BF16 has twice the weight bytes), not total device traffic.
This is GEMV comparison evidence, not complete-model serving latency. Shape,
copies, rotation bytes and call counts are printed with every measurement.

The fast kernel deliberately changes reduction order. Both Omega variants use
v11's identical tolerance and NaN policy; no epsilon or bound was loosened.
Kernel numerical-order details live in `tests/amd-exec/fp8block_fast.md` in the
pinned source repository. `list.json`/`list.md` contain derived ABI, object hashes,
heads, sampled CPU rows and bounds; the unchanged baseline's provenance is in
`baseline/`. `build.json`, `selfcheck.json`, `cell-commands.json` and
`SHA256SUMS` are generated records, not hand-maintained claims.

## Building and checking the bundle

`fp8block_fast_bundle.py` copies these committed packaging sources alongside the
accepted objects. From that new bundle directory, under an externally held free
queen pool lock and nice 10:

```sh
python3 build.py
python3 selfcheck.py --out NEW_SELFCHECK_DIRECTORY
python3 finalize.py --pool-lock ACTUAL_HELD_POOL_LOCK --log BUILD_LOG --log SELFCHECK_LOG
```

Capture the first two commands' combined stdout/stderr in the named logs. The
self-check runs the full cell with simulated HIP/torch implementations and the
full shape list. It verifies object digests/entry selection, ABI parameters,
all-row checks, data identity, both Omega timing paths and rotation bookkeeping.
It also plants a child SIGSEGV, an escaped mutant and a wrong last finite output;
each must report `FAIL_OR_INCOMPLETE`. The simulator does not execute instructions,
model memory latency or qualify the card. Real instruction behavior is covered
separately by the seeded host executor acceptance and ultimately the relay.

Python/numpy and LLVM are outside measuring/oracle instruments. They do not
produce the production kernel object. A final overall PASS from the relay is
required before interpreting its recorded device timings; failed or missing
groups are not performance wins.
