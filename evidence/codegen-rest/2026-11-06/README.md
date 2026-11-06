# CodegenRest receipt — 2026-11-06

## Outcome and scope

IMPLEMENTED: attn256, pattn256, router, FP8 GEMM, BF16 GEMV, pq and qg now default to first-party seeded Omega. The six retained C builders are differential-only under their default-off migration flags. Public API unchanged. EXERCISED: CPU differential, gcov, controls, seven recompiled Omega mutants, host checks and six-family warp simulation PASS. Hardware QUALIFIED: NOT_RUN. MERGED: no. DEPLOYED: no.

Branch: `feat/omega-codegen-rest`, based on `22ebf8f`. Registration commit `3334df7`; approved isolated replay meta[6] commit `5604269`. The implementation revision is the commit containing this receipt. Pinned omega-mv: `e5b32567`; omc SHA256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. Existing Omega Bridge exporter only; no new host dependency or compiler pin.

Remaining C is the permitted memory/ABI shell: spec widening/result copies, program allocation/statistics wrappers and the shared replay/allocator/encoder path. No family admission, geometry, scheduling or instruction-selection logic remains in the default C builders. No compiler/bridge extension is required by these six ports. RegAllocOm owns meta[7]; this lane only changes approved meta[6]. See `om/codegen/README.md` for the root wire and source-to-state mapping.

## Domain and measured differential

The domain is **partition plus boundaries derived from the C branch conditions, branch coverage X/Y** (the measured X/Y for each builder is below), not a uint32 Cartesian product. The retained builders are independently compiled with GCC 13.3.0 `-O0 --coverage -ffp-contract=off`. Each family receives at least 10,000 deterministic generated scalar perturbations, 0 and UINT32_MAX in every u32 field, and threshold-minus-one/threshold/threshold-plus-one cases. Additional valid coupled seeds prevent earlier predicates from masking ROWS alignment, XBATCH output offsets, head-ratio and qg-immediate boundaries.

The finite control partition covers all admitted K steps and all resource-changing geometry/mode/mutant classes: attention depth/prep/multi/H/G/old-vs-general head layout; prefill group sizes; router warp/depth/expert/shared/ROWS forms; GEMM lane bytes/MT/warps/rowmap/unroll classes; BF16 rows/warps/depth/mask/batch/stride forms; pq input/group forms and qg RT/warp/epilogue/direct/wait/group/output-span forms. Wide valid head pairs cover full-width immediate arithmetic. Equal wide non-power query/KV head pairs additionally exercise the repaired attention pre-logarithm refusal. Scalar values that only change immediate payloads are represented by their source-derived boundaries and generated inputs.

Every accepted case compares every IR field, the complete allocation structure including live intervals/assignments, encoding return/length/bytes, and the default public shell against the independently renamed C builder. Refusals and public query results also match. Resource maxima include constructed refused cases, not only successful solves.

| Family | Cases | Accepted | Max IR count | Max virtual regs | Lines | Branches |
| --- | ---: | ---: | ---: | ---: | --- | --- |
| attn256 | 54781 | 5604 | 901 | 167 | 515/516 | 267/294 |
| pattn256 | 14742 | 383 | 632 | 111 | 279/280 | 161/192 |
| router | 40134 | 4635 | 672 | 114 | 340/341 | 236/260 |
| gemm | 101152 | 9138 | 1024 | 256 | 275/276 | 235/250 |
| bf16 | 332689 | 49846 | 997 | 96 | 213/214 | 202/222 |
| qg | 155749 | 20571 | 892 | 106 | 232/233 | 269/301 |

`coverage.json` contains the measured per-source counts and every uncovered arc. Branch indices are zero-based within gcov's branch array for that source line. The following inventory accounts for every uncovered arc and uncovered statement; no uncovered input-admission branch is silently treated as covered.

### Why remaining alternatives cannot be reached

