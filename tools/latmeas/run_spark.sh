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
    gitdir=${CUDAPP_RECEIPT_GIT:-.}
    launch_cmd=${2:-}
    launch_rc=${3:-}
    {
        echo "utc: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
        echo "head: $(git -C "$gitdir" rev-parse HEAD)"
        echo "head_parents: $(git -C "$gitdir" rev-parse HEAD^@ 2>/dev/null | tr '\n' ' ')"
        if [ -n "$(git -C "$gitdir" status --porcelain)" ]; then
            echo "worktree: dirty"
        else
            echo "worktree: clean"
        fi
        if [ -n "$launch_cmd" ]; then
            echo "launch_command: $launch_cmd"
            echo "launch_rc: $launch_rc"
        fi
        echo "kernel: $(uname -srvm)"
        echo "gcc: $(gcc --version | head -n 1)"
        if [ -r /proc/driver/nvidia/version ]; then
            echo "driver: $(head -n 1 /proc/driver/nvidia/version)"
        else
            echo "driver: unread"
        fi
        found=0
        sysroot=${CUDAPP_RECEIPT_SYSROOT:-}
        for info in ${CUDAPP_RECEIPT_GPU_INFO:-/proc/driver/nvidia/gpus/*/information}; do
            [ -f "$info" ] || continue
            found=1
            bdf=$(basename "$(dirname "$info")")
            echo "device: $bdf"
            model=$(sed -n 's/^Model:[[:space:]]*//p' "$info" | head -n 1)
            bus=$(sed -n 's/^Bus Location:[[:space:]]*//p' "$info" | head -n 1)
            # The open module prints "Model: Unknown" for the GB10. The doctor names
            # that chip from PCI id 0x2e12 (src/omega_gpu_chip.c).
            if [ -z "$model" ] || [ "$model" = "Unknown" ]; then
                ven=$(cat "$sysroot/sys/bus/pci/devices/$bdf/vendor" 2>/dev/null || true)
                dev=$(cat "$sysroot/sys/bus/pci/devices/$bdf/device" 2>/dev/null || true)
                ven=$(printf '%s' "$ven" | tr -d '[:space:]')
                dev=$(printf '%s' "$dev" | tr -d '[:space:]')
                if [ "$ven" = "0x10de" ] && [ "$dev" = "0x2e12" ]; then
                    echo "model: NVIDIA GB10 (sm_121)"
                    echo "model_source: pci 0x2e12; proc Model was ${model:-empty}"
                else
                    echo "model: Unknown"
                    echo "model_source: proc Model was ${model:-empty}; pci ${ven:-unread}:${dev:-unread}"
                fi
            else
                echo "model: $model"
            fi
            if [ -n "$bus" ]; then
                echo "bus: $bus"
            fi
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
    write_receipt "$tmp" "flock /tmp/aien-gb10.lock timeout 600 ./build/gpu_latmeas_test --launch $tmp/kernels --out $tmp/raw.json" 0
    got=$(awk '/^raw_json_sha256: / {print $2}' "$tmp/RECEIPT.txt")
    want=$(sha256sum "$tmp/raw.json" | awk '{print $1}')
    grep -q '^utc: ' "$tmp/RECEIPT.txt"
    grep -q '^head: ' "$tmp/RECEIPT.txt"
    grep -q '^head_parents: ' "$tmp/RECEIPT.txt"
    grep -q '^kernel: ' "$tmp/RECEIPT.txt"
    grep -q '^gcc: ' "$tmp/RECEIPT.txt"
    grep -q '^device: ' "$tmp/RECEIPT.txt"
    test "$got" = "$want"
    grep -q '^launch_command: ' "$tmp/RECEIPT.txt"
    grep -q '^launch_rc: 0$' "$tmp/RECEIPT.txt"
    rm -rf "$tmp"
    # proc Model "Unknown" plus PCI 10de:2e12 is the GB10 the doctor names.
    fake=$(mktemp -d)
    mkdir -p "$fake/gpus/000f:01:00.0" "$fake/sys/bus/pci/devices/000f:01:00.0"
    printf 'Model: Unknown\nBus Location: 000f:01:00.0\n' > "$fake/gpus/000f:01:00.0/information"
    printf '0x10de\n' > "$fake/sys/bus/pci/devices/000f:01:00.0/vendor"
    printf '0x2e12\n' > "$fake/sys/bus/pci/devices/000f:01:00.0/device"
    printf '{}\n' > "$fake/raw.json"
    CUDAPP_RECEIPT_SYSROOT=$fake CUDAPP_RECEIPT_GPU_INFO=$fake/gpus/000f:01:00.0/information \
        write_receipt "$fake"
    grep -q '^model: NVIDIA GB10 (sm_121)$' "$fake/RECEIPT.txt"
    grep -q '^model_source: pci 0x2e12' "$fake/RECEIPT.txt"
    rm -rf "$fake"
    # An untracked file must mark the tree dirty. git diff --quiet would miss it.
    repo=$(mktemp -d)
    rout=$(mktemp -d)
    git -C "$repo" init -q
    git -C "$repo" config user.email receipt@example.com
    git -C "$repo" config user.name receipt
    echo tracked > "$repo/a"
    git -C "$repo" add a
    git -C "$repo" commit -q -m init
    printf '{}\n' > "$rout/raw.json"
    echo untracked > "$repo/untracked"
    CUDAPP_RECEIPT_GIT=$repo write_receipt "$rout" "launch-cmd" 7
    grep -q '^worktree: dirty$' "$rout/RECEIPT.txt"
    grep -q '^launch_rc: 7$' "$rout/RECEIPT.txt"
    rm "$repo/untracked"
    CUDAPP_RECEIPT_GIT=$repo write_receipt "$rout" "launch-cmd" 0
    grep -q '^worktree: clean$' "$rout/RECEIPT.txt"
    rm -rf "$repo" "$rout"
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
launch_cmd="flock /tmp/aien-gb10.lock timeout 600 ./build/gpu_latmeas_test --launch $out/kernels --out $out/raw.json"
flock /tmp/aien-gb10.lock timeout 600 ./build/gpu_latmeas_test --launch "$out/kernels" --out "$out/raw.json"
rc=$?
set -e
if [ -f "$out/raw.json" ]; then
    python3 tools/latmeas/fill_template.py --results "$out/raw.json" --md "$out/sm121_vs_NAK.md"
fi
write_receipt "$out" "$launch_cmd" "$rc"
exit "$rc"
