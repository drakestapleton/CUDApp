#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Copyright 2026 Drake Stapleton (AIEN / aien-dev)
#
# Mutants of the numeric Omega units (om/numeric/numeric.om, om/numeric/tile.om).
# Each mutant edits one site of a copy, regenerates the units with om/units.sh into a
# scratch directory, builds tests/numeric_om_diff_test.c against them and must be
# caught: the C-vs-Omega differential has to abort with "FAIL numeric diff".
# The unmutated copy (control) must pass. Needs OMEGA_OMC and OMEGA_BRIDGE (om/units.sh).
# Prints NUMERIC OM MUTANTS PASS <caught>/<total> on success.
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
CC=${CC:-cc}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
CFLAGS="-std=gnu11 -Wall -Wextra -Werror -D_GNU_SOURCE -O2 -ffp-contract=off -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_DIFF -DNUMERIC_DIFF_RANDOM=4096"

# name|file|search|replacement (the search text must occur exactly once)
MUTANTS='tie_to_odd|numeric.om|if and eq g 1 or ne r 0 eq and mant 1 1|if and eq g 1 or ne r 0 eq and mant 1 0
f16_nan_quiet_bit|numeric.om|ret or or sign INF or QBIT shl f 13|ret or or sign INF shl f 13
narrow_nan_quiet_bit|numeric.om|shl ulong 1 sub man 1 shr f sub 23 man|shl ulong 1 sub man 2 shr f sub 23 man
exp_ln2_hi|numeric.om|mul fk f32of 1060205056|mul fk f32of 1060205057
tile_stride|tile.om|get v add i d|get v add i 1'

run() { # $1 = label, $2 = directory with numeric.om and tile.om
	out="$TMP/out-$1"
	mkdir -p "$out"
	OMEGA_UNITS="numeric=$2/numeric.om numeric_tile=$2/numeric.om,$2/tile.om" OMEGA_UNITS_DEST="$out" \
		sh "$ROOT/om/units.sh" > "$out/units.log" 2>&1 || { echo "FAIL $1: om/units.sh"; cat "$out/units.log"; exit 1; }
	$CC $CFLAGS -I"$out" -I"$ROOT/src" -o "$out/diff" "$ROOT/tests/numeric_om_diff_test.c" \
		"$ROOT/src/omega_numeric.c" "$ROOT/src/omega_numeric_reduce.c" "$out/cudapp_om_numeric.c" \
		"$out/cudapp_om_numeric_tile.c" -lm -ffunction-sections -fdata-sections -Wl,--gc-sections \
		> "$out/cc.log" 2>&1 || { echo "FAIL $1: build"; cat "$out/cc.log"; exit 1; }
	set +e
	"$out/diff" > "$out/run.log" 2>&1
	rc=$?
	set -e
}

mkdir -p "$TMP/control"
cp "$ROOT/om/numeric/numeric.om" "$ROOT/om/numeric/tile.om" "$TMP/control/"
run control "$TMP/control"
[ "$rc" = 0 ] && grep -q 'NUMERIC OM DIFF PASS' "$out/run.log" || { echo "FAIL control rc=$rc"; cat "$out/run.log"; exit 1; }
echo "control: rc=0 $(tail -n 1 "$out/run.log")"

total=0
caught=0
printf '%s\n' "$MUTANTS" > "$TMP/list"
while IFS='|' read -r name file search repl; do
	total=$((total + 1))
	dir="$TMP/m-$name"
	mkdir -p "$dir"
	cp "$ROOT/om/numeric/numeric.om" "$ROOT/om/numeric/tile.om" "$dir/"
	n=$(grep -cF -- "$search" "$dir/$file" || true)
	[ "$n" = 1 ] || { echo "FAIL mutant $name: site found $n times"; exit 1; }
	SEARCH="$search" REPL="$repl" awk '{ i = index($0, ENVIRON["SEARCH"]); if (i) $0 = substr($0, 1, i - 1) ENVIRON["REPL"] substr($0, i + length(ENVIRON["SEARCH"])); print }' \
		"$dir/$file" > "$dir/$file.new"
	mv "$dir/$file.new" "$dir/$file"
	run "$name" "$dir"
	if [ "$rc" != 0 ] && grep -q 'FAIL numeric diff' "$out/run.log"; then
		caught=$((caught + 1))
		echo "mutant $name: caught rc=$rc $(grep -m1 'FAIL numeric diff' "$out/run.log")"
	else
		echo "FAIL mutant $name survived rc=$rc"
	fi
done < "$TMP/list"
[ "$caught" = "$total" ] || { echo "FAIL numeric mutants $caught/$total caught"; exit 1; }
echo "NUMERIC OM MUTANTS PASS $caught/$total"
