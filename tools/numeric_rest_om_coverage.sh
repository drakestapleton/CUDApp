#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Verification only. Invoke under the device-hidden CPU envelope.
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT=${1:?usage: numeric_rest_om_coverage.sh OUTPUT_DIRECTORY}
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
CC=${CC:-cc}
$CC -std=gnu11 -Wall -Wextra -Werror -D_GNU_SOURCE -O0 --coverage -ffp-contract=off -DCUDAPP_MIGRATION_PENDING_C_NUMERIC_REST_DIFF -ffunction-sections -fdata-sections -Wl,--gc-sections -I"$ROOT/src" -o "$OUT/diff" "$ROOT/tests/numeric_rest_om_diff_test.c" "$ROOT/src/omega_numeric.c" "$ROOT/src/omega_numeric_reduce.c" "$ROOT/src/omega_blackwell_qmd.c" "$ROOT/src/cudapp_om_bw_qmd.c" "$ROOT"/src/cudapp_om_numeric*.c -lm
"$OUT/diff"
(cd "$OUT" && gcov -b -c -f diff-omega_numeric.gcno diff-omega_numeric_reduce.gcno)
