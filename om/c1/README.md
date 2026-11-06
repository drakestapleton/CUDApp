# C1 checked kernels

The source of record for the kernel language, checker2 and language-to-IR
lowering is omega-mv at `omega-mv.lock` in this directory:
`bb552199e8970bbdcb42c79acc8a36f9c1189e23`,
[draft #1044](https://github.com/aien-dev/omega-mv/pull/1044), not a merged
compiler release. CUDApp owns sm_121 instruction encoding and cubin packaging.
No frontend source is copied into CUDApp.

## Host acceptance

Set `OMEGA_C1_SOURCE` to a clean checkout of the exact pin, with its pinned
`seed/out/omc` artifact present. Then run:

```
taskset -c 10-14 nice -n 10 make -j3 test-c1
taskset -c 10-14 nice -n 10 make -j3 test
```

`test-c1` remains standalone while its omega-mv source is a pending draft;
it joins `HOST_TESTS` only once that draft merges. Missing or changed source
or a mismatched compiler SHA-256 is NOT_RUN (the source-check script exits 3,
and Make reports failure), never PASS. No fallback checkout or compiler is used.

The pipeline is:

1. Omegese source kernels, including vector add, shared-memory tiled GEMM and
   decode attention, emit canonical OSC IR.
2. checker2 verifies types, ownership, memory phases and races. Named negative
   fixtures must be refused. The seeded Omega checker is compared to the C
   migration oracle on the same bytes.
3. `seed/oscgpu/*.om` lowers the checked unit to Blackwell IR. All dumps,
   refusals, metadata and machine bytes are compared with the C migration
   oracle, including deliberately broken lowering variants.
4. CUDApp's default `om/encoder/bw_enc_sm121.om` encodes those instructions.
   Conversion opcodes 64/65 are append-only; training opcodes 60..63 retain
   their existing meanings. Ordinary guard predicates are encoded fail-closed.
5. CUDApp's default `om/cubin/cubin_elf.om` packs sm_121 ELF cubins. The test
   parses the section table and checks exact `.text` roundtrip; changing a text
   byte must be caught.
6. The existing SIMT simulator runs the matched Blackwell IR against the host
   references under both fragment schedules. It models divergence, shared
   memory, barriers and shuffles. It is an IR simulator, **not** an independent
   machine-byte decoder or a GPU driver. Byte equality and ELF roundtrip bind
   its input to the generated cubin, but do not establish hardware correctness.

Buffer bounds remain checked at runtime: invalid indices trap with status code
3 and poison propagation, and cannot perform later stores. They are not a
claim that checker2 statically proves every dynamic buffer index in bounds.
A literal index beyond a fixed shared-array length is statically refused as
`C2-GPU-BOUNDS`. The dynamic bounds tests separately exercise refused execution;
neither tier is a claim of a complete static bounds proof.

Host build dependencies are the pinned compiler artifact, C compiler and the
omega-bridge-generated bindings already committed in each repository. C glue
in `sim.c` only adapts the test ABI and checks ELF output; the language lowering,
encoder and cubin writer are Omega. Existing C lowering/checker implementations
are retained as explicit migration/differential oracles, not represented as
removed dependencies.

## Chip runbook — NOT_RUN

No command in `test-c1` builds a device launcher or opens `/dev/nvidia*`.
A fresh Drake-authorized exclusive GPU window is required for all steps below.
Historical omega-mv receipts do not qualify the draft pin.

After authorization and the current hardware runbook's preflight, repeat the
host command above and preserve its output and the exact source/compiler/image
hashes. In the pinned language checkout, use the generated CUDApp machine bytes:

```
taskset -c 10-14 nice -n 10 make -C "$OMEGA_C1_SOURCE" -j3 \
  test-osc-gpu-chip OSC_GPU_OM_DIR="$PWD/build/c1/om/om"
```

That launcher is the existing migration path in omega-mv. It checks the generated
machine bytes against the CUDApp corpus before launch. It does not test cubin
loading; that requires CUDApp's module-loader hardware acceptance as a separate
receipt. Collect the vector add/GEMM/attention reference verdicts, status-word
bounds refusals, and a red run for each lowering mutant. Do not weaken the gate
or use a historical receipt if the new path is refused.

The live-35B replacement gate is also **NOT_RUN**. In a separately authorized
serving window, the operator must identify the actual serving process/model,
freeze a prompt corpus plus deterministic generation settings and golden token
IDs, and select one matching live kernel shape. Capture baseline tokens before
changing the kernel dispatch. Replace only that dispatch with the checked
kernel, capture token IDs under identical settings, require exact ID equality,
and restore the original dispatch on any refusal or mismatch. Record model,
weights/tokenizer hashes, dispatch identity, source/image hashes, settings,
baseline and replacement token streams, and rollback result. No live model or
service is modified by this PR, and simulator equality is not that receipt.
