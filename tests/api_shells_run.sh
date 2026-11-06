#!/bin/bash
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Focused host differential for the kernel-family API shells. No GPU.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${OUT_DIR:-$ROOT/build/api-shells}"
GEN="${API_SHELLS_GEN:-$ROOT/src}"
mkdir -p "$OUT"
if [[ "${1:-}" == "regen" ]]; then
  export OMEGA_OMC="${OMEGA_OMC:-$HOME/work/cudastack/omega-mv/seed/out/omc}"
  export OMEGA_BRIDGE="${OMEGA_BRIDGE:-$HOME/work/cudastack/omega-bridge}"
  export OMEGA_UNITS="om/api/apish_shp.om om/api/apish_pack.om om/api/apish_slot.om om/api/apish_blk.om om/api/apish_trit.om"
  export OMEGA_UNITS_DEST="$OUT/gen"
  mkdir -p "$OMEGA_UNITS_DEST"
  bash om/units.sh
  GEN="$OMEGA_UNITS_DEST"
fi
cc -std=c11 -O2 -ffp-contract=off -I"$ROOT/src" -c "$ROOT/tests/api_shells_oracle.c" -o "$OUT/oracle.o"
cc -std=c11 -O2 -ffp-contract=off -I"$ROOT/src" -c "$ROOT/tests/api_shells_diff.c" -o "$OUT/diff.o"
cc -std=c11 -O2 -ffp-contract=off -I"$ROOT/src" -c "$ROOT/src/omega_fp8_slot_table.c" -o "$OUT/slot_table.o"
cc -std=c11 -O2 -ffp-contract=off -I"$ROOT/src" -c "$ROOT/src/omega_gpu_fp8_pack.c" -o "$OUT/gpu_pack.o"
for name in shp pack slot blk trit; do
  cc -std=c11 -O2 -ffp-contract=off -I"$ROOT/src" -c "$GEN/cudapp_om_apish_$name.c" -o "$OUT/unit_$name.o"
done
for name in nan scale fold foldh acc mag slot idle pad; do
  cc -std=c11 -O2 -ffp-contract=off -I"$ROOT/src" -c "$ROOT/src/cudapp_om_ar_$name.c" -o "$OUT/ar_$name.o"
done
cc -o "$OUT/api_shells_diff" \
  "$OUT/diff.o" "$OUT/oracle.o" "$OUT/slot_table.o" "$OUT/gpu_pack.o" \
  "$OUT/unit_shp.o" "$OUT/unit_pack.o" "$OUT/unit_slot.o" "$OUT/unit_blk.o" "$OUT/unit_trit.o" \
  "$OUT/ar_nan.o" "$OUT/ar_scale.o" "$OUT/ar_fold.o" "$OUT/ar_foldh.o" "$OUT/ar_acc.o" "$OUT/ar_mag.o" \
  "$OUT/ar_slot.o" "$OUT/ar_idle.o" "$OUT/ar_pad.o" \
  -lpthread -lm
"$OUT/api_shells_diff"
