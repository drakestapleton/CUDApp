#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# NVIDIA GPU stage of the portable-application reproduction (docs/repro/README.md step 4). Replays the
# producer's modules with tests/multitarget/nvidia-gpu.sh (NVIDIA's own Vulkan driver, a vendor-runtime
# route, not CUDA++) and checks the six GPU output files against docs/repro/expected.sha256.
# Tested only on a DGX Spark (GB10, Linux AArch64, NVIDIA driver 580.173.02); nvidia-gpu.sh refuses any
# other host with NOT_RUN (docs/repro/README.md "Known limits").
#
# usage: replay-nvidia.sh /abs/OUT        (OUT = produce.sh's OUT; results go to OUT/nvidia)
# Environment:
#   MT_GPU_WINDOW  required: an id for the exclusive GPU session this runs in (on the Spark the open
#                  spark-window id; elsewhere any label you record, after making sure nothing else uses the GPU)
#   TOOLROOT       DIR/root from fetch-tools.sh (vulkan headers)
#   HELLO_GPU, HELLO_GPU_CONTROL  optional CUDA++ native baseline (build/hello_gpu); unset: NOT_RUN rows
# Exit: 0 REPRO NVIDIA PASS; 1 FAIL; 64 usage; 74 I/O; 77 NOT_RUN (nvidia-gpu.sh NOT_RUN).
set -u
LC_ALL=C
export LC_ALL
[ "$#" -eq 1 ] || { echo 'usage: replay-nvidia.sh /abs/OUT' >&2; exit 64; }
O=$1
case $O in /*) ;; *) echo 'usage: OUT must be absolute' >&2; exit 64 ;; esac
[ -n "${TOOLROOT:-}" ] || { echo 'usage: set TOOLROOT to fetch-tools.sh DIR/root' >&2; exit 64; }
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
D=$O/nvidia
mkdir -p "$D" || exit 74
cp "$O"/gpu-in/* "$D"/ || exit 74
CONFORM=$O/run/conform MT_CFLAGS="-I$TOOLROOT/usr/include" sh "$root/tests/multitarget/nvidia-gpu.sh" "$D" > "$D/nvidia-gpu.log" 2>&1
rc=$?
tail -n 1 "$D/nvidia-gpu.log"
[ "$rc" -eq 77 ] && { echo 'REPRO NVIDIA NOT_RUN (see nvidia-gpu.log)'; exit 77; }
[ "$rc" -eq 0 ] || { echo "REPRO NVIDIA FAIL nvidia-gpu.sh exit $rc"; exit 1; }
mkdir -p "$D/out" || exit 74
for f in vadd fsum16 gemm16; do
  cp "$D/$f.vk.gpu.out" "$D/out/" && cp "$D/$f.vk.mutant.gpu.out" "$D/out/" || exit 74
done
grep '  out/' "$root/docs/repro/expected.sha256" > "$D/expected-out.sha256" || exit 74
if (cd "$D" && sha256sum -c expected-out.sha256) > "$D/replay.check" 2>&1; then
  echo "REPRO NVIDIA PASS files=$(grep -c ': OK$' "$D/replay.check") (equal to the published GB10 and Apple M2 outputs)"
else
  cat "$D/replay.check"; echo 'REPRO NVIDIA FAIL an output differs from docs/repro/expected.sha256'; exit 1
fi
