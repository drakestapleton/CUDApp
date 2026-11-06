# GDN seeded Omega migration — 2026-11-06

Branch `feat/omega-codegen-gdn`, code at `448ba719a5cf4b718e9cbd4ae4339679050c8d6a`,
based on `22ebf8f` (checked portable IR replay) plus a cherry-pick of the optional
GPR-limit metadata change (`5604269`, now `3404837`). The public GDN header is
untouched.

## Verification inputs

- Omega source: `e5b32567a7aff3ce594e3254a622a846a6cd190b`.
- Seeded omc SHA-256: `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Omega bridge source: `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4`.

Every command ran as
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 <cmd>`
(no `/dev/nvidia*` inside), with at most `make -j3`. See `om/codegen/GDN.md` for
the environment. No chip authorization, device access, service change or
deployment is part of this receipt.

## Gates

| Gate | Command | rc | Final line | Verdict |
|---|---|---|---|---|
| Reproducible units | `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` | PASS |
| C-oracle differential | `make -j3 test-codegen-gdn-diff` | 0 | `CODEGEN GDN DIFF PASS (448 programs: decode 80 multi 80 copy 48 prep 80 scan 80 post 80; 4592 refusals)` | PASS |
| Record negative control | (inside the differential) | — | `CODEGEN GDN RECORD NEGATIVE CONTROL CAUGHT` | PASS |
| Omega source mutants | `make -j3 test-codegen-gdn-mutants` | 0 | `CODEGEN GDN OMEGA MUTANTS PASS (7 source mutants killed)` | PASS |
| Host + warp simulator | `make -j3 test-codegen-gdn-sim` | 0 | `gpu_gdn_test: 46 checks, 0 failed: PASS`, `gpu_gdn_multi_test: 14 checks, 0 failed: PASS` | PASS |
| GB10 chip battery | `build/gpu_gdn_test`, `build/gpu_gdn_multi_test` | — | — | NOT_RUN (no GPU window) |
| Full `make test` | — | — | — | NOT_RUN (integration owner) |

Logs: `units-check.log`, `diff.log`, `mutants.log`, `simulator.log`.

Host byte equality and simulator parity are EXERCISED, not QUALIFIED.
