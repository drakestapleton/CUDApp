# Seeded Omega ternary IR builder

The default implementation of `omega_blackwell_codegen_tern.c` is seeded Omega.
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_TERN=1` selects the retained C differential
oracle. The switch is private to the C file. The public header and API are unchanged.

## Units and host boundary

- `cg_tern_common.om`: the accepted configuration domain and coalesced-output policy.
- `cg_tern.om`: allocation requests, instruction construction, branch patching,
  nested loop intervals, all layout/depth/chunk/epilogue/mutant variants, and the
  existing generic eight-word replay format.
- `cg_tern_policy.om`: the public shape, staging, and packed-word-offset queries.
- `cg_tern_sched.om`: fixed-latency scheduling over allocated physical register
  spans, branch target timing, and the I2FP consumer checker. Scheduling happens
  after generic replay/register allocation, as it did in the C oracle.

The C shell performs API/null handling and memory serialization. Register
allocation and record replay use `omega_gpu_om_ir_replay`; there is no second IR
replay convention. The scheduler's separate temporary view contains opcode,
control, immediate, destination and three sources in eight-word records. Its
map view contains physical register numbers followed by bundle widths. These
are a private scheduling view, not another construction/replay protocol.

The builder's explicit state-slot notes name every retained scalar and array.
Phase `pN` corresponds to top-level statement N after the C oracle's entry guard;
phases are split only to satisfy the seeded compiler's live-name bound. The
flat state preserves the original interleaved allocation and instruction order.
In particular, wide-register allocation is conditional, not an eagerly evaluated
choice between two allocation calls. Signed immediates are truncated to their
32-bit record field before packing the adjacent control word.

## Verification

Use the pinned compiler and bridge already checked by `om/units.sh`. Run host,
simulator, differential and mutant gates in the device-free namespace below.
A missing device node is a host-path defect to fix, not permission to rerun
without the wrapper.

```sh
export OMEGA_OMC=/path/to/omega-mv/seed/out/omc
export OMEGA_BRIDGE=/path/to/omega-bridge
export OMEGA_MV=/path/to/omega-mv
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/units.sh --check
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-codegen-tern-diff
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 python3 om/codegen/tern_mutants.py
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-codegen-tern-sim
```

The differential includes the independent C oracle in the test translation
unit. It enumerates the entire finite accepted configuration domain, including
all shipped mutation selectors, and compares emitted IR, register allocation,
encoded instruction bytes, public policies, and I2FP checker results. Expected
builder refusals remain refusals; a refusal is not an emitted kernel. A deliberately
flipped Omega output record must be detected independently of source mutation.

The source-mutation gate changes Omega, regenerates it through the seeded bridge,
and requires the differential to kill each compiled mutant. Its point set spans
both layouts, gathered and direct output, integer and float epilogues, shallow and
deep staging, and chunked input. Temporary mutated sources/C/binaries are deleted.
Python and the retained C/algebra oracle are verification tools, not production
implementations or new build/runtime dependencies.

The simulator target combines the pinned Omega ternary/algebra battery with
CUDApp's current `bw_warp_sim_k.h`. CUDApp's simulator honors scalar LDG immediate
offsets; the older simulator at the pinned Omega revision does not. The adapter
pre-includes the current simulator and calls only the simulator battery, while
the linker discards the original device-capable entry point.

PR #229 was inspected before migration. Its ternary-header change only corrects
the QMD staging-cap comment from 32 KiB to 64 KiB. It changes no builder-read
field or accepted configuration. The untouched public header therefore supports
either merge order; the exhaustive builder domain is the same in both cases.

## Qualification boundary

Host byte equality and warp-simulator parity are not GB10 qualification. No GPU
window is granted to this lane. In a separately authorized exclusive window,
first run the integrated CUDApp host and simulator gates, then use Omega's
existing `tests/gpu_tern_test.c` chip battery against a binary linked to the
integrated CUDApp library. Record the exact CUDApp/Omega revisions and binary
hash, run its correctness and shipped mutant controls before bandwidth, and save
its final verdict/receipt. Never run the pinned Omega checkout's old C builder
and call that qualification of this migration. Do not change live services,
chip locks, driver state or boot configuration as part of this host-only work.
Device access for an eventual chip battery requires its own explicit
authorization; this lane does not disable or bypass the host-test namespace.
