<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# Attention API kernel generation in seeded Omega

## Implementation and provenance

Branch: `feat/api-kernel-gen-attn-om`, stacked on
`origin/feat/api-kernel-gen-om` at `bac386c`. The source-only registration
commits are `35205e50` and `4acdb918`. Shared registration is integration-owned
commit `20b33b52`, merged before the attention C cutover. Its aggregate identity
staging omission was repaired by parent commit `aff2669c`, applied here as
`51e0f36b`. The integration PR is
[CUDApp #340](https://github.com/aien-dev/CUDApp/pull/340); this lane opens no PR.

The default attention generator is `omega_attention_ir`, computed by:

- `om/kgen/attention_emit.om`: exact record serialization, virtual registers,
  scheduler scoreboards, batched loads, loop intervals and reconvergence.
- `om/kgen/attention_tuned.om`: HD64/HD128, both storage formats, GQA groups,
  direct/split outputs, divergent-region and existing behavioral controls.
- `om/kgen/attention_general.om`: every head dimension from 1 through 256,
  whole-warp thread rounding, dimension tails and the general controls.
- `om/kgen/attention_ir.om`: checked packed-spec dispatch and scratch layout.

`src/omega_gpu_attention_api.c` calls the existing single
`omega_gpu_om_ir_replay` entry. No replay entry, opcode, allocator trust path,
physical register budget or public API is added or changed. The replay already
solves allocation and checks closed-loop invariants; the new C path does not
solve allocation twice. The old emitters and scheduler compile only with
`CUDAPP_MIGRATION_PENDING_C_API_KGEN=1`, default **0**.

Each instruction is exactly eight `u64` words: opcode; destination; three
sources; uniform register; immediate plus control in the upper word; flags.
Metadata has 512 `u64` words. Fields 0/1/2 are instruction/GPR/UGPR counts;
4 is emitter error; 5 is closed-loop count; 16..31 hold loop intervals;
64..319 and 320..383 hold virtual register widths. Internal scoreboard/region
scratch is private to the emitter. Fields **6 and 7 are cleared on success**:
attention uses the pre-existing default allocator, not the extended allocator.

Remaining C in this file is API validation/host-plan dispatch, allocation,
cache ownership, artifact encoding/digest calls and device launch glue. The
existing host launch-shape helpers remain unchanged; the assigned migration
is the attention IR generator, not another host-view/launch-plan cutover.
The handwritten C generator remains only as the default-OFF differential
oracle. Generated C is checked output of the first-party bounded Omega bridge,
not a second handwritten implementation.

Compiler: seeded `omega-mv@e5b32567`, SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Bridge: existing integration checkout `/tmp/integ/bridge-73-79e8c36` at
`79e8c36`, the same converter used by the stacked parent. The independent
bootstrap compiler path is `~/work/cudastack/omega-bridge/build/bootstrap/omc`.
Host compiler and gcov: Ubuntu GCC `13.3.0-6ubuntu2~24.04.1`; bubblewrap `0.9.0`.
These are verification/build tools, not replacement attention implementations.

## Differential and shape partition

`tests/api_kernel_gen_attention.c` includes the retained C generator with the
migration flag ON. Before either allocator runs, it compares every field of
every eight-word instruction record, both virtual register width spaces and
all closed-loop endpoints. It then compares allocation success/refusal,
the full solved allocator (including uniform allocation), encoded instruction
bytes, and complete final cubin containers. Both containers use identical valid
attention parameter/shared-memory descriptors and independently derived
declared register counts. A perturbed-record invocation flips a serialized
immediate; a second invocation flips the final byte of the packed Omega cubin.
Both must return **1**. The make target requires those red results rather than
treating a crash or generation failure as a passing negative control.

The original receipts below establish IR/encoded-text equality only; they did
not compare final cubin bytes. The acceptance extension and its separately
preserved receipts are in [cubin-followup/README.md](cubin-followup/README.md).

The partition is derived from `gen_attention`, `gen_attention_general`,
`heads_per_cta`, `gen_threads` and the admitted `KSpec` fields:

- Both BF16 and F32.
- Every general head dimension 1..256, thus every D/4 tail and every warp
  boundary. HD0, HD257 and `UINT32_MAX` are refused. Tuned HD64/HD128 plus
  adjacent/representative invalid values are checked independently.
- Tuned `log2bs` and `log2gqa` each exhaust 0..31, including all shift values
  and the GQA-group transitions 0/1/2/3. This is broader than the public tuned
  plan, which admits `log2bs <= 16` and `log2gqa <= 6`.
- General block shifts 0/1/30/31 on every head dimension and every general
  mutant value 0..5 exercise the ordinary and saturating slot masks.
- Every split value -1..4 (direct output and all supported partial counts).
  Split5, reserved kind bits and invalid selector values are refused.
- Every tuned mutant 0..6, divergent value 0..2 and GQA group 1/2/4, in both
  formats and both tuned dimensions. General mutants 1 and 5 intentionally
  leave the generator unchanged; their host-side behavior belongs to the
  existing API path, while generator byte equality is still checked.
- Shift values 32/33/`UINT32_MAX`, invalid tuned dimensions including
  63/65/127/129, and malformed packed selectors are refused before unsafe
  shifts or dimensions can reach the retained C oracle. Inputs that the public
  plan cannot admit are refusal tests, not C undefined-behavior experiments.

`tests/attention_kgen_mutants.py` independently perturbs five Omega semantics:
descriptor offset, query argument offset, thread special register, scalar
argument offset and the scheduler stall-control field. Every mutant must
regenerate, compile, link and fail the behavioral differential with rc 1.
A missing textual site, compiler failure, crash or surviving mutant fails the
gate; none counts as a kill.

## Observed acceptance

The final generator differential reports **39,423 cases**, with raw records and
encoded bytes identical. The deliberately corrupted record returns **1** and
is detected. All five independently regenerated/compiled Omega mutants return
**1** with record mismatches; the mutation gate returns **0**.

All commands below ran under the wrapper in the next section. Full outputs
are the corresponding `.log` files; `rc.tsv` consolidates their observed
results, retaining successful earlier rows when selected gates are rerun.

| Command | rc | Observed final line |
|---|---:|---|
| `make -j3 -f Makefile -f tests/attention_kgen.mk test-api-kgen-attention` | 0 | `PASS attention red control detected perturbed IR record` |
| `make -j3 -f Makefile -f tests/attention_kgen.mk test-api-kgen-attention-mutants` | 0 | `PASS attention seeded semantic mutants: 5 killed` |
| `make -j3 test-gpu-attention` | 0 | `PASS: 348 checks, 0 failed` |
| `make -j3 test-gpu-decode-attn` | 0 | `gpu_decode_attn_test: 12 checks, 0 failed: PASS` |
| `make -j3 test-gpu-decode-attn-maxpos` | 0 | `test-gpu-decode-attn-maxpos: 7 of 7 controls killed (5 binding mutants)` |
| `make -j3 test-gpu-decode-attn256` | 0 | `gpu_decode_attn256_test: 14 checks, 0 failed: PASS` |
| `make -j3 test-gpu-prefill-host` | 0 | `test-gpu-prefill-host: PASS (8 prefill tests, host model + warp simulator, no device)` |
| `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` |
| `crumb compile .` | 0 | `crumb compile: 46 crumbs, 0 rewritten` (HEAD recorded in log) |
| `crumb verify .` | 0 | `crumb verify: OK: 46 crumbs current` |

The first default-path build exposed a common `rc` declaration accidentally
inside the migration guard; it was moved outside the guard before the listed
successful host run. The first full unit regeneration check then exposed the
parent aggregate identity omission described above. Only the regeneration and
crumb gates were rerun after that manifest-only repair; the generator, encoded
output and host test sources were unchanged. These are resolved failures, not
waived gates.

| Tier / delivery state | Verdict |
|---|---|
| IMPLEMENTED: seeded attention emitters and default replay cutover | PASS |
| EXERCISED: raw IR / encoding differential and negative control | PASS |
| EXERCISED: five semantic Omega mutants | PASS |
| EXERCISED: required host/model/simulator targets | PASS |
| EXERCISED: regeneration and crumb gates | PASS |
| QUALIFIED: physical chip | NOT_RUN — no authorized GPU window |
| MERGED / DEPLOYED | NOT_RUN — integration owner retains custody |

## C oracle coverage

The retained C oracle fixture is compiled with `-O0 --coverage`. After the
serial differential and mutant gates, this command ran with the same bwrap,
affinity and nice wrapper, from this evidence directory:

```sh
gcov --json-format -b -c -f \
  "$HOME/work/cudastack/lanes/KernGenAttnOm/build/api-kgen/attention_diff.gcno"
```

Observed rc **0**, final line `Lines executed:51.74% of 1380`. The complete
machine-readable receipt is `attention_diff.gcov.json.gz`. That gcov receipt
retains its original build/source paths. Gcov reports `gen_attention` **100.00%
of 312 lines**, `gen_attention_general` **100.00% of 176 lines**, and no
untaken branch edges inside either generator. The whole included API file is
51.25% of 1204 lines because the direct oracle intentionally bypasses the API,
cache, allocation and launch functions; those unchanged host interfaces are
exercised separately by the required host targets, not claimed as covered by
this oracle profile.

Every untaken oracle helper branch is accounted for below. Line numbers refer
to `src/omega_gpu_attention_api.c` at this cutover:

| Lines | Untaken outcome and reason |
|---|---|
| 82 / 94 | Invalid scheduler-kind default; every source emitter passes a declared kind. |
| 89 | Predicated EXIT; attention's tail always emits an unconditional EXIT. |
| 90 | Unconditional scheduler BRA; source loop/region branches are predicated, and the final self-branch is appended directly. |
| 96, 99–102, 140, 149, 215 | Append/register exhaustion; all admitted partitions fit the existing fixed emitter limits. This is not an allocation-failure injection profile. |
| 136 | Invalid scoreboard / excess batch size; source scoreboards and batches are bounded constants. |
| 147 | Empty-batch alternative; every source caller of the fixed-in-batch helper has opened a load batch. |
| 171, 172, 175 | Shuffle/barrier inside an open divergent region; the structured source closes regions before those operations. |
| 180 | Invalid forward-patch location; patch locations are obtained from the current emitted stream. |
| 186, 189, 197, 200, 209 | Region-stack exhaustion, missing/open-region corruption and helper failures; source nesting is bounded and balanced, including the intentional divergent controls. |
| 195 | Unconditional region exit; the attention source uses conditional region exits. |
| 220, 224 | Loop-stack/count exhaustion and unmatched loop end; the source loop nesting and closures are fixed and balanced. |
| 230 | A combined live-interval-extension short-circuit outcome is absent from the emitted intervals; shape partitions do not alter that allocator safety condition. |
| 242 | Loop-local value used after its loop; all admitted generated loops satisfy the invariant. |
| 244 | Uniform-destination opcode alternatives in the GPR redefinition check; uniform loads do not redefine the matching GPR. |

In `omega_bw_reconv.h`, untaken edges at lines 42/45/50/58/60/70/73/74/78
are the same capacity/invalid-region/invalid-record guards: valid source
regions never exhaust the stack/exits or provide invalid join/branch sites.
All 28 helper lines execute, but invalid-state fault injection is not claimed.
Zero-hit functions after the generator in the included API file and in its
view/table headers are outside this direct-emitter oracle; they are not silently
counted as generator coverage. Differential diagnostic error branches likewise
remain mostly cold because equality is the expected result; the explicit red
record and semantic mutants exercise the record mismatch path.

## Reproduction and safety

All test, simulator, differential and mutant executions inherit private `/dev`
from this exact outer command. No GPU, live-RM or VFIO device is exposed.
No service, lock, GPU module or firmware state is changed.

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 \
  env OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc" \
      OMEGA_BRIDGE=/tmp/integ/bridge-73-79e8c36 \
      OMEGA_SEED="$HOME/work/cudastack/omega-bridge/build/bootstrap/omc" \
  python3 tests/attention_kgen_gate.py
```

The runner serializes the named gates, limits make to `-j3`, writes each
command's full output to its own `.log`, and records the return code and final
line in `rc.tsv`. `test-gpu-attention` itself covers host, simulator, sweep,
HD128 host/simulator, divergent HD128 simulator and general simulator modes.
`test-gpu-decode-attn` also depends on the max-position gate; that gate is run
explicitly once more because it is independently named in the assignment.
`test-gpu-decode-attn256` and `test-gpu-prefill-host` are host/simulator-only.
No full `make test` is run by this lane. Paths in the original runner's logs are
normalized from the local home directory to `~`; commands and other output are
unchanged. Follow-up receipts describe their own capture conventions.

## Wolfram queries

The source literal conversions and adjacent shape boundaries were obtained with:

```text
WolframLanguageEvaluator:
{FromDigits[#,16]& /@ {"ff800000","3f800000","ffff0000","80000000","000fc000","358","380","388","390","398","3a0","3a4","3a8","3ac","3b0","3b4","3b8","3bc"},
 Table[{v-1,v,v+1},{v,{1,2,4,6,31,32,64,128,256}}]}
-> {{4286578688,1065353216,4294901760,2147483648,1032192,856,896,904,912,920,928,932,936,940,944,948,952,956},
    {{0,1,2},{1,2,3},{3,4,5},{5,6,7},{30,31,32},{31,32,33},{63,64,65},{127,128,129},{255,256,257}}}

WolframLanguageEvaluator:
{BitOr[15,BitShiftLeft[255,8],BitShiftLeft[7,40]],BitOr[11,512,65536],BitOr[12,1024,65536],BitOr[49,512],BitOr[50,1024],BitOr[48,1280],BitOr[45,1536],BitOr[46,3072],BitOr[54,2048]}
-> {7696581459727,66059,66572,561,1074,1328,1581,3118,2102}
```

## Chip runbook — NOT_RUN

There is no GPU window for this lane. Host/model/simulator evidence is not
hardware qualification. Do not execute this runbook until Drake grants the
current exclusive window and the current hardware gate/runbook is followed.
A stale receipt or a free-looking lock is not authorization.

In the authorized window, record the source/compiler/bridge revisions and
regenerate-check the committed units. Build the default-Omega library, preserve
the physical register budget, and use the existing `gpu_attention_test` chip
modes for HD64, HD128, GQA/split and general shapes. Run existing decode and
prefill chip gates from the same build. Record exact commands, device and driver
identity, output bytes against existing references, permitted negative controls
and passive driver fault evidence. Use the current operator procedure for GPU
custody, service suspension/restoration and chip receipt capture. Never run an
unapproved mutant on hardware or weaken refusal/memory/clock gates to proceed.
The integration owner decides whether those receipts qualify the change; this
lane neither merges nor deploys it.

The existing test binaries parse the following chip payloads when neither
`--host-only` nor `--sim` is supplied. They are **NOT_RUN**, and are not
authorization to execute. The operator must use the current approved chip
launcher/custody procedure, explicitly authorize the battery's built-in negative
controls, and maintain the prescribed affinity and build concurrency limits.
The private-`/dev` host wrapper above cannot produce a chip receipt.

```sh
./build/gpu_attention_test --out chip-attn64.json
./build/gpu_attention_test --hd 128 --out chip-attn128.json
./build/gpu_attention_test --sweep --out chip-attn-sweep.json
./build/gpu_attention_test --general --out chip-attn-general.json
./build/gpu_decode_attn_test --out chip-decode-attn.json
./build/gpu_decode_attn256_test --out chip-decode-attn256.json
./build/gpu_prefill_attn_test --out chip-prefill-attn.json
./build/gpu_prefill_attn256_test --out chip-prefill-attn256.json
```

Build targets are the corresponding `build/<binary>` paths, with the migration
flag left at its default zero. Stop on a gate refusal, unavailable window,
unexpected device state, fault or failed positive/negative control; preserve
the receipt and follow the operator's restoration procedure. Do not convert a
refusal, simulator result or skipped command into PASS.
