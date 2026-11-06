# CheckersRest2 hosted migration receipt

IMPLEMENTED and EXERCISED on branch `feat/checkers-rest-om`, code revision `8f76aaf`. This receipt and the dated roadmap note are documentation-only additions after those checks. QUALIFIED (hardware), MERGED and DEPLOYED: NOT_RUN. No full `make test` was run.

## Source pins and execution boundary

- Seeded omega-mv compiler: `e5b32567`, omc SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Converter: omega-bridge #73, `79e8c3613a6f6f9e8eaaa9c013fc25bcb39e0538`, `src/toc.om` SHA-256 `d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538`.
- Separate pin commit `e2c0e70` is inherited through the stack on CUDApp #325 (`5b1e69c7b7d74e5c823f2fa41bd58950b11aa59a`). This lane remains pending omega-bridge #71 and #73, both open drafts when checked.
- All commands below ran from the CheckersRest worktree. Every test, mutant, differential, simulator and make-test invocation used the exact boundary below. Builds were at most `-j3`. No live GPU, service, driver, VFIO or lock was touched.

```sh
B='bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10'
# Prefix every command in the table with $B and these variables except where the table specifies its own env:
env OMEGA_SEED=$HOME/work/cudastack/omega-bridge/build/bootstrap/omc \
    OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc \
    OMEGA_BRIDGE=/tmp/integ/bridge-73-79e8c36
```

## Acceptance commands and observed final lines

| Command after the boundary/environment above | rc | Observed final line |
| --- | --- | --- |
| `sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` |
| `make -j3 test-gpu-reconv` | 0 | `PASS: 119 checks, 0 failed` |
| `make -j3 test-gpu-gpr-boundary` | 0 | `gpu_gpr_boundary_test: 298 checks, 0 failed: PASS` |
| `make -j3 test-bw-hazard` | 0 | `bw_hazard_test: 264 checks, 0 failed: PASS` |
| `sh tests/regalloc_om/run.sh` | 0 | `PASS bind differential cases=50686 refused=38401` |
| `python3 tests/regalloc_om/mutants.py` | 0 | `PASS recompiled Omega semantic mutants=20` |
| `make -j3 test-checkers-om` | 0 | `PASS checker recompiled Omega semantic mutants=10` |
| `make -j3 test-frag-packers-om` | 0 | `PASS fragment recompiled Omega semantic mutants=6` |
| `env PHYSICS_DIR=. GPU_ENGINE_BMUT_OUT=build/checkers-backend-mutants.json sh tools/gpu_engine_backend_mutations.sh` | 0 | `mutants: 120 total, 107 killed, 13 survived as redundant, 0 survived as test gaps (receipt build/checkers-backend-mutants.json)` |

The two named semantic targets also reported:

```text
PASS checker differential, ordered failure state and red control
PASS fragment metadata, short-view refusal and row-compaction bitwise differential
PASS five fragment packers and result sentinels bitwise differential, source partition and row0 ASan regression
```

The final allocator runner additionally reported:

```text
PASS scalar register and matmul policies cases=10000 including uint32 wrap boundaries
PASS regalloc differential cases=181614 refused=179089 encoded=2525 record-flip=caught
PASS signed GE host-reference cases=98 register and immediate
PASS existing encoder golden fixtures and vecadd SHA-256
PASS matmul warp-sim cases=15 CPU oracle bit-identical
PASS hazard decode differential cases=431079 refused=135482
PASS hazard check differential cases=40495 refused=16278 findings=107648 capped=1 fixtures=485 c_s=0.852 omega_s=88.745
PASS bind differential cases=50686 refused=38401
```

Raw final gate logs remain at `/tmp/checkers-rest-final-gates/{units-check,gpu-reconv,gpu-gpr-boundary,bw-hazard,regalloc,regalloc-mutants}.log` in the working environment. The differential outputs, source-coverage files, sanitizer executables and recompiled-mutant outputs remain in `build/checkers-om/`, `build/packers-om/` and `build/regalloc-om/`; binaries are deliberately not committed.

The optional backend JSON has a separate pre-existing shared-runner defect: `tools/mutation_runner.sh:183` does not JSON-escape all fields. Its line for `BE_WAIT_EQUALITY_RECHECK` emits raw double quotes around `>=`, so the JSON is invalid even though the sweep exits 0 and reports all rows. This receipt uses the observed stdout, not a parsed JSON claim. The smallest reproduction and required lossless-escaping repair were handed to Integrator4, outside this lane's assigned adapter ownership. No row was weakened or sanitized to hide the defect.

## Source partition, negative controls and mutants

`checkers.c` derives cases from the predicates in all three emitter/reconvergence headers: emitter classes, predicates, pending/held scoreboard state, append and allocation outcomes, prefetch/consume boundaries, loop depth and interval comparisons, logarithm boundaries, region depth/capacity, every join refusal and ordered late-failure writeback. Joins use BRA=46, BSSY=54, BSYNC=55. The C oracle's annotated `omega_bw_em.h.gcov`, `omega_bw_em_q.h.gcov` and `omega_bw_reconv.h.gcov` contain no zero/never-executed branch outcome. The red output record must mismatch.

Ten recompiled checker semantic mutants compiled successfully, executed successfully and differed from the full oracle output: pending mask, predicate preservation, prefetch hold, consume release, loop capacity, interval equality, BSSY opcode, exit opcode, depth decrement and relative immediate. Compile failures and traps are not credited.

