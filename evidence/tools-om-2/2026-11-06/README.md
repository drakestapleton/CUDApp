# tools-om-2 host receipts, 2026-11-06 (CUDApp#314, branch feat/tools-om)

Lane ToolsOm2 continued the tools/ migration from b043be1e. Every test, mutant and differential below ran as
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh tests/tools-om/<unit>.sh`
(library builds `make -j3`). Pinned omc: omega-mv e5b32567, sha256
2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f (checked by `tom_pin` in each script).
Status: IMPLEMENTED and EXERCISED on the host. Not QUALIFIED (chip tiers NOT_RUN), not MERGED, not DEPLOYED.

| Unit | Tool | Switch (default OFF) | Final line |
|---|---|---|---|
| own30b_prof | tools/own30b_decode_profile.c | `CUDAPP_MIGRATION_PENDING_C_TOOLS_OWN30B` | `tools-om own30b: PASS` |
| doctor (+fmt) | tools/doctor/cudapp_doctor.c | `CUDAPP_MIGRATION_PENDING_C_TOOLS_DOCTOR` | `tools-om doctor: PASS` |
| ampcorp | tools/ampere_corpus.c | `CUDAPP_MIGRATION_PENDING_C_TOOLS_AMPCORP` | `tools-om ampcorp: PASS` |
| ampfam | tools/ampere_families.c | `CUDAPP_MIGRATION_PENDING_C_TOOLS_AMPFAM` | `tools-om ampfam: PASS (differential 1331 cases, real library 355 corpus files, negative control, 9 mutants killed)` |
| artexp | tools/artifact_export.c | `CUDAPP_MIGRATION_PENDING_C_TOOLS_ARTEXP` | `tools-om artexp: PASS (differential 116 cases, 266 files, negative control, 10 mutants killed)` |

ampfam real-library run (`TOOLS_OM_AMPFAM_REAL=1`):
`real library: output and 355 corpus files identical ( 111 ir 22 refused 111 sm121.bin 111 sm86.bin )`.

`coverage/` holds the gcov branch reports of each C oracle (`<unit>.gcov.txt` summary and the annotated
`<file>.c.gcov`), built `--coverage -O0` and run under the same differential, which also checked the coverage
build's output against the oracle. Justifications for every untaken branch are in tests/tools-om/README.md.
Oracle branch coverage (taken at least once): ampfam 100% of 218; artexp 93.48% of 138 (9 untaken: library
refusals made unreachable by earlier checks, and a NULL `strtoul` end pointer). `artexp-run.log` is the full
log of the final artexp run. Home paths are replaced by `~`.

NOT_RUN: dagsim (tools/dagsim/dagsim.c, dagsim_cli.c) was not ported in this lane; its scheduler stays in
om/tools/dag_eval.om from b043be1e and the parse/resolve/format C is unchanged. corpus_diff 1200 was not run.
