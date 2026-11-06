#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host verification of native/share/share_native.om, the OM-2 function-mode zero-copy unit for
# the one hosted native library. External harness; not a runtime dependency. It builds a
# single-input hosted library with omega-bridge tools/native --host into $OUT/lib as a test
# artifact (never linked into the product; Integration generates the combined library), runs
# tests/share/native-share-test.c against the seccomp fake RM, then rebuilds the library from
# each mutant in tests/share/native-mutants.txt and requires the test to fail.
# Usage: run-share-native.sh [--no-mutants]
# Env (all required): OMEGA_NATIVE_OMC (pinned program-mode omc for tools/native),
# OMEGA_NATIVE_OMC2 (pinned OM-2 omc), ZERO_COPY_NATIVE_BRIDGE (omega-bridge checkout with the
# shared explicit native context). Optional: CC, ZERO_COPY_OUT (build/zero-copy).
# Exit 2 = NOT_RUN (missing toolchain); 1 = FAIL; 0 = PASS.
set -eu
MODE=${1:-}
OUT=${ZERO_COPY_OUT:-build/zero-copy}/native
SRC=native/share/share_native.om
for v in OMEGA_NATIVE_OMC OMEGA_NATIVE_OMC2 ZERO_COPY_NATIVE_BRIDGE; do
    eval "x=\${$v:-}"
    if test -z "$x"; then
        echo "zero-copy native: NOT_RUN ($v unset)"
        exit 2
    fi
done
pin() {
    got=$(sha256sum < "$2" | cut -c1-64)
    if test "$got" != "$3"; then
        echo "zero-copy native: FAIL $1 sha256 $got, want $3" >&2
        exit 1
    fi
}
# omega-mv dbedb2ca (OM-2 explicit native context, omega-bridge#69 248a1984 / A64 #1047 b8f8f137).
pin OMEGA_NATIVE_OMC2 "$OMEGA_NATIVE_OMC2" b8f8f1371bc0303f8da02db8b754e085d5dd852620c8d8d676c5769c233d9195
pin OMEGA_NATIVE_OMC "$OMEGA_NATIVE_OMC" 734f0a39319fbfa930bcc62cb30dbf06d47d78e126372b74f5baf478e6126b61
T=third_party/nvidia-open-580.173.02
TOP=$(pwd)
rm -rf "$OUT"
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)

# Build the library from one unit into $1 and the test against it; quiet unless it fails.
build() {
    (cd "$ZERO_COPY_NATIVE_BRIDGE" && tools/native --host "$1" "$2") > "$1.log" 2>&1 || return 1
    ${CC:-cc} -std=gnu11 -O2 -Wall -Wextra -Werror -pthread \
        -I$T/src/common/sdk/nvidia/inc -I$T/kernel-open/common/inc \
        -I$T/kernel-open/nvidia-uvm -I$T/src/nvidia/arch/nvalloc/unix/include -I"$1" \
        tests/share/native-share-test.c -L"$1" -Wl,-rpath,"$1" -lomega_native -o "$1/native-share-test"
}

if ! build "$OUT/lib" "$TOP/$SRC"; then
    cat "$OUT/lib.log" >&2
    echo "zero-copy native: FAIL build of $SRC" >&2
    exit 1
fi
"$OUT/lib/native-share-test"
test "$MODE" = --no-mutants && exit 0

# Mutants: "search|replace" lines; the search text must occur exactly once in the unit.
n=0
while IFS='|' read -r from to; do
    case "$from" in '' | '#'*) continue ;; esac
    n=$((n + 1))
    count=$(grep -cF -- "$from" "$SRC" || true)
    if test "$count" != 1; then
        echo "zero-copy native: FAIL mutant $n site found $count times: $from" >&2
        exit 1
    fi
    dir="$OUT/m$n"
    mkdir -p "$dir"
    awk -v f="$from" -v t="$to" '{ i = index($0, f); if (i) $0 = substr($0, 1, i - 1) t substr($0, i + length(f)); print }' \
        "$SRC" > "$dir/share_native.om"
    if ! build "$dir/lib" "$dir/share_native.om"; then
        echo "zero-copy native: FAIL mutant $n does not build (a vacuous mutant): $from -> $to" >&2
        exit 1
    fi
    if "$dir/lib/native-share-test" > "$dir/run.log" 2>&1; then
        echo "zero-copy native: FAIL mutant $n survived: $from -> $to" >&2
        exit 1
    fi
    echo "zero-copy native: mutant $n caught ($(tail -n 1 "$dir/run.log")): $from -> $to"
done < tests/share/native-mutants.txt
echo "ZERO-COPY SHARE NATIVE MUTANTS PASS ($n/$n caught)"
