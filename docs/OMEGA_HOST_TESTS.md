# Portable host checks and target-only commands

`make test` is the host suite. It does not need a CUDA toolkit, NVIDIA
userspace tools or a GPU device node. Carried seeded-Omega mutation gates,
including `test-codegen-f64`, require the pinned compiler and bridge checkouts;
set their environment as described in [the f64 host runbook](CODEGEN_F64.md).
Builds and simulations run in the required device-free `bwrap` namespace;
they must not open `/dev/nvidia*` or `/dev/aien-gpu`.

On x86-64, when you did not set `CC`, `make` uses gcc. GNU make's built-in
`CC=cc` is left alone on AArch64, so a GB10 `make` still invokes `cc`.
`make CC=clang test` still uses the compiler you named. Engine simulator
recipes that name `gcc` directly keep that compiler.

The default `CUDAPP_MEM_RESERVE_MIB` is 4096. It is not changed. On the x86-64
VM in `evidence/HOST-X86-20261030`, `make CC=gcc -j4 test` exits 2 at that
default and passes only with `CUDAPP_MEM_RESERVE_MIB=0`. The failing check
refuses a 741.9 MiB serving upload because 4.6 GiB is free and the runtime
keeps 4.0 GiB free. A host that cannot keep 4096 MiB free fails the same way.
GitHub Actions runs `CUDAPP_MEM_RESERVE_MIB=0 make CC=gcc -j4 test` and
`make CC=gcc gpu-engine-freestanding` on `ubuntu-24.04`
(`.github/workflows/host-x86.yml`). The AArch64 `host-tests` job is unchanged.
GB10 stays NOT_RUN.

On x86-64, `make test` does not count checks that execute the seeded AArch64
binding or the pinned `omc`. Those targets still exist. Invoking one directly
exits 2 and names the requirement. `make test-omega-target-only` prints the
same lines and exits 0; that exit is the report, not a pass of the named
checks. On AArch64 those checks stay inside `make test`.

`test-gpu-imadhi-rc` and `test-dagsim` stay in `make test` on x86-64. They are
host programs and do not open a device. On the VM in
`evidence/HOST-X86-20261030`, after main `b4f0480` was merged,
`CUDAPP_MEM_RESERVE_MIB=0 make CC=gcc -j4 test` exited 0 and ended
`CUDA++ HOST TESTS PASS (57 targets)`. Inside that run `test-gpu-imadhi-rc`
printed PASS for `bw_warp_sim.h`, `bw_warp_sim_tc.h`, `bw_warp_sim_k.h` and
`bw_warp_sim_pa.h`. `test-dagsim` printed `dagsim_unit: 171 checks, 0 failed`.
The same `make CC=gcc test-gpu-imadhi-rc` command by itself also exited 0.
GB10 stays NOT RUN.

Three targets in that suite are the fresh-checkout contract:

```sh
make test-omega-slice-runner      # missing compiler, missing slice, wrong pin
make test-omega-arch-guard        # x86-64 production refusal
make test-omega-barrier-contract  # dsb sy stays on AArch64; x86-64 keeps a real fence
make test-omega-binding-portable  # revision and epoch vectors
make test-omega-target-only       # names the checks this host does not run
```

## Binary64 codegen

`test-codegen-f64` is registered in `CARRIED_HOST_TESTS` by
`mk/codegen-f64.mk`. It runs independent C/seeded byte differentials,
IEEE binary64 edge vectors, register-pair pressure/refusal, the append-only
replay bound, a simulated DFMA/DSETP probe and six compiled seeded mutants.
The `codegen_f64_probe --chip` mode is never a host-suite dependency and is
NOT_RUN without a current exclusive window. See [the encoding facts and
chip runbook](CODEGEN_F64.md).

## Memory ordering

`nvrm/cpu_barrier.h` is the publication barrier.

- On AArch64 the instruction is `dsb sy`. That is the GB10 host barrier. The
  production backend is not rebuilt with a no-op, an empty `asm`, or a dropped
  fence.
- On x86-64 the instruction is a sequentially consistent fence. The object
  contains that fence. It is how the portable host suite compiles the same
  sources.
- `OMEGA_BLACKWELL_HOST_SIMULATOR` is a compile definition for the fake-driver
  engine tests (`test-gpu-engine-sim` and `test-gpu-engine-backend-mutants`).
  It lets those tests call the backend on a non-AArch64 host. It does not
  enable a real device. The production library is built without it. On x86-64
  that library's Blackwell open and the direct matmul and vector entries return
  `ENOTSUP` and the text `requires AArch64 Linux`, and they do not call the
  driver.

