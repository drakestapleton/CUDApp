# Binary64 BlackwellIR register forms

This change is stacked on CUDApp `9be13fb2595f1b3668288b36c4d17ef976bcb3a7` (native half seam). The seeded compiler is omega-mv `e5b32567`, `omc` SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. `om/encoder/bw_enc_sm121.om` owns encoding. The conditional C encoder is an independent differential oracle. No CUDA runtime, cuBLAS or libdevice implementation is adopted.

## Encoding facts

Derived offline on the host with nvcc 13.0.88 (`-arch=sm_121 -cubin`) and nvdisasm 13.0.85 from small inline-PTX CUDA probes in `/tmp`, outside the checkout. Only these encoding facts are retained here, not cubins, PTX, NVIDIA headers or disassembly listings. The four words are little endian. Register fields are eight bits. Predicate guards occupy w0[15:12]: 7 always, 0 P0, 8 !P0. Explicit `BlackwellIRInsn.control` is preserved; zero selects `0x000e2400`. This default is not a chip latency qualification. Consumers must schedule dependencies, including predicate production, using the existing emitter's scoreboard categories.

| IR | Operation | w0 low opcode | Other fields |
|---|---|---|---|
| 69 | DADD.RN | 0x7229 | dst w0[23:16], a w0[31:24], b **w2[7:0]**, w1 zero |
| 70 | DMUL.RN | 0x7228 | dst w0[23:16], a w0[31:24], b w1[7:0], w2 zero |
| 71 | DFMA.RN | 0x722b | dst w0[23:16], a w0[31:24], b w1[7:0], c w2[7:0] |
| 72 | DSETP | 0x722a | a w0[31:24], b w1[7:0], comparison/combiner w2 |
| 73 | MUFU.RCP64H | 0x7308 | scalar dst w0[23:16], scalar source w1[7:0], w2 0x1800 |
| 74 | MUFU.RSQ64H | 0x7308 | scalar dst w0[23:16], scalar source w1[7:0], w2 0x1c00 |
| 75 | F2F.F64.F32 | 0x7310 | pair dst w0[23:16], scalar source w1[7:0], w2 0x201800 |
| 76 | F2F.F32.F64.RN | 0x7310 | scalar dst w0[23:16], pair source w1[7:0], w2 0x301000 |
| 77 | I2F.F64.S32 | 0x7312 | pair dst w0[23:16], scalar source w1[7:0], w2 0x201c00 |
| 78 | I2F.F64.U32 | 0x7312 | pair dst w0[23:16], scalar source w1[7:0], w2 0x201800 |
| 79 | F2I.S32.F64.TRUNC | 0x7311 | scalar dst w0[23:16], pair source w1[7:0], w2 0x30d100 |

DSETP uses FSETP's comparison-mask convention in `imm`: `0x03f00000 | mask << 12`, with zero selecting default GE (`0x03f06000`). Relation bits are LT=1, EQ=2, GT=4, UNORDERED=8. All masks 0..15 work, including ordered/unordered variants, NUM, NAN, false and true. This IR exposes destination P0 only (`dst_vreg=-1`); a GPR destination is refused. Hardware predicate destinations live in w2[19:17], not the GPR destination field. Predicate guards remain independent of comparison masks.

All f64 values use `omega_bw_ir_alloc_vreg64`, which reserves an even-aligned pair. Pair operands are refused if their physical base is odd or would include R255 (RZ). DFMA's absent third operand is RZ. Scalar conversion operands can be odd. MUFU.*64H is **not a whole-pair write**: use source and destination subreg 1 on pair allocations and explicitly zero the low destination for the PTX-style full result. The low input is ignored and the low destination is untouched by SASS. Register allocation already has an opcode-independent pair class; the seeded allocator migration is owned by the RegAllocOm lane, with no opcode-specific allocation shim here.

`BW_IR_LAST` is 79. `omega_gpu_om_ir_replay` uses that one enum bound and refuses 80. The half gate tests `BW_IR_LAST+1` instead of a fixed unknown opcode. Main's published ATOMG MIN keeps opcode 64; the #280 merge moves unpublished integ conversion/binary64 opcodes 64..78 to 65..79. Historical receipts below retain their original source numbering.

