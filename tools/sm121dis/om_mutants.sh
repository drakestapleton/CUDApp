# ORACLE-free mutation harness (pinned omc + omega-bridge regenerate each mutant; no NVIDIA tool runs here).
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# om_mutants.sh <out-dir> <kernel-dir>: three one-line mutants of om/disasm/sm121_dis.om (CUDA++ C6, DRA-475). Each is
# regenerated through om/units.sh by the pinned compiler and bridge into its own binding, linked into its own sm121dis,
# and run on every kernel of <kernel-dir>; its text must differ from the unmutated decoder's on at least one kernel.
# Needs OMEGA_OMC and OMEGA_BRIDGE (om/units.sh refuses other pins with exit 3, reported NOT_RUN).
set -u
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT=$1; K=$2
CC=${CC:-gcc}
SRC=$ROOT/om/disasm/sm121_dis.om
# 1: the first source register field read one bit too high; 2: PT printed as QT; 3: the MOV rows' fixed hi bits
set -- 's/^ret pk 0 24 8$/ret pk 0 25 8/' 's/^lit o s 21584$/lit o s 21585/' 's/^put t 1 3840$/put t 1 3841/'
mkdir -p "$OUT"
D0=$OUT/om-mut0
mkdir -p "$D0"
"$CC" -std=gnu11 -Wall -Wextra -Werror -O2 -I"$ROOT/src" -o "$D0/sm121dis" "$ROOT/tools/sm121dis/sm121dis.c" \
    "$ROOT/src/omega_sm121_disasm.c" "$ROOT/src/cudapp_om_sm121_dis.c" || exit 1
for f in "$K"/*.bin; do "$D0/sm121dis" --pairs "$f"; done > "$D0/all.txt" 2>&1
n=0
for m in "$@"; do
    n=$((n + 1)); d=$OUT/om-mut$n
    mkdir -p "$d"
    sed "$m" "$SRC" > "$d/sm121_dis.om"
    if cmp -s "$d/sm121_dis.om" "$SRC"; then echo "MUTANT $n BROKEN: '$m' changed nothing"; exit 1; fi
    rc=0
    OMEGA_UNITS=$d/sm121_dis.om OMEGA_UNITS_DEST=$d sh "$ROOT/om/units.sh" > "$d/units.log" 2>&1 || rc=$?
    if [ "$rc" = 3 ]; then cat "$d/units.log"; echo "VERDICT NOT_RUN: pinned omc and bridge needed"; exit 3; fi
    [ "$rc" = 0 ] || { cat "$d/units.log"; echo "MUTANT $n: regeneration failed ($rc)"; exit 1; }
    # -I$d first: the mutant's own generated cudapp_native.h declares its omega_sm121_dis
    "$CC" -std=gnu11 -Wall -Wextra -Werror -O2 -I"$d" -I"$ROOT/src" -o "$d/sm121dis" "$ROOT/tools/sm121dis/sm121dis.c" \
        "$ROOT/src/omega_sm121_disasm.c" "$d/cudapp_om_sm121_dis.c" || exit 1
    for f in "$K"/*.bin; do "$d/sm121dis" --pairs "$f"; done > "$d/all.txt" 2>&1
    if cmp -s "$D0/all.txt" "$d/all.txt"; then echo "MUTANT $n SURVIVED: '$m'"; exit 1; fi
    echo "mutant $n red ('$m'): $(diff "$D0/all.txt" "$d/all.txt" | grep -c '^<') corpus words print differently"
done
echo "VERDICT PASS: every .om mutant changes the corpus text"
