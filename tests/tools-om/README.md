# tools-om: host tools held to their C oracles

Each seeded Omega unit under `om/tools/` replaces the decision logic of one host tool. The former C stays in
the same file behind a default-OFF switch `CUDAPP_MIGRATION_PENDING_C_TOOLS_<PART>` and is the oracle here.
Every script in this folder runs, in order:

1. the tool's existing self-test, built both ways;
2. a differential: the same input stream through the oracle build and the Omega build, compared with `cmp`;
3. a negative control: one digit of the last record of a matching Omega output is flipped, and the same `cmp`
   must reject it against the oracle;
4. recompiled mutants: one line of the `.om` is replaced, the copy goes through the pinned
   `omc` (sha256 `2c824805…029f`, omega-mv `e5b32567`) and `omega-bridge/tools/bridge-seeded c`, and the
   differential must change;
5. with `TOOLS_OM_COVERAGE=DIR`, gcov branch data of the oracle under the differential.

Run every script and make target unprivileged and off the serving cores:

```
export OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=~/work/cudastack/omega-bridge
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make test-tools-om
```

A script prints `NOT_RUN` and exits 3 when `OMEGA_OMC` is not the pinned build.

## own30b_prof (`tools/own30b_decode_profile.c`, switch `..._TOOLS_OWN30B`)

`own30b_diff.c` includes the tool with `-DOWN30B_PROFILE_DIFF` (no wrap; the exit dump still goes to stderr) and drives
`classify` and `record` from splitmix64. Six seeds: heavy mutation (400‰, every 64th launch fully random),
light mutation, 4200 clean tokens (past the 4096-token table, so `dropped` counts), 4300 mixed tokens, 30‰,
and a two-token 1000‰ run. Mutations: a wrong or `OTHER` stage, a failed launch (`rc` −1 or 1..200), a null
launch, a random launch (boundary sets for `n_args`, `args[8]`, `grid_y`, `num_elements`, null `args`), a clock
set back, a wrapped `end`, a 64-bit byte count, and stray launches between steps.

Partition, from the source: classification by `n_args` (FP8 10, DATTN 25, ROUTER 23, BF16 8, other), the FP8
`args[8]/4` widths 5120/2048/1536/2048 crossed with `grid_y` 1/8/other, `num_elements` 4096/2048/151936/other,
`args[18]` (attention cached tokens, including 0xffffffff); recording by expected stage (`pos % 6` below 288,
then the head), the invalidation causes, the head with and without an armed, valid, complete token, the table
full, and modular arithmetic of times and bytes.

Coverage of the oracle under the differential: every branch of `classify`, `record` and the dump is taken.
The two untaken branch groups are in `prof_tok`, the test accessor used only by the `-DOWN30B_PROFILE_TEST`
self-test (which passes in both builds); it holds no profile decision.

Mutants (8, all killed): expected-stage bound, completion position, dropped increment, the gate/up width, the
gap subtraction order, the attention byte formula, the decimal printer, and the re-arm after a head.

## fmt (`om/tools/fmt.om`, shared by the tool units)

`fm_f x pr o` writes `printf("%.Pf", x)` and `fm_g x pr o` writes `printf("%.Pg", x)`, exactly: the f64 is
expanded into its full decimal digits (base-10^9 limbs of m·2^k, or of m·5^s with the point s places left;
at most 767 significant digits) and rounded once, ties to even, as glibc does. `fmt.sh` composes the file
with a test export `fmtt` and `fmt_diff.c` compares it with glibc `snprintf` on the specials (±0, ±inf,
±nan, ties such as 0.125, 2.5, 0.0005, the extremes 2^-1074, 2^-1022, DBL_MAX) and `FMT_DIFF_N` (40000)
splitmix64 doubles in five families, each at `%.0f %.1f %.2f %.6f %.17f %.0g %.1g %.6g %.17g`: 360351
conversions byte-identical. Mutants (6, all killed): ties to even, the round-down test, the 5^13 step,
the two-digit exponent minimum, and both `%g` style bounds (exponent below −4, exponent at least P).

## doctor (`tools/doctor/cudapp_doctor.c`, switch `..._TOOLS_DOCTOR`)