`make test-omega-arch-guard` runs that refusal on x86-64. On AArch64 the same
target prints `NOT_RUN` and does not build the binary, because a cross
compile is not an x86-64 runtime result and the production path must not be
asked to open a device from this check. `CROSS_COMPILE_ONLY=1` records the
same `NOT_RUN` after a cross compile.

`make test-omega-barrier-contract` rejects a raw `dsb sy` outside
`#if defined(__aarch64__)`, checks the header split, and disassembles a tiny
caller. A one-line unguarded fixture must fail. If `aarch64-linux-gnu-gcc` is
absent on an x86-64 host, the AArch64 object row prints `NOT_RUN`. The source
contract still runs.

## Seeded binding on a non-AArch64 host

`om/units.sh` emits every seeded unit as checked portable C through the pinned
omega-bridge (`om/omega-bridge.lock`): `src/cudapp_om_<unit>.c` for the hosted
units and `src/cudapp_freest_<unit>.c` for the import-free scalar units. They
are generated and not edited. Every host compiles the same files, so x86-64
runs `bw_enc_sm121.om`, `cubin_elf.om` and the native units through the same C
that AArch64 links. `make test-om-encoder`, `make test-om-cubin`,
`make test-codegen-freestanding` and `make test-gpu-native-runtime` run on
x86-64.

`make test-omega-binding-portable` checks the revision and epoch vectors
against the generated `src/cudapp_freest_revision.c` and
`src/cudapp_freest_epoch.c` on every host.

The pinned compiler omc is an AArch64 binary. These commands execute it, so on
x86-64 they exit 2 unless `OMEGA_OMC` is given explicitly and executes there
(`.github/workflows/ampere-x86.yml` registers the pinned QEMU provenance oracle
for that job). They run for real on AArch64 when their other tools are
present:

```sh
make test-om-spec-encoder
make test-portable-receipts
make test-model-capacity
make test-prof
```

`make test-codegen-freestanding` on x86-64 still runs the freestanding C
encoder checks. The hosted `.om` comparison prints `NOT_RUN` and is not
performed. `CUDAPP_ENCODER=diff` on x86-64 is an error: that mode executes the
seeded binding.

## Omega slice gate

The eight slices stay in `omega/slices/`: `doorbell.osc`, `sequencer.osc`,
`nvrm.osc`, `va.osc`, `open_plan.osc`, `kmod.osc`, `mesa.osc`, `launch.osc`.
A missing name fails before any compiler is required.

The compiler checkout is explicit. There is no default directory.

```sh
OMEGA_OSC_COMPILER_DIR=/path/to/omega-osc-cubin tests/omega/run.sh 120
# or: tests/omega/run.sh 120 /path/to/omega-osc-cubin
# or: make test-omega-slices OMEGA_OSC_COMPILER_DIR=/path/to/omega-osc-cubin
```

`make test-omega-slices` is not part of `make test`. A fresh checkout has no
sibling compiler, so the portable suite only runs the refusal checks.

The harness must be a clean Git checkout of source commit
`1accb93164ce68a2ac2e2c680acf54782f260d62` (the pin recorded on
`roadmap/15-reproducible-host`). The runner then uses
`build/compiler/test_osc_compiler` and builds that target when the binary is
absent. Failures:

| Condition | Exit | Diagnostic |
|---|---|---|
| `OMEGA_OSC_COMPILER_DIR` unset and no directory argument | 2 | `compiler harness not configured` plus the required commit |
| Directory is not a Git checkout | 2 | `harness directory is not a Git checkout` |
| `HEAD` is a different commit | 2 | `wrong harness source` with expected and found revisions |
| Tracked or untracked changes | 2 | `harness checkout is modified` |
| A named slice file is absent | 1 | `missing required slice` and the path |
| Binary still absent after the build | 2 | `compiler binary still missing` |

The 2026-10-22 receipt on that branch recorded an AArch64 binary SHA-256 of
`a007283780347dd4a4b024844dc0c81fc09e0926f27f83e50c07d42508886f23` and a
passing eight-slice run. This tree does not re-measure that binary. A run
prints the source commit and the binary SHA-256 of whatever it actually
executed.

## What is portable, and what is not

