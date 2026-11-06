# NO_POSIX session Omega selection — 2026-11-06

IMPLEMENTED and EXERCISED on host CPU only. Chip tier NOT_RUN. Not merged or deployed.

## Source custody and scope

Branch `feat/no-posix-session-om` starts at #334
`7f64c24ed5016eab0a612b9ebe78c1cfc859679a`. Merge commit `d103b01c`
merges #335 `ee886e852b490e5cd44ef3a09e91b83c67942dab` (native build mechanism).
PR base remains `feat/session-cmdlist-rest-om`; the #335 merge is an explicit dependency.
The BF16 API-view seam is intentionally a separate branch based on #323, as directed
by the integration owner. #335's existing scalar `no_bf16` remains registered here.

`om/no-posix.sh` now registers admit/words/park/book/streams alongside #335's roots.
The canonical units are pure. No alternate implementation was necessary. The parameter
`open` in admit was renamed `opened`: `open` is reserved in the OM-2 profile. This is
an alpha-renaming, not a mathematical change. Portable bindings and unit identities
were regenerated. NO_POSIX shells make strong `omega2_*` calls; the C decisions and
packet headers require the default-OFF `CUDAPP_MIGRATION_PENDING_C_NO_POSIX_SESSION=1`.
The session's extra-queue path is included, avoiding a residual NO_POSIX C fallback.
The stale C-only op3/op4/op5/op12 admission mirrors were corrected to the canonical
Omega contracts; the round-up mirror now handles zero alignment and uint32 carry.

Compiler: omega-mv e5b32567, omc sha256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Native bridge #335 pin: 741c1ebb; `/tmp/bridge-pin-merge`.
Native generator bootstrap compiler: `/tmp/no-posix-baseline-omc`, #335's pinned
OM-1 bootstrap. Hosted portable regeneration uses the separately pinned converter
in `~/work/cudastack/omega-bridge`; do not interchange the bridge paths.

## Acceptance commands and observations

Every command below was run with this device-isolating prefix:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

For native commands set:

```
OMEGA_NATIVE_OMC=/tmp/no-posix-baseline-omc
OMEGA_BRIDGE=/tmp/bridge-pin-merge
OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc
```

- `make -j3 test-no-posix-session`: PASS. Differential compares all result/status/output
  bytes for `decide`, `emit`, `scan`, `book` and session-used `streams_words` op6 between
  the hosted native binding and the import-free freestanding binding. Partitions are
  derived from the canonical sources: all operation branches plus refusal; booleans;
  8/64 register and hook boundaries; 352/896 offsets; 4096 page and 8192 QMD boundaries;
  uint32/sign-bit truncation and carry; UINT32_MAX and UINT64_MAX with neighbors;
  packet low/high address fields; every parked-table position and stale generation;
  all queue-open combinations and queue indices. Wrong view lengths refuse without
  modifying the caller's result. Explicit red executable exits 90. Omitting the native
  object fails linkage. Combined native/API object has no unresolved symbols.
- Symbol controls inspect actual NO_POSIX session and command-list objects with
  `nm -a` and `objdump -dr`, compiled both with explicit flag=0 and flag=1. OFF has
  `omega2_decide/emit/scan/streams_words/book` strong references and no C decision/
  packet-header oracle symbols. ON has `ses_c`, `bw_mthd`, `xq_admit_c`, `clbook_c`
  and no references to those Omega exports. This is the red selection control, not
  a claim that unrelated command-list policy has become freestanding.
- Six independently compiled source mutants killed: gpr-floor, arg-offset,
  packet-tag, park-found, book-successor, queue-bound. Every mutant first builds;
  compiler failures are harness failures, not kills.
- `make -j3 test-no-posix-native`: rc 0. Final line:
  `PASS C_NO_POSIX encoder oracle dump equals native dump; gcov recorded`.
  Its pre-existing bare ELF/import/interpreter/TLS, C cubin/encoder differential,
  injected-byte red, and six encoder/BF16 mutants also PASS with the enlarged set.
- With `OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge`, run
  `make -j3 test-gpu-session-platform test-om-cmdlist`: rc 0. Session platform
  `16 passed, 0 failed`; simulated Linux reference `17 passed, 0 failed`;
  VA slots final line `SUMMARY passed=10 failed=0`; hosted Omega session mutants
  `16 of 16 mutants killed; 7 regenerated Omega-unit mutants`. Command-list planner,
  simulator, real shell (no device), and cost gates PASS. Their reported program
  counts are harness observations, not performance or chip claims.

Detailed generated artifacts: `build/no_posix/session-check/commands.log`,
`hosted.bin`, `freestanding.bin`, `*-0.nm`, `*-1.nm`, `*.objdump`, mutant observed
bytes; `build/no_posix/check/commands.log`; `build/gpu_session_mut/*/{build,platform,policy}.log`.
The committed `native-session.log` records the final dedicated gate run.
An earlier combined command used the native bridge for the portable mutation gate
and correctly failed its converter pin check; rerunning with the documented portable
bridge passed. No gate was relaxed.

## Chip runbook — NOT_RUN

Only the operator may authorize an exclusive GPU window. After authorization, use
the existing session/command-list chip runbook from #334, build this branch with the
pinned bridge and flag OFF, run the normal production session launch/close, parked
handle refusal/recovery, and command-list packet paths on the chip, then compare the
same paths against flag ON in a separate build directory. Record exact source heads,
object identities, expected/observed packets and refusal outcomes. Keep the live model,
services, lock file, NVIDIA modules and vfio untouched outside that authorization.
Host and simulator PASS are not hardware qualification.

## Guide impact

No language, host-word, ABI or repository-ownership contract changed. The bridge and
compiler pins are unchanged; this extends CUDApp's existing #335 root registration.
No omega-harness skill example or pin needs modification.