Decimal encoding constants are verified with Wolfram, not hand conversion:

`WolframLanguageEvaluator: FromDigits[#,16]& /@ {"7229","7228","722b","722a","7308","1800","1c00","7310","301000","201800","7312","201c00","7311","30d100","03f06000","000e2400"} -> {29225,29224,29227,29226,29448,6144,7168,29456,3149824,2103296,29458,2104320,29457,3199232,66084864,926720}`.

The independent fused-cancellation vector is also checked:

`WolframLanguageEvaluator: Together[(1+2^-52)(1-2^-52)-1] -> -1/20282409603651670423947251286016 = -2^-104`.

## Simulator precision contract

Compile simulator users with `-ffp-contract=off`. DADD/DMUL use host IEEE binary64 arithmetic, DFMA uses `fma`, and narrowing is a host nearest-even float conversion in the default rounding environment. Tests explicitly select FE_TONEAREST. NaN results are compared as NaNs, not by undocumented payload propagation. Integer conversion truncates and clamps to the signed destination range; an f64 NaN produces INT32_MIN, not zero, per [PTX cvt](https://docs.nvidia.com/cuda/parallel-thread-execution/index.html#data-movement-and-conversion-instructions-cvt).

The [PTX reciprocal f64](https://docs.nvidia.com/cuda/parallel-thread-execution/index.html#floating-point-instructions-rcp-approx-ftz-f64) and [rsqrt f64](https://docs.nvidia.com/cuda/parallel-thread-execution/index.html#floating-point-instructions-rsqrt-approx-ftz-f64) specifications describe 1.11.20 high-word inputs/results, ignored low input, sign-preserving flush of subnormal input/results, and canonical NaN `0x7fffffff00000000`. The simulator models the normal high word by truncating the host reciprocal/rsqrt, not by reproducing NVIDIA's undisclosed approximation table. It models the documented special values and word side effects exactly. Those sections do not publish a numerical normal-input error bound; no chip ULP or relative-error claim is made. A consumer needing a proved libm bound must supply its own refinement and oracle checks.

## Host acceptance

Use the required device-free namespace for **every** host test, simulator and mutant. It hides `/dev/nvidia*`, `/dev/cuse` and `/dev/vfio`. Export:

```sh
export OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc"
export OMEGA_MV="$HOME/work/cudastack/omega-mv"
export OMEGA_BRIDGE="$HOME/work/cudastack/omega-bridge"
export OMEGA_BRIDGE_DIR="$OMEGA_BRIDGE"
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-codegen-f64
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/units.sh --check
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test
```

The focused target runs the seeded encoder, an independently compiled C-oracle differential, IEEE/special-value vectors, an actual DFMA/DSETP probe kernel in the warp simulator, the 63-pair acceptance / 64-pair refusal boundary under the default R2..R127 allocation limit, the replay and encoder bounds, and six successfully compiled seeded mutants. Mutants change an opcode bit, DFMA operand order, pair alignment, DSETP mode, opcode bound, and MUFU function. Compile errors do not count as kills. The simulator probe's DMUL-instead-of-DFMA negative control must fail before it is reported as caught. No session or device-open call is reachable in `--sim`; argument validation rejects missing or unknown modes.

The all-mode probe is also a regression for the primary warp simulator's
predicated ordinary instructions: default dispatch must execute the computed
predicate mask, not all live lanes. Previously an `@P0 MOV` ran even for
DSETP's always-false mask, producing predicate 1 instead of 0.
The header-only C1Kernels repair is carried as prerequisite `5bd6f1f`,
attributed to `7de6da63c83dabec2f36569b75e67f60e9092390` (PR #301),
with owner consent; it also includes C1's predicated SHFL guards.
Integration must not independently apply that prerequisite twice.

The full gate passes only with rc 0 and final `CUDA++ HOST TESTS PASS (<n> targets)`. A timeout is INCONCLUSIVE and is not retried. The integrator owns the integrated full gate; lane receipts describe only their exact stacked revision.

### Lane exercise receipt

Pinned compiler: `omega-mv@e5b32567`, `omc` SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Offline encoding oracle: nvcc 13.0.88, nvdisasm 13.0.85; temporary
CUDA source/cubin outside the repository, no oracle listings committed.
The exact wrapped commands above used `make -s -j3` and the four exported
paths. Focused host/simulator/mutants: **PASS**, rc 0, final
`codegen-f64 mutants: PASS (6 compiled seeded mutants killed)`.
Regeneration check: **PASS**, rc 0, final
`PASS portable Omega units regenerate byte for byte`.
The expected DMUL negative-control diagnostic precedes the probe's PASS;
it reports `got=0x1p+0 want=-0x1p-104`, not a normal-path mismatch.
The lane's **single full-suite attempt was FAIL**, rc 2, not a timeout.
It stopped at `mk/runtime-r3.mk:12` (`test-runtime-portability-check`):
`FAIL: no row in the table for src/omega_gpu_dnn_types.h`, followed by
`FAIL: portability table does not match the tree`. The last output line,
after concurrent workers finished, was
`mutants: 120 total, 107 killed, 13 survived as redundant, 0 survived as test gaps (receipt /tmp/mutrun.TVMyD6/out.json)`.
There was no `CUDA++ HOST TESTS PASS` line. That DNN-header registration
defect was handed to AttentionC through Main/Integrator. The full suite
is not retried here; only the repaired portability target is to be exercised.
This attempt preceded the isolated C1 SHFL-guard prerequisite import.
AttentionC's isolated portability-row fix `4593b23` is carried as `b486d33`.
At Main's direction, only the targeted wrapped command was re-run:
`make -s -j3 test-runtime-portability` with the same four pinned environment
paths and `bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10`.
Result: **PASS**, rc 0, 153 rows; final
`ALL CONTROLS BEHAVED AS REQUIRED`. This resolves the observed classification
defect; it does not change the once-only full-suite verdict to PASS.
The integration owner must run the final integrated full gate.

The matching cross-repository encoder-guide update is handed off in
[omega-harness #191](https://github.com/aien-dev/omega-harness/issues/191).
No guide ownership or merge authority is inferred from this codegen lane.

## Authorized-window chip runbook

Chip is **NOT_RUN**. No window, service stop, GPU lock, module or device access is authorized by this document. Keep the live services untouched. The current device-free host-test rule must be explicitly replaced by Main/operator authority for the exclusive chip window; do not expose device nodes merely to make this runbook work.

1. Obtain current exclusive-window authorization and the approved runner/device policy. Recheck ownership and live-service rollback procedure through the current hardware runbook. Do not infer permission from an old lock or receipt.
2. Run the host commands above first. `build/codegen_f64_probe --sim` must pass in the device-free namespace, including its negative control. Encoding-only and host results are not chip qualification.
3. In the approved chip runner only, execute `build/codegen_f64_probe --chip`. The fixture allocates/launches through CUDA++ (ARCH-0042), never nvcc or libcuda. The kernel launches one 32-thread block; each lane writes 16 bytes: DFMA result low/high, DSETP predicate as 0/1, and a canary. Inputs are supplied as raw binary64 argument words. It checks all DSETP masks for finite fused cancellation, a rounding case, subnormals, signed zero, infinity and NaN. Every returned lane is compared with host `fma` and an independent IEEE relation-mask reference. NaNs are checked by classification. The DMUL negative control is simulated before session open and can never launch on chip.
4. Require rc 0 and final `codegen-f64 probe chip: PASS (DFMA, all DSETP modes, DMUL negative control)`. A missing device, launch error, timeout, mismatch or wrong canary is not PASS. Preserve the exact source SHA, seeded/compiler pins, launch geometry, environment, rc and final line in a new receipt. Do not commit code bytes or disassembly. Stop on the first failure and follow the authorized rollback procedure.
5. This probe qualifies DFMA/DSETP only. MUFU normal-value accuracy, every arithmetic/conversion op on chip, production scheduling and downstream DGEMM/libm remain separate qualifications. No host result silently qualifies them.