- **E**: Unreachable diagnostic at this constructor prefix: only fixed-array append/allocation failure or an already-set emitter error can select it. The source-derived finite shape partition covers every resource-changing prefix; no caller-supplied IR, allocation limit, scheduler kind or loop stack enters these builders. Reachable GEMM capacity refusals are exercised separately, not excluded. These remaining sites cannot be the first failing operation of an admitted prefix.
- **K**: The switch/default error arm cannot be selected: every call passes one of the locally declared K_* constants, and the switch handles all of them. The uncovered default statement is the same unreachable arm.
- **X**: Every emitted EXIT is constructed from I0 and remains unpredicated. Therefore its predicate-true scheduling alternative cannot occur.
- **B**: All prefill branches passed through emit use bra_if_p0 or bra_if_p0_fwd, which explicitly set predicate_p0=true. The unpredicated terminal self-branch is appended directly, not passed through this conditional.
- **L**: The source balances its fixed loop_begin/loop_end calls and never exceeds its fixed nesting bound. A malformed external loop stack is not an input.
- **U**: Each loop is closed after a branch with no virtual-register operands. Thus no virtual-register last read can be the final branch instruction; the last_use<end false arc cannot occur.
- **I**: All prefill virtual registers visited by this loop closure have an explicit definition. The first_def<0 alternative cannot occur in this constructor.
- **S**: Zero head fields are replaced by positive defaults before this condition. Hence h<1 and/or kvh==0 cannot occur at this shape check; invalid nonzero ratios are covered.
- **P**: All log2 arguments are powers of two after admission: lane_bytes is 8 or 16, mt is a nonzero power of two, LPB=128/lane_bytes and ROWW=32*lane_bytes. Therefore log2u cannot return -1 and the downstream negative-result alternatives cannot occur.
- **R**: For admitted warps 8,16,32, W/4 is even and q advances by two from zero. Consequently q+1<W/4 is always true in these paired-row loops.
- **F**: rows<=4 and normalized xbatch<=4 are already established, so rows*xbatch<=16. The >16 alternative is unreachable; the neighbouring ystride/shared-memory alternatives are covered.
- **D**: Earlier admission bounds rows<=4, k<=16384 and depth<=16. Even their loose combined upper bound on this offset is 106496, below the signed-immediate bound 8388608. The rejecting alternative is impossible.
- **Q**: The immutable plan comes only from om/prefill-shape/order.om: at most six records, opcodes 1..6, role indices 0..3, bounded immediates. Guarded plans have exactly one branch record with a nonnull initially-negative branch slot; unguarded plans have none. Invalid count/role/immediate/opcode, repeated/null branch, and downstream adapter-error alternatives therefore cannot occur.
- **C**: The fixed-size native Omega call has valid ABI arguments and is deterministic. Its exporter-error alternative cannot occur; any bad shape/order is returned as the ordinary boolean admission value instead.
- **A**: Earlier native_qgshape/qg_offset admission already refuses epi>1 and n==0, so these later duplicate alternatives cannot occur.
- **G**: rt=8 requests the existing BW_PHYS_GPR_HW_LIMIT through the validated setter. This is an accepted constant, not a caller-controlled limit; setter failure cannot occur.
- **O**: The same deterministic order query already succeeded during spec admission. Repeating it with the same group/column count and guard=1 cannot fail; guard=0 only removes records and cannot invalidate that admitted plan. The native ABI arguments are fixed and valid.

E is a per-prefix claim, not an assertion that every geometry fits the resource arrays: GEMM's reachable capacity/allocation refusals are in the corpus and covered. Its residual V128/UV64/prologue checks occur before the capacity-demanding constructor sections; later residual checks cannot be selected as the first failure of the finite admitted emitter-prefix classes. The other builders' fixed validated geometry classes stay within their append/allocation prefixes. No fault injection into private emitter state is an API input.

### `src/omega_blackwell_codegen_attn256.c`

