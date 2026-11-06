#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host only. Invoke inside the required bwrap/taskset/nice envelope.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"
tmp=build/dagsim-corpus
mkdir -p "$tmp"
. tests/tools-om/lib.sh
tom_pin
srcs="tools/dagsim/dagsim_cli.c tools/dagsim/dagsim.c src/runtime/rx_costmodel.c src/sha256.c src/cudapp_om_dag_eval.c src/cudapp_om_dag_parse.c src/cudapp_om_dag_fmt.c src/cudapp_om_dag_res.c src/cudapp_om_dag_diff.c"
# Integrated runtime cost-model and SHA decisions are Omega providers.
srcs="$srcs src/cudapp_om_core_sha.c src/cudapp_om_cm_*.c"
$CC $TOM_CFLAGS -Itools/dagsim -DCUDAPP_MIGRATION_PENDING_C_TOOLS=1 -DCUDAPP_MIGRATION_PENDING_C_DAGSIM_PARSE=1 -DCUDAPP_MIGRATION_PENDING_C_DAGSIM_RESOLVE=1 $srcs -lm -o "$tmp/ds_c"
$CC $TOM_CFLAGS -Itools/dagsim $srcs -lm -o "$tmp/ds_om"
python3 tests/dagsim/corpus_diff.py "$tmp/ds_c" "$tmp/ds_om" 1200
rc=0
python3 tests/dagsim/corpus_diff.py "$tmp/ds_c" /bin/true 50 || rc=$?
[ "$rc" = 1 ] || { echo "FAIL corpus driver negative control rc=$rc"; exit 1; }
echo 'corpus driver negative control: caught (rc=1)'
"$tmp/ds_c" eval tests/dagsim/fixtures/valid_disjoint_80.oev --asof 2026-10-20 > "$tmp/oracle.out"
"$tmp/ds_om" eval tests/dagsim/fixtures/valid_disjoint_80.oev --asof 2026-10-20 > "$tmp/omega.out"
tom_flip "$tmp/oracle.out" "$tmp/omega.out"
echo 'dagsim corpus: PASS (1200 cases, driver and flipped-record red controls)'
