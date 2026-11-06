# DEC builder and GDN admission in seeded Omega: host evidence, 2026-11-06

Units: `cg_dec` (`om/codegen/cg_dec_emit.om`, `om/codegen/cg_dec.om`) and `gdn_admit` (`om/gdn/admit.om`),
compiled by the pinned omc e5b32567 (sha256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f).
The C oracles remain behind the default-off switches `CUDAPP_MIGRATION_PENDING_C_CODEGEN_DEC` and
`CUDAPP_MIGRATION_PENDING_C_GDN_ADMIT`. Every command ran as
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 <cmd>`.

| Command | Result (final line) | rc |
|---|---|---|
| `make -j3 test-codegen-dec-diff` | `PASS dec differential cases=3991680 accepted=1220470 record-negative-controls=1220470 IR+allocation+encoded-bytes` | 0 |
| `make -j3 test-gdn-admit-diff` | `PASS gdn admission differential cases=1276 admitted=268 refused=1008 verdict+outputs` | 0 |
| `make -j3 test-codegen-dec-sim` | `PASS dec warp simulation runs=66 (6 kernels x 6 schedules, 5 builder mutants caught)` | 0 |
| `make -j3 test-dec-mutants` | `PASS dec mutants killed=11/11 (cg_dec 6, gdn_admit 5)` | 0 |
| `sh om/units.sh --check` | `PASS portable Omega units regenerate byte for byte` | 0 |
| `crumb compile . && crumb verify .` | `crumb verify: OK: 41 crumbs current` | 0 |

Domain. The DEC differential is the full product of kind {-1..9, INT_MAX}, 18 modes, 21 widths, 10 group
counts, 11 strides and 8 mutant values, covering 0 and UINT32_MAX and v-1/v/v+1 around each refusal predicate.
The accepted count is frozen at 1220470. Each accepted case also replays one flipped record (`imm ^= 1`), and the
comparison must fail on it (the negative control). The admission differential walks each of the 8 admission
operations over its refusal boundaries.

Mutants. The `dec-width-bound` site is `gt n 36` because `gt n 33..35` are equivalent mutants: n = width / 256
and no multiple of 1024 lies in 8193..9215. The first run's `dec-gather-stride` mutant (actually the AXPY stride
rule) survived; stride 128 was added to the domain and it now dies.

Branch coverage of the C oracles (gcov -b, `--coverage -O0`):
- `src/omega_gpu_gdn_admit.c`: lines 100% of 32, branches taken 100% of 52.
- `src/omega_blackwell_codegen_dec.c`: lines 98.94% of 565, branches executed 98.03%, taken at least once 85.90%
  of 305 (`gcov-dec.txt`). Uncovered branch sites and why no case reaches them:
  - IR append / vreg alloc failure (`e->err = -1` in the emit helpers, lines 48, 56-59, 108) and every later
    `if (e->err) return -1`: the IR capacity is never exceeded by any accepted spec, so they are unreachable.
  - `loop_begin`/`loop_end` depth guards (110, 112) and `em.depth != 0` (675): loops are always balanced.
  - The switch default (34) and predicated EXIT (42): no builder emits them.
  - `magic * SL - 65536u >= 256u` (541): for SL 1..16 the magic reciprocal is always exact.
  - Line 117 (live-range extension) and 539 short-circuit subterms: covered in value, one operand order not taken.
  - `omega_dec_prog_stats` (693-695): outside the switch and not ported; `tests/gpu_decode_graph_test.c` exercises it.

Wolfram: `wolfram.txt` holds the exact f32 bit checks for every float literal of the unit.

Status: IMPLEMENTED and EXERCISED on the host. Chip tiers NOT_RUN (see `om/codegen/DEC-CHIP.md`). Not merged.
