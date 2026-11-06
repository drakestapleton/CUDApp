# Linux platform residual host evidence

IMPLEMENTED and EXERCISED on host only. Chip/live-RM/VFIO: NOT_RUN. MERGED/DEPLOYED: NOT_RUN.

Compiler: omega-mv e5b32567, omc SHA-256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f. Existing omega-bridge bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4; no repin, new host dependency or public API change.

## Command ledger

Every log records its exact command, output and return code. Every executing test/mutant/coverage driver runs as `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`. Build parallelism is at most -j3.

| Log | rc | Final observed result |
|---|---:|---|
| generate.log | 0 | wrote checked portable Omega units to this worktree's src |
| focused-final.log | 0 | SUMMARY passed=10 failed=0 |
| regeneration-check.log | 0 | PASS portable Omega units regenerate byte for byte |
| coverage-build.log | 0 | no compiler diagnostics |
| coverage-run.log | 0 | PASS platform rest source-partition differential and ABI refusals |
| coverage-gcov.log | 0 | Lines executed:93.03% of 201 |
| crumb-fix.log | 0 | crumb verify: OK: 42 crumbs current |
| rest-final.log | 0 | PASS platform rest mutants (8 killed) |
| crumb-final.log | 0 | crumb verify: OK: 42 crumbs current |

The focused gate includes test-platform-linux-rest, test-platform-linux-policy, test-platform-linux-effects, test-platform-linux-wait (with its killed source mutant), test-platform-linux-fake-device and test-gpu-session-platform. Session platform: 16 passed, 0 failed. Linux reference simulated-driver integration: 17 passed, 0 failed. NVRM native-slot fixture: 10 passed, 0 failed. Eight new seeded residual mutants are killed; the explicit red result comparison fails as required. The real memfd regression uses only an anonymous host descriptor; it does not open a GPU device.

`focused.log` retains the failed rc 2 integration receipt: fixing memfd capture invalidated the old timeout test's assumption that the fallback C driver wait must be called. The updated test wraps the actual Omega wait entry, requires a nonzero bound handle, one poll per session slice and zero fallback waits, while preserving all timeout/latch/park/no-release assertions. This checks the intended path instead of weakening the timeout contract.

## Retained C oracle branch coverage

`--coverage -O0` over tests/platform_linux_rest.c includes src/omega_gpu_platform_linux_rest_oracle.h. gcov reports 100.00% of 34 oracle lines executed, 100.00% of 73 oracle branches executed and 100.00% of 73 branch edges taken. There are no uncovered retained-oracle branches requiring exceptions. The all-extra-slots-occupied case was added when gcov exposed the loop-exhaustion edge.

The generated C binding's lower branch percentage is not a handwritten-oracle coverage claim: the generator emits fixed-extent safety/refusal machinery that valid scalar packets do not enter. Public ABI short-view refusals are explicitly tested. Its generated implementation is checked byte-for-byte by om/units.sh --check.

## Prerequisite repairs and remaining C

The registration commit restores the exact missing #281 NVRM source/generated files from e2086965ba8ec60d7e385b98abd7565caa13293d that #297's merge dropped despite retaining their unit registrations. No NVRM computation is rewritten. The invalid trailing comma in om/platform-linux/.crumb is repaired so crumb compile/verify can run.

Function classification, the native-record bridge extension proposal and the chip runbook are in om/platform-linux/REST.md. Native pointer publication, C struct gather/scatter, pthread/clock/syscalls, allocation/free and driver lifetime/error unwinding stay C. Marker/policy/mapping/time/submission decisions default to seeded Omega; the scalar C oracle defaults OFF. Existing #273/#297 Omega computation/effects remain in place. No compiler/language/encoding/bridge pin/API/ownership change affects an omega-harness skill; this branch updates its own migration guide and runbook.

WolframLanguageEvaluator: `ToCharacterCode["/memfd:"]` -> `{47,109,101,109,102,100,58}`; `2^63` -> `9223372036854775808`.
