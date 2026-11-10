# Core utilities in seeded Omega

The default core algorithms are first-party `.om`. `CUDAPP_MIGRATION_PENDING_C_CORE`
selects the handwritten C differential oracle, not a platform fallback. The cubin
writer remains in CUDApp. Nothing is imported from another repository at build time.

| Public implementation | Computation |
| --- | --- |
| `sha256.c` | `sha256.om` |
| `omega_canonical.c` | `canonical.om`, `idparse.om` and their entry wrappers |
| `omega_core.c` | `object.om`, `graph.om`, `payload.om`, `param.om`, `effect.om`, `eval.om` |
| `omega_vector.c` | `vector.om`, `vecspec.om`, `vpay.om` |
| `omega_gpu_fmt.c` | `fmt.om`, `fmtnum.om`, `fmtclip.om` |
| `omega_gpu_hostmath.c` | `hostmath.om` |
| `omega_cubin.c` | Existing `../cubin/cubin_elf.om`; `cubinmsg.om` renders layout refusals; `cubadmit.om` decides target and null-parameter admission |
| `omega_ampere_codegen.c` | Existing `../encoder/ampere_enc_sm86.om`; `ampack.om` sign-extends and packs instruction fields |
| `cudapp_freest_f16.c`, `cudapp_freest_revision.c`, `cudapp_freest_epoch.c` | Existing generated bindings, already sourced from Omega |

The SHA implementation is a local, attributed copy of
`aien-dev/omega-mv@acb3f15cdc691b1b046b48b0dad0f9255a12ba4b:om/sha256/sha256.om`.
Its original Apache-2.0 attribution is preserved. The adapter also preserves the C
context's post-padding count and repeated-finalization behavior. Official NIST
byte-oriented CAVP short/long vectors and their provenance are in
`tests/vectors/core/`.

## ABI and memory ownership

C retains allocation, release, bounded staging, pointer/span adaptation and C
variadic argument extraction. Sorting, validation, serialization, hashing, checked
arithmetic, vector operations, format parsing/rendering and floating-point
polynomials are in Omega. Cubin target admission, the null-parameter check and the
error-message decision tree are also Omega; Ampere field sign-extension and flag
packing are in `ampack.om`.
`tests/core_cubin_refusal_ref.inc` is an independent frozen C oracle, not a
production implementation. No new runtime public API or version is introduced.

Bootstrap object layouts have compile-time size/offset assertions. Short caller
buffers are staged into the fixed-size views accepted by the pinned compiler.
Canonical output retains partial writes on refusal. Overlapping vector inputs
retain the C sequential aliasing behavior. The formatter keeps truncation and
unsupported-token behavior. Host arithmetic is compared bit-for-bit against the C
oracle compiled with `-ffp-contract=off`; each Omega operation is non-contracting.

`native.sh` composes the same local sources into OM-2 native units, with 23 core
exports plus separate existing cubin/Ampere units. Native aggregate registration
and no-libc qualification belong to the integration lane. Composition is not
native execution evidence. There is no `NO_POSIX` C fallback.

## Native realization identity composition

Compose `om/cache/digest.om`, `om/blackwell/identity.om` and
`om/blackwell/id_native.om` after an `om2` prefix. The adapter exposes
`bw_id_native` with the native public `long` result while retaining the
canonical `bw_identity` integer result and policy. The digest and identity
output parameters are named `dst`: `out` is reserved in OM-2. Their types,
function names and byte behavior do not change.

`tests/identity_native.om` emits the complete digest bytes for 4096 distinct
machine-wire and 4096 distinct realization-wire inputs. It also checks the
length refusal. Run its composition against both the pre-rename and renamed
units and compare stdout. Native root registration in the production aggregate
is separate from this compatibility prerequisite. These are host checks, not
physical identity or GPU qualification.

## Host gates

Every test and compiler-driven mutant must run with device nodes hidden:

```sh
run_host() {
  bwrap --dev-bind / / --dev /dev --proc /proc -- \
    taskset -c 10-14 nice -n 10 "$@"
}
export OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc"
export OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge"
run_host sh om/units.sh
run_host make -j3 test-core-util test-core-cubin-message test-core-mutants
run_host make -j3 test-om-cubin test-ampere-cubin test-ampere-encoder test-ampere-plan
run_host sh om/core/native.sh
run_host sh om/core/native.sh --check
run_host sh om/units.sh --check
```

`test-core-util` covers FIPS/CAVP SHA, 10,000 random one-shot and streaming
comparisons, padding/context boundaries, canonical objects and all header pairs,
parser quirks, effects/evaluation, vector overlap/refusal boundaries, formatter
truncation, host math, constructors and all 65,536 binary16 encodings.
`test-core-cubin-message` compares 10,000 diagnostic cases and output-capacity
boundaries. Negative controls flip records. `test-core-mutants` recompiles six
independent semantic SHA mutants, requires the baseline to pass and treats a
compiler failure as a broken mutant rather than a kill.

The existing cubin gate compares image bytes, status and diagnostics against the
legacy writer and includes an image-bit negative control. `test-core-residue`
checks target and null-parameter admission against the C `snprintf` format, packs
one cubin per accepted target under `/tmp` and unlinks it, and compares Ampere
field packing with the inline sign-extend formula. Its red check flips one image
bit. `test-core-residue-mutants` recompiles eight semantic mutants. Ampere's plan
target uses the warp simulator, not hardware. Missing-device failures are never
grounds to rerun outside the namespace. Hardware authorization and qualification
are separate; see the lane receipt and `CHIP.md`.

Receipts: [core utilities](../../evidence/core-util/2026-11-06/README.md),
[core residue](../../evidence/core-residue/2026-11-06/README.md).
