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
