# Seeded Omega GDN builders

The default implementation of `src/omega_blackwell_codegen_gdn.c` is seeded Omega.
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_GDN=1` selects the retained C differential
oracle. The switch is private to the C file. The public header and API are unchanged.

## Units and host boundary

- `cg_gdn_shape.om`: `omega_gdn_shape` (ratio 0 means 3, ratios 1..3, the
  parameter, tail and state sizes, bf16 halving). Shared by both units.
- `cg_gdn_common.om`: the eight-word record emitter, control words, the held
  scoreboard prefetch bookkeeping, branch/region/loop records and the operand
  helpers (`omega_gd_*`).
- `cg_gdn.om`: entry `cg_gdn k a c d b m`. Kind 0 decode, 1 multi-token decode,
  2 state copy/zero, 3 prefill prep, 4 prefill scan, 5 prefill post. `a` is the
  mutant selector, `c` the copy mode, `d` the ratio with `state_bf16 != 0` in bit 32.
  Every refusal the C builders make is made here (unknown kind, copy with a mutant
  or an unknown mode, a GDN kernel with a copy mode, mutant outside 0..4, an
  unsupported ratio). Multi sets the GPR limit 246 through metadata word 6.
- `cg_gdn_policy.om`: entry `cg_gdn_policy q ra bf`, the public shape queries.

The C shell keeps API/null handling and the three stats accessors; the stats
accessors are shared by the oracle and the default build. Register allocation,
loop-interval checks and record replay use `omega_gpu_om_ir_replay`; no replay op
or metadata word was added.

Vreg ids live in a flat state view (`s`, 512 words). A slot is a name only; the
allocation order is the C order, phase by phase. Phase functions are split where
the seeded compiler's live-name/register bound requires it (`pr1`/`pr1b`,
`pr2`/`pr2b`/`pr2c`), and argument expressions deeper than one operation are
first assigned to `h1..h4`. Neither changes the emitted order.

## Verification

Run every gate in the device-free namespace. A missing device node is a host-path
defect to fix, not permission to rerun without the wrapper.

```sh
export OMEGA_OMC=/path/to/omega-mv/seed/out/omc
export OMEGA_BRIDGE=/path/to/omega-bridge OMEGA_BRIDGE_DIR=/path/to/omega-bridge
export OMEGA_MV=/path/to/omega-mv
W="bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10"
$W sh om/units.sh --check
$W make -j3 test-codegen-gdn-diff
$W make -j3 test-codegen-gdn-mutants
$W make -j3 test-codegen-gdn-sim
```

`test-codegen-gdn-diff` links the C oracle into the test translation unit and
enumerates mutant -1..5, copy -1..3, ratio {0,1,2,3,4,UINT32_MAX} and
state_bf16 {0,1,2,UINT32_MAX} for all six builders. It compares refusals, IR
instructions, register allocation, encoded bytes, the shape policy (including
untouched output on refusal) and all stats accessors, checks NULL handling, and
ends with a negative control: one flipped bit in an emitted Omega record must
show up as an IR difference.

`gdn_mutants.py` changes one unique site in the Omega sources, regenerates every
unit that contains it through `tools/bridge-seeded`, rebuilds the differential and
requires it to fail with `GDN DIFF` or `GDN SHAPE DIFF`. Temporary sources, C and
binaries are deleted. Python and the retained C are verification tools only.

`test-codegen-gdn-sim` runs the existing `gpu_gdn_test --host-only`, `--sim` and
`gpu_gdn_multi_test --sim` batteries against `libomega_gpu.a`, which now contains
the Omega builders.

## Qualification boundary (chip tier NOT_RUN)

Host byte equality and warp-simulator parity are not GB10 qualification. No GPU
window was granted to this lane. In a separately authorized exclusive window:

1. Run the integrated host and simulator gates above on the exact revision.
2. Build `build/gpu_gdn_test` and `build/gpu_gdn_multi_test` from that revision and
   record the CUDApp revision and binary SHA-256 values.
3. Run `build/gpu_gdn_test` (default mode is the chip battery: chained steps against
   the double oracle, rerun bit-identity, mutants 1-4 caught, record path), then
   `build/gpu_gdn_test --bf16`, then `build/gpu_gdn_multi_test` without `--sim`.
4. Keep each final verdict line and the `--out` receipt as the chip receipt.

Do not change live services, chip locks, driver state or boot configuration as
part of host work.
