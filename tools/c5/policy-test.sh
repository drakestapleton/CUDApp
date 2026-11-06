#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only: never opens a GPU, invokes chip.sh, or downloads a compiler.
set -eu
cd "$(dirname "$0")/../.."
OMC=${OMEGA_OMC:-${OMC:-../omega-mv/seed/out/omc}}
B=${OUT_DIR:-build}/c5-policy
mkdir -p "$B"
read -r repo rev path want < tools/c5/omc.pin
[ -x "$OMC" ] || { echo 'C5 policy: NOT_RUN (pinned OMEGA_OMC missing)' >&2; exit 2; }
got=$(sha256sum "$OMC"); got=${got%% *}
[ "$got" = "$want" ] || { echo 'C5 policy: NOT_RUN (compiler pin mismatch)' >&2; exit 2; }
cat tools/c5/policy.om tools/c5/model.om tools/c5/cases.om > "$B/positive.om"
"$OMC" < "$B/positive.om" > "$B/positive" 2> "$B/positive.err" || { cat "$B/positive.err" >&2; exit 1; }
chmod +x "$B/positive"
"$B/positive"
# Mutation is a verification oracle only. Every mutant must compile and fail at its specified assertion.
python3 tools/c5/mutants.py "$B"
for entry in deadline:6 overflow:4 owner:12 generation:13 range:16 ack:19 order:20 epoch:37 stale:42; do
    name=${entry%:*}; want=${entry#*:}
    "$OMC" < "$B/$name.om" > "$B/$name" 2> "$B/$name.err" || { cat "$B/$name.err" >&2; exit 1; }
    chmod +x "$B/$name"
    rc=0; "$B/$name" > "$B/$name.log" 2>&1 || rc=$?
    [ "$rc" -eq "$want" ] || { echo "C5 mutant $name: FAIL (exit $rc, expected assertion $want)" >&2; exit 1; }
    echo "C5 mutant $name: caught (assertion $rc)"
done
sha256sum "$OMC" tools/c5/policy.om tools/c5/model.om tools/c5/cases.om "$B/positive"
echo 'C5 policy host+sim+mutants: PASS'
