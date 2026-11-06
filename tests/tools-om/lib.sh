# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Shared helpers for tests/tools-om/*.sh. Source after setting ROOT and tmp; the caller runs
# under bwrap/taskset (see tests/tools-om/README.md). Every helper fails the script on a broken
# expectation; nothing is skipped silently.
OMC=${OMEGA_OMC:-$HOME/work/cudastack/omega-mv/seed/out/omc}
BRIDGE=${OMEGA_BRIDGE:-$HOME/work/cudastack/omega-bridge}
OMC_PIN=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
CC=${CC:-cc}
TOM_CFLAGS="-std=gnu11 -Wall -Wextra -Werror -D_GNU_SOURCE -O2 -ffp-contract=off -Isrc"

tom_pin() {
  [ "$(sha256sum < "$OMC" | cut -c1-64)" = "$OMC_PIN" ] || { echo "NOT_RUN: omc is not the pinned e5b32567 build"; exit 3; }
}

# tom_mutant NAME SRC SITE REPLACEMENT: copy SRC with the one line SITE replaced, compile it through the
# pinned bridge to $tmp/NAME.c. SITE must occur exactly once, as a whole line.
tom_mutant() {
  name=$1 src=$2 site=$3 repl=$4
  n=$(grep -c -x -F -- "$site" "$src" || true)
  [ "$n" = 1 ] || { echo "FAIL mutant $name: site found $n times"; exit 1; }
  awk -v s="$site" -v r="$repl" '$0 == s { print r; next } { print }' "$src" > "$tmp/$name.om"
  OMEGA_SEEDED_OMC="$OMC" sh "$BRIDGE/tools/bridge-seeded" c "$tmp/$name.om" > "$tmp/$name.c"
}

# tom_flip ORACLE CANDIDATE: CANDIDATE matched ORACLE byte for byte. Write CANDIDATE.flip with the last
# decimal digit of the last record changed and require the same cmp to reject it against ORACLE.
tom_flip() {
  cmp -s "$1" "$2" || { echo "FAIL negative control: $2 did not match $1 before the flip"; exit 1; }
  python3 - "$2" "$2.flip" <<'PY'
import sys
b = bytearray(open(sys.argv[1], "rb").read())
i = max(i for i, c in enumerate(b) if 48 <= c <= 57)
b[i] = 48 + (b[i] - 48 + 1) % 10
open(sys.argv[2], "wb").write(b)
PY
  if cmp -s "$1" "$2.flip"; then echo "FAIL negative control: flipped record not caught"; exit 1; fi
  echo "negative control: flipped record caught ($(basename "$2"))"
}
