# Dagsim parser and formatter: host receipts, 2026-11-06

Draft PR: https://github.com/aien-dev/CUDApp/pull/331, `feat/dagsim-parse-om`, stacked on `feat/tools-om` (#314).
Base: `b0cbe919896e1ca6b5e581b7adea00981a897299`.
Registration was committed and pushed first as `6c19271`; cutover implementation was committed and pushed as `e68f87b8a903e691383bc5f39d924991e2c4acb1`. The final commit expands the source-derived partition, repairs long-date streaming and adds these receipts; the implementation hashes below identify the exercised computation.

## Verdicts

- IMPLEMENTED: parser/tokenizer, record validation, reference resolution, numeric/date conversion and parse-error formatting in `om/tools/dag_parse.om`; CLI naming, format-class/numeric decisions and stable sorting in `om/tools/dag_fmt.om`.
- EXERCISED / host PASS: byte-exact successful models, error/status parity, formatter decisions, CLI output, deterministic DAG corpus, red controls and recompiled mutants below.
- QUALIFIED / chip NOT_RUN. These are host text/CLI paths. No GPU device, serving process, service port or GPU lock was opened or changed.
- MERGED: no. DEPLOYED: no. This is a draft, not approval or merge authority.
- Full `make test`: NOT_RUN, intentionally reserved for integration.

## Implementation and explicit ABI decisions

The normal build uses Omega. The former C parser and formatter stay behind the exact default-OFF `CUDAPP_MIGRATION_PENDING_C_DAGSIM_PARSE` switch, used only as an acceptance oracle. There is no parser C fallback. Existing scheduler/costmodel integration and post-parse `ds_find_op/res` byte scans are unchanged; this receipt does not claim removal of computation outside the assigned parse/format slice.

The C shell retains file I/O, argv handling, allocation, typed packing/unpacking and `printf`. The parser receives fixed views `uchar[1048576]`, `ulong[16384]`, and `uchar[256]` error output. Header words 0–39 are reserved as specified; record starts/strides are resource 64/12, op 448/26, dep 7104/2, buffer 9152/8 and removed 11200/8. Scratch begins at 14000. The C model is zeroed before successful unpacking and each absent index uses the low 32 bits of -1.

Inputs longer than **1048575 bytes** are explicitly refused with `DS_PARSE_ERROR` and `file size out of range`, including exactly 1048576 bytes. The original text parser has no equivalent cap; those boundary cases check the documented wrapper refusal rather than pretending to compare an unbounded grammar with a bounded view. There is no hidden oversized-input fallback.

The formatter's text view is **256 bytes**, raised from the proposed 64: the existing next-experiment wording does not fit 64. Its mode table is in the `.om` source. Actual names/branches follow the C source, not inaccurate draft assumptions: a sync self-reference is rejected; a `removed` op must be absent; repeated removal records can parse. Decimal and hexadecimal ratios are rounded ties-to-even, preserving negative zero and the oracle's integer saturation behavior on the exercised partition. This is not an exhaustive proof of all `strtod` inputs or locales.

Reproducible defects fixed during differential development included signed-zero rejection for buffer sizes, negative-underflow sign loss, hexadecimal float handling, unsigned saturation, C unsigned date arithmetic, numeric leading vertical-tab/form-feed, and NaN/signed-zero score decisions. Existing mutant link recipes were repaired to link the newly required parse/format units rather than failing before a mutant could execute.

A final boundary regression found the initial fixed-view date wrapper wrongly refused long strings that the C oracle accepted (long leading zero/space runs and ignored trailing junk). The pre-fix reproducer exited 1 with `format differential: 3589 checks, 18 failures`. Mode 21 now streams 256-byte chunks with all scanning/saturation/calendar decisions in Omega; C only copies chunks and dispatches the returned status. The permanent differential covers zero/space runs in all three fields, split signs, malformed endings and trailing junk at 255/256/257 and 511/512/513 bytes. No total-length date cap or C parse fallback was added.

## Commands, return codes and final lines

