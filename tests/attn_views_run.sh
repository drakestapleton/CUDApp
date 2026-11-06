#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
set -eu
cd "$(dirname "$0")/.."
t=$(mktemp -d)
trap 'rm -rf "$t"' EXIT
${CC:-cc} -std=gnu11 -O2 -ffp-contract=off -Isrc tests/attn_views_diff.c src/cudapp_om_attn_args.c -o "$t/diff"
"$t/diff"
