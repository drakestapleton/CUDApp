# Production archive floating-point contract

Append-only supplement to the numeric remainder receipt. The direct numeric
test links already passed `-ffp-contract=off`, but the generic production
object rule did not. `mk/numeric.mk` now appends that required flag to every
numeric object, including the seven generated units, even if a caller sets
`CFLAGS` on the make command line. No global compiler flag or unrelated
object rule changed.

Every build/run below used the same device-hidden bwrap/taskset/nice envelope
as the main receipt. No device or live service was accessed.

## Build-flag negative control

Command inside the envelope (rc 0):

```
make -B -j3 \
  'CFLAGS=-std=gnu11 -Wall -Wextra -Werror -D_GNU_SOURCE -O2 -ffp-contract=fast -Isrc' \
  build/cudapp_om_numeric.o build/cudapp_om_numeric_tile.o \
  build/cudapp_om_numeric_rest.o build/cudapp_om_numeric_patch.o \
  build/cudapp_om_numeric_check.o build/cudapp_om_numeric_trace.o \
  build/cudapp_om_numeric_reduce.o
```

[archive-fp-build.log](archive-fp-build.log) records every actual compiler
invocation. All have the deliberately conflicting `-ffp-contract=fast`
followed by `-ffp-contract=off`; the latter wins. The final compiler line
builds `cudapp_om_numeric_reduce.o` with that ordering.

## Exercised archive paths

- `make -j3 test-gpu-scan`: rc 0, final
  `C3 scan sim: PASS (139 checks)`; this refreshes the archive from those
  objects and exercises the existing host/simulator E1 path.
  [archive-scan-sim.log](archive-scan-sim.log).
- Compile the existing remainder differential driver, C oracle and adapters
  with `-ffp-contract=off`, but resolve its strong Omega entries from the
  actual `build/libomega_gpu.a` instead of recompiling generated sources:
  compile rc 0; runtime rc 0; `NUMERIC REST OM DIFF PASS; host only; chip NOT_RUN`,
  final `checks=8418438`. The complete compile and execution commands are in
  [archive-diff.log](archive-diff.log).

The main receipt's scalar/remainder mutation and direct differential results
are unchanged: no algorithm or generated source was edited for this build
correction. This supplement establishes that the production archive uses
and exercises the same floating-point build contract. Host/simulator PASS;
chip/live-RM/VFIO/HW NOT_RUN. No merge or deployment.
