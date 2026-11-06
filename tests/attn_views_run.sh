#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
set -eu
cd "$(dirname "$0")/.."
exec python3 tests/attn_views_check.py "$@"
