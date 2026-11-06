# NVRM_ALLOC — host receipts, 2026-11-06

## Scope and provenance

IMPLEMENTED and EXERCISED (host/fake RM). Hardware QUALIFIED: NOT_RUN.
MERGED: no. DEPLOYED: no. This is stacked on PR #336,
`feat/nvrm-struct-om` at `b9ef8f14eaa2e5810e8fc9ed06db540ff92990d2`.
The registration-first commit is `ae8ff1d62b7402e0ac10b1a993606ca062b1b753`.
The #281 vendor-header baseline is untouched. No public API signature changes.

The Omega default owns VA sorted insertion, overlap refusal, first-fit reuse,
pad/tail splitting, coalescing, capacity refusal, bump arithmetic, recycled-slot
stack operations, counter successor, proc-line decimal parsing/reductions,
method-header encoding and Ampere packet validation. Packet admission composes
the existing `amp_runtime.om` policy. C retains file reads, lock/CAS effects,
mapped pointer construction, state/error publication and the original oracle
behind `CUDAPP_MIGRATION_PENDING_C_NVRM_ALLOC=1` (default 0).
The VA table is borrowed in place; no allocation or whole-table copy is added.

Generation retains the existing `om/units.sh` pins:

- omc e5b32567: `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`
- converter: `d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538`
- seeded frontend: `a3a455bd77381e132215924295763e8728889d7d578278ae376de78324be8629`
- checked converter checkout used: `/tmp/bridge-pin-merge`; the current ordinary
  bridge checkout has a different converter digest and was not substituted.
- existing freest units require `OMEGA_SEED` baseline
  `191b776ef98821597c3e30655da6d680a9b66edbc9ceaca3c39c61ea6df098e9`.

Verification oracles: GCC `13.3.0` (Ubuntu `13.3.0-6ubuntu2~24.04.1`),
Python `3.12.3`, existing C/vendor fixtures. They are not production replacements.
No new host implementation dependency is adopted. Harness skill review: no
language/compiler/ABI/pin/repository/gate contract is changed; the family source
map and commands are updated in `nvrm/README.md`.

## Executed commands

For all commands below, `W` denotes exactly:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

Environment used:

```sh
OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc
OMEGA_BRIDGE=/tmp/bridge-pin-merge
OMEGA_SEED=$HOME/work/cudastack/omega-bridge/build/bootstrap/omc
```

| Command | rc | Final line / result | Receipt |
| --- | --- | --- | --- |
| `W sh om/units.sh` | 0 | `wrote checked portable Omega units to .../src` | generation command output; committed C and identity files |
| `W make -j3 test-nvrm-alloc` | 0 | `NVRM ALLOC MIGRATION PASS (10 compiled semantic mutants; HOST_ONLY)` | `alloc.log` |
| `W make -j3 test-nvrm-wire test-nvrm-native test-nvrm-boundary test-nvrm-differential test-nvrm-struct test-ampere-runtime test-streams-submit build/test_nvrm_va_slots` | 0 | `test_omega_gpu_platform_linux (reference only, simulated driver): 17 passed, 0 failed` | `host.log` |
| `W ./build/test_nvrm_va_slots` | 0 | `SUMMARY passed=10 failed=0` | `slots.log` |
| isolated policy command below | 0 | `NVRM POLICY MUTANTS PASS (6 killed)`; EXIT trap regenerated originals | `policy.log` |
| Extracted qualification-script gcc commands, each with allocator oracle flag 0 and 1, under `W` | 0 | `NVRM QUALIFICATION SCRIPT BUILD PASS (6 builds; execution NOT_RUN)` | `script-build.log` |

Policy's inherited script uses fixed `/tmp` names. Its execution isolated those
names with an additional tmpfs, while preserving the pinned converter and its
writable, uniquely named build scratch:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc --tmpfs /tmp \
  --bind /tmp/bridge-pin-merge /tmp/bridge-pin-merge -- \
  taskset -c 10-14 nice -n 10 make -j3 test-nvrm-policy
