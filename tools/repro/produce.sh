#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Producer stage of the portable-application reproduction (docs/repro/README.md step 3), Linux AArch64,
# CPU only. Runs the host gate tests/multitarget/run.sh with the launcher enabled on Mesa lavapipe (a CPU
# Vulkan driver), keeps its outputs, copies the 14 GPU replay inputs to OUT/gpu-in with a SHA256SUMS
# file, and checks them against the published digests in docs/repro/expected.sha256. It opens no GPU.
#
# usage: produce.sh /abs/pinned-omc /abs/TOOLROOT /abs/OUT
#   TOOLROOT  DIR/root from tools/repro/fetch-tools.sh
# Environment: LVP_ICD (lavapipe manifest, default /usr/share/vulkan/icd.d/lvp_icd.json), HELLO_GPU
#   (optional CUDA++ build/hello_gpu for the --sim digest cross-check; unset: that row NOT_RUN).
# Exit: 0 REPRO PRODUCE PASS; 1 FAIL (run.sh failed or a digest differs); 64 usage; 74 I/O;
# 77 NOT_RUN (run.sh's own NOT_RUN: not Linux AArch64).
set -u
LC_ALL=C
export LC_ALL
[ "$#" -eq 3 ] || { echo 'usage: produce.sh /abs/pinned-omc /abs/TOOLROOT /abs/OUT' >&2; exit 64; }
omc=$1; R=$2; O=$3
for p in "$omc" "$R" "$O"; do case $p in /*) ;; *) echo 'usage: paths must be absolute' >&2; exit 64 ;; esac; done
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
mkdir -p "$O/gpu-in" || exit 74
{
  echo "HOST $(uname -srm)"
  echo "CUDAPP $(git -C "$root" rev-parse HEAD 2>/dev/null) dirty=$(git -C "$root" status --porcelain 2>/dev/null | wc -l | tr -d ' ')"
  echo "DATE $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "CC $(${MT_CC:-cc} --version 2>/dev/null | head -n 1)"
  echo "OMC $(sha256sum < "$omc" | cut -d ' ' -f 1)"
} > "$O/identity.txt"
SPIRV_VAL=$R/usr/bin/spirv-val LD_LIBRARY_PATH=$R/usr/lib/aarch64-linux-gnu CUDAPP_MIGRATION_PENDING_SPIRV_HOST=1 \
  MT_CFLAGS="-I$R/usr/include" LVP_ICD=${LVP_ICD:-/usr/share/vulkan/icd.d/lvp_icd.json} MT_OUT="$O/run" \
  sh "$root/tests/multitarget/run.sh" "$omc" > "$O/run.log" 2>&1
rc=$?
tail -n 1 "$O/run.log"
[ "$rc" -eq 77 ] && exit 77
[ "$rc" -eq 0 ] || { echo "REPRO PRODUCE FAIL run.sh exit $rc (see $O/run.log)"; exit 1; }
G="vadd.vk.spv vadd.vk.mutant.spv fsum16.vk.spv fsum16.vk.mutant.spv gemm16.vk.spv gemm16.vk.mutant.spv a.bin b.bin x.bin ga.bin gb.bin vadd.ref fsum16.ref gemm16.ref"
for f in $G; do cp "$O/run/$f" "$O/gpu-in/$f" || exit 74; done
(cd "$O/gpu-in" && sha256sum $G > SHA256SUMS) || exit 74
grep '  gpu-in/' "$root/docs/repro/expected.sha256" > "$O/expected-gpu-in.sha256" || exit 74
if (cd "$O" && sha256sum -c expected-gpu-in.sha256) > "$O/produce.check" 2>&1; then
  echo "REPRO PRODUCE PASS files=$(grep -c ': OK$' "$O/produce.check") (equal to the published digests)"
else
  cat "$O/produce.check"; echo 'REPRO PRODUCE FAIL a producer output differs from docs/repro/expected.sha256'; exit 1
fi
