#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host verification of native/share/share.om (ROADMAP section 3, zero-copy shared memory).
# External harness; not a runtime dependency. No device file is opened: every RM, UVM and
# device-file word in a private copy of context.om + share.om is renamed to the fake in
# tests/share/fake-rm.om, and a guard refuses the run if any raw device word survives.
# Usage: run-share-host.sh [--no-mutants | --suite-only]
#   --suite-only  full suite only, for mutants.sh; prints "zero-copy: compiled" once built.
# Env: OMEGA_OMC (pinned), CC, ZERO_COPY_OUT (build/zero-copy), ZERO_COPY_SHARE_SRC
# (native/share/share.om), ZERO_COPY_ABI (reuse a generated abi.om instead of rebuilding).
set -eu
: "${OMEGA_OMC:?set the pinned seeded compiler}"
MODE=${1:-}
OUT=${ZERO_COPY_OUT:-build/zero-copy}
SRC=${ZERO_COPY_SHARE_SRC:-native/share/share.om}
PIN=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
GOT=$(sha256sum < "$OMEGA_OMC" | cut -c1-64)
if test "$GOT" != "$PIN"; then
    echo "zero-copy: FAIL OMEGA_OMC sha256 $GOT, want the pinned omc $PIN (omega-mv e5b32567)" >&2
    exit 1
fi
mkdir -p "$OUT"
T=third_party/nvidia-open-580.173.02

# Host reference: layouts, request numbers and statuses from the pinned NVIDIA headers.
if test -n "${ZERO_COPY_ABI:-}"; then
    cp "$ZERO_COPY_ABI" "$OUT/abi.om"
else
    ${CC:-cc} -std=gnu11 -Wall -Wextra -Werror \
        -I$T/src/common/sdk/nvidia/inc -I$T/kernel-open/common/inc \
        -I$T/kernel-open/nvidia-uvm -I$T/src/nvidia/arch/nvalloc/unix/include \
        tests/share/abi-oracle.c -o "$OUT/abi-oracle"
    "$OUT/abi-oracle" > "$OUT/abi.om"
fi

# Rename device words; drop the shared byte-array operands, which the fake reads directly.
fake() {
    sed -E '/^note /!{
s/fopen nvctl 0 len nvctl 524290 0/fkopen 1/
s/fopen nvdev 0 len nvdev 524290 0/fkopen 2/
s/fopen nvuvm 0 len nvuvm 524290 0/fkopen 3/
/\b(devcall|devctl|uvmctl)\b/s/ (nvhdr|nvpar|nvcard) / /g
s/\bdevcall\b/fkcall/g
s/\bdevctl\b/fkctl/g
s/\buvmctl\b/fkuvm/g
s/\bfclose\b/fkclose/g
s/\bmapbind\b/fkbind/g
s/\bmapnew\b/fknew/g
}' "$@"
}

# Build one variant: $1 main body function, $2 output stem.
build() {
    cat native/nvrm/context.om "$SRC" | fake > "$OUT/$2.dev.om"
    if grep -v '^note ' "$OUT/$2.dev.om" | grep -nE '\b(devcall|devctl|uvmctl|fopen|fclose|mapbind|mapnew)\b'; then
        echo "zero-copy: FAIL raw device word survived the rewrite" >&2
        exit 1
    fi
    if grep -v '^note ' tests/share/fake-rm.om tests/share/share-test.om | grep -nE '\b(devcall|devctl|uvmctl|fopen)\b'; then
        echo "zero-copy: FAIL fake or test names a device word" >&2
        exit 1
    fi
    { cat "$OUT/$2.dev.om" "$OUT/abi.om" tests/share/fake-rm.om tests/share/share-test.om
      printf 'fn main to int\nret %s\nend\n' "$1"; } > "$OUT/$2.om"
    "$OMEGA_OMC" < "$OUT/$2.om" > "$OUT/$2"
    chmod +x "$OUT/$2"
}

# Simulated tier: the full suite against the fake RM.
build tall share
echo "zero-copy: compiled"
rc=0
"$OUT/share" > "$OUT/share.stdout" 2> "$OUT/share.stderr" || rc=$?
cat "$OUT/share.stderr" >&2
if test "$rc" != 0 || test "$(cat "$OUT/share.stdout")" != ZERO_COPY_SHARE_HOST_PASS; then
    echo "zero-copy: FAIL suite exited $rc (103 = write through a read-only mapping)" >&2
    exit 1
fi
echo "zero-copy: suite pass"
test "$MODE" = --suite-only && exit 0

# Negative control: a write through a read-only import must trap (exit 103), not land.
build trowrite rowrite
rc=0
"$OUT/rowrite" > "$OUT/rowrite.stdout" 2>/dev/null || rc=$?
test "$(cat "$OUT/rowrite.stdout")" = ZERO_COPY_SHARE_RO_ARMED
if test "$rc" != 103; then
    echo "zero-copy: FAIL read-only import write exited $rc, want trap 103" >&2
    exit 1
fi
echo "zero-copy: read-only write trapped 103"

# Chip gate: compiled from the unrewritten sources so the window runs exactly this binary.
# It opens /dev/nvidia*; this script never executes it (mode 0644).
{ cat native/nvrm/context.om "$SRC" tests/share/chip-share.om
  printf 'fn main to int\nret cmain\nend\n'; } > "$OUT/chip-share.om"
"$OMEGA_OMC" < "$OUT/chip-share.om" > "$OUT/chip-share"
chmod 0644 "$OUT/chip-share"
echo "zero-copy: chip gate compiled, NOT_RUN (needs an authorized GPU window)"

if test "$MODE" != --no-mutants; then
    ZERO_COPY_OUT="$OUT" sh tests/share/mutants.sh
fi
sha256sum "$OUT/abi.om" "$OUT/share" "$OUT/chip-share" "$SRC"
echo "ZERO-COPY SHARE HOST PASS"