The unit is `doctor=om/tools/fmt.om,om/tools/doctor.om`: `fmt.om` is an exact `%.Pf`/`%.Pg` formatter
(base-10^9 limbs of m·2^k or m·5^s, ties to even), `doctor.om` picks every line, parses the module version,
formats the memory figures and counts the FAIL lines. The C shell keeps the probes and passes them as facts:
`fopen`/`fgets` of the version file, `access(2)` of the device files and nvcc paths, the PCI scan and chip
table (`src/omega_gpu_chip.c`), the meminfo `sscanf("%llu")` (glibc parsing of the number stays the probe) and
`uname(2)`. Exit 2 is new: the unit refuses an ABI or compiler text holding `%` or a newline (the C printed them
inside a printf format) and a chip refusal text without a newline after its `GPU <bdf>: ` prefix.

`doctor.sh` builds both and compares stdout, stderr and exit on 40 machines: 38 fake roots under
`CUDAPP_DOCTOR_ROOT`, one 4300-byte root (the C `path()` truncates at 4200 bytes, the root still prints in
full) and the real machine with `CUDAPP_DOCTOR_ROOT` unset (its memory line is dropped: MemAvailable changes
between runs).

Partition, from the source: version file missing, empty (`fgets` fails), first line with an open module of the
ABI, a closed module, another version (open and closed), a version before `Module`, no `Module`, digit runs of
length 2, without a dot, of 63 and 64 bytes, a run followed by a letter, no digits, a first line over 1023
bytes (`fgets` cut) and of exactly 120 bytes with its newline (the `%.120s` edge), `Open Kernel Module` split;
each device file missing or not readable and writable, all three missing; PCI folder missing, empty, an audio
function only, the GB10, an unknown GPU, both, 17 unknown GPUs (the 16 cap), a bdf holding a newline (the
refusal's first newline falls inside its prefix: `%.*s` with a negative precision prints to the end), a bdf
of 70 bytes (`%.63s`), a bdf holding `%`; meminfo missing, without the two keys, the ties 0.25 and 0.75 GiB,
2^64−1 and 2^53+1 kB, ordinary values, unified (GB10) and system wording; nvcc at either path, not executable;
one FAIL (singular) and several.

Coverage of the oracle under the differential: lines 100% of 121, branches executed 100% and taken 95.24%
of 84. The four untaken branches cannot be reached:
- `more()`'s `while (*s)` exit: every text handed to `more()` ends without a newline (the fixed texts and the
  chip refusal, which ends `attached.` and is under 2047 bytes), so the loop always leaves through `!nl`;
- `s += k ? k - 1 : 0` with `k == 0`: `s` points at a digit, so `k >= 1`;
- `r->device == 0x2e12u` false: the chip table has one row, the GB10;
- `uname(&u) != 0`: `uname(2)` cannot fail with a valid buffer on Linux.

Mutants (10, all killed): the 120-byte cut, the run minimum and maximum, the unified flag, the 16-GPU cap,
the refusal prefix length, the GiB precision, the plural, the closed-module test and the `more()` segment step.

## ampcorp (`tools/ampere_corpus.c`, switch `..._TOOLS_AMPCORP`)

