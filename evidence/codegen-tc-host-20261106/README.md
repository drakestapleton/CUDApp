# TC host computation cutover — 2026-11-06

IMPLEMENTED on base `feat/omega-codegen-tc` at `57b197495671`.
No public ABI or instruction-generation change. The default strong entry
`omega_tc_host` owns admission, resource queries, FP32 scaling, BF16 RNE,
four-element permutation and weight conversion. The C oracle is default-OFF
behind `CUDAPP_MIGRATION_PENDING_C_CODEGEN_TC_HOST`.

The BF16 rounding implementation is the existing canonical
`om/numeric/numeric.om` from `feat/numeric-om` (`1b78bfe99aba`), not a third
rounding algorithm. `tch_scale` performs exact power-of-two scaling using integer
FP32 classes. NaNs are compared by class; signed zero and all finite values are
compared bitwise. The C shell only marshals complete four-element groups.

Compiler: seeded `omega-mv e5b32567`, SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
The converter/frontend pins in `om/units.sh` are unchanged.

## Commands and observed results

Every runtime command below uses this prefix (no GPU devices are visible):

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

* `make -s -j3 test-codegen-tc-host`: rc 0,
  `TC-HOST-DIFF PASS cases=988323`; red comparison returns 1 at case 1 and
  the enclosing gate prints `TC-HOST-RED PASS`.
* `env OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=~/work/cudastack/omega-bridge python3 tests/codegen_tc_host_mutants.py`:
  rc 0, `TC-HOST MUTANTS PASS (8 killed)` and `TC-HOST STRONG-LINK PASS`.
* `make -s -j3 test-codegen-tc-host-cubin`: rc 0,
  `TC-HOST-CUBIN-IDENTITY PASS cases=25`. Full container bytes are compared
  for every mutant selector and both TC2 epilogues/direct modes.
* `make -s -j3 test-codegen-tc-sim`: rc 0, final `GPU_PFGEMM: PASS`;
  TC probe/bandwidth, peak, TC GEMM and TC2 prefill simulator checks pass.
* `make -s -j3 test-codegen-tc`: rc 0,
  `CODEGEN TC DIFF PASS (180517 cases, 52380 matching refusals)`;
  the existing record-flip negative control was caught. This enumerates the
  family instruction/encoded-byte differential, separately from the host query
  partition and the full-container identity gate.

Admission partitions derive from each source predicate: accepted K multiples,
shape/warp choices, mutant selectors, TC2 epilogue/direct/AQN, the strict output
bound and alignment. Each scalar axis also receives 0, UINT32_MAX and the
listed v-1/v/v+1 transition values. Conversion covers both zeros, subnormal
endpoints, normal endpoints, overflow, infinities, quiet/signaling NaNs and
65536 deterministic bit-pattern groups with untouched-tail checks.

| Omega mutant | Verdict |
| --- | --- |
| K lower bound 128 → 256 | KILLED |
| TC K ceiling 32768 → 16384 | KILLED |
| TC2 K ceiling 16384 → 8192 | KILLED |
| strict output bound >= → > | KILLED |
| shared-byte multiplier 4096 → 2048 | KILLED |
| weight shift 4 → 3 | KILLED |
| scale exponent adjustment 64 → 63 | KILLED |
| permutation slot 2 → 1 | KILLED |

The unmodified binding passes before mutation. Converter/link failures are not
kills. The link control removes the implementation symbol and requires an
unresolved `omega_tc_host`; there is no weak fallback.

## Qualification and chip runbook

Host differential, simulator and representative cubin identity: PASS.
Chip / live RM / VFIO / hardware qualification: NOT_RUN. No service or driver
was touched. These host results do not qualify the chip.

After explicit operator authorization of an exclusive GPU window, run the
existing `gpu_tc_peak_test`, `gpu_tcgemm_test`, and `gpu_pfgemm_test` hardware
modes under the current window procedure, comparing the same generated kernels
and recording the source revision and device receipt. Do not run hardware modes
inside the host sandbox or infer authorization from this runbook. Full `make
test` remains the integrator's gate.

Wolfram constants checked with `WolframLanguageEvaluator`:
`{2^64,2^23,2^31,16^^7fffffff,16^^7f800000,16^^7fff,16^^40,2^23,16*128*2,2*16*128,2^23,127-126+64,16^^ffffffff}`
→ `{18446744073709551616,8388608,2147483648,2147483647,2139095040,32767,64,8388608,4096,4096,8388608,65,4294967295}`.
