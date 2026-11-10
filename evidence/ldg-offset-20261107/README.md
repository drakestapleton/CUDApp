# LDG.E byte offset: the seeded encoder dropped it (2026-11-07)

Status: HOST_TESTED. Not chip-tested in this change.

## Defect
At main `b774723f537cec16a359fe082b8100317a5b5490`, `om/encoder/bw_enc_sm121.om` encoded op 11 (`BW_IR_LDG_E`) as
`w1 = ureg` and ignored `imm`. The C oracle `bw_c_encode_insn` (src/omega_blackwell_codegen.c:408) writes
`w1 = ureg | (imm & 0xffffff) << 8` and refuses offsets outside the signed 24-bit field, as the `.om` encoder already
did for LDG.E.64 / LDG.E.128 / STG.E. `CUDAPP_ENCODER=om` is the default, so every LDG.E with a nonzero offset loaded
from offset 0. Producer: `src/omega_blackwell_codegen_tern.c:283` (`ldg32` at `r * row_bytes + d * step_bytes`), so
tern weight rows other than the first read the first row's words.

## How it was found
CUDApp's own corpus diff build (`CUDAPP_ENCODER=diff`, `tools/sm121dis/corpus.sh`) aborts in `bw_hazard_test`:
`ENCODER DIFF: .om and C encoders disagree (program, 310 insns)` after 3264 SAME encodes
(`corpus-before-main-b774723f.log`, `corpus-before-difflog-summary.txt`). A referee instrument that applies the C
oracle to captured encoder inputs names the two instructions: insns 41 and 171, op 11, imm 0x40, C w1 `0x00004004`,
`.om` w1 `0x00000004` (`corpus-before-encref-diffs.txt`). The inputs were captured with a link-time wrap of
`omega_bw_enc_sm121`. Both instruments (`encref.c`, `ngcap.c`) are handwritten C, so they are not in this
repository (desk review of b056bf7e); they are kept outside it as labeled instruments, history
`b056bf7e:evidence/ldg-offset-20261107/`.

## Oracles (independent, never shipped)
- NVIDIA `nvdisasm` 13.0.85 `--binary SM121` on our own words (`ldg_words.bin`) decodes `1e027981 00004008 0c1e1900
  000f2200` as `LDG.E R2, desc[UR8][R30.64+0x40]`, and the -0x100000 and +0x7fffff forms likewise (`nvdisasm.txt`).
  `make test-om-encoder-ldg` reruns it on every accepted row, including -0x800000.
- CUDApp's pinned C encoder (`bw_c_encode_insn`, src/omega_blackwell_codegen.c) in the corpus diff build.

## Change
- `om/encoder/bw_enc_sm121.om` op 11: refuse offsets outside signed 24 bits; put the offset at w1 bits 8..31.
- Regenerated bindings with `om/units.sh` (pinned omc e5b32567 sha256 2c824805..., omega-bridge bc11b30b built from
  source): `cudapp_om_bw_enc_sm121`, and the units that compose the encoder (`encode`, `q38_emit`, `validate`),
  plus `cudapp_om_units.ident`. `units.sh --check`: `PASS portable Omega units regenerate byte for byte`.
- `om/encoder/ldg_offset_test.om` + `make test-om-encoder-ldg` (seeded Omega, pinned omc): exact words for byte
  offsets 0, +0x40, -0x100000, +0x7fffff, -0x800000; refusal of +0x800000, -0x800001, 0x7fffffff, 0x80000000; the
  nvdisasm oracle must decode the five accepted words as those offsets. Red checks: the pre-fix op-11 line exits 11,
  a field bound widened by one exits 20, oracle text missing the -0x800000 line exits 4. Wired into CI next to
  `test-sm121-mmio`. This replaces the C golden rows and mutant 4 that 870fd16c added (pure seeded Omega).

## Results (Linux AArch64, DGX Spark, core 1)
| Check | Before (main binding) | After |
|---|---|---|
| `make test-om-encoder-ldg` (.om gate) | main's encoder: rc 11, row 1 (+0x40) wrong (`ldg-gate-before-main.log`) | PASS 5 exact words, 4 refusals, oracle decodes 5; 3 red checks (`ldg-gate.log`) |
| `make test-om-encoder` | n/a | golden instructions and fixture programs byte-identical; red check aborts (`test-om-encoder.log`) |
| `make test-om-encoder-mutants` | 3 mutants | 3/3 red (`mutants.log`) |
| `make test-om-units` | n/a | PASS (`units.log`) |
| `make test-om-encoder-corpus` (corpus diff build) | aborts in bw_hazard_test after 3264 SAME | 3191 distinct kernels identical .om vs C; diff build 4495 program encodes and 398 single-instruction encodes, all SAME (`corpus-after.log`, exit 0) |
| freestanding C path | n/a | PASS, 14 fixtures, 0 failing checks |
