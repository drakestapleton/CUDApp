# Seeded Omega tensor-core builders

`cg_tc.om` owns the instruction selection, HMMA spacing, held-scoreboard
transitions, staging, branch fixups and epilogues for the builders formerly
implemented in `omega_blackwell_codegen_tc.c` and `omega_blackwell_codegen_tc2.c`.
`cg_tc_emit.om` emits the shared eight-word record format defined by
`omega_gpu_om_ir.h`. The C entry points marshal scalar arguments and replay those
records through `omega_gpu_om_ir_replay`. The existing public headers, CPU data
conversions and shape-query functions are unchanged. `omega_bw_em.h` remains
available to unrelated C builders.

The default is Omega. Set `CUDAPP_MIGRATION_PENDING_C_CODEGEN_TC=1` only to select
the old C differential oracle. This is not a compatibility implementation or a
second production record format. The independent renamed C objects in the test
are always built with the oracle switch enabled.

## Source layout

The exported function is `omega_cg_tc`. Kinds select probe (0), peak (1), read
bandwidth (2), grouped GEMM (3), or grouped GEMM v2 (4). The v2 selector carries
both F32 and SwiGLU epilogues and the direct-output flag. Scalar inputs are packed
by the unchanged public C entry points; the record ABI is common to the other
codegen migrations.

The phase functions preserve the original builder's order. `note r[index] name`
marks builder-local values. Lexical scratch slots are reused after scope exit;
these are host-side builder scratch values, not GPU registers. Pending explicit
instruction structs have their own scratch slots so intervening helper emission
cannot overwrite them. Pure expression helpers bound the seeded compiler's live
value count. All functions obey the function-mode argument limit.

Metadata slots 7 and 8 are emitter-private held-scoreboard and loop-start state;
the generic replay consumes only its documented metadata slots. Packing masks
immediates to their original uint32 width. The instruction-count and virtual
register caps match the pre-existing C builder caps. Regalloc failures are
compared as refusals, not reported as successful programs.

## Reproducible host gates

Use the pinned seeded compiler `omega-mv@e5b32567` (omc SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`),
with the repository's pinned `om/omega-bridge.lock`. Set `OMEGA_OMC`,
`OMEGA_BRIDGE`, `OMEGA_MV` and `OMEGA_BRIDGE_DIR` to their local checkouts.

```sh
sh om/units.sh
sh om/units.sh --check
taskset -c 10-14 nice -n 10 make -j3 test-codegen-tc
taskset -c 10-14 nice -n 10 make -j3 test-codegen-tc-mutants
taskset -c 10-14 nice -n 10 make -j3 test-codegen-tc-sim
taskset -c 10-14 nice -n 10 make -j3 test
```

The last three focused targets are registered in `HOST_TESTS`. The mutant gate
requires the compiler and bridge environment variables; missing tools or a
compile failure fail the gate rather than counting as a killed mutant.

### Configuration coverage

The differential compares every instruction field, the complete solved register
allocation, and successfully encoded instruction bytes. Neither two encoding
failures nor two partially built programs count as byte-equality success.

* Probe: both accepted accumulator modes, invalid mutants and null output.
* Peak: every chain count, stall value and accepted thread count. Iteration count
  is an immediate only: representative interior values and uint32 boundary
  values cover that independent dimension. Zero iterations and invalid shape
  boundaries are checked as refusals.
* Bandwidth: every accepted thread count and both control-flow classes of
  `groups` (one/shared and greater-than-one/distinct), plus zero and uint32
  boundary values. Iteration count enters only the count immediate and the
  uint32 region-size product; 1, 2, an interior value and UINT32_MAX cover their
  boundary behavior. The runtime loop is not unrolled by the builder.
* Grouped GEMM: every K, row tile, column tile, warp count and accepted built-in
  mutant. Shapes refused by the existing register allocator must still refuse.
* Grouped GEMM v2: every K, row tile, warp count, A-fragment count, epilogue,
  direct-output mode and built-in mutant. At K=128 every accepted N is tested;
  at every other K both accepted N endpoints are tested. This is a complete
  factored boundary set, not an exhaustive Cartesian product: N affects only
  affine output offsets, independently of K, while K changes the loop count and
  input offsets. Shape validation, all nonlinear instruction decisions and all
  epilogue/direct decisions are enumerated. Invalid scalar-field boundaries and
  null specs/output are checked separately.

N endpoints follow the original strict 23-bit output-offset guard:
`Floor[(2^23-1)/(64 rt (16 warps))] (16 warps)`.
WolframLanguageEvaluator query
`Table[{rt,w,Floor[(2^23-1)/(64 rt (16 w))] (16 w)},{rt,{2,4,8}},{w,{4,8,16}}]`
returned
`{{{2,4,65472},{2,8,65408},{2,16,65280}},{{4,4,32704},{4,8,32640},{4,16,32512}},{{8,4,16320},{8,8,16256},{8,16,16128}}}`.

The record negative control flips one generated control word and requires the
same comparator to reject it. The separate source-mutant gate recompiles Omega
with changes to probe/peak HMMA spacing, bandwidth folding, GEMM FP8 decoding,
and v2 accumulator rescaling (exercised with both epilogues). Each mutant must
compile and link successfully, then fail the IR differential.

### Simulator and chip distinction

The simulator target executes the emitted probe, both bandwidth address modes,
peak HMMA chains, grouped GEMM, and both v2 epilogues/direct modes. It reuses the
existing tensor-core simulator and its built-in arithmetic mutants. It does not
model scoreboard timing, physical hardware scheduling or chip throughput.

Chip tier is **NOT_RUN**: no GPU window is authorized. During a separately
approved exclusive window, use the existing `gpu_tc_probe_test`,
`gpu_tc_peak_test`, `gpu_tcgemm_test` and `gpu_pfgemm_test` chip runbooks at this
revision, with the existing session/lock and hardware gates intact. Record device,
compiler/unit identities, launch shapes, expected/observed values, timing policy
and built-in mutant outcomes in a new receipt. Host or simulator PASS does not
qualify the hardware path. No merge or deployment is authorized by this lane.
