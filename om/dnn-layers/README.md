# Seeded Omega DNN layers

The integration branch is `feat/dnn-layers-om`, stacked on attention #282,
any-shape GEMM #283 (`04f16fe8`) and C1's append-only unsigned-conversion
foundation (`3bf3ac4`). No GPU execution is authorized by this work.

## Native scalar half seam

`BW_IR_F16_TO_F32` is opcode 66 and `BW_IR_F32_TO_F16` is 67. Existing opcodes
0..65 retain their values. Generic replay admits `BW_IR_LAST` and refuses
`BW_IR_LAST+1`; the replay-boundary regression also kills a compiled source
mutant restoring the former bound of 63.

The sm_121 encoding authority is the already recorded E1_CVT golden words in
`src/omega_numeric.c`: `HADD2.F32 R9, -RZ, R2.H0_H0` has words
`ff097230 20000002 00004100`; `F2F.F16.F32 R9, R2` has
`00097304 00000002 00200800`. Widening uses HADD2, not an invented scalar
F2F widening instruction. The source is the low half of the input register;
narrowing is round-to-nearest-even and clears the upper half. Widening is
fixed latency; narrowing uses a dependency barrier before consuming its result.
The seeded encoder and migration-pending C differential encoder use the same
recorded forms; allocation and instruction selection remain separate.

WolframLanguageEvaluator query:

```wolfram
FromDigits[#,16]& /@ {"ff007230","20000000","4100","7304",
                       "00200800","010fca00","010e2800"}
```

Result: `{4278219312,536870912,16640,29444,2099200,17811968,17704960}`.
These are encoding integers, not measured performance or accuracy claims.

`tests/gpu_dnn_half_test.c` checks all binary16 bit patterns against the
independent integer E1 reference, all finite half round trips, and a deterministic
binary32 narrowing sweep. NaNs are compared by class, not payload. It separately
checks native words across registers and all three IR guard modes, plus an
unknown-opcode refusal. The simulator uses the host `_Float16` conversion,
not the integer reference. No tolerance is applied to finite conversion bits.

Host-only commands:

```sh
taskset -c 10-14 nice -n 10 make -j3 test-dnn-half test-om-ir-boundary
mkdir -p build/dnn-half-bins
taskset -c 10-14 nice -n 10 build/dnn_half_test build/dnn-half-bins/half.bin
# Compare the generated words with the seeded disassembler and offline nvdisasm:
SM121DIS_JOBS=3 tools/sm121dis/check.sh /path/to/sm121dis \
  build/dnn-half-bins build/dnn-half-oracle
```

The disassembler is a read-only verification oracle, not a production dependency.
Its exact source revision and the exercised commands belong in the dated receipt.
Listings and raw instruction binaries stay under ignored build directories.

Chip execution and qualification are **NOT_RUN**. A passing simulator or offline
disassembler check is not a chip receipt. The ROADMAP status is not upgraded.
