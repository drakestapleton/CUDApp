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

write_receipt() {
    rec=$1/RECEIPT.txt
    {
        echo "utc: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
        echo "head: $(git rev-parse HEAD)"
        echo "head_parents: $(git rev-parse HEAD^@ 2>/dev/null | tr '\n' ' ')"
        if git diff --quiet && git diff --cached --quiet; then
            echo "worktree: clean"
        else
            echo "worktree: dirty"
        fi
        echo "kernel: $(uname -srvm)"
        echo "gcc: $(gcc --version | head -n 1)"
        if [ -r /proc/driver/nvidia/version ]; then
            echo "driver: $(head -n 1 /proc/driver/nvidia/version)"
        else
            echo "driver: unread"
        fi
        found=0
        for info in /proc/driver/nvidia/gpus/*/information; do
            [ -f "$info" ] || continue
            found=1
            echo "device: $(basename "$(dirname "$info")")"
            sed -n 's/^Model:[[:space:]]*/model: /p; s/^Bus Location:[[:space:]]*/bus: /p' "$info"
        done
        if [ "$found" -eq 0 ]; then
            echo "device: none"
        fi
        if [ -f "$1/raw.json" ]; then
            echo "raw_json_sha256: $(sha256sum "$1/raw.json" | awk '{print $1}')"
        else
            echo "raw_json_sha256: absent"
        fi
    } > "$rec"
    echo "latmeas: wrote $rec"
}

if [ "$1" = "--receipt-self-test" ]; then
    tmp=$(mktemp -d)
    printf '{}\n' > "$tmp/raw.json"
    write_receipt "$tmp"
    got=$(awk '/^raw_json_sha256: / {print $2}' "$tmp/RECEIPT.txt")
    want=$(sha256sum "$tmp/raw.json" | awk '{print $1}')
    grep -q '^utc: ' "$tmp/RECEIPT.txt"
    grep -q '^head: ' "$tmp/RECEIPT.txt"
    grep -q '^head_parents: ' "$tmp/RECEIPT.txt"
    grep -q '^kernel: ' "$tmp/RECEIPT.txt"
    grep -q '^gcc: ' "$tmp/RECEIPT.txt"
    grep -q '^device: ' "$tmp/RECEIPT.txt"
    test "$got" = "$want"
    rm -rf "$tmp"
    echo "receipt self-test PASS"
    exit 0
fi

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
echo "latmeas: the launch bound is the estimated chip window printed above"
# flock waits here without holding the lock. timeout covers only the launch.
set +e
flock /tmp/aien-gb10.lock timeout 600 ./build/gpu_latmeas_test --launch "$out/kernels" --out "$out/raw.json"
rc=$?
set -e
if [ -f "$out/raw.json" ]; then
    python3 tools/latmeas/fill_template.py --results "$out/raw.json" --md "$out/sm121_vs_NAK.md"
fi
write_receipt "$out"
exit "$rc"
