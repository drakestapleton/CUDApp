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
