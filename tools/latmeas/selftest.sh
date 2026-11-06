#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host self-test: encode, disassemble, oracle, NAK diff, template dry run.
# No device is opened. Synthetic template output stays in a temp file.
set -eu
bin=${1:?binary}
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"
"$bin" --self-test
"$bin" --print-plan >/dev/null
python3 tools/latmeas/diff_nak.py --check >/dev/null
python3 tools/latmeas/fill_template.py --self-test
dir=$(mktemp -d)
trap 'rm -rf "$dir"' EXIT
"$bin" --emit "$dir/kernels" >/dev/null
test -f "$dir/kernels/manifest.txt"
test -f "$dir/kernels/alu_alu_4.bin"
if grep -q '90 s' tools/latmeas/run_spark.sh; then
    echo "FAIL: run_spark.sh still quotes a 90 s bound" >&2
    exit 1
fi
sh tools/latmeas/run_spark.sh --receipt-self-test
echo "latmeas host self-test PASS"