Portable, included in `make test`:

```sh
make
make test
make test-omega-slice-runner
make test-omega-arch-guard
make test-omega-barrier-contract
make test-omega-binding-portable
build/hello_gpu --sim
make doctor
```

`make doctor` reads `/proc`, `/sys` and `/dev`. It does not open a GPU. On a
machine that is not a DGX Spark it reports the missing pieces. That report is
the result.

Target-only, not a silent pass inside `make test`:

```sh
make test-omega-slices          # needs the pinned OSC compiler above
make test-om-encoder            # AArch64; seeded encoder binding
make test-om-cubin              # AArch64; seeded cubin packer
make test-gpu-native-runtime    # AArch64; seeded native units
make test-om-spec-encoder       # pinned omc; samples read by Omega's own sm121dis
make test-om-spec-encoder-nvdisasm-oracle  # same, read by NVIDIA nvdisasm (oracle)
make test-om-disasm-fimm test-om-disasm-mutants  # pinned omc; .om disassembler float sweep and mutants
make test-sm121dis              # pinned omc and nvdisasm; oracle agreement and .om vs C corpus
make test-portable-receipts     # pinned omc
make test-model-capacity        # pinned omc
make test-prof                  # pinned omc; profiler host and simulator tiers, chip stays NOT_RUN
make hello-gpu                  # GB10, opens the NVIDIA device nodes
./build/gpu_<name>_test --out receipt.json
```

`make test-gpu-engine-gb10-compile` only compiles the Phase B chip source. It
does not run it. `make test-gpu-scan` compiles `src/omega_numeric.c`, whose
host reference uses AArch64 FP instructions (`mrs fpcr`, `fmadd`). On x86-64
that compile fails in the assembler. So `make test` runs `test-gpu-scan`,
`test-gpu-scan-diff` and `test-gpu-scan-diff-selftest` on AArch64 hosts only
(`mk/scan-sort.mk`); an x86-64 `make test` leaves them out.
`make test-gpu-scan-mutants` needs `OMEGA_MV` and `OMEGA_BRIDGE_DIR` and is
outside `make test`. `make test-gpu-gemm` is the separate GEMM host and
simulator target and is also outside `make test`.

An AArch64 host build of this tree keeps `dsb sy` in `nvrm_cpu_barrier`. A
GB10 run of the device path is a separate receipt. A host or simulator pass
does not qualify it.

## QMMA builder migration

`make test-codegen-qmma-diff` is host-only and part of `make test`. The default
QMMA probe, peak and F2E4 probe builders execute `om/codegen/cg_qmma.om`
through checked portable Omega IR replay. `cg_qmma_patch.om` owns placeholder
validation and rewriting. `CUDAPP_MIGRATION_PENDING_C_CODEGEN_QMMA=1` selects
the old C oracle; it defaults to zero.

The differential is **exhaustive over structure, bit-basis over niter**.
For every supported peak chain count, issue stall and thread count it tests
the unique union of `{1,7,UINT32_MAX}` and
`{2^k,2^k-1,2^k+1 | k=0..31}`. Zero is a matched refusal.
It also checks both probe variants at every supported wait count and F2E4.
Each accepted case compares encoded bytes, patched bytes, patch counts and
packed cubins between C and Omega. For each structural configuration, both
implementations' patched bytes must differ from their niter=7 baseline only
in the same MOVI word-1 immediate field, whose decoded value must equal niter.
The field is located from the baseline IR, not a hard-coded instruction index.
A parity-dependent stall mutant must fail this confinement check independently
of C/Omega equality; a separate corrupted-control-word mutant must fail byte equality.
Unsupported configurations, null outputs, corrupt placeholders and short
buffers are also compared with C.

Source-level scalar dependence: `src/omega_blackwell_codegen_qmma.c:110-122`
rejects zero and copies `iters` only into `movi(e,cnt,iters)`.
On the peak path, `om/codegen/cg_qmma.om:416-419` copies the scalar into spec
slot 1, `:98-101` rejects zero, and `:294-297` copies it into the single MOVI
immediate. No other peak emission depends on that scalar. The executable
confinement checks strengthen this source argument; they do not claim literal
enumeration of every uint32 iteration count.

