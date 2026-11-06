# API shells host receipt, 2026-11-06

Lane: kernel-family API shells. Branch `feat/api-shells-om`. Worktree `~/work/cudastack/lanes/ApiShellsOm`. Base `origin/main` `b774723f537cec16a359fe082b8100317a5b5490`.

Status labels: IMPLEMENTED (seeded Omega units plus C ABI shells). EXERCISED on the host and the warp simulator. QUALIFIED is not claimed. MERGED is not claimed. DEPLOYED is not claimed. Chip tier is NOT_RUN. No GPU window was authorized. Nothing here opened `/tmp/aien-gb10.lock`, port 18091, the nvidia modules, or vfio.

## What moved

Shape, alignment, overflow, refusal, slot-table, and FP8 pack decisions live in `om/api/apish_shp.om`, `apish_pack.om`, `apish_slot.om`, `apish_blk.om`, and `apish_trit.om`. `om/units.sh` registers those units when `OMEGA_UNITS` is unset. The generated bindings are `src/cudapp_om_apish_*.c`. `src/omega_api_shells.h` is the C wrapper. `CUDAPP_MIGRATION_PENDING_C_API_SHELLS` (default off) keeps the previous C expressions as the differential oracle. Public entry points are unchanged. `docs/runtime/omega_runtime_api.manifest` was not touched.

Pinned compiler: omega-mv `e5b32567`, `~/work/cudastack/omega-mv/seed/out/omc`, sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

## Commands

```
env -u OMEGA_UNITS OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc" \
  OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge" \
  taskset -c 10-14 nice -n 10 bash om/units.sh --check
```

Exit 0. Last line: `PASS portable Omega units regenerate byte for byte`. Wall time 5.72 s.

```
env -u OMEGA_UNITS taskset -c 10-14 nice -n 10 bash tests/api_shells_run.sh
```

Exit 0. Last line: `PASS api-shells checks=753157`. Wall time 2.42 s. That runner compares the default Omega path with the pending C oracle, including FP8 pack bytes, and flips one oracle record as the negative control.

```
env -u OMEGA_UNITS OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc" \
  OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge" \
  taskset -c 10-14 nice -n 10 python3 tests/api_shells_mutants.py
```

Exit 0. Last line: `PASS mutants killed=8/8`. Wall time 26.44 s. Killed: `lane-16-to-8`, `alignok-always`, `refb-nan-zero`, `slot-busy-ok`, `tgbm-64-to-32`, `pad8-no-round`, `dsplit-bad-align-ok`, `pa256-bound-off-by-one`.

After the library rebuild (`taskset -c 10-14 nice -n 10 make -j3`, cores 10-14):

| Command | rc | Last line |
| --- | --- | --- |
| `make test-gpu-fp8-slots` | 0 | `GPU_FP8_SLOTS: PASS (24 checks, 0 failed)` |
| `make test-gpu-tern-sched` | 0 | `gpu_tern_sched_test: PASS` |
| `make test-gpu-decode-attn256` | 0 | host `gpu_decode_attn256_test: 61 checks, 0 failed: PASS`; sim `gpu_decode_attn256_test: 14 checks, 0 failed: PASS` (`sim: 10975936 warp issues in total`) |
| `make test-gpu-moe-window` | 0 | `PASS: 23 checks, 0 failed` |
| `make test-gpu-fp8gemm` | 0 | host `gpu_fp8gemm_test: 59 checks, 0 failed: PASS`; sim `gpu_fp8gemm_test: 21 checks, 0 failed: PASS` (`sim: 7687894 warp issues in total`) |
| `./build/gpu_pfgemm_test --host-only` and `--sim` | 0 | `GPU_PFGEMM: PASS` (both modes) |
| `make test-gpu-prefill-host` | 0 | `test-gpu-prefill-host: PASS (8 prefill tests, host model + warp simulator, no device)` |

`make test-gpu-prefill-host` also ran `gpu_tcgemm_test`, `gpu_prefill_attn_test`, `gpu_pfgemm_test`, `gpu_qgemm_test`, and `gpu_pfhost_test` in `--host-only` and `--sim`. Warp-sim issue counts from that rebuild: decode attn256 `10975936`, fp8gemm `7687894`, prefill attn `10233092`. Wall time for `make -j1 test-gpu-moe-window test-gpu-fp8gemm test-gpu-prefill-host` was 570.00 s. `gpu_prefill_attn256_test` is a chip-only carried binary and was not run.

`make test` (the integrated host suite) is NOT_RUN. The integrator owns that gate. This lane did not start it.

## Verdicts

| Tier | Verdict |
| --- | --- |
| Host differential, 753157 checks, negative control | PASS |
| Semantic mutants, 8 of 8 killed | PASS |
| Focused family host targets listed above | PASS |
| Warp simulator (`gpu_decode_attn256_test --sim`, `gpu_fp8gemm_test --sim`) | PASS |
| Chip (`evidence/api-shells/2026-11-06/CHIP.md`) | NOT_RUN |
| `make test` | NOT_RUN |
| QUALIFIED / MERGED / DEPLOYED | not claimed |

## C that stayed in C

Seeded Omega at this pin has no pointer type, no host memory, and no GPU launcher. ARCH-0042 keeps GPU execution in CUDA++. The shells still own session lock, allocation, sync, launch, and byte copies (tile tables, KV rows, trit words, FP8 scale folds into caller buffers). Each of those copies calls an Omega predicate for the decision (`omega_api_tritok`, `omega_api_finite`, `omega_api_scalef`, `omega_api_slot`, and the shape helpers). A null pointer check stays in C because the compiler cannot name a host pointer.

Extension proposal, not done here: an omega-bridge host walk that takes a pointer, a count, and an Omega predicate, and stops at the first refusal. That would move the loop, not a decision this port still owns. It needs a bridge change, not an `omc` syntax guess.

`omega_gpu_tgemm_api.c` has no separate family binary in the Makefile. Its shape, block, and size refusals are in the differential (`omega_api_tgbm`, `omega_api_tgspec`, `omega_api_tgsz`).
