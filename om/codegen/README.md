# Seeded Blackwell codegen: remaining families

The default implementations of the attn256, pattn256, router, FP8 GEMM,
BF16 GEMV, activation quantizer (pq), and grouped quantized GEMM (qg)
builders are the `cg_*.om` units in this directory. The public C signatures
are unchanged. The original builders remain differential oracles behind
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_<FAMILY>=1`, default off, where FAMILY is
ATTN256, PATTN256, ROUTER, GEMM, BF16, or QG.

## Ownership and data flow

`src/omega_codegen_rest_wire.h` only widens public structure fields and copies
query results. It contains no configuration validation, instruction selection,
scheduling, geometry calculation, register assignment, or encoding policy.
Each root copies its configuration prefix before replacing the buffer with
IR records. The builder's memory shell calls `omega_gpu_om_ir_replay` with
that root callback; the common CUDApp replay path owns IR decoding,
loop-aware liveness, one register-allocation solve, and seeded encoding.

The roots share `cg_rest_emit.om`. Its instruction scheduling is the existing
non-256 attention lane's emitter convention, reused without changing that
lane's files. Qg additionally composes the existing
`om/prefill-shape/admit.om` and `order.om` policies through `om/units.sh`.
There is no host-language interpreter or source converter in the production
path. The committed `.om` is compiled by the pinned seeded compiler through
the existing Omega Bridge exporter.

Function-local state is explicitly indexed in a bounded array to respect the
seeded function-mode parameter and live-name limits. Input/output slot maps
follow each helper declaration; local declarations carry their C name, type,
and state slot. Instruction values occupy separate state records: initializing
an instruction before an intervening helper emits another instruction must
not lose the first instruction's value. Conditional calls and conditional
allocations retain C's short-circuit and lazy-branch semantics. Array initializer
elements are emitted sequentially: allocating a conditional second scale
register must not move it ahead of the first initializer. The qg SwiGLU
32-row, four-warp, 256-column configuration is a focused regression for that
ordering, as well as part of the output-span corpus.

## Root ABI

Each `cg_<family>` root takes four scalars (`kind`, reserved `a`, `b`, `c`),
an edit array of 8192 words, and an edit metadata array of 512 words.
The memory shell places each spec's fields, in declaration order, in words
0 onward. Signed `mutant` fields are sign-extended. Word 15 indicates a
non-null spec and word 16 a non-null query-output structure. Invalid kinds
are refused. Query roots place their scalar result in metadata word 48 and
structure fields, in declaration order, in words 32 onward.

| Unit | Query kinds | Builder kind |
| --- | --- | --- |
| attn256 | 0 shape | 1 decode attention |
| pattn256 | 0 shape, 1 spec admission | 2 prefill attention |
| router | 0 shared-memory layout | 1 router |
| gemm | 0 steps, 1 shared bytes | 2 FP8 GEMM |
| bf16 | 0 steps, 1 shared bytes | 2 BF16 GEMV |
| qg | 0 pq admission, 2 qg admission, 3 shared bytes, 4 span | 1 pq, 5 qg |

Successful builders return the existing eight-word IR-record ABI documented
in `src/omega_gpu_om_ir.h`. Metadata word 6 is the opt-in physical GPR limit:
zero preserves the historical `omega_bw_ir_init` default; a nonzero value
must be greater than `BW_PHYS_GPR_MAX` and no greater than
`BW_PHYS_GPR_HW_LIMIT`. Replay validates the full-width value before changing
the destination program. The qg rt=8 geometry requests the existing hardware
limit; the other geometries retain zero. The metadata word 7 scheduler scratch
is cleared by the terminal unconditional EXIT before the root returns; this
lane does not define any exact-last-use-override schema.

## Admission repair

The legacy FP8 GEMM admission check multiplied full-width `mt` before checking
the shared-memory bound. A large power-of-two `mt` could wrap the product to
zero and admit a geometry that overruns fixed accumulator arrays. Both the C
oracle and Omega now compare `mt` against the shared-memory bound divided by
the already validated lane-byte factor, before any multiplication. The
boundary corpus includes the high-bit `mt` counterexample. Ordinary admitted
geometries are unchanged.
For multi-token attention, a valid head ratio can still have a non-power-of-two
virtual-row count above the high bit. The old logarithm shifted a 32-bit value
by 32 before reaching its refusal. UBSan reproduced this with equal
`UINT32_MAX` query/KV head counts and one query per CTA. C and Omega now reject
non-powers before the logarithm. The corpus includes the high-bit boundary on
both sides, the maximum, and a small non-power; valid power-of-two rows retain
the same instruction stream.


## Reproduction

Use omega-mv e5b32567 and seeded omc SHA256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Set `OMEGA_MV`, `OMEGA_OMC`, and `OMEGA_BRIDGE` to the checked sibling trees.
All host tests, simulations, differentials and mutants must run with a minimal
`/dev`; none of these commands authorize hardware access:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 make -j3 test-codegen-rest
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 make -j3 test-codegen-rest-sim
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 make -j3 test-om-ir-limit
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 sh om/units.sh --check
```

`tests/codegen_rest_check.py` independently compiles the C oracle with renamed
symbols and the default Omega path. It checks IR fields, the complete allocator
state, encoded bytes, public geometry queries, and refusals, including null
builder inputs. The grids cover every admitted K step, discrete geometry axes,
all pq group counts through 32, BF16 mask/batch/stride/mutant interactions,
and qg output-span/group interactions. Attention includes full-width valid
head-count pairs, not only individually invalid field perturbations.
Every family also receives zero and `UINT32_MAX` for every scalar input,
each branch-threshold center and its immediate neighbours, and deterministic
generated single-field perturbations. The domain is a **partition plus
boundaries derived from the C branch conditions**; it is not an enumeration
of the uint32 Cartesian product. The carried target builds the retained
builders with `-O0 --coverage` and writes per-source line/branch counts and
uncovered arc locations to `coverage.json` in its build directory. The dated
receipt records branch coverage X/Y and accounts for every uncovered arc.
The runner flips an IR record and recompiles semantic scheduler mutants for
every unit, plus the required ignore-meta[6] qg mutant.

Hardware qualification is separate. See [CHIP.md](CHIP.md) and the dated
receipt under `evidence/codegen-rest/2026-11-06/`.
