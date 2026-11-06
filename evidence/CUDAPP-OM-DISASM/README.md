<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# CUDAPP-OM-DISASM: the sm_121 decoder in seeded Omega (2026-11-05, host only)

The decoder is `om/disasm/sm121_dis.om`, compiled with the pinned seeded omc (omega-mv e5b32567, sha256
2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f) through `om/units.sh` into
`src/cudapp_om_sm121_dis.c`. The C decoder stays behind `CUDAPP_MIGRATION_PENDING_C_DISASM` as the differential oracle.
No NVIDIA output is stored here: the listings were compared in place and then deleted with the build directory.
Everything ran on the host. Nothing ran on the chip (chip: NOT_RUN).

Rows 3 to 6 ran as `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make OUT_DIR=<dir> <target>`,
with no GPU device nodes visible. Rows 1 and 2 ran before that rule, as `taskset -c 10-14 nice -n 10 make ...` without bwrap.

| Target | rc | Result |
| --- | --- | --- |
| `test-om-units test-om-disasm` | 0 | 11 golden words; 11 corrupted words refused; 26,528 words, 0 differ (.om vs C); planted red fails; units regenerate byte for byte |
| `test-om-disasm-fimm test-om-disasm-mutants` | 0 | 37,088 floats, 0 differ; `OM_DISASM_MUTANTS PASS (4/4 killed)` |
| `test-sm121dis` (oracle: nvdisasm 13.0.85) | 0 | 3,191 kernels, 1,237,248 words, 0 failed vs nvdisasm; .om vs C 0 differing kernels; planted-fault decoder fails on 3,190 kernels (the one it misses is a 32-word EXIT/branch/padding kernel with no source register); 0 opens of /dev/nvidia*, /dev/cuse or /dev/vfio under strace |
| `test-om-disasm test-om-spec-encoder`, decoy `nvdisasm` first on PATH, CUDA dirs off PATH, strace execve | 0 | 0 execve naming nvdisasm; decoy called 0 times; 0 device opens |
| `test-om-spec-encoder-nvdisasm-oracle NVDISASM=/usr/local/cuda/bin/nvdisasm` | 0 | `(c) nvdisasm reads every sample instruction back` |
| `test-om-spec-encoder-nvdisasm-oracle NVDISASM=` (negative control) | 2 | `NOT_RUN: ... needs NVDISASM=<path to nvdisasm>` |

The constants for the new rows are in `wolfram-constants.txt`.
