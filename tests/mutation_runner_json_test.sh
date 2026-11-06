#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Copyright 2026 Drake Stapleton (AIEN / aien-dev)
#
# Regression: tools/mutation_runner.sh must write a receipt that parses as JSON
# and round-trips every string losslessly, including quotes, backslashes, tabs,
# control bytes and a multi-line build-failure note. Before the fix the guards
# text was inserted raw and the note had quotes/backslashes deleted.
set -u
here=$(cd "$(dirname "$0")/.." && pwd)
t=$(mktemp -d "${TMPDIR:-/tmp}/mrjson.XXXXXX") || exit 1
trap 'rm -rf "$t"' EXIT INT TERM
mkdir -p "$t/src"
printf 'value 1\n' > "$t/src/a.txt"
tab=$(printf '\t'); esc=$(printf '\033'); bel=$(printf '\007')
guards="checks \"q >= 1\" and back\\slash${tab}tab ${esc}[esc ${bel}bel"
printf '%s\n' "M_KILL~a.txt~s/value 1/value 2/~$guards" "M_BUILD~a.txt~s/value 1/value 3/~build \"breaks\"" > "$t/rows"
# The build fails only for mutant M_BUILD, with an error line holding quotes and a backslash.
build='if grep -q "value 3" a.txt; then echo "x.c:1: error: bad \"tok\" \\ here"; exit 1; fi'
sh "$here/tools/mutation_runner.sh" -k table -m "$t/rows" -d "$t/src" -b "$build" \
    -t 'grep -q "value 1" a.txt' -o "$t/out.json" >"$t/log" 2>&1
GUARDS=$guards python3 - "$t/out.json" <<'PY' || { echo "mutation runner JSON: FAIL"; exit 1; }
import json, os, sys
try:
    d = json.load(open(sys.argv[1]))
except ValueError as e:
    print("receipt does not parse:", e); sys.exit(1)
m = {r["id"]: r for r in d["mutants"]}
want = {
    ("M_KILL", "checks"): os.environ["GUARDS"],
    ("M_KILL", "status"): "KILLED",
    ("M_BUILD", "checks"): 'build "breaks"',
    ("M_BUILD", "status"): "ERROR",
    ("M_BUILD", "note"): 'build failed: x.c:1: error: bad "tok" \\ here',
}
bad = [(k, m.get(k[0], {}).get(k[1]), v) for k, v in want.items() if m.get(k[0], {}).get(k[1]) != v]
for b in bad: print("mismatch", b)
sys.exit(1 if bad else 0)
PY
# A receipt destination that cannot be opened must fail before running mutants.
if sh "$here/tools/mutation_runner.sh" -k table -m "$t/rows" -d "$t/src" \
    -t 'touch ran; exit 1' -o "$t/missing/out.json" >"$t/unwritable.log" 2>&1; then
    echo "mutation runner JSON: FAIL (unwritable receipt accepted)"
    exit 1
else
    rc=$?
    [ "$rc" -eq 2 ] || { echo "mutation runner JSON: FAIL (receipt exit $rc)"; exit 1; }
fi
echo "mutation runner JSON: PASS"
