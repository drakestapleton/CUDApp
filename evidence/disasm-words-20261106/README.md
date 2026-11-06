# Disassembly word policy, 2026-11-06

IMPLEMENTED and EXERCISED on host, stacked on #311 at 69a9342c98f1. Not merged or deployed. Chip NOT_RUN; no GPU window.

The decoder was not replaced again. `om/disasm/words.om` owns little-endian half-word assembly and last-EXIT padding selection. `omega_dis_words.h` is the strong-call shell; C arithmetic is available only with default-OFF `CUDAPP_MIGRATION_PENDING_C_DISASM_WORDS=1`. The listing and --pairs consumers share it. The C loop advances over input records and performs output effects only.

## Acceptance

Command (rc 0):

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge make -j3 test-disasm-words test-om-disasm test-om-units
```

Observed: `DISASM WORDS PASS cases=460800`; listing and --pairs byte assembly differential PASS; missing Omega binding refused at link; planted difference rejected. All five .om mutants killed: byte-shift, opcode, mask, end, retain. Final focused line: `DISASM WORDS DIFFERENTIAL RED LINK MUTANTS PASS`. Existing decoder family: eleven golden words and eleven corrupted words; 26528 words with zero differences. Units: `PASS portable Omega units regenerate byte for byte`.

Partitions derive from the byte load, opcode mask and EXIT comparison: every byte value at every half-word byte position, every low 16-bit opcode/guard pattern, offsets and previous ends at 0/1/15/16/17/UINT32_MAX-1/UINT32_MAX, null code, partial records, absent/repeated EXIT. CLI fixture is byte values 0 through 63. gcov reports all oracle lines and all six branches taken; gcov's relative-source listing warning does not affect counter data. Raw logs are in acceptance.log; detailed local generated mutant logs are under build/dis-words.

Compiler sha256: 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f; bridge pins unchanged from om/units.sh. WolframLanguageEvaluator: `FromDigits["fff",16], FromDigits["94d",16]` -> `{4095,2381}` (evaluated as a list). Wolfram setup checks: `Integrate[x^2,{x,0,1}]` -> `1/3`; WolframAlpha `sqrt(27)/2` -> `(3 sqrt(3))/2`.

## Chip runbook (NOT_RUN)

After operator grants an exclusive window, run the existing sm_121 kernel chip family, retain emitted code in ignored scratch, inspect the same bytes through listing and --pairs, compare to the C-word oracle build, and save a new current-commit receipt. Do not infer chip qualification from this host-only byte policy migration. No public API or skill contract changed; no harness guide update is needed.
