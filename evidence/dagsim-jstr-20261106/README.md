# dagsim JSON escaping, 2026-11-06

IMPLEMENTED / EXERCISED host-only, stacked on #341 at 56baf9103995 (which includes #331). Not merged or deployed. Chip and live-device tiers NOT_RUN. The scheduler, parser and resolver were not replaced again.

`om/tools/dag_jstr.om` owns byte classification, escape selection, hexadecimal digits and output length. `jstr` retains NUL-terminated stream traversal and stdio effects. The original C escaping is retained only under default-OFF `CUDAPP_MIGRATION_PENDING_C_DAGSIM_JSON_ESCAPE=1`. The normal call is strong; a missing binding fails the link.

## Acceptance

Command, rc 0:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge make -j3 test-dagsim-jstr-om test-dagsim test-dagsim-parse-om test-dagsim-resolve-om test-om-units
```

Focused final line: `JSON DIFFERENTIAL RED LINK FIVE MUTANTS PASS`. All 256 bytes compared to the C formatting rule. Complete stream output compared against the actual retained `jstr` oracle for empty string and all non-NUL bytes; NUL remains the string terminator and is also tested directly at the byte-unit interface. Source partitions include control/printable boundary 31/32/33, quote 33/34/35, backslash 91/92/93 and both hexadecimal nibbles. Planted output flip fails; missing unit fails linking. Five .om mutants killed: quote, slash, control, hex, length. gcov reports jstr called 256 times with 100% blocks, and all conditional and traversal branches taken.

Family gates: test-dagsim unit/CLI/lifetime and scheduler mutation controls PASS; parser partitions and six mutants PASS; resolver 4498 checks with zero failures and nine mutants PASS; missing scheduler remains refused; portable units regenerate byte for byte. Receipts in acceptance.log and oracle.gcov (home prefix normalized to `~`). Detailed generated mutant diagnostics remain in ignored build/dagsim-jstr.

Compiler pin unchanged: omega-mv e5b32567, sha256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f. Bridge pins unchanged from om/units.sh. No arithmetic constants are newly derived: ASCII and JSON byte values are literal encoding values, and boundary enumeration is integer bookkeeping. No public API, compiler, pins, language or harness guide changed.

## Chip runbook (NOT_RUN)

This offline JSON tool does not execute GPU work. In a separately authorized hardware window, save dagsim output describing that run and compare its byte stream with the C-escape oracle build, without giving dagsim device-node access. Do not count this as GPU qualification. No device node was opened in these fixtures.
