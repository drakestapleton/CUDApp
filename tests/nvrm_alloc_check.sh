#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
set -eu
[ ! -e /dev/nvidiactl ] || { echo 'REFUSED: isolate /dev before NVRM allocation checks' >&2; exit 2; }
exec python3 tests/nvrm_alloc_check.py
