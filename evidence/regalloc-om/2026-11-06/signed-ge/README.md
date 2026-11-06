# Signed ISETP.GE simulator regression

IMPLEMENTED and EXERCISED on the host. Hardware qualification is NOT_RUN.

The only shared simulator change is the `BW_IR_ISETP_GE` case in
`tests/bw_warp_sim_tc.h`. It compares signed 32-bit source 1 with either signed
source 2 or the signed immediate. No other opcode, predicate-mask handling,
SHFL-after-exit refusal, IR enumeration, or replay field is changed. C1's
`7de6da63` changes the separate `tests/bw_warp_sim.h`; that file is untouched.

The original reproduction was BF16 `frag_prog`, K=16, one row tile, not blocked:
its instruction 38 was refused as unmodelled opcode 10. The regression exercises
that case and the valid blocked/unblocked shapes through K=64. All outputs are
bit-identical to the existing non-contracting CPU matmul oracle.

Command (exit 0):

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 sh tests/regalloc_om/sim_ge.sh
```

Observed output:

```text
PASS signed GE host-reference cases=98 register and immediate
PASS existing encoder golden fixtures and vecadd SHA-256
PASS matmul warp-sim cases=15 CPU oracle bit-identical
FAIL signed GE lhs=-2147483648 rhs=0 register=0
PASS unsigned GE semantic mutant KILLED compile=0 exercise=1
```

The signed reference covers every pair from INT_MIN, INT_MIN+1, -2, -1, 0, 1,
and INT_MAX in both register/register and register/immediate forms. The negative
control recompiles a private header copy with only the new case changed to an
unsigned comparison. Compilation succeeds; the boundary test rejects it with
exit 1. Compile failure is not credited as a killed mutant.

The isolated gate selects the retained C register-allocator/matmul/encoder
oracles so this commit can be cherry-picked before their Omega cutovers. The
same simulator regression is also exercised by the lane's Omega successor gate.
Runtime logs are in `build/regalloc-om/signed-ge/`. No device or live RM path is
linked by this test, and the command ran with a minimal device namespace.
