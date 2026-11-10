# nvdisasm call sites (CUDA++ C6, DRA-475; t_81083ab1, 2026-11-07)

Every place that RUNS NVIDIA nvdisasm (not comments or prose). "ORACLE" = the check exists to compare Omega's output
with NVIDIA's independent reading, offline and read-only; nothing it prints enters a kernel. "READER" = a test that
used nvdisasm only to read machine code back; those now use Omega's own disassembler.

## CUDApp (branch hive/t_81083ab1-om-disasm, base f1b50cbf)

| file:line | what runs | class | status on this branch |
|---|---|---|---|
| tests/device-trace/run-emitter-checks.sh:100 | `nvdisasm --binary SM121 words.bin` | READER (was line 92) | REPLACED: reads with build/sm121dis (line 96); nvdisasm only when NVDISASM is set, as an ORACLE that must print the same text (40/40 equal, HOST_TESTED) |
| tools/artifact_smoke.sh:22 | `nvdisasm -c <cubin>` | ORACLE (container reader) | KEPT as oracle; ADDED build/sm121dis read of the kernel text role (`.text.bin`, line 17) |
| tools/sm121dis/check.sh:19, :55 | `nvdisasm -b SM121 -hex` per corpus kernel, `--version` | ORACLE (C6 parity) | kept |
| tools/sm121dis/negctl.sh:33 | `nvdisasm -b SM121 -hex` on the flipped copies | ORACLE (new negative control) | new |
| mk/sm121dis.mk:102, :104, :106, :109 | test-sm121-mmio: `--version`, `sha256sum`, `-b SM121 -hex` twice | ORACLE (MMIO semantic contract, issue #171) | kept |
| mk/om-encoder.mk:118, :125, :147, :154 | test-om-encoder-ldg / -atomg: `--version`, `-b SM121 -hex` | ORACLE (encoder exact words) | kept |
| tools/test_bw_spec_encoder.sh:122, :129, :136, :143, :150, :211, :217 | `-b SM121` on spec-encoder samples | ORACLE (spec encoder vs independent decoder) | kept |
| mk/ampere-cubin.mk:24 | `nvdisasm -c sm86.cubin` | ORACLE, sm_86 (Ampere): our decoder is sm_121 only | kept |
| tools/ampere_nvdisasm.py:21 (called from mk/ampere-encoder.mk:39) | `-b SM86 -hex` | ORACLE, sm_86 | kept |
| .github/workflows/ci.yml:37-45, :85-92; ampere-x86.yml:50-58 | installs nvdisasm 13.0.85 for the oracle jobs | ORACLE install | kept (not run here) |

Readers that already used Omega's decoder before this branch (no nvdisasm): gpu_elementwise/fp8_api/reconv/attention/
mathlibs/latmeas tests (`omega_sm121_listing`), tests/bw_hazard_test.c --disasm and tests/bw_ldg_mmio_test.c
(`omega_sm121_disasm`). They now run the Omega decoder (om/disasm/sm121_dis.om) through the same calls.

ZERO-NVDISASM static check: it was FAILING at base f1b50cbf (15 lines naming the toolkit outside its 3-file exception).
tools/sm121dis/oracle-allow.tsv now lists those exact lines with the reason each is an oracle, interop probe or
negative pattern; any other line, or an edited allowed line, still fails.

## omega-mv (Radicle main 1a7aa1e5): reported, not changed here

| file:line | what runs | class |
|---|---|---|
| tools/sm121dis/check.sh:18, :47 | `-b SM121 -hex`, `--version` | ORACLE (C6 parity; decoder there is still the hand-written C src/omega_sm121_disasm.c) |
| seed/sm121/gate6.sh:72, :82, :108 | `--version`; `-b SM121` on the edge words (nvd.bin); `-b SM121` on vecadd.bin | ORACLE ("reference reader only") |
| seed/kflow/kflow.om:452 (execprog), tool path from seed/kflow/m1.spec:6, specs/tmatmul16.spec:6, specs/tsoftmax.spec:6 | KernelFlow `verify` stage runs `nvdisasm -b SM<arch>` (texts `-b`, `SM` in kflow.om:17) | READER inside a pipeline stage: candidate for build/sm121dis on sm_121 jobs; sm_80 jobs (seed/kflow/sm80) have no own decoder |
| Makefile:376-383 (test-numeric-reduce-nvdisasm) | REDUCE_NVDISASM = build/sm121dis | already Omega's decoder (name kept for receipts) |
| tools/divsqrt_nvdisasm_check.sh, tools/ldst_nvdisasm_check.sh | build/sm121dis | already Omega's decoder |
| tools/sm121dis/zero_nvdisasm.sh:69-70 | planted calls in its own red check | negative control |

omega-mv's src/omega_sm121_disasm.c is the same hand-written C decoder (omega-mv PR #614, merged a0eafa39). Porting
omega-mv to the Omega decoder is the same change as this CUDApp branch; NOT done here (CUDApp side first, per queen).
