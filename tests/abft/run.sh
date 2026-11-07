#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# DRA-420/DRA-387 GEMM fault-detection evaluator: build and run on the host.
#   OMEGA_OMC=/abs/omega-mv/seed/out/omc sh tests/abft/run.sh [OUT_DIR] [FIRST LAST]
# Exit status: 0 every gate passed; otherwise the failing step's status.
# The pinned compiler is omega-mv e5b32567 seed/out/omc (sha256 below); any other compiler is refused.
set -eu
cd "$(dirname "$0")/../.."
PIN=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
OMC=${OMEGA_OMC:?set OMEGA_OMC to the pinned omega-mv e5b32567 seed/out/omc}
OUT=${1:-build/abft}
mkdir -p "$OUT"
printf '%s  %s\n' "$PIN" "$OMC" | sha256sum -c - >/dev/null
sha256sum -c tests/abft/FROZEN.sha256 >/dev/null
cat tests/abft/frozen.om native/abft/lib.om native/abft/check.om native/abft/cand.om \
	native/abft/sched.om native/abft/run.om > "$OUT/abft.om"
"$OMC" < "$OUT/abft.om" > "$OUT/abft.tmp"
chmod +x "$OUT/abft.tmp" && mv "$OUT/abft.tmp" "$OUT/abft"
cat native/learn53/task.om tests/learn53/fixtures.om native/abft/cul1.om tests/abft/cul1-unit.om \
	> "$OUT/cul1-unit.om"
"$OMC" < "$OUT/cul1-unit.om" > "$OUT/cul1-unit.tmp"
chmod +x "$OUT/cul1-unit.tmp" && mv "$OUT/cul1-unit.tmp" "$OUT/cul1-unit"
sha256sum "$OUT/abft" "$OUT/cul1-unit" "$OUT/abft.om" "$OUT/cul1-unit.om"
"$OUT/cul1-unit"
shift $(( $# > 0 ? 1 : 0 ))
rc=0
"$OUT/abft" "$@" > "$OUT/abft.log" || rc=$?
grep '^GATE' "$OUT/abft.log" || true
exit $rc
