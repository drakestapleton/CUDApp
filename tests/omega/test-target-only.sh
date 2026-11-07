#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Report host checks that this machine does not run. make passes the names in
# CUDAPP_TARGET_ONLY_TESTS. Exit 0 means the report was printed. It is not a
# pass of those checks. Invoking one of them directly exits 2.
set -eu
host=$(uname -m)
if [ $# -eq 0 ]; then
    echo "target-only report: NOT_RUN (this host is $host and runs the pinned compiler; make test runs the seeded binding and the pinned-compiler rows)"
    exit 0
fi
for name in "$@"; do
    echo "$name: NOT_RUN (requires the pinned AArch64 omc; not part of the portable host suite)"
done
echo "target-only report: the lines above are refusals. make $1 and the other named targets exit 2 unless OMEGA_OMC is given and executes on this host. This report exiting 0 is not a pass of them."
