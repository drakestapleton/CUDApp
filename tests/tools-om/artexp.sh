#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# artexp acceptance: tools/artifact_export.c (seeded Omega om/tools/artexp.om) against its C oracle
# (-DCUDAPP_MIGRATION_PENDING_C_TOOLS_ARTEXP). Both builds link the real library (host only, no device).
# Every case runs each build in a fresh directory and compares stdout, stderr, exit and every file the
# export wrote (name, size, sha256) byte for byte. Then a flipped-record negative control and recompiled
# mutants. TOOLS_OM_COVERAGE=DIR writes gcov branch data of the oracle.
# Run under: bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
. tests/tools-om/lib.sh
tom_pin
SRC=tools/artifact_export.c
OFF=-DCUDAPP_MIGRATION_PENDING_C_TOOLS_ARTEXP
make -j3 OUT_DIR="$tmp/b" "$tmp/b/libomega_gpu.a" > "$tmp/b.log" 2>&1 ||
  { tail -20 "$tmp/b.log"; echo "FAIL building the library"; exit 1; }
LIBS="src/omega_native_artifact.c src/omega_cubin.c $tmp/b/libomega_gpu.a -lpthread -lm"
$CC $TOM_CFLAGS $OFF -o "$tmp/ae_c" $SRC $LIBS
$CC $TOM_CFLAGS -o "$tmp/ae_om" $SRC $LIBS

