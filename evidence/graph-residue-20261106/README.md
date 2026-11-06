<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# Slice 8 host receipt, 2026-11-06

Base: #310, 78b8a3e409d65a29a158be618b658a6077b73f74.
Compiler: omega-mv e5b32567, omc SHA256
2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f.

IMPLEMENTED and EXERCISED on the aarch64 host. Chip/live-RM/VFIO NOT_RUN.
Not MERGED, DEPLOYED or hardware QUALIFIED. Full make test belongs to Integrator3.

All executed targets used:
`bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.

- `make -j3 test-graph-residue test-graph-residue-mutants test-hello-sim test-gpu-decode-graph test-model-graphs-om test-model-graphs-warp test-model-graphs-nested test-gpu-moe-window OMEGA_OMC=... OMEGA_BRIDGE=...`: rc0, final line `PASS graph residue: 7 compiled semantic mutants killed`; see gates.log.
- `make -j3 test-graph-residue-coverage test-graph-residue-mutants OMEGA_OMC=... OMEGA_BRIDGE=...`: rc0, same mutant final line; see coverage.log. This includes the UINT32_MAX/adjacent-boundary additions and reruns the differential/red check.

Coverage is measured gcov output, not an estimate. The scalar C oracle header
executed every executable line and branch. Hello's instrumented executable
exercised the simulator, short and past-end controls; uncovered branches are
partitioned and justified in om/graph/README.md. The separate differential covers
checker corruption and reporting branches. gcov files contain source annotations;
they are not a second implementation. Home-directory prefixes in filed logs are
normalized to `~` without changing results.

Mutants compiled and killed: poison, input-period, checker-sub, shape-cap,
timeline-room, perm-stride, load-scoreboard. A nonzero compile result would fail
the gate rather than count as a kill. Red differential flips the deterministic
result after otherwise normal execution; comparison must fail.

During porting, checked narrowing rejected a byte store lacking a mask, and the
differential caught the STORE scoreboard YIELD mismatch. Both were repaired;
these failures were not counted as passing evidence.

Math receipts (mounted WolframLanguageEvaluator):

- `{FromDigits["FFC0FFEE",16],2^20,128*1024,Ceiling[2^20/(128*1024)],(2^20)*{8,4,1}}` -> `{4290838510,1048576,131072,8,{8388608,4194304,1048576}}`.
- `{2^64-4096,(32+48)*128,((32+48)+48)*128,(24+4)*512,24*256+4*512}` -> `{18446744073709547520,10240,16384,14336,8192}`.

The 195.1 GB/s report constant is retained from the existing hello implementation
and historical read-ceiling receipt; it is not a new measurement here. The chip
runbook is examples/hello_gpu/README.md. No device was opened by this lane.
