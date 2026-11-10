# ATOMG.E.MIN.64.STRONG.SYS: BlackwellIR op 64 (2026-11-07, Hive NG lane ng-gpu)

## Why
Omega NG kernel traps (omega-mv `seed/ng/IR.md` s7.7, Hive NG D10): when threads of a launch trap, the launch reports
the code of the trapping thread with the lowest linear id. The NG sm_121 back end keeps one 64-bit trap word per launch
(initialised to all ones) and each trapping thread folds `linear_id << 4 | code` into it with an unsigned 64-bit
atomic min, then exits. The encoder had no MIN or CAS atomic (only ADD/EXCH .STRONG.SYS and the F32 adds).

## Words (oracle: nvdisasm 13.0 V13.0.85 `--binary SM121` on our own bytes)
`atomg_field_probe.bin`: ATOMG_ADD_STRONG_SYS's words with w2 bits 23..26 (operation) 0..15 and bits 9..11 (type)
0..7, decoded in `atomg_field_probe_nvdisasm.txt`: operation 1 = MIN, type 0 = (U32), 1 = S32, 2 = 64, 3 = S64.
`min_words.bin` / `nvdisasm.txt`: the three golden rows:

| BlackwellIR (d, a, b, ureg) | words | nvdisasm |
|---|---|---|
| 4, 2, 6, 8 | 020479a8 80000006 089f5508 00321e00 | ATOMG.E.MIN.64.STRONG.SYS PT, R4, desc[UR8][R2.64], R6 |
| -1, 2, 6, 8 | 02ff79a8 80000006 089f5508 00321e00 | ATOMG.E.MIN.64.STRONG.SYS PT, RZ, desc[UR8][R2.64], R6 |
| 10, 12, 14, 4 | 0c0a79a8 8000000e 089f5504 00321e00 | ATOMG.E.MIN.64.STRONG.SYS PT, R10, desc[UR4][R12.64], R14 |

Decimal constants by Wolfram: FromDigits of 79a8, 089f5500, 089f5700 base 16 = 31144, 144659712, 144660224.

## Change
- `src/omega_blackwell_codegen.h`: `BW_IR_ATOMG_MIN_U64_STRONG_SYS` = 64 (additive, after the training R5 ops).
- `src/omega_blackwell_codegen.c`: C oracle case, static assert 60-64. This case was written in this change, so for
  op 64 it is not independent of the `.om` encoder; the independent ORACLE for op 64 is nvdisasm. It keeps the C and
  diff encoder builds (`CUDAPP_ENCODER=c|diff`) able to encode op 64.
- `om/encoder/bw_enc_sm121.om`: `enc_e` op 64; bindings regenerated with `om/units.sh`.
- `src/omega_bw_hazard.c`: the ATOMG decoder reads the type field, so 64-bit forms read and write register pairs
  (it treated every ATOMG data operand as one register); `tests/bw_hazard_test.c` row for op 64.
- `om/encoder/atomg_min_test.om` + `make test-om-encoder-atomg` (seeded Omega, pinned omc; CI step): the three rows
  above as exact words, and nvdisasm 13.0.85 must decode them as those instructions. Red checks: type 2 -> 3 (.S64)
  exits 10, an absent destination not encoded as RZ exits 11, oracle text missing the R14 line exits 4.
  (Rebased onto CUDApp#373 9714eba8: the C golden rows and mutant 5 this change first added are replaced by this
  gate, as the desk review of #373 required for the same pattern.)
- `tests/bw_encode_control_test.c`, `tests/test_omega_ampere_encoder.c`: op ranges end at op 64 (control word kept;
  SM86 refuses it like the other late ops).

## Results (Linux AArch64, DGX Spark, core 1, omc e5b32567 sha 2c824805..., omega-bridge bc11b30b)
| check | result |
|---|---|
| `make test-om-encoder-atomg` with the encoder of the base (#373 9714eba8) | rc 10: op 64 refused (`atomg-gate-before-9714eba.log`) |
| `make test-om-encoder-atomg` after | PASS 3 exact rows, oracle decodes 3; 3 red checks (`atomg-gate.log`) |
| `make test-om-encoder` | 145 golden instructions and 14 fixture programs byte-identical .om vs C; red check aborts |
| `make test-om-encoder-mutants` | 3/3 red |
| `make test-om-units` | PASS, units regenerate byte for byte |
| `make test-bw-hazard` | 264 checks, 0 failed |
| `make test-bw-encode-control` | 520 encodings, 0 dropped the control word |
| `make test-ampere-encoder` | PASS |

Not done here: chip behaviour (CHIP_TESTED needs a GB10 lease and a receipt; the NG trap-word probe is planned for the
ng-gpu chip window) and the warp simulators (`tests/bw_warp_sim*.h` do not model op 64; they fail loudly on it).
