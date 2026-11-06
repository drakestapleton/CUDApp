# Emitter and reconvergence decisions

Default host builds use `om/codegen/cg_em.om` and `cg_reconv.om` through the header wrappers. `CUDAPP_MIGRATION_PENDING_C_EM` and `CUDAPP_MIGRATION_PENDING_C_RECONV` select the retained C verification oracle; neither is defined by default. `OMEGA_GPU_NO_POSIX` retains that oracle until the freestanding ABI can call these portable units. C owns IR records, table traversal, allocation/append calls, and the ordered writes to those records. Omega owns control words, scoreboard transitions, interval admission, relative branch offsets and region admission/patch decisions. Join opcodes are BRA 46, BSSY 54 and BSYNC 55.

The wire encodes signed C int fields as uint32 values widened to uint64, decoded explicitly in Omega. The instruction count remains unsigned size_t. A failed join copies the decremented depth back before checking instruction fields; a late failure retains the BSSY immediate and earlier exit patches. Failed append retains the preceding scoreboard changes.

From the repository root, prefix **every** command below with:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

```sh
sh tests/regalloc_om/checkers.sh
env OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/tmp/integ/bridge-73-79e8c36 python3 tests/regalloc_om/checkers_mutants.py
```

`checkers.c` partitions the source predicates: all emitter kinds and invalid kinds, predicated/unpredicated branches, append success/failure, pending masks and held bits, prefetch/consume barriers 0..31, loop capacity and all interval comparisons, log2 powers/nonpowers, begin/exit bounds, every join refusal and valid joins, including failure after an earlier exit was patched. Allocation and append doubles are deliberately limited to injecting those external outcomes; no device is used. The C oracle is compiled separately and full mutable state/immediates are compared. An extra output record is the red comparison control. `build/checkers-om/` contains the two outputs, red output, gcov source reports and recompiled-mutant evidence. Compilation refusal is not a killed mutant. Host evidence is not chip qualification.

The source-partition run exercises every C branch outcome in all three headers. Ten recompiled semantic mutants are required by `checkers_mutants.py`, including a bad join opcode, lost depth decrement and wrong relative immediate.

## Hazard mutant boundary

`hazard_diff.c` forces a size-4 LDG with a live destination in every generated program and in a direct decoder witness. Comparison includes read/write counts and every register-range length. The size-class `< 4` mutation is killed, rather than relying on incidental random coverage.

`mutants.py` requires twenty recompiled non-equivalent semantic mutations to be killed. It also lists `hazard-branch-sign` as **EQUIVALENT**, never killed: `hazard-branch-sign-proof.txt` records the mounted Wolfram query and `{False, True, {0, 0, 32}}` result proving that the two sign-extension constants give identical low uint32 bits after the six-bit shift for the complete input domain.

## Padded row-zero packing

The original row-zero loop underflowed `k - c0` for complete padding blocks and read beyond the source. The `k=1,kp=48` source-sized regression in `packers.c` reproduced an AddressSanitizer heap-buffer-overflow before the repair (exit 1); the repaired C path exits 0 under the same sanitizer and preserves non-row-zero destination words.
