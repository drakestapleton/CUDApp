#!/bin/sh
# cuda202 (DRA-202) clean-room reproduction, step B: docs/repro/README.md step 4 on the GB10 inside one
# exclusive spark-window lease (chip + quiet). Run the whole script under the queen pool lock, so the CPU
# cores are held BEFORE the GPU window opens and the window is never held while waiting for CPU:
#   flock -o pool-10-11.lock taskset -c 10-11 nice -n 10 sh stepB.sh
# (first attempt, window 20261107T025904Z-d0e9ce, took the pool lock inside the window, waited on it and
# was closed unused). Uses only the fresh clone and outputs of stepA.sh.
set -u
B=$HOME/work/hive/t_c2fbba86/rr
mkdir -p "$B/win" || exit 74
spark-window open cuda202-repro --lease 20 --gb10 exclusive --quiet exclusive --wait > "$B/win/open.txt" 2>&1
wid=$(spark-window id 2>/dev/null)
if [ -z "$wid" ] || ! spark-window status 2>/dev/null | grep -q "^window $wid  name=cuda202-repro "; then
  echo "NOT_RUN: our window is not the open one (wid=$wid)"; cat "$B/win/open.txt"; exit 77
fi
echo "WINDOW-OPEN $wid $(date -u +%FT%TZ)"
nvidia-smi --query-compute-apps=pid,process_name,used_memory --format=csv > "$B/win/compute-apps.before.txt" 2>&1
cd "$B/CUDApp" || { spark-window close "$wid"; exit 74; }
env -i HOME=$B/home PATH=/usr/bin:/bin TMPDIR=$B/tmp LC_ALL=C MT_GPU_WINDOW=$wid TOOLROOT=$B/tools/root \
  sh tools/repro/replay-nvidia.sh "$B/out" > "$B/win/replay-nvidia.out" 2>&1
echo "replay exit $?" >> "$B/win/replay-nvidia.out"
nvidia-smi --query-compute-apps=pid,process_name,used_memory --format=csv > "$B/win/compute-apps.after.txt" 2>&1
spark-window close "$wid" > "$B/win/close.txt" 2>&1
echo "WINDOW-CLOSE $wid $(date -u +%FT%TZ)"
cat "$B/win/replay-nvidia.out"
cp "$HOME/yardsticks/windows/$wid/open.json" "$B/win/open.json" 2>/dev/null
cp "$HOME/yardsticks/windows/$wid/close.json" "$B/win/close.json" 2>/dev/null
