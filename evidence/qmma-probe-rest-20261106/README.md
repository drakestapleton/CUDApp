# QMMA probe residual migration — 2026-11-06

Base `feat/omega-codegen-a` at `af70a66cf94e`.

IMPLEMENTED: mark recognition and little-endian reads use strong
`omega_qm_host`. Patch-selftest IR construction uses strong `omega_qm_self`,
which reuses the existing QMMA record initializer, allocator bookkeeping,
scheduler and packer. No existing instruction generator was ported again.
The selftest's opcode/mask checks and packed class counts run in Omega;
C retains buffers, replay/encode/patch calls, traversal and diagnostics.
Old hand-written computation remains only under default-OFF
`CUDAPP_MIGRATION_PENDING_C_QMMA_PROBE_REST=1`. Public API is unchanged.
The existing family differential explicitly enables the independent old oracle.

Owned files: `om/codegen/qm_host.om`, `om/codegen/qm_self.om`, unit registry,
generated binding C/header/ident, `src/omega_blackwell_codegen_qmma.c`,
`mk/codegen-a.mk`, `tests/qmma_probe_rest_test.c`,
`tests/qmma_probe_rest_mutants.py`, oracle flag in the existing differential,
ROADMAP note and this receipt. Unit registration commit: `49b83714`.

## Commands and observed results

All runtime commands use this required device-isolating prefix:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

* `make -s -j3 test-qmma-probe-rest`: rc 0,
  `QMMA-REST SELFTEST IR/ENCODED/CUBIN IDENTITY PASS`,
  `QMMA-REST DIFF PASS cases=2949`; intentionally flipped result fails case 1,
  then `QMMA-REST RED PASS`.
* With `OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc` and
  `OMEGA_BRIDGE=~/work/cudastack/omega-bridge`,
  `make -s -j3 test-qmma-probe-rest-mutants`: rc 0,
  `QMMA-REST MUTANTS PASS (8 killed)` and both
  `QMMA-REST STRONG-LINK host PASS` / `QMMA-REST STRONG-LINK self PASS`.
* `make -s -j3 test-codegen-qmma-diff test-codegen-qmma-host test-codegen-tgemm-sim`:
  rc 0. QMMA host final `GPU_QMMA_PROBE: PASS` with zero failures;
  family differential final `CODEGEN QMMA DIFF PASS (714275 cases)`,
  including full cubin identity, 94-value exhaustive iteration bit-basis,
  iteration-dependence mutant and flipped-record mutant caught.
  Existing TGEMM host and simulator both end `GPU_TGEMM: PASS`;
  all five simulator cases match both schedules and four mutants are caught.
  The existing QMMA host gate checks its arithmetic model; there is no
  QMMA warp-simulator option, so the adjacent existing codegen-family
  TGEMM simulator gate is reported separately, not as QMMA chip execution.

EXERCISED: spec partition, red control, eight source mutants, both link
controls, selftest/full-family cubin identity, family host/model and existing
TGEMM simulator PASS. QUALIFIED on chip: NOT_RUN.

Partition axes include source opcode, immediate and word/mask transition
values v-1/v/v+1, zero and UINT32_MAX. Little-endian reads cover each byte
position at every byte value. Verification covers F2FP, QMMA, unmarked HMMA
and irrelevant instructions. The original C selftest's captured IR is
independently encoded, patched and packed, then compared byte-for-byte with
the Omega selftest's full cubin. Matching success text is also required.

| Omega source mutation | Verdict |
| --- | --- |
| HMMA marker opcode 16 → 15 | KILLED |
| read byte shift 8 → 9 | KILLED |
| F2FP golden word -1 | KILLED |
| QMMA golden opcode +1 | KILLED |
| HMMA golden word +1 | KILLED |
| F2FP class count increment -1 | KILLED |
| selftest store offset 64 → 68 | KILLED |
| selftest F2FP source subregister 1 → 0 | KILLED |

Converter or link errors are not mutant kills. The two strong-link controls
remove each implementation in turn and require its unresolved symbol.
The unchanged seeded compiler/converter pins are recorded by `om/units.sh`.
Compiler: `omega-mv e5b32567`, SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

WolframLanguageEvaluator:
`{16^^ffff,16^^ffffff00,16^^723e,16^^048070ff,16^^727a,16^^2c00,16^^723c,16^^41800,2+1+256+65536,2^64-1}`
→ `{65535,4294967040,29246,75526399,29306,11264,29244,268288,65795,18446744073709551615}`.
These are exact integer opcode masks/counts, not measured hardware figures.

## Chip runbook — NOT_RUN

After an operator-authorized exclusive window, run the existing
`gpu_qmma_probe_test` hardware probe and peak sweep, recording this source
revision, seed, all model/layout/negative-control output and final verdict.
Do not use host selftests as chip qualification. No GPU device, live service,
module or GPU lock was touched here. Full `make test` is integrator-owned.
MERGED/DEPLOYED: no.
