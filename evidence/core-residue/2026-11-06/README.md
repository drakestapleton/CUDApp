# Core residue receipt, 2026-11-06

Branch `feat/core-residue-om`, stacked on #308 (`feat/core-util-om` @ `68c8cff`). Registration is `bfa721a`. This directory records the residue slice. It does not replace `evidence/core-util/2026-11-06/`.

## What #308 already moved

| File | Already Omega in #308 | Residue added here |
| --- | --- | --- |
| `sha256.c` | `om/core/sha256.om`; C selected by `CUDAPP_MIGRATION_PENDING_C_CORE` | Source-derived length partition 0, 55, 56, 63, 64, 65, 128, 192 in `test-core-util` |
| `src/omega_core.c` | object, graph, payload, param, effect, eval | none; glue and pointer staging stay C |
| `src/omega_vector.c` | vector, vecspec, vpay | none |
| `src/omega_canonical.c` | canonical, idparse and entry wrappers | none |
| `src/omega_gpu_hostmath.c` | hostmath | none |
| `src/omega_gpu_fmt.c` | fmt, fmtnum, fmtclip | none |
| `src/omega_cubin.c` | `om/cubin/cubin_elf.om` layout; `cubinmsg.om` refusal text | `cubadmit.om` target and null-parameter admission |
| `src/omega_ampere_codegen.c` | `om/encoder/ampere_enc_sm86.om` | `ampack.om` sign-extend and flag packing |

## Stay-C

| Site | Reason |
| --- | --- |
| `omega_cubin.c` malloc, free, memcpy, file staging, ELF byte stores in the C writer | Pointer and file I/O. The writer is the default-OFF oracle `CUDAPP_MIGRATION_PENDING_C_CORE` / `CUDAPP_MIGRATION_PENDING_CUBIN_DIFF`. ARCH-0042 keeps the cubin writer in CUDApp. |
| `omega_cubin_rc_name` | String table, not layout arithmetic. |
| `cubin_admit` message copy | The Omega unit fills a 64-byte buffer. C copies it into the caller buffer and stops at `err_cap`. |
| `omega_ampere_codegen.c` IR field reads and output staging | Pointer I/O. Sign-extend loops remain only under `CUDAPP_MIGRATION_PENDING_C_CORE`. |
| `sha256.c`, `omega_core.c`, `omega_vector.c`, `omega_canonical.c`, `omega_gpu_fmt.c`, `omega_gpu_hostmath.c` | Host staging and the explicit C oracle. Computation is already `.om`. |

## Math

`WolframLanguageEvaluator: {Mod[56, 64], 128/64, 192/64, 256 + 32, BitAnd[16^^FFFFFFFF, 2147483648], 2^64 - 2^32, 2^31} -> {56, 2, 3, 288, 2147483648, 18446744069414584320, 2147483648}`.

56 is the SHA padding boundary (`Mod[56, 64]`). 128 and 192 are two and three 64-byte blocks. 288 is `BW_MAX_VREGS + BW_MAX_UVREGS`. `2147483648` is the int32 sign bit. `18446744069414584320` is the high-32 sign-extend mask.

## Gates

Host commands ran under `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.

| Command | rc | Final line |
| --- | --- | --- |
| `make -j3 test-core-residue` | 0 | `CORE-RESIDUE red check: one flipped image bit aborts` |
| `make -j3 test-core-util test-core-cubin-message` | 0 | `CORE-UTIL builders PASS: graph full boundary, every constructor and aliasing ID lookup` |
| `sh om/core/native.sh --check` | 0 | `CORE-UTIL standalone native inputs --check PASS` |
| `make -j3 test-core-residue-mutants` | 0 | `CORE-RESIDUE mutants PASS: 8 recompiled semantic mutants killed` |
| `make -j3 test-om-cubin` | 0 | `red check: one flipped image bit aborts the cubin diff, as it must` |
| `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` |
| `make -j3 test-ampere-encoder` | 0 | `Ampere encoder host fixtures: all IR opcodes, source modes, target preservation and refusals` |

Host tier: **PASS** (EXERCISED). Chip, live-RM and VFIO: **NOT_RUN**. Not QUALIFIED, MERGED or DEPLOYED. See `CHIP.md`.

No cubin is committed. Images are created with `mkstemp` under `/tmp` and unlinked in the test.
