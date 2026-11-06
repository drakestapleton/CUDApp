#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Run this whole runner under the device-free bwrap contract.
set -eu
cd "$(dirname "$0")/../.."
OUT=build/packers-om
mkdir -p "$OUT"
rm -f "$OUT"/oracle-*.gcda
CC=${CC:-cc}
FLAGS='-std=gnu11 -O2 -Wall -Wextra -Werror -ffp-contract=off -ffunction-sections -fdata-sections -Isrc'
$CC $FLAGS -O0 --coverage -DCUDAPP_MIGRATION_PENDING_C_PACK tests/regalloc_om/packers.c src/omega_blackwell_codegen.c -Wl,--gc-sections -o "$OUT/oracle"
$CC $FLAGS tests/regalloc_om/packers.c src/omega_blackwell_codegen.c src/cudapp_om_cg_pack_len.c src/cudapp_om_cg_pack_h.c src/cudapp_om_cg_sentin.c -Wl,--gc-sections -o "$OUT/successor"
"$OUT/oracle" > "$OUT/oracle.out"
"$OUT/successor" > "$OUT/successor.out"
cmp "$OUT/oracle.out" "$OUT/successor.out"
"$OUT/successor" --red > "$OUT/red.out"
if cmp -s "$OUT/oracle.out" "$OUT/red.out"; then echo 'FAIL fragment differential red control'; exit 1; fi
ln -sfn ../../src "$OUT/src"
ln -sfn ../../tests "$OUT/tests"
(cd "$OUT" && gcov -b -c oracle-omega_blackwell_codegen.gcno > gcov.log)
$CC $FLAGS tests/regalloc_om/packers_units.c src/cudapp_om_cg_pack_len.c src/cudapp_om_cg_pack_h.c src/cudapp_om_cg_mrow.c -o "$OUT/units"
"$OUT/units"
$CC $FLAGS -O1 -g -fsanitize=address tests/regalloc_om/packers.c src/omega_blackwell_codegen.c src/cudapp_om_cg_pack_len.c src/cudapp_om_cg_pack_h.c src/cudapp_om_cg_sentin.c -Wl,--gc-sections -o "$OUT/row0-asan"
"$OUT/row0-asan" --row0
echo 'PASS five fragment packers and result sentinels bitwise differential, source partition and row0 ASan regression'
