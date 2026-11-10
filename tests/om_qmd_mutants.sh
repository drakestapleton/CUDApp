#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Recompile seven semantic mutants of om/qmd/bw_qmd.om. Each must fail the sweep.
# Host only. Needs OMEGA_OMC (pinned e5b32567) and OMEGA_BRIDGE.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
OMC=${OMEGA_OMC:-$HOME/work/cudastack/omega-mv/seed/out/omc}
BRIDGE=${OMEGA_BRIDGE:-$HOME/work/cudastack/omega-bridge}
SRC=$ROOT/om/qmd/bw_qmd.om
OUT=$ROOT/build/om-qmd/mutants
mkdir -p "$OUT"
killed=0
run_one() {
  name=$1
  from=$2
  to=$3
  cp "$SRC" "$OUT/$name.om"
  # The from-string must occur once. grep -c counts lines.
  n=$(grep -c -F "$from" "$OUT/$name.om")
  if [ "$n" != 1 ]; then
    echo "FAIL $name site count $n"
    exit 1
  fi
  # Replace the one site. python keeps the rest byte-stable.
  python3 - "$OUT/$name.om" "$from" "$to" <<'PY'
import pathlib, sys
path, old, new = sys.argv[1:]
text = pathlib.Path(path).read_text()
pathlib.Path(path).write_text(text.replace(old, new, 1))
PY
  OMEGA_SEEDED_OMC="$OMC" sh "$BRIDGE/tools/bridge-seeded" c "$OUT/$name.om" > "$OUT/$name.c"
  gcc -std=gnu11 -O2 -Wall -Wextra -Werror -I"$ROOT/src" -o "$OUT/$name" \
    "$ROOT/tests/om_qmd_sweep_test.c" "$ROOT/src/omega_blackwell_qmd.c" "$OUT/$name.c"
  set +e
  # The caller runs this script under the bwrap/taskset wrapper; a nested bwrap
  # can fail to create namespaces, and that failure would read as a kill.
  "$OUT/$name" >"$OUT/$name.out" 2>"$OUT/$name.err"
  rc=$?
  set -e
  if [ "$rc" = 0 ]; then
    echo "SURVIVED $name"
    exit 1
  fi
  echo "KILLED $name rc=$rc"
  killed=$((killed + 1))
}
run_one m_qmd0_w4 'put q 4 8388610' 'put q 4 8388611'
run_one m_qmd1_w4 'put q 4 20905984' 'put q 4 20905985'
run_one m_qmd1_w14 'put q 14 793772964' 'put q 14 793772965'
run_one m_qmd1_offset 'put q 14 793772964' 'put q 13 793772964'
run_one m_cbank_w0 'put q 0 641859584' 'put q 0 641859585'
run_one m_cbank_w223 'put q 223 16776640' 'put q 223 16776641'
run_one m_shared_cap 'if gt sm 32768' 'if gt sm 16384'
echo "PASS $killed mutants killed"
