#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# om_vs_c.sh <om-sm121dis> <c-sm121dis> <kernel-dir> <work-dir>: the .om decoder (default build) against the
# migration-pending C decoder (CUDAPP_DISASM=c build) on every <kernel-dir>/*.bin. Both print --pairs; the two
# texts must be byte for byte the same, exit status included. No NVIDIA tool is used. Exit 0 only when every
# kernel is identical; 2 when a decoder or the kernels are missing.
set -u
OM=$1; C=$2; DIR=$3; WORK=$4
[ -x "$OM" ] && [ -x "$C" ] || { echo "VERDICT NOT_RUN: decoder missing"; exit 2; }
ls "$DIR"/*.bin >/dev/null 2>&1 || { echo "VERDICT NOT_RUN: no kernels in $DIR"; exit 2; }
mkdir -p "$WORK"
k=0; bad=0; words=0
for f in "$DIR"/*.bin; do
    b=$(basename "$f" .bin)
    k=$((k + 1))
    ro=0; rc=0
    "$OM" --pairs "$f" > "$WORK/$b.om" 2>&1 || ro=$?
    "$C" --pairs "$f" > "$WORK/$b.c" 2>&1 || rc=$?
    words=$((words + $(wc -l < "$WORK/$b.c")))
    if [ "$ro" -ne "$rc" ] || ! cmp -s "$WORK/$b.om" "$WORK/$b.c"; then
        bad=$((bad + 1))
        echo "[FAIL] $b: om exit $ro, C exit $rc"
        diff "$WORK/$b.c" "$WORK/$b.om" | head -6
    fi
    rm -f "$WORK/$b.om" "$WORK/$b.c"
done
echo "om vs C: kernels: $k, words: $words, differing kernels: $bad"
[ $bad = 0 ] && { echo "VERDICT PASS"; exit 0; }
echo "VERDICT FAIL"; exit 1
