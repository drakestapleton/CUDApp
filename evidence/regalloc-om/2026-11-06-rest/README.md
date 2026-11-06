# Register-allocator family remainder: hazard checker and binding/sizing in seeded Omega

Date 2026-11-06. Branch `feat/regalloc-om` (CUDApp draft PR #309), parent
commit `0df685d`. Pinned seeded compiler omega-mv e5b32567, omc SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`; portable
bridge pins as in `om/units.sh`. Host: DGX Spark (aarch64), cc (GCC).

Every command below ran as
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 <cmd>`
with `OMEGA_OMC`, `OMEGA_BRIDGE` set. No GPU device was opened. The chip tier is
NOT_RUN.

## What moved to Omega

| Source | Generated C | Export | C caller (default Omega) | C oracle switch |
| --- | --- | --- | --- | --- |
| om/codegen/bw_hazard.om | src/cudapp_om_bw_hazard.c | omega_hz_chk | src/omega_bw_hazard.c `omega_bw_hazard_check` | CUDAPP_MIGRATION_PENDING_C_HAZARD |
| bw_hazard.om + bw_hazard_dec.om | src/cudapp_om_bw_hazard_dec.c | omega_hz_dec | src/omega_bw_hazard.c `omega_bw_decode` | CUDAPP_MIGRATION_PENDING_C_HAZARD |
| om/codegen/cg_bind.om | src/cudapp_om_cg_bind.c | omega_cg_bind | src/omega_bw_bind.h, used by matmul.c (spec, binding, dispatch), realize.c (machine id, realization id, vector launch), encoder.c (digest admission), codegen.c (program padding, buffer fit, instruction count) | CUDAPP_MIGRATION_PENDING_C_BIND |

Freestanding builds (`OMEGA_GPU_NO_POSIX`) keep the C, as for the earlier units.
The C keeps memory, structs, SHA-256 and pointer checks.

Documented divergences from the C:

- The hazard checker takes fixed-size views (the portable bridge has no unsized
  views): at most 2048 instructions (32768 code bytes). The C accepted longer
  code; Omega refuses it with -1. `hazard_diff.c` checks n=2048 agrees and
  n=2049 is refused.
- `realization_compute_id` hashes the arch word little-endian; the C hashed host
  bytes. Identical on the little-endian hosts CUDApp supports.

## Results (all PASS unless stated)

| Step | rc | Final line |
| --- | --- | --- |
| `sh om/units.sh --check` | 0 | PASS portable Omega units regenerate byte for byte |
| `make -j3` | 0 | cp build/libomega_gpu.a build/libcudapp.a |
| `make test-bw-hazard` | 0 | bw_hazard_test: 264 checks, 0 failed: PASS |
| `make test-om-encoder` | 0 | red check: one flipped bit aborts the encoder diff, as it must |
| `make test-codegen-freestanding` | 0 | PASS: freestanding codegen contract (14 fixtures, 0 failing checks) |
| `make test-om-encoder-mutants` | 0 | mutant 3 red (NOPC 1032192→1032193): golden words FAIL, dump vs C DIFFERS |
| `sh tests/regalloc_om/run.sh` | 0 | PASS bind differential cases=50686 refused=38401 |
| `python3 tests/regalloc_om/mutants.py` | 0 | PASS recompiled Omega semantic mutants=19 |
| `sh tools/runtime_portability_check.sh . docs/runtime/portability_table.tsv mk/omega-gpu-engine.mk` | 0 | PASS: 151 rows |
| `sh tests/regalloc_om/sim_ge.sh` | 0 | PASS unsigned GE semantic mutant KILLED compile=0 exercise=1 |

`run.sh` hazard lines: `PASS hazard decode differential cases=431078
refused=135482` and `PASS hazard check differential cases=40495 refused=16372
findings=89806 capped=1 fixtures=485 c_s=2.572 omega_s=221.565` (logs:
`/tmp/rar-pt/final-*.log`, not committed).

Performance: the generated portable C for the hazard checker is about 86x
slower than the C in this run (2.572 s against 221.565 s over the same cases,
with other lanes sharing cores 10-14; an earlier quieter run measured 1.265 s
against 123.028 s, 97x). Ratios from Wolfram:
`N[221.565/2.572] -> 86.145`, `N[123.028/1.265] -> 97.255`. It uses
checked bounds helpers on every access. The checker runs only in tests.

## Mutants

`mutants.py` now has 19 cases: the 10 previous ones plus 9 new, 7 in
`cg_bind.om` (spec-m-limit, spec-precision-byte, dispatch-bf16,
vector-ceil-div, pad-minimum, fit-boundary, arch-le) and 2 in `bw_hazard.om`
(hazard-size-6, hazard-op-2433). A `bw_hazard.om` mutant regenerates both
hazard units, as `om/units.sh` concatenates them. Compile refusals are not
counted. Result: rc 0, `PASS recompiled Omega semantic mutants=19`, 19 KILLED.

Candidates dropped from the list:

- `vector-uint32-wrap` did not compile (omc error 7); not counted.
- `hazard-u2i-sign` (`hz_u2i` returns the unsigned value) survived. It is an
  equivalent mutant: `hz_bra` wraps `t` above 2^31-1 by 2^32 afterwards, so the
  result is the same for every input.
- `hazard-branch-sign` (BRA offset sign extension 0xC0000000 → 0x40000000) and
  `hazard-size-class` (`if le s 4` → `if lt s 4`) survived. These are test gaps:
  `hazard_diff.c` does not generate negative BRA offsets whose decoded target
  differs observably, nor size class 4. Strengthening the generator is open
  work.

## Branch coverage of the C oracles (gcov -b, -O0, from the differentials)

| Function | Branches taken |
| --- | --- |
| omega_bw_hazard.c (whole file) | 92.18% of 243 (lines 98.45% of 194) |
| legacy_omega_bw_decode | 76/77 |
| legacy_omega_bw_hazard_check | 54/60 |
| step / join / kill_mask / size_regs | 54/54, 6/6, 4/4, 6/6 |
| legacy_omega_matmul_spec_init | 14/14 |
| legacy_omega_blackwell_bind_matmul_realization | 3/6 |
| legacy_omega_blackwell_codegen_matmul | 8/10 |
| legacy_omega_blackwell_get_machine_id | 1/2 |
| legacy_omega_blackwell_realization_compute_id | 1/2 |
| legacy_omega_blackwell_realize_vector | 7/12 |

Untaken branches: NULL-pointer and allocation-failure checks, which stay in C
around the Omega call; `omega_bw_hazard_kind` (name strings, not ported); the
vector encoder and SHA failure paths in `realize_vector` (not ported logic);
`R`/`W` `n <= 0` and the `note` duplicate-finding scan's 64-entry overflow.

## Not done / NOT_RUN

- Chip tier: NOT_RUN (no GPU window). See `tests/regalloc_om/CHIP.md`.
- Full `make test`: NOT_RUN (Integrator owns it).
- Freestanding Omega: NOT_RUN; freestanding keeps the C.
- C inventory slice 4: `src/omega_bw_hazard.c` decision logic is ported
  (`omega_bw_hazard.h` stays as the C interface). `src/omega_bw_em.h`,
  `src/omega_bw_em_q.h` and `src/omega_bw_reconv.h` are not ported by this lane
  (oracles `test-gpu-reconv`, `test-gpu-gpr-boundary` untouched).
- The A/B packers in `omega_blackwell_codegen.c` are not ported. pack_a's
  source reaches m·k = 2^27 elements, above the bridge's 2^24-element
  fixed-view limit, and exact-size views would force a full copy. An extension
  (unsized or larger views in the portable bridge) is needed.
