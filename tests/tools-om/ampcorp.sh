#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# ampcorp acceptance: tools/ampere_corpus.c (seeded Omega om/tools/ampcorp.om) against its C oracle
# (-DCUDAPP_MIGRATION_PENDING_C_TOOLS_AMPCORP) through tests/tools-om/ampcorp_diff.c, which supplies the SM86
# encoder outcome per case: exit or signal, stderr and every written file byte for byte, a flipped-record
# negative control, and recompiled mutants. TOOLS_OM_COVERAGE=DIR also writes gcov branch data of the oracle.
# Run under: bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
. tests/tools-om/lib.sh
tom_pin
SRC=tools/ampere_corpus.c
DRV=tests/tools-om/ampcorp_diff.c
OFF=-DCUDAPP_MIGRATION_PENDING_C_TOOLS_AMPCORP
$CC $TOM_CFLAGS $OFF -o "$tmp/ac_c" $DRV $SRC
$CC $TOM_CFLAGS -o "$tmp/ac_om" $DRV $SRC src/cudapp_om_ampcorp.c
run() { mkdir -p "$2.d" && (cd "$2.d" && "$1") > "$2.out"; }
run "$tmp/ac_c" "$tmp/c"
run "$tmp/ac_om" "$tmp/om"
cmp "$tmp/c.out" "$tmp/om.out" || { echo "FAIL ampcorp differential"; diff "$tmp/c.out" "$tmp/om.out" | head -20; exit 1; }
cases=$(grep -c '^case ' "$tmp/c.out")
files=$(grep -c '^  file ' "$tmp/c.out")
outs=$(grep -E '^  (exit|signal) ' "$tmp/c.out" | sort | uniq -c | tr -s ' \n' ' ')
echo "differential: $cases cases identical ($files files; outcomes:$outs)"
tom_flip "$tmp/c.out" "$tmp/om.out"

killed=0
mut() { # NAME SITE REPLACEMENT
  tom_mutant "$1" om/tools/ampcorp.om "$2" "$3"
  $CC $TOM_CFLAGS -o "$tmp/$1.bin" $DRV $SRC "$tmp/$1.c"
  run "$tmp/$1.bin" "$tmp/$1"
  if cmp -s "$tmp/c.out" "$tmp/$1.out"; then echo "FAIL mutant $1 survived"; exit 1; fi
  echo "mutant $1: killed ($(diff "$tmp/c.out" "$tmp/$1.out" | grep -m1 '^[<>]' | cut -c1-60))"
  killed=$((killed + 1))
}
mut dotdot 'if eq sub i s 2' 'if eq sub i s 3'
mut backslash 'if eq get d i uchar 92' 'if eq get d i uchar 93'
mut lg2 'if gt and op 4294967295 get f 8' 'if ge and op 4294967295 get f 8'
mut f2e4 'if eq im get f 12' 'if ne im get f 12'
mut stray 'if ne get code i uchar 165' 'if eq get code i uchar 0'
mut pathmax 'if ge n ACMAX' 'if gt n ACMAX'
mut rcsign 'if ne shr rc 63 0' 'if eq shr rc 63 0'
mut lenchk 'if or ne rc 0 ne nb get f 5' 'if ne rc 0'
echo "ampcorp mutants: $killed killed"

if [ -n "${TOOLS_OM_COVERAGE:-}" ]; then
  mkdir -p "$TOOLS_OM_COVERAGE" "$tmp/cov"
  cp $SRC "$tmp/cov/ampere_corpus.c"
  (cd "$tmp/cov" && $CC -std=gnu11 -D_GNU_SOURCE -O0 --coverage -I"$ROOT/src" $OFF -DAMPCORP_GCOV -o ac_cov "$ROOT/$DRV" ampere_corpus.c)
  run "$tmp/cov/ac_cov" "$tmp/cov/r"
  (cd "$tmp/cov" && gcov -b -c ac_cov-ampere_corpus.c > "$TOOLS_OM_COVERAGE/ampcorp.gcov.txt" 2>&1)
  cp "$tmp/cov/ampere_corpus.c.gcov" "$TOOLS_OM_COVERAGE/ampere_corpus.c.gcov"
  echo "coverage: $TOOLS_OM_COVERAGE/ampcorp.gcov.txt"
fi

# TOOLS_OM_AMPCORP_REAL=1: the corpus producers of test-ampere-encoder-oracle (the SM86 encoder is real), built
# with CUDAPP_AMPERE_CORPUS=1 both ways in two OUT_DIRs, one corpus folder per producer; every file name (pid
# replaced) and sha256 must match. Builds the library twice.
real=
if [ -n "${TOOLS_OM_AMPCORP_REAL:-}" ]; then
  for v in om c; do
    sw=0; [ $v = c ] && sw=1
    make -j3 OUT_DIR="$tmp/b-$v" CUDAPP_AMPERE_CORPUS=1 CUDAPP_MIGRATION_PENDING_C_TOOLS_AMPCORP=$sw \
      "$tmp/b-$v/ampere_encoder_test" "$tmp/b-$v/ampere_families" "$tmp/b-$v/ampere_matmul_families" \
      > "$tmp/b-$v.log" 2>&1 || { tail -20 "$tmp/b-$v.log"; echo "FAIL building the $v producers"; exit 1; }
    for p in ampere_encoder_test ampere_families 'ampere_matmul_families --dump'; do
      d="$tmp/b-$v/corpus-${p%% *}"
      mkdir -p "$d"
      OMEGA_AMPERE_CORPUS_DIR="$d" "$tmp/b-$v/"$p > "$d.log" 2>&1 || { echo "FAIL $v $p"; exit 1; }
      (cd "$d" && for f in *; do echo "${p%% *} $(echo "$f" | sed 's/^[0-9]*-/PID-/') $(sha256sum < "$f" | cut -c1-64)"; done) \
        | sort -V >> "$tmp/b-$v.manifest"
    done
  done
  cmp "$tmp/b-c.manifest" "$tmp/b-om.manifest" || { echo "FAIL ampcorp real producers"; exit 1; }
  tom_flip "$tmp/b-c.manifest" "$tmp/b-om.manifest"
  real=", real producers $(wc -l < "$tmp/b-c.manifest") files"
  echo "real producers: $(wc -l < "$tmp/b-c.manifest") corpus files identical ($(cut -d' ' -f2 "$tmp/b-c.manifest" | sed 's/^PID-[0-9]*\.//' | sort | uniq -c | tr -s ' \n' ' '))"
fi
echo "tools-om ampcorp: PASS (differential $cases cases$real, negative control, $killed mutants killed)"