```

An initial read-only bridge bind refused its temporary build directory; it did
not exercise mutants. The writable bind above completed successfully. Initial
migration-gate compilation exposed missing hosted fixed-view length arguments;
those callers were corrected before the passing receipts. A mutant that changed
the method mode was caught by the direct header assertion, rather than by a
completed trace; the runner now records that assertion as the semantic kill.
No failed run is labelled PASS.

## Oracle coverage

`tests/nvrm_alloc_test.c` is compiled twice, selecting the original C or Omega.
After every operation the runner byte-compares return value, bump position,
free count, fault state, error buffer and every active range (not a checksum).
There are 32768 seeded random alloc/free operations, plus deterministic exact
fits, alignment pad/tail splits, both-direction coalescing, overlap/zero/overflow
releases, corrupt-range overflow refusal and table-capacity cases. Capacity is
exercised below/at the bound and with an insertion beyond it. Size/alignment
classes include zero, neighboring page/power-of-two values, UINT32_MAX and
UINT64_MAX neighborhoods. Counter edges straddle the nonwrapping increment
limit; the recycled pool is filled beyond capacity and drained through reuse
and fresh allocation.

Method inputs cross field boundaries and include zero and UINT32_MAX. Packet
cases cover envelope/count/subchannel/method/class refusals, admitted mixed
streams, unmapped CPU storage, non-discrete bypass and word counts 2097149,
2097150, 2097151. The latter crosses the implicit million-iteration bound using
minimum-sized admitted packets. The changed-result red control flips one result
bit before comparison and must be rejected.

`tests/nvrm_counts_test.c` independently retains the original sscanf/token
algorithm. It checks optional signs, whitespace, digit prefixes, absent digits,
saturating overflow, prefix/colon mismatch and buddy orders 0..33 across zone
and token variants. Existing fake-RM/native/struct/wire/streams gates remain PASS.

Four missing-entry controls each require link failure naming the corresponding
strong symbol: `omega_nvrm_va`, `omega_nvrm_alloc_scalar`, `omega_nvrm_counts`,
`omega_nvrm_alloc_push`. No weak fallback is present.

## Compiled semantic mutants

All were generated from changed `.om` source and compiled successfully before
execution. Compilation refusal is not counted as a kill.

| Mutant | Changed computation | Verdict |
| --- | --- | --- |
| overlap-edge | previous-end comparison includes equality | KILLED |
| coalesce-next | reverse following-range adjacency | KILLED |
| exact-fit | reverse exact end comparison | KILLED |
| split-tail | omit split count increment | KILLED |
| pool-reuse | replace recycled value by zero | KILLED |
| counter-step | decrement stride | KILLED |
| method-mode | change encoding dispatch mode | KILLED (header assertion) |
| packet-envelope | change incrementing packet opcode | KILLED |
| decimal-digit | replace decimal digit by zero | KILLED (counter assertion) |
| order-edge | exclude equal buddy order | KILLED (counter assertion) |

Existing struct and policy gates also killed their own 12 and 6 mutants;
these are separate from the ten new allocation-family mutants.

## Remaining gates and chip runbook — NOT_RUN

Full repository integration belongs to the integrator and was not run here.
Live RM, device launch, chip parity, VFIO and hardware qualification are NOT_RUN.
No service, driver/module, GPU lock or protected port was changed.

Only a new, explicit operator-authorized exclusive window permits hardware
work. Use current `AGENTS.md` and `nvrm/README.md` live-RM procedures, resolve the
native transport's pending compiler/bridge prerequisites, record exact source
and tool identities and allocate a new evidence directory. Do not treat this
receipt as authorization, or remove the isolated `/dev` barrier in a host test.

In that authorized procedure:

1. Build separate Omega-default and `-DCUDAPP_MIGRATION_PENDING_C_NVRM_ALLOC=1`
   oracle artifacts, recording the actual compiler command, not merely an
   environment variable. Both M16 compiler commands and the lifecycle compiler
   command now forward this flag; all six default/oracle builds passed without
   executing the resulting hardware programs.
2. Run existing lifecycle gates and M16 requalification with the authorized
   hardware runner. Include repeated alloc/free reuse and split/coalesce,
   independent clients/slot growth, stale/duplicate frees and failure injection.
3. Compare RM statuses, allocation/free accounting, VA ownership/refusal and
   packet outcomes. Pointer-bearing request fields must reference the same
   parameter spans, not fabricated addresses. Preserve quarantine on uncertain
   cleanup. Run a real negative control and retain its refusal evidence.
4. Capture fresh driver diagnostics and receipt identities. Call the hardware
   tier PASS only for that exact exercised revision; otherwise leave NOT_RUN.

## Wolfram queries

`WolframLanguageEvaluator`:

```text
{2^64-1-2^32,2^32,2^13-1,FromDigits["c7c0",16],
 FromDigits["02b4",16],FromDigits["02c0",16],FromDigits["005c",16],
 FromDigits["0028",16],FromDigits["021c",16],BitOr[2^0,2^4,2^12]}
-> {18446744069414584319,4294967296,8191,51136,692,704,92,40,540,4113}
{Floor[2097151/2],Ceiling[Floor[2097151/2]/999999]}
-> {1048575,2}
```

These are source constants and modeled bounds, not measurements or hardware
qualification. Test reference computations are verification oracles.
