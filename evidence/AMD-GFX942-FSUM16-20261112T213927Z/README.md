# gfx942 fsum16: host receipt

Source: `inputs.sha256` identifies the source files, committed alongside this receipt. `base.commit` identifies the starting revision. No device was opened and nothing was pushed, installed or deployed.

Compiler: omega-mv `e5b32567`, binary sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. Both gates check this pin before compilation/output setup. LLVM 18.1.3 and Python 3.12.3 are test oracles only; their version lines are in the logs. Host: Linux AArch64 on spark-b87b.

Commands from the repository root (home prefix abbreviated as `$HOME`; individual outcomes below):

```sh
flock "$HOME/handoffs/hive/queen/pool-14.lock" nice -n 10 sh -c 'AMD_OUT=$HOME/work/hive/amdserve/Gfx942Exec/fsum-host-3 sh tests/amd/run.sh $HOME/work/hive/amdserve/AmdGfx942/pin/omc > ../fsum-host-3.log 2>&1; rc=$?; cat ../fsum-host-3.log; [ "$rc" -eq 0 ] || exit "$rc"; AMD_EXEC_OUT=$HOME/work/hive/amdserve/Gfx942Exec/fsum-exec-3 sh tests/amd-exec/run.sh $HOME/work/hive/amdserve/AmdGfx942/pin/omc > ../fsum-exec-3.log 2>&1; rc=$?; cat ../fsum-exec-3.log; exit "$rc"'
flock "$HOME/handoffs/hive/queen/pool-14.lock" nice -n 10 sh -c 'AMD_EXEC_OUT=$HOME/work/hive/amdserve/Gfx942Exec/fsum-exec-4 sh tests/amd-exec/run.sh $HOME/work/hive/amdserve/AmdGfx942/pin/omc > ../fsum-exec-4.log 2>&1; rc=$?; cat ../fsum-exec-4.log; exit "$rc"'
```

The first command's host gate exited 0 (`host-gate.log`); its executor gate exited 1 on the explicitly checked order-sensitivity precondition (`failed-order-vector.log`). After correcting only that test vector, the second command's complete executor gate exited 0 (`exec-gate.log`). The successful host result is not relabeled as a successful combined command. Logs replace the home prefix with literal `$HOME`; `SHA256SUMS` identifies these portable copies.

The host gate reports `.text`, metadata and descriptor parity against the independent assembly, and byte-identical builds. The vadd full-object digest remains unchanged. The executor gate reports all requested boundary counts, random rows, whole-memory guard checks and caught negative controls. Its `reduce_ref` calculation records left-to-right bits `3f800000` versus reverse-order bits `00000000` for the printed row; the mutated instruction table executes and mismatches, rather than failing to build.

Failures and corrections:

- No Omega compiler errors occurred in this extension.
- LLVM rejected plain `n` and quoted `"n"` as an argument name with `invalid HSA metadata` (retained failed host logs). A targeted oracle-only probe showed renaming it `count` assembles, whereas `!!str n` does not; LLVM's `!str n` tag assembles and preserves the required name. Final metadata bytes match the producer.
- The original order-sensitive row using `4b800000,cb800000,3f800000` was not order-sensitive: the reference assertion stopped the gate. It was corrected to `4c000000,cc000000,3f800000` followed by zero words. The assertion remains mandatory and now the running mutant is caught.

Status: IMPLEMENTED and EXERCISED on the host. AMD hardware, timing, races, hardware NaN payloads, ROCr execution and Qwen inference are NOT_RUN. No merge/deployment claim and no remaining child or process.
