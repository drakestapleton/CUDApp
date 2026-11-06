#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Mutation test for native/streams/submit.om: each mutant is regenerated with the
# pinned AArch64 context toolchain (same variables as native/nvrm/units.sh), linked
# into the fake-device fixture, and must make the C-vs-Omega image compare (or the
# fixture's own asserts) fail. HOST_ONLY.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BRIDGE=${OMEGA_NVRM_BRIDGE:?set OMEGA_NVRM_BRIDGE to the pinned hosted/context bridge}
: "${OMEGA_NATIVE_OMC:?}" "${OMEGA_NATIVE_OMC2:?}"
[ "$(uname -m)" = aarch64 ] || { echo "NOT_RUN: submit mutants regenerate AArch64 only" >&2; exit 3; }
OUT=${OUT_DIR:-build}
CC=${CC:-cc}
CFLAGS="-std=gnu11 -Wall -Wextra -Werror -D_GNU_SOURCE -O2 -Isrc -Im16 -Invrm -Ithird_party/nvidia-open-580.173.02/src/common/sdk/nvidia/inc -Ithird_party/nvidia-open-580.173.02/kernel-open/common/inc -Ithird_party/nvidia-open-580.173.02/kernel-open/nvidia-uvm -Ithird_party/nvidia-open-580.173.02/src/nvidia/arch/nvalloc/unix/include -Ithird_party/aien-gpu-abi1"
cd "$ROOT"
[ -f "$OUT/streams-submit-c.img" ] || { echo "run make test-streams-submit first" >&2; exit 2; }
SRCS="src/omega_ampere_qmd.c src/cudapp_om_amp_runtime.c src/cudapp_om_nvrm_wire_pack.c src/cudapp_om_nvrm_wire_reply.c src/cudapp_om_nvrm_heap_wire.c src/cudapp_om_nvrm_uvm_wire.c src/cudapp_om_nvrm_uvm_map_wire.c src/omega_gpu_chip.c src/cudapp_om_chip_admit.c"
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
export OMEGA_NATIVE_OMC OMEGA_NATIVE_OMC2
caught=0 n=0
while IFS='|' read -r name from to; do
    [ -n "$name" ] || continue
    n=$((n + 1))
    [ "$(grep -cF -- "$from" native/streams/submit.om)" = 1 ] || { echo "FAIL mutant $name: site not unique" >&2; exit 1; }
    awk -v f="$from" -v t="$to" '{ i = index($0, f); if (i) $0 = substr($0, 1, i - 1) t substr($0, i + length(f)); print }' \
        native/streams/submit.om > "$T/submit.om"
    rm -rf "$T/gen"
    "$BRIDGE/tools/native" --host --target aarch64linux "$T/gen" native/nvrm/transport.om "$T/submit.om" >/dev/null
    cp "$T/gen/omega_native.h" "$T/cudapp_nvrm_native.h"
    $CC -I"$T" $CFLAGS -DCUDAPP_MIGRATION_PENDING_C_NVRM=0 -DCUDAPP_MIGRATION_PENDING_C_SUBMIT=0 \
        -o "$T/m" nvrm/submit_test.c "$T/gen/omega_native.c" $SRCS
    if "$T/m" "$T/m.img" >/dev/null 2>&1 && cmp -s "$OUT/streams-submit-c.img" "$T/m.img"; then
        echo "FAIL mutant $name survived"
    else
        echo "mutant $name caught"; caught=$((caught + 1))
    fi
done <<'EOF'
valid-bit|shl ulong 1 41|shl ulong 1 40
address-mask|sub va and va 3|va
slot|mul slot 8 8|mul add slot 1 8 8
gpput-wrap|rem and add pos 1 4294967295 entries|rem add pos 1 entries
gpput-stale|rem and add pos 1 4294967295 entries|rem pos entries
doorbell-offset|ret mapset bell off 4 token|ret mapset bell 0 4 token
nwords-bound|gt nwords 2097151|gt nwords 4294967295
EOF
[ "$caught" = "$n" ] || { echo "STREAMS SUBMIT MUTANTS FAIL ($caught/$n caught)"; exit 1; }
echo "STREAMS SUBMIT MUTANTS PASS ($caught/$n caught)"
