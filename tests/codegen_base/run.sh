#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
cd "$ROOT"
: "${CC:=cc}"
: "${CODEGEN_BASE_OUT:=$ROOT/build/codegen-base-om}"
OUT=$CODEGEN_BASE_OUT
mkdir -p "$OUT"
# These are generated build profiles, never historical evidence.
rm -f "$OUT"/*.gcda
if [ -n "${OMEGA_OMC:-}" ]; then sh om/units.sh --check; fi
python3 - "$OUT" <<'PY'
import pathlib, re, sys
out = pathlib.Path(sys.argv[1])
s = pathlib.Path('src/omega_blackwell_codegen.c').read_text()
names = sorted(set(re.findall(r'^(?:int|void|size_t|uint32_t|uint16_t) (omega_\w+)\(', s, re.M)))
base = ''.join('#define '+n+' legacy_'+n+'\n' for n in names)
(out/'oracle_base.h').write_text(base)
(out/'oracle_sass.h').write_text(''.join('#define '+n+' sass_'+n+'\n' for n in names))
s = pathlib.Path('src/omega_gpu_om_ir.c').read_text()
names = sorted(set(re.findall(r'^(?:int|const char \*)\s*(omega_\w+)\(', s, re.M)))
(out/'oracle_ir.h').write_text(base+''.join('#define '+n+' legacy_'+n+'\n' for n in names))
PY
FLAGS="-std=gnu11 -O2 -Wall -Wextra -Werror -ffp-contract=off -ffunction-sections -fdata-sections -Isrc"
COV="-O0 --coverage"
$CC $FLAGS $COV -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_BASE \
  -include "$OUT/oracle_base.h" -c src/omega_blackwell_codegen.c -o "$OUT/oracle_base.o"
$CC $FLAGS $COV -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_BASE -include "$OUT/oracle_ir.h" \
  -c src/omega_gpu_om_ir.c -o "$OUT/oracle_ir.o"
$CC $FLAGS -DCUDAPP_MIGRATION_PENDING_C_CODEGEN_BASE -DCUDAPP_MIGRATION_PENDING_C_ENCODER \
  -include "$OUT/oracle_sass.h" -c src/omega_blackwell_codegen.c -o "$OUT/oracle_sass.o"
OBJECTS="$OUT/oracle_base.o $OUT/oracle_ir.o $OUT/oracle_sass.o"
for src in src/omega_blackwell_codegen.c src/omega_gpu_om_ir.c src/omega_blackwell_matmul.c src/omega_cubin.c src/sha256.c \
           src/cudapp_om_*.c src/cudapp_freest_revision.c src/cudapp_freest_epoch.c src/cudapp_freest_f16.c src/cudapp_freest_leftovers.c; do
  name=$(basename "$src" .c)
  $CC $FLAGS -c "$src" -o "$OUT/$name.o"
  OBJECTS="$OBJECTS $OUT/$name.o"
done
$CC $FLAGS tests/codegen_base/differential.c $OBJECTS --coverage -Wl,--gc-sections -o "$OUT/differential"
"$OUT/differential"
$CC $FLAGS -DCODEGEN_BASE_RED tests/codegen_base/differential.c $OBJECTS --coverage -Wl,--gc-sections -o "$OUT/red"
if "$OUT/red" > "$OUT/red.log" 2>&1; then echo "FAIL: one-bit red control survived"; exit 1; fi
echo "PASS: one flipped SASS bit rejected by byte comparison"
gcov -b -c -o "$OUT/oracle_ir.gcno" src/omega_gpu_om_ir.c > "$OUT/gcov-ir.log"
gcov -b -c -o "$OUT/oracle_base.gcno" src/omega_blackwell_codegen.c > "$OUT/gcov-base.log"
mv omega_gpu_om_ir.c.gcov "$OUT/"
mv omega_blackwell_codegen.c.gcov "$OUT/"
for header in omega_bw_bind.h.gcov omega_path_check.h.gcov; do
  if [ -f "$header" ]; then mv "$header" "$OUT/"; fi
done
if [ "${1:-}" = --mutants ]; then
  export CODEGEN_BASE_OBJECTS="$OBJECTS"
  python3 tests/codegen_base/mutants.py
fi
echo "CODEGEN BASE HOST ACCEPTANCE PASS"
