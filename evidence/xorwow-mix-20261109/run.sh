#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# L441 receipt runner: build omega/libraries/xorwow_mix.om with the pinned omc,
# compare its scores with the Wolfram values in expected.txt, then build three
# mutants that must fail with their own exit codes.
# Usage: OMC=/abs/path/to/omc sh evidence/xorwow-mix-20261109/run.sh OUTDIR
# Prints RESULT PASS and exits 0 only if every row holds.
set -u
PIN=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
R=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
SRC=$R/omega/libraries/xorwow_mix.om
EXP=$R/evidence/xorwow-mix-20261109/expected.txt
OUT=${1:?usage: OMC=... run.sh OUTDIR}
[ -x "${OMC:-}" ] || { echo "NOT_RUN: set OMC to the pinned seeded compiler"; exit 3; }
[ "$(sha256sum < "$OMC" | cut -c1-64)" = "$PIN" ] || { echo "NOT_RUN: omc is not $PIN"; exit 3; }
mkdir -p "$OUT"
fail=0
echo "omc sha256 $PIN"
echo "source sha256 $(sha256sum < "$SRC" | cut -c1-64)"

# build NAME SRCFILE: compile to $OUT/NAME, print the compile status
build() {
  "$OMC" < "$2" > "$OUT/$1" 2> "$OUT/$1.omc-stderr"
  rc=$?
  chmod +x "$OUT/$1"
  echo "compile $1 rc=$rc"
  return $rc
}

if build good "$SRC"; then
  "$OUT/good" > "$OUT/good.stdout"
  rc=$?
  cat "$OUT/good.stdout"
  echo "run good rc=$rc"
  [ "$rc" = 0 ] || fail=1
  if cmp -s "$OUT/good.stdout" "$EXP"; then echo "PASS stdout equals the Wolfram scores"; else echo "FAIL stdout differs from the Wolfram scores"; fail=1; fi
  echo "binary sha256 $(sha256sum < "$OUT/good" | cut -c1-64)"
else
  fail=1
fi

# mutant NAME WANT FROM TO: replace one exact line, require exit WANT
mutant() {
  awk -v from="$3" -v to="$4" 'BEGIN{n=0} $0==from{print to; n++; next} {print} END{if(n!=1) exit 9}' "$SRC" > "$OUT/$1.om" \
    || { echo "FAIL $1: line not found once"; fail=1; return; }
  if build "$1" "$OUT/$1.om"; then
    "$OUT/$1" > "$OUT/$1.stdout"
    rc=$?
    if [ "$rc" = "$2" ]; then echo "PASS mutant $1 caught: exit $rc"; else echo "FAIL mutant $1: exit $rc, wanted $2"; fail=1; fi
  else
    fail=1
  fi
}
mutant weyl 10 'd = xmixmask add get xmst 5 362437' 'd = xmixmask add get xmst 5 362438'
mutant nofold 30 'ret xor o shr o 16' 'ret o'
mutant bmlen 20 'lc = sub add i 1 lc' 'lc = lc'

if [ "$fail" = 0 ]; then echo "RESULT PASS"; else echo "RESULT FAIL"; fi
exit "$fail"