| Line | Uncovered arcs / statement | Function | Reason |
| ---: | --- | --- | --- |
| 37 | 11 | `emit` | K |
| 45 | 0 | `emit` | X |
| 49 | statement | `emit` | K |
| 51 | 0 | `emit` | E |
| 59 | 0 | `V` | E |
| 60 | 0 | `V64` | E |
| 61 | 0 | `V128` | E |
| 62 | 0 | `UV64` | E |
| 113 | 1 | `patch_bra` | E |
| 121 | 0 | `tail` | E |
| 123 | 0 | `loop_begin` | L |
| 125 | 0 | `loop_end` | L |
| 130 | 7 | `loop_end` | U |
| 154 | 1, 7 | `legacy_omega_dattn256_shape` | S |
| 199 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 209 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 215 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 256 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 318 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 320 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 333 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 370 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 384 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 492 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 495 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 586 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |
| 642 | 0 | `legacy_omega_blackwell_codegen_decode_attn256_prog` | E |

### `src/omega_blackwell_codegen_pattn256.c`

| Line | Uncovered arcs / statement | Function | Reason |
| ---: | --- | --- | --- |
| 38 | 11 | `emit` | K |
| 46 | 0 | `emit` | X |
| 47 | 1 | `emit` | B |
| 50 | statement | `emit` | K |
| 52 | 0 | `emit` | E |
| 60 | 0 | `V` | E |
| 61 | 0 | `V64` | E |
| 62 | 0 | `V128` | E |
| 63 | 0 | `UV64` | E |
| 112 | 1 | `patch_bra` | E |
| 119 | 0 | `tail` | E |
| 121 | 0 | `loop_begin` | L |
| 123 | 0 | `loop_end` | L |
| 128 | 1, 7 | `loop_end` | I, U |
| 139 | 0 | `hmma` | E |
| 143 | 0 | `nop` | E |
| 150 | 0 | `lds_pf` | E |
| 162 | 1 | `legacy_omega_pattn256_shape` | S |
| 175 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 179 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 187 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 200 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 207 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 209 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 234 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 236 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 265 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 303 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 350 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 381 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |
| 382 | 0 | `legacy_omega_blackwell_codegen_prefill_attn256_prog` | E |

### `src/omega_blackwell_codegen_router.c`

| Line | Uncovered arcs / statement | Function | Reason |
| ---: | --- | --- | --- |
| 36 | 11 | `emit` | K |
| 44 | 0 | `emit` | X |
| 48 | statement | `emit` | K |
| 50 | 0 | `emit` | E |
| 58 | 0 | `V` | E |
| 59 | 0 | `V64` | E |
| 60 | 0 | `V128` | E |
| 61 | 0 | `UV64` | E |
| 107 | 1 | `patch_bra` | E |
| 115 | 0 | `tail` | E |
| 117 | 0 | `loop_begin` | L |
| 119 | 0 | `loop_end` | L |
| 124 | 7 | `loop_end` | U |
| 191 | 0 | `legacy_omega_blackwell_codegen_router_prog` | E |
| 196 | 0 | `legacy_omega_blackwell_codegen_router_prog` | E |
| 223 | 0 | `legacy_omega_blackwell_codegen_router_prog` | E |
| 236 | 1 | `legacy_omega_blackwell_codegen_router_prog` | R |
| 238 | 1 | `legacy_omega_blackwell_codegen_router_prog` | R |
| 265 | 0 | `legacy_omega_blackwell_codegen_router_prog` | E |
| 306 | 0 | `legacy_omega_blackwell_codegen_router_prog` | E |
| 359 | 0 | `legacy_omega_blackwell_codegen_router_prog` | E |
| 380 | 0 | `legacy_omega_blackwell_codegen_router_prog` | E |
| 386 | 0 | `legacy_omega_blackwell_codegen_router_prog` | E |
| 426 | 0 | `legacy_omega_blackwell_codegen_router_prog` | E |
| 447 | 0 | `legacy_omega_blackwell_codegen_router_prog` | E |

### `src/omega_blackwell_codegen_gemm.c`

