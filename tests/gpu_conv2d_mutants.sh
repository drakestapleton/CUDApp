#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Compile mutated temporary sources before requiring a behavioral failure.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
: "${OMEGA_OMC:?set pinned OMEGA_OMC}"
: "${OMEGA_BRIDGE:?set pinned OMEGA_BRIDGE}"
OUT_DIR=${OUT_DIR:-build}
CC=${CC:-cc}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
for mutant in product stride dilation mask filter; do
 mkdir -p "$T/$mutant/src" "$T/$mutant/gen"
 cp om/gemm-any/gemm_ir.om "$T/$mutant/src/gemm.om"
 cp om/conv2d/conv.om om/conv2d/plan.om "$T/$mutant/src/"
 python3 - "$T/$mutant/src" "$mutant" <<'PY'
import pathlib,sys
root=pathlib.Path(sys.argv[1])
changes={
 'product':('gemm.om','op = ulong 33','op = ulong 30'),
 'stride':('conv.om','get r 26 get r 32 get s 8 GN','get r 26 get r 32 1 GN'),
 'dilation':('conv.om','get r 26 get r 9 get s 14 get r 26','get r 26 get r 9 1 get r 26'),
 'mask':('conv.om','gm ins m get r 22 get r 34 GM GN','gb ins m 2 get r 22 GN GM'),
 'filter':('conv.om','get r 29 get r 27 get s 6 get r 28','get r 29 get r 28 get s 5 get r 27'),
}
f,old,new=changes[sys.argv[2]]
p=root/f;text=p.read_text();assert text.count(old)==1,(old,text.count(old));p.write_text(text.replace(old,new))
PY
 OMEGA_UNITS="conv2d=$T/$mutant/src/gemm.om,$T/$mutant/src/conv.om conv_plan=$T/$mutant/src/gemm.om,$T/$mutant/src/conv.om,$T/$mutant/src/plan.om" OMEGA_UNITS_DEST="$T/$mutant/gen" sh om/units.sh > "$T/$mutant/generate.log" 2>&1 || { cat "$T/$mutant/generate.log";exit 1; }
 "$CC" -std=gnu11 -O2 -ffp-contract=off -Isrc -Itests -Wl,--wrap=omega_gpu_session_open,--wrap=omega_gpu_session_lock,--wrap=omega_gpu_session_unlock,--wrap=omega_gpu_session_free,--wrap=omega_gpu_session_alloc tests/gpu_conv2d_test.c "$T/$mutant/gen/cudapp_om_conv2d.c" "$T/$mutant/gen/cudapp_om_conv_plan.c" "$OUT_DIR/libomega_gpu.a" -lpthread -lm -o "$T/$mutant/test"
 if "$T/$mutant/test" --sim > "$T/$mutant/test.log" 2>&1;then echo "conv2d mutant SURVIVED: $mutant";cat "$T/$mutant/test.log";exit 1;fi
 if ! grep -q 'equal' "$T/$mutant/test.log";then cat "$T/$mutant/test.log";exit 1;fi
 echo "conv2d mutant KILLED after compile: $mutant"
done
echo 'conv2d source mutants: PASS (5 compiled, 5 behaviorally killed)'
ulimit -c 0
"$CC" -std=gnu11 -O2 -ffp-contract=off -Isrc -Itests -DCONV2D_GUARD_MUTANT -Wl,--wrap=omega_gpu_session_open,--wrap=omega_gpu_session_lock,--wrap=omega_gpu_session_unlock,--wrap=omega_gpu_session_free,--wrap=omega_gpu_session_alloc tests/gpu_conv2d_test.c "$OUT_DIR/libomega_gpu.a" -lpthread -lm -o "$T/guard-test"
set +e
"$T/guard-test" --sim > "$T/guard.log" 2>&1
rc=$?
set -e
[ "$rc" = 134 ] || { echo "conv2d direct-open guard failed: rc=$rc";exit 1; }
echo 'conv2d direct-open mutant: PASS (SIGABRT before device access)'
