#!/bin/sh
# cuda202 (DRA-202) clean-room reproduction, step C, run ON the Mac (Apple M2): docs/repro/README.md step 5
# from a fresh clone of the pushed branch, cleared environment (env -i), empty HOME, nice 10. The producer
# outputs (gpu-in) were copied here from the Spark's stepA output with scp beforehand.
# usage: macrun.sh COMMIT
set -u
C=$1
B=$HOME/work/hive/cuda202
mkdir -p "$B/home" "$B/tmp" || exit 74
E="env -i HOME=$B/home PATH=/usr/bin:/bin:/usr/sbin:/sbin TMPDIR=$B/tmp LC_ALL=C"
$E git clone -q --filter=blob:none https://github.com/aien-dev/CUDApp "$B/CUDApp" && (cd "$B/CUDApp" && $E git checkout -q "$C") || exit 74
cd "$B/CUDApp" || exit 74
echo "CUDAPP $(git rev-parse HEAD) $(sw_vers -productVersion) $(uname -m) $(date -u +%FT%TZ)"
$E nice -n 10 sh tools/repro/replay-apple.sh "$B/out"; echo "replay exit $?"
