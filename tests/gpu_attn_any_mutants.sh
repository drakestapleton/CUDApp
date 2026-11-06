#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Mutate copies only; source generation must succeed, then the behavioral test
# (not compiler rejection) must kill each mutant. Never opens the device.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
: "${OMEGA_OMC:?set pinned OMEGA_OMC}"
: "${OMEGA_BRIDGE:?set pinned OMEGA_BRIDGE}"
OUT_DIR=${OUT_DIR:-build}
CC=${CC:-cc}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
for mutant in mask group normalize tail refusal; do
    mkdir -p "$T/$mutant/src" "$T/$mutant/gen"
    cp om/attn-any/*.om "$T/$mutant/src/"
    python3 - "$T/$mutant/src" "$mutant" <<'PY'
import pathlib, sys
root = pathlib.Path(sys.argv[1])
changes = {
    'mask': ('kernel.om', 'if ne and mode 256 0', 'if eq and mode 256 0'),
    'group': ('plan.om', 'kh = div head div get s 3 get s 4', 'kh = ulong 0'),
    'normalize': ('kernel.om', 'aa_rg get r 22 get r 12 AN AN', 'aa_rg get r 22 get r 44 AN AN'),
    'tail': ('kernel.om', 'flags = or flags 1536', 'flags = ulong 0'),
    'refusal': ('plan.om', 'gt get s 2 256', 'gt get s 2 257'),
}
f, old, new = changes[sys.argv[2]]
p = root/f
text = p.read_text()
assert text.count(old) == 1, (f, old, text.count(old))
p.write_text(text.replace(old, new))
PY
    OMEGA_UNITS="attn_any=$T/$mutant/src/emit.om,$T/$mutant/src/kernel.om attn_any_plan=$T/$mutant/src/plan.om" OMEGA_UNITS_DEST="$T/$mutant/gen" sh om/units.sh > "$T/$mutant/generate.log" 2>&1 || { cat "$T/$mutant/generate.log"; exit 1; }
    "$CC" -std=gnu11 -O2 -ffp-contract=off -Isrc -Itests tests/gpu_attn_any_test.c "$T/$mutant/gen/cudapp_om_attn_any.c" "$T/$mutant/gen/cudapp_om_attn_any_plan.c" "$OUT_DIR/libomega_gpu.a" -lpthread -lm -o "$T/$mutant/test"
    if "$T/$mutant/test" --quick > "$T/$mutant/test.log" 2>&1; then
        echo "attn-any mutant SURVIVED: $mutant"
        cat "$T/$mutant/test.log"
        exit 1
    fi
    echo "attn-any mutant KILLED: $mutant"
done
echo 'attn-any source mutants: PASS (5 killed after successful generation)'
