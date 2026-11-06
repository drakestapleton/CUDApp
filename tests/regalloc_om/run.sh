#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
: "${CC:=cc}"
: "${REGALLOC_OUT:=$ROOT/build/regalloc-om}"
mkdir -p "$REGALLOC_OUT"
# The successor links the portable generated C of om/codegen (om/units.sh; src/cudapp_om_<unit>.c). The pinned
# compiler regenerates it byte for byte first, so these sources are the checked lowering of the .om under test.
if [ -n "${OMEGA_OMC:-}" ]; then sh om/units.sh --check; fi
UNITS="src/cudapp_om_regalloc.c src/cudapp_om_regalloc_ir.c src/cudapp_om_regalloc_live.c src/cudapp_om_cg_matmul.c
  src/cudapp_om_cg_matmul_params.c src/cudapp_om_cg_matmul_i32.c src/cudapp_om_cg_matmul_tensor.c
  src/cudapp_om_cg_matmul_loop.c src/cudapp_om_cg_matmul_frag.c src/cudapp_om_cg_matmul_block.c
  src/cudapp_om_cg_vecadd.c src/cudapp_om_bw_enc_sm121.c src/cudapp_freest_f16.c src/cudapp_om_core_sha.c src/cudapp_om_cg_bind.c src/cudapp_freest_leftovers.c"
UNITS="$UNITS src/cudapp_om_cg_fields.c src/cudapp_om_cg_replay.c src/cudapp_om_cg_life.c
  src/cudapp_om_cg_invar.c src/cudapp_om_cg_budget.c"
HAZARD="src/cudapp_om_bw_hazard.c src/cudapp_om_bw_hazard_dec.c"
python3 - "$REGALLOC_OUT/oracle_names.h" <<'PY'
import pathlib,re,sys
source=pathlib.Path('src/omega_blackwell_codegen.c').read_text()
names=sorted(set(re.findall(r'^(?:int|void|size_t|uint32_t|uint16_t) (omega_\w+)\(',source,re.M)))
pathlib.Path(sys.argv[1]).write_text(''.join('#define '+n+' legacy_'+n+'\n' for n in names))
for f in ('matmul','realize'):
    s=pathlib.Path('src/omega_blackwell_'+f+'.c').read_text()
    ns=sorted(set(re.findall(r'^(?:int|void|size_t|float|double|uint16_t|uint32_t) (omega_\w+)\(',s,re.M)))
    pathlib.Path(sys.argv[1]).with_name(f+'_names.h').write_text(''.join('#define '+n+' legacy_'+n+'\n' for n in ns))
PY
FLAGS="-std=gnu11 -O2 -Wall -Wextra -Werror -ffunction-sections -fdata-sections -Isrc"
$CC $FLAGS -DCUDAPP_MIGRATION_PENDING_C_REGALLOC -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_MATMUL -DCUDAPP_MIGRATION_PENDING_C_ENCODER -DCUDAPP_MIGRATION_PENDING_C_BIND -include "$REGALLOC_OUT/oracle_names.h" -c src/omega_blackwell_codegen.c -o "$REGALLOC_OUT/oracle.o"
$CC $FLAGS -c src/omega_blackwell_codegen.c -o "$REGALLOC_OUT/successor.o"
$CC $FLAGS tests/regalloc_om/liveness.c "$REGALLOC_OUT/successor.o" src/omega_gpu_om_ir.c $UNITS \
  -Wl,--gc-sections -o "$REGALLOC_OUT/liveness"
"$REGALLOC_OUT/liveness"
$CC $FLAGS tests/regalloc_om/differential.c "$REGALLOC_OUT/oracle.o" "$REGALLOC_OUT/successor.o" \
  src/omega_gpu_om_ir.c src/omega_blackwell_matmul.c src/sha256.c $UNITS \
  -Wl,--gc-sections -o "$REGALLOC_OUT/differential"
"$REGALLOC_OUT/differential" "$@"
$CC $FLAGS -Wno-unused-function -ffp-contract=off -Itests tests/regalloc_om/sim.c "$REGALLOC_OUT/successor.o" \
  src/omega_gpu_om_ir.c src/omega_blackwell_matmul.c src/omega_blackwell_encoder.c src/sha256.c $UNITS \
  -Wl,--gc-sections -lm -o "$REGALLOC_OUT/sim"
"$REGALLOC_OUT/sim"
HZ_LEGACY="-Domega_bw_decode=legacy_omega_bw_decode -Domega_bw_hazard_check=legacy_omega_bw_hazard_check
  -Domega_bw_hazard_kind=legacy_omega_bw_hazard_kind"
$CC $FLAGS -DCUDAPP_MIGRATION_PENDING_C_HAZARD $HZ_LEGACY -c src/omega_bw_hazard.c -o "$REGALLOC_OUT/hz_oracle.o"
$CC $FLAGS tests/regalloc_om/hazard_diff.c "$REGALLOC_OUT/hz_oracle.o" src/omega_bw_hazard.c $HAZARD \
  -Wl,--gc-sections -o "$REGALLOC_OUT/hazard_diff"
"$REGALLOC_OUT/hazard_diff"
for f in matmul realize; do
  $CC $FLAGS -DCUDAPP_MIGRATION_PENDING_C_BIND -include "$REGALLOC_OUT/${f}_names.h" -c src/omega_blackwell_$f.c \
    -o "$REGALLOC_OUT/bind_oracle_$f.o"
done
$CC $FLAGS tests/regalloc_om/bind_diff.c "$REGALLOC_OUT/bind_oracle_matmul.o" "$REGALLOC_OUT/bind_oracle_realize.o" \
  "$REGALLOC_OUT/oracle.o" "$REGALLOC_OUT/successor.o" src/omega_blackwell_matmul.c src/omega_blackwell_realize.c \
  src/omega_blackwell_encoder.c src/omega_gpu_om_ir.c src/sha256.c $UNITS -Wl,--gc-sections -o "$REGALLOC_OUT/bind_diff"
"$REGALLOC_OUT/bind_diff"