Domain query:
`WolframLanguageEvaluator: v=Union[Join[{1,7,2^32-1},Flatten[Table[{2^k,2^k-1,2^k+1},{k,0,31}]]]]; {Length[v],Count[v,_?(#>0&)],Length[v]*16*15*32}`
→ `{94,93,721920}` (including the refused-zero probes).
The strengthened matrix/confinement target is **NOT_RUN** to completion:
its focused run was cancelled at the operator's wrap-up directive. The initial
receipt below predates this strengthening and must not be cited as its PASS.
Initial pre-confinement receipt:
The focused command exited zero and printed
`CODEGEN QMMA DIFF PASS (7748 cases)` on 2026-11-06, after printing
`CODEGEN QMMA MUTANT CAUGHT`. This is host parity, not chip qualification.
`make test-codegen-qmma-host` also exited zero on 2026-11-06 UTC:
the patch oracle-bit self-test and model checks passed, ending
`GPU_QMMA_PROBE: PASS`.

Constants were transcribed from the existing C oracle and checked with
`WolframLanguageEvaluator: AssociationMap[FromDigits[#,16]&,{"51303832","46453433","38383838","fc000","358","380","388","ffff","7209","723e","07800000","048070ff","723c","727a","41800","2c00","ffffff00","ffff0000"}]`
→ respectively
`1362114610,1178940467,943208504,1032192,856,896,904,65535,29193,29246,125829120,75526399,29244,29306,268288,11264,4294967040,4294901760`.

Chip tier is **NOT_RUN**. In a separately authorized exclusive GB10 window,
build `taskset -c 10-14 nice -n 10 make -j3 build/gpu_qmma_probe_test`,
then run `./build/gpu_qmma_probe_test --host-only` and
`./build/gpu_qmma_probe_test` using that window's required device authorization
and receipt capture procedure. The default command exercises arithmetic,
the dropped-C negative control, and the peak sweep. Do not run it outside
that window.

## Tiled BF16 training GEMM builder migration

The default `omega_blackwell_codegen_tgemm_prog` executes the first-party
`cg_tgemm.om` builder and `cg_tgemm_spec.om` admission/query unit through
checked portable IR replay. `cg_tgemm_emit.om` shares the QMMA allocator,
record packer and scoreboard-control primitives. The old C oracle remains
behind `CUDAPP_MIGRATION_PENDING_C_CODEGEN_TGEMM=1`, default zero.
Native portable view lengths count elements, not bytes.

`make test-codegen-tgemm-diff` checks every structural shape and supported
mutant along the full independent K and N axes, compares all public query
results and refusals, and compares complete IR plus allocator state.
Each structural baseline also compares encoded bytes and packed cubins.
Equal complete IR and allocator state feed the same deterministic encoder;
this is stronger than comparing only encoded bytes.
The independent-axes command is not an exhaustive Cartesian-product receipt.
`make test-codegen-tgemm-exhaustive` visits the complete supported bounded
K/N/RT/warps/AQN/mutant product, including C refusals.
Both modes require detection of an independently corrupted Omega control word.
`make test-codegen-tgemm-sim` runs the carried budget checks and warp simulator
with its arithmetic reference and mutant controls; it opens no GPU.

Host receipt (2026-11-06 UTC, pinned `omc` e5b32567 and Bridge bc11b30b):
`taskset -c 10-14 nice -n 10 make -j3 test-codegen-tgemm-diff`
returned zero with `CODEGEN TGEMM MUTANT CAUGHT` and
`CODEGEN TGEMM AXES DIFF PASS (55295 accepted cases)`.
The printed first-instruction control-word mismatch belongs to the deliberate
mutant, not a positive-case failure. The carried reference command
`taskset -c 10-14 nice -n 10 make -j3 test-codegen-qmma-host test-codegen-tgemm-sim`
also exited zero on 2026-11-06 UTC. QMMA ended `GPU_QMMA_PROBE: PASS`;
TGEMM budget checks ended `GPU_TGEMM: PASS`; TGEMM simulation ended
`GPU_TGEMM: PASS`, with both fragment orders matching the arithmetic reference
and mutants 1–4 caught. Full-product and chip verdicts remain separate.

Constant transcription query:
`WolframLanguageEvaluator: {AssociationMap[FromDigits[#,16]&,{"21","25","26","d8","100","10000","ffff0000","fffffffe"}],2^32,2^17,2^23}`
→ `{<|"21"->33,"25"->37,"26"->38,"d8"->216,"100"->256,"10000"->65536,"ffff0000"->4294901760,"fffffffe"->4294967294|>,4294967296,131072,8388608}`.

