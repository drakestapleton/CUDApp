#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Must run under the lane bwrap wrapper. No live device operations.
set -eu
[ ! -e /dev/nvidiactl ] || { echo 'REFUSED: tests require isolated /dev'; exit 3; }
exec python3 "$(dirname "$0")/nvrm_struct_check.py"