| Line | Uncovered arcs / statement | Function | Reason |
| ---: | --- | --- | --- |
| 38 | 11 | `emit` | K |
| 51 | 0 | `emit` | X |
| 55 | statement | `emit` | K |
| 67 | 0 | `V128` | E |
| 68 | 0 | `UV64` | E |
| 114 | 0 | `loop_begin` | L |
| 116 | 0 | `loop_end` | L |
| 131 | 0 | `ldw_pf` | E |
| 135 | 3 | `log2u` | P |
| 173 | 1, 3, 4 | `legacy_omega_blackwell_codegen_fp8gemm_prog` | P |
| 185 | 0 | `legacy_omega_blackwell_codegen_fp8gemm_prog` | E |
| 229 | 0 | `legacy_omega_blackwell_codegen_fp8gemm_prog` | E |
| 282 | 0 | `legacy_omega_blackwell_codegen_fp8gemm_prog` | E |
| 287 | 0 | `legacy_omega_blackwell_codegen_fp8gemm_prog` | E |

### `src/omega_blackwell_codegen_bf16.c`

| Line | Uncovered arcs / statement | Function | Reason |
| ---: | --- | --- | --- |
| 40 | 11 | `emit` | K |
| 53 | 0 | `emit` | X |
| 57 | statement | `emit` | K |
| 59 | 0 | `emit` | E |
| 67 | 0 | `V` | E |
| 68 | 0 | `V64` | E |
| 69 | 0 | `V128` | E |
| 70 | 0 | `UV64` | E |
| 107 | 1 | `patch_bra` | E |
| 114 | 0 | `tail` | E |
| 119 | 0 | `loop_begin` | L |
| 121 | 0 | `loop_end` | L |
| 126 | 7 | `loop_end` | U |
| 143 | 5 | `spec_ok` | F |
| 148 | 0 | `spec_ok` | D |
| 166 | 0 | `legacy_omega_blackwell_codegen_bf16_gemv_prog` | E |
| 173 | 0 | `legacy_omega_blackwell_codegen_bf16_gemv_prog` | E |
| 190 | 0 | `legacy_omega_blackwell_codegen_bf16_gemv_prog` | E |
| 203 | 0 | `legacy_omega_blackwell_codegen_bf16_gemv_prog` | E |
| 209 | 0 | `legacy_omega_blackwell_codegen_bf16_gemv_prog` | E |
| 296 | 0 | `legacy_omega_blackwell_codegen_bf16_gemv_prog` | E |

### `src/omega_blackwell_codegen_qg.c`

| Line | Uncovered arcs / statement | Function | Reason |
| ---: | --- | --- | --- |
| 33 | 0 | `legacy_omega_blackwell_codegen_pq_prog` | E |
| 37 | 0 | `legacy_omega_blackwell_codegen_pq_prog` | E |
| 72 | 0 | `legacy_omega_blackwell_codegen_pq_prog` | E |
| 81 | 1 | `legacy_omega_qg_spec_ok` | C |
| 82 | 1 | `legacy_omega_qg_spec_ok` | A |
| 84 | 1 | `legacy_omega_qg_spec_ok` | A |
| 88 | 1 | `legacy_omega_qg_spec_ok` | C |
| 102 | 0 | `qg_order_emit` | Q |
| 105 | 1, 3, 5, 6 | `qg_order_emit` | Q |
| 108 | 6 | `qg_order_emit` | Q |
| 114 | 1, 2 | `qg_order_emit` | Q |
| 115 | statement | `qg_order_emit` | Q |
| 118 | 0 | `qg_order_emit` | E |
| 130 | 2 | `legacy_omega_blackwell_codegen_qgemm_prog` | G |
| 133 | 1, 2 | `legacy_omega_blackwell_codegen_qgemm_prog` | O |
| 134 | 1, 2 | `legacy_omega_blackwell_codegen_qgemm_prog` | O |
| 141 | 0 | `legacy_omega_blackwell_codegen_qgemm_prog` | E |
| 149 | 1, 2 | `legacy_omega_blackwell_codegen_qgemm_prog` | E, Q |
| 172 | 0 | `legacy_omega_blackwell_codegen_qgemm_prog` | E |
| 194 | 0 | `legacy_omega_blackwell_codegen_qgemm_prog` | E |
| 239 | 0 | `legacy_omega_blackwell_codegen_qgemm_prog` | E |
| 262 | 0 | `legacy_omega_blackwell_codegen_qgemm_prog` | E |
| 266 | 0 | `legacy_omega_blackwell_codegen_qgemm_prog` | E |
| 282 | 0 | `legacy_omega_blackwell_codegen_qgemm_prog` | E |
| 286 | 0 | `legacy_omega_blackwell_codegen_qgemm_prog` | E |
| 319 | 0 | `legacy_omega_blackwell_codegen_qgemm_prog` | E |

