#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Invoke only inside the lane's device-isolated bwrap envelope.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
OUT=${OUT:-build/api-rest}
mkdir -p "$OUT"
objs=
for src in src/cudapp_om_ar_*.c src/cudapp_om_cg_pack_len.c src/cudapp_om_cg_pack_h.c src/cudapp_om_cg_mrow.c; do
  name=$(basename "$src" .c)
  ${CC:-cc} -std=gnu11 -O2 -ffp-contract=off ${REST_CFLAGS:-} -Isrc -c "$src" -o "$OUT/$name.o"
  objs="$objs $OUT/$name.o"
done
${CC:-cc} -std=gnu11 -Wall -Wextra -Werror -O2 -ffp-contract=off ${REST_CFLAGS:-} -Isrc -c tests/api_rest_diff.c -o "$OUT/api_rest_diff.o"
${CC:-cc} ${REST_CFLAGS:-} -o "$OUT/api_rest_diff" "$OUT/api_rest_diff.o" $objs build/libomega_gpu.a -lm -lpthread
"$OUT/api_rest_diff"
