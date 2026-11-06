#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# fmt.om acceptance: om/tools/fmt.om (fm_f = "%.Pf", fm_g = "%.Pg"), composed with a test export, against
# glibc snprintf through tests/tools-om/fmt_diff.c; a flipped-record negative control; recompiled mutants.
# FMT_DIFF_N (default 40000) random doubles, nine conversions each, plus the specials.
# Run under: bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
. tests/tools-om/lib.sh
tom_pin
N=${FMT_DIFF_N:-40000}
UNIT=$tmp/fmtt.om
{ cat om/tools/fmt.om; cat <<'OM'

note fmtt: test export, conversion g (0 "%.Pf", else "%.Pg") of the f64 with bits xb at precision pr.
fn fmtt xb ulong pr ulong g ulong o edit uchar 1600 to ulong
x = bitcast f64 xb
if ne g 0
ret fm_g x pr o
end
ret fm_f x pr o
end
OM
} > "$UNIT"
OMEGA_SEEDED_OMC="$OMC" sh "$BRIDGE/tools/bridge-seeded" c "$UNIT" > "$tmp/fmtt.c"
$CC $TOM_CFLAGS -o "$tmp/fmt_diff" tests/tools-om/fmt_diff.c "$tmp/fmtt.c" -lm
"$tmp/fmt_diff" "$N" c > "$tmp/c.out"
"$tmp/fmt_diff" "$N" om > "$tmp/om.out"
cmp "$tmp/c.out" "$tmp/om.out" || { echo "FAIL fmt differential"; diff "$tmp/c.out" "$tmp/om.out" | head; exit 1; }
lines=$(wc -l < "$tmp/c.out")
echo "differential: $lines conversions identical to glibc snprintf"
tom_flip "$tmp/c.out" "$tmp/om.out"

killed=0
"$tmp/fmt_diff" 2000 c > "$tmp/c.small"
mut() { # NAME SITE REPLACEMENT
  tom_mutant "$1" "$UNIT" "$2" "$3"
  $CC $TOM_CFLAGS -o "$tmp/$1.bin" tests/tools-om/fmt_diff.c "$tmp/$1.c" -lm
  "$tmp/$1.bin" 2000 om > "$tmp/$1.out" || true
  if cmp -s "$tmp/c.small" "$tmp/$1.out"; then echo "FAIL mutant $1 survived"; exit 1; fi
  echo "mutant $1: killed ($(cmp "$tmp/c.small" "$tmp/$1.out" | sed 's/.*, //'))"; killed=$((killed + 1))
}
mut ties_even 'ret and sub ulong get d sub q 1 48 1' 'ret ulong 1'
mut round_down 'if lt c 53' 'if lt c 52'
mut pow5 'n = fm_mul l n 1220703125' 'n = fm_mul l n 1220703124'
mut exp_digits 'if lt xa 10' 'if lt xa 9'
mut g_small 'if gt xa 4' 'if gt xa 5'
mut g_large 'if ge xa pr' 'if gt xa pr'
echo "fmt mutants: $killed killed"
echo "tools-om fmt: PASS (differential $lines conversions, negative control, $killed mutants killed)"
