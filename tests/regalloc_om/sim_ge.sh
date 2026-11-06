#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Run inside the device-free bwrap namespace, like every host/simulator gate.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
: "${CC:=cc}"
: "${SIM_GE_OUT:=$ROOT/build/regalloc-om/signed-ge}"
mkdir -p "$SIM_GE_OUT/mutant"
FLAGS="-std=gnu11 -O2 -Wall -Wextra -Werror -Wno-unused-function -ffp-contract=off -ffunction-sections -fdata-sections -Isrc -I$ROOT/build/regalloc-om/include"
ORACLE="-DCUDAPP_MIGRATION_PENDING_C_REGALLOC -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_MATMUL -DCUDAPP_MIGRATION_PENDING_C_ENCODER"
SOURCES="tests/regalloc_om/sim.c src/omega_blackwell_codegen.c src/omega_blackwell_matmul.c src/omega_blackwell_encoder.c src/sha256.c src/cudapp_om_bw_enc_sm121.c src/cudapp_freest_f16.c"
$CC $FLAGS $ORACLE -Itests $SOURCES -Wl,--gc-sections -lm -o "$SIM_GE_OUT/baseline"
"$SIM_GE_OUT/baseline" >"$SIM_GE_OUT/baseline.log" 2>&1
cat "$SIM_GE_OUT/baseline.log"
python3 - "$SIM_GE_OUT/mutant/bw_warp_sim_tc.h" <<'PY'
import pathlib,sys
source=pathlib.Path('tests/bw_warp_sim_tc.h').read_text()
old='case BW_IR_ISETP_GE: l->p0 = (int32_t)a >= (in->src2_vreg >= 0 ? (int32_t)b : (int32_t)in->imm); break;'
new='case BW_IR_ISETP_GE: l->p0 = a >= (in->src2_vreg >= 0 ? b : in->imm); break;'
if source.count(old) != 1:
    raise SystemExit('FAIL unsigned mutant site not unique')
pathlib.Path(sys.argv[1]).write_text(source.replace(old,new))
PY
$CC $FLAGS $ORACLE -I"$SIM_GE_OUT/mutant" -Itests $SOURCES -Wl,--gc-sections -lm -o "$SIM_GE_OUT/unsigned-mutant"
set +e
"$SIM_GE_OUT/unsigned-mutant" >"$SIM_GE_OUT/unsigned-mutant.log" 2>&1
rc=$?
set -e
if [ "$rc" -ne 1 ]; then
    cat "$SIM_GE_OUT/unsigned-mutant.log"
    echo "FAIL unsigned GE mutant exercise=$rc expected=1"
    exit 1
fi
if ! grep -q '^FAIL signed GE ' "$SIM_GE_OUT/unsigned-mutant.log"; then
    cat "$SIM_GE_OUT/unsigned-mutant.log"
    echo 'FAIL unsigned GE mutant failed for an unrelated reason'
    exit 1
fi
cat "$SIM_GE_OUT/unsigned-mutant.log"
echo 'PASS unsigned GE semantic mutant KILLED compile=0 exercise=1'
