# Core utility Omega migration receipt

State: IMPLEMENTED and EXERCISED on the host. Not MERGED, DEPLOYED or hardware
QUALIFIED. Branch: `feat/core-util-om`; base: CUDApp
`b774723f537cec16a359fe082b8100317a5b5490`. Registration was committed first as
`26dcdff04713`. Implementation revision is the commit containing this receipt.

## Pins and scope

- Seeded compiler: `aien-dev/omega-mv@e5b32567`, binary SHA-256
  `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Existing frozen bridge pin is unchanged. Generated `.ident` files record the
  converter/frontend/source digests; `src/cudapp_om_units.ident` is the index.
- Local attributed SHA source: omega-mv
  `acb3f15cdc691b1b046b48b0dad0f9255a12ba4b:om/sha256/sha256.om`.
- Official NIST CAVP files and hashes: `tests/vectors/core/README.md`.
- Default core computation is in `om/core/`; legacy C is explicitly selected by
  `CUDAPP_MIGRATION_PENDING_C_CORE`. Existing cubin ELF, Ampere encoder, binary16,
  revision and epoch Omega code is reused, not represented as a new port.
- No public runtime API, compiler pin, bridge pin, language syntax, host word,
  repository ownership or trust gate changed. Project guides are updated in
  `om/core/README.md`, `docs/CUBIN_WRITERS.md` and a dated ROADMAP note. Native
  aggregate wiring and its matching integration guidance remain the Integrator's
  shared interface; these standalone inputs are not an aggregate qualification.

## Commands and observed results

All test, mutant and simulator commands used this device-free wrapper:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 COMMAND
```

Generation additionally used `OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc`
and `OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge`. Build concurrency was `-j3`.
Logs retain command/compiler output; the negative controls intentionally print a
mismatch before their enclosing gate reports that the mismatch was caught.

| Wrapped command | rc | Observed final result / log |
| --- | --- | --- |
| `sh om/units.sh` | 0 | Wrote checked portable units; `generate-final.log` |
| `make -j3 test-core-sha-state` | 0 | `CORE-UTIL SHA context PASS`; `state.log` |
| `make -j3 test-core-util` | 0 | SHA, canonical, effects/eval, vector, fmt, hostmath, f16 and builders PASS; `host.log` |
| `python3 tests/test_core_mutants.py --out-dir build --receipt evidence/core-util/2026-11-06/mutants` | 0 | `CORE-UTIL mutants PASS: 6 recompiled semantic mutants killed`; `mutants.log` and `mutants/` |
| Initial `make -j3 test-core-cubin-message test-om-cubin test-ampere-encoder test-ampere-plan` | 2 | Existing cubin target omitted its artifact executable prerequisite; `cubin-ampere.log` |
| After repair: `make -j3 test-core-cubin-message test-om-cubin test-ampere-plan` | 0 | Cubin differential and bit flip caught; Ampere host/sim PASS; `cubin-repaired.log` |
| `make -j3 test-ampere-cubin` | 0 | Default and differential binaries: `AMPERE CUBIN HOST PASS; device NOT_RUN`; `ampere-cubin.log` |
| `sh om/core/native.sh` and `sh om/core/native.sh --check` | 0 | Standalone native inputs write / byte-identical check PASS |
| `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte`; `regeneration-check.log` |

The initial combined command also exercised `test-ampere-encoder`: its fixtures
reported all IR opcodes, source modes, target preservation and refusals. Its
success does not convert the initial combined command's failure into a PASS.

## Coverage and verdicts

| Tier | Verdict | Evidence |
| --- | --- | --- |
| SHA FIPS / official CAVP | PASS | Empty, `abc`, multi-block FIPS vector, million-`a`; all 65 short-message and all 64 long-message CAVP records |
| SHA C differential | PASS | 10,000 generated inputs and streaming splits; digest flip caught |
| Public SHA context semantics | PASS | Padding boundaries, chunk splits and repeated finalization match C |
| Canonical / IDs | PASS | 10,000 objects, all version/kind header pairs, 16,384 ASCII parser pairs, bounded-output refusals |
| Effects / checked evaluation | PASS | 10,000 generated cases |
| Vector | PASS | 10,000 cases; chunk/refusal boundaries and partial/exact aliasing |
| Formatting | PASS | 10,000 cases, capacity boundaries, unsupported/truncated tokens |
| Host math | PASS | 10,000 bitwise non-contracting cases and threshold/exponent boundaries; C oracle compiled `-ffp-contract=off` |
| Binary16 | PASS | Every one of the 65,536 encodings; NaN payload and quieting behavior checked |
| Builders | PASS | Every constructor, full graph boundary and aliasing ID lookup |
| Semantic mutants | PASS | Initial state, round constant, padding marker, two-block boundary, bit count and length-byte-order mutants recompiled and killed |
| Cubin diagnostics | PASS | 10,000 C comparisons, truncation, bridge-status boundaries and message-record flip |
| Existing Ampere cubin tests | PASS | Both default Omega and C differential paths: `AMPERE CUBIN HOST PASS; device NOT_RUN` |
| Existing cubin artifact / manifest / sweep | PASS | 869 identical code/byte/message calls, 750 images; sweep includes 854 calls and 111037984 image bytes; image flip caught |
| Ampere warp simulator | PASS | `hello_gpu --target=sm_86 --sim`: 1048576 elements checked, zero wrong or unwritten; namespace hides devices |
| Native no-libc aggregate execution | NOT_RUN | Shared composition/bridge qualification belongs to BridgeHost and Integrator |
| Full integrated `make test` | NOT_RUN | Integrator-owned gate; not launched by this lane |
| Chip / live RM / VFIO / hardware | NOT_RUN | No current authorization; no hardware qualification claimed |

## Defects fixed during the port

- The copied SHA finalizer initially left the public context count unchanged.
  The minimal regression observed `count=0 reference=64` on empty input. The
  finalizer now advances by the same padding and length bytes as C, and its
  complete context matches after repeated finalization.
- Negative sine/cosine quadrants could encounter a checked negative-to-unsigned
  cast. The signed quadrant is masked before conversion; negative and threshold
  bit patterns are explicit regression inputs.
- Partially overlapping vectors require sequential behavior rather than a
  snapshot of a whole batch. The adapter stages one element for that ABI case.
- The existing cubin gate expanded `ARTIFACT_TEST` before its later definition,
  omitting the executable prerequisite. The target now names the existing
  `$(OUT_DIR)/gpu_artifact_export_test` prerequisite directly. The repaired gate
  passes; no test was weakened or moved outside the namespace.

The remaining C is memory/ABI adaptation: allocation/free, fixed-view staging,
pointer/span and object-tag admission, variadic argument extraction, and stable
C string-pointer storage. No numeric, hashing, canonical serialization, vector,
format-token or cubin-image algorithm remains in the default handwritten C
implementation. Function-mode globals and read-only text literals are not
available (OMEGESE, function mode); diagnostic literals are encoded as local
Omega stores instead. A future bridge static-text/lifetime projection could
replace C string-pointer storage, but no such compiler change is required for
these exercised API behaviors. No outside runtime dependency was introduced.

Hardware procedure: [CHIP.md](CHIP.md). Exact mathematical queries and returned
results used for constants and identities: [wolfram.txt](wolfram.txt).
