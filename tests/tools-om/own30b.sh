#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# own30b_prof acceptance: self-tests of both builds, differential against the C oracle
# (-DCUDAPP_MIGRATION_PENDING_C_TOOLS_OWN30B), a flipped-record negative control and eight recompiled
# mutants of om/tools/own30b_prof.om. TOOLS_OM_COVERAGE=DIR also writes gcov branch data of the oracle.
# Run under: bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
. tests/tools-om/lib.sh
tom_pin
OM=om/tools/own30b_prof.om
DRV=tests/tools-om/own30b_diff.c
OFF=-DCUDAPP_MIGRATION_PENDING_C_TOOLS_OWN30B
# seed tokens mutations-per-mille: heavy mutation, light mutation, clean past the 4096-token table, mixed past it.
CONFIGS="1:40:400 2:300:3 3:4200:0 4:4300:1 5:600:30 6:2:1000"

$CC $TOM_CFLAGS -DOWN30B_PROFILE_TEST $OFF -o "$tmp/test_c" tools/own30b_decode_profile.c
$CC $TOM_CFLAGS -DOWN30B_PROFILE_TEST -o "$tmp/test_om" tools/own30b_decode_profile.c src/cudapp_om_own30b_prof.c
"$tmp/test_c" | sed 's/^/oracle: /'
"$tmp/test_om" | sed 's/^/omega: /'
$CC $TOM_CFLAGS $OFF -o "$tmp/diff_c" $DRV
$CC $TOM_CFLAGS -o "$tmp/diff_om" $DRV src/cudapp_om_own30b_prof.c

run_all() { # BIN PREFIX
  for cfg in $CONFIGS; do
    IFS=: read -r s t m <<EOF
$cfg
EOF
    "$1" "$s" "$t" "$m" > "$2.$s.out" 2> "$2.$s.err"
  done
}
run_all "$tmp/diff_c" "$tmp/c"
run_all "$tmp/diff_om" "$tmp/om"
for cfg in $CONFIGS; do
  s=${cfg%%:*}
  cmp "$tmp/c.$s.out" "$tmp/om.$s.out"
  cmp "$tmp/c.$s.err" "$tmp/om.$s.err"
  echo "differential seed $s: identical ($(tail -n 1 "$tmp/c.$s.out"), $(head -n 1 "$tmp/c.$s.err" | sed 's/.*\(tokens=.*\)/\1/'), $(wc -l < "$tmp/c.$s.out") classifications)"
done
tom_flip "$tmp/c.3.err" "$tmp/om.3.err"

killed=0
mut() { # NAME SITE REPLACEMENT
  tom_mutant "$1" $OM "$2" "$3"
  $CC $TOM_CFLAGS -o "$tmp/$1.bin" $DRV "$tmp/$1.c"
  run_all "$tmp/$1.bin" "$tmp/$1" || true
  for cfg in $CONFIGS; do
    s=${cfg%%:*}
    if ! cmp -s "$tmp/c.$s.out" "$tmp/$1.$s.out" || ! cmp -s "$tmp/c.$s.err" "$tmp/$1.$s.err"; then
      echo "mutant $1: killed (seed $s)"; killed=$((killed + 1)); return 0
    fi
  done
  echo "FAIL mutant $1 survived"; exit 1
}
mut expect 'if lt pos 288' 'if lt pos 287'
mut complete 'if eq get st 45 289' 'if eq get st 45 288'
mut dropped 'put st 1 add get st 1 1' 'put st 1 add get st 1 2'
mut gu_shape 'if eq n 1536' 'if eq n 1537'
mut gap 'put st r add get st r sub beg get st 4' 'put st r add get st r sub get st 4 beg'
mut attn 'put st 5 mul 2048 add get a 5 1' 'put st 5 mul 2048 add get a 5 2'
mut digits 'x = div x 10' 'x = div x 11'
mut rearm 'put st 2 1' 'put st 2 0'
echo "own30b_prof mutants: $killed killed"

if [ -n "${TOOLS_OM_COVERAGE:-}" ]; then
  mkdir -p "$TOOLS_OM_COVERAGE"
  cov=$tmp/cov
  mkdir -p "$cov"
  cp tools/own30b_decode_profile.c $DRV "$cov/"
  sed -i 's#"../../tools/own30b_decode_profile.c"#"own30b_decode_profile.c"#' "$cov/own30b_diff.c"
  (cd "$cov" && $CC -std=gnu11 -D_GNU_SOURCE -O0 --coverage -I"$ROOT/src" $OFF -c -o own30b_diff.o own30b_diff.c && $CC --coverage -o diff_cov own30b_diff.o)
  for cfg in $CONFIGS; do
    IFS=: read -r s t m <<EOF
$cfg
EOF
    "$cov/diff_cov" "$s" "$t" "$m" > /dev/null 2>&1
  done
  (cd "$cov" && gcov -b -c own30b_diff.c > "$TOOLS_OM_COVERAGE/own30b.gcov.txt" 2>&1)
  cp "$cov/own30b_decode_profile.c.gcov" "$TOOLS_OM_COVERAGE/own30b_decode_profile.c.gcov"
  echo "coverage: $TOOLS_OM_COVERAGE/own30b.gcov.txt"
fi
echo "tools-om own30b_prof: PASS (differential $(echo $CONFIGS | wc -w) seeds, negative control, $killed mutants killed)"
