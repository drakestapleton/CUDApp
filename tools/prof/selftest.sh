#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host and simulator gate for tools/prof. No GPU is opened. The caller chooses CPU placement
# (taskset/nice): fixed cores 10-14 here failed on a 4-CPU hosted runner (taskset: Invalid argument).
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
PROF="$ROOT/tools/prof"
: "${OMEGA_OMC:=$HOME/work/cudastack/omega-mv/seed/out/omc}"
expected=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
if [ ! -x "$OMEGA_OMC" ]; then
    echo "NOT_RUN: pinned omc is not executable at $OMEGA_OMC"
    exit 2
fi
actual=$(sha256sum < "$OMEGA_OMC" | cut -c1-64)
if [ "$actual" != "$expected" ]; then
    echo "NOT_RUN: profiler compiler pin mismatch"
    exit 2
fi
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM

"$OMEGA_OMC" < "$PROF/prof.om" > "$T/prof" 2>"$T/omc.err"
chmod +x "$T/prof"

"$T/prof" words 4294967408 0 > "$T/words.txt"
printf '%s\n' 537206870 0 0 112 1 0 > "$T/words.exp"
diff -u "$T/words.exp" "$T/words.txt"
cc -std=c11 -Wall -Wextra -Werror -o "$T/oracle" "$PROF/stamp_oracle.c"
"$T/oracle" 4294967408 0 > "$T/oracle.txt"
diff -u "$T/words.txt" "$T/oracle.txt"
"$T/prof" words 100 1 > "$T/w1.txt"
"$T/oracle" 100 1 > "$T/o1.txt"
diff -u "$T/w1.txt" "$T/o1.txt"

"$T/prof" fix "$T/fix.bin"
cc -std=c11 -Wall -Wextra -Werror -o "$T/wfix" "$PROF/write_fix.c"
"$T/wfix" "$T/cfix.bin"
cmp "$T/fix.bin" "$T/cfix.bin"

"$T/prof" sim "$T/sim.txt"
diff -u "$PROF/golden.txt" "$T/sim.txt"
"$T/prof" report "$T/fix.bin" "$T/rep.txt"
diff -u "$PROF/golden.txt" "$T/rep.txt"

grep -F 'k 0 time_ns 1000000000 bytes 195100000000 flops 115140000000 ai_milli 590 bound_probe mem' "$T/sim.txt" >/dev/null
grep -F 'ai_milli 590158892 bound_probe compute' "$T/sim.txt" >/dev/null
grep -F 'ai_milli 590158 bound_probe ridge' "$T/sim.txt" >/dev/null
grep -F 'probe_mem_milli 1000 probe_flop_milli 1 sec5_mem_237_milli 823 sec5_mem_207_milli 942 sec5_flop_milli 0' "$T/sim.txt" >/dev/null
grep -F 'sec5_flop_milli 484' "$T/sim.txt" >/dev/null
grep -F 'probe_mem_milli 500 probe_flop_milli 500 sec5_mem_237_milli 411 sec5_mem_207_milli 471 sec5_flop_milli 242' "$T/sim.txt" >/dev/null
grep -F 'ridge_probe_milli 590158' "$T/sim.txt" >/dev/null

set +e
"$T/prof" >/dev/null 2>&1
usage=$?
set -e
[ "$usage" -eq 2 ]

python3 - "$T" << 'PY'
import pathlib, sys
t = pathlib.Path(sys.argv[1])
b = bytearray((t / "fix.bin").read_bytes())
swap = bytearray(b)
swap[112:120], swap[128:136] = swap[128:136], swap[112:120]
(t / "swap.bin").write_bytes(swap)
zero = bytearray(b)
zero[128:136] = zero[112:120]
(t / "zero.bin").write_bytes(zero)
bad = bytearray(b)
bad[0] = 0
(t / "bad.bin").write_bytes(bad)
(t / "short.bin").write_bytes(bytes(b[:32]))
PY

set +e
"$T/prof" report "$T/swap.bin" "$T/swap.txt" >/dev/null 2>&1
swap=$?
"$T/prof" report "$T/zero.bin" "$T/zero.txt" >/dev/null 2>&1
zero=$?
"$T/prof" report "$T/bad.bin" "$T/bad.txt" >/dev/null 2>&1
bad=$?
"$T/prof" report "$T/short.bin" "$T/short.txt" >/dev/null 2>&1
short=$?
set -e
[ "$swap" -eq 3 ]
[ "$zero" -eq 3 ]
[ "$bad" -eq 4 ]
[ "$short" -eq 4 ]

sed 's/bad = lt t1 t0/bad = lt t1 t1/' "$PROF/prof.om" > "$T/order.om"
grep -F 'bad = lt t1 t1' "$T/order.om" >/dev/null
if grep -F 'bad = lt t1 t0' "$T/order.om" >/dev/null; then
    echo "order mutant sed did not apply"
    exit 1
fi
"$OMEGA_OMC" < "$T/order.om" > "$T/order" 2>"$T/order.err"
chmod +x "$T/order"
set +e
"$T/order" report "$T/swap.bin" "$T/order.txt" >/dev/null 2>&1
ord=$?
set -e
[ "$ord" -ne 3 ]

sed 's/const flop_at ulong 16/const flop_at ulong 8/' "$PROF/prof.om" > "$T/flop.om"
grep -F 'const flop_at ulong 8' "$T/flop.om" >/dev/null
if grep -F 'const flop_at ulong 16' "$T/flop.om" >/dev/null; then
    echo "flop mutant sed did not apply"
    exit 1
fi
"$OMEGA_OMC" < "$T/flop.om" > "$T/flop" 2>"$T/flop.err"
chmod +x "$T/flop"
"$T/flop" report "$T/fix.bin" "$T/flop.txt"
if diff -q "$PROF/golden.txt" "$T/flop.txt" >/dev/null; then
    echo "flop mutant was not caught"
    exit 1
fi

set +e
sh "$PROF/chip.sh" > "$T/chip.txt"
chip=$?
set -e
[ "$chip" -eq 2 ]
grep -F 'NOT_RUN' "$T/chip.txt" >/dev/null
if grep -F '/dev/nvidia' "$PROF/chip.sh" >/dev/null; then
    echo "chip.sh names a GPU device node"
    exit 1
fi

echo "PROFILER HOST SIM PASS words oracle fixture sim report mutants swap=$swap order=$ord chip=$chip"