## Commands actually exercised

All test, differential, mutant, simulator and compiler commands below ran behind this exact device-free prefix:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

The checkout was `~/work/cudastack/lanes/CodegenRest`; `OMEGA_MV=~/work/cudastack/omega-mv`, `OMEGA_OMC=$OMEGA_MV/seed/out/omc`, and `OMEGA_BRIDGE=~/work/cudastack/omega-bridge`. The shell-expanded command tails were:

| Command tail (after the prefix) | rc | Observed final line |
| --- | ---: | --- |
| `env OMEGA_OMC=… OMEGA_BRIDGE=… sh om/units.sh` | 0 | `wrote checked portable Omega units to …/src` |
| `env OMEGA_OMC=… OMEGA_BRIDGE=… sh om/units.sh --check` | 0 | `PASS portable Omega units regenerate byte for byte` |
| `make -j3 OUT_DIR=build/codegen-rest-final OMEGA_OMC=… OMEGA_MV=… OMEGA_BRIDGE=… test-codegen-rest` | 0 | `PASS codegen-rest CPU-only differential controls` |
| `make -j3 OUT_DIR=build/codegen-rest-host OMEGA_MV=… OMEGA_BRIDGE=… test-codegen-rest-sim` | 0 | `GPU_QGEMM: PASS` |
| Final relink of the five named non-qg host test binaries, then each with `--host-only` and `--sim` | 0 each | per-family table below; full commands in `host-sim-five.txt` |
| Final `make -j3 … build/codegen-rest-host/gpu_qgemm_test`, then qg `--host-only` and `--sim` | 0 each | `GPU_QGEMM: PASS` |
| After attention repair: `make -j3 … build/codegen-rest-host/gpu_decode_attn256_test test-om-ir-limit`, then attention `--host-only` and `--sim` | 0 each | attention host/sim PASS; replay-limit PASS |

The final carried gate is stdout excerpt `differential-mutants.txt` (bg_982, rc 0). The underlying runner uses `gcov --json-format --branch-counts --branch-probabilities` on each retained-builder object; full generated gcov data is in `build/codegen-rest-final/codegen-rest`, with the compact permanent inventory in `coverage.json`. Logs replace `/home/drakestapleton` with `~`; no other output transformation is applied to the simulator logs.

| Family | Host final line | Warp-sim final line | Warp issues |
| --- | --- | --- | ---: |
| attn256 | 61 checks, 0 failed: PASS | 14 checks, 0 failed: PASS | 10975936 |
| pattn256 | 19 checks, 0 failed: PASS | 30 checks, 0 failed: PASS | 53330976 |
| router | 88 checks, 0 failed: PASS | 15 checks, 0 failed: PASS | 1703934 |
| gemm | 59 checks, 0 failed: PASS | 21 checks, 0 failed: PASS | 7687894 |
| bf16 | 191 checks, 0 failed: PASS | 23 checks, 0 failed: PASS | 359562 |
| qg / pq | GPU_QGEMM: PASS | GPU_QGEMM: PASS | reported per case, not aggregated |

