#!/bin/sh
# Ten killed mutants. Each site is unique in its unit. A mutant that still
# prints PASS fails the gate. The .om edit is restored before the binary
# runs. Regeneration goes to a temp DEST so src/ stays clean.
# Compile is unwrapped; the mutant binary runs under bwrap.
set -eu
OUT=${1:?out}
OMC=${2:?omc}
BRIDGE=${3:?bridge}
ROOT=${4:?root}
BWRAP="bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10"
CC=${CC:-cc}
AV="bf16 e4m3 ewarg fp8am fp8en fp8q fp8sc gdnch gdnsc gdnsl rope scan widen gdnex gdnpt gdnut fp8x fp8xb"
restore=""

cleanup() {
  if [ -n "$restore" ] && [ -f "$restore.bak" ]; then
    mv "$restore.bak" "$restore"
  fi
}
trap cleanup EXIT

run_one() {
  name=$1
  file=$2
  old=$3
  new=$4
  src="$ROOT/om/apiviews/$file"
  if ! grep -F -q "$old" "$src"; then
    echo "FAIL mutant $name: site missing"
    exit 1
  fi
  n=$(grep -F -c "$old" "$src")
  if [ "$n" != 1 ]; then
    echo "FAIL mutant $name: site found $n times"
    exit 1
  fi
  cp "$src" "$src.bak"
  restore=$src
  sed -i "s|$old|$new|" "$src"
  dest=$(mktemp -d)
  if ! OMEGA_OMC="$OMC" OMEGA_BRIDGE="$BRIDGE" OMEGA_BRIDGE_RT="${OMEGA_BRIDGE_RT:-/tmp/bridge-73-apiviews}" \
      OMEGA_UNITS_DEST="$dest" sh "$ROOT/om/units.sh" >/tmp/apiviews-mutant-$name.log 2>&1; then
    echo "FAIL mutant $name: regen rc $?"
    cat /tmp/apiviews-mutant-$name.log
    rm -rf "$dest"
    exit 1
  fi
  mv "$src.bak" "$src"
  restore=""
  srcs=""
  for u in $AV; do
    if [ "$u" = "${file%.om}" ]; then
      srcs="$srcs $dest/cudapp_om_av_$u.c"
    else
      srcs="$srcs $ROOT/src/cudapp_om_av_$u.c"
    fi
  done
  bin=$OUT/mutant-$name
  if ! $CC -std=c11 -Wall -Wno-unused-function -I"$ROOT/src" -ffp-contract=off -O0 \
      -DCUDAPP_MIGRATION_PENDING_APIVIEWS_DIFF \
      -o "$bin" "$ROOT/tests/apiviews_host_test.c" $srcs "$ROOT/src/omega_gpu_hostmath.c" -lm; then
    echo "KILLED mutant $name: compile refused"
    rm -rf "$dest"
    return 0
  fi
  set +e
  $BWRAP "$bin" >"$dest/out" 2>"$dest/err"
  rc=$?
  set -e
  if [ "$rc" -eq 0 ] && grep -q '^PASS apiviews host' "$dest/out"; then
    echo "FAIL mutant $name: still PASS"
    cat "$dest/out" "$dest/err"
    rm -rf "$dest"
    exit 1
  fi
  echo "KILLED mutant $name rc $rc"
  rm -rf "$dest"
}

mkdir -p "$OUT"
run_one e4m3 e4m3.om 'if gt code 126' 'if gt code 64'
run_one scan scan.om 'if and eq a 0.0f32 eq b 0.0f32' 'if and eq a 1.0f32 eq b 0.0f32'
run_one fp8q fp8q.om 'if gt n 128' 'if gt n 64'
run_one ewarg ewarg.om 'if gt p0 2048' 'if gt p0 8'
run_one gdnch gdnch.om 'ret add mul hk 128 s' 'ret add mul hk 128 add s 1'
run_one gdnex gdnex.om 'put ng i neg f32 av_exp1 f64 get a i' 'put ng i f32 av_exp1 f64 get a i'
run_one gdnpt gdnpt.om 'put dst add base 3 0.0f32' 'put dst add base 3 1.0f32'
run_one gdnut gdnut.om 'if eq rem hh div vh 16 0' 'if eq rem hh div vh 16 1'
run_one fp8x fp8x.om 'fold = bitcast f32 uint 1602224128' 'fold = bitcast f32 uint 1065353216'
run_one fp8xb fp8xb.om 'fold = bitcast f32 uint 1602224128' 'fold = bitcast f32 uint 1065353216'
echo "PASS 10 mutants killed"
