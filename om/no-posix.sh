#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Existing portable roots, explicitly composed as OM2 for caller-stack native ABI.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
: "${OMEGA_BRIDGE:?set OMEGA_BRIDGE to omega-bridge 741c1ebb}"
: "${OMEGA_OMC:?set OMEGA_OMC to the pinned seeded compiler}"
[ "$(sha256sum < "$OMEGA_OMC" | cut -c1-64)" = 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f ]
[ "$(sha256sum < "$OMEGA_BRIDGE/tools/native" | cut -c1-64)" = 14b2b51ed062ebaa7ea99e83752cc075452465b9ddba4939eb171692358ecb6b ] || { echo 'NOT_RUN: native frontend pin mismatch'; exit 3; }
[ "$(sha256sum < "$OMEGA_BRIDGE/src/native.om" | cut -c1-64)" = e2077722aaeffb1121b70160e22357ea47f369e0039ab1ea06dc700f6f4e45d7 ] || { echo 'NOT_RUN: native generator pin mismatch'; exit 3; }
OUT=${1:-$ROOT/build/no_posix}
mkdir -p "$OUT/sources"
for pair in encoder/bw_enc_sm121 cubin/cubin_elf numeric/no_bf16 apiviews/no_avbf; do
  name=${pair##*/}
  { printf 'om2\n'; cat "$ROOT/om/$pair.om"; } > "$OUT/sources/$name.om"
done
OMEGA_NATIVE_OMC2="$OMEGA_OMC" sh "$OMEGA_BRIDGE/tools/native" --freestanding "$OUT/native" \
  "$OUT/sources/bw_enc_sm121.om" "$OUT/sources/cubin_elf.om" "$OUT/sources/no_bf16.om" "$OUT/sources/no_avbf.om"
