#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Recompile semantic mutants of om/engine/eng.om. Each mutant breaks one decision,
# is regenerated through the pinned seeded bridge and linked in place of
# src/cudapp_om_eng.c into two oracles:
#   diff  the om-vs-C engine differential (tests/om_eng_diff_test.c); its record
#         must differ from the C oracle transcript build/om-qmd/eng_c.txt
#   be    the Blackwell backend host test (tests/test_omega_blackwell_engine.c
#         against tests/fake_m16_native.c); it must print [FAIL]
# A mutant is KILLED when at least one oracle catches it; SURVIVED fails the sweep.
# The BE_* and WRAP_* rows are the backend-mutation rows of
# tools/gpu_engine_backend_mutations.sh whose decisions moved into eng.om.
# Host only, no device. Needs OMEGA_OMC (pinned e5b32567), OMEGA_BRIDGE and
# build/om-qmd/eng_c.txt (make test-om-eng-diff). Run under the bwrap wrapper.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
OMC=${OMEGA_OMC:-$HOME/work/cudastack/omega-mv/seed/out/omc}
BRIDGE=${OMEGA_BRIDGE:-$HOME/work/cudastack/omega-bridge}
SRC=$ROOT/om/engine/eng.om
OUT=$ROOT/build/om-qmd/eng-mutants
REF=$ROOT/build/om-qmd/eng_c.txt
[ -f "$REF" ] || { echo "FAIL om eng mutants: $REF missing (run make test-om-eng-diff)"; exit 2; }
ENG=${OM_ENG_BUILD_LINE:?OM_ENG_BUILD_LINE (set by make test-om-eng-mutants)}
TP=$ROOT/third_party/nvidia-open-580.173.02
INC="-I$ROOT/m16 -I$ROOT/nvrm -I$TP/src/common/sdk/nvidia/inc -I$TP/kernel-open/common/inc -I$TP/kernel-open/nvidia-uvm -I$TP/src/nvidia/arch/nvalloc/unix/include"
BESRCS="src/omega_blackwell_submit.c src/omega_blackwell_engine.c src/omega_gpu_engine.c src/omega_blackwell_matmul.c src/omega_blackwell_codegen.c src/omega_blackwell_encoder.c src/omega_blackwell_qmd.c src/omega_blackwell_realize.c src/omega_vector.c src/omega_core.c src/omega_canonical.c src/sha256.c"
BESRCS="$BESRCS ${OM_ENG_BE_BINDING:?OM_ENG_BE_BINDING (set by make test-om-eng-mutants)} src/omega_gpu_om_ir.c"
rm -rf "$OUT"
mkdir -p "$OUT"
killed=0
total=0
run_one() {
  name=$1
  from=$2
  to=$3
  total=$((total + 1))
  m=$OUT/$name
  mkdir -p "$m"
  # The from-string (it may span lines) must occur exactly once.
  n=$(python3 -c 'import sys; print(open(sys.argv[1]).read().count(sys.argv[2]))' "$SRC" "$from")
  if [ "$n" != 1 ]; then
    echo "FAIL $name site count $n"
    exit 1
  fi
  python3 -c 'import sys; p,a,b=sys.argv[1:]; t=open(p).read(); open(p+".mut","w").write(t.replace(a,b,1))' "$SRC" "$from" "$to"
  mv "$SRC.mut" "$m/eng.om"
  OMEGA_SEEDED_OMC="$OMC" sh "$BRIDGE/tools/bridge-seeded" c "$m/eng.om" > "$m/eng.c"
  (cd "$ROOT" && $ENG "$m/eng.c" -o "$m/eng_om")
  (cd "$ROOT" && gcc -std=gnu11 -O2 -Wall -Wextra -Werror -D_GNU_SOURCE -DOMEGA_BLACKWELL_HOST_SIMULATOR -pthread \
    -ffunction-sections -fdata-sections -Isrc -Itests $INC -o "$m/be" tests/test_omega_blackwell_engine.c \
    tests/fake_m16_native.c $BESRCS "$m/eng.c" -Wl,--gc-sections -lm)
  set +e
  "$m/eng_om" "$m/eng_om.txt" 2>"$m/eng_om.ops"
  "$m/eng_om" --compare "$m/eng_om.txt" "$REF" >"$m/diff.out" 2>&1
  drc=$?
  "$m/be" >"$m/be.out" 2>&1
  if grep -q '\[FAIL\]' "$m/be.out"; then brc=1; else brc=0; fi
  set -e
  if [ "$drc" = 0 ] && [ "$brc" = 0 ]; then
    echo "SURVIVED $name"
    exit 1
  fi
  echo "KILLED $name diff=$drc be=$brc ($(head -n 1 "$m/diff.out"))"
  killed=$((killed + 1))
}
# wait and gate decisions
run_one m_seqge_bit 'h = and shr d 31 1' 'h = and shr d 30 1'
run_one m_wrest_stale_stall 'if ge now get w 2' 'if ge now get m 12'
run_one m_mmshape_no_u32 'gx = and div c 8 4294967295' 'gx = div c 8'
run_one m_cache_page 'put w 1 and add get m 1 4095 18446744073709547520' 'put w 1 and add get m 1 4096 18446744073709547520'
run_one m_life_floor 'bv gt b 274877906944' 'bv ge b 274877906944'
run_one m_reqf_sign 'z = and bv ne a 0 bv eq shr a 63 0' 'z = bv ne a 0'
# relocated tools/gpu_engine_backend_mutations.sh rows
run_one BE_ROUND_PAGES_ZERO_ACCEPTED 'if eq k 0
if eq a 0
put w 0 1' 'if eq k 0
if eq a 1
put w 0 1'
run_one BE_ROUND_PAGES_NO_ROUNDING 'v = and add a 4095 18446744073709547520' 'v = a'
run_one BE_BUILD_COUNT_OVERFLOW 'if gt a 4294967232' 'if gt a 4294967295'
run_one BE_THREADS_WRONG 'put w 2 64' 'put w 2 32'
run_one BE_GRID_ROUNDS_DOWN 'put w 1 div add a 63 64' 'put w 1 div a 64'
run_one BE_QMD_RELEASE_CHECK 'put w 0 and z bv eq get m 6 2172715008' 'put w 0 1'
run_one WRAP_MARKER_TIMEOUT 'put w 0 5000' 'put w 0 6000'
run_one WRAP_SEMAPHORE_TIMEOUT 'put w 1 5000' 'put w 1 6000'
run_one WRAP_MARKER2_TIMEOUT 'put w 2 5000' 'put w 2 6000'
run_one WRAP_VISIBILITY_TIMEOUT 'put w 3 5000' 'put w 3 6000'
echo "PASS om eng mutants: $killed/$total killed"
