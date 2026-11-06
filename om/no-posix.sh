#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Existing portable roots, explicitly composed as OM2 for caller-stack native ABI.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
: "${OMEGA_BRIDGE:?set OMEGA_BRIDGE to omega-bridge 6a8f0090}"
: "${OMEGA_OMC:?set OMEGA_OMC to the pinned seeded compiler}"
[ "$(sha256sum < "$OMEGA_OMC" | cut -c1-64)" = 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f ]
OUT=${1:-$ROOT/build/no_posix}
mkdir -p "$OUT/sources"
for pair in encoder/bw_enc_sm121 cubin/cubin_elf numeric/no_bf16; do
  name=${pair##*/}
  { printf 'om2\n'; cat "$ROOT/om/$pair.om"; } > "$OUT/sources/$name.om"
done
OMEGA_NATIVE_OMC2="$OMEGA_OMC" sh "$OMEGA_BRIDGE/tools/native" --freestanding "$OUT/native" \
  "$OUT/sources/bw_enc_sm121.om" "$OUT/sources/cubin_elf.om" "$OUT/sources/no_bf16.om"
