#!/bin/sh
# DRA-197 unmodified-TensorFold baseline on the GB10. Run ONLY inside an exclusive spark-window lease:
#   spark-window open TensorFoldBaseline --lease 20 --gb10 exclusive --wait   (then run this, then close)
# Usage: baseline.sh OUTDIR
# Env: TF_BIN (tensorfold-native), TF_SRC (source tree with tools/), TF_MODEL (checkpoint dir).
set -eu
OUT=${1:?outdir}
TF_BIN=${TF_BIN:?}; TF_SRC=${TF_SRC:?}; TF_MODEL=${TF_MODEL:?}
PORT=${TF_PORT:-18197}
NAME=tf2b
CPU="taskset -c 0-2"
mkdir -p "$OUT"
[ -z "$(ls -A "$OUT")" ] || { echo "REFUSE: $OUT not empty" >&2; exit 2; }
ss -ltn | grep -q ":$PORT " && { echo "REFUSE: port $PORT busy" >&2; exit 2; }

{
  date -u +%FT%TZ
  echo "bin_sha256 $(sha256sum "$TF_BIN" | cut -d' ' -f1)"
  echo "src_head $(git -C "$TF_SRC" rev-parse HEAD) dirty=$(git -C "$TF_SRC" status --porcelain | wc -l)"
  echo "model_sha256 $(sha256sum "$TF_MODEL/model.safetensors" | cut -d' ' -f1)"
  nvidia-smi --query-gpu=name,driver_version,clocks.sm,clocks.max.sm,temperature.gpu,power.draw --format=csv
  /usr/local/cuda/bin/nvcc --version | tail -1
  free -m | head -2
  uname -r
} > "$OUT/env.txt" 2>&1

serve() { # $1 label, rest: extra serve flags
  lbl=$1; shift
  t0=$(date +%s.%N)
  $CPU "$TF_BIN" serve "$TF_MODEL" --host 127.0.0.1 --port "$PORT" --name "$NAME" \
    --context 8192 --no-thinking --no-update-check "$@" > "$OUT/server-$lbl.log" 2>&1 &
  SPID=$!
  i=0
  until curl -sf "http://127.0.0.1:$PORT/v1/models" >/dev/null 2>&1; do
    kill -0 $SPID 2>/dev/null || { echo "server $lbl died" >&2; tail -30 "$OUT/server-$lbl.log" >&2; exit 3; }
    i=$((i+1)); [ $i -lt 600 ] || { echo "server $lbl not ready in 300s" >&2; kill $SPID; exit 3; }
    sleep 0.5
  done
  t1=$(date +%s.%N)
  echo "$lbl ready_s $(python3 -c "print(round($t1-$t0,3))")" >> "$OUT/load.txt"
}
stop() { kill "$SPID" 2>/dev/null || true; wait "$SPID" 2>/dev/null || true; }
trap stop EXIT
B="http://127.0.0.1:$PORT"

# 1. Default server: single stream, then concurrency with token-equality checks.
serve default
$CPU python3 "$TF_SRC/tools/bench_openai.py" "$B" "$NAME" --tokens 256 --reps 5 --temperatures 0 \
  --label default --output "$OUT/openai-default.json" > "$OUT/openai-default.log" 2>&1
$CPU python3 "$TF_SRC/tools/bench_concurrent.py" "$B" "$NAME" --levels 1,4,8 --tokens 256 --reps 3 \
  --temperatures 0 --alone --serial --label default --output "$OUT/concurrent-default.json" \
  > "$OUT/concurrent-default.log" 2>&1
stop

# 2. Kernel-time profile of the same single-stream workload (CUDA graphs traced per node).
NSYS=/usr/local/cuda/bin/nsys
t0=$(date +%s)
$CPU $NSYS profile --trace=cuda --cuda-graph-trace=node --sample=none --cpuctxsw=none \
  -o "$OUT/prof" "$TF_BIN" serve "$TF_MODEL" --host 127.0.0.1 --port "$PORT" --name "$NAME" \
  --context 8192 --no-thinking --no-update-check > "$OUT/server-prof.log" 2>&1 &
SPID=$!
until curl -sf "$B/v1/models" >/dev/null 2>&1; do
  kill -0 $SPID 2>/dev/null || { echo "nsys server died" >&2; tail -30 "$OUT/server-prof.log" >&2; exit 3; }
  [ $(( $(date +%s) - t0 )) -lt 300 ] || { echo "nsys server not ready" >&2; exit 3; }
  sleep 0.5
done
$CPU python3 "$TF_SRC/tools/bench_openai.py" "$B" "$NAME" --tokens 256 --reps 3 --temperatures 0 \
  --label prof --output "$OUT/openai-prof.json" > "$OUT/openai-prof.log" 2>&1
kill -INT $SPID; wait $SPID 2>/dev/null || true
$NSYS stats --report cuda_gpu_kern_sum --format csv -o "$OUT/kern" "$OUT/prof.nsys-rep" > "$OUT/nsys-stats.log" 2>&1 || true
date -u +%FT%TZ >> "$OUT/env.txt"
echo DONE
