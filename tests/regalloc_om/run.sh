#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
: "${OMEGA_NATIVE_OMC:=$HOME/work/cudastack/lanes/bh-tools/omc-native}"
: "${OMEGA_NATIVE_OMC2:=$HOME/work/cudastack/omega-mv/seed/out/omc}"
: "${OMEGA_BRIDGE:=$HOME/work/cudastack/omega-bridge}"
: "${CC:=cc}"
: "${REGALLOC_OUT:=$ROOT/build/regalloc-om}"
export OMEGA_NATIVE_OMC OMEGA_NATIVE_OMC2
mkdir -p "$REGALLOC_OUT/include"
"$OMEGA_BRIDGE/tools/native" "$REGALLOC_OUT/native" om/codegen/regalloc.om om/codegen/regalloc_ir.om om/codegen/cg_matmul.om
cp "$REGALLOC_OUT/native/omega_native.h" "$REGALLOC_OUT/include/cudapp_hosted_native.h"
python3 - "$REGALLOC_OUT/oracle_names.h" <<'PY'
import pathlib,re,sys
source=pathlib.Path('src/omega_blackwell_codegen.c').read_text()
names=sorted(set(re.findall(r'^(?:int|void|size_t|uint32_t|uint16_t) (omega_\w+)\(',source,re.M)))
pathlib.Path(sys.argv[1]).write_text(''.join('#define '+n+' legacy_'+n+'\n' for n in names))
PY
FLAGS="-std=gnu11 -O2 -Wall -Wextra -Werror -ffunction-sections -fdata-sections -Isrc -I$REGALLOC_OUT/include"
$CC $FLAGS -DCUDAPP_MIGRATION_PENDING_C_REGALLOC -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_MATMUL -DCUDAPP_MIGRATION_PENDING_C_ENCODER -include "$REGALLOC_OUT/oracle_names.h" -c src/omega_blackwell_codegen.c -o "$REGALLOC_OUT/oracle.o"
$CC $FLAGS -c src/omega_blackwell_codegen.c -o "$REGALLOC_OUT/successor.o"
$CC $FLAGS tests/regalloc_om/differential.c "$REGALLOC_OUT/oracle.o" "$REGALLOC_OUT/successor.o" src/omega_blackwell_matmul.c src/sha256.c src/cudapp_om_bw_enc_sm121.c src/cudapp_freest_f16.c -Wl,--gc-sections -L"$REGALLOC_OUT/native" -Wl,-rpath,"$REGALLOC_OUT/native" -lomega_native -o "$REGALLOC_OUT/differential"
"$REGALLOC_OUT/differential" "$@"