Chip tier is **NOT_RUN**. In a separately authorized exclusive GB10 window,
build `taskset -c 10-14 nice -n 10 make -j3 build/gpu_tgemm_test`,
run `./build/gpu_tgemm_test --host-only` and
`./build/gpu_tgemm_test --sim`, then capture
`./build/gpu_tgemm_test --chip` under the current device authorization and
receipt procedure. Only the last command opens the device. Its carried chip
path checks both uncached and cached buffers, output poisoning, repeat
identity, arithmetic reference and mutant 4. Host parity is not chip qualification.

## Seeded attention IR builders

The decode and tensor-prefill attention builders use
`om/codegen/cg_attn*.om` and `om/codegen/cg_pattn*.om` by default. Omega owns
configuration loops, RMSNorm/RoPE instruction selection, scoreboard scheduling,
branch fixups, and loop metadata. The C adapter only validates the public spec
and invokes `omega_gpu_om_ir_replay`; the public headers are unchanged.

The original builders remain differential oracles behind
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_ATTN` and
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_PATTN` (both off by default).
`make test-codegen-attn`, carried by `make test`, builds those oracles separately
and enumerates every accepted spec: decode depth 1 through 4 and mutant 0 through
4; prefill warps 4 or 8 and mutant 0 through 4, with FUSED/APPEND/ATTEND and
mirror permitted only for APPEND. These spec guards define a finite complete
accepted space; invalid-boundary and null cases are checked separately.

The differential checks the full IR, register allocation, and padded encoded
bytes. Every accepted spec also runs a one-record negative control and an
Omega scheduling mutant (`kind=1`, changing the first load's stall).
Production adapters always use `kind=0`. Runtime launch parameters, including
sequence length and pointer addresses, do not change the generated program.

Regenerate and check the committed bridge output with the pins in
`om/units.sh`:

```sh
OMEGA_OMC=/path/to/omega-mv/seed/out/omc \
OMEGA_BRIDGE=/path/to/omega-bridge sh om/units.sh
OMEGA_OMC=/path/to/omega-mv/seed/out/omc \
OMEGA_BRIDGE=/path/to/omega-bridge sh om/units.sh --check
make test-codegen-attn
build/gpu_decode_attn_test --host-only
build/gpu_decode_attn_test --sim
build/gpu_prefill_attn_test --host-only
build/gpu_prefill_attn_test --sim
```

The compiler pin is omega-mv `e5b32567`, as checked by `om/units.sh`. These are
host/simulator checks, not chip qualification. A separately authorized GB10
window is required before running either test binary without `--host-only` or
`--sim`. During that window, follow the current hardware authorization and
service-safety procedure, then run `build/gpu_decode_attn_test --out decode.json`
and `build/gpu_prefill_attn_test --out prefill.json`. Both default modes open the
GPU; neither is run by the migration tests. Preserve the receipts separately
from the host evidence and perform the window's required closeout checks.

## Prefill host-work builders in Omega

Router logits and MoE weighted combine default to `om/codegen/cg_pfhost*.om`.
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_PFHOST=1` selects the retained C builder
oracle; it is off by default. Public APIs and spec layouts are unchanged.
The Omega phases construct the records, virtual-register shapes, schedule,
loop boundaries and branch displacements. C only replays the generic tape.

```sh
make test-codegen-pfhost
make build/gpu_pfhost_test
build/gpu_pfhost_test --sim
```

The differential enumerates every `k`, `e`, `t`, `h`, `top_k` and mutant
selector accepted by the C spec validators. It compares return values, every
IR field, solved register allocation and encoded bytes, including configurations
whose builders refuse. A flipped instruction is the comparator's negative
control. Separate router-address and combine-address Omega source mutants
are regenerated with the pinned compiler and converter, linked in isolation,
and must fail the same differential. Generation requires `OMEGA_OMC` and
`OMEGA_BRIDGE` as documented above; it does not install a fallback compiler.
Without an executable pinned compiler on non-AArch64 hosts, source mutation
checks are `NOT_RUN`; the exhaustive checked-C differential still runs.
The existing simulator checks both warp scheduling orders and the numerical
mutants against the host references.

Chip qualification is separate and needs a current exclusive GPU window.
Only after obtaining that authorization, build `build/gpu_pfhost_test` and
run its `--chip` mode using the current machine's window procedure. Never
interpret the host differential or simulator as a hardware receipt.
