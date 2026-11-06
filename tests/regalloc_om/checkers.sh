#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Invoke this entire runner under the device-free bwrap contract.
set -eu
cd "$(dirname "$0")/../.."
OUT=build/checkers-om
mkdir -p "$OUT"
rm -f "$OUT"/oracle-*.gcda "$OUT"/successor-*.gcda
CC=${CC:-cc}
FLAGS='-std=gnu11 -O0 --coverage -Wall -Wextra -Werror -ffunction-sections -fdata-sections -Isrc'
$CC $FLAGS -DCUDAPP_MIGRATION_PENDING_C_EM -DCUDAPP_MIGRATION_PENDING_C_RECONV tests/regalloc_om/checkers.c -Wl,--gc-sections -o "$OUT/oracle"
$CC $FLAGS tests/regalloc_om/checkers.c src/cudapp_om_cg_em.c src/cudapp_om_cg_reconv.c -Wl,--gc-sections -o "$OUT/successor"
"$OUT/oracle" > "$OUT/oracle.out"
"$OUT/successor" > "$OUT/successor.out"
cmp "$OUT/oracle.out" "$OUT/successor.out"
"$OUT/successor" --red > "$OUT/red.out"
if cmp -s "$OUT/oracle.out" "$OUT/red.out"; then echo 'FAIL checker red control'; exit 1; fi
ln -sfn ../../src "$OUT/src"
ln -sfn ../../tests "$OUT/tests"
(cd "$OUT" && gcov -b -c oracle-checkers.gcno > gcov.log)
$CC $FLAGS -fsanitize=undefined -fno-sanitize-recover=undefined -DCUDAPP_MIGRATION_PENDING_C_EM -DCUDAPP_MIGRATION_PENDING_C_RECONV tests/regalloc_om/checkers.c -Wl,--gc-sections -o "$OUT/log-overflow"
"$OUT/log-overflow" --log-overflow > "$OUT/log-overflow.out"
grep -qx 'log-overflow -1' "$OUT/log-overflow.out"
echo 'PASS checker differential, ordered failure state and red control'
