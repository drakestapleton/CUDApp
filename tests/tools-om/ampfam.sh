#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# ampfam acceptance: tools/ampere_families.c (seeded Omega om/tools/ampfam.om) against its C oracle
# (-DCUDAPP_MIGRATION_PENDING_C_TOOLS_AMPFAM). tests/tools-om/ampfam_stub.c stands in for the generators and
# encoders and traces every call; the baseline and ten outcomes at each of the 133 steps compare stdout,
# stderr, exit and the call trace byte for byte. Then a flipped-record negative control and recompiled
# mutants. TOOLS_OM_COVERAGE=DIR writes gcov branch data of the oracle. TOOLS_OM_AMPFAM_REAL=1 also links
# both builds against the real library and compares their output and SM86 corpus files.
# Run under: bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
. tests/tools-om/lib.sh
tom_pin
SRC=tools/ampere_families.c
STUB=tests/tools-om/ampfam_stub.c
OFF=-DCUDAPP_MIGRATION_PENDING_C_TOOLS_AMPFAM
KINDS="g g7 r a0keep rc1 rc0 rcm1 rcm2 bplus bkeep"
$CC $TOM_CFLAGS $OFF -o "$tmp/af_c" $SRC $STUB
$CC $TOM_CFLAGS -o "$tmp/af_om" $SRC $STUB src/cudapp_om_ampfam.c

one() { # BIN CASE: the report of one run
  rm -f "$tmp/tr"
  rc=0
  AMPFAM_CASE=$2 AMPFAM_TRACE_FD=3 "$1" 3>> "$tmp/tr" > "$tmp/o" 2> "$tmp/e" || rc=$?
  echo "case $2"
  sed 's/^/  out /' "$tmp/o"
  sed 's/^/  err /' "$tmp/e"
  echo "  exit $rc"
  [ ! -f "$tmp/tr" ] || sed 's/^/  /' "$tmp/tr"
}
run() { # BIN OUT
  {
    one "$1" none
    k=1
    while [ $k -le 133 ]; do
      for kind in $KINDS; do one "$1" "$k:$kind"; done
      k=$((k + 1))
    done
  } > "$2"
}
run "$tmp/af_c" "$tmp/c.out"
run "$tmp/af_om" "$tmp/om.out"
cmp "$tmp/c.out" "$tmp/om.out" || { echo "FAIL ampfam differential"; diff "$tmp/c.out" "$tmp/om.out" | head -20; exit 1; }
cases=$(grep -c '^case ' "$tmp/c.out")
steps=$(sed -n '/^case none$/,/^case 1:g$/p' "$tmp/c.out" | grep -c '^  gen ')
grep -q '^  out Ampere extended existing generator families encoded (host only)$' "$tmp/c.out" ||
  { echo "FAIL ampfam baseline did not finish"; exit 1; }
outs=$(grep '^  exit ' "$tmp/c.out" | sort | uniq -c | tr -s ' \n' ' ')
echo "differential: $cases cases identical ($steps steps in the baseline; outcomes:$outs)"
tom_flip "$tmp/c.out" "$tmp/om.out"

killed=0
mut() { # NAME SITE REPLACEMENT
  tom_mutant "$1" om/tools/ampfam.om "$2" "$3"
  $CC $TOM_CFLAGS -o "$tmp/$1.bin" $SRC $STUB "$tmp/$1.c"
  run "$tmp/$1.bin" "$tmp/$1.out"
  if cmp -s "$tmp/c.out" "$tmp/$1.out"; then echo "FAIL mutant $1 survived"; exit 1; fi
  echo "mutant $1: killed ($(diff "$tmp/c.out" "$tmp/$1.out" | grep -m1 '^[<>]' | cut -c1-60))"
  killed=$((killed + 1))
}
mut line 'x = af_set s 22 0 112' 'x = af_set s 22 0 113'
mut ternskip 'if and gt k get f 17 eq e get f 18' 'if and ge k get f 17 eq e get f 18'
mut refuserc 'if or ne rc sub ulong 0 2 ne get s 4 0' 'if or ne rc sub ulong 0 1 ne get s 4 0'
mut append 'if eq m get f 2' 'if ne m get f 2'
mut resmode 'x = af_fl s 2 get f 9' 'x = af_fl s 2 get f 7'
mut binit 'put s 10 get s 5' 'put s 10 ulong 0'
mut qpc 'x = af_fl s 5 4' 'x = af_fl s 5 6'
mut lencheck 'if or ne rc 0 ne get s 3 get s 4' 'if ne rc 0'
mut gdnbf16 'x = af_set s 24 1 122' 'x = af_set s 24 0 122'
echo "ampfam mutants: $killed killed"