CM=647116fc1ba1e1394982fc8ba181ce565293f160a6b56afc0f02b2128a675704
AT=003696e422bba7274877ba9e01a61076ea0f862f32760cbaa06c60f85a115359
G=0123456789abcdef0123456789abcdef01234567
LONG=$(printf 'p%.0s' $(seq 1 5000))
# One case per line: SETUP then the arguments (shell words). SETUP: - nothing, exists (out.cubin is there),
# link (out.manifest is a symlink), nodir (the prefix names a missing directory).
cat > "$tmp/cases" <<EOF
-
- matmul
- attention
- bogus
- matmul --out
- matmul --bogus x
- matmul --out out --m
- matmul --out out --contract-id
- matmul --out out --commit
- matmul --out out --k
- matmul --out out --n
- attention --out out --q-heads
- attention --out out --kv-heads
- matmul --m 16 --k 16 --n 8 --contract-id $CM --commit $G
- matmul --m 16 --k 16 --n 8 --out out --commit $G
- matmul --m 16 --k 16 --n 8 --contract-id ${CM}0 --out out --commit $G
- matmul --m 16 --k 16 --n 8 --contract-id $(printf %.63s $CM) --out out --commit $G
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out out
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out out --commit $(printf %.39s $G)
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out out --commit ${G}0
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out out --commit 0123456789ABCDEF0123456789abcdef01234567
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out out --commit 0123456789abcdeg0123456789abcdef01234567
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out out --commit '012345678/abcdef0123456789abcdef01234567'
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out out --commit '0123456789:bcdef0123456789abcdef01234567'
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out out --commit '0123456789\`bcdef0123456789abcdef01234567'
- matmul --m 16 --k 16 --n 8 --contract-id $AT --out out --commit $G
- matmul --m 16 --k 16 --n 8 --contract-id $(printf %.63s $CM)5 --out out --commit $G
- attention --q-heads 8 --kv-heads 8 --contract-id $CM --out out --commit $G
- attention --q-heads 8 --kv-heads 8 --contract-id $(printf %.63s $AT)8 --out out --commit $G
- other --m 16 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmu --m 16 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmull --m 16 --k 16 --n 8 --contract-id $CM --out out --commit $G
- attentio --q-heads 8 --kv-heads 8 --contract-id $AT --out out --commit $G
- matmul --m 8208 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 16400 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 16 --n 8200 --contract-id $CM --out out --commit $G
- matmul --m 8193 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 16385 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 16 --n 8193 --contract-id $CM --out out --commit $G
- matmul --m 0 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 0 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 16 --n 0 --contract-id $CM --out out --commit $G
- matmul --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 17 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 17 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 16 --n 9 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 24 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 24 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 16 --n 12 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 32 --k 16 --n 64 --contract-id $CM --out out --commit $G
- matmul --m 64 --k 2048 --n 512 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 32 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 8192 --k 16384 --n 8192 --contract-id $CM --out out --commit $G
- matmul --m 8192 --k 32 --n 16 --contract-id $CM --out out --commit $G
- matmul --m 8192 --k 32 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 512 --k 32 --n 1024 --contract-id $CM --out out --commit $G
- matmul --m 256 --k 48 --n 520 --contract-id $CM --out out --commit $G
- matmul --m 8192 --k 16 --n 8192 --contract-id $CM --out out --commit $G
- matmul --m ' 16' --k '	16' --n '+8' --contract-id $CM --out out --commit $G
- matmul --m 0x10 --k 0X10 --n 010 --contract-id $CM --out out --commit $G
- matmul --m 0x1F --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 0x --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 0xg --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 08 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 020 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m '16 ' --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m '' --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 1e3 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m -16 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m -0 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 16 --n -18446744073709551608 --contract-id $CM --out out --commit $G
- matmul --m 16 --k 16 --n -18446744073709551616 --contract-id $CM --out out --commit $G
- matmul --m 4294967295 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 4294967296 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 18446744073709551615 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 18446744073709551616 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 99999999999999999999999 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 18446744073709551632 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m -18446744073709551632 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 00000000000000000000000000000016 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 0x00000000000000000000000000000010 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m '  -  16' --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m +-16 --k 16 --n 8 --contract-id $CM --out out --commit $G
- matmul --m 32 --m 16 --k 16 --n 8 --contract-id $AT --contract-id $CM --out x --out out --commit 1 --commit $G
- matmul --m 16 --k 16 --n 8 --q-heads 3 --kv-heads 999 --contract-id $CM --out out --commit $G
- attention --q-heads 8 --kv-heads 8 --contract-id $AT --out out --commit $G
- attention --q-heads 16 --kv-heads 8 --contract-id $AT --out out --commit $G
- attention --q-heads 32 --kv-heads 8 --contract-id $AT --out out --commit $G
- attention --q-heads 64 --kv-heads 8 --contract-id $AT --out out --commit $G
- attention --q-heads 64 --kv-heads 16 --contract-id $AT --out out --commit $G
- attention --q-heads 64 --kv-heads 32 --contract-id $AT --out out --commit $G
- attention --q-heads 64 --kv-heads 64 --contract-id $AT --out out --commit $G
- attention --q-heads 32 --kv-heads 1 --contract-id $AT --out out --commit $G
- attention --q-heads 64 --kv-heads 1 --contract-id $AT --out out --commit $G
- attention --q-heads 4 --kv-heads 1 --contract-id $AT --out out --commit $G
- attention --q-heads 2 --kv-heads 1 --contract-id $AT --out out --commit $G
- attention --q-heads 1 --kv-heads 1 --contract-id $AT --out out --commit $G
- attention --q-heads 12 --kv-heads 4 --contract-id $AT --out out --commit $G
- attention --q-heads 48 --kv-heads 1 --contract-id $AT --out out --commit $G
- attention --q-heads 24 --kv-heads 4 --contract-id $AT --out out --commit $G
- attention --q-heads 0 --kv-heads 8 --contract-id $AT --out out --commit $G
- attention --q-heads 8 --kv-heads 0 --contract-id $AT --out out --commit $G
- attention --q-heads 8 --kv-heads 3 --contract-id $AT --out out --commit $G
- attention --q-heads 65 --kv-heads 1 --contract-id $AT --out out --commit $G
- attention --q-heads 1 --kv-heads 65 --contract-id $AT --out out --commit $G
- attention --q-heads 64 --kv-heads 65 --contract-id $AT --out out --commit $G
- attention --kv-heads 8 --contract-id $AT --out out --commit $G
- attention --q-heads 0x40 --kv-heads 010 --contract-id $AT --out out --commit $G
- attention --q-heads 8 --kv-heads 8 --m 1 --contract-id $AT --out out --commit $G
exists matmul --m 16 --k 16 --n 8 --contract-id $CM --out out --commit $G
link matmul --m 16 --k 16 --n 8 --contract-id $CM --out out --commit $G
nodir matmul --m 16 --k 16 --n 8 --contract-id $CM --out missing/out --commit $G
exists attention --q-heads 8 --kv-heads 8 --contract-id $AT --out out --commit $G
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out $LONG --commit $G
- matmul --m 16 --k 16 --n 8 --contract-id $CM --out '' --commit $G
EOF

