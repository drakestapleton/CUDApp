#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# One-shot GB10 latency run. Builds and encodes before taking the shared lock.
# Usage: sh tools/latmeas/run_spark.sh OUTPUT_DIR
set -eu
if [ $# -ne 1 ] || [ -z "$1" ]; then
    echo "usage: sh tools/latmeas/run_spark.sh OUTPUT_DIR" >&2
    exit 2
fi
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
out=$1
case "$out" in
    /*) ;;
    *) out=$root/$out ;;
esac
cd "$root"
mkdir -p "$out"
echo "latmeas: building (outside the chip lock)"
make -j"$(nproc)" build/gpu_latmeas_test build/cudapp-doctor
echo "latmeas: doctor (outside the chip lock; NOT READY aborts before flock)"
./build/cudapp-doctor
git rev-parse HEAD > "$out/head.txt"
./build/gpu_latmeas_test --print-plan
./build/gpu_latmeas_test --emit "$out/kernels"
python3 tools/latmeas/diff_nak.py --check --md "$out/diff.md"
echo "latmeas: taking flock /tmp/aien-gb10.lock for the launch only; timeout 600"
echo "latmeas: estimated chip window 90 s upper bound (printed again by the binary)"
# flock waits here without holding the lock. timeout covers only the launch.
set +e
flock /tmp/aien-gb10.lock timeout 600 ./build/gpu_latmeas_test --launch "$out/kernels" --out "$out/raw.json"
rc=$?
set -e
if [ -f "$out/raw.json" ]; then
    python3 tools/latmeas/fill_template.py --results "$out/raw.json" --md "$out/sm121_vs_NAK.md"
fi
exit "$rc"
