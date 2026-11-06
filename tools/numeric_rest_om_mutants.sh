#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Run this entire script inside the lane's device-hidden bwrap CPU envelope.
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
CC=${CC:-cc}
LOG=${NUMERIC_REST_LOG_DIR:-$TMP/log}
mkdir -p "$LOG"
FILES='numeric.om tile.om rest.om patch.om check.om trace.om reduce.om'
MUTANTS='source_lane|rest.om|ret xor b 63|ret xor b 62
ftz_dropped|rest.om|ret nrclass 7 nrelem a nrclass 7 b nrclass 7 c nrclass 7 d 0|ret nrelem a b c d 0
parity_compare|trace.om|if eq cmpeq get tr 9 ex got 0|if eq cmpeq get tr 9 ex got 1
minmax_padding|rest.om|if or eq op 1 eq op 2|if eq op 99
patch_exit|patch.om|ret word4 k 31053 0 58720256 1042944|ret word4 k 31052 0 58720256 1042944
seed_bound|rest.om|1048576.0f64|1.0f64'
copy_units() {
    mkdir -p "$1"
    for f in $FILES; do cp "$ROOT/om/numeric/$f" "$1/$f"; done
}
run() {
    label=$1; dir=$2; out="$TMP/out-$label"
    mkdir -p "$out"
    OMEGA_UNITS="numeric=$dir/numeric.om numeric_tile=$dir/numeric.om,$dir/tile.om numeric_rest=$dir/numeric.om,$dir/rest.om numeric_patch=$dir/numeric.om,$dir/rest.om,$dir/patch.om numeric_check=$dir/numeric.om,$dir/rest.om,$dir/patch.om,$dir/check.om numeric_trace=$dir/numeric.om,$dir/rest.om,$dir/trace.om numeric_reduce=$dir/numeric.om,$dir/tile.om,$dir/rest.om,$dir/reduce.om" OMEGA_UNITS_DEST="$out" sh "$ROOT/om/units.sh" > "$LOG/$label-units.log" 2>&1 || { cat "$LOG/$label-units.log"; exit 1; }
    $CC -std=gnu11 -Wall -Wextra -Werror -D_GNU_SOURCE -O2 -ffp-contract=off -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_REST_DIFF -ffunction-sections -fdata-sections -Wl,--gc-sections -I"$out" -I"$ROOT/src" -o "$out/diff" "$ROOT/tests/numeric_rest_om_diff_test.c" "$ROOT/src/omega_numeric.c" "$ROOT/src/omega_numeric_reduce.c" "$ROOT/src/omega_blackwell_qmd.c" "$out"/cudapp_om_*.c -lm > "$LOG/$label-cc.log" 2>&1 || { cat "$LOG/$label-cc.log"; exit 1; }
    set +e
    "$out/diff" > "$LOG/$label-run.log" 2>&1
    rc=$?
    set -e
}
copy_units "$TMP/control"
run control "$TMP/control"
[ "$rc" = 0 ] || { cat "$LOG/control-run.log"; exit 1; }
echo "control rc=$rc $(grep 'NUMERIC REST OM DIFF PASS' "$LOG/control-run.log")"
# Every exported unit is a strong link dependency. Omission must fail to link.
for omit in rest patch check trace reduce; do
    srcs=''
    for unit in numeric numeric_tile numeric_rest numeric_patch numeric_check numeric_trace numeric_reduce; do
        [ "$unit" = "numeric_$omit" ] || srcs="$srcs $out/cudapp_om_$unit.c"
    done
    if $CC -std=gnu11 -D_GNU_SOURCE -O0 -ffp-contract=off -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_REST_DIFF -ffunction-sections -fdata-sections -Wl,--gc-sections -I"$out" -I"$ROOT/src" -o "$TMP/link-$omit" "$ROOT/tests/numeric_rest_om_diff_test.c" "$ROOT/src/omega_numeric.c" "$ROOT/src/omega_numeric_reduce.c" "$ROOT/src/omega_blackwell_qmd.c" $srcs -lm > "$LOG/link-$omit.log" 2>&1; then
        echo "FAIL missing strong unit $omit linked"; exit 1
    fi
    grep -q "undefined reference.*omega_numeric_$omit" "$LOG/link-$omit.log" || { cat "$LOG/link-$omit.log"; exit 1; }
    echo "strong-link $omit refused as expected"
done
printf '%s\n' "$MUTANTS" > "$TMP/list"
caught=0; total=0
while IFS='|' read -r name file search repl; do
    total=$((total+1)); dir="$TMP/$name"; copy_units "$dir"
    count=$(grep -cF "$search" "$dir/$file" || true)
    [ "$count" = 1 ] || { echo "FAIL $name site count=$count"; exit 1; }
    SEARCH="$search" REPL="$repl" awk '{i=index($0,ENVIRON["SEARCH"]);if(i)$0=substr($0,1,i-1) ENVIRON["REPL"] substr($0,i+length(ENVIRON["SEARCH"]));print}' "$dir/$file" > "$dir/$file.new"
    mv "$dir/$file.new" "$dir/$file"
    run "$name" "$dir"
    if [ "$rc" != 0 ] && grep -q '^FAIL ' "$LOG/$name-run.log"; then
        caught=$((caught+1)); echo "mutant $name killed rc=$rc $(grep -m1 '^FAIL ' "$LOG/$name-run.log")"
    else echo "FAIL mutant $name survived rc=$rc"; exit 1; fi
done < "$TMP/list"
[ "$caught" = "$total" ]
echo "NUMERIC REST OM MUTANTS PASS $caught/$total; chip NOT_RUN"
