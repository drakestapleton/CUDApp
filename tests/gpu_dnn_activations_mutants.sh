#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Each mutated source must pass seeded compilation AND checked-C generation.
# Exit 1 (wrong behavior) or the expected refusal guard abort counts as a kill.
set -eu
ulimit -c 0
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
: "${OMEGA_OMC:?set pinned compiler}"
: "${OMEGA_BRIDGE:?set pinned bridge}"
OUT_DIR=${OUT_DIR:-build}
CC=${CC:-cc}
mkdir -p "$OUT_DIR"
T=$(mktemp -d "$OUT_DIR/dnn-activations-mutants.XXXXXX")
"$CC" -std=gnu11 -O2 -ffp-contract=off -DDA_GUARD_MUTANT -Isrc -Itests -Wl,--wrap=omega_gpu_session_open -Wl,--wrap=omega_gpu_session_lock -Wl,--wrap=omega_gpu_session_unlock -Wl,--wrap=omega_gpu_session_free -Wl,--wrap=omega_gpu_session_alloc tests/gpu_dnn_activations_test.c "$OUT_DIR/libomega_gpu.a" -lpthread -lm -o "$T/guard"
rc=0
"$T/guard" --sim > "$T/guard.log" 2>&1 || rc=$?
if [ "$rc" != 134 ]; then cat "$T/guard.log";echo "guard mutant survived rc=$rc";exit 1;fi
case $(cat "$T/guard.log") in *"DNN-HOST-GUARD session_open"*) ;; *) cat "$T/guard.log";exit 1;; esac
echo 'dnn-activations isolation control: PASS (compiled direct-open mutant aborts under --sim)'
for mutant in relu erf tanh silu tail dtype refusal; do
    mkdir -p "$T/$mutant/src" "$T/$mutant/gen"
    cp om/dnn-activations/*.om "$T/$mutant/src/"
    python3 - "$T/$mutant/src" "$mutant" <<'PY'
import pathlib,sys
changes={
'relu':('kernel.om','aa_mov x m 12 0','da_copy x m 12 8'),
'erf':('kernel.om','0.3275911f32','0.4275911f32'),
 'tanh':('kernel.om','0.044715f32','0.44715f32'),
 'silu':('kernel.om','da_mulc x m 11 9 -1.0f32','da_mulc x m 11 9 1.0f32'),
 'tail':('kernel.om','aa_mad x m 7 1 32 0','aa_mad x m 7 1 31 0'),
 'dtype':('kernel.om','aa_load x m 8 4 0 dtype','aa_load x m 8 4 0 0'),
 'refusal':('plan.om','if or eq n 0 gt n 4294967295','if or eq n 999 gt n 4294967295'),
}
f,old,new=changes[sys.argv[2]]
p=pathlib.Path(sys.argv[1])/f
s=p.read_text();assert s.count(old)==1,(f,old,s.count(old))
p.write_text(s.replace(old,new))
PY
    OMEGA_UNITS="dnn_activations=om/attn-any/emit.om,$T/$mutant/src/kernel.om dnn_activations_plan=$T/$mutant/src/plan.om" OMEGA_UNITS_DEST="$T/$mutant/gen" sh om/units.sh > "$T/$mutant/generate.log" 2>&1 || { cat "$T/$mutant/generate.log"; exit 1; }
    "$CC" -std=gnu11 -O2 -ffp-contract=off -Isrc -Itests -Wl,--wrap=omega_gpu_session_open -Wl,--wrap=omega_gpu_session_lock -Wl,--wrap=omega_gpu_session_unlock -Wl,--wrap=omega_gpu_session_free -Wl,--wrap=omega_gpu_session_alloc tests/gpu_dnn_activations_test.c "$T/$mutant/gen/cudapp_om_dnn_activations.c" "$T/$mutant/gen/cudapp_om_dnn_activations_plan.c" "$OUT_DIR/libomega_gpu.a" -lpthread -lm -o "$T/$mutant/test"
    rc=0
    "$T/$mutant/test" --quick > "$T/$mutant/test.log" 2>&1 || rc=$?
    if [ "$mutant:$rc" = refusal:134 ]; then
        case $(cat "$T/$mutant/test.log") in *"DNN-HOST-GUARD session_lock"*) ;; *) cat "$T/$mutant/test.log";exit 1;; esac
    elif [ "$rc" != 1 ]; then
        echo "dnn-activations mutant INVALID/SURVIVED: $mutant rc=$rc"
        cat "$T/$mutant/test.log";exit 1
    fi
    echo "dnn-activations mutant KILLED after seeded compile: $mutant"
done
echo "dnn-activations mutation evidence: $T"
echo 'dnn-activations source mutants: PASS (7 compiled, 7 behaviorally killed)'
