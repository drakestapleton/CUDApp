# SM86 refusal of training R5 opcodes in the Ampere corpus

Host receipt for the native x86 Ampere oracle after omega-mv train r5 removed `omega_tc_patch_code`.

## Commit and machine

- Parent: `8f37d75e2a842c5214cfc563d8eb16ddb88d6c38`. This folder is committed with the four source edits it describes.
- Omega compared: `19b8340dbae70888c49fcc98299be928831bfd26`.
- Pinned omc: `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f` (omega-mv `e5b32567`). omega-bridge `3dc4a00e` (`om/omega-bridge.lock`).
- Machine: Linux 6.12.94+ x86_64, gcc 13.3.0. No GPU. `CUDAPP_MEM_RESERVE_MIB=0` for `make test`.
- Date: 2026-11-02.
- nvdisasm 13.0.85, verification only. Device qualification: NOT_RUN.

## What upstream did

omega-mv at `e5b32567` still has `omega_tc_patch_code`. Train r5 (`19b8340`) deletes that function and the placeholder marks. `SHFL.BFLY`, `F2FP.BF16.F32.PACK_AB`, and the two f32 strong atomics are native sm_121 encoder entries. omega-mv has no `tools/ampere_corpus.c`. The SM86 encoder (`om/encoder/ampere_enc_sm86.om`) already returns failure when `op > 59` and writes no bytes.

## Commands

```
make OUT_DIR=build-ampere-oracle CUDAPP_AMPERE_CORPUS=1 test-ampere-encoder-oracle
```

Exit 0. `SM86 offline oracle: 4478 kernels, every opcode/operand matched`. `native FP8 refusals: 350 kernels`. See `oracle.txt`.

```
sh om/units.sh --check
make test-om-units
```

Both print `PASS portable Omega units regenerate byte for byte`. Exit 0. Generated `src/cudapp_om_*.c` files were not edited. See `units.txt`.

```
CUDAPP_MEM_RESERVE_MIB=0 make -j8 test
```

Last line: `CUDA++ HOST TESTS PASS (80 targets)`. Exit 0. See `make-test-tail.txt`.

```
bash tools/extract/drift.sh --self-test <omega> 19b8340dbae70888c49fcc98299be928831bfd26
```

`DRIFT-GATE: PASS (920 GPU paths, omega 19b8340dbae7, none moved away from CUDA++ since the baseline)`. `SELF-TEST: PASS`. Exit 0. See `drift-self-test.txt`. `tools/ampere_corpus.c` and `tools/ampere_families.c` are not in the drift set. `tests/test_omega_ampere_encoder.c` and `src/omega_ampere_codegen.h` are CUDA++ only.

## CI on 1c3d33f

GitHub Actions run 41387070372, job `native-x86-host`, was cancelled at the 40 minute job cap. The log had already printed `CUDA++ HOST TESTS PASS (83 targets)` and `Ampere extended existing generator families encoded (host only)`, then `python3 tools/ampere_nvdisasm.py` ran until `The operation was canceled`. The job budget is now 60 minutes, the same budget `host-x86.yml` gives the x86 suite. The commands and the corpus compare are unchanged.

## Not run

GB10 chip runs. NOT_RUN.