The fragment partition covers all five packers, empty and partial tiles, extra complete padding tiles, B column subranges, guarded whole destinations, f32 rounding ties, subnormals, infinities and NaN bit patterns. It also compares sentinel counts and complete buffers for signed zeros, NaNs and positive/negative set flags. `gcov --coverage -O0` reports every branch outcome taken for the owned C packers, table initializer, conversion helper and result sentinels; unrelated functions in the same translation unit are intentionally outside that differential. The additional-byte red control must mismatch. Metadata and short-view refusals must preserve output, and mrow overlap is refused by the portable ABI. Six recompiled packer mutants compiled and executed with rc 0 and produced different full outputs: A column mapping, A lane stride, B stride, padding, bf16 tie rounding and row0 padding.

The hazard generator forces an LDG size-field-4 instruction with a live destination and compares `nr`, `nw` and all read/write range lengths. `hazard-size-class` is killed with `FAIL hazard differential failures=44870`. The final existing allocator/hazard/bind mutant runner kills 20 non-equivalent mutants.

`hazard-branch-sign` remains explicitly listed EQUIVALENT and is not counted as killed. Mounted WolframLanguageEvaluator query, recorded verbatim in `tests/regalloc_om/hazard-branch-sign-proof.txt`:

```wolfram
{Reduce[0 <= s < 2^30 && 0 <= l < 64 && Mod[64 (s + 3221225472) + l, 4294967296] != Mod[64 (s + 1073741824) + l, 4294967296], {s,l}, Integers], FullSimplify[Mod[64 (s + 3221225472) + l, 4294967296] == Mod[64 (s + 1073741824) + l, 4294967296], Element[{s,l},Integers]], {BitAnd[3221225472,2^30-1],BitAnd[1073741824,2^30-1],64 (3221225472-1073741824)/4294967296}}
```

Observed result: `{False, True, {0, 0, 32}}`. This searches the complete normalized word domain and proves equivalence, not a sampled failure to find a witness.

## Defects fixed separately

- `25a698c`: source-sized row0 packing (`k=1,kp=48`) reproduced ASan heap-buffer-overflow, rc 1, because unsigned `k-c0` underflowed in complete padding tiles. The corrected C oracle checks `c0>=k` before subtraction; the sanitizer regression now exits 0.
- `22ae637`: `log2u(UINT32_MAX)` reproduced UBSan rc 1, `shift exponent 32 is too large for 32-bit type 'unsigned int'`, using `cc -std=gnu11 -O0 -fsanitize=undefined -fno-sanitize-recover=undefined -ffunction-sections -fdata-sections -Isrc -DCUDAPP_MIGRATION_PENDING_C_EM -DCUDAPP_MIGRATION_PENDING_C_RECONV tests/regalloc_om/checkers.c -Wl,--gc-sections -o build/checkers-om/log-overflow`, followed by `build/checkers-om/log-overflow --log-overflow` inside the boundary. The bounded C search and expanded differential now pass; `checkers.sh` repeats the UBSan gate and requires `log-overflow -1`.
- `8f76aaf`: backend mutation adapter now links `src/cudapp_om_cg_bind.c` and the other portable providers reached by codegen wrappers, rather than failing at link time.

## Providers and C deliberately retained

- `cg_em.om` and `cg_reconv.om`: fixed-view step units. C header wrappers retain record construction/traversal, allocation/append effects and ordered copying of results back to records. Pure control/scoreboard/admission/patch decisions default to Omega. Old decisions are behind default-off `CUDAPP_MIGRATION_PENDING_C_EM` and `CUDAPP_MIGRATION_PENDING_C_RECONV`.
- `cg_pack_len.om`: fixed 12-word metadata, lengths/admission in Omega. `cg_pack_h.om`: fixed metadata plus unsized source-byte and destination-ushort views, five physical native slots. Shapes are checked without an extension or staging copy. Old five-pack C bodies are behind default-off `CUDAPP_MIGRATION_PENDING_C_PACK`.
- `cg_mrow.om`: canonical fixed `{m,n,np}` metadata plus two unsized f32 views; copies bitwise to a disjoint destination. ApiViewsRest2 was given the stable provider ABI from `343a061`; no duplicate provider was created.
- `cg_sentin.om`: fixed `{m,n,np,set_as_uint32,poison_f32_bits}` plus editable f32 view. C only marshals poison bits, extent and return status; tile selection/counting/comparison is Omega. The retained C sentinel is under C_PACK.
- `OMEGA_GPU_NO_POSIX` still selects the C verification bodies because this freestanding path cannot yet call these hosted portable OM2/runtime-view units. This is explicit retained C, not a claim that the entire CUDA stack is already migrated.
- Generated `src/cudapp_om_*.c` is checked converter output, not a handwritten computation fallback. The pre-existing compiler/bridge pins are unchanged except for the separately inherited #73 pin commit. No compiler syntax, host-word, repository-ownership or hardware-gate policy is changed; the CUDApp unit guide and dated roadmap note are updated.

## Hardware follow-through

Chip, live-RM, VFIO and performance qualification: NOT_RUN. The operator-owned deferred chip runbook is in `tests/regalloc_om/CHECKERS.md`. No hardware result, merge approval or deployment is implied by these host receipts.