if [ -n "${TOOLS_OM_COVERAGE:-}" ]; then
  mkdir -p "$TOOLS_OM_COVERAGE" "$tmp/cov"
  cp $SRC "$tmp/cov/ampere_families.c"
  (cd "$tmp/cov" && $CC -std=gnu11 -D_GNU_SOURCE -O0 --coverage -I"$ROOT/src" $OFF -o af_cov ampere_families.c "$ROOT/$STUB")
  run "$tmp/cov/af_cov" "$tmp/cov/r.out"
  cmp -s "$tmp/c.out" "$tmp/cov/r.out" || { echo "FAIL ampfam coverage build differs from the oracle"; exit 1; }
  (cd "$tmp/cov" && gcov -b -c af_cov-ampere_families.c > "$TOOLS_OM_COVERAGE/ampfam.gcov.txt" 2>&1)
  cp "$tmp/cov/ampere_families.c.gcov" "$TOOLS_OM_COVERAGE/ampere_families.c.gcov"
  echo "coverage: $TOOLS_OM_COVERAGE/ampfam.gcov.txt"
fi

# TOOLS_OM_AMPFAM_REAL=1: one library with CUDAPP_AMPERE_CORPUS=1 (real generators and encoders), the tool
# linked against it both ways; stdout, stderr, exit and every corpus file (name with the pid replaced and
# sha256) must match.
real=
if [ -n "${TOOLS_OM_AMPFAM_REAL:-}" ]; then
  make -j3 OUT_DIR="$tmp/b" CUDAPP_AMPERE_CORPUS=1 "$tmp/b/libomega_gpu.a" > "$tmp/b.log" 2>&1 ||
    { tail -20 "$tmp/b.log"; echo "FAIL building the library"; exit 1; }
  for v in om c; do
    sw=; [ $v = c ] && sw=$OFF
    $CC $TOM_CFLAGS $sw -DCUDAPP_AMPERE_CORPUS -o "$tmp/real-$v" $SRC "$tmp/b/libomega_gpu.a" -lpthread -lm
    d="$tmp/corpus-$v"
    mkdir -p "$d"
    rc=0
    OMEGA_AMPERE_CORPUS_DIR="$d" "$tmp/real-$v" > "$tmp/real-$v.out" 2>&1 || rc=$?
    { cat "$tmp/real-$v.out"; echo "exit $rc"
      (cd "$d" && for f in *; do echo "$(echo "$f" | sed 's/^[0-9]*-/PID-/') $(sha256sum < "$f" | cut -c1-64)"; done) | sort -V
    } > "$tmp/real-$v.manifest"
  done
  cmp "$tmp/real-c.manifest" "$tmp/real-om.manifest" || { echo "FAIL ampfam real build"; diff "$tmp/real-c.manifest" "$tmp/real-om.manifest" | head; exit 1; }
  grep -qx 'exit 0' "$tmp/real-c.manifest" || { echo "FAIL ampfam real oracle did not pass"; exit 1; }
  tom_flip "$tmp/real-c.manifest" "$tmp/real-om.manifest"
  files=$(grep -c '^PID-' "$tmp/real-c.manifest")
  real=", real library $files corpus files"
  echo "real library: output and $files corpus files identical ($(grep '^PID-' "$tmp/real-c.manifest" | cut -d' ' -f1 | sed 's/^PID-[0-9]*\.//' | sort | uniq -c | tr -s ' \n' ' '))"
fi
echo "tools-om ampfam: PASS (differential $cases cases$real, negative control, $killed mutants killed)"