Every test, differential, mutant, generated-unit comparison and make target ran inside exactly:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 <command>
```

The calls used `env OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge` after the prefix. Builds used at most `-j3`; the tools aggregate was serial (`-j1`). Logs are copied without truncation; home-directory prefixes are normalized to `~`.

| Command after prefix/environment | rc | Observed evidence / final line | Log |
|---|---:|---|---|
| `make -j3 test-dagsim-parse-om` | 0 | `parse differential: 492 cases, 0 failures`; `format differential: 3631 checks, 0 failures`; `CLI differential: 25 commands, 0 failures`; `parser mutants: 6 killed`; final `gcov: oracle and unpacker branch receipts written` | `parse-format.log` |
| `make -j3 test-dagsim` | 0 | `dagsim_unit: 203 checks, 0 failed`; original CLI checks pass; lifetime/predict controls killed; final `omega mutants: 5 killed` | `test-dagsim.log` |
| `make -j3 test-dagsim-corpus-om` | 0 | `PASS n=1200 bad=0`; final `dagsim corpus: PASS (1200 cases, driver and flipped-record red controls)` | `corpus.log` |
| `make -j1 test-tools-om` | 0 | own30b_prof, fmt, doctor, ampcorp, ampfam, artexp all PASS; final `tools-om artexp: PASS (differential 116 cases, 266 files, negative control, 10 mutants killed)` | `tools-om.log` |
| `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` | `units-check.log` |

`test-dagsim-corpus-om` invokes the unchanged existing `python3 tests/dagsim/corpus_diff.py ds_c ds_om 1200`, building `ds_c` with both C oracle switches (parser/formatter plus evaluator). The negative run against `/bin/true` returns 1 with `FAIL n=50 bad=5`; the gate requires that failure. The shared `tom_flip` changes one decimal digit of the last matching CLI output record and requires `cmp` to reject it. The parser red control deliberately changes an oracle model byte and is caught with rc 1. No expected red failure is counted as a passing comparison.

## Source-derived partition and mutants

`tests/dagsim/parse_partition.py` produces 492 cases from the tokenizer and record handlers. It covers every keyword, positional/KV tokens, first-equals splitting, empty/comment/NUL/CRLF/no-final-newline forms, duplicate/unknown/missing fields, all provenance/removal/operation classes, decimal/hex/Inf/NaN/malformed numerics, signed zero, overflow/underflow and date forms. Adjacent boundaries cover resource 32, op/buffer/removed 256, dep 1024, positional 8, KV 24, keyword/key 31 data bytes, value 127, stored ID/source/physics capacities, physical line 1024 and total input 1048576. The comparison checks exact error bytes/status and the entire successful `ds_model`, not selected fields. The additional 1200 DAGs exercise the original full C CLI oracle.

Six unique-site parser mutants compile successfully through the pinned seeded bridge and then exit **1 from the differential assertion**, never from a compiler failure:

1. resource bound 32 to 31;
2. op bound 256 to 255 (one contextual site in `dp_op`);
3. physical line refusal 1024 to 1023;
4. SHA length 40 to 39;
5. unused-field refusal disabled;
6. finite-buffer check disabled.

`mutant-*.log` records the actual mismatching cases. The unchanged evaluator's five mutants are separately exercised by `test-dagsim`; they are not substituted for parser mutants.

`oracle.gcov` and `unpacker.gcov` are annotated branch/call counts from the C oracle and normal C wrapper objects, built with `--coverage`. These are not Omega coverage. Uncalled file-I/O/evaluator functions in this focused report are outside its parser comparator; allocation-failure edges are not injected. No claim of exhaustive branch coverage is made.

## Source pins and host dependencies

- Seeded compiler: omega-mv `e5b32567`, omc SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Existing seeded bridge checkout: `bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4`; used to emit checked portable C from first-party `.om`. No handwritten C computation is added as the implementation.
- Parser `.om` SHA-256: `da585acce70b5410383d49bccb0fd2b5a330b570c27dc80f6f4ca6f174121061`.
- Formatter `.om` SHA-256: `3d73ff3a835ff3ff205d547dfe0b840e85fc65196c4f9d74c894831504370153`.
- Generated parser C SHA-256: `c292c81887231769556140a357792732690de4dd0f509bb4f5f855a979c2f835`.
- Generated formatter C SHA-256: `d1d3d6eb51e89fa26da2015772480a32adfb20637a5ad8304109ed11080c97bf`.
- Host C oracle/build toolchain: GCC/gcov `13.3.0` (`Ubuntu 13.3.0-6ubuntu2~24.04.1`) and glibc `2.39` (`Ubuntu GLIBC 2.39-0ubuntu8.9`); Python `3.12.3`; bubblewrap `0.9.0`. GCC/libc build and run the existing I/O/printing shell and generated checked C. Python/gcov/bwrap are host verification/isolation tools, not production parser implementations.

The compiler, bridge and host C toolchain are existing build dependencies; Python/gcov/C oracle/`cmp` are verification tools, not replacement implementations. No compiler/language feature, pin, bridge contract, host word or repository ownership changed. `tests/tools-om/README.md` documents the new unit ABI, guard and commands; no omega-harness language-skill change is required.

## Wolfram receipts used for encoding

Mounted Wolfram verification: `Integrate[x^2,{x,0,1}] -> 1/3`; WolframAlpha `sqrt(27)/2 -> 3 sqrt(3)/2`.

`WolframLanguageEvaluator: Table[{x, First[ImportByteArray[ExportByteArray[{x}, "Real64", ByteOrdering -> -1], "UnsignedInteger64", ByteOrdering -> -1]]}, {x,{0.05,1.*^-6,1.*^15}}] -> {{0.05,4587366580439587226},{1.*^-6,4517329193108106637},{1.*^15,4831355200913801216}}`.

`WolframLanguageEvaluator: First[ImportByteArray[ExportByteArray[{1.*^-9}, "Real64", ByteOrdering -> -1], "UnsignedInteger64", ByteOrdering -> -1]] -> 4472406533629990549` (nanoseconds-to-seconds multiplier).

`WolframLanguageEvaluator: {2^32-1,2^52,2^53,2^63,2^64-1,2^11-1,Floor[1074/32]+1} -> {4294967295,4503599627370496,9007199254740992,9223372036854775808,18446744073709551615,2047,34}`.

`WolframLanguageEvaluator: {2^63-1,2^63} -> {9223372036854775807,9223372036854775808}` (streaming signed-integer saturation limits).

`WolframLanguageEvaluator: {Quotient[2^64-1,10],Mod[2^64-1,10],2^11-1,2^10-1} -> {1844674407370955161,5,2047,1023}`.

`WolframLanguageEvaluator: {64,64+32*12,64+32*12+256*26,64+32*12+256*26+1024*2,64+32*12+256*26+1024*2+256*8,64+32*12+256*26+1024*2+256*8+256*8} -> {64,448,7104,9152,11200,13248}` (packed word offsets).
