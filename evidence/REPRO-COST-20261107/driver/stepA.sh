#!/bin/sh
# cuda202 (DRA-202) clean-room reproduction, step A: Spark CPU only, run under the queen pool lock
# (flock -o pool-10-11.lock taskset -c 10-11 nice -n 10). Follows docs/repro/README.md steps 1-3 from a
# fresh clone of the pushed branch, with a cleared environment (env -i) and an empty HOME, so no state of
# the author's account is used. Also re-runs the cost meter and its gate from that clone.
# usage: stepA.sh COMMIT
set -u
C=$1
B=$HOME/work/hive/t_c2fbba86/rr
mkdir -p "$B/home" "$B/tmp" || exit 74
E="env -i HOME=$B/home PATH=/usr/bin:/bin TMPDIR=$B/tmp LC_ALL=C"
cd "$B" || exit 74
echo "STEP1 $(date -u +%FT%TZ) clone"
$E git clone -q https://github.com/aien-dev/CUDApp "$B/CUDApp" && (cd "$B/CUDApp" && $E git checkout -q "$C") || exit 74
$E git clone -q --filter=blob:none --no-checkout https://github.com/aien-dev/omega-mv "$B/omega-mv" || exit 74
(cd "$B/omega-mv" && $E git sparse-checkout set --no-cone /seed/out/ && $E git checkout -q 369dbc96daa1684db2d7c2c7085bbf16b3fd77ee && sha256sum seed/out/omc) || exit 74
OMC=$B/omega-mv/seed/out/omc
echo "CUDAPP $(cd "$B/CUDApp" && git rev-parse HEAD)"
cd "$B/CUDApp" || exit 74
echo "STEP2 $(date -u +%FT%TZ) fetch-tools"
$E sh tools/repro/fetch-tools.sh "$B/tools"; echo "fetch exit $?"
echo "STEP3 $(date -u +%FT%TZ) produce"
$E sh tools/repro/produce.sh "$OMC" "$B/tools/root" "$B/out"; echo "produce exit $?"
echo "COST $(date -u +%FT%TZ)"
$E sh tools/repro/cost.sh "$OMC" > "$B/cost.out"; echo "cost exit $?"
cmp "$B/cost.out" tests/repro/cost.expect && echo "cost output = tests/repro/cost.expect"
$E sh tests/repro/run.sh "$OMC" > "$B/gate.log" 2>&1; echo "repro gate exit $?"
tail -n 1 "$B/gate.log"
echo "END $(date -u +%FT%TZ)"