`om/tools/ampcorp.om` holds every decision of the SM86 IR capture: the folder check (no `\`, no `..` path
component), the IR metadata (little-endian u32 `op`, `imm` pairs), the classification of a legal refusal (an
opcode past `BW_IR_MUFU_LG2`, `BW_IR_HMMA_BF16` with `OMEGA_QMMA_MARK`, `BW_IR_FMNMX_MAX` with
`OMEGA_QMMA_MARK_F2E4`), the check that a refusal returned `OMEGA_GPU_ENCODE_UNSUPPORTED` with 0 bytes and left
the 0xa5 fill untouched, the check that an existing kernel encoded to its SM121 length, and the text of the paths
(`<dir>/<pid>-<serial>.<suffix>`, refused at 4096 bytes and over) and of the abort message (signed `rc`). The C
keeps `getenv`, the encoder call, `open`/`fdopen`/`fwrite`, the serial counter and `abort`; the header constants
go in as facts. Deviations: the metadata is written little-endian (the C wrote host order: the same bytes on the
aarch64 and x86-64 hosts); a folder text of 131072 bytes or more aborts (an environment string is shorter; only
`setenv` in the producer could make one).

`ampcorp_diff.c` defines `omega_gpu_encode_program_target`, so each case picks the encoder outcome, and runs
every case in a child in a fresh folder; the report holds exit or signal, stderr and each written file (name,
size, FNV-1a 64), with the pid replaced. 62 cases. Partition, from the source: folder unset, empty, plain, `./`,
trailing `/` and `//`, `..` first, middle, last and alone, `...`, `.x`, `x.`, `.`, `a..`, `..a`, `\` alone and
inside, a missing folder (`perror`, abort), paths of 4095/4096/4097 bytes for `sm121.bin` and 4095/4096 for
`refused`; the serial over 3 and 12 captures and across an unsafe folder (no increment); 0 and 1024 instructions;
`MUFU_LG2` (not a refusal) and the opcodes after it, each marker on its own and on the other opcode, a marker ±1,
the refusal first and last; a refusal with a stray byte at 0, 8191 and 16383, with 1 byte, with `rc` −1 or 0,
a refusal opcode that encodes; an existing kernel refused, one byte short or long, `rc` 7, `INT_MIN`, `INT_MAX`,
−1 at 1024 instructions, 0 bytes of 0; a write that fails in `fclose` and in `fwrite` (a link to `/dev/full`).
With `TOOLS_OM_AMPCORP_REAL=1` the script also builds the corpus producers of `test-ampere-encoder-oracle`
both ways (`CUDAPP_AMPERE_CORPUS=1`, `make CUDAPP_MIGRATION_PENDING_C_TOOLS_AMPCORP=1` for the oracle) and
compares every corpus file by name and sha256 with the real SM86 encoder.

Coverage of the oracle under the differential: lines 100% of 45, branches executed 100% and taken 96.77% of 62.
The two untaken branches cannot be reached without fault injection:
- `n < 0` after `snprintf`: the format holds only `%s`, `%ld` and `%u`, which have no encoding error;
- `fdopen` failing after `open` succeeded: only an allocation failure makes it fail.

Mutants (8, all killed): the `..` length, the `\` byte, the `MUFU_LG2` bound, the F2E4 marker, the 0xa5 fill
check, the 4096-byte path bound, the sign of `rc` in the message, and the SM121 length check.

## ampfam (`tools/ampere_families.c`, switch `..._TOOLS_AMPFAM`)

`om/tools/ampfam.om` holds the step order of the 30 generator families, every specification field, which
steps must encode on SM86 and which must be refused (`rc` −2 with 0 bytes), the short-circuit of a failing
step and both messages, which name the source line of the former C (kept in place, so `__LINE__` matches).
The C shell dispatches the generator id the unit asks for, runs the SM121 or SM86 encoder and prints the
unit's text.

`ampfam_stub.c` stands in for the 30 generators and both encoders and traces every call (generator id, name
and specification bytes; encoder limits). Partition, from the source: the baseline (133 steps, 399 trace
lines) and, at each of the 133 steps, ten outcomes: generator failure, generator `rc` 7, a refusal on an
encode step, an SM121 length of 0, SM86 `rc` 1, 0, −1 and −2 against the step's expectation, SM86 bytes one
more than SM121, SM86 bytes unchanged. 1331 cases compare stdout, stderr, exit and the trace byte for byte.
With `TOOLS_OM_AMPFAM_REAL=1` both builds also link one `CUDAPP_AMPERE_CORPUS=1` library (real generators
and encoders) and compare output, exit and every corpus file by name and sha256.

Coverage of the oracle under the differential: lines 100% of 94, branches executed and taken 100% of 218,
calls 100% of 120. No branch needs a justification.

Mutants (9, all killed): the line of a message, the ternary-GEMV skip bound, the refusal `rc`, the append
check, the residual mode field, the SM86 byte initialisation, the queries-per-CTA field, the SM121 length
check and the GDN BF16 flag.