`host-sim-attn-final.txt` is the post-attention-repair rerun, `host-sim-qg.txt` is post-initializer-order repair, and `host-sim-five.txt` records the final other-family host/sim commands. The earlier simulator target's missing-router-binary failure was fixed by making its prerequisites explicit rather than expanding variables before their definitions; the complete target and final focused reruns then passed.

## Controls and defects fixed

- IR record flip: caught for all six families, rc 1 with an IR mismatch.
- Recompiled Omega mutants: fixed-stall mutation killed in every family (six); qg ignore-meta[6] killed by rt=8 refusal mismatch (one). Exact first failing instruction/case is in `differential-mutants.txt`.
- Replay-limit test: meta6=0 preserves existing fixtures; accepted nonzero boundaries work; full-width out-of-range values refuse before destination mutation. The final qg corpus matches rt=8 IR, allocation and bytes to C, and the ignore-meta6 mutant is killed.
- FP8 GEMM admission now bounds full-width mt before multiplying, preventing wrap-to-zero followed by accumulator-array overrun. C oracle and Omega both repaired.
- Qg conditional array initializer allocation order is preserved; the rt=2/warps=4/SwiGLU/n=256 case exposed and now guards the swapped virtual-register defect.
- Attention wide-head logarithm: before repair, UBSan at retained-source line 219 reported `runtime error: shift exponent 32 is too large for 32-bit type 'unsigned int'`, rc 1, for depth=1, q_heads=kv_heads=UINT32_MAX, q_per_cta=1, multi=1. Both implementations now refuse non-powers before the logarithm. The same `cc -O0 -g -fsanitize=undefined -fno-sanitize-recover=undefined -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_ATTN256=1 -Isrc <minimal-main> src/omega_blackwell_codegen_attn256.c build/codegen-rest-host/libomega_gpu.a -lpthread -lm` reproduction, compile and run both device-free, returns rc 0 with no diagnostics after repair. Its main returns whether the builder wrongly accepts that spec. The permanent differential includes this case, the high-bit neighbours, and a small non-power.

## Wolfram derivation receipts

- `Table[{rt,epi,Floor[If[epi==0,(8388607-36)/(4 (16 rt-8)),8388607/(16 rt-8)]]},{rt,{2,4,8}},{epi,{0,1}}]` -> `{{{2,0,87380},{2,1,349525}},{{4,0,37448},{4,1,149796}},{{8,0,17476},{8,1,69905}}}`. These exact qg_offset limits and adjacent legal spans are tested.
- `{4*4,(4-1)*16384*2+512*16,2^23,Table[{lb,128/lb,32*lb},{lb,{8,16}}],Table[{w,w/4,Table[q+1<w/4,{q,0,w/4-1,2}]},{w,{8,16,32}}]}` -> `{16,106496,8388608,{{8,16,256},{16,8,512}},{{8,2,{True}},{16,4,{True,True}},{32,8,{True,True,True,True}}}}`. This proves F, D, P and R's arithmetic implications; BF16 constants are in its header lines 54 and 65.
- Signed comparison bias and format constants are recorded in `replay-and-regeneration.txt`.

## Tier verdicts and remaining gates

| Tier | Verdict |
| --- | --- |
| Seeded regeneration byte check | PASS |
| Six-family C/Omega IR + complete allocation + bytes + refusals | PASS |
| Source-derived gcov corpus and uncovered-arc accounting | PASS |
| Record-flip negative controls | PASS |
| Seven recompiled Omega mutants | PASS |
| Focused host checks | PASS |
| Six-family warp simulation | PASS |
| Integrated full make test | NOT_RUN — integration owner owns it |
| Chip / live RM / VFIO / hardware | NOT_RUN — no authorized window |
| Merge / deployment | NOT_RUN |

Chip runbook: `om/codegen/CHIP.md`. No device node, GPU lock, NVIDIA module or live service was opened or changed. Host and emulator results are not chip qualification. Integrator2 owns integration, shared meta7 reconciliation and the harness-guide adoption; this lane does not modify another repository's guide or pins.
