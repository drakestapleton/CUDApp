#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
set -eu
ulimit -c 0
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
: "${OMEGA_OMC:?set pinned OMEGA_OMC}"
: "${OMEGA_BRIDGE:?set pinned OMEGA_BRIDGE}"
OUT_DIR=${OUT_DIR:-build}
CC=${CC:-cc}
T="$OUT_DIR/dnn-rows-evidence/mutants"
mkdir -p "$T"
for mutant in maximum normalize tail epsilon refusal half; do
    mkdir -p "$T/$mutant/src" "$T/$mutant/gen"
    cp om/dnn-rows/*.om "$T/$mutant/src/"
    python3 - "$T/$mutant/src" "$mutant" <<'PY'
import pathlib,sys
changes={
'maximum':('kernel.om','aa_bin x m 37 9 9 8','aa_bin x m 30 9 9 8'),
'normalize':('kernel.om','aa_bin x m 32 8 8 11','aa_bin x m 32 8 8 15'),
'tail':('kernel.om','aa_load x m 8 4 0 or mode 1536','aa_load x m 8 4 0 mode'),
'epsilon':('kernel.om','aa_bin x m 30 9 9 13','aa_bin x m 30 9 9 21'),
'refusal':('plan.om','ge get s 4 2139095040','gt get s 4 2139095040'),
'half':('kernel.om','aa_load x m 8 4 0 or mode 1536','aa_load x m 8 4 0 or and mode 1 1536')}
f,old,new=changes[sys.argv[2]]
p=pathlib.Path(sys.argv[1])/f
s=p.read_text();assert s.count(old)==1,(f,old,s.count(old));p.write_text(s.replace(old,new))
PY
    ABS=$(CDPATH= cd "$T/$mutant" && pwd)
    OMEGA_UNITS="dnn_rows=om/attn-any/emit.om,$ABS/src/kernel.om dnn_rows_plan=$ABS/src/plan.om" OMEGA_UNITS_DEST="$ABS/gen" sh om/units.sh > "$T/$mutant/generate.log" 2>&1 || { cat "$T/$mutant/generate.log";exit 1; }
    "$CC" -std=gnu11 -O2 -ffp-contract=off -Isrc -Itests tests/gpu_dnn_rows_test.c "$T/$mutant/gen/cudapp_om_dnn_rows.c" "$T/$mutant/gen/cudapp_om_dnn_rows_plan.c" "$OUT_DIR/libomega_gpu.a" -Wl,--wrap=omega_gpu_session_open,--wrap=omega_gpu_session_alloc,--wrap=omega_gpu_session_lock,--wrap=omega_gpu_session_unlock,--wrap=omega_gpu_session_free -lpthread -lm -o "$T/$mutant/test"
    if "$T/$mutant/test" --quick > "$T/$mutant/test.log" 2>&1; then
        echo "dnn-rows mutant SURVIVED: $mutant";cat "$T/$mutant/test.log";exit 1
    else
        rc=$?
        echo "behavior_rc=$rc" > "$T/$mutant/result.txt"
        if test "$mutant" = refusal; then
            test "$rc" = 134 && grep -q 'HOST DEVICE GUARD: session_lock' "$T/$mutant/test.log" || { echo "refusal mutant failed without device guard: rc=$rc";exit 1; }
        else
            test "$rc" = 1 || { echo "mutant crashed rather than behavioral rejection: $mutant rc=$rc";exit 1; }
        fi
    fi
    echo "dnn-rows mutant KILLED: $mutant (generation and C compilation succeeded)"
done
mkdir -p "$T/direct-open"
python3 - "$T/direct-open/test.c" <<'PY'
import pathlib,sys
s=pathlib.Path('tests/gpu_dnn_rows_test.c').read_text()
old='    allow_chip=chip;'
assert s.count(old)==1
pathlib.Path(sys.argv[1]).write_text(s.replace(old,old+'\n    if(!chip) omega_gpu_session_open();'))
PY
"$CC" -std=gnu11 -O2 -ffp-contract=off -Isrc -Itests "$T/direct-open/test.c" "$OUT_DIR/libomega_gpu.a" -Wl,--wrap=omega_gpu_session_open,--wrap=omega_gpu_session_alloc,--wrap=omega_gpu_session_lock,--wrap=omega_gpu_session_unlock,--wrap=omega_gpu_session_free -lpthread -lm -o "$T/direct-open/test"
if "$T/direct-open/test" --sim > "$T/direct-open/test.log" 2>&1; then
    echo 'dnn-rows direct-open guard mutant SURVIVED';exit 1
else
    rc=$?
    echo "behavior_rc=$rc" > "$T/direct-open/result.txt"
    test "$rc" = 134 && grep -q 'HOST DEVICE GUARD: session_open' "$T/direct-open/test.log" || { echo "direct-open mutant failed without guard: rc=$rc";exit 1; }
fi
echo 'dnn-rows direct-open guard mutant: PASS (compiled; killed by abort under --sim)'
echo 'dnn-rows source mutants: PASS (5 behavioral failures, refusal guard abort, direct-open guard abort; all compiled)'
