#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Apple GPU stage of the portable-application reproduction (docs/repro/README.md step 5), macOS on Apple
# silicon. Fetches MoltenVK v1.4.2 (Apache-2.0) by fixed URL and checks its sha256, replays the
# producer's modules with tests/multitarget/apple-gpu.sh (MoltenVK over Metal, a vendor-runtime route)
# and checks the six GPU output files against docs/repro/expected.sha256. Nothing is installed: MoltenVK
# is unpacked under OUT, so removing OUT undoes this step.
#
# usage: replay-apple.sh /abs/OUT    (OUT holds gpu-in/ copied from the producer; results go to OUT/apple)
# Environment: MT_EXPECT_DEVICE (default "Apple M2"; apple-gpu.sh refuses another device name).
# Exit: 0 REPRO APPLE PASS; 1 FAIL; 64 usage; 74 I/O or download; 77 NOT_RUN (apple-gpu.sh NOT_RUN).
set -u
LC_ALL=C
export LC_ALL
[ "$#" -eq 1 ] || { echo 'usage: replay-apple.sh /abs/OUT' >&2; exit 64; }
O=$1
case $O in /*) ;; *) echo 'usage: OUT must be absolute' >&2; exit 64 ;; esac
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
url=https://github.com/KhronosGroup/MoltenVK/releases/download/v1.4.2/MoltenVK-macos.tar
want=f95765a6229cb7b915990a2890ce12ebe36a730b021545d3d52ae69ce4c4024e
M=$O/moltenvk
mkdir -p "$M" "$O/apple" || exit 74
[ -f "$M/MoltenVK-macos.tar" ] || curl -fsSL --retry 2 -o "$M/MoltenVK-macos.tar" "$url" || { echo 'REPRO APPLE FAIL MoltenVK download' >&2; exit 74; }
got=$(shasum -a 256 "$M/MoltenVK-macos.tar" | cut -d ' ' -f 1)
[ "$got" = "$want" ] || { echo "REPRO APPLE FAIL MoltenVK-macos.tar sha256=$got want=$want"; exit 1; }
[ -f "$M/MoltenVK/MoltenVK/dynamic/dylib/macOS/libMoltenVK.dylib" ] || (cd "$M" && tar -xf MoltenVK-macos.tar) || exit 74
D=$O/apple
cp "$O"/gpu-in/* "$D"/ || exit 74
MVK_LIB=$M/MoltenVK/MoltenVK/dynamic/dylib/macOS/libMoltenVK.dylib MVK_INCLUDE=$M/MoltenVK/MoltenVK/include \
  sh "$root/tests/multitarget/apple-gpu.sh" "$D" > "$D/apple-gpu.log" 2>&1
rc=$?
tail -n 1 "$D/apple-gpu.log"
[ "$rc" -eq 77 ] && { echo 'REPRO APPLE NOT_RUN (see apple-gpu.log)'; exit 77; }
[ "$rc" -eq 0 ] || { echo "REPRO APPLE FAIL apple-gpu.sh exit $rc"; exit 1; }
mkdir -p "$D/out" || exit 74
for f in vadd fsum16 gemm16; do
  cp "$D/$f.vk.gpu.out" "$D/out/" && cp "$D/$f.vk.mutant.gpu.out" "$D/out/" || exit 74
done
grep '  out/' "$root/docs/repro/expected.sha256" > "$D/expected-out.sha256" || exit 74
if (cd "$D" && shasum -a 256 -c expected-out.sha256) > "$D/replay.check" 2>&1; then
  echo "REPRO APPLE PASS files=$(grep -c ': OK$' "$D/replay.check") (equal to the published GB10 and Apple M2 outputs)"
else
  cat "$D/replay.check"; echo 'REPRO APPLE FAIL an output differs from docs/repro/expected.sha256'; exit 1
fi