one() { # BIN SETUP ARGS...: the report of one case
  bin=$1 setup=$2
  shift 2
  d="$tmp/run"
  rm -rf "$d"; mkdir -p "$d"
  case $setup in
    exists) : > "$d/out.cubin" ;;
    link) ln -s /nonexistent "$d/out.manifest" ;;
  esac
  rc=0
  (cd "$d" && "$bin" "$@" < /dev/null > "$tmp/o" 2> "$tmp/e") || rc=$?
  echo "case $setup $(printf '%s|' "$@" | cut -c1-200)"
  sed 's/^/  out /' "$tmp/o"
  sed 's/^/  err /' "$tmp/e"
  echo "  exit $rc"
  (cd "$d" && find . \( -type f -o -type l \) | LC_ALL=C sort | while read -r f; do
    if [ -L "$f" ]; then echo "  link $f"; else echo "  file $f $(wc -c < "$f") $(sha256sum < "$f" | cut -c1-64)"; fi
  done)
}
run() { # BIN OUT: every case of $tmp/cases
  bin=$1
  while IFS= read -r line; do
    eval "set -- $line"
    one "$bin" "$@"
  done < "$tmp/cases" > "$2"
}
run "$tmp/ae_c" "$tmp/c.out"
run "$tmp/ae_om" "$tmp/om.out"
cmp "$tmp/c.out" "$tmp/om.out" || { echo "FAIL artexp differential"; diff "$tmp/c.out" "$tmp/om.out" | head -20; exit 1; }
cases=$(grep -c '^case ' "$tmp/c.out")
files=$(grep -c '^  file ' "$tmp/c.out")
outs=$(grep '^  exit ' "$tmp/c.out" | sort | uniq -c | tr -s ' \n' ' ')
grep -q '^  out artifact_id=' "$tmp/c.out" || { echo "FAIL artexp oracle exported nothing"; exit 1; }
echo "differential: $cases cases identical, $files files written (exits:$outs)"
tom_flip "$tmp/c.out" "$tmp/om.out"

killed=0
mut() { # NAME SITE REPLACEMENT
  tom_mutant "$1" om/tools/artexp.om "$2" "$3"
  $CC $TOM_CFLAGS -o "$tmp/$1.bin" $SRC "$tmp/$1.c" $LIBS
  run "$tmp/$1.bin" "$tmp/$1.out"
  if cmp -s "$tmp/c.out" "$tmp/$1.out"; then echo "FAIL mutant $1 survived"; exit 1; fi
  echo "mutant $1: killed ($(diff "$tmp/c.out" "$tmp/$1.out" | grep -m1 '^[<>]' | cut -c1-60))"
  killed=$((killed + 1))
}
mut ktile 'if ne rem k 16 0' 'if ne rem k 8 0'
mut single 'if eq get s 14 16' 'if eq get s 14 32'
mut colgrid 'gx = div n 8' 'gx = div n 16'
mut hexbase 'b = ulong 16' 'b = ulong 10'
mut overflow 'ov = ulong 1' 'ov = ulong 0'
mut u32max 'if gt v 4294967295' 'if gt v 4294967296'
mut shared 'sh = and mul sh 512 4294967295' 'sh = and mul sh 256 4294967295'
mut lowhex 'if not or and ge c 48 le c 57 and ge c 97 le c 102' 'if not or and ge c 48 le c 57 and ge c 97 le c 101'
mut pow2 'while lt shl ulong 1 l r bound 64' 'while le shl ulong 1 l r bound 64'
mut hbound 'if gt get s 17 64' 'if gt get s 17 63'
echo "artexp mutants: $killed killed"

if [ -n "${TOOLS_OM_COVERAGE:-}" ]; then
  mkdir -p "$TOOLS_OM_COVERAGE" "$tmp/cov"
  cp $SRC "$tmp/cov/artifact_export.c"
  (cd "$tmp/cov" && $CC -std=gnu11 -D_GNU_SOURCE -O0 --coverage -I"$ROOT/src" $OFF -o ae_cov artifact_export.c \
    "$ROOT/src/omega_native_artifact.c" "$ROOT/src/omega_cubin.c" "$tmp/b/libomega_gpu.a" -lpthread -lm)
  run "$tmp/cov/ae_cov" "$tmp/cov/r.out"
  cmp -s "$tmp/c.out" "$tmp/cov/r.out" || { echo "FAIL artexp coverage build differs from the oracle"; exit 1; }
  (cd "$tmp/cov" && gcov -b -c ae_cov-artifact_export.c > "$TOOLS_OM_COVERAGE/artexp.gcov.txt" 2>&1)
  cp "$tmp/cov/artifact_export.c.gcov" "$TOOLS_OM_COVERAGE/artifact_export.c.gcov"
  echo "coverage: $TOOLS_OM_COVERAGE/artexp.gcov.txt"
fi
echo "tools-om artexp: PASS (differential $cases cases, $files files, negative control, $killed mutants killed)"
